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

#include "xls/passes/arith_simplification_pass.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/status/statusor.h"
#include "xls/common/bits_util.h"
#include "xls/common/status/matchers.h"
#include "xls/interpreter/function_interpreter.h"
#include "xls/ir/function.h"
#include "xls/ir/function_builder.h"
#include "xls/ir/ir_matcher.h"
#include "xls/ir/ir_test_base.h"
#include "xls/ir/package.h"
#include "xls/passes/dce_pass.h"

namespace m = ::xls::op_matchers;

namespace xls {
namespace {

using status_testing::IsOkAndHolds;

class ArithSimplificationPassTest : public IrTestBase {
 protected:
  ArithSimplificationPassTest() = default;

  absl::StatusOr<bool> Run(Package* p) {
    PassResults results;
    return ArithSimplificationPass(kMaxOptLevel)
        .Run(p, PassOptions(), &results);
  }

  void CheckUnsignedDivide(int n, int divisor);
  void CheckSignedDividePowerOfTwo(int n, int divisor);
  void CheckSignedDivideNotPowerOfTwo(int n, int divisor);
};

TEST_F(ArithSimplificationPassTest, Arith1) {
  auto p = CreatePackage();
  XLS_ASSERT_OK_AND_ASSIGN(Function * f, ParseFunction(R"(
   fn double_shift(x:bits[32]) -> bits[32] {
     three:bits[32] = literal(value=3)
     two:bits[32] = literal(value=2)
     xshrl3:bits[32] = shrl(x, three)
     xshrl3_shrl2:bits[32] = shrl(xshrl3, two)
     ret result: bits[32] = add(xshrl3_shrl2, xshrl3_shrl2)
   }
   )",
                                                       p.get()));
  ASSERT_THAT(Run(p.get()), IsOkAndHolds(true));
  EXPECT_THAT(f->return_value(),
              m::Add(m::Concat(m::Literal(0), m::BitSlice()),
                     m::Concat(m::Literal(0), m::BitSlice())));
}

TEST_F(ArithSimplificationPassTest, DoubleNeg) {
  auto p = CreatePackage();
  XLS_ASSERT_OK_AND_ASSIGN(Function * f, ParseFunction(R"(
     fn simple_neg(x:bits[2]) -> bits[2] {
        neg1:bits[2] = neg(x)
        ret result: bits[2] = neg(neg1)
     }
  )",
                                                       p.get()));
  ASSERT_THAT(Run(p.get()), IsOkAndHolds(true));
  EXPECT_THAT(f->return_value(), m::Param());
}

TEST_F(ArithSimplificationPassTest, MulBy0) {
  auto p = CreatePackage();
  XLS_ASSERT_OK_AND_ASSIGN(Function * f, ParseFunction(R"(
     fn mul_zero(x:bits[8]) -> bits[8] {
        zero:bits[8] = literal(value=0)
        ret result: bits[8] = umul(x, zero)
     }
  )",
                                                       p.get()));
  ASSERT_THAT(Run(p.get()), IsOkAndHolds(true));
  EXPECT_THAT(f->return_value(), m::Literal(0));
}

TEST_F(ArithSimplificationPassTest, MulBy42) {
  auto p = CreatePackage();
  XLS_ASSERT_OK(ParseFunction(R"(
     fn mul_zero(x:bits[8]) -> bits[8] {
        literal.1: bits[8] = literal(value=42)
        ret result: bits[8] = umul(x, literal.1)
     }
  )",
                              p.get())
                    .status());
  ASSERT_THAT(Run(p.get()), IsOkAndHolds(false));
}

TEST_F(ArithSimplificationPassTest, SMulBy1SignExtendedResult) {
  auto p = CreatePackage();
  XLS_ASSERT_OK_AND_ASSIGN(Function * f, ParseFunction(R"(
     fn mul_zero(x:bits[8]) -> bits[16] {
        one: bits[8] = literal(value=1)
        ret result: bits[16] = smul(x, one)
     }
  )",
                                                       p.get()));
  ASSERT_THAT(Run(p.get()), IsOkAndHolds(true));
  EXPECT_THAT(f->return_value(), m::SignExt(m::Param("x")));
}

TEST_F(ArithSimplificationPassTest, SMulBy16SignExtendedResult) {
  auto p = CreatePackage();
  XLS_ASSERT_OK_AND_ASSIGN(Function * f, ParseFunction(R"(
     fn mul_zero(x:bits[8]) -> bits[16] {
        one: bits[8] = literal(value=16)
        ret result: bits[16] = smul(x, one)
     }
  )",
                                                       p.get()));
  ASSERT_THAT(Run(p.get()), IsOkAndHolds(true));
  EXPECT_THAT(f->return_value(),
              m::Concat(m::BitSlice(m::SignExt(m::Param("x"))),
                        m::Literal(UBits(0, 4))));
}

TEST_F(ArithSimplificationPassTest, UMulBy1ZeroExtendedResult) {
  auto p = CreatePackage();
  XLS_ASSERT_OK_AND_ASSIGN(Function * f, ParseFunction(R"(
     fn mul_zero(x:bits[8]) -> bits[16] {
        one: bits[8] = literal(value=1)
        ret result: bits[16] = umul(x, one)
     }
  )",
                                                       p.get()));
  ASSERT_THAT(Run(p.get()), IsOkAndHolds(true));
  EXPECT_THAT(f->return_value(), m::ZeroExt(m::Param("x")));
}

TEST_F(ArithSimplificationPassTest, UMulBy256ZeroExtendedResult) {
  auto p = CreatePackage();
  XLS_ASSERT_OK_AND_ASSIGN(Function * f, ParseFunction(R"(
     fn mul_zero(x:bits[8]) -> bits[16] {
        one: bits[12] = literal(value=256)
        ret result: bits[16] = umul(x, one)
     }
  )",
                                                       p.get()));
  ASSERT_THAT(Run(p.get()), IsOkAndHolds(true));
  EXPECT_THAT(f->return_value(),
              m::Concat(m::BitSlice(m::ZeroExt(m::Param("x"))),
                        m::Literal(UBits(0, 8))));
}

TEST_F(ArithSimplificationPassTest, UModBy4) {
  auto p = CreatePackage();
  XLS_ASSERT_OK_AND_ASSIGN(Function * f, ParseFunction(R"(
     fn umod_power_of_two(x:bits[16]) -> bits[16] {
        literal.1: bits[16] = literal(value=4)
        ret result: bits[16] = umod(x, literal.1)
     }
  )",
                                                       p.get()));
  ASSERT_THAT(Run(p.get()), IsOkAndHolds(true));
  EXPECT_THAT(f->return_value(), m::And(m::Param("x"), m::Literal(3)));
}

TEST_F(ArithSimplificationPassTest, UModBy1) {
  auto p = CreatePackage();
  XLS_ASSERT_OK_AND_ASSIGN(Function * f, ParseFunction(R"(
     fn umod_power_of_two(x:bits[16]) -> bits[16] {
        literal.1: bits[16] = literal(value=1)
        ret result: bits[16] = umod(x, literal.1)
     }
  )",
                                                       p.get()));
  ASSERT_THAT(Run(p.get()), IsOkAndHolds(true));
  EXPECT_THAT(f->return_value(), m::Literal(0));
}

TEST_F(ArithSimplificationPassTest, UModBy512) {
  auto p = CreatePackage();
  XLS_ASSERT_OK_AND_ASSIGN(Function * f, ParseFunction(R"(
     fn umod_power_of_two(x:bits[16]) -> bits[16] {
        literal.1: bits[16] = literal(value=512)
        ret result: bits[16] = umod(x, literal.1)
     }
  )",
                                                       p.get()));
  ASSERT_THAT(Run(p.get()), IsOkAndHolds(true));
  EXPECT_THAT(f->return_value(), m::And(m::Param("x"), m::Literal(0x1ff)));
}

TEST_F(ArithSimplificationPassTest, UModBy42) {
  auto p = CreatePackage();
  XLS_ASSERT_OK_AND_ASSIGN(Function * f, ParseFunction(R"(
     fn umod_power_of_two(x:bits[16]) -> bits[16] {
        literal.1: bits[16] = literal(value=42)
        ret result: bits[16] = umod(x, literal.1)
     }
  )",
                                                       p.get()));
  ASSERT_THAT(Run(p.get()), IsOkAndHolds(false));
  EXPECT_THAT(f->return_value(), m::UMod());
}

TEST_F(ArithSimplificationPassTest, UModBy0) {
  auto p = CreatePackage();
  XLS_ASSERT_OK_AND_ASSIGN(Function * f, ParseFunction(R"(
     fn umod_power_of_two(x:bits[16]) -> bits[16] {
        literal.1: bits[16] = literal(value=0)
        ret result: bits[16] = umod(x, literal.1)
     }
  )",
                                                       p.get()));
  ASSERT_THAT(Run(p.get()), IsOkAndHolds(false));
  EXPECT_THAT(f->return_value(), m::UMod());
}

TEST_F(ArithSimplificationPassTest, UDivBy4) {
  auto p = CreatePackage();
  XLS_ASSERT_OK_AND_ASSIGN(Function * f, ParseFunction(R"(
     fn mul_zero(x:bits[16]) -> bits[16] {
        literal.1: bits[16] = literal(value=4)
        ret result: bits[16] = udiv(x, literal.1)
     }
  )",
                                                       p.get()));
  ASSERT_THAT(Run(p.get()), IsOkAndHolds(true));
  EXPECT_THAT(f->return_value(),
              m::Concat(m::Literal(UBits(0, 2)), m::BitSlice(m::Param("x"))));
}

TEST_F(ArithSimplificationPassTest, SDivBy1) {
  auto p = CreatePackage();
  XLS_ASSERT_OK_AND_ASSIGN(Function * f, ParseFunction(R"(
     fn sdiv_by_1(x:bits[16]) -> bits[16] {
        literal.1: bits[16] = literal(value=1)
        ret result: bits[16] = sdiv(x, literal.1)
     }
  )",
                                                       p.get()));
  ASSERT_THAT(Run(p.get()), IsOkAndHolds(true));
  EXPECT_THAT(f->return_value(), m::Param("x"));
}

TEST_F(ArithSimplificationPassTest, OneBitSDivByMinus1) {
  // 0b1 is -1 for a bits[1] type so sdiv by literal one should not apply.
  auto p = CreatePackage();
  XLS_ASSERT_OK_AND_ASSIGN(Function * f, ParseFunction(R"(
     fn one_bit_sdiv(x:bits[1]) -> bits[1] {
        literal.1: bits[1] = literal(value=1)
        ret result: bits[1] = sdiv(x, literal.1)
     }
  )",
                                                       p.get()));
  ASSERT_THAT(Run(p.get()), IsOkAndHolds(true));
  EXPECT_THAT(f->return_value(), m::Neg());

  auto interp_and_check = [&f](int x, int expected) {
    constexpr int N = 1;
    XLS_ASSERT_OK_AND_ASSIGN(InterpreterResult<Value> r,
                             InterpretFunction(f, {Value(SBits(x, N))}));
    EXPECT_EQ(r.value, Value(SBits(expected, N)));
  };
  // Even though the operation is negate, a 1-bit value can't be negated so the
  // overall effect is no change.
  interp_and_check(0, 0);
  interp_and_check(-1, -1);
}

TEST_F(ArithSimplificationPassTest, SDivWithLiteralDivisorSweep) {
  // Sweep all possible values for a Bits[3] SDIV with a literal divisor.
  for (int64_t divisor = -4; divisor <= 3; ++divisor) {
    for (int64_t dividend = -4; dividend <= 3; ++dividend) {
      auto p = CreatePackage();
      Type* u3 = p->GetBitsType(3);
      FunctionBuilder fb(TestName(), p.get());
      fb.SDiv(fb.Param("dividend", u3), fb.Literal(SBits(divisor, 3)));
      XLS_ASSERT_OK_AND_ASSIGN(Function * f, fb.Build());

      Value expected;
      // XLS divide by zero semantics are to return the maximal
      // positive/negative value.
      if (divisor == 0) {
        expected = Value(SBits(dividend >= 0 ? 3 : -4, 3));
      } else if (dividend == -4 && divisor == -1) {
        // Overflow. In this case we just return the truncated twos-complement
        // result (0b0100 => 0b100).
        expected = Value(SBits(-4, 3));
      } else {
        expected = Value(SBits(dividend / divisor, 3));
      }
      XLS_VLOG(1) << absl::StreamFormat("%d / %d = %d (%s)", dividend, divisor,
                                        expected.bits().ToInt64().value(),
                                        expected.ToString());
      XLS_ASSERT_OK_AND_ASSIGN(
          InterpreterResult<Value> before_result,
          InterpretFunction(f, {Value(SBits(dividend, 3))}));
      XLS_ASSERT_OK_AND_ASSIGN(Value before_value,
                               InterpreterResultToStatusOrValue(before_result));
      EXPECT_EQ(before_value, expected)
          << absl::StreamFormat("Before: %d / %d", dividend, divisor);

      XLS_ASSERT_OK(Run(p.get()));

      XLS_ASSERT_OK_AND_ASSIGN(
          InterpreterResult<Value> after_result,
          InterpretFunction(f, {Value(SBits(dividend, 3))}));
      XLS_ASSERT_OK_AND_ASSIGN(Value after_value,
                               InterpreterResultToStatusOrValue(after_result));
      EXPECT_EQ(after_result.value, expected)
          << absl::StreamFormat("After: %d / %d", dividend, divisor);
    }
  }
}

bool Contains(const std::string& haystack, const std::string& needle) {
  return haystack.find(needle) != std::string::npos;
}

// Optimizes the IR for an unsigned divide by the given non-power of two. Checks
// that optimized IR matches expected IR. Numerically validates (via
// interpretation) the optimized IR, exhaustively up to 2^n.
void ArithSimplificationPassTest::CheckUnsignedDivide(int n, int divisor) {
  auto p = CreatePackage();
  FunctionBuilder fb("UnsignedDivideBy" + std::to_string(divisor), p.get());
  fb.UDiv(fb.Param("x", p->GetBitsType(n)),
          fb.Literal(Value(UBits(divisor, n))));
  XLS_ASSERT_OK_AND_ASSIGN(Function * f, fb.Build());
  ASSERT_THAT(Run(p.get()), IsOkAndHolds(true));

  // Clean up the dumped IR
  PassResults results;
  ASSERT_THAT(DeadCodeEliminationPass().Run(p.get(), PassOptions(), &results),
              IsOkAndHolds(true));

  std::string optimized_ir = f->DumpIr();

  // A non-power of two divisor will be rewritten to shifts (which are further
  // rewritten to slices), mul. No div, add, or sub.
  EXPECT_TRUE(Contains(optimized_ir, "bit_slice"));
  EXPECT_TRUE(Contains(optimized_ir, "umul"));
  EXPECT_FALSE(Contains(optimized_ir, "div"));
  EXPECT_FALSE(Contains(optimized_ir, "add"));
  EXPECT_FALSE(Contains(optimized_ir, "sub"));

  // compute x/divisor. assert result == expected
  auto interp_and_check = [n, &f](int x, int expected) {
    XLS_ASSERT_OK_AND_ASSIGN(InterpreterResult<Value> r,
                             InterpretFunction(f, {Value(UBits(x, n))}));
    EXPECT_EQ(r.value, Value(UBits(expected, n)));
  };

  // compute RoundToZero(i/divisor)
  auto div_rt0 = [=](int i) { return i / divisor; };

  for (int i = 0; i < Exp2<int>(n); ++i) {
    interp_and_check(i, div_rt0(i));
  }
}

// Exhaustively test unsigned division. Vary n and divisor (excluding powers of
// two).
TEST_F(ArithSimplificationPassTest, UnsignedDivideAllNonPowersOfTwoExhaustive) {
  constexpr int kTestUpToN = 10;
  for (int divisor = 1; divisor < Exp2<int>(kTestUpToN); ++divisor) {
    if (!IsPowerOfTwo(static_cast<uint>(divisor))) {
      for (int n = Bits::MinBitCountUnsigned(divisor); n <= kTestUpToN; ++n) {
        CheckUnsignedDivide(n, divisor);
      }
    }
  }
}

// Regression test for
// https://github.com/google/xls/issues/736
TEST_F(ArithSimplificationPassTest, UDivWrongIssue736) {
  constexpr uint64_t nBits = 43;
  constexpr uint64_t divisor = UINT64_C(1876853526877);

  auto p = CreatePackage();
  FunctionBuilder fb("UnsignedDivideBy" + std::to_string(divisor), p.get());
  fb.UDiv(fb.Param("x", p->GetBitsType(nBits)),
          fb.Literal(Value(UBits(divisor, nBits))));
  XLS_ASSERT_OK_AND_ASSIGN(Function * f, fb.Build());
  ASSERT_THAT(Run(p.get()), IsOkAndHolds(true));

  // Clean up the dumped IR
  PassResults results;
  ASSERT_THAT(DeadCodeEliminationPass().Run(p.get(), PassOptions(), &results),
              IsOkAndHolds(true));

  std::string optimized_ir = f->DumpIr();

  // A non-power of two divisor will be rewritten to shifts (which are further
  // rewritten to slices), mul. No div, add, or sub.
  EXPECT_TRUE(Contains(optimized_ir, "bit_slice"));
  EXPECT_TRUE(Contains(optimized_ir, "umul"));
  EXPECT_FALSE(Contains(optimized_ir, "div"));
  EXPECT_FALSE(Contains(optimized_ir, "add"));
  EXPECT_FALSE(Contains(optimized_ir, "sub"));

  // compute x/divisor. assert result == expected
  auto interp_and_check = [&f](uint64_t x, uint64_t expected) {
    XLS_ASSERT_OK_AND_ASSIGN(InterpreterResult<Value> r,
                             InterpretFunction(f, {Value(UBits(x, nBits))}));
    EXPECT_EQ(r.value, Value(UBits(expected, nBits)));
  };

  // compute RoundToZero(i/divisor)
  auto div_rt0 = [=](uint64_t i) { return i / divisor; };

  // The input that triggered the bug
  uint64_t x = UINT64_C(5864062014805);  // 0x555_5555_5555
  interp_and_check(x, div_rt0(x));
}

// Regression test for
// https://github.com/google/xls/issues/736
TEST_F(ArithSimplificationPassTest, SDivWrongIssue736) {
  constexpr uint64_t nBits = 66;
  constexpr int64_t divisor =
      INT64_C(2305843009213693950);  // floor(dividend/2 - 1) = 2^61 - 2

  auto p = CreatePackage();
  FunctionBuilder fb("UnsignedDivideBy" + std::to_string(divisor), p.get());
  fb.SDiv(fb.Param("x", p->GetBitsType(nBits)),
          fb.Literal(Value(SBits(divisor, nBits))));
  XLS_ASSERT_OK_AND_ASSIGN(Function * f, fb.Build());
  ASSERT_THAT(Run(p.get()), IsOkAndHolds(true));

  // Clean up the dumped IR
  PassResults results;
  ASSERT_THAT(DeadCodeEliminationPass().Run(p.get(), PassOptions(), &results),
              IsOkAndHolds(true));

  std::string optimized_ir = f->DumpIr();

  // A non-power of two divisor will be rewritten to shifts (which are further
  // rewritten to slices), mul, sub. No div or add.
  EXPECT_TRUE(Contains(optimized_ir, "bit_slice"));
  EXPECT_TRUE(Contains(optimized_ir, "smul"));
  EXPECT_TRUE(Contains(optimized_ir, "sub"));
  EXPECT_FALSE(Contains(optimized_ir, "div"));
  EXPECT_FALSE(Contains(optimized_ir, "add"));

  // compute x/divisor. assert result == expected
  auto interp_and_check = [&f](int64_t x, int64_t expected) {
    XLS_ASSERT_OK_AND_ASSIGN(InterpreterResult<Value> r,
                             InterpretFunction(f, {Value(UBits(x, nBits))}));
    EXPECT_EQ(r.value, Value(UBits(expected, nBits)));
  };

  // compute RoundToZero(i/divisor)
  auto div_rt0 = [=](int64_t i) { return i / divisor; };

  // The input that triggered the bug
  int64_t x = INT64_C(4611686018427387903);  // 2^62 - 1

  // obviously, this should be 2
  interp_and_check(x, div_rt0(x));
}

// Optimizes the IR for a divide by a power of two (which may be negative or
// positive). Checks that optimized IR matches expected IR. Numerically
// validates (via interpretation) the optimized IR, exhaustively up to 2^N.
void ArithSimplificationPassTest::CheckSignedDividePowerOfTwo(int n,
                                                              int divisor) {
  auto p = CreatePackage();
  FunctionBuilder fb(TestName(), p.get());
  fb.SDiv(fb.Param("x", p->GetBitsType(n)),
          fb.Literal(Value(SBits(divisor, n))));
  XLS_ASSERT_OK_AND_ASSIGN(Function * f, fb.Build());
  ASSERT_THAT(Run(p.get()), IsOkAndHolds(true));

  // Clean up the dumped IR
  PassResults results;
  ASSERT_THAT(DeadCodeEliminationPass().Run(p.get(), PassOptions(), &results),
              IsOkAndHolds(true));

  std::string optimized_ir = f->DumpIr();

  if (std::abs(divisor) > 1) {
    // A power of two divisor will be rewritten to shifts (which are further
    // rewritten to slices) and an add. There will be no multiply (unlike other
    // divisors).
    EXPECT_TRUE(Contains(optimized_ir, "bit_slice"));
    EXPECT_TRUE(Contains(optimized_ir, "add"));
    EXPECT_FALSE(Contains(optimized_ir, "div"));
    EXPECT_FALSE(Contains(optimized_ir, "mul"));
  } else if (divisor == 1) {
    EXPECT_FALSE(Contains(optimized_ir, "neg"));
    EXPECT_FALSE(Contains(optimized_ir, "bit_slice"));
    EXPECT_FALSE(Contains(optimized_ir, "add"));
    EXPECT_FALSE(Contains(optimized_ir, "div"));
    EXPECT_FALSE(Contains(optimized_ir, "mul"));
  } else if (divisor == -1) {
    EXPECT_TRUE(Contains(optimized_ir, "neg"));
    EXPECT_FALSE(Contains(optimized_ir, "bit_slice"));
    EXPECT_FALSE(Contains(optimized_ir, "add"));
    EXPECT_FALSE(Contains(optimized_ir, "div"));
    EXPECT_FALSE(Contains(optimized_ir, "mul"));
  }

  // compute x/divisor. assert result == expected
  auto interp_and_check = [n, &f](int x, int expected) {
    XLS_ASSERT_OK_AND_ASSIGN(InterpreterResult<Value> r,
                             InterpretFunction(f, {Value(SBits(x, n))}));
    EXPECT_EQ(r.value, Value(SBits(expected, n)));
  };
  // compute RoundToZero(i/divisor)
  auto div_rt0 = [=](int i) {
    const int q = std::abs(i) / std::abs(divisor);
    const bool exactly_one_negative =
        (i < 0 || divisor < 0) && !(i < 0 && divisor < 0);
    return exactly_one_negative ? -q : q;
  };

  // N-1 because we create signed values
  for (int i = 0; i < Exp2<uint>(n - 1); ++i) {
    interp_and_check(i, div_rt0(i));
    interp_and_check(-i, div_rt0(-i));
  }

  // Avoid overflow: -2^(N-1) * -1 = 2^(N-1) which won't fit in a signed N-bit
  // integer.
  if (divisor != -1) {
    const int last = -Exp2<uint>(n - 1);
    interp_and_check(last, div_rt0(last));
  }
}

// Exhaustively test signed division by power of two. Vary N and divisor.
TEST_F(ArithSimplificationPassTest, SignedDivideAllPowersOfTwoExhaustive) {
  const int kTestUpToN = 14;
  // first divisor = 2^0 = 1
  for (int exp = 0; exp <= kTestUpToN; ++exp) {
    const int divisor = Exp2<int>(exp);
    for (int n = Bits::MinBitCountSigned(divisor); n <= kTestUpToN; ++n) {
      CheckSignedDividePowerOfTwo(n, divisor);
      CheckSignedDividePowerOfTwo(n, -divisor);
    }
  }
}

void ArithSimplificationPassTest::CheckSignedDivideNotPowerOfTwo(int n,
                                                                 int divisor) {
  auto p = CreatePackage();
  FunctionBuilder fb("SignedDivideBy" + std::to_string(divisor), p.get());
  fb.SDiv(fb.Param("x", p->GetBitsType(n)),
          fb.Literal(Value(SBits(divisor, n))));
  XLS_ASSERT_OK_AND_ASSIGN(Function * f, fb.Build());
  ASSERT_THAT(Run(p.get()), IsOkAndHolds(true));

  // Clean up the dumped IR
  PassResults results;
  ASSERT_THAT(DeadCodeEliminationPass().Run(p.get(), PassOptions(), &results),
              IsOkAndHolds(true));

  std::string optimized_ir = f->DumpIr();

  // A non-power of two divisor will be rewritten to shifts (which are further
  // rewritten to slices), mul, sub. No div or add.
  EXPECT_TRUE(Contains(optimized_ir, "bit_slice"));
  EXPECT_TRUE(Contains(optimized_ir, "smul"));
  EXPECT_TRUE(Contains(optimized_ir, "sub"));
  EXPECT_FALSE(Contains(optimized_ir, "div"));
  EXPECT_FALSE(Contains(optimized_ir, "add"));

  // compute x/divisor. assert result == expected
  auto interp_and_check = [n, &f](int x, int expected) {
    XLS_ASSERT_OK_AND_ASSIGN(InterpreterResult<Value> r,
                             InterpretFunction(f, {Value(SBits(x, n))}));
    EXPECT_EQ(r.value, Value(SBits(expected, n)));
  };

  // compute RoundToZero(i/divisor)
  auto div_rt0 = [=](int i) {
    const int q = std::abs(i) / std::abs(divisor);
    const bool exactly_one_negative =
        (i < 0 || divisor < 0) && !(i < 0 && divisor < 0);
    return exactly_one_negative ? -q : q;
  };

  // N-1 because we create signed values
  for (int i = 0; i < Exp2<uint>(n - 1); ++i) {
    interp_and_check(i, div_rt0(i));
    interp_and_check(-i, div_rt0(-i));
  }
  const int last = -Exp2<uint>(n - 1);
  interp_and_check(last, div_rt0(last));
}

// Exhaustively test signed division. For divisor, test all non-powers of 2, up
// to...
TEST_F(ArithSimplificationPassTest, SignedDivideAllNonPowersOfTwoExhaustive) {
  constexpr int kTestUpToN = 10;
  for (int divisor = 1; divisor < Exp2<int>(kTestUpToN - 1); ++divisor) {
    if (!IsPowerOfTwo(static_cast<uint>(divisor))) {
      for (int n = Bits::MinBitCountSigned(divisor); n <= kTestUpToN; ++n) {
        CheckSignedDivideNotPowerOfTwo(n, divisor);
        CheckSignedDivideNotPowerOfTwo(n, -divisor);
      }
    }
  }
}

TEST_F(ArithSimplificationPassTest, MulBy1NarrowedResult) {
  auto p = CreatePackage();
  XLS_ASSERT_OK_AND_ASSIGN(Function * f, ParseFunction(R"(
     fn mul_zero(x:bits[8]) -> bits[3] {
        one: bits[8] = literal(value=1)
        ret result: bits[3] = umul(x, one)
     }
  )",
                                                       p.get()));
  ASSERT_THAT(Run(p.get()), IsOkAndHolds(true));
  EXPECT_THAT(f->return_value(),
              m::BitSlice(m::Param("x"), /*start=*/0, /*width=*/3));
}

TEST_F(ArithSimplificationPassTest, UMulByMaxPowerOfTwo) {
  auto p = CreatePackage();
  XLS_ASSERT_OK_AND_ASSIGN(Function * f, ParseFunction(R"(
     fn mul_zero(x:bits[16]) -> bits[16] {
        literal.1: bits[8] = literal(value=128)
        ret result: bits[16] = umul(x, literal.1)
     }
  )",
                                                       p.get()));
  ASSERT_THAT(Run(p.get()), IsOkAndHolds(true));
  EXPECT_THAT(f->return_value(),
              m::Concat(m::BitSlice(m::Param("x")), m::Literal(UBits(0, 7))));
}

TEST_F(ArithSimplificationPassTest, SMulByMinNegative) {
  // The minimal negative number has only one bit set like powers of two do, but
  // the mul-by-power-of-two optimization should not kick in.
  auto p = CreatePackage();
  XLS_ASSERT_OK_AND_ASSIGN(Function * f, ParseFunction(R"(
     fn mul_zero(x:bits[8]) -> bits[8] {
        literal.1: bits[8] = literal(value=128)
        ret result: bits[8] = smul(x, literal.1)
     }
  )",
                                                       p.get()));
  ASSERT_THAT(Run(p.get()), IsOkAndHolds(false));
  EXPECT_THAT(f->return_value(), m::SMul());
}

TEST_F(ArithSimplificationPassTest, SMulByMinusOne) {
  // A single-bit value of 1 is a -1 when interpreted as a signed number. The
  // Mul-by-power-of-two optimization should not kick in in this case.
  auto p = CreatePackage();
  XLS_ASSERT_OK_AND_ASSIGN(Function * f, ParseFunction(R"(
     fn mul_zero(x:bits[8]) -> bits[3] {
        one: bits[1] = literal(value=1)
        ret result: bits[3] = smul(x, one)
     }
  )",
                                                       p.get()));
  ASSERT_THAT(Run(p.get()), IsOkAndHolds(false));
  EXPECT_THAT(f->return_value(), m::SMul());
}

TEST_F(ArithSimplificationPassTest, UDivBy1) {
  auto p = CreatePackage();
  XLS_ASSERT_OK_AND_ASSIGN(Function * f, ParseFunction(R"(
     fn div_one(x:bits[8]) -> bits[8] {
        one:bits[8] = literal(value=1)
        ret result: bits[8] = udiv(x, one)
     }
  )",
                                                       p.get()));
  ASSERT_THAT(Run(p.get()), IsOkAndHolds(true));
  EXPECT_THAT(f->return_value(), m::Param("x"));
}

TEST_F(ArithSimplificationPassTest, MulBy1) {
  auto p = CreatePackage();
  XLS_ASSERT_OK_AND_ASSIGN(Function * f, ParseFunction(R"(
     fn mul_zero(x:bits[8]) -> bits[8] {
        one:bits[8] = literal(value=1)
        ret result: bits[8] = umul(x, one)
     }
  )",
                                                       p.get()));
  ASSERT_THAT(Run(p.get()), IsOkAndHolds(true));
  EXPECT_THAT(f->return_value(), m::Param("x"));
}

TEST_F(ArithSimplificationPassTest, CanonicalizeXorAllOnes) {
  auto p = CreatePackage();
  XLS_ASSERT_OK_AND_ASSIGN(Function * f, ParseFunction(R"(
     fn f(x:bits[2]) -> bits[2] {
        literal.1: bits[2] = literal(value=3)
        ret result: bits[2] = xor(x, literal.1)
     }
  )",
                                                       p.get()));
  ASSERT_THAT(Run(p.get()), IsOkAndHolds(true));
  EXPECT_THAT(f->return_value(), m::Not(m::Param()));
}

TEST_F(ArithSimplificationPassTest, OverlargeShiftAfterSimp) {
  auto p = CreatePackage();
  XLS_ASSERT_OK_AND_ASSIGN(Function * f, ParseFunction(R"(
     fn f(x:bits[2]) -> bits[2] {
        literal.1: bits[2] = literal(value=2)
        shrl.2: bits[2] = shrl(x, literal.1)
        ret result: bits[2] = shrl(shrl.2, literal.1)
     }
  )",
                                                       p.get()));
  ASSERT_THAT(Run(p.get()), IsOkAndHolds(true));
  EXPECT_THAT(f->return_value(), m::Literal(0));
}

TEST_F(ArithSimplificationPassTest, ShiftRightArithmeticByLiteral) {
  auto p = CreatePackage();
  XLS_ASSERT_OK_AND_ASSIGN(Function * f, ParseFunction(R"(
     fn f(x:bits[16]) -> bits[16] {
        literal.1: bits[16] = literal(value=13)
        ret result: bits[16] = shra(x, literal.1)
     }
  )",
                                                       p.get()));
  ASSERT_THAT(Run(p.get()), IsOkAndHolds(true));
  EXPECT_THAT(
      f->return_value(),
      m::SignExt(m::BitSlice(m::Param("x"), /*start=*/13, /*width=*/3)));
}

TEST_F(ArithSimplificationPassTest, OverlargeShiftRightArithmeticByLiteral) {
  auto p = CreatePackage();
  XLS_ASSERT_OK_AND_ASSIGN(Function * f, ParseFunction(R"(
     fn f(x:bits[16]) -> bits[16] {
        literal.1: bits[16] = literal(value=1234)
        ret result: bits[16] = shra(x, literal.1)
     }
  )",
                                                       p.get()));
  ASSERT_THAT(Run(p.get()), IsOkAndHolds(true));
  EXPECT_THAT(f->return_value(),
              m::SignExt(m::BitSlice(/*start=*/15, /*width=*/1)));
}

TEST_F(ArithSimplificationPassTest, CompareBoolAgainstOne) {
  auto p = CreatePackage();
  XLS_ASSERT_OK_AND_ASSIGN(Function * f, ParseFunction(R"(
     fn f(x:bits[1]) -> bits[1] {
        literal.1: bits[1] = literal(value=1)
        ret result: bits[1] = eq(x, literal.1)
     }
  )",
                                                       p.get()));
  EXPECT_TRUE(f->return_value()->Is<CompareOp>());
  ASSERT_THAT(Run(p.get()), IsOkAndHolds(true));
  EXPECT_TRUE(f->return_value()->Is<Param>());
}

TEST_F(ArithSimplificationPassTest, CompareBoolAgainstZero) {
  auto p = CreatePackage();
  XLS_ASSERT_OK_AND_ASSIGN(Function * f, ParseFunction(R"(
     fn f(x:bits[1]) -> bits[1] {
        literal.1: bits[1] = literal(value=0)
        ret result: bits[1] = eq(x, literal.1)
     }
  )",
                                                       p.get()));
  EXPECT_TRUE(f->return_value()->Is<CompareOp>());
  ASSERT_THAT(Run(p.get()), IsOkAndHolds(true));
  EXPECT_THAT(f->return_value(), m::Not(m::Param()));
}

TEST_F(ArithSimplificationPassTest, DoubleNegation) {
  auto p = CreatePackage();
  XLS_ASSERT_OK_AND_ASSIGN(Function * f, ParseFunction(R"(
     fn f(x: bits[42]) -> bits[42] {
        not.2: bits[42] = not(x)
        ret result: bits[42] = not(not.2)
     }
  )",
                                                       p.get()));
  EXPECT_TRUE(f->return_value()->Is<UnOp>());
  ASSERT_THAT(Run(p.get()), IsOkAndHolds(true));
  EXPECT_THAT(f->return_value(), m::Param("x"));
}

TEST_F(ArithSimplificationPassTest, NaryOrEliminateSeveralZeros) {
  auto p = CreatePackage();
  XLS_ASSERT_OK_AND_ASSIGN(Function * f, ParseFunction(R"(
     fn f(x: bits[8], y: bits[8]) -> bits[8] {
        literal.3: bits[8] = literal(value=0)
        literal.4: bits[8] = literal(value=0)
        ret result: bits[8] = or(x, literal.3, y, literal.4)
     }
  )",
                                                       p.get()));
  ASSERT_THAT(Run(p.get()), IsOkAndHolds(true));
  EXPECT_THAT(f->return_value(), m::Or(m::Param("x"), m::Param("y")));
}

TEST_F(ArithSimplificationPassTest, NaryAndEliminateSeveralOnes) {
  auto p = CreatePackage();
  XLS_ASSERT_OK_AND_ASSIGN(Function * f, ParseFunction(R"(
     fn f(x: bits[8], y: bits[8]) -> bits[8] {
        literal.3: bits[8] = literal(value=0xff)
        literal.4: bits[8] = literal(value=0xff)
        ret result: bits[8] = and(x, literal.3, y, literal.4)
     }
  )",
                                                       p.get()));
  ASSERT_THAT(Run(p.get()), IsOkAndHolds(true));
  EXPECT_THAT(f->return_value(), m::And(m::Param("x"), m::Param("y")));
}

TEST_F(ArithSimplificationPassTest, NaryAndEliminateAllOnes) {
  auto p = CreatePackage();
  XLS_ASSERT_OK_AND_ASSIGN(Function * f, ParseFunction(R"(
     fn f() -> bits[8] {
        literal.1: bits[8] = literal(value=0xff)
        literal.2: bits[8] = literal(value=0xff)
        ret result: bits[8] = and(literal.1, literal.2)
     }
  )",
                                                       p.get()));
  ASSERT_THAT(Run(p.get()), IsOkAndHolds(true));
  EXPECT_THAT(f->return_value(), m::Literal(255));
}

TEST_F(ArithSimplificationPassTest, NaryFlattening) {
  auto p = CreatePackage();
  XLS_ASSERT_OK_AND_ASSIGN(Function * f, ParseFunction(R"(
     fn f(x: bits[8], y: bits[8], z: bits[8]) -> bits[8] {
        literal.3: bits[8] = literal(value=0x1f)
        literal.4: bits[8] = literal(value=0x0f)
        and.5: bits[8] = and(z, literal.3)
        ret result: bits[8] = and(x, y, literal.4, and.5)
     }
  )",
                                                       p.get()));
  ASSERT_THAT(Run(p.get()), IsOkAndHolds(true));
  EXPECT_THAT(f->return_value(), m::And(m::Param("x"), m::Param("y"),
                                        m::Param("z"), m::Literal(15)));
}

TEST_F(ArithSimplificationPassTest, NaryLiteralConsolidation) {
  auto p = CreatePackage();
  XLS_ASSERT_OK_AND_ASSIGN(Function * f, ParseFunction(R"(
fn f(x: bits[8], y: bits[8], z: bits[8]) -> bits[8] {
  literal.4: bits[8] = literal(value=15)
  literal.5: bits[8] = literal(value=31)
  ret result: bits[8] = and(x, y, literal.4, z, literal.5)
}
  )",
                                                       p.get()));
  ASSERT_THAT(Run(p.get()), IsOkAndHolds(true));
  EXPECT_THAT(f->return_value(), m::And(m::Param("x"), m::Param("y"),
                                        m::Param("z"), m::Literal(15)));
}

TEST_F(ArithSimplificationPassTest, XAndNotX) {
  auto p = CreatePackage();
  XLS_ASSERT_OK_AND_ASSIGN(Function * f, ParseFunction(R"(
     fn f(x: bits[32]) -> bits[32] {
        not.2: bits[32] = not(x)
        ret result: bits[32] = and(x, not.2)
     }
  )",
                                                       p.get()));
  ASSERT_THAT(Run(p.get()), IsOkAndHolds(true));
  EXPECT_THAT(f->return_value(), m::Literal(0));
}

TEST_F(ArithSimplificationPassTest, NotXAndX) {
  auto p = CreatePackage();
  XLS_ASSERT_OK_AND_ASSIGN(Function * f, ParseFunction(R"(
     fn f(x: bits[32]) -> bits[32] {
        not.2: bits[32] = not(x)
        ret result: bits[32] = and(not.2, x)
     }
  )",
                                                       p.get()));
  ASSERT_THAT(Run(p.get()), IsOkAndHolds(true));
  EXPECT_THAT(f->return_value(), m::Literal(0));
}

TEST_F(ArithSimplificationPassTest, SignExtTwice) {
  auto p = CreatePackage();
  XLS_ASSERT_OK_AND_ASSIGN(Function * f, ParseFunction(R"(
     fn f(x: bits[8]) -> bits[32] {
        sign_ext.2: bits[24] = sign_ext(x, new_bit_count=24)
        ret result: bits[32] = sign_ext(sign_ext.2, new_bit_count=32)
     }
  )",
                                                       p.get()));
  ASSERT_THAT(Run(p.get()), IsOkAndHolds(true));
  EXPECT_THAT(f->return_value(), m::SignExt(m::Param()));
}

TEST_F(ArithSimplificationPassTest, CollapseToNaryOr) {
  auto p = CreatePackage();
  XLS_ASSERT_OK_AND_ASSIGN(Function * f, ParseFunction(R"(
 fn f(w: bits[8], x: bits[8], y: bits[8], z: bits[8]) -> bits[8] {
    or.5: bits[8] = or(w, x)
    or.6: bits[8] = or(y, z)
    ret result: bits[8] = or(or.5, or.6)
 }
  )",
                                                       p.get()));
  ASSERT_THAT(Run(p.get()), IsOkAndHolds(true));
  EXPECT_THAT(f->return_value(), m::Or(m::Param("w"), m::Param("x"),
                                       m::Param("y"), m::Param("z")));
}

TEST_F(ArithSimplificationPassTest, CollapseToNaryOrWithOtherUses) {
  auto p = CreatePackage();
  XLS_ASSERT_OK_AND_ASSIGN(Function * f, ParseFunction(R"(
 fn f(w: bits[8], x: bits[8], y: bits[8], z: bits[8]) -> (bits[8], bits[8]) {
    w_or_x: bits[8] = or(w, x)
    y_or_z: bits[8] = or(y, z)
    tmp0: bits[8] = or(w_or_x, y_or_z)
    ret result: (bits[8], bits[8]) = tuple(w_or_x, tmp0)
 }
  )",
                                                       p.get()));
  ASSERT_THAT(Run(p.get()), IsOkAndHolds(true));
  // `Or(w, x)` should not be folded because it has more than one use.
  EXPECT_THAT(f->return_value(),
              m::Tuple(m::Or(m::Param("w"), m::Param("x")),
                       m::Or(m::Or(), m::Param("y"), m::Param("z"))));
}

TEST_F(ArithSimplificationPassTest, CollapseOneSideToNaryOr) {
  auto p = CreatePackage();
  XLS_ASSERT_OK_AND_ASSIGN(Function * f, ParseFunction(R"(
 fn f(x: bits[8], y: bits[8], z: bits[8]) -> bits[8] {
    or.4: bits[8] = or(y, z)
    ret result: bits[8] = or(x, or.4)
 }
  )",
                                                       p.get()));
  ASSERT_THAT(Run(p.get()), IsOkAndHolds(true));
  EXPECT_THAT(f->return_value(),
              m::Or(m::Param("x"), m::Param("y"), m::Param("z")));
}

TEST_F(ArithSimplificationPassTest, NorWithLiteralZeroOperands) {
  auto p = CreatePackage();
  XLS_ASSERT_OK_AND_ASSIGN(Function * f, ParseFunction(R"(
 fn f(x: bits[8], y: bits[8], z: bits[8]) -> bits[8] {
    literal.1: bits[8] = literal(value=0)
    literal.2: bits[8] = literal(value=0)
    ret result: bits[8] = nor(x, literal.1, y, literal.2, z)
 }
  )",
                                                       p.get()));
  ASSERT_THAT(Run(p.get()), IsOkAndHolds(true));
  EXPECT_THAT(f->return_value(),
              m::Nor(m::Param("x"), m::Param("y"), m::Param("z")));
}

TEST_F(ArithSimplificationPassTest, NandWithLiteralAllOnesOperands) {
  auto p = CreatePackage();
  XLS_ASSERT_OK_AND_ASSIGN(Function * f, ParseFunction(R"(
 fn f(x: bits[8], y: bits[8], z: bits[8]) -> bits[8] {
    literal.1: bits[8] = literal(value=255)
    ret result: bits[8] = nand(x, literal.1, y, z)
 }
  )",
                                                       p.get()));
  ASSERT_THAT(Run(p.get()), IsOkAndHolds(true));
  EXPECT_THAT(f->return_value(),
              m::Nand(m::Param("x"), m::Param("y"), m::Param("z")));
}

TEST_F(ArithSimplificationPassTest, SingleOperandNand) {
  auto p = CreatePackage();
  FunctionBuilder fb(TestName(), p.get());
  fb.AddNaryOp(Op::kNand, {fb.Param("x", p->GetBitsType(32))});
  XLS_ASSERT_OK_AND_ASSIGN(Function * f, fb.Build());
  ASSERT_THAT(Run(p.get()), IsOkAndHolds(true));
  EXPECT_THAT(f->return_value(), m::Not(m::Param("x")));
}

TEST_F(ArithSimplificationPassTest, SingleOperandOr) {
  auto p = CreatePackage();
  FunctionBuilder fb(TestName(), p.get());
  fb.AddNaryOp(Op::kOr, {fb.Param("x", p->GetBitsType(32))});
  XLS_ASSERT_OK_AND_ASSIGN(Function * f, fb.Build());
  ASSERT_THAT(Run(p.get()), IsOkAndHolds(true));
  EXPECT_THAT(f->return_value(), m::Param("x"));
}

TEST_F(ArithSimplificationPassTest, AndWithDuplicateOperands) {
  auto p = CreatePackage();
  XLS_ASSERT_OK_AND_ASSIGN(Function * f, ParseFunction(R"(
    fn id_and(x: bits[32], y: bits[32]) -> bits[32] {
       ret result: bits[32] = and(x, y, y, x, pos=[(0,1,5)])
    }
)",
                                                       p.get()));
  ASSERT_THAT(Run(p.get()), IsOkAndHolds(true));
  EXPECT_THAT(f->return_value(), m::And(m::Param("x"), m::Param("y")));
}

TEST_F(ArithSimplificationPassTest, AndWithSameOperands) {
  auto p = CreatePackage();
  XLS_ASSERT_OK_AND_ASSIGN(Function * f, ParseFunction(R"(
    fn id_and(x: bits[32], y: bits[32]) -> bits[32] {
       ret result: bits[32] = and(x, x, pos=[(0,1,5)])
    }
)",
                                                       p.get()));
  ASSERT_THAT(Run(p.get()), IsOkAndHolds(true));
  EXPECT_THAT(f->return_value(), m::Param("x"));
}

TEST_F(ArithSimplificationPassTest, OrWithSameOperands) {
  auto p = CreatePackage();
  XLS_ASSERT_OK_AND_ASSIGN(Function * f, ParseFunction(R"(
    fn id_or(x: bits[32], y: bits[32]) -> bits[32] {
       ret result: bits[32] = or(x, x, pos=[(0,1,5)])
    }
)",
                                                       p.get()));
  ASSERT_THAT(Run(p.get()), IsOkAndHolds(true));
  EXPECT_THAT(f->return_value(), m::Param("x"));
}

TEST_F(ArithSimplificationPassTest, NandWithDuplicateOperands) {
  auto p = CreatePackage();
  XLS_ASSERT_OK_AND_ASSIGN(Function * f, ParseFunction(R"(
    fn id_or(x: bits[32], y: bits[32]) -> bits[32] {
       ret result: bits[32] = nand(x, x, y, y, x, pos=[(0,1,5)])
    }
)",
                                                       p.get()));
  ASSERT_THAT(Run(p.get()), IsOkAndHolds(true));
  EXPECT_THAT(f->return_value(), m::Nand(m::Param("x"), m::Param("y")));
}

TEST_F(ArithSimplificationPassTest, NandWithSameOperands) {
  auto p = CreatePackage();
  XLS_ASSERT_OK_AND_ASSIGN(Function * f, ParseFunction(R"(
    fn id_or(x: bits[32], y: bits[32]) -> bits[32] {
       ret result: bits[32] = nand(x, x, x, pos=[(0,1,5)])
    }
)",
                                                       p.get()));
  ASSERT_THAT(Run(p.get()), IsOkAndHolds(true));
  EXPECT_THAT(f->return_value(), m::Not(m::Param("x")));
}

TEST_F(ArithSimplificationPassTest, XorWithSameOperandsEven) {
  auto p = CreatePackage();
  XLS_ASSERT_OK_AND_ASSIGN(Function * f, ParseFunction(R"(
    fn id_or(x: bits[32], y: bits[32]) -> bits[32] {
       ret result: bits[32] = xor(x, x, pos=[(0,1,5)])
    }
)",
                                                       p.get()));
  ASSERT_THAT(Run(p.get()), IsOkAndHolds(true));
  EXPECT_THAT(f->return_value(), m::Literal(0));
}

TEST_F(ArithSimplificationPassTest, XorWithSameOperandsOdd) {
  auto p = CreatePackage();
  XLS_ASSERT_OK_AND_ASSIGN(Function * f, ParseFunction(R"(
    fn id_or(x: bits[32], y: bits[32]) -> bits[32] {
       ret result: bits[32] = xor(x, x, x, pos=[(0,1,5)])
    }
)",
                                                       p.get()));
  ASSERT_THAT(Run(p.get()), IsOkAndHolds(true));
  EXPECT_THAT(f->return_value(), m::Param("x"));
}

TEST_F(ArithSimplificationPassTest, AddWithZero) {
  auto p = CreatePackage();
  XLS_ASSERT_OK_AND_ASSIGN(Function * f, ParseFunction(R"(
    fn add_zero(x: bits[32]) -> bits[32] {
      zero: bits[32] = literal(value=0)
      ret result: bits[32] = add(x, zero)
    }
)",
                                                       p.get()));
  ASSERT_THAT(Run(p.get()), IsOkAndHolds(true));
  EXPECT_THAT(f->return_value(), m::Param("x"));
}

TEST_F(ArithSimplificationPassTest, ZeroWidthMulOperand) {
  auto p = CreatePackage();
  XLS_ASSERT_OK_AND_ASSIGN(Function * f, ParseFunction(R"(
    fn id_or(x: bits[0], y: bits[32]) -> bits[32] {
       ret result: bits[32] = smul(x, y, pos=[(0,1,5)])
    }
)",
                                                       p.get()));
  ASSERT_THAT(Run(p.get()), IsOkAndHolds(true));
  EXPECT_THAT(f->return_value(), m::Literal(UBits(0, 32)));
}

TEST_F(ArithSimplificationPassTest, SltZero) {
  auto p = CreatePackage();
  XLS_ASSERT_OK_AND_ASSIGN(Function * f, ParseFunction(R"(
    fn slt_zero(x: bits[32]) -> bits[1] {
      zero: bits[32] = literal(value=0)
      ret result: bits[1] = slt(x, zero)
    }
)",
                                                       p.get()));
  ASSERT_THAT(Run(p.get()), IsOkAndHolds(true));
  EXPECT_THAT(f->return_value(),
              m::BitSlice(m::Param("x"), /*start=*/31, /*width=*/1));
}

TEST_F(ArithSimplificationPassTest, SGeZero) {
  auto p = CreatePackage();
  XLS_ASSERT_OK_AND_ASSIGN(Function * f, ParseFunction(R"(
    fn sge_zero(x: bits[32]) -> bits[1] {
      zero: bits[32] = literal(value=0)
      ret result: bits[1] = sge(x, zero)
    }
)",
                                                       p.get()));
  ASSERT_THAT(Run(p.get()), IsOkAndHolds(true));
  EXPECT_THAT(f->return_value(),
              m::Not(m::BitSlice(m::Param("x"), /*start=*/31, /*width=*/1)));
}

TEST_F(ArithSimplificationPassTest, InvertedComparison) {
  {
    auto p = CreatePackage();
    FunctionBuilder fb(TestName(), p.get());
    Type* u32 = p->GetBitsType(32);
    fb.Not(fb.ULt(fb.Param("x", u32), fb.Param("y", u32)));
    XLS_ASSERT_OK_AND_ASSIGN(Function * f, fb.Build());
    ASSERT_THAT(Run(p.get()), IsOkAndHolds(true));
    EXPECT_THAT(f->return_value(), m::UGe(m::Param("x"), m::Param("y")));
  }
  {
    auto p = CreatePackage();
    FunctionBuilder fb(TestName(), p.get());
    Type* u32 = p->GetBitsType(32);
    fb.Not(fb.SGe(fb.Param("x", u32), fb.Param("y", u32)));
    XLS_ASSERT_OK_AND_ASSIGN(Function * f, fb.Build());
    ASSERT_THAT(Run(p.get()), IsOkAndHolds(true));
    EXPECT_THAT(f->return_value(), m::SLt(m::Param("x"), m::Param("y")));
  }
  {
    auto p = CreatePackage();
    FunctionBuilder fb(TestName(), p.get());
    Type* u32 = p->GetBitsType(32);
    fb.Not(fb.Eq(fb.Param("x", u32), fb.Param("y", u32)));
    XLS_ASSERT_OK_AND_ASSIGN(Function * f, fb.Build());
    ASSERT_THAT(Run(p.get()), IsOkAndHolds(true));
    EXPECT_THAT(f->return_value(), m::Ne(m::Param("x"), m::Param("y")));
  }
}

TEST_F(ArithSimplificationPassTest, ULtMask) {
  auto p = CreatePackage();
  XLS_ASSERT_OK_AND_ASSIGN(Function * f, ParseFunction(R"(
    fn f(x: bits[4]) -> bits[1] {
      literal.1: bits[4] = literal(value=0b0011)
      ret result: bits[1] = ult(x, literal.1)
    }
)",
                                                       p.get()));
  ASSERT_THAT(Run(p.get()), IsOkAndHolds(true));
  EXPECT_THAT(
      f->return_value(),
      m::Nor(m::Or(m::BitSlice(m::Param("x"), /*start=*/2, /*width=*/1),
                   m::BitSlice(m::Param("x"), /*start=*/3, /*width=*/1)),
             m::And(m::BitSlice(m::Param("x"), /*start=*/0, /*width=*/1),
                    m::BitSlice(m::Param("x"), /*start=*/1, /*width=*/1))));
}

TEST_F(ArithSimplificationPassTest, UGtMask) {
  auto p = CreatePackage();
  XLS_ASSERT_OK_AND_ASSIGN(Function * f, ParseFunction(R"(
    fn f(x: bits[4]) -> bits[1] {
      literal.1: bits[4] = literal(value=0b0011)
      ret result: bits[1] = ugt(x, literal.1)
    }
)",
                                                       p.get()));
  ASSERT_THAT(Run(p.get()), IsOkAndHolds(true));
  EXPECT_THAT(f->return_value(),
              m::Or(m::BitSlice(m::Param("x"), /*start=*/2, /*width=*/1),
                    m::BitSlice(m::Param("x"), /*start=*/3, /*width=*/1)));
}

TEST_F(ArithSimplificationPassTest, UGtMaskAllOnes) {
  auto p = CreatePackage();
  XLS_ASSERT_OK_AND_ASSIGN(Function * f, ParseFunction(R"(
    fn f(x: bits[4]) -> bits[1] {
      literal.1: bits[4] = literal(value=0b1111)
      ret result: bits[1] = ugt(x, literal.1)
    }
)",
                                                       p.get()));
  ASSERT_THAT(Run(p.get()), IsOkAndHolds(true));
  EXPECT_THAT(f->return_value(), m::Literal(0));
}

TEST_F(ArithSimplificationPassTest, ShiftByConcatWithZeroValue) {
  auto p = CreatePackage();
  FunctionBuilder fb(TestName(), p.get());
  fb.Shll(fb.Param("x", p->GetBitsType(32)),
          fb.Concat({fb.Literal(Value(UBits(0, 24))),
                     fb.Param("y", p->GetBitsType(8))}));
  XLS_ASSERT_OK_AND_ASSIGN(Function * f, fb.Build());
  ASSERT_THAT(Run(p.get()), IsOkAndHolds(true));
  EXPECT_THAT(f->return_value(), m::Shll(m::Param("x"), m::Param("y")));
}

TEST_F(ArithSimplificationPassTest, ShiftByConcatWithZeroValueAndOthers) {
  auto p = CreatePackage();
  FunctionBuilder fb(TestName(), p.get());
  fb.Shll(fb.Param("x", p->GetBitsType(32)),
          fb.Concat({fb.Literal(Value(UBits(0, 16))),
                     fb.Param("y", p->GetBitsType(8)),
                     fb.Param("z", p->GetBitsType(8))}));
  XLS_ASSERT_OK_AND_ASSIGN(Function * f, fb.Build());
  ASSERT_THAT(Run(p.get()), IsOkAndHolds(true));
  EXPECT_THAT(f->return_value(),
              m::Shll(m::Param("x"), m::Concat(m::Param("y"), m::Param("z"))));
}

TEST_F(ArithSimplificationPassTest, GuardedShiftOperation) {
  // Test that a shift amount clamped to the shift's width is removed.
  auto p = CreatePackage();
  FunctionBuilder fb(TestName(), p.get());
  BValue x = fb.Param("x", p->GetBitsType(100));
  BValue amt = fb.Param("amt", p->GetBitsType(32));
  BValue limit = fb.Literal(Value(UBits(100, 32)));
  BValue clamped_amt =
      fb.Select(fb.UGt(amt, limit), /*on_true=*/limit, /*on_false=*/amt);
  fb.Shll(x, clamped_amt);
  XLS_ASSERT_OK_AND_ASSIGN(Function * f, fb.Build());
  ASSERT_THAT(Run(p.get()), IsOkAndHolds(true));
  EXPECT_THAT(f->return_value(), m::Shll(m::Param("x"), m::Param("amt")));
}

TEST_F(ArithSimplificationPassTest, GuardedShiftOperationWithDefault) {
  // Test that a shift amount clamped to the shift's width is removed.
  // Uses the default element of a sel.
  auto p = CreatePackage();
  FunctionBuilder fb(TestName(), p.get());
  BValue x = fb.Param("x", p->GetBitsType(100));
  BValue amt = fb.Param("amt", p->GetBitsType(32));
  BValue limit = fb.Literal(Value(UBits(100, 32)));
  BValue clamped_amt =
      fb.Select(fb.UGt(amt, limit), /*cases=*/{limit}, /*default_value=*/amt);
  fb.Shll(x, clamped_amt);
  XLS_ASSERT_OK_AND_ASSIGN(Function * f, fb.Build());
  ASSERT_THAT(Run(p.get()), IsOkAndHolds(true));
  EXPECT_THAT(f->return_value(), m::Shll(m::Param("x"), m::Param("amt")));
}

TEST_F(ArithSimplificationPassTest, GuardedArithShiftOperation) {
  // Test that a shift amount clamped to the shift's width is removed for
  // arithmetic shift right.
  auto p = CreatePackage();
  FunctionBuilder fb(TestName(), p.get());
  BValue x = fb.Param("x", p->GetBitsType(100));
  BValue amt = fb.Param("amt", p->GetBitsType(32));
  BValue limit = fb.Literal(Value(UBits(100, 32)));
  BValue clamped_amt =
      fb.Select(fb.UGt(amt, limit), /*on_true=*/limit, /*on_false=*/amt);
  fb.Shra(x, clamped_amt);
  XLS_ASSERT_OK_AND_ASSIGN(Function * f, fb.Build());
  ASSERT_THAT(Run(p.get()), IsOkAndHolds(true));
  EXPECT_THAT(f->return_value(), m::Shra(m::Param("x"), m::Param("amt")));
}

TEST_F(ArithSimplificationPassTest, GuardedShiftOperationHighLimit) {
  auto p = CreatePackage();
  FunctionBuilder fb(TestName(), p.get());
  BValue x = fb.Param("x", p->GetBitsType(100));
  BValue amt = fb.Param("amt", p->GetBitsType(32));
  // Set a clamp limit higher than the width of the shift, the end result is the
  // same as if the limit were the shift amount.
  BValue limit = fb.Literal(Value(UBits(1234, 32)));
  BValue clamped_amt =
      fb.Select(fb.UGt(amt, limit), /*on_true=*/limit, /*on_false=*/amt);
  fb.Shrl(x, clamped_amt);
  XLS_ASSERT_OK_AND_ASSIGN(Function * f, fb.Build());
  ASSERT_THAT(Run(p.get()), IsOkAndHolds(true));
  EXPECT_THAT(f->return_value(), m::Shrl(m::Param("x"), m::Param("amt")));
}

TEST_F(ArithSimplificationPassTest, GuardedShiftOperationLowLimit) {
  // Test that a shift amount clamped to a value less that the shift's width is
  // not removed.
  auto p = CreatePackage();
  FunctionBuilder fb(TestName(), p.get());
  BValue x = fb.Param("x", p->GetBitsType(100));
  BValue amt = fb.Param("amt", p->GetBitsType(32));
  BValue limit = fb.Literal(Value(UBits(99, 32)));
  BValue clamped_amt =
      fb.Select(fb.UGt(amt, limit), /*on_true=*/limit, /*on_false=*/amt);
  fb.Shll(x, clamped_amt);
  XLS_ASSERT_OK_AND_ASSIGN(Function * f, fb.Build());
  ASSERT_THAT(Run(p.get()), IsOkAndHolds(false));
  EXPECT_THAT(f->return_value(), m::Shll(m::Param("x"), m::Select()));
}

TEST_F(ArithSimplificationPassTest, EqAggregateType) {
  auto p = CreatePackage();
  FunctionBuilder fb(TestName(), p.get());
  Type* u32 = p->GetBitsType(32);
  BValue x = fb.Param("x", p->GetTupleType({u32, u32}));
  BValue y = fb.Param("y", p->GetTupleType({u32, u32}));
  BValue x_eq_x = fb.Eq(x, x);
  BValue x_eq_y = fb.Eq(x, y);
  fb.Tuple({x_eq_x, x_eq_y});
  XLS_ASSERT_OK_AND_ASSIGN(Function * f, fb.Build());

  ASSERT_THAT(Run(p.get()), IsOkAndHolds(true));
  EXPECT_THAT(f->return_value(),
              m::Tuple(m::Literal(1), m::Eq(m::Param("x"), m::Param("y"))));
}

TEST_F(ArithSimplificationPassTest, NeAggregateType) {
  auto p = CreatePackage();
  FunctionBuilder fb(TestName(), p.get());
  Type* u32 = p->GetBitsType(32);
  BValue x = fb.Param("x", p->GetTupleType({u32, u32}));
  BValue y = fb.Param("y", p->GetTupleType({u32, u32}));
  BValue x_eq_x = fb.Ne(x, x);
  BValue x_eq_y = fb.Ne(x, y);
  fb.Tuple({x_eq_x, x_eq_y});
  XLS_ASSERT_OK_AND_ASSIGN(Function * f, fb.Build());

  ASSERT_THAT(Run(p.get()), IsOkAndHolds(true));
  EXPECT_THAT(f->return_value(),
              m::Tuple(m::Literal(0), m::Ne(m::Param("x"), m::Param("y"))));
}

TEST_F(ArithSimplificationPassTest, EqNeZeroWidthTypes) {
  auto p = CreatePackage();
  FunctionBuilder fb(TestName(), p.get());
  BValue x = fb.Param("x", p->GetTupleType({}));
  BValue y = fb.Param("y", p->GetTupleType({}));
  BValue x_eq_y = fb.Eq(x, y);
  BValue x_ne_y = fb.Ne(x, y);
  fb.Tuple({x_eq_y, x_ne_y});
  XLS_ASSERT_OK_AND_ASSIGN(Function * f, fb.Build());

  ASSERT_THAT(Run(p.get()), IsOkAndHolds(true));
  EXPECT_THAT(f->return_value(), m::Tuple(m::Literal(1), m::Literal(0)));
}

}  // namespace
}  // namespace xls
