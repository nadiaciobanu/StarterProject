# Cloud Trace Starter Project - Baking Supplies Finder

The FoodFinder API can be called with baking ingredients and will return vendors that sell the ingredients!

FoodFinder makes underlying calls to the FoodSupplier API to find out which vendors sell the ingredient, then to the FoodVendor API to find out the inventory and price of the ingredient for each vendor.

To build:
```
bazel build :FoodSupplier
bazel build :FoodFinder
bazel build :FoodVendor
```

To run:
```
./bazel-bin/FoodSupplier
./bazel-bin/FoodVendor
./bazel-bin/FoodFinder
```

Make sure `FoodSupplier` and `FoodVendor` are running before attempting to run `FoodFinder`.

Some code was inspired by https://github.com/arcticmatt/grpc_getting_started
