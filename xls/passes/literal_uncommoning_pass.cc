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

#include "xls/passes/literal_uncommoning_pass.h"

#include "absl/status/statusor.h"
#include "xls/common/status/status_macros.h"
#include "xls/ir/dfs_visitor.h"
#include "xls/ir/node_iterator.h"

namespace xls {

absl::StatusOr<bool> LiteralUncommoningPass::RunOnFunctionBaseInternal(
    FunctionBase* f, const PassOptions& options, PassResults* results) const {
  // Construct separate list of the initial literals to avoid iterator
  // invalidation problems because we will be adding additional nodes during
  // the transformation.
  std::vector<Literal*> literals;
  for (const auto& node : f->nodes()) {
    if (node->Is<Literal>() && node->GetType()->IsBits()) {
      literals.push_back(node->As<Literal>());
    }
  }

  bool changed = false;
  for (Literal* literal : literals) {
    bool is_first_use = true;
    std::vector<Node*> uses(literal->users().begin(), literal->users().end());
    for (Node* use : uses) {
      // Iterate through the operands explicitly because we want a new literal
      // for each operand slot.
      for (int64_t operand_no = 0; operand_no < use->operand_count();
           ++operand_no) {
        if (use->operand(operand_no) == literal) {
          if (is_first_use) {
            // Keep the first use, and clone the literal for the remaining uses.
            is_first_use = false;
            continue;
          }
          XLS_ASSIGN_OR_RETURN(Node * clone,
                               literal->Clone(/*new_operands=*/{}));
          XLS_RETURN_IF_ERROR(use->ReplaceOperandNumber(operand_no, clone));
          changed = true;
        }
      }
    }
  }

  return changed;
}

}  // namespace xls
