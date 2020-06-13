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

const std::map<std::string, std::map<std::string, float>> * kInventories;
const std::map<std::string, std::map<std::string, float>> * kPrices;

class FoodVendorService final : public InternalFoodService::Service {

    // Create delay between 0 and 99 milliseconds
    void CreateRandomDelay();

    // Decide whether to throw an error (12.5% chance)
    bool IsCreateRandomError();

    // Called by FoodFinder
    Status GetIngredientInfo(ServerContext* context, const VendorRequest* request,
                             VendorReply* reply) override;
};

void RunFoodVendor();
