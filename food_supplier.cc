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
#include <map>
#include <chrono>
#include <thread>
#include <ctime>
#include <cstdlib>

#include <grpcpp/grpcpp.h>
#include <grpcpp/opencensus.h>

#include "food.grpc.pb.h"
#include "opencensus/trace/context_util.h"
#include "opencensus/trace/span.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using grpc::StatusCode;
using food::InternalFoodService;
using food::SupplierRequest;
using food::SupplierReply;


const std::map<std::string, std::vector<std::string>> vendor_map = \
    {
        {"eggs", {"Costco"}},
        {"milk", {"Costco", "Safeway"}},
        {"flour", {"Safeway", "Superstore"}},
        {"sugar", {"Costco", "Safeway", "Superstore"}}
    };


class FoodSupplierService final : public InternalFoodService::Service {

    // Create delay between 0 and 99 milliseconds
    void CreateRandomDelay() {
        srand(time(0));
        int delay = rand() % 100;

        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
    }

    // Decide whether to throw an error (20% chance)
    bool IsCreateRandomError() {
        srand(time(0));
        int random_number = rand() % 5;
        if (random_number == 0) {
            return true;
        }
        return false;
    }

    // Called by FoodFinder
    Status GetVendors(ServerContext* context, const SupplierRequest* request,
                      SupplierReply* reply) override {
        opencensus::trace::Span span = grpc::GetSpanFromServerContext(context);
        span.AddAttribute("my_attribute", "red");

        CreateRandomDelay();

        if (IsCreateRandomError()) {
            return Status(StatusCode::ABORTED, "Random Error");
        }

        const std::string ingredient = request->ingredient();

        if (vendor_map.find(ingredient) != vendor_map.end()) {
            std::vector<std::string> vendors = vendor_map.at(ingredient);

            for (const std::string& vendor : vendors) {
                reply->add_vendors(vendor);
            }
        }
        return Status::OK;
    }
};


void RunFoodSupplier() {
    const std::string server_address = "localhost:50051";
    FoodSupplierService service;

    ServerBuilder builder;

    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;

    server->Wait();
}


int main(int argc, char** argv) {
    RunFoodSupplier();

    return 0;
}
