# Cloud Trace Starter Project - Baking Supplies Finder

The FoodFinder API can be called with baking ingredients and will return vendors that sell the ingredients!

FoodFinder makes underlying calls to the FoodSupplier API to find out which vendors sell the ingredient, then returns the list of vendors to the user.

To build:
```
bazel build :FoodSupplier
bazel build :FoodFinder
```

To run:
```
bazel run :FoodSupplier
bazel run :FoodFinder
```

Some code was inspired by https://github.com/arcticmatt/grpc_getting_started
