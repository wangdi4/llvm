; INTEL_FEATURE_SW_DTRANS
; REQUIRES: intel_feature_sw_dtrans

; Tests if the intrinsic llvm.public.type.test is omitted
; without whole program safe.


; RUN: opt < %s -passes='module(intel-fold-wp-intrinsic)' -S 2>&1 | FileCheck %s

declare i1 @llvm.intel.wholeprogramsafe()

define i32 @main(i8* %vtable) {
entry:
  %whprsafe = call i1 @llvm.intel.wholeprogramsafe()
  br i1 %whprsafe, label %if.whpr, label %if.end

if.whpr:
  %a = call i1 @llvm.public.type.test(i8* %vtable, metadata !"_ZTS1A")
  call void @llvm.assume(i1 %a)
  br label %if.end

if.end:
  ret i32 0
}

declare void @llvm.assume(i1)
declare i1 @llvm.public.type.test(i8*, metadata)

; CHECK: br i1 false, label %if.whpr, label %if.end
; CHECK-NOT: @llvm.intel.wholeprogramsafe()
; CHECK-NOT: @llvm.type.test(i8* %vtable, metadata !"_ZTS1A")
; CHECK-NOT: @llvm.public.type.test(i8* %vtable, metadata !"_ZTS1A")
; CHECK: call void @llvm.assume(i1 true)
; CHECK-NEXT: br label %if.end

; end INTEL_FEATURE_SW_DTRANS