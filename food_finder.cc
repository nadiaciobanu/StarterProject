/*
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <iostream>
#include <memory>
#include <string>
#include <sstream>
#include <tuple>

#include <grpcpp/grpcpp.h>
#include <grpcpp/opencensus.h>

#include "food.grpc.pb.h"

#include "absl/strings/string_view.h"
#include "absl/time/clock.h"
#include "opencensus/exporters/trace/zipkin/zipkin_exporter.h"
#include "opencensus/exporters/stats/stackdriver/stackdriver_exporter.h"
#include "opencensus/trace/sampler.h"
#include "opencensus/trace/span.h"
#include "opencensus/stats/stats.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using grpc::StatusCode;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;

using food::InternalFoodService;
using food::ExternalFoodService;
using food::SupplierRequest;
using food::SupplierReply;
using food::VendorRequest;
using food::VendorReply;
using food::FinderRequest;
using food::FinderReply;

const std::string kGeneralErrorString = "ERROR";
const int kServerTimeout = 85;

// For metrics
ABSL_CONST_INIT const absl::string_view kRPCErrorMeasureName = "rpc_error_count";
ABSL_CONST_INIT const absl::string_view kRPCCountMeasureName = "rpc_count";
ABSL_CONST_INIT const absl::string_view kRPCLatencyMeasureName = "rpc_latency";

opencensus::stats::MeasureInt64 RPCErrorCountMeasure() {
  static const auto measure =
      opencensus::stats::MeasureInt64::Register(
          kRPCErrorMeasureName, "Number of RPC errors encountered.", "By");
  return measure;
}

opencensus::stats::MeasureInt64 RPCCountMeasure() {
  static const auto measure =
      opencensus::stats::MeasureInt64::Register(
          kRPCCountMeasureName, "Number of RPC calls made.", "By");
  return measure;
}

opencensus::stats::MeasureDouble RPCLatencyMeasure() {
  static const auto measure =
      opencensus::stats::MeasureDouble::Register(
          kRPCLatencyMeasureName, "Latency of RPC calls made.", "ms");
  return measure;
}

opencensus::tags::TagKey MethodKey() {
  static const opencensus::tags::TagKey key =
      opencensus::tags::TagKey::Register("method");
  return key;
}


class FoodFinder {
 public:
    FoodFinder(std::shared_ptr<Channel> channel)
            : stub_(InternalFoodService::NewStub(channel)) {}

    // Call to FoodSupplier
    // Return bool to signal success or failure.
    // If success, also return list of vendors. If failure, also return error string.
    std::tuple<bool, std::vector<std::string>> GetVendors(const std::string& ingredient) {
        SupplierRequest request;
        request.set_ingredient(ingredient);

        SupplierReply reply;
        ClientContext context;

        // Set timeout for server
        context.set_deadline(std::chrono::system_clock::now() +
            std::chrono::milliseconds(kServerTimeout));

        absl::Time start = absl::Now();

        Status status = stub_->GetVendors(&context, request, &reply);

        // Record latency
        absl::Time end = absl::Now();
        double latency = absl::ToDoubleMilliseconds(end - start);
        opencensus::stats::Record({{RPCLatencyMeasure(), latency}});

        // Record RPC call
        opencensus::stats::Record({{RPCCountMeasure(), 1}});

        if (!status.ok()) {
            std::cout << status.error_code() << ": " << status.error_message()
                      << std::endl;

            // Record error for metrics
            opencensus::stats::Record({{RPCErrorCountMeasure(), 1}});

            std::string custom_error_message = "FoodSupplier " + status.error_message();
            std::vector<std::string> error = {custom_error_message};

            return std::make_tuple(false, error);
        }

        std::vector<std::string> vendors = {};

        for (const std::string& vendor : reply.vendors()) {
            vendors.push_back(vendor);
        }
        return std::make_tuple(true, vendors);
    }

    // Call to FoodVendor
    // Return bool to signal success or failure.
    // If success, also return ingredient info. If failure, also return error string.
    std::tuple<bool, std::string> GetIngredientInfo(const std::string& ingredient, const std::string& vendorName) {
        VendorRequest request;
        request.set_ingredient(ingredient);
        request.set_vendor_name(vendorName);

        VendorReply reply;
        ClientContext context;

        // Set timeout for server
        context.set_deadline(std::chrono::system_clock::now() +
            std::chrono::milliseconds(kServerTimeout));
 
        absl::Time start = absl::Now();

        Status status = stub_->GetIngredientInfo(&context, request, &reply);

        // Record latency
        absl::Time end = absl::Now();
        double latency = absl::ToDoubleMilliseconds(end - start);
        opencensus::stats::Record({{RPCLatencyMeasure(), latency}});

        // Record RPC call
        opencensus::stats::Record({{RPCCountMeasure(), 1}});

        if (!status.ok()) {
            std::cout << status.error_code() << ": " << status.error_message()
                      << std::endl;

            // Record error for metrics
            opencensus::stats::Record({{RPCErrorCountMeasure(), 1}});

            std::string custom_error_message = "FoodVendor " + status.error_message();

            return std::make_tuple(false, custom_error_message);
        }

        std::string ingredient_info = FormatIngredientInfo(reply.inventory_count(), reply.price());
        return std::make_tuple(true, ingredient_info);
    }

 private:
    std::unique_ptr<InternalFoodService::Stub> stub_;

    std::string FormatIngredientInfo(int inventory_count, float price) {
        std::ostringstream oss;
        oss << inventory_count << " available @ $" << price;
        return oss.str();
    }
};


class FoodFinderService final : public ExternalFoodService::Service {
    const std::string supplier_address_ = "localhost:50051";
    const std::string vendor_address_ = "localhost:50061";
    const absl::string_view endpoint_ = "http://localhost:9411/api/v2/spans";

    Status GetVendorsInfo(ServerContext* context, const FinderRequest* request,
                      FinderReply* reply) override {
        const std::string ingredient = request->ingredient();
        
        FoodFinder supplier_finder(grpc::CreateChannel(
                supplier_address_, grpc::InsecureChannelCredentials()));

        FoodFinder vendor_finder(grpc::CreateChannel(
                vendor_address_, grpc::InsecureChannelCredentials()));

        static opencensus::trace::AlwaysSampler sampler;

        // For metrics
        grpc::RegisterOpenCensusPlugin();
        grpc::RegisterOpenCensusViewsForExport();

        RegisterExporters();

        // Begin FoodFinder span
        opencensus::trace::Span finder_span = opencensus::trace::Span::StartSpan(
            "FoodFinder", /* parent = */ nullptr, {&sampler});
        finder_span.AddAnnotation("Requested ingredient: " + ingredient);

        // Begin FoodSupplier span
        opencensus::trace::Span supplier_span = opencensus::trace::Span::StartSpan(
            "FoodSupplier", &finder_span, {&sampler});

        std::tuple<bool, std::vector<std::string>> supplier_return = supplier_finder.GetVendors(ingredient);
        bool success = std::get<0>(supplier_return);

        // FoodSupplier returned an error
        if (!success) {
            std::string error_message = std::get<1>(supplier_return).at(0);
            supplier_span.AddAnnotation("ERROR: " + error_message);

            supplier_span.End();
            finder_span.End();
            return Status(StatusCode::ABORTED, error_message);
        }

        std::vector<std::string> vendors = std::get<1>(supplier_return);

        int num_vendors = vendors.size();

        if (num_vendors == 0) {
            supplier_span.AddAnnotation("No vendors found");
            supplier_span.End();

            reply->add_vendors_info("None");
            finder_span.End();
            return Status::OK;
        }

        supplier_span.AddAnnotation(std::to_string(num_vendors) + " vendors found");
        supplier_span.End();

        // Begin FoodVendor span
        opencensus::trace::Span vendor_span = opencensus::trace::Span::StartSpan(
                "FoodVendor", &finder_span, {&sampler});

        for (const std::string& vendor : vendors) {
            const std::string span_name = "FoodVendor - " + vendor;
            opencensus::trace::Span curr_vendor_span = opencensus::trace::Span::StartSpan(
                span_name, &vendor_span, {&sampler});

            std::tuple<bool, std::string> vendor_return = vendor_finder.GetIngredientInfo(ingredient, vendor);
            bool success = std::get<0>(vendor_return);

            if (!success) {
                std::string error_message = std::get<1>(vendor_return);
                curr_vendor_span.AddAnnotation("ERROR: " + error_message);

                curr_vendor_span.End();
                vendor_span.End();
                finder_span.End();
                return Status(StatusCode::ABORTED, error_message);
            }

            std::string ingredient_info = std::get<1>(vendor_return);
            std::ostringstream oss;
            oss << vendor << ": " << ingredient_info;
            reply->add_vendors_info(oss.str());

            curr_vendor_span.End();
        }
        vendor_span.End();

        finder_span.End();
        return Status::OK;
    }

 private:
    void RegisterExporters() {
        // Zipkin
        opencensus::exporters::trace::ZipkinExporterOptions options = opencensus::exporters::trace::ZipkinExporterOptions(endpoint_);
        options.service_name = "FoodService";
        opencensus::exporters::trace::ZipkinExporter::Register(options);

        // StackDriver
        const char* project_id = getenv("STACKDRIVER_PROJECT_ID");
        if (project_id == nullptr) {
            std::cerr << "The STACKDRIVER_PROJECT_ID environment variable is not set: "
                        "not exporting to Stackdriver.\n";
        }
        else {
            opencensus::exporters::stats::StackdriverOptions stats_opts;
            stats_opts.project_id = project_id;
            opencensus::exporters::stats::StackdriverExporter::Register(
                std::move(stats_opts));
        }
    }
};


void RegisterViews() {
    RPCErrorCountMeasure();
    opencensus::stats::ViewDescriptor()
        .set_name("FoodService/RPCErrorCount")
        .set_description("Number of RPC errors")
        .set_measure(kRPCErrorMeasureName)
        .set_aggregation(opencensus::stats::Aggregation::Count())
        .add_column(MethodKey())
        .RegisterForExport();

    RPCCountMeasure();
    opencensus::stats::ViewDescriptor()
        .set_name("FoodService/RPCCount")
        .set_description("Number of RPC calls")
        .set_measure(kRPCCountMeasureName)
        .set_aggregation(opencensus::stats::Aggregation::Count())
        .add_column(MethodKey())
        .RegisterForExport();

    RPCLatencyMeasure();
    opencensus::stats::ViewDescriptor()
        .set_name("FoodService/RPCLatency")
        .set_description("Latency of RPC calls")
        .set_measure(kRPCCountMeasureName)
        .set_aggregation(opencensus::stats::Aggregation::Distribution(
          opencensus::stats::BucketBoundaries::Explicit(
              {0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100})))
        .add_column(MethodKey())
        .RegisterForExport();
}


void RunFoodFinder() {
    const std::string server_address = "localhost:50071";
    FoodFinderService service;

    RegisterViews();

    ServerBuilder builder;

    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;

    server->Wait();
}


int main(int argc, char** argv) {
    RunFoodFinder();

    return 0;
}