; "-match-full-lines" is needed so that any additional metadata would cause a fail.
; RUN: opt -passes="instcombine" -S < %s | FileCheck %s --match-full-lines

%struct.Outer = type { %struct.Inner, %struct.Inner }
%struct.Inner = type { [10 x i32] }

; Test !intel-tbaa metadata combining and propagation.
define i32 @test_merge(ptr %o) {
; CHECK-LABEL: define i32 @test_merge(ptr %o) {
; CHECK-NEXT:    [[INNER2_ARRAY_ELEM:%.*]] = getelementptr inbounds [[STRUCT_OUTER:%.*]], ptr [[O:%.*]], i64 0, i32 1, i32 0, i64 7
; CHECK-NEXT:    [[LD:%.*]] = load i32, ptr [[INNER2_ARRAY_ELEM]], align 4, !tbaa !0
; CHECK-NEXT:    ret i32 [[LD]]
;
  %inner2 = getelementptr inbounds %struct.Outer, ptr %o, i32 0, i32 1, !intel-tbaa !7
  %inner2.array.elem = getelementptr inbounds [10 x i32], ptr %inner2, i32 0, i32 7, !intel-tbaa !9
  %ld = load i32, ptr %inner2.array.elem, !tbaa !10
  ret i32 %ld
}

; Test merging of the GEPs with missing !intel-tbaa annotation in the middle of
; the GEPs chain.
define i32 @test_no_merge(ptr %o) {
; CHECK-LABEL: define i32 @test_no_merge(ptr %o) {
; CHECK-NEXT:    [[INNER1_ARRAY_ELEM:%.*]] = getelementptr inbounds [[STRUCT_OUTER:%.*]], ptr [[O:%.*]], i64 0, i32 1, i32 0, i64 7
; CHECK-NEXT:    [[LD:%.*]] = load i32, ptr [[INNER1_ARRAY_ELEM]], align 4, !tbaa !0
; CHECK-NEXT:    ret i32 [[LD]]
;
  %inner1 = getelementptr inbounds %struct.Outer, ptr %o, i32 0, i32 0, !intel-tbaa !6
  %inner1.array = getelementptr inbounds %struct.Inner, ptr %inner1, i32 1, i32 0
  %inner1.array.elem = getelementptr inbounds [10 x i32], ptr %inner1.array, i32 0, i32 7, !intel-tbaa !9
  %ld = load i32, ptr %inner1.array.elem, !tbaa !10
  ret i32 %ld
}

; Special case of two-operand base GEP. Verify that no merging happens because
; the base GEP isn't annotated.
define ptr @test_no_merge_array_ptr(ptr %p) {
; CHECK-LABEL: define ptr @test_no_merge_array_ptr(ptr %p) {
; CHECK-NEXT:    [[ARRAYIDX1:%.*]] = getelementptr inbounds [10 x i32], ptr [[P:%.*]], i64 3, i64 6
; CHECK-NEXT:    ret ptr [[ARRAYIDX1]]
;
  %arrayidx = getelementptr inbounds [10 x i32], ptr %p, i32 3
  %arrayidx1 = getelementptr inbounds [10 x i32], ptr %arrayidx, i32 0, i32 6, !intel-tbaa !9
  ret ptr %arrayidx1
}


; CHECK: !0 = !{!1, !1, i64 0}
; CHECK: !1 = !{!"int", !2, i64 0}
; CHECK: !2 = !{!"omnipotent char", !3, i64 0}
; CHECK: !3 = !{!"Simple C++ TBAA"}

!0 = !{!"Simple C++ TBAA"}
!1 = !{!"omnipotent char", !0, i64 0}
!2 = !{!"int", !1, i64 0}
!3 = !{!"array@typeinfo name for int [10]", !2, i64 0}
!4 = !{!"struct@typeinfo name for Inner", !3, i64 0}
!5 = !{!"struct@typeinfo name for Outer", !4, i64 0, !4, i64 40}

!6 = !{!5, !4, i64 0}  ; Outer::Inner1
!7 = !{!5, !4, i64 40} ; Outer::Inner2
!8 = !{!4, !3, i64 0}  ; Inner::array
!9 = !{!3, !2, i64 0}  ; array::element

!10 = !{!2, !2, i64 0}  ; int* access
