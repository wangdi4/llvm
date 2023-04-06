; REQUIRES: asserts, intel_feature_sw_dtrans

; This test checks that the debug trace is printed correctly if the intrinsic
; llvm.intel.wholeprogramsafe intrinsic is not in the IR.

; RUN: opt < %s -passes='module(intel-fold-wp-intrinsic)' -debug-only=intel-fold-wp-intrinsic -S 2>&1 | FileCheck %s

define i32 @main(i1 %val) {
entry:
  %x = alloca i32, align 4
  store i32 0, ptr %x, align 4

  br i1 %val, label %if.whpr, label %if.nowhpr

if.whpr:
  store i32 1, ptr %x
  br label %if.end

if.nowhpr:
  store i32 2, ptr %x
  br label %if.end

if.end:
  %retval = load i32, ptr %x, align 4
  ret i32 %retval
}

; CHECK: Intrinsic llvm.intel.wholeprogramsafe not found
