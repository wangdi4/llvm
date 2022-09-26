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

; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-vec-dir-insert -hir-vplan-vec -vplan-force-vf=4 -print-after=hir-vplan-vec -hir-details -hir-cg -S -tbaa -mattr=+avx2 -enable-intel-advanced-opts < %s 2>&1 | FileCheck %s

; CHECK:        BEGIN REGION { modified }
; CHECK:              + DO i64 i1 = 0, 1023, 4   <DO_LOOP> <auto-vectorized> <novectorize>
; CHECK:              |   %.extended = shufflevector %p2,  undef,  <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>;
; CHECK:              |   %shuffle = shufflevector undef,  %.extended,  <i32 8, i32 1, i32 9, i32 3, i32 10, i32 5, i32 11, i32 7>;
; CHECK:              |   %.extended1 = shufflevector %p1,  undef,  <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>;
; CHECK:              |   %shuffle2 = shufflevector %shuffle,  %.extended1,  <i32 0, i32 8, i32 2, i32 9, i32 4, i32 10, i32 6, i32 11>;
; CHECK:              |   %extract.0. = extractelement &((<4 x i8**>)(%arr)[i1 + <i64 0, i64 1, i64 2, i64 3>].0),  0;
; CHECK:              |   (<8 x i8*>*)(%extract.0.)[0] = %shuffle2;
; CHECK:              |   <LVAL-REG> {al:8}(<8 x i8*>*)(NON-LINEAR i8** %extract.0.)[i64 0] inbounds
; CHECK:              + END LOOP
; CHECK:        END REGION

; Checks for LLVM-IR after HIR-CG
; CHECK-LABEL: define hidden fastcc void @parse_value(%struct.hashtable_list* %arr, i8* %p1, i8* %p2)
; CHECK:         [[EXTRACT:%.*]] = extractelement <4 x i8**> [[WIDE_GEP:%.*]], i64 0
; CHECK-NEXT:    store i8** [[EXTRACT]], i8*** [[TEMP20:%.*]], align 8
; CHECK-NEXT:    [[TEMP20_LD:%.*]] = load i8**, i8*** [[TEMP20]], align 8
; CHECK-NEXT:    [[TEMP20_BC:%.*]] = bitcast i8** [[TEMP20_LD]] to <8 x i8*>*
; CHECK-NEXT:    [[TEMP19_LD:%.*]] = load <8 x i8*>, <8 x i8*>* [[TEMP19:%.*]], align 64
; CHECK-NEXT:    store <8 x i8*> [[TEMP19_LD]], <8 x i8*>* [[TEMP20_BC]], align 8

target triple = "x86_64-unknown-linux-gnu"

%struct.hashtable_list = type { %struct.hashtable_list*, %struct.hashtable_list* }

define hidden fastcc void @parse_value(%struct.hashtable_list* %arr, i8* %p1, i8* %p2) {
entry:
  br label %header

header:
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %header ]
  %gep1 = getelementptr inbounds %struct.hashtable_list, %struct.hashtable_list* %arr, i64 %iv, i32 1
  %bc1 = bitcast %struct.hashtable_list** %gep1 to i8**
  store i8* %p1, i8** %bc1, align 8, !tbaa !4
  %gep2 = getelementptr inbounds %struct.hashtable_list, %struct.hashtable_list* %arr, i64 %iv, i32 0
  %bc2 = bitcast %struct.hashtable_list** %gep2 to i8**
  store i8* %p2, i8** %bc2, align 8, !tbaa !5
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
