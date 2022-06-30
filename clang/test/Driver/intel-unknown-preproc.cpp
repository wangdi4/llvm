/// Test that checks unknown source extensions with preprocessing enabled.
/// When this combination is used, preprocessing should occur instead of
/// linking.
// RUN: %clangxx --intel -E %S/Inputs/intel-unknown-preproc.cx -### 2>&1 | FileCheck %s
// CHECK: "-E"
