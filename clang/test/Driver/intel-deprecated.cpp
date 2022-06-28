/// Test for any deprecated options
// RUN: %clangxx -m32 %s -### 2>&1 | FileCheck %s -DOPTION=-m32
// RUN: %clang_cl -Qm32 %s -### 2>&1 | FileCheck %s -DOPTION=-Qm32
// CHECK: option '[[OPTION]]' is deprecated and will be removed in a future release
