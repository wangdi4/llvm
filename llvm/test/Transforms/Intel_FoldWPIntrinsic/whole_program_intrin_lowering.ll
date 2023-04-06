; REQUIRES: intel_feature_sw_dtrans

; Tests if the intrinsic llvm.intel.wholeprogramsafe was lowered correctly
; into false since the whole program analysis didn't run. Also, the intrinsic
; llvm.intel.wholeprogramsafe should be removed.

; Opt driver forces this pass to go through the legacy pass manager
; For more details, check function 'shouldForceLegacyPM' in opt.cpp
; RUN: opt < %s -pre-isel-intrinsic-lowering -S 2>&1 | FileCheck %s

declare i1 @llvm.intel.wholeprogramsafe()

define i32 @main() {
entry:
  %x = alloca i32, align 4
  store i32 0, ptr %x, align 4

  %whprsafe = call i1 @llvm.intel.wholeprogramsafe()
  br i1 %whprsafe, label %if.whpr, label %if.nowhpr

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

; CHECK:       %x = alloca i32, align 4
; CHECK-NEXT:  store i32 0, ptr %x, align 4
; CHECK-NEXT:  br i1 false, label %if.whpr, label %if.nowhpr

; CHECK-NOT: @llvm.intel.wholeprogramsafe()
