; Tests if the intrinsic llvm.intel.wholeprogramsafe was converted correctly
; into false since the optimization levele is O0. Also, the intrinsic
; llvm.intel.wholeprogramsafe should be removed.

; RUN: opt < %s -xmain-opt-level=0 -intel-fold-wp-intrinsic -whole-program-assume -S 2>&1 | FileCheck %s

; RUN: opt < %s -xmain-opt-level=0 -passes='module(intel-fold-wp-intrinsic)' -whole-program-assume -S 2>&1 | FileCheck %s

declare i1 @llvm.intel.wholeprogramsafe()

define i32 @main() {
entry:
  %x = alloca i32, align 4
  store i32 0, i32* %x, align 4

  %whprsafe = call i1 @llvm.intel.wholeprogramsafe()
  br i1 %whprsafe, label %if.whpr, label %if.nowhpr

if.whpr:
  store i32 1, i32* %x
  br label %if.end

if.nowhpr:
  store i32 2, i32* %x
  br label %if.end

if.end:
  %retval = load i32, i32* %x, align 4
  ret i32 %retval
}

; CHECK:       %x = alloca i32, align 4
; CHECK-NEXT:  store i32 0, i32* %x, align 4
; CHECK-NEXT:  br i1 false, label %if.whpr, label %if.nowhpr

; CHECK-NOT: @llvm.intel.wholeprogramsafe()
