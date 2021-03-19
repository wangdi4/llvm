// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -fintel-compatibility \
// RUN:            -fintel-compatibility-enable=DisplayFullFilePath \
// RUN:            -disable-llvm-passes -emit-llvm %s -o - | \
// RUN:            FileCheck %s --check-prefix=CHECK-LIN-FILENAME

// RUN: %clang_cc1 -triple x86_64-windows-msvc -fintel-compatibility \
// RUN:            -fintel-compatibility-enable=DisplayFullFilePath \
// RUN:            -disable-llvm-passes -emit-llvm %s -o - | \
// RUN:            FileCheck %s --check-prefix=CHECK-WIN-FILENAME

// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -fintel-compatibility \
// RUN:            -fintel-compatibility-disable=DisplayFullFilePath \
// RUN:            -disable-llvm-passes -emit-llvm %s -o - | \
// RUN:            FileCheck %s --check-prefix=CHECK-LIN-PATH

// RUN: %clang_cc1 -triple x86_64-windows-msvc -fintel-compatibility \
// RUN:            -fintel-compatibility-disable=DisplayFullFilePath \
// RUN:            -disable-llvm-passes -emit-llvm %s -o - | \
// RUN:            FileCheck %s --check-prefix=CHECK-WIN-PATH

// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -fintel-compatibility \
// RUN:            -disable-llvm-passes -emit-llvm %s -o - | \
// RUN:            FileCheck %s --check-prefix=CHECK-LIN

// RUN: %clang_cc1 -triple x86_64-windows-msvc -fintel-compatibility \
// RUN:            -disable-llvm-passes -emit-llvm %s -o - | \
// RUN:            FileCheck %s --check-prefix=CHECK-WIN

void takes_const_char_ptr(const char * FN = __builtin_FILE());

int main() {
   takes_const_char_ptr();
}

// CHECK-WIN-FILENAME: linkonce_odr dso_local unnamed_addr constant {{.*}} c"display-filename-or-path.cpp\00", comdat, align 1
// CHECK-WIN-PATH: linkonce_odr dso_local unnamed_addr constant {{.*}} c"{{.+}}display-filename-or-path.cpp\00", comdat, align 1
// CHECK-WIN: linkonce_odr dso_local unnamed_addr constant {{.*}} c"display-filename-or-path.cpp\00", comdat, align 1

// CHECK-LIN-PATH: @.str = private unnamed_addr constant {{.*}} c"{{.+}}display-filename-or-path.cpp\00", align 1
// CHECK-LIN: @.str = private unnamed_addr constant {{.*}} c"display-filename-or-path.cpp\00", align 1
// CHECK-LIN-FILENAME: @.str = private unnamed_addr constant {{.*}} c"display-filename-or-path.cpp\00", align 1
