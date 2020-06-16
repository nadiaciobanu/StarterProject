#include <iostream>
#include <string>
#include <tuple>

#include <grpcpp/grpcpp.h>

#include "food.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using food::ExternalFoodService;
using food::FinderRequest;
using food::FinderReply;

const std::string kGeneralErrorString = "ERROR";
const std::string kUserWelcomeMessage = "Welcome to FoodFinder!";
const std::string kUserInputPrompt = "Please input the ingredient you would like to find: ";

class FoodClient {
 public:
    FoodClient(std::shared_ptr<Channel> channel)
            : stub_(ExternalFoodService::NewStub(channel)) {}
    
    // Call to FoodFinder
    // Return bool to signal success or failure.
    // If success, also return list of vendors with vendor information.
    // If failure, also return error string.
    std::tuple<bool, std::vector<std::string>> GetVendorsInfo(const std::string& ingredient);

 private:
    std::unique_ptr<ExternalFoodService::Stub> stub_;
};
