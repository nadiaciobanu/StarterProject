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
using food::FoodService;
using food::SupplierRequest;
using food::SupplierReply;
using food::VendorRequest;
using food::VendorReply;

class FoodFinder {
 public:
    FoodFinder(std::shared_ptr<Channel> channel)
            : stub_(FoodService::NewStub(channel)) {}

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
        request.set_vendorname(vendorName);

        VendorReply reply;
        ClientContext context;

        Status status = stub_->GetIngredientInfo(&context, request, &reply);

        if (!status.ok()) {
            std::cout << status.error_code() << ": " << status.error_message()
                      << std::endl;
            return "Information not found";
        }

        return formatIngredientInfo(reply.inventorycount(), reply.price());
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

    const std::string supplier_address = "localhost:50051";
    const std::string vendor_address = "localhost:50061";

    FoodFinder supplierFinder(grpc::CreateChannel(
            supplier_address, grpc::InsecureChannelCredentials()));

    FoodFinder vendorFinder(grpc::CreateChannel(
            vendor_address, grpc::InsecureChannelCredentials()));

    static opencensus::trace::AlwaysSampler sampler;

    // Initialize and enable the Zipkin trace exporter.
    const absl::string_view endpoint = "http://localhost:9411/api/v2/spans";
    opencensus::exporters::trace::ZipkinExporter::Register(
        opencensus::exporters::trace::ZipkinExporterOptions(endpoint));

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

        for (const std::string& vendor : vendors) {
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
