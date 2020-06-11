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
using grpc::StatusCode;
using food::InternalFoodService;
using food::VendorRequest;
using food::VendorReply;


const std::map<std::string, std::map<std::string, float>> inventories = \
    {
        {"Costco", {{"eggs", 10}, {"milk", 45}, {"sugar", 24}}},
        {"Safeway", {{"milk", 65}, {"sugar", 20}, {"flour", 58}}},
        {"Superstore", {{"flour", 4}, {"sugar", 18}}}
    };

const std::map<std::string, std::map<std::string, float>> prices = \
    {
        {"Costco", {{"eggs", 1.00}, {"milk", 2.57}, {"sugar", 4.00}}},
        {"Safeway", {{"milk", 3.50}, {"sugar", 3.00}, {"flour", 5.45}}},
        {"Superstore", {{"flour", 2.00}, {"sugar", 3.35}}}
    };


class FoodVendorService final : public InternalFoodService::Service {

    // Create delay between 0 and 99 milliseconds
    void CreateRandomDelay() {
        srand(time(0));
        int delay = rand() % 100;

        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
    }

    // Decide whether to throw an error (25% chance)
    bool IsCreateRandomError() {
        srand(time(0));
        int random_number = rand() % 4;
        if (random_number == 0) {
            return true;
        }
        return false;
    }

    // Called by FoodFinder
    Status GetIngredientInfo(ServerContext* context, const VendorRequest* request,
                             VendorReply* reply) override {
        CreateRandomDelay();

        if (IsCreateRandomError()) {
            return Status(StatusCode::ABORTED, "Random Error");
        }

        const std::string vendor = request->vendor_name();
        const std::string ingredient = request->ingredient();

        const std::map<std::string, float> vendor_inventory = inventories.at(vendor);
        const std::map<std::string, float> vendor_prices = prices.at(vendor);

        const int ingredient_inventory = vendor_inventory.at(ingredient);
        const float ingredient_price = vendor_prices.at(ingredient);

        reply->set_inventory_count(ingredient_inventory);
        reply->set_price(ingredient_price);
        return Status::OK;
    }
};


void RunFoodVendor() {
    const std::string server_address = "localhost:50061";
    FoodVendorService service;

    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;

    server->Wait();
}


int main(int argc, char** argv) {
    RunFoodVendor();

    return 0;
}
