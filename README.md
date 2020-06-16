# Cloud Trace Starter Project - Baking Supplies Finder

The FoodFinder API can be called with baking ingredients and will return vendors that sell the ingredients, along with their inventories and prices.

FoodFinder makes underlying calls to the FoodSupplier API to find out which vendors sell the ingredient, then to the FoodVendor API to find out the inventory and price of the ingredient for each vendor.

A FoodFinder client, called FoodClient, is also provided and can be used to make calls to the FoodFinder API.

## Build
To build the program, run:
```
bazel build :all
```

## Run
Run the services in seperate terminal windows:
```
./bazel-bin/food_supplier
./bazel-bin/food_vendor
./bazel-bin/food_finder
```

Then run the client:
```
./bazel-bin/food_client
```

Make sure `FoodSupplier`, `FoodVendor` and `FoodFinder` are running before attempting to run `FoodClient`.

## Telemetry

This project has been instrumented using [OpenCensus](https://opencensus.io/). You can export traces and metrics produced by interactions between the food services. Currently, the project supports exporting traces to Zipkin or GCP, and metrics to GCP.

### Traces in Zipkin

To view traces in Zipkin, install [Zipkin](https://zipkin.io/pages/quickstart) and visit http://localhost:9411/zipkin.

![Zipkin](https://user-images.githubusercontent.com/14475923/84815064-4dfcfe80-afc7-11ea-97fa-53e4feefdec3.png)

### Traces in Google Cloud Platform

You can also export traces from Zipkin to GCP by creating a GCP project and following these [instructions](https://cloud.google.com/trace/docs/zipkin).

![Trace_GCP](https://user-images.githubusercontent.com/14475923/84826762-4abe3e80-afd8-11ea-820a-6ee4e43940aa.png)

### Metrics in Google Cloud Platform

You can export metrics to GCP using [Stackdriver](https://cloud.google.com/products/operations). Just set your `STACKDRIVER_PROJECT_ID` environment variable to the name of your GCP project.

![Metrics](https://user-images.githubusercontent.com/14475923/84816082-d039f280-afc8-11ea-876f-9e96644ea1fd.png)
