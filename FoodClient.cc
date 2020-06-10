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

        if (!status.ok()) {
            std::cout << status.error_code() << ": " << status.error_message()
                      << std::endl;
            return {};
        }

        std::vector<std::string> vendors_info = {};

        for (const std::string& vendor : reply.vendors_info()) {
            vendors_info.push_back(vendor);
        }

        return vendors_info;
    } 

 private:
    std::unique_ptr<ExternalFoodService::Stub> stub_;
};


int main(int argc, char** argv) {
    std::cout << std::endl << "Welcome to FoodFinder!" << std::endl;

    while (true) {
        std::cout << std::endl << "Please input the ingredient you would like to find: ";

        std::string input_ingredient;
        std::cin >> input_ingredient;

        std::cout << "Searching for vendors for " << input_ingredient << "..." <<std::endl;

        const std::string finder_address = "localhost:50071";

        FoodClient finder_client(grpc::CreateChannel(
                finder_address, grpc::InsecureChannelCredentials()));

        std::vector<std::string> vendors_with_info = finder_client.GetVendorsInfo(input_ingredient);

        for (std::string vendor_info : vendors_with_info) {
            std::cout << "- " << vendor_info << std::endl;
        }
    }
    return 0;
}