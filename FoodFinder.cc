/*
 *
 * Copyright 2015 gRPC authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
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

#include <grpcpp/grpcpp.h>

//#ifdef BAZEL_BUILD
#include "food.grpc.pb.h"
//#else
//#include "food.grpc.pb.h"
//#endif

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using food::FoodService;
using food::FoodRequest;
using food::FoodReply;

class FoodFinder {
 public:
  FoodFinder(std::shared_ptr<Channel> channel)
      : stub_(FoodService::NewStub(channel)) {}

  // Assembles the client's payload, sends it and presents the response back
  // from the server.
  std::string GetVendors(const std::string& ingredient) {
    // Data we are sending to the server.
    FoodRequest request;
    request.set_ingredient(ingredient);

    // Container for the data we expect from the server.
    FoodReply reply;

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // The actual RPC.
    Status status = stub_->GetVendors(&context, request, &reply);

    // Act upon its status.
    if (status.ok()) {
      return reply.vendors();
    }
    else {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      return "RPC failed";
    }
  }

 private:
  std::unique_ptr<FoodService::Stub> stub_;
};

int main(int argc, char** argv) {
  std::cout << std::endl << "Welcome to FoodFinder!" << std::endl;
  std::string inputIngredient = "";

  while (true) {
    std::cout << std::endl << "Please input the ingredient you would like to find (X to quit): ";

    std::string inputIngredient;
    std::cin >> inputIngredient;

    if (inputIngredient.compare("X") == 0) {
      break;
    }

    std::cout << "Searching for vendors for " << inputIngredient << "..." <<std::endl;

    std::string address = "localhost";
    std::string port = "50051";
    std::string server_address = address + ":" + port;
    //std::cout << "Client querying server address: " << server_address << std::endl;


    // Instantiate the client. It requires a channel, out of which the actual RPCs
    // are created. This channel models a connection to an endpoint (in this case,
    // localhost at port 50051). We indicate that the channel isn't authenticated
    // (use of InsecureChannelCredentials()).
    FoodFinder finder(grpc::CreateChannel(
        server_address, grpc::InsecureChannelCredentials()));

    std::string reply = finder.GetVendors(inputIngredient);
    std::cout << "Vendors found: " << reply << std::endl;
  }

  return 0;
}
