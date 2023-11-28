// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//

// This is a testcase generator to generate min/max instruction lit tests
// covering every combination of min/max pattern and foldable comparison. To
// regnerate tests, do:
//
// $ icpx -std=c++11 genlits.cpp -o genlits && ./genlits

#include <iostream>
#include <fstream>
#include <string>

using namespace std;

string get_llvm_type(char opclass, int bits) {
  if (opclass == 'f') {
    return bits == 64 ? "double" : "float";
  }
  return "i" + to_string(bits);
}

struct TestFile {
  const char* op;
  const char opclass;
  const int bits;
  const string llvm_type;
  ofstream file;
  TestFile(const char* op_in, char opclass_in, int bits_in)
    : op{op_in}, opclass{opclass_in}, bits{bits_in},
      llvm_type{get_llvm_type(opclass, bits)},
      file{string{op} + opclass + to_string(bits) + ".ll"} {}
};

void gen_header(ostream& out) {
  out << R"header(; RUN: llc -mtriple=csa < %s | FileCheck --implicit-check-not cmp --implicit-check-not merge %s
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

)header";
}

void gen_footer(TestFile& test) {
  if (test.opclass == 'f') {
    test.file << "declare " << test.llvm_type
      << " @llvm." << test.op << "num.f" << test.bits << "("
      << test.llvm_type << " %a, " << test.llvm_type << " %b)\n";
  }
}

void gen_cmp_minmax_test(TestFile& test, const char* cmp, bool reverse_cmp) {
  const string test_name = string{"test"} + test.opclass + to_string(test.bits)
    + cmp + (reverse_cmp ? 'r' : 's');
  test.file << "define " << test.llvm_type << " @" << test_name;
  test.file << "(" << test.llvm_type << " %a, " << test.llvm_type << " %b) {\n";
  test.file << "; CHECK-LABEL: " << test_name << "\n";
  test.file << "; CHECK: " << ".result .lic .i" << test.bits
    << " %[[RES:[a-z0-9_.]+]]" << "\n";
  test.file << "; CHECK: " << test.op << test.opclass << test.bits
      << " %[[RES]], %ign" << "\n";
  if (test.opclass == 'f') {
    test.file << "  %cmp = fcmp o" << cmp << " " << test.llvm_type
      << (reverse_cmp ? " %b, %a\n" : " %a, %b\n");
  } else {
    test.file << "  %cmp = icmp " << test.opclass << cmp << " "
      << test.llvm_type << (reverse_cmp ? " %b, %a\n" : " %a, %b\n");
  }
  test.file << "  %res = select i1 %cmp, "
    << test.llvm_type << " %a, " << test.llvm_type << " %b\n";
  test.file << "  ret " << test.llvm_type << " %res\n";
  test.file << "}\n\n";
}

void gen_cmp_minmax_cmp_test(
  TestFile& test, const char* cmp, bool reverse_cmp, const char* cmp2
) {
  const string test_name = string{"test"} + test.opclass + to_string(test.bits)
    + cmp + (reverse_cmp ? 'r' : 's') + cmp2;
  test.file << "define {" << test.llvm_type << ", i1} @" << test_name;
  test.file << "(" << test.llvm_type << " %a, " << test.llvm_type << " %b) {\n";
  test.file << "; CHECK-LABEL: " << test_name << "\n";
  test.file << "; CHECK: " << ".result .lic .i" << test.bits
    << " %[[RES:[a-z0-9_.]+]]" << "\n";
  test.file << "; CHECK: " << test.op << test.opclass << test.bits
    << " %[[RES]], [[CMP:[^,]+]]" << "\n";
  if (cmp2[1] == 'e') {
    test.file << "; CHECK: not1 [[NOT:[^,]+]], [[CMP]]\n";
  }
  if (test.opclass == 'f') {
    test.file << "  %cmp = fcmp o" << cmp << " " << test.llvm_type
      << (reverse_cmp ? " %b, %a\n" : " %a, %b\n");
  } else {
    test.file << "  %cmp = icmp " << test.opclass << cmp << " "
      << test.llvm_type << (reverse_cmp ? " %b, %a\n" : " %a, %b\n");
  }
  test.file << "  %res = select i1 %cmp, "
    << test.llvm_type << " %a, " << test.llvm_type << " %b\n";
  if (test.opclass == 'f') {
    test.file << "  %cmp2 = fcmp o" << cmp2 << " " << test.llvm_type
      << " %a, %b\n";
  } else {
    test.file << "  %cmp2 = icmp " << test.opclass << cmp2 << " "
      << test.llvm_type << " %a, %b\n";
  }
  test.file << "  %ret0 = insertvalue {" << test.llvm_type << ", i1} undef, "
    << test.llvm_type << " %res, 0\n";
  test.file << "  %ret1 = insertvalue {" << test.llvm_type
    << ", i1} %ret0, i1 %cmp2, 1\n";
  test.file << "  ret {" << test.llvm_type << ", i1} %ret1\n";
  test.file << "}\n\n";
}

void gen_cmp_minmax_tests(
  TestFile& test, const char* cmp, bool reverse_cmp
) {
  gen_cmp_minmax_test(test, cmp, reverse_cmp);
  gen_cmp_minmax_cmp_test(test, cmp, reverse_cmp, "lt");
  gen_cmp_minmax_cmp_test(test, cmp, reverse_cmp, "le");
  gen_cmp_minmax_cmp_test(test, cmp, reverse_cmp, "gt");
  gen_cmp_minmax_cmp_test(test, cmp, reverse_cmp, "ge");
}

void gen_cmp_minmax_tests_both_dirs(
  TestFile& min, TestFile& max, const char* cmp
) {
  const bool same_max = (*cmp == 'g');
  gen_cmp_minmax_tests(same_max ? max : min, cmp, false);
  gen_cmp_minmax_tests(same_max ? min : max, cmp,  true);
}

void gen_cmp_minmax_tests_all_cmps(TestFile& min, TestFile& max) {
  gen_cmp_minmax_tests_both_dirs(min, max, "lt");
  gen_cmp_minmax_tests_both_dirs(min, max, "le");
  gen_cmp_minmax_tests_both_dirs(min, max, "gt");
  gen_cmp_minmax_tests_both_dirs(min, max, "ge");
}

void gen_fminmax_test(TestFile& test) {
  const string test_name = string{"test"} + test.opclass + to_string(test.bits)
    + 'f' + test.op;
  test.file << "define " << test.llvm_type << " @" << test_name;
  test.file << "(" << test.llvm_type << " %a, " << test.llvm_type << " %b) {\n";
  test.file << "; CHECK-LABEL: " << test_name << "\n";
  test.file << "; CHECK: " << ".result .lic .i" << test.bits
    << " %[[RES:[a-z0-9_.]+]]" << "\n";
  test.file << "; CHECK: " << test.op << test.opclass << test.bits
    << " %[[RES]], %ign" << "\n";
  test.file << "  %res = tail call " << test.llvm_type
    << " @llvm." << test.op << "num.f" << test.bits << "("
    << test.llvm_type << " %a, " << test.llvm_type << " %b)\n";
  test.file << "  ret " << test.llvm_type << " %res\n";
  test.file << "}\n\n";
}

void gen_fminmax_cmp_test(TestFile& test, const char* cmp) {
  const string test_name = string{"test"} + test.opclass + to_string(test.bits)
    + 'f' + test.op + cmp;
  test.file << "define {" << test.llvm_type << ", i1} @" << test_name;
  test.file << "(" << test.llvm_type << " %a, " << test.llvm_type << " %b) {\n";
  test.file << "; CHECK-LABEL: " << test_name << "\n";
  test.file << "; CHECK: " << ".result .lic .i" << test.bits
    << " %[[RES:[a-z0-9_.]+]]" << "\n";
  test.file << "; CHECK: " << test.op << test.opclass << test.bits
    << " %[[RES]], [[CMP:[^,]+]]" << "\n";
  if (cmp[1] == 'e') {
    test.file << "; CHECK: not1 [[NOT:[^,]+]], [[CMP]]\n";
  }
  test.file << "  %res = tail call " << test.llvm_type
    << " @llvm." << test.op << "num.f" << test.bits << "("
    << test.llvm_type << " %a, " << test.llvm_type << " %b)\n";
  test.file << "  %cmp = fcmp o" << cmp << " " << test.llvm_type << " %a, %b\n";
  test.file << "  %ret0 = insertvalue {" << test.llvm_type << ", i1} undef, "
    << test.llvm_type << " %res, 0\n";
  test.file << "  %ret1 = insertvalue {" << test.llvm_type
    << ", i1} %ret0, i1 %cmp, 1\n";
  test.file << "  ret {" << test.llvm_type << ", i1} %ret1\n";
  test.file << "}\n\n";
}

void gen_fminmax_tests(TestFile& test) {
  gen_fminmax_test(test);
  gen_fminmax_cmp_test(test, "lt");
  gen_fminmax_cmp_test(test, "le");
  gen_fminmax_cmp_test(test, "gt");
  gen_fminmax_cmp_test(test, "ge");
}

void gen_minmax_tests(char opclass, int bits) {
  TestFile min {"min", opclass, bits};
  gen_header(min.file);
  TestFile max {"max", opclass, bits};
  gen_header(max.file);
  gen_cmp_minmax_tests_all_cmps(min, max);
  if (opclass == 'f') {
    gen_fminmax_tests(min);
    gen_fminmax_tests(max);
  }
  gen_footer(min);
  gen_footer(max);
}

void gen_minmax_tests_all_opclasses(int bits) {
  gen_minmax_tests('s', bits);
  gen_minmax_tests('u', bits);
  if (bits == 32 or bits == 64) gen_minmax_tests('f', bits);
}

void gen_minmax_tests_all_bits() {
  gen_minmax_tests_all_opclasses(8);
  gen_minmax_tests_all_opclasses(16);
  gen_minmax_tests_all_opclasses(32);
  gen_minmax_tests_all_opclasses(64);
}

int main() {
  gen_minmax_tests_all_bits();
}
