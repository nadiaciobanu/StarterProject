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

#ifdef BAZEL_BUILD
#include "food.grpc.pb.h"
#else
#include "food.grpc.pb.h"
#endif

#include "absl/strings/string_view.h"
#include "opencensus/exporters/trace/zipkin/zipkin_exporter.h"
#include "opencensus/trace/sampler.h"
#include "opencensus/trace/span.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using food::FoodService;
using food::SupplierRequest;
using food::SupplierReply;
using food::VendorRequest;
using food::VendorReply;

class FoodFinder {
 public:
    FoodFinder(std::shared_ptr<Channel> channel)
            : stub_(FoodService::NewStub(channel)) {}

    // Assembles the client's payload, sends it and presents the response back
    // from the server.
    std::vector<std::string> GetVendors(const std::string& ingredient) {
        // Data we are sending to the server.
        SupplierRequest request;
        request.set_ingredient(ingredient);

        // Container for the data we expect from the server.
        SupplierReply reply;

        // Context for the client. It could be used to convey extra information to
        // the server and/or tweak certain RPC behaviors.
        ClientContext context;

        // The actual RPC.
        Status status = stub_->GetVendors(&context, request, &reply);

        // Act upon its status.
        if (status.ok()) {
            std::vector<std::string> vendors = {};

            if (reply.vendors_size() > 0) {
                for (std::string vendor : reply.vendors()) {
                    vendors.push_back(vendor);
                }
            }
            return vendors;
        }
        else {
            std::cout << status.error_code() << ": " << status.error_message()
                      << std::endl;
            return {};
        }
    }

    std::string GetIngredientInfo(const std::string& ingredient, const std::string& vendorName) {
        // Data we are sending to the server.
        VendorRequest request;
        request.set_ingredient(ingredient);
        request.set_vendorname(vendorName);

        // Container for the data we expect from the server.
        VendorReply reply;

        // Context for the client. It could be used to convey extra information to
        // the server and/or tweak certain RPC behaviors.
        ClientContext context;

        // The actual RPC.
        Status status = stub_->GetIngredientInfo(&context, request, &reply);

        // Act upon its status.
        if (status.ok()) {
            return formatIngredientInfo(reply.inventorycount(), reply.price());
        }
        else {
            std::cout << status.error_code() << ": " << status.error_message()
                      << std::endl;
            return "Information not found";
        }
    }


 private:
    std::unique_ptr<FoodService::Stub> stub_;

    std::string formatIngredientInfo(int invCount, float price) {
        std::ostringstream oss;
        oss << invCount << " available @ $" << price;
        return oss.str();
    }
};


void runFoodFinder() {
    std::cout << std::endl << "Welcome to FoodFinder!" << std::endl;

    std::string supplier_address = "localhost:50051";
    std::string vendor_address = "localhost:50061";

    FoodFinder supplierFinder(grpc::CreateChannel(
            supplier_address, grpc::InsecureChannelCredentials()));

    FoodFinder vendorFinder(grpc::CreateChannel(
            vendor_address, grpc::InsecureChannelCredentials()));

    static opencensus::trace::AlwaysSampler sampler;

    // ZIPKIN - Uncomment to use

    // Initialize and enable the Zipkin trace exporter.
    //const absl::string_view endpoint = "http://localhost:9411/api/v2/spans";
    //opencensus::exporters::trace::ZipkinExporter::Register(
        //opencensus::exporters::trace::ZipkinExporterOptions(endpoint));

    while (true) {
        std::cout << std::endl << "Please input the ingredient you would like to find: ";

        std::string inputIngredient;
        std::cin >> inputIngredient;

        std::cout << "Searching for vendors for " << inputIngredient << "..." <<std::endl;

        // Trace call to FoodSupplier
        opencensus::trace::Span supplierSpan = opencensus::trace::Span::StartSpan(
            "FoodSupplier", /* parent = */ nullptr, {&sampler});

        std::vector<std::string> vendors = supplierFinder.GetVendors(inputIngredient);

        supplierSpan.End();

        if (vendors.size() == 0) {
            std::cout << "Vendors found: None" << std::endl;
            continue;
        }

        for (std::string vendor : vendors) {
            // Trace call to FoodVendor
            opencensus::trace::Span vendorSpan = opencensus::trace::Span::StartSpan(
                "FoodVendor", /* parent = */ nullptr, {&sampler});

            std::string ingredientInfo = vendorFinder.GetIngredientInfo(inputIngredient, vendor);
            std::cout << "- " << vendor << ": " << ingredientInfo << std::endl;

            vendorSpan.End();
        }
    }
}


int main(int argc, char** argv) {
    runFoodFinder();

    return 0;
}
