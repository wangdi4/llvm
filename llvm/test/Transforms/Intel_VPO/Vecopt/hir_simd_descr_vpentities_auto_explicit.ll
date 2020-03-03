; Test to check correctness of VPlan entities generated for auto-recognized and explicit SIMD reduction descriptors when vectorzing with incoming HIR.

; __inline
; int bar(int p0, int p1, int n, int *ptr, int step) {
;     char c = step;
; #pragma omp simd reduction(+: p0, p1)
;     for(int i = 0; i < n; i++) {
;         p0 += *ptr * c;
;         p1 += 2*i;
;         c++;
;     }
;     return p0 +p1;
; }
;
; int A[5];
;
; int foo() {
;     return bar(A[0], A[1], A[2], &A[3], A[4]);
; }

; Input HIR
; <0>     BEGIN REGION { }
; <2>           %6 = @llvm.directive.region.entry(); [ DIR.OMP.SIMD(),  QUAL.OMP.REDUCTION.ADD(&((%p0.addr.i)[0])),  QUAL.OMP.REDUCTION.ADD(&((%p1.addr.i)[0])),  QUAL.OMP.NORMALIZED.IV(null),  QUAL.OMP.NORMALIZED.UB(null) ]
; <3>           %7 = (@A)[0][3];
; <4>           %p0.addr.i.promoted = (%p0.addr.i)[0];
; <5>           %p1.addr.i.promoted = (%p1.addr.i)[0];
; <6>           %add10.i5 = %p1.addr.i.promoted;
; <7>           %add8.i4 = %p0.addr.i.promoted;
; <9>           %c.0.i2 = %3;
; <31>
; <31>          + DO i1 = 0, %2 + -1, 1   <DO_LOOP>  <MAX_TC_EST = 4294967295> <simd>
; <14>          |   %add8.i4 = %add8.i4  +  (sext.i8.i32(%c.0.i2) * %7); <Safe Reduction>
; <16>          |   %add10.i = %add10.i5  +  2 * i1;
; <20>          |   %c.0.i2 = i1 + trunc.i32.i8(%3) + 1;
; <22>          |   %add10.i5 = %add10.i;
; <31>          + END LOOP
; <31>
; <27>          (%p0.addr.i)[0] = %add8.i4;
; <28>          (%p1.addr.i)[0] = %add10.i;
; <29>          @llvm.directive.region.exit(%6); [ DIR.OMP.END.SIMD() ]
; <0>     END REGION

; For the above HIR, reduction for the variable %p0.addr.i is auto-recognized as SRA while that for %p1.addr.i is recognized explicitly via clause descriptors.
; We use aliases of the descriptor variables to correctly identify the init and finalize VPValues.


; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -hir-temp-cleanup -hir-last-value-computation -VPlanDriverHIR -disable-vplan-codegen -vplan-entities-dump -vplan-print-after-vpentity-instrs -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-last-value-computation,hir-vec-dir-insert,vplan-driver-hir" -disable-vplan-codegen -vplan-entities-dump -vplan-print-after-vpentity-instrs -disable-output < %s 2>&1 | FileCheck %s
; REQUIRES: asserts

; Check entities dump and VPlan IR
; CHECK: Reduction list
; CHECK: (+) Start: i32 [[V1_START:%.*]] Exit: i32 [[V1_EXIT:%.*]]
; CHECK: (+) Start: i32 [[V2_START:%.*]] Exit: i32 [[V2_EXIT:%.*]]

; CHECK-LABEL: REGION
; CHECK: i32 {{%vp.*}} = reduction-init i32 0 i32 [[V1_START]]
; CHECK: i32 {{%vp.*}} = reduction-init i32 0 i32 [[V2_START]]
; CHECK: i32 {{%vp.*}} = reduction-final{u_add} i32 [[V1_EXIT]]
; CHECK: i32 {{%vp.*}} = reduction-final{u_add} i32 [[V2_EXIT]]

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = dso_local local_unnamed_addr global [5 x i32] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define dso_local i32 @_Z3foov() local_unnamed_addr {
entry:
  %p0.addr.i = alloca i32, align 4
  %p1.addr.i = alloca i32, align 4
  %0 = load i32, i32* getelementptr inbounds ([5 x i32], [5 x i32]* @A, i64 0, i64 0), align 16, !tbaa !2
  %1 = load i32, i32* getelementptr inbounds ([5 x i32], [5 x i32]* @A, i64 0, i64 1), align 4, !tbaa !2
  %2 = load i32, i32* getelementptr inbounds ([5 x i32], [5 x i32]* @A, i64 0, i64 2), align 8, !tbaa !2
  %3 = load i32, i32* getelementptr inbounds ([5 x i32], [5 x i32]* @A, i64 0, i64 4), align 16, !tbaa !2
  %4 = bitcast i32* %p0.addr.i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %4)
  %5 = bitcast i32* %p1.addr.i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %5)
  store i32 %0, i32* %p0.addr.i, align 4, !tbaa !7
  store i32 %1, i32* %p1.addr.i, align 4, !tbaa !7
  %cmp.i = icmp sgt i32 %2, 0
  br i1 %cmp.i, label %DIR.OMP.SIMD.2, label %_Z3bariiiPii.exit

DIR.OMP.SIMD.2:                                   ; preds = %entry
  %conv.i = trunc i32 %3 to i8
  %6 = call token @llvm.directive.region.entry() #2 [ "DIR.OMP.SIMD"(), "QUAL.OMP.REDUCTION.ADD"(i32* %p0.addr.i), "QUAL.OMP.REDUCTION.ADD"(i32* %p1.addr.i), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null) ]
  %7 = load i32, i32* getelementptr inbounds ([5 x i32], [5 x i32]* @A, i64 0, i64 3), align 4, !tbaa !7
  %p0.addr.i.promoted = load i32, i32* %p0.addr.i, align 4, !tbaa !7
  %p1.addr.i.promoted = load i32, i32* %p1.addr.i, align 4, !tbaa !7
  br label %omp.inner.for.body.i

omp.inner.for.body.i:                             ; preds = %omp.inner.for.body.i, %DIR.OMP.SIMD.2
  %add10.i5 = phi i32 [ %add10.i, %omp.inner.for.body.i ], [ %p1.addr.i.promoted, %DIR.OMP.SIMD.2 ]
  %add8.i4 = phi i32 [ %add8.i, %omp.inner.for.body.i ], [ %p0.addr.i.promoted, %DIR.OMP.SIMD.2 ]
  %.omp.iv.i.0 = phi i32 [ %add11.i, %omp.inner.for.body.i ], [ 0, %DIR.OMP.SIMD.2 ]
  %c.0.i2 = phi i8 [ %inc.i, %omp.inner.for.body.i ], [ %conv.i, %DIR.OMP.SIMD.2 ]
  %conv6.i = sext i8 %c.0.i2 to i32
  %mul7.i = mul nsw i32 %7, %conv6.i
  %add8.i = add nsw i32 %add8.i4, %mul7.i
  %mul9.i = shl nuw nsw i32 %.omp.iv.i.0, 1
  %add10.i = add nsw i32 %add10.i5, %mul9.i
  %inc.i = add i8 %c.0.i2, 1
  %add11.i = add nuw nsw i32 %.omp.iv.i.0, 1
  %exitcond = icmp eq i32 %add11.i, %2
  br i1 %exitcond, label %omp.loop.exit.i, label %omp.inner.for.body.i

omp.loop.exit.i:                                  ; preds = %omp.inner.for.body.i
  %add8.i.lcssa = phi i32 [ %add8.i, %omp.inner.for.body.i ]
  %add10.i.lcssa = phi i32 [ %add10.i, %omp.inner.for.body.i ]
  store i32 %add8.i.lcssa, i32* %p0.addr.i, align 4, !tbaa !7
  store i32 %add10.i.lcssa, i32* %p1.addr.i, align 4, !tbaa !7
  call void @llvm.directive.region.exit(token %6) #2 [ "DIR.OMP.END.SIMD"() ]
  %.pre = load i32, i32* %p0.addr.i, align 4, !tbaa !7
  %.pre6 = load i32, i32* %p1.addr.i, align 4, !tbaa !7
  br label %_Z3bariiiPii.exit

_Z3bariiiPii.exit:                                ; preds = %entry, %omp.loop.exit.i
  %8 = phi i32 [ %1, %entry ], [ %.pre6, %omp.loop.exit.i ]
  %9 = phi i32 [ %0, %entry ], [ %.pre, %omp.loop.exit.i ]
  %add12.i = add nsw i32 %8, %9
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %4)
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %5)
  ret i32 %add12.i
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind }


!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA5_i", !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C++ TBAA"}
!7 = !{!4, !4, i64 0}
