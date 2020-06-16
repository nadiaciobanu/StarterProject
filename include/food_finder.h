#include <iostream>
#include <memory>
#include <string>
#include <sstream>
#include <tuple>

#include <grpcpp/grpcpp.h>
#include <grpcpp/opencensus.h>

#include "food.grpc.pb.h"

#include "absl/strings/string_view.h"
#include "absl/time/clock.h"
#include "opencensus/exporters/trace/zipkin/zipkin_exporter.h"
#include "opencensus/exporters/stats/stackdriver/stackdriver_exporter.h"
#include "opencensus/trace/sampler.h"
#include "opencensus/trace/span.h"
#include "opencensus/stats/stats.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using grpc::StatusCode;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;

using food::InternalFoodService;
using food::ExternalFoodService;
using food::SupplierRequest;
using food::SupplierReply;
using food::VendorRequest;
using food::VendorReply;
using food::FinderRequest;
using food::FinderReply;

const std::string kGeneralErrorString = "ERROR";
const int kServerTimeout = 85;

// For metrics
ABSL_CONST_INIT const absl::string_view kRPCErrorMeasureName = "rpc_error_count";
ABSL_CONST_INIT const absl::string_view kRPCCountMeasureName = "rpc_count";
ABSL_CONST_INIT const absl::string_view kRPCLatencyMeasureName = "rpc_latency";

opencensus::stats::MeasureInt64 RPCErrorCountMeasure();
opencensus::stats::MeasureInt64 RPCCountMeasure();
opencensus::stats::MeasureDouble RPCLatencyMeasure();
opencensus::tags::TagKey MethodKey();


class FoodFinder {
 public:
    FoodFinder(std::shared_ptr<Channel> channel)
            : stub_(InternalFoodService::NewStub(channel)) {}

    // Call to FoodSupplier
    // Return bool to signal success or failure.
    // If success, also return list of vendors. If failure, also return error string.
    std::tuple<bool, std::vector<std::string>> GetVendors(const std::string& ingredient);

    // Call to FoodVendor
    // Return bool to signal success or failure.
    // If success, also return ingredient info. If failure, also return error string.
    std::tuple<bool, std::string> GetIngredientInfo(const std::string& ingredient, const std::string& vendorName);

 private:
    std::unique_ptr<InternalFoodService::Stub> stub_;
    std::string FormatIngredientInfo(int inventory_count, float price);
};


class FoodFinderService final : public ExternalFoodService::Service {
    const std::string supplier_address_ = "localhost:50051";
    const std::string vendor_address_ = "localhost:50061";
    const absl::string_view endpoint_ = "http://localhost:9411/api/v2/spans";

    Status GetVendorsInfo(ServerContext* context, const FinderRequest* request,
                          FinderReply* reply) override;

 private:
    void RegisterExporters();
};


void RegisterViews();

void RunFoodFinder();
