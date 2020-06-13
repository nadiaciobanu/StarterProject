#include "food_vendor.h"


// Create delay between 0 and 99 milliseconds
void FoodVendorService::CreateRandomDelay() {
    srand(time(0));
    int delay = rand() % 100;

    std::this_thread::sleep_for(std::chrono::milliseconds(delay));
}


// Decide whether to throw an error (12.5% chance)
bool FoodVendorService::IsCreateRandomError() {
    srand(time(0));
    int random_number = rand() % 8;
    if (random_number == 0) {
        return true;
    }
    return false;
}


// Called by FoodFinder
Status FoodVendorService::GetIngredientInfo(ServerContext* context, const VendorRequest* request,
                            VendorReply* reply) {
    CreateRandomDelay();

    if (IsCreateRandomError()) {
        return Status(StatusCode::ABORTED, "Random Error");
    }

    const std::string vendor = request->vendor_name();
    const std::string ingredient = request->ingredient();

    const std::map<std::string, float> vendor_inventory = kInventories->at(vendor);
    const std::map<std::string, float> vendor_prices = kPrices->at(vendor);

    const int ingredient_inventory = vendor_inventory.at(ingredient);
    const float ingredient_price = vendor_prices.at(ingredient);

    reply->set_inventory_count(ingredient_inventory);
    reply->set_price(ingredient_price);
    return Status::OK;
}


void RunFoodVendor() {
    const std::string server_address = "localhost:50061";

    kInventories = new std::map<std::string, std::map<std::string, float>>(
        {
            {"Costco", {{"eggs", 10}, {"milk", 45}, {"sugar", 24}}},
            {"Safeway", {{"milk", 65}, {"sugar", 20}, {"flour", 58}}},
            {"Superstore", {{"flour", 4}, {"sugar", 18}}}
        });

    kPrices = new std::map<std::string, std::map<std::string, float>>(
        {
            {"Costco", {{"eggs", 1.00}, {"milk", 2.57}, {"sugar", 4.00}}},
            {"Safeway", {{"milk", 3.50}, {"sugar", 3.00}, {"flour", 5.45}}},
            {"Superstore", {{"flour", 2.00}, {"sugar", 3.35}}}
        });

    FoodVendorService service;
    ServerBuilder builder;

    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;

    server->Wait();

    delete kInventories;
    delete kPrices;
}


int main(int argc, char** argv) {
    RunFoodVendor();

    return 0;
}
