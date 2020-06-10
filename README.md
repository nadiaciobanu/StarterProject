# Cloud Trace Starter Project - Baking Supplies Finder

The FoodFinder API can be called with baking ingredients and will return vendors that sell the ingredients, along with their inventories and prices.

FoodFinder makes underlying calls to the FoodSupplier API to find out which vendors sell the ingredient, then to the FoodVendor API to find out the inventory and price of the ingredient for each vendor.

To build:
```
bazel build :food_supplier
bazel build :food_vendor
bazel build :food_finder
bazel build :food_client
```

To run:
```
./bazel-bin/food_supplier
./bazel-bin/food_vendor
./bazel-bin/food_finder
./bazel-bin/food_client
```

Make sure `FoodSupplier`, `FoodVendor` and `FoodFinder` are running before attempting to run `FoodFinder`.

To view traces, install [Zipkin](https://zipkin.io/pages/quickstart) and visit http://localhost:9411/zipkin
