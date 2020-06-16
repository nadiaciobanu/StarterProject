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
using food::SupplierRequest;
using food::SupplierReply;

const std::map<std::string, std::vector<std::string>> * kVendorMap;

class FoodSupplierService final : public InternalFoodService::Service {
    // Called by FoodFinder
    Status GetVendors(ServerContext* context, const SupplierRequest* request,
                      SupplierReply* reply) override;

 private:
    const int kMaxRandomDelay_ = 100;
    const int kRandomErrorChanceDenom_ = 8;
};

void RunFoodSupplier();
