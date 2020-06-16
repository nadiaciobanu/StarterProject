#include <iostream>
#include <memory>
#include <string>
#include <map>

#include <grpcpp/grpcpp.h>

#include "food.grpc.pb.h"
#include "food_utils.h"

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
    // Called by FoodFinder
    Status GetIngredientInfo(ServerContext* context, const VendorRequest* request,
                             VendorReply* reply) override;

 private:
    const int kMaxRandomDelay_ = 100;
    const int kRandomErrorChanceDenom_ = 8;
};

void RunFoodVendor();
