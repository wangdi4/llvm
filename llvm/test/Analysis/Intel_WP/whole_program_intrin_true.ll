; Tests if the intrinsic llvm.intel.wholeprogramsafe was converted correctly
; into true since there is whole program safe. Also, the intrinsic
; llvm.intel.wholeprogramsafe should be removed.


; RUN: opt < %s -wholeprogramanalysis -whole-program-assume -S 2>&1 | FileCheck %s
; RUN: opt < %s -passes='require<wholeprogram>' -whole-program-assume -S 2>&1 | FileCheck %s

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
; CHECK-NEXT:  br i1 true, label %if.whpr, label %if.nowhpr

; CHECK-NOT: @llvm.intel.wholeprogramsafe()
