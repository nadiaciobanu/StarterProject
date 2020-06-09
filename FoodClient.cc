#include <iostream>
#include <string>

#include <grpcpp/grpcpp.h>

#include "food.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using food::ExternalFoodService;
using food::FinderRequest;
using food::FinderReply;

class FoodClient {
 public:
    FoodClient(std::shared_ptr<Channel> channel)
            : stub_(ExternalFoodService::NewStub(channel)) {}
    
    std::vector<std::string> GetVendorsInfo(const std::string& ingredient) {
        // Data we are sending to the server.
        FinderRequest request;
        request.set_ingredient(ingredient);

        FinderReply reply;
        ClientContext context;

        Status status = stub_->GetVendorsInfo(&context, request, &reply);

        if (status.ok()) {
            std::vector<std::string> vendorsInfo = {};

            if (reply.vendorsinfo_size() > 0) {
                for (std::string vendor : reply.vendorsinfo()) {
                    vendorsInfo.push_back(vendor);
                }
            }
            return vendorsInfo;
        }
        else {
            std::cout << status.error_code() << ": " << status.error_message()
                      << std::endl;
            return {};
        }
    } 

 private:
    std::unique_ptr<ExternalFoodService::Stub> stub_;
};

int main(int argc, char** argv) {

    std::cout << std::endl << "Welcome to FoodFinder!" << std::endl;

    while (true) {
        std::cout << std::endl << "Please input the ingredient you would like to find: ";

        std::string inputIngredient;
        std::cin >> inputIngredient;

        std::cout << "Searching for vendors for " << inputIngredient << "..." <<std::endl;

        std::string finder_address = "localhost:50071";

        FoodClient finderClient(grpc::CreateChannel(
                finder_address, grpc::InsecureChannelCredentials()));

        std::vector<std::string> vendorsWithInfo = finderClient.GetVendorsInfo(inputIngredient);

        for (std::string vendorInfo : vendorsWithInfo) {
            std::cout << "- " << vendorInfo << std::endl;
        }
    }
    return 0;
}