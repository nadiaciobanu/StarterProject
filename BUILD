# Copyright 2017 gRPC authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

licenses(["notice"])  # 3-clause BSD

package(default_visibility = ["//visibility:public"])

load("@rules_proto//proto:defs.bzl", "proto_library")
load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_proto_library")
load("@com_github_grpc_grpc//bazel:cc_grpc_library.bzl", "cc_grpc_library")

# The following three rules demonstrate the usage of the cc_grpc_library rule in
# in a mode compatible with the native proto_library and cc_proto_library rules.
proto_library(
    name = "food_proto",
    srcs = ["food.proto"],
)

cc_proto_library(
    name = "food_cc_proto",
    deps = [":food_proto"],
)

cc_grpc_library(
    name = "food_cc_grpc",
    srcs = [":food_proto"],
    grpc_only = True,
    deps = [":food_cc_proto"],
)

cc_binary(
    name = "FoodFinder",
    srcs = ["FoodFinder.cc"],
    defines = ["BAZEL_BUILD"],
    deps = [
        ":food_cc_grpc",
        # http_archive made this label available for binding
        "@com_github_grpc_grpc//:grpc++",
        # For OpenCensus
        "@io_opencensus_cpp//opencensus/trace",
        "@io_opencensus_cpp//opencensus/exporters/trace/zipkin:zipkin_exporter",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/strings",
],
)

cc_binary(
    name = "FoodSupplier",
    srcs = ["FoodSupplier.cc"],
    defines = ["BAZEL_BUILD"],
    deps = [
        ":food_cc_grpc",
        # http_archive made this label available for binding
        "@com_github_grpc_grpc//:grpc++",
    ],
)

cc_binary(
    name = "FoodVendor",
    srcs = ["FoodVendor.cc"],
    defines = ["BAZEL_BUILD"],
    deps = [
        ":food_cc_grpc",
        # http_archive made this label available for binding
        "@com_github_grpc_grpc//:grpc++",
    ],
)

cc_binary(
    name = "FoodClient",
    srcs = ["FoodClient.cc"],
    defines = ["BAZEL_BUILD"],
    deps = [
        ":food_cc_grpc",
        # http_archive made this label available for binding
        "@com_github_grpc_grpc//:grpc++",
    ],
)