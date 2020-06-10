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

To view traces, install [Zipkin](https://zipkin.io/pages/quickstart) and visit http://localhost:9411/zipkin
