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
using food::SupplierRequest;
using food::SupplierReply;


const std::map<std::string, std::vector<std::string>> * kVendorMap;


class FoodSupplierService final : public InternalFoodService::Service {
    // Create delay between 0 and 99 milliseconds
    void CreateRandomDelay();

    // Decide whether to throw an error (12.5% chance)
    bool IsCreateRandomError();

    // Called by FoodFinder
    Status GetVendors(ServerContext* context, const SupplierRequest* request,
                      SupplierReply* reply) override;
};