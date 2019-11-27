; Tests if the intrinsic llvm.intel.wholeprogramsafe was converted correctly
; into false since there is not whole program safe. Also, the intrinsic
; llvm.intel.wholeprogramsafe should be removed and the wrapper should be removed.

; RUN: opt < %s -intel-fold-wp-intrinsic -simplifycfg -S 2>&1 | FileCheck %s
; RUN: opt < %s -passes='module(intel-fold-wp-intrinsic),function(simplify-cfg)' -S 2>&1 | FileCheck %s

declare i1 @llvm.intel.wholeprogramsafe()

define i32 @main() {
entry:
  %x = alloca i32, align 4
  store i32 0, i32* %x, align 4

  %whprsafe = call i1 @llvm.intel.wholeprogramsafe()
  br i1 %whprsafe, label %if.whpr, label %if.end

if.whpr:
  %y = alloca i32, align 4
  store i32 1, i32* %y
  br label %if.end

if.end:
  %retval = load i32, i32* %x, align 4
  ret i32 %retval
}

; CHECK:       %x = alloca i32, align 4
; CHECK-NEXT:  store i32 0, i32* %x, align 4
; CHECK-NEXT:  %retval = load i32, i32* %x, align 4
; CHECK-NEXT:  ret i32 %retval

; CHECK-NOT: @llvm.intel.wholeprogramsafe()
; CHECK-NOT: br i1 %whprsafe, label %if.whpr, label %if.end
; CHECK-NOT: br i1 false, label %if.whpr, label %if.end
; CHECK-NOT: %y = alloca i32, align 4
; CHECK-NOT: store i32 1, i32* %y
; CHECK-NOT: br label %if.end

