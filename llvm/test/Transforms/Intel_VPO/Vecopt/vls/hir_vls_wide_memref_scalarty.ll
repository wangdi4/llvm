; Test to check HIR vector CG support for VLS accesses where scalar type
; of widened memory ref is correctly passed to HIR utilities.

; Incoming HIR
; BEGIN REGION { }
;       %entry.region = @llvm.directive.region.entry(); [ DIR.VPO.AUTO.VEC() ]
;
;       + DO i1 = 0, 1023, 1   <DO_LOOP>
;       |   (i8**)(%arr)[i1].1 = &((%p1)[0]);
;       |   (i8**)(%arr)[i1].0 = &((%p2)[0]);
;       + END LOOP
;
;       @llvm.directive.region.exit(%entry.region); [ DIR.VPO.END.AUTO.VEC() ]
; END REGION

; RUN: opt -passes='hir-ssa-deconstruction,hir-temp-cleanup,hir-vec-dir-insert,hir-vplan-vec,print<hir>,hir-cg' -vplan-force-vf=4 -hir-details -S -aa-pipeline=tbaa -mattr=+avx2 -enable-intel-advanced-opts < %s 2>&1 | FileCheck %s

; CHECK:        BEGIN REGION { modified }
; CHECK:              + DO i64 i1 = 0, 1023, 4   <DO_LOOP> <auto-vectorized> <novectorize>
; CHECK:              |   %.extended = shufflevector %p2,  undef,  <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>;
; CHECK:              |   %shuffle = shufflevector undef,  %.extended,  <i32 8, i32 1, i32 9, i32 3, i32 10, i32 5, i32 11, i32 7>;
; CHECK:              |   %.extended1 = shufflevector %p1,  undef,  <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>;
; CHECK:              |   %shuffle2 = shufflevector %shuffle,  %.extended1,  <i32 0, i32 8, i32 2, i32 9, i32 4, i32 10, i32 6, i32 11>;
; CHECK:              |   (<8 x ptr>*)(%arr)[i1].0 = %shuffle2;
; CHECK:              |   <LVAL-REG> {al:8}(<8 x ptr>*)(LINEAR ptr %arr)[LINEAR i64 i1].0 inbounds
; CHECK:              + END LOOP
; CHECK:        END REGION

; Checks for LLVM-IR after HIR-CG
; CHECK-LABEL: define hidden fastcc void @parse_value(ptr %arr, ptr %p1, ptr %p2)
; CHECK:         store <8 x ptr> %shuffle28, ptr [[TEMP18:%.*]], align 64
; CHECK-NEXT:    [[TEMP0:%.*]] = load i64, ptr %i1.i64, align 8
; CHECK-NEXT:    [[TEMP1:%.*]] = getelementptr inbounds %struct.hashtable_list, ptr %arr, i64 [[TEMP0]], i32 0
; CHECK-NEXT:    [[TEMP18DOT:%.*]] = load <8 x ptr>, ptr [[TEMP18]], align 64
; CHECK-NEXT:    store <8 x ptr> [[TEMP18DOT]], ptr [[TEMP1]], align 8

target triple = "x86_64-unknown-linux-gnu"

%struct.hashtable_list = type { ptr, ptr }

define hidden fastcc void @parse_value(ptr %arr, ptr %p1, ptr %p2) {
entry:
  br label %header

header:
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %header ]
  %gep1 = getelementptr inbounds %struct.hashtable_list, ptr %arr, i64 %iv, i32 1
  store ptr %p1, ptr %gep1, align 8, !tbaa !4
  %gep2 = getelementptr inbounds %struct.hashtable_list, ptr %arr, i64 %iv, i32 0
  store ptr %p2, ptr %gep2, align 8, !tbaa !5
  %iv.next = add nuw nsw i64 %iv, 1
  %iv.cmp = icmp eq i64 %iv.next, 1024
  br i1 %iv.cmp, label %exit, label %header

exit:
  ret void
}

!1 = !{!"omnipotent char", !2, i64 0}
!2 = !{!"Simple C/C++ TBAA"}
!3 = !{!"pointer@_ZTSP14hashtable_list", !1, i64 0}
!4 = !{!6, !3, i64 8}
!5 = !{!6, !3, i64 0}
!6 = !{!"struct@hashtable_bucket", !3, i64 0, !3, i64 8}
