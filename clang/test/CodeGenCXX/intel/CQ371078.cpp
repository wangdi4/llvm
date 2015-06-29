// RUN: %clang_cc1 -fintel-compatibility -triple x86_64-unknown-linux-gnu -emit-llvm -o - -std=c++11 -o - %s | FileCheck --check-prefix LINUX %s
// RUN: %clang_cc1 -fintel-compatibility -triple x86_64-windows-msvc -emit-llvm -o - -std=c++11 -o - %s | FileCheck --check-prefix WIN_MSVC %s

// LINUX: @_ZN14SettingsStruct6NumberE = linkonce_odr constant i32 23,
// WIN_MSVC: @"\01?Number@SettingsStruct@@2HB" = linkonce_odr constant i32 23,
struct SettingsStruct {
  static constexpr int Number = 23;
};

int foo(const int &arg) { return arg; }

int main() { return foo(SettingsStruct::Number); }
