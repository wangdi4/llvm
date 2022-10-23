; RUN: opt -opaque-pointers -passes='cgscc(inline)' -inline-report=0xe807 -S < %s  2>&1 | FileCheck --check-prefix=CHECK-NEW %s
; RUN: opt -opaque-pointers -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='cgscc(inline)' -inline-report=0xe886 -S | opt -passes='inlinereportemitter' -inline-report=0xe886 -S 2>&1 | FileCheck --check-prefix=CHECK-MOLD %s

; CHECK-OLD: COMPILE FUNC: bf_musttail
; CHECK-OLD: llvm.icall.branch.funnel {{.*}}Callee is intrinsic{{.*}}

; CHECK-OLD: COMPILE FUNC: fn_musttail
; CHECK-OLD: bf_musttail {{.*}}Callee calls branch funnel{{.*}}

; CHECK-OLD: COMPILE FUNC: bf_musttail_always
; CHECK-OLD: llvm.icall.branch.funnel {{.*}}Callee is intrinsic{{.*}}

; CHECK-OLD: COMPILE FUNC: fn_musttail_always
; CHECK-OLD: bf_musttail_always {{.*}}Callee calls branch funnel{{.*}}

; CHECK-NEW: COMPILE FUNC: bf_musttail
; CHECK-NEW: llvm.icall.branch.funnel {{.*}}Callee is intrinsic{{.*}}

; CHECK-NEW: COMPILE FUNC: fn_musttail
; CHECK-NEW: bf_musttail {{.*}}Callee calls branch funnel{{.*}}

; CHECK-NEW: COMPILE FUNC: bf_musttail_always
; CHECK-NEW: llvm.icall.branch.funnel {{.*}}Callee is intrinsic{{.*}}

; CHECK-NEW: COMPILE FUNC: fn_musttail_always
; CHECK-NEW: bf_musttail_always {{.*}}Callee calls branch funnel{{.*}}

; CHECK-MOLD: COMPILE FUNC: fn_musttail
; CHECK-MOLD: bf_musttail {{.*}}Callee calls branch funnel{{.*}}

; CHECK-MOLD: COMPILE FUNC: bf_musttail
; CHECK-MOLD: llvm.icall.branch.funnel {{.*}}Callee is intrinsic{{.*}}

; CHECK-MOLD: COMPILE FUNC: fn_musttail_always
; CHECK-MOLD: bf_musttail_always {{.*}}Callee calls branch funnel{{.*}}

; CHECK-MOLD: COMPILE FUNC: bf_musttail_always
; CHECK-MOLD: llvm.icall.branch.funnel {{.*}}Callee is intrinsic{{.*}}


target datalayout = "e-p:64:64"
target triple = "x86_64-unknown-linux-gnu"

@vt1_1 = constant [1 x ptr] [ptr @vf1_1], !type !0
@vt1_2 = constant [1 x ptr] [ptr @vf1_2], !type !0
declare i32 @vf1_1(ptr, i32)

declare i32 @vf1_2(ptr, i32)

declare void @llvm.icall.branch.funnel(...)

define void @fn_musttail(ptr %0, ...) #0 {
  musttail call void (ptr, ...) @bf_musttail(ptr %0, ...)
  ret void
}

define hidden void @bf_musttail(ptr nest %0, ...) alwaysinline {
  musttail call void (...) @llvm.icall.branch.funnel(ptr %0, ptr @vt1_1, ptr @vf1_1, ptr @vt1_2, ptr @vf1_2, ...)
  ret void
}

define void @fn_musttail_always(ptr %0, ...) #0 {
  musttail call void (ptr, ...) @bf_musttail_always(ptr %0, ...)
  ret void
}

define hidden void @bf_musttail_always(ptr nest %0, ...) alwaysinline {
  musttail call void (...) @llvm.icall.branch.funnel(ptr %0, ptr @vt1_1, ptr @vf1_1, ptr @vt1_2, ptr @vf1_2, ...)
  ret void
}

attributes #0 = { "target-features"="+retpoline" }
attributes #1 = { nounwind readnone willreturn }
attributes #2 = { nounwind willreturn }
attributes #3 = { nounwind }

!0 = !{i32 0, !"typeid1"}
!1 = !{i32 0, !"typeid2"}
!2 = !{i32 0, !"typeid3"}
!3 = !{i32 0, !4}
!4 = distinct !{}

