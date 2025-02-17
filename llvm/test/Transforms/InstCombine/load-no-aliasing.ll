; NOTE: Assertions have been autogenerated by utils/update_test_checks.py
; RUN: opt -tbaa -instcombine -S < %s | FileCheck %s

; Check that load to load forwarding works with non aliasing store inbetween.
define i32 @test_load_store_load_combine(ptr, ptr) {
; CHECK-LABEL: @test_load_store_load_combine(
; CHECK-NEXT:    [[A:%.*]] = load i32, ptr [[TMP0:%.*]], align 4, !tbaa [[TBAA0:![0-9]+]]
; CHECK-NEXT:    [[F:%.*]] = sitofp i32 [[A]] to float
; CHECK-NEXT:    store float [[F]], ptr [[TMP1:%.*]], align 4, !tbaa [[TBAA4:![0-9]+]]
; CHECK-NEXT:    ret i32 [[A]]
;
  %a = load i32, ptr %0, align 4, !tbaa !0
  %f = sitofp i32 %a to float
  store float %f, ptr %1, align 4, !tbaa !4
  %b = load i32, ptr %0, align 4, !tbaa !0
  ret i32 %b
}

!0 = !{!1, !1, i64 0}
!1 = !{!"int", !2, i64 0}
!2 = !{!"omnipotent char", !3, i64 0}
!3 = !{!"Simple C++ TBAA"}
!4 = !{!5, !5, i64 0}
!5 = !{!"float", !2, i64 0}
