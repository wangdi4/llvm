; "-match-full-lines" is needed so that any additional metadata would cause a fail.
; RUN: opt -opaque-pointers=0 -passes="instcombine" -S < %s | FileCheck %s --match-full-lines

%struct.Outer = type { %struct.Inner, %struct.Inner }
%struct.Inner = type { [10 x i32] }

; Test !intel-tbaa metadata combining and propagation.
define i32 @test_merge(%struct.Outer* %o) {
; CHECK-LABEL: define i32 @test_merge(%struct.Outer* %o) {
; CHECK-NEXT:    [[INNER2_ARRAY_ELEM:%.*]] = getelementptr inbounds [[STRUCT_OUTER:%.*]], %struct.Outer* [[O:%.*]], i64 0, i32 1, i32 0, i64 7, !intel-tbaa !0
; CHECK-NEXT:    [[LD:%.*]] = load i32, i32* [[INNER2_ARRAY_ELEM]], align 4, !tbaa !7
; CHECK-NEXT:    ret i32 [[LD]]
;
  %inner2 = getelementptr inbounds %struct.Outer, %struct.Outer* %o, i32 0, i32 1, !intel-tbaa !7
  %inner2.array = getelementptr inbounds %struct.Inner, %struct.Inner* %inner2, i32 0, i32 0, !intel-tbaa !8
  %inner2.array.elem = getelementptr inbounds [10 x i32], [10 x i32]* %inner2.array, i32 0, i32 7, !intel-tbaa !9
  %ld = load i32, i32* %inner2.array.elem, !tbaa !10
  ret i32 %ld
}

; Test merging of the GEPs with missing !intel-tbaa annotation in the middle of
; the GEPs chain.
define i32 @test_no_merge(%struct.Outer* %o) {
; CHECK-LABEL: define i32 @test_no_merge(%struct.Outer* %o) {
; CHECK-NEXT:    [[INNER1_ARRAY_ELEM:%.*]] = getelementptr inbounds [[STRUCT_OUTER:%.*]], %struct.Outer* [[O:%.*]], i64 0, i32 0, i32 0, i64 7
; CHECK-NEXT:    [[LD:%.*]] = load i32, i32* [[INNER1_ARRAY_ELEM]], align 4, !tbaa !7
; CHECK-NEXT:    ret i32 [[LD]]
;
  %inner1 = getelementptr inbounds %struct.Outer, %struct.Outer* %o, i32 0, i32 0, !intel-tbaa !6
  %inner1.array = getelementptr inbounds %struct.Inner, %struct.Inner* %inner1, i32 0, i32 0
  %inner1.array.elem = getelementptr inbounds [10 x i32], [10 x i32]* %inner1.array, i32 0, i32 7, !intel-tbaa !9
  %ld = load i32, i32* %inner1.array.elem, !tbaa !10
  ret i32 %ld
}

; Special case of two-operand base GEP. Verify that no merging happens because
; the base GEP isn't annotated.
define i32* @test_no_merge_array_ptr([10 x i32]* %p) {
; CHECK-LABEL: define i32* @test_no_merge_array_ptr([10 x i32]* %p) {
; CHECK-NEXT:    [[ARRAYIDX1:%.*]] = getelementptr inbounds [10 x i32], [10 x i32]* [[P:%.*]], i64 3, i64 6
; CHECK-NEXT:    ret i32* [[ARRAYIDX1]]
;
  %arrayidx = getelementptr inbounds [10 x i32], [10 x i32]* %p, i32 3
  %arrayidx1 = getelementptr inbounds [10 x i32], [10 x i32]* %arrayidx, i32 0, i32 6, !intel-tbaa !9
  ret i32* %arrayidx1
}

; CHECK: !0 = !{!1, !4, i64 40}
; CHECK: !1 = !{!"struct@typeinfo name for Outer", !2, i64 0, !2, i64 40}
; CHECK: !2 = !{!"struct@typeinfo name for Inner", !3, i64 0}
; CHECK: !3 = !{!"array@typeinfo name for int [10]", !4, i64 0}
; CHECK: !4 = !{!"int", !5, i64 0}
; CHECK: !5 = !{!"omnipotent char", !6, i64 0}
; CHECK: !6 = !{!"Simple C++ TBAA"}
; CHECK: !7 = !{!4, !4, i64 0}


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
