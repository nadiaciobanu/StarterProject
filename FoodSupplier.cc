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

#include "food.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using food::InternalFoodService;
using food::SupplierRequest;
using food::SupplierReply;


const std::map<std::string, std::vector<std::string>> vendorMap = \
    {
        {"eggs", {"Costco"}},
        {"milk", {"Costco", "Safeway"}},
        {"flour", {"Safeway", "Superstore"}},
        {"sugar", {"Costco", "Safeway", "Superstore"}}
    };


class FoodSupplierService final : public InternalFoodService::Service {

    void CreateRandomDelay() {
        srand(time(0));
        int delay = rand() % 100;

        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
    }

    // Called by FoodFinder
    Status GetVendors(ServerContext* context, const SupplierRequest* request,
                      SupplierReply* reply) override {
        // Random delay
        CreateRandomDelay();

        const std::string ingredient = request->ingredient();

        if (vendorMap.find(ingredient) != vendorMap.end()) {
            const std::vector<std::string> vendors = vendorMap.at(ingredient);

            for (const std::string& vendor : vendors) {
                reply->add_vendors(vendor);
            }
        }
        return Status::OK;
    }
};


void runFoodSupplier() {
    std::string server_address = "0.0.0.0:50051";
    FoodSupplierService service;

    ServerBuilder builder;

    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;

    server->Wait();
}


int main(int argc, char** argv) {
    runFoodSupplier();

    return 0;
}
