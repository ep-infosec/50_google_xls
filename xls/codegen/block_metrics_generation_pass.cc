// Copyright 2022 The XLS Authors
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

#include "xls/codegen/block_metrics_generation_pass.h"

#include "absl/strings/str_format.h"
#include "xls/codegen/block_metrics.h"
#include "xls/common/status/status_macros.h"
#include "xls/ir/node_util.h"

namespace xls::verilog {

absl::StatusOr<bool> BlockMetricsGenerationPass::RunInternal(
    CodegenPassUnit* unit, const CodegenPassOptions& options,
    PassResults* results) const {
  if (!unit->signature.has_value()) {
    return absl::InvalidArgumentError(
        "Block metrics should be run after "
        "signature generation.");
  }

  XLS_ASSIGN_OR_RETURN(BlockMetricsProto block_metrics,
                       GenerateBlockMetrics(unit->block));
  XLS_RETURN_IF_ERROR(unit->signature->ReplaceBlockMetrics(block_metrics));

  return true;
}

}  // namespace xls::verilog
