// Copyright 2020 The XLS Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Command line tool for sending "device RPCs" to XLS-generated device
// functions.
//
// TODO(leary): 2019-04-07 Probably want a way to select the desired output
// format; e.g. -hex, -dec, -bin and so on.

#include "absl/flags/flag.h"
#include "absl/status/statusor.h"
#include "xls/common/init_xls.h"
#include "xls/common/logging/logging.h"
#include "xls/ir/ir_parser.h"
#include "xls/tools/device_rpc_strategy.h"
#include "xls/tools/device_rpc_strategy_factory.h"

ABSL_FLAG(std::string, target_device, "",
          "Target (category of) device for DRPC targeting; e.g. ice40.");
ABSL_FLAG(int64_t, device_ordinal, 0,
          "Device ordinal within the -target_device category, useful when "
          "multiple are present.");
ABSL_FLAG(std::string, function_type, "", "Function type being invoked.");

namespace xls {
namespace tools {
namespace {

void RealMain(absl::Span<const std::string_view> args) {
  std::string target_device = absl::GetFlag(FLAGS_target_device);
  XLS_QCHECK(!target_device.empty()) << "Must provide -target_device";

  std::string function_type_string = absl::GetFlag(FLAGS_function_type);
  XLS_QCHECK(!function_type_string.empty()) << "Must provide -function_type";

  Package package("dummy_to_hold_types");
  auto function_type_status =
      Parser::ParseFunctionType(function_type_string, &package);
  XLS_QCHECK_OK(function_type_status.status());
  FunctionType* function_type = function_type_status.value();

  std::vector<Value> arguments;
  for (int64_t i = 0; i < args.size(); ++i) {
    std::string_view s = args[i];
    absl::StatusOr<Value> argument =
        Parser::ParseValue(s, function_type->parameter_type(i));
    XLS_QCHECK_OK(argument.status());
    arguments.push_back(std::move(argument).value());
  }

  absl::StatusOr<std::unique_ptr<DeviceRpcStrategy>> drpc_status =
      DeviceRpcStrategyFactory::GetSingleton()->Create(target_device);
  XLS_QCHECK_OK(drpc_status.status());

  std::unique_ptr<DeviceRpcStrategy> drpc = std::move(drpc_status).value();
  XLS_QCHECK_OK(drpc->Connect(absl::GetFlag(FLAGS_device_ordinal)));

  absl::StatusOr<Value> rpc_status =
      drpc->CallUnnamed(*function_type, arguments);
  XLS_QCHECK_OK(rpc_status.status());

  std::cout << rpc_status.value().ToString(FormatPreference::kHex) << std::endl;
}

}  // namespace
}  // namespace tools
}  // namespace xls

int main(int argc, char** argv) {
  std::vector<std::string_view> positional_arguments =
      xls::InitXls(argv[0], argc, argv);
  xls::tools::RealMain(positional_arguments);
}
