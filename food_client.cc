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
    std::tuple<bool, std::vector<std::string>> GetVendorsInfo(const std::string& ingredient) {
        FinderRequest request;
        request.set_ingredient(ingredient);

        FinderReply reply;
        ClientContext context;

        Status status = stub_->GetVendorsInfo(&context, request, &reply);

        if (!status.ok()) {
            std::vector<std::string> error = {status.error_message()};
            return std::make_tuple(false, error);
        }

        std::vector<std::string> vendors_info = {};

        for (const std::string& vendor : reply.vendors_info()) {
            vendors_info.push_back(vendor);
        }

        return std::make_tuple(true, vendors_info);
    } 

 private:
    std::unique_ptr<ExternalFoodService::Stub> stub_;
};


std::string GetUserInput() {
    std::cout << std::endl << kUserInputPrompt;

    std::string input_ingredient;
    std::cin >> input_ingredient;

    std::cout << "Searching for vendors for " << input_ingredient << "..." <<std::endl;
    return input_ingredient;
}


void PrintResults(std::vector<std::string> vendors_with_info) {
    for (const std::string& vendor_info : vendors_with_info) {
        std::cout << "- " << vendor_info << std::endl;
    }
}


int main(int argc, char** argv) {
    std::cout << std::endl << kUserWelcomeMessage << std::endl;
    const std::string finder_address = "localhost:50071";

    while (true) {
        std::string input_ingredient = GetUserInput();

        FoodClient finder_client(grpc::CreateChannel(
                finder_address, grpc::InsecureChannelCredentials()));

        std::tuple<bool, std::vector<std::string>> finder_return = finder_client.GetVendorsInfo(input_ingredient);
        bool success = std::get<0>(finder_return);

        if (success) {
            std::vector<std::string> vendors_with_info = std::get<1>(finder_return);
            PrintResults(vendors_with_info);
        }
        else {
            std::string error_message = std::get<1>(finder_return).at(0);
            std::cout << "ERROR: " << error_message << std::endl;
        }
    }
    return 0;
}