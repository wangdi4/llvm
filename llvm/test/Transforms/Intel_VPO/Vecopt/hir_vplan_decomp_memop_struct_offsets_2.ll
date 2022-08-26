; Test HIR decomposition of memory operands with complex trailing struct offsets.
; Input LLVM-IR generated for below C code with command: icx -O2 -print-module-before-loopopt

; struct RT {
;     char A;
;     int B[10][20];
;     char C;
; };
;
; struct ST {
;     int X;
;     double Y;
;     struct RT Z;
; } st1;
;
; void foo() {
;     int i1;
; #pragma omp simd
;     for (i1 = 0; i1 < 20; i1++)
;         st1.Z.B[5][i1] = i1;
; }

; Input HIR
; <18>     + DO i1 = 0, 19, 1   <DO_LOOP>
; <8>      |   (@st1)[0].2.1[5][i1] = i1;
; <18>     + END LOOP


; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -vplan-print-after-plain-cfg -vplan-dump-subscript-details -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec" -vplan-print-after-plain-cfg -vplan-dump-subscript-details -disable-output < %s 2>&1 | FileCheck %s

; Check decomposed VPInstructions
; CHECK: i64 [[I1:%vp.*]] = phi
; CHECK: i32* [[ADDR1:%vp.*]] = subscript inbounds %struct.ST* @st1 {i64 0 : i64 0 : i64 824 : %struct.ST*(%struct.ST) (2 1 )} {i64 0 : i64 5 : i64 80 : [10 x [20 x i32]]([20 x i32])} {i64 0 : i64 [[I1]] : i64 4 : [20 x i32](i32)}
; CHECK-NEXT: store i32 {{%vp.*}} i32* [[ADDR1]]

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.ST = type { i32, double, %struct.RT }
%struct.RT = type { i8, [10 x [20 x i32]], i8 }

@st1 = common dso_local local_unnamed_addr global %struct.ST zeroinitializer, align 8

; Function Attrs: nounwind uwtable
define dso_local void @foo() local_unnamed_addr {
omp.inner.for.body.lr.ph:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null) ]
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %omp.inner.for.body.lr.ph
  %indvars.iv = phi i64 [ %indvars.iv.next, %omp.inner.for.body ], [ 0, %omp.inner.for.body.lr.ph ]
  %arrayidx = getelementptr inbounds %struct.ST, %struct.ST* @st1, i64 0, i32 2, i32 1, i64 5, i64 %indvars.iv
  %1 = trunc i64 %indvars.iv to i32
  store i32 %1, i32* %arrayidx, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 20
  br i1 %exitcond, label %DIR.OMP.END.SIMD.2, label %omp.inner.for.body

DIR.OMP.END.SIMD.2:                               ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #1 = { nounwind }

!2 = !{!3, !4, i64 20}
!3 = !{!"struct@ST", !4, i64 0, !7, i64 8, !8, i64 16}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = !{!"double", !5, i64 0}
!8 = !{!"struct@RT", !5, i64 0, !9, i64 4, !5, i64 804}
!9 = !{!"array@_ZTSA10_A20_i", !10, i64 0}
!10 = !{!"array@_ZTSA20_i", !4, i64 0}
