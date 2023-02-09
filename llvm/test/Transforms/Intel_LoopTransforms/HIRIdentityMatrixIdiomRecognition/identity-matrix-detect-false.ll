; REQUIRES: asserts

; RUN: opt -opaque-pointers=0 -passes="hir-ssa-deconstruction,hir-identity-matrix-idiom-recognition,print<hir>" -enable-alt-identity-matrix-detection -debug-only=hir-hlnode-utils -S %s 2>&1 | FileCheck %s

; Verify that cases of invalid identity matrix are detected. We can accept false
; negatives, but not false positives

; FORTRAN Source code examples
;   real A(5,5), D(5,5,5)
;   real B(5,5), C(5,5)
;   do i=1,5
;     do j=1,5
;       A(j,i) = 0.0
;     enddo
;     A(i,0) = 1.0
;     A(i,i) = 1.0
;   enddo
;   do i=1,4
;     do j=1,4
;       B(j,i) = 0.0
;     enddo
;     B(i,i) = 1.0
;   enddo
;   do i=1,5
;     do j=1,5
;       C(j,i) = 0.0
;     enddo
;     call foo()
;     C(i,i) = 1.0
;   enddo
;   do i=1,5
;     A(i,i) = 1.0
;     do j=1,5
;       A(j,i) = 0.0
;     enddo
;     A(i,i) = 1.0
;   enddo

; CHECK: Found Diag Inst in OuterLp: <18>         (@A)[0][i1][i1] = 1.000000e+00;
; CHECK-NOT: Found Identity Matrix for Loop

;Module Before HIR
; ModuleID = 'false.f90'
source_filename = "false.f90"
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

define void @globals_mp_ident_(float* noalias nocapture readnone %DT, float* noalias nocapture readnone %T1) local_unnamed_addr #1 {
alloca:
  %"A[]6" = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 1, i64 20, float* elementtype(float) getelementptr inbounds ([5 x [5 x float]], [5 x [5 x float]]* @A, i64 0, i64 0, i64 0), i64 0)
  br label %bb3

bb3:                                              ; preds = %bb16, %alloca
  %indvars.iv154 = phi i64 [ %indvars.iv.next155, %bb16 ], [ 1, %alloca ]
  %"A[]" = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 1, i64 20, float* elementtype(float) getelementptr inbounds ([5 x [5 x float]], [5 x [5 x float]]* @A, i64 0, i64 0, i64 0), i64 %indvars.iv154)
  br label %bb7

bb7:                                              ; preds = %bb7, %bb3
  %indvars.iv151 = phi i64 [ %indvars.iv.next152, %bb7 ], [ 1, %bb3 ]
  %"A[][]" = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* elementtype(float) %"A[]", i64 %indvars.iv151)
  store float 0.000000e+00, float* %"A[][]", align 4
  %indvars.iv.next152 = add nuw nsw i64 %indvars.iv151, 1
  %exitcond153 = icmp eq i64 %indvars.iv.next152, 6
  br i1 %exitcond153, label %bb16, label %bb7

bb16:                                             ; preds = %bb7
  %"A[]6[]" = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* elementtype(float) nonnull %"A[]6", i64 %indvars.iv154)
  store float 1.000000e+00, float* %"A[]6[]", align 4
  %"A[]9[]" = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* elementtype(float) nonnull %"A[]", i64 %indvars.iv154)
  store float 1.000000e+00, float* %"A[]9[]", align 4
  %indvars.iv.next155 = add nuw nsw i64 %indvars.iv154, 1
  %exitcond156 = icmp eq i64 %indvars.iv.next155, 6
  br i1 %exitcond156, label %bb26.preheader, label %bb3

bb26.preheader:                                   ; preds = %bb16
  br label %bb26

bb26:                                             ; preds = %bb26.preheader, %bb39
  %indvars.iv148 = phi i64 [ %indvars.iv.next149, %bb39 ], [ 1, %bb26.preheader ]
  %"B[]" = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 1, i64 20, float* elementtype(float) getelementptr inbounds ([5 x [5 x float]], [5 x [5 x float]]* @B, i64 0, i64 0, i64 0), i64 %indvars.iv148)
  br label %bb30

bb30:                                             ; preds = %bb30, %bb26
  %indvars.iv145 = phi i64 [ %indvars.iv.next146, %bb30 ], [ 1, %bb26 ]
  %"B[][]" = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* elementtype(float) %"B[]", i64 %indvars.iv145)
  store float 0.000000e+00, float* %"B[][]", align 4
  %indvars.iv.next146 = add nuw nsw i64 %indvars.iv145, 1
  %exitcond147 = icmp eq i64 %indvars.iv.next146, 5
  br i1 %exitcond147, label %bb39, label %bb30

bb39:                                             ; preds = %bb30
  %"B[]34[]" = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* elementtype(float) nonnull %"B[]", i64 %indvars.iv148)
  store float 1.000000e+00, float* %"B[]34[]", align 4
  %indvars.iv.next149 = add nuw nsw i64 %indvars.iv148, 1
  %exitcond150 = icmp eq i64 %indvars.iv.next149, 5
  br i1 %exitcond150, label %bb44.preheader, label %bb26

bb44.preheader:                                   ; preds = %bb39
  br label %bb44

bb44:                                             ; preds = %bb44.preheader, %bb57
  %indvars.iv142 = phi i64 [ %indvars.iv.next143, %bb57 ], [ 1, %bb44.preheader ]
  %"C[]" = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 1, i64 20, float* elementtype(float) getelementptr inbounds ([5 x [5 x float]], [5 x [5 x float]]* @C, i64 0, i64 0, i64 0), i64 %indvars.iv142)
  br label %bb48

bb48:                                             ; preds = %bb48, %bb44
  %indvars.iv139 = phi i64 [ %indvars.iv.next140, %bb48 ], [ 1, %bb44 ]
  %"C[][]" = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* elementtype(float) %"C[]", i64 %indvars.iv139)
  store float 0.000000e+00, float* %"C[][]", align 4
  %indvars.iv.next140 = add nuw nsw i64 %indvars.iv139, 1
  %exitcond141 = icmp eq i64 %indvars.iv.next140, 6
  br i1 %exitcond141, label %bb57, label %bb48

bb57:                                             ; preds = %bb48
  tail call void (...) @foo_()
  %"C[]59[]" = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* elementtype(float) nonnull %"C[]", i64 %indvars.iv142)
  store float 1.000000e+00, float* %"C[]59[]", align 4
  %indvars.iv.next143 = add nuw nsw i64 %indvars.iv142, 1
  %exitcond144 = icmp eq i64 %indvars.iv.next143, 6
  br i1 %exitcond144, label %bb68.preheader, label %bb44

bb68.preheader:                                   ; preds = %bb57
  br label %bb68

bb68:                                             ; preds = %bb68.preheader, %bb86
  %indvars.iv136 = phi i64 [ %indvars.iv.next137, %bb86 ], [ 1, %bb68.preheader ]
  %"A[]72" = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 1, i64 20, float* elementtype(float) getelementptr inbounds ([5 x [5 x float]], [5 x [5 x float]]* @A, i64 0, i64 0, i64 0), i64 %indvars.iv136)
  %"A[]72[]" = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* elementtype(float) %"A[]72", i64 %indvars.iv136)
  store float 1.000000e+00, float* %"A[]72[]", align 4
  br label %bb77

bb77:                                             ; preds = %bb77, %bb68
  %indvars.iv = phi i64 [ %indvars.iv.next, %bb77 ], [ 1, %bb68 ]
  %"A[]77[]" = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* elementtype(float) %"A[]72", i64 %indvars.iv)
  store float 0.000000e+00, float* %"A[]77[]", align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 6
  br i1 %exitcond, label %bb86, label %bb77

bb86:                                             ; preds = %bb77
  store float 1.000000e+00, float* %"A[]72[]", align 4
  %indvars.iv.next137 = add nuw nsw i64 %indvars.iv136, 1
  %exitcond138 = icmp eq i64 %indvars.iv.next137, 6
  br i1 %exitcond138, label %bb1, label %bb68

bb1:                                              ; preds = %bb86
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8, i64, i64, float*, i64) #2

declare void @foo_(...) local_unnamed_addr

attributes #0 = { norecurse nounwind readnone "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { "pre_loopopt" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #2 = { nounwind readnone speculatable }

!omp_offload.info = !{}
