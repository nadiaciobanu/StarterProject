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

#ifdef BAZEL_BUILD
#include "food.grpc.pb.h"
#else
#include "food.grpc.pb.h"
#endif

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using food::FoodService;
using food::VendorRequest;
using food::VendorReply;

std::map<std::string, std::string> vendorMap;


// Logic and data behind the server's behavior.
class FoodVendorService final : public FoodService::Service {

    Status GetIngredientInfo(ServerContext* context, const VendorRequest* request,
                             VendorReply* reply) override {
        //std::string ingredient = request->ingredient();

        reply->set_inventorycount(3);
        reply->set_price(10.00);
        return Status::OK;
    }
};


void runFoodVendor() {
    std::string address = "0.0.0.0";
    std::string port = "50061";
    std::string server_address = address + ":" + port;
    FoodVendorService service;

    ServerBuilder builder;
    // Listen on the given address without any authentication mechanism.
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    // Register "service" as the instance through which we'll communicate with
    // clients. In this case it corresponds to an *synchronous* service.
    builder.RegisterService(&service);
    // Finally assemble the server.
    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;

    // Wait for the server to shutdown. Note that some other thread must be
    // responsible for shutting down the server for this call to ever return.
    server->Wait();
}


void initVendors() {
    vendorMap["eggs"] = "Costco";
    vendorMap["milk"] = "Costco, Safeway";
    vendorMap["flour"] = "Safeway, Superstore";
    vendorMap["sugar"] = "Costco, Safeway, Superstore";
}


int main(int argc, char** argv) {
    initVendors();
    runFoodVendor();

    return 0;
}
