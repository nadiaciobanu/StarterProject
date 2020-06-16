#include "food_supplier.h"


// Called by FoodFinder
Status FoodSupplierService::GetVendors(ServerContext* context, const SupplierRequest* request,
                    SupplierReply* reply) {
    CreateRandomDelay(kMaxRandomDelay_);

    if (IsCreateRandomError(kRandomErrorChanceDenom_)) {
        return Status(StatusCode::ABORTED, "Random Error");
    }

    const std::string ingredient = request->ingredient();

    if (kVendorMap->find(ingredient) != kVendorMap->end()) {
        std::vector<std::string> vendors = kVendorMap->at(ingredient);

        for (const std::string& vendor : vendors) {
            reply->add_vendors(vendor);
        }
    }
    return Status::OK;
}


void RunFoodSupplier() {
    const std::string server_address = "localhost:50051";

    kVendorMap = new std::map<std::string, std::vector<std::string>>(
        {
            {"eggs", {"Costco"}},
            {"milk", {"Costco", "Safeway"}},
            {"flour", {"Safeway", "Superstore"}},
            {"sugar", {"Costco", "Safeway", "Superstore"}}
        });

    FoodSupplierService service;
    ServerBuilder builder;

    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;

    server->Wait();

    delete kVendorMap;
}


int main(int argc, char** argv) {
    RunFoodSupplier();

    return 0;
}
