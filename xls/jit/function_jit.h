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

#ifndef XLS_JIT_FUNCTION_JIT_H_
#define XLS_JIT_FUNCTION_JIT_H_

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/types/span.h"
#include "xls/common/status/status_macros.h"
#include "xls/ir/events.h"
#include "xls/ir/function.h"
#include "xls/ir/value.h"
#include "xls/ir/value_view.h"
#include "xls/jit/function_base_jit.h"
#include "xls/jit/jit_runtime.h"
#include "xls/jit/orc_jit.h"

namespace xls {

// Data structure containing jitted object code and metadata about how to call
// it.
struct JitObjectCode {
  // Name of the top-level jitted function in the object code.
  std::string function_name;
  std::vector<uint8_t> object_code;

  // Size of the buffers for the parameters and result.
  std::vector<int64_t> parameter_buffer_sizes;
  int64_t return_buffer_size;

  // Minimum size of the temporary buffer passed to the jitted function.
  int64_t temp_buffer_size;
};

// This class provides a facility to execute XLS functions (on the host) by
// converting it to LLVM IR, compiling it, and finally executing it. Not
// thread-safe due to sharing of result and temporary buffers between
// invocations of Run.
class FunctionJit {
 public:
  // Returns an object containing a host-compiled version of the specified XLS
  // function.
  static absl::StatusOr<std::unique_ptr<FunctionJit>> Create(
      Function* xls_function, int64_t opt_level = 3);

  // Returns the bytes of an object file containing the compiled XLS function.
  static absl::StatusOr<JitObjectCode> CreateObjectCode(Function* xls_function,
                                                        int64_t opt_level = 3);

  // Executes the compiled function with the specified arguments.
  absl::StatusOr<InterpreterResult<Value>> Run(absl::Span<const Value> args);

  // As above, buth with arguments as key-value pairs.
  absl::StatusOr<InterpreterResult<Value>> Run(
      const absl::flat_hash_map<std::string, Value>& kwargs);

  // Executes the compiled function with the arguments and results specified as
  // "views" - flat buffers onto which structures layouts can be applied (see
  // value_view.h).
  //
  // Argument packing and unpacking into and out of LLVM-space can consume a
  // surprisingly large amount of execution time. Deferring such transformations
  // (and applying views) can eliminate this overhead and still give access tor
  // result data. Users needing less performance can still use the
  // Value-returning methods above for code simplicity.
  // TODO(https://github.com/google/xls/issues/506): 2021-10-13 Figure out
  // if we want a way to return events in the view and packed view interfaces
  // (or if their performance-focused execution means events are unimportant).
  absl::Status RunWithViews(absl::Span<uint8_t* const> args,
                            absl::Span<uint8_t> result_buffer,
                            InterpreterEvents* events);

  // Similar to RunWithViews(), except the arguments here are _packed_views_ -
  // views whose data elements are tightly packed, with no padding bits or bytes
  // between them. The function return value is specified as the last arg - its
  // storage must ALSO be pre-allocated before this call.
  //
  // Example (for a binary float32 operation):
  // float RunFloat(float a_f, float b_f);
  //  using PF32 = PackedTupleView<PackedBitsView<23>, ..., PackedBitsView<1>>;
  //  PF32 a(&a_f);
  //  PF32 b(&b_f);
  //  float x_f;
  //  PF32 x(&x_f);
  //  jit->RunWithPackedViews(a, b, x);
  //  return x_f;
  //
  // For most users, the autogenerated DSLX headers should be used as the JIT -
  // and especially the packed-view-using-call - interface; there are some
  // sharp edges here!
  // TODO(rspringer): Add user data support here.
  template <typename... ArgsT>
  absl::Status RunWithPackedViews(ArgsT... args) {
    XLS_RET_CHECK(jitted_function_base_.packed_function.has_value());
    uint8_t* arg_buffers[sizeof...(ArgsT)];
    uint8_t* result_buffer;
    // Walk the type tree to get each arg's data buffer into our view/arg list.
    PackArgBuffers(arg_buffers, &result_buffer, args...);

    InterpreterEvents events;
    uint8_t* output_buffers[1] = {result_buffer};
    jitted_function_base_.packed_function.value()(
        arg_buffers, output_buffers, temp_buffer_.data(), &events,
        /*user_data=*/nullptr, runtime(), /*continuation_point=*/0);

    return InterpreterEventsToStatus(events);
  }

  // Returns the function that the JIT executes.
  Function* function() { return xls_function_; }

  // Gets the size of the compiled function's arguments (or return value) in the
  // native LLVM data layout (not the packed layout).
  int64_t GetArgTypeSize(int arg_index) const {
    return jitted_function_base_.input_buffer_sizes.at(arg_index);
  }
  int64_t GetReturnTypeSize() const {
    return jitted_function_base_.output_buffer_sizes[0];
  }

  // Gets the size of the compiled function's arguments (or return value) in the
  // packed layout.
  int64_t GetPackedArgTypeSize(int arg_index) const {
    return jitted_function_base_.packed_input_buffer_sizes.at(arg_index);
  }
  int64_t GetPackedReturnTypeSize() const {
    return jitted_function_base_.output_buffer_sizes[0];
  }

  // Returns the size of the temporary buffer which must be passed to the jitted
  // function. The buffer is used to hold temporary node values inside the
  // jitted function.
  int64_t GetTempBufferSize() const {
    return jitted_function_base_.temp_buffer_size;
  }

  // Returns the name of the jitted function.
  std::string_view GetJittedFunctionName() const {
    return jitted_function_base_.function_name;
  }

  JitRuntime* runtime() const { return jit_runtime_.get(); }

 private:
  explicit FunctionJit(Function* xls_function) : xls_function_(xls_function) {}

  static absl::StatusOr<std::unique_ptr<FunctionJit>> CreateInternal(
      Function* xls_function, int64_t opt_level, bool emit_object_code);

  // Builds a function which wraps the natively compiled XLS function `callee`
  // (as built by xls::BuildFunction) with another function which accepts the
  // input arguments as an array of pointers to buffers and the output as a
  // pointer to a buffer. The input/output values are in the native LLVM data
  // layout. The function signature is:
  //
  //   void f(uint8_t*[] inputs, uint8_t* output,
  //          void* events, void* user_data, void* jit_runtime)
  //
  // `inputs` is an array containing a pointer for each input argument. The
  // pointer points to a buffer containing the respective argument in the native
  // LLVM data layout.
  //
  // `outputs` points to an empty buffer appropriately sized to accept the
  // result in the native LLVM data layout.
  absl::StatusOr<llvm::Function*> BuildWrapper(llvm::Function* callee);

  // As BuildWrapper but the inputs and outputs are taken/returned in a packed
  // representation.
  absl::StatusOr<llvm::Function*> BuildPackedWrapper(llvm::Function* callee);

  // Simple templates to walk down the arg tree and populate the corresponding
  // arg/buffer pointer.
  template <typename FrontT, typename... RestT>
  void PackArgBuffers(uint8_t** arg_buffers, uint8_t** result_buffer,
                      FrontT front, RestT... rest) {
    arg_buffers[0] = front.buffer();
    PackArgBuffers(&arg_buffers[1], result_buffer, rest...);
  }

  // Base case for the above recursive template.
  template <typename LastT>
  void PackArgBuffers(uint8_t** arg_buffers, uint8_t** result_buffer,
                      LastT front) {
    *result_buffer = front.buffer();
  }

  // Invokes the jitted function with the given argument and outputs.
  void InvokeJitFunction(absl::Span<uint8_t* const> arg_buffers,
                         uint8_t* output_buffer, InterpreterEvents* events);

  std::unique_ptr<OrcJit> orc_jit_;

  Function* xls_function_;

  // Buffers to hold the arguments, result, temporary storage. This is allocated
  // once and then re-used with each invocation of Run. Not thread-safe.
  std::vector<std::vector<uint8_t>> arg_buffers_;
  std::vector<uint8_t> result_buffer_;
  std::vector<uint8_t> temp_buffer_;

  // Raw pointers to the buffers held in `arg_buffers_`.
  std::vector<uint8_t*> arg_buffer_ptrs_;

  JittedFunctionBase jitted_function_base_;
  std::unique_ptr<JitRuntime> jit_runtime_;
};

}  // namespace xls

#endif  // XLS_JIT_FUNCTION_JIT_H_
