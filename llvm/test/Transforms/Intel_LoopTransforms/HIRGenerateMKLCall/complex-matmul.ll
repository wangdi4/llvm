; Test for generating mkl call for matrix multiplication with complex data type

; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-loop-interchange -hir-generate-mkl-call -print-after=hir-generate-mkl-call -S < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-interchange,hir-generate-mkl-call,print<hir>" -aa-pipeline="basic-aa" -S < %s 2>&1 | FileCheck %s
;

; Fortran complex matmul source code-
; program main
; complex*8 a3(50,50),b3(50,50), c3(50,50)
; n = 50
; do j=1,n
;    do k = 1,n
;       do i = 1,n
;          c3(i,j) = c3(i,j) + a3(i,k) * b3(k,j)
;       enddo
;    enddo
; enddo
; end

; Before HIR Generate MKL Call-
; + DO i1 = 0, 49, 1   <DO_LOOP>
; |   + DO i2 = 0, 49, 1   <DO_LOOP>
; |   |   %.unpack277 = (@"main_$B3")[0][i1 + 1][i2 + 1].0;
; |   |   %.unpack279 = (@"main_$B3")[0][i1 + 1][i2 + 1].1;
; |   |
; |   |   + DO i3 = 0, 49, 1   <DO_LOOP>
; |   |   |   %.unpack273 = (@"main_$A3")[0][i2 + 1][i3 + 1].0;
; |   |   |   %.unpack275 = (@"main_$A3")[0][i2 + 1][i3 + 1].1;
; |   |   |   %mul = %.unpack275  *  %.unpack279;
; |   |   |   %mul92 = %.unpack273  *  %.unpack277;
; |   |   |   %sub93 = %mul92  -  %mul;
; |   |   |   %mul94 = %.unpack275  *  %.unpack277;
; |   |   |   %mul95 = %.unpack273  *  %.unpack279;
; |   |   |   %add96 = %mul94  +  %mul95;
; |   |   |   %add100 = (@"main_$C3")[0][i1 + 1][i3 + 1].0  +  %sub93;
; |   |   |   %add102 = (@"main_$C3")[0][i1 + 1][i3 + 1].1  +  %add96;
; |   |   |   (@"main_$C3")[0][i1 + 1][i3 + 1].0 = %add100;
; |   |   |   (@"main_$C3")[0][i1 + 1][i3 + 1].1 = %add102;
; |   |   + END LOOP
; |   + END LOOP
; + END LOOP


; After HIR Generate MKL Call-
; CHECK: BEGIN REGION { modified }
; CHECK: 0 = &((i8*)(@"main_$C3")[0][0][0].0);
; CHECK: 1 = 8;
; CHECK: 2 = 0;
; CHECK: 3 = 0;
; CHECK: 4 = 2;
; CHECK: 5 = 0;
; CHECK: 6 = 50;
; CHECK: 7 = 8;
; CHECK: 8 = 1;
; CHECK: 9 = 50;
; CHECK: 10 = 400;
; CHECK: 11 = 1;
; CHECK: 0 = &((i8*)(@"main_$A3")[0][0][0].0);
; CHECK: 1 = 8;
; CHECK: 2 = 0;
; CHECK: 3 = 0;
; CHECK: 4 = 2;
; CHECK: 5 = 0;
; CHECK: 6 = 50;
; CHECK: 7 = 8;
; CHECK: 8 = 1;
; CHECK: 9 = 50;
; CHECK: 10 = 400;
; CHECK: 11 = 1;
; CHECK: 0 = &((i8*)(@"main_$B3")[0][0][0].0);
; CHECK: 1 = 8;
; CHECK: 2 = 0;
; CHECK: 3 = 0;
; CHECK: 4 = 2;
; CHECK: 5 = 0;
; CHECK: 6 = 50;
; CHECK: 7 = 8;
; CHECK: 8 = 1;
; CHECK: 9 = 50;
; CHECK: 10 = 400;
; CHECK: 11 = 1;
; CHECK: @matmul_mkl_c64_
; CHECK: END REGION


;Module Before HIR
; ModuleID = 'complex-matmul-loop.f'
source_filename = "complex-matmul-loop.f"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%complex_64bit = type { float, float }

@"main_$C3" = internal unnamed_addr global [50 x [50 x %complex_64bit]] zeroinitializer, align 16
@"main_$B3" = internal unnamed_addr constant [50 x [50 x %complex_64bit]] zeroinitializer, align 16
@"main_$A3" = internal unnamed_addr constant [50 x [50 x %complex_64bit]] zeroinitializer, align 16
@0 = internal unnamed_addr constant i32 2

define void @MAIN__() local_unnamed_addr #0 {
alloca:
  %0 = tail call i32 @for_set_reentrancy(i32* nonnull @0)
  br label %bb5

bb5:                                              ; preds = %bb10, %alloca
  %indvars.iv127 = phi i64 [ 1, %alloca ], [ %indvars.iv.next128, %bb10 ]
  %1 = tail call %complex_64bit* @llvm.intel.subscript.p0s_complex_64bits.i64.i64.p0s_complex_64bits.i64(i8 1, i64 1, i64 400, %complex_64bit* getelementptr inbounds ([50 x [50 x %complex_64bit]], [50 x [50 x %complex_64bit]]* @"main_$C3", i64 0, i64 0, i64 0), i64 %indvars.iv127)
  %2 = tail call %complex_64bit* @llvm.intel.subscript.p0s_complex_64bits.i64.i64.p0s_complex_64bits.i64(i8 1, i64 1, i64 400, %complex_64bit* getelementptr inbounds ([50 x [50 x %complex_64bit]], [50 x [50 x %complex_64bit]]* @"main_$B3", i64 0, i64 0, i64 0), i64 %indvars.iv127)
  br label %bb9

bb9:                                              ; preds = %bb14, %bb5
  %indvars.iv124 = phi i64 [ 1, %bb5 ], [ %indvars.iv.next125, %bb14 ]
  %3 = tail call %complex_64bit* @llvm.intel.subscript.p0s_complex_64bits.i64.i64.p0s_complex_64bits.i64(i8 1, i64 1, i64 400, %complex_64bit* getelementptr inbounds ([50 x [50 x %complex_64bit]], [50 x [50 x %complex_64bit]]* @"main_$A3", i64 0, i64 0, i64 0), i64 %indvars.iv124)
  %4 = tail call %complex_64bit* @llvm.intel.subscript.p0s_complex_64bits.i64.i64.p0s_complex_64bits.i64(i8 0, i64 1, i64 8, %complex_64bit* %2, i64 %indvars.iv124)
  %.elt118 = getelementptr inbounds %complex_64bit, %complex_64bit* %4, i64 0, i32 0
  %.unpack119 = load float, float* %.elt118, align 4
  %.elt120 = getelementptr inbounds %complex_64bit, %complex_64bit* %4, i64 0, i32 1
  %.unpack121 = load float, float* %.elt120, align 4
  br label %bb13

bb13:                                             ; preds = %bb13, %bb9
  %indvars.iv = phi i64 [ 1, %bb9 ], [ %indvars.iv.next, %bb13 ]
  %5 = tail call %complex_64bit* @llvm.intel.subscript.p0s_complex_64bits.i64.i64.p0s_complex_64bits.i64(i8 0, i64 1, i64 8, %complex_64bit* %1, i64 %indvars.iv)
  %.elt = getelementptr inbounds %complex_64bit, %complex_64bit* %5, i64 0, i32 0
  %.unpack = load float, float* %.elt, align 4
  %.elt112 = getelementptr inbounds %complex_64bit, %complex_64bit* %5, i64 0, i32 1
  %.unpack113 = load float, float* %.elt112, align 4
  %6 = tail call %complex_64bit* @llvm.intel.subscript.p0s_complex_64bits.i64.i64.p0s_complex_64bits.i64(i8 0, i64 1, i64 8, %complex_64bit* %3, i64 %indvars.iv)
  %.elt114 = getelementptr inbounds %complex_64bit, %complex_64bit* %6, i64 0, i32 0
  %.unpack115 = load float, float* %.elt114, align 4
  %.elt116 = getelementptr inbounds %complex_64bit, %complex_64bit* %6, i64 0, i32 1
  %.unpack117 = load float, float* %.elt116, align 4
  %mul = fmul float %.unpack117, %.unpack121
  %mul40 = fmul float %.unpack115, %.unpack119
  %sub = fsub float %mul40, %mul
  %mul41 = fmul float %.unpack117, %.unpack119
  %mul42 = fmul float %.unpack115, %.unpack121
  %add = fadd float %mul41, %mul42
  %add45 = fadd float %.unpack, %sub
  %add47 = fadd float %.unpack113, %add
  store float %add45, float* %.elt, align 4
  store float %add47, float* %.elt112, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 51
  br i1 %exitcond, label %bb14, label %bb13

bb14:                                             ; preds = %bb13
  %indvars.iv.next125 = add nuw nsw i64 %indvars.iv124, 1
  %exitcond126 = icmp eq i64 %indvars.iv.next125, 51
  br i1 %exitcond126, label %bb10, label %bb9

bb10:                                             ; preds = %bb14
  %indvars.iv.next128 = add nuw nsw i64 %indvars.iv127, 1
  %exitcond129 = icmp eq i64 %indvars.iv.next128, 51
  br i1 %exitcond129, label %bb1, label %bb5

bb1:                                              ; preds = %bb10
  ret void
}

declare i32 @for_set_reentrancy(i32*) local_unnamed_addr

; Function Attrs: nounwind readnone speculatable
declare %complex_64bit* @llvm.intel.subscript.p0s_complex_64bits.i64.i64.p0s_complex_64bits.i64(i8, i64, i64, %complex_64bit*, i64) #1

attributes #0 = { "pre_loopopt" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { nounwind readnone speculatable }


