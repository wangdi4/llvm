// RUN: %clang_cc1 -std=c++11 -fintel-compatibility -triple x86_64-pc-win32 -O0 -fexceptions -fcxx-exceptions %s -emit-llvm -o - | FileCheck --check-prefix=CHECK %s
// RUN: %clang_cc1 -std=c++11 -fintel-compatibility-enable=StringCharStarCatchable -triple x86_64-pc-win32 -O0 -fexceptions -fcxx-exceptions %s -emit-llvm -o - | FileCheck --check-prefix=CHECK_INTEL %s
// RUN: %clang_cc1 -std=c++11 -fintel-compatibility -fintel-compatibility-disable=StringCharStarCatchable -triple x86_64-pc-win32 -O0 -fexceptions -fcxx-exceptions %s -emit-llvm -o - | FileCheck %s
// RUN: %clang_cc1 -std=c++11 -triple x86_64-pc-win32 -O0 -fexceptions -fcxx-exceptions %s -emit-llvm -o - | FileCheck %s

// Check that string literals are catchable by pointer-to-non-const handlers on
// Windows targets with -fintel-compatibility option.
// Test cases below check the type of thrown objects.
// Catchpads code that compiler emits for Microsoft target didn't change and
// depends only on handler itself, so we test just throws.

void TestCharString() {
  try { throw "abcd"; }
  catch (char *s) {}
  // CHECK-LABEL: TestCharString
  // CHECK_INTEL: invoke void @_CxxThrowException({{.*}} @_TI2PEAD)
  // CHECK: invoke void @_CxxThrowException({{.*}} @_TIC2PEAD)
}

void TestCharStringParen() {
  try { throw ("abc"); }
  catch (char *s) {}
  // CHECK-LABEL: TestCharStringParen
  // CHECK_INTEL: invoke void @_CxxThrowException({{.*}} @_TI2PEAD)
  // CHECK: invoke void @_CxxThrowException({{.*}} @_TIC2PEAD)
}

void TestWCharString() {
  try { throw L"abc"; }
  catch (wchar_t *s) {}
  // CHECK-LABEL: TestWCharString
  // CHECK_INTEL: invoke void @_CxxThrowException({{.*}} @_TI2PEA_W)
  // CHECK: invoke void @_CxxThrowException({{.*}} @_TIC2PEA_W)
}

void TestWCharStringParen() {
  try { throw (L"abc"); }
  catch (wchar_t *s) {}
  // CHECK-LABEL: TestWCharStringParen
  // CHECK_INTEL: invoke void @_CxxThrowException({{.*}} @_TI2PEA_W)
  // CHECK: invoke void @_CxxThrowException({{.*}} @_TIC2PEA_W)
}

void TestUTF8String() {
  try { throw u8"abc"; }
  catch (char *s) {}
  // CHECK-LABEL: TestUTF8String
  // CHECK_INTEL: invoke void @_CxxThrowException({{.*}} @_TI2PEAD)
  // CHECK: invoke void @_CxxThrowException({{.*}} @_TIC2PEAD)
}

void TestUTF8StringParen() {
  try { throw (u8"abc"); }
  catch (char *s) {}
  // CHECK-LABEL: TestUTF8StringParen
  // CHECK_INTEL: invoke void @_CxxThrowException({{.*}} @_TI2PEAD)
  // CHECK: invoke void @_CxxThrowException({{.*}} @_TIC2PEAD)
}

void TestUTF16String() {
  try { throw u"abc"; }
  catch (char16_t *s) {}
  // CHECK-LABEL: TestUTF16String
  // CHECK_INTEL: invoke void @_CxxThrowException({{.*}} @_TI2PEA_S)
  // CHECK: invoke void @_CxxThrowException({{.*}} @_TIC2PEA_S)
}

void TestUTF16StringParen() {
  try { throw (u"abc"); }
  catch (char16_t *s) {}
  // CHECK-LABEL: TestUTF16StringParen
  // CHECK_INTEL: invoke void @_CxxThrowException({{.*}} @_TI2PEA_S)
  // CHECK: invoke void @_CxxThrowException({{.*}} @_TIC2PEA_S)
}

void TestUTF32String() {
  try { throw U"abc"; }
  catch (char32_t *s) {}
  // CHECK-LABEL: TestUTF32String
  // CHECK_INTEL: invoke void @_CxxThrowException({{.*}} @_TI2PEA_U)
  // CHECK: invoke void @_CxxThrowException({{.*}} @_TIC2PEA_U)
}

void TestUTF32StringParen() {
  try { throw (U"abc"); }
  catch (char32_t *s) {}
  // CHECK-LABEL: TestUTF32StringParen
  // CHECK_INTEL: invoke void @_CxxThrowException({{.*}} @_TI2PEA_U)
  // CHECK: invoke void @_CxxThrowException({{.*}} @_TIC2PEA_U)
}

void Run() {
  TestCharString();
  TestCharStringParen();
  TestWCharString();
  TestWCharStringParen();
  TestUTF8String();
  TestUTF8StringParen();
  TestUTF16String();
  TestUTF16StringParen();
  TestUTF32String();
  TestUTF32StringParen();
}
