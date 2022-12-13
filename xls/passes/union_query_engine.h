// Copyright 2021 The XLS Authors
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

#ifndef XLS_PASSES_UNION_QUERY_ENGINE_H_
#define XLS_PASSES_UNION_QUERY_ENGINE_H_

#include <memory>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/status/statusor.h"
#include "absl/types/optional.h"
#include "xls/ir/bits.h"
#include "xls/ir/interval_set.h"
#include "xls/ir/nodes.h"
#include "xls/passes/query_engine.h"

namespace xls {

// A query engine that combines the results of multiple given query engines.
//
// `GetKnownBits` and `GetKnownBitsValues` use `const_cast<...>(this)` under the
// hood, so it is undefined behavior to define a `const UnionQueryEngine`
// variable (but `const UnionQueryEngine*` is fine, the storage location just
// must be mutable). This is due to an infelicity in the QueryEngine API that
// will be fixed at some point.
class UnionQueryEngine : public QueryEngine {
 public:
  explicit UnionQueryEngine(std::vector<std::unique_ptr<QueryEngine>> engines) {
    engines_ = std::move(engines);
  }

  absl::StatusOr<ReachedFixpoint> Populate(FunctionBase* f) override;

  bool IsTracked(Node* node) const override;

  LeafTypeTree<TernaryVector> GetTernary(Node* node) const override;

  LeafTypeTree<IntervalSet> GetIntervals(Node* node) const override;

  bool AtMostOneTrue(absl::Span<TreeBitLocation const> bits) const override;

  bool AtLeastOneTrue(absl::Span<TreeBitLocation const> bits) const override;

  bool KnownEquals(const TreeBitLocation& a,
                   const TreeBitLocation& b) const override;

  bool KnownNotEquals(const TreeBitLocation& a,
                      const TreeBitLocation& b) const override;

  bool Implies(const TreeBitLocation& a,
               const TreeBitLocation& b) const override;

  std::optional<Bits> ImpliedNodeValue(
      absl::Span<const std::pair<TreeBitLocation, bool>> predicate_bit_values,
      Node* node) const override;

 private:
  absl::flat_hash_map<Node*, Bits> known_bits_;
  absl::flat_hash_map<Node*, Bits> known_bit_values_;
  std::vector<std::unique_ptr<QueryEngine>> engines_;
};

}  // namespace xls

#endif  // XLS_PASSES_UNION_QUERY_ENGINE_H_
