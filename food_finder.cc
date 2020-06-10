/*
 *
 * Copyright 2015 gRPC authors.
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

#include <grpcpp/grpcpp.h>

#include "food.grpc.pb.h"

#include "absl/strings/string_view.h"
#include "opencensus/exporters/trace/zipkin/zipkin_exporter.h"
#include "opencensus/trace/sampler.h"
#include "opencensus/trace/span.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
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


class FoodFinder {
 public:
    FoodFinder(std::shared_ptr<Channel> channel)
            : stub_(InternalFoodService::NewStub(channel)) {}

    // Call to FoodSupplier
    std::vector<std::string> GetVendors(const std::string& ingredient) {
        SupplierRequest request;
        request.set_ingredient(ingredient);

        SupplierReply reply;
        ClientContext context;

        Status status = stub_->GetVendors(&context, request, &reply);

        if (!status.ok()) {
            std::cout << status.error_code() << ": " << status.error_message()
                      << std::endl;
            return {};
        }

        std::vector<std::string> vendors = {};

        for (const std::string& vendor : reply.vendors()) {
            vendors.push_back(vendor);
        }
        return vendors;   
    }

    // Call to FoodVendor
    std::string GetIngredientInfo(const std::string& ingredient, const std::string& vendorName) {
        VendorRequest request;
        request.set_ingredient(ingredient);
        request.set_vendor_name(vendorName);

        VendorReply reply;
        ClientContext context;

        Status status = stub_->GetIngredientInfo(&context, request, &reply);

        if (!status.ok()) {
            std::cout << status.error_code() << ": " << status.error_message()
                      << std::endl;
            return "Information not found";
        }

        return FormatIngredientInfo(reply.inventory_count(), reply.price());
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
    const std::string supplier_address = "localhost:50051";
    const std::string vendor_address = "localhost:50061";
    const absl::string_view endpoint = "http://localhost:9411/api/v2/spans";

    Status GetVendorsInfo(ServerContext* context, const FinderRequest* request,
                      FinderReply* reply) override {
        const std::string ingredient = request->ingredient();
        
        FoodFinder supplier_finder(grpc::CreateChannel(
                supplier_address, grpc::InsecureChannelCredentials()));

        FoodFinder vendor_finder(grpc::CreateChannel(
                vendor_address, grpc::InsecureChannelCredentials()));

        static opencensus::trace::AlwaysSampler sampler;

        // Enable the Zipkin trace exporter
        opencensus::exporters::trace::ZipkinExporter::Register(
            opencensus::exporters::trace::ZipkinExporterOptions(endpoint));

        // Begin FoodFinder span
        opencensus::trace::Span finder_span = opencensus::trace::Span::StartSpan(
            "FoodFinder", /* parent = */ nullptr, {&sampler});
        finder_span.AddAnnotation("Requested ingredient: " + ingredient);

        // Begin FoodSupplier span
        opencensus::trace::Span supplier_span = opencensus::trace::Span::StartSpan(
            "FoodSupplier", &finder_span, {&sampler});

        std::vector<std::string> vendors = supplier_finder.GetVendors(ingredient);

        int num_vendors = vendors.size();
        supplier_span.AddAnnotation(std::to_string(num_vendors) + " vendors found");

        supplier_span.End();

        if (vendors.size() == 0) {
            reply->add_vendors_info("None");
            finder_span.End();
            return Status::OK;
        }

        // Begin FoodVendor span
        opencensus::trace::Span vendor_span = opencensus::trace::Span::StartSpan(
                "FoodVendor", &finder_span, {&sampler});

        for (const std::string& vendor : vendors) {
            const std::string span_name = "FoodVendor - " + vendor;
            opencensus::trace::Span curr_vendor_span = opencensus::trace::Span::StartSpan(
                span_name, &vendor_span, {&sampler});

            std::string ingredient_info = vendor_finder.GetIngredientInfo(ingredient, vendor);
            std::ostringstream oss;
            oss << vendor << ": " << ingredient_info;

            reply->add_vendors_info(oss.str());

            curr_vendor_span.End();
        }
        vendor_span.End();

        finder_span.End();
        return Status::OK;
    }
};


void RunFoodFinder() {
    const std::string server_address = "0.0.0.0:50071";
    FoodFinderService service;

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