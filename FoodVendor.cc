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

#include <grpcpp/grpcpp.h>

#include "food.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using food::FoodService;
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


class FoodVendorService final : public FoodService::Service {

    // Called by FoodFinder
    Status GetIngredientInfo(ServerContext* context, const VendorRequest* request,
                             VendorReply* reply) override {
        const std::string vendor = request->vendorname();
        const std::string ingredient = request->ingredient();

        const std::map<std::string, float> vendorInventory = inventories.at(vendor);
        const std::map<std::string, float> vendorPrices = prices.at(vendor);

        const int ingredientInventory = vendorInventory.at(ingredient);
        const float ingredientPrice = vendorPrices.at(ingredient);

        reply->set_inventorycount(ingredientInventory);
        reply->set_price(ingredientPrice);
        return Status::OK;
    }
};


void runFoodVendor() {
    const std::string server_address = "0.0.0.0:50061";
    FoodVendorService service;

    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;

    server->Wait();
}


int main(int argc, char** argv) {
    runFoodVendor();

    return 0;
}
