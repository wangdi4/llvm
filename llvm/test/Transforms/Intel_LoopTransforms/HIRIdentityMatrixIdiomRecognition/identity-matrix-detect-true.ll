; REQUIRES: asserts

; RUN: opt -debug -hir-ssa-deconstruction -hir-identity-matrix-idiom-recognition -enable-alt-identity-matrix-detection -debug-only=hir-hlnode-utils -S %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-identity-matrix-idiom-recognition,print<hir>" -enable-alt-identity-matrix-detection -debug-only=hir-hlnode-utils -aa-pipeline=basic-aa -S %s 2>&1 | FileCheck %s

; Verify that cases of valid identity matrix are detected. We can accept false
; negatives, but not false positives

; FORTRAN Source code examples
;    real A(5,5)
;    real B(5,5), C(5,5)
;    do i=1,5
;      do j=1,5
;        A(j,i) = 0.0
;      enddo
;      A(i,i) = 1.0
;    enddo
;    do i=1,5
;      do j=1,5
;        B(j,i) = 0.0
;        C(j,i) = 0.0
;      enddo
;      B(i,i) = 1.0
;      C(i,i) = 1.0
;    enddo

; CHECK: Found Diag Inst in OuterLp: <{{[0-9]+}}>         (@A)[0][i1][i1] = 1.000000e+00;
; CHECK: Found Zero Instruction: <{{[0-9]+}}>          (@A)[0][i1][i2] = 0.000000e+00;
; CHECK: Found Ident Matrix, DiagInst: <{{[0-9]+}}>         (@A)[0][i1][i1] = 1.000000e+00;

; CHECK: Found Diag Inst in OuterLp: <{{[0-9]+}}>         (@C)[0][i1][i1] = 1.000000e+00;
; CHECK: Found Diag Inst in OuterLp: <{{[0-9]+}}>         (@B)[0][i1][i1] = 1.000000e+00;
; CHECK: Found Zero Instruction: <{{[0-9]+}}>         (@C)[0][i1][i2] = 0.000000e+00;
; CHECK: Found Zero Instruction: <{{[0-9]+}}>         (@B)[0][i1][i2] = 0.000000e+00;
; CHECK: Found Ident Matrix, DiagInst: <{{[0-9]+}}>         (@C)[0][i1][i1] = 1.000000e+00;
; CHECK: Found Ident Matrix, DiagInst: <{{[0-9]+}}>         (@B)[0][i1][i1] = 1.000000e+00;

;Module Before HIR
; ModuleID = 'true.f90'
source_filename = "true.f90"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@C = common dso_local local_unnamed_addr global [5 x [5 x float]] zeroinitializer, align 8
@B = common dso_local local_unnamed_addr global [5 x [5 x float]] zeroinitializer, align 8
@D = common dso_local local_unnamed_addr global [5 x [5 x [5 x float]]] zeroinitializer, align 8
@A = common dso_local local_unnamed_addr global [5 x [5 x float]] zeroinitializer, align 8

; Function Attrs: norecurse nounwind readnone
define void @globals._() local_unnamed_addr #0 {
alloca:
  ret void
}

; Function Attrs: nofree nounwind writeonly
define void @globals_mp_ident_(float* noalias nocapture readnone %DT, float* noalias nocapture readnone %T1) local_unnamed_addr #1 {
alloca:
  br label %bb3

bb3:                                              ; preds = %bb16, %alloca
  %indvars.iv77 = phi i64 [ %indvars.iv.next78, %bb16 ], [ 1, %alloca ]
  %"A[]" = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 1, i64 20, float* elementtype(float) getelementptr inbounds ([5 x [5 x float]], [5 x [5 x float]]* @A, i64 0, i64 0, i64 0), i64 %indvars.iv77)
  br label %bb7

bb7:                                              ; preds = %bb7, %bb3
  %indvars.iv74 = phi i64 [ %indvars.iv.next75, %bb7 ], [ 1, %bb3 ]
  %"A[][]" = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* elementtype(float) %"A[]", i64 %indvars.iv74)
  store float 0.000000e+00, float* %"A[][]", align 4
  %indvars.iv.next75 = add nuw nsw i64 %indvars.iv74, 1
  %exitcond76 = icmp eq i64 %indvars.iv.next75, 6
  br i1 %exitcond76, label %bb16, label %bb7

bb16:                                             ; preds = %bb7
  %"A[]6[]" = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* elementtype(float) nonnull %"A[]", i64 %indvars.iv77)
  store float 1.000000e+00, float* %"A[]6[]", align 4
  %indvars.iv.next78 = add nuw nsw i64 %indvars.iv77, 1
  %exitcond79 = icmp eq i64 %indvars.iv.next78, 6
  br i1 %exitcond79, label %bb21.preheader, label %bb3

bb21.preheader:                                   ; preds = %bb16
  br label %bb21

bb21:                                             ; preds = %bb21.preheader, %bb39
  %indvars.iv71 = phi i64 [ %indvars.iv.next72, %bb39 ], [ 1, %bb21.preheader ]
  %"B[]" = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 1, i64 20, float* elementtype(float) getelementptr inbounds ([5 x [5 x float]], [5 x [5 x float]]* @B, i64 0, i64 0, i64 0), i64 %indvars.iv71)
  %"C[]" = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 1, i64 20, float* elementtype(float) getelementptr inbounds ([5 x [5 x float]], [5 x [5 x float]]* @C, i64 0, i64 0, i64 0), i64 %indvars.iv71)
  br label %bb25

bb25:                                             ; preds = %bb25, %bb21
  %indvars.iv = phi i64 [ %indvars.iv.next, %bb25 ], [ 1, %bb21 ]
  %"B[][]" = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* elementtype(float) %"B[]", i64 %indvars.iv)
  store float 0.000000e+00, float* %"B[][]", align 4
  %"C[][]" = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* elementtype(float) %"C[]", i64 %indvars.iv)
  store float 0.000000e+00, float* %"C[][]", align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 6
  br i1 %exitcond, label %bb39, label %bb25

bb39:                                             ; preds = %bb25
  %"B[]35[]" = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* elementtype(float) nonnull %"B[]", i64 %indvars.iv71)
  store float 1.000000e+00, float* %"B[]35[]", align 4
  %"C[]40[]" = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* elementtype(float) nonnull %"C[]", i64 %indvars.iv71)
  store float 1.000000e+00, float* %"C[]40[]", align 4
  %indvars.iv.next72 = add nuw nsw i64 %indvars.iv71, 1
  %exitcond73 = icmp eq i64 %indvars.iv.next72, 6
  br i1 %exitcond73, label %bb1, label %bb21

bb1:                                              ; preds = %bb39
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8, i64, i64, float*, i64) #2

attributes #0 = { norecurse nounwind readnone "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { nofree nounwind writeonly "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #2 = { nounwind readnone speculatable }

!omp_offload.info = !{}
