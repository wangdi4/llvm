; RUN: opt -aa-pipeline="basic-aa" -passes="hir-ssa-deconstruction,hir-dead-store-elimination" -hir-create-function-level-region -print-before=hir-dead-store-elimination -print-after=hir-dead-store-elimination -S -disable-output 2>&1 < %s | FileCheck %s
;
;
; FORTRAN Source Code:
;subroutine sub(a,b,n)
;  real c(n), a(n),b(n)
;  c = a + b + 1.0
;  print*,a
;end subroutine sub
;
;*** IR Dump Before HIR Dead Store Elimination ***
; [Note]
; This region has both a loop and non-loop code.
;
; CHECK:      BEGIN REGION { }
; CHECK:           + DO i1 = 0, sext.i32.i64(%"sub_$N_fetch") + -1, 1   <DO_LOOP>
; CHECK:           |   %"sub_$A[]_fetch" = (%"sub_$A")[i1];
; CHECK:           |   %"sub_$B[]_fetch" = (%"sub_$B")[i1];
; CHECK:           |   %add = %"sub_$A[]_fetch"  +  %"sub_$B[]_fetch";
; CHECK:           |   %add8 = %add  +  1.000000e+00;
; CHECK:           |   (%"sub_$C")[i1] = %add8;
; CHECK:           + END LOOP
;
; CHECK:           (%addressof)[0][0] = 26;
; CHECK:           (%addressof)[0][1] = 5;
; CHECK:           (%addressof)[0][2] = 1;
; CHECK:           (%addressof)[0][3] = 0;
; CHECK:           (%ARGBLOCK_0)[0].0 = 4 * %slct;
; CHECK:           (%ARGBLOCK_0)[0].1 = &((%"sub_$A")[0]);
; CHECK:           %func_result40 = @for_write_seq_lis(&((%"var$1")[0]),  -1,  1239157112576,  &((%addressof)[0][0]),  &((%ARGBLOCK_0)[0]));
; CHECK:           ret ;
; CHECK:     END REGION

;*** IR Dump After HIR Dead Store Elimination ***
; CHECK:     BEGIN REGION { modified }
; CHECK-NOT:       + DO i1 = 0
;
; CHECK:           (%addressof)[0][0] = 26;
; CHECK:           (%addressof)[0][1] = 5;
; CHECK:           (%addressof)[0][2] = 1;
; CHECK:           (%addressof)[0][3] = 0;
; CHECK:           (%ARGBLOCK_0)[0].0 = 4 * %slct;
; CHECK:           (%ARGBLOCK_0)[0].1 = &((%"sub_$A")[0]);
; CHECK:           %func_result40 = @for_write_seq_lis(&((%"var$1")[0]),  -1,  1239157112576,  &((%addressof)[0][0]),  &((%ARGBLOCK_0)[0]));
; CHECK:           ret ;
; CHECK:     END REGION

; ModuleID = 'unusedArray.f90'
source_filename = "unusedArray.f90"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @sub_(ptr noalias %"sub_$A", ptr noalias nocapture readonly %"sub_$B", ptr noalias nocapture readonly %"sub_$N") local_unnamed_addr #0 {
alloca:
  %"var$1" = alloca [8 x i64], align 16
  %addressof = alloca [4 x i8], align 1
  %ARGBLOCK_0 = alloca { i64, ptr }, align 8
  %"sub_$N_fetch" = load i32, ptr %"sub_$N", align 4
  %int_sext = sext i32 %"sub_$N_fetch" to i64
  %rel = icmp sgt i64 %int_sext, 0
  %slct = select i1 %rel, i64 %int_sext, i64 0
  %mul = shl nuw nsw i64 %slct, 2
  %"sub_$C" = alloca float, i64 %slct, align 4
  %rel1354 = icmp slt i32 %"sub_$N_fetch", 1
  br i1 %rel1354, label %bb22, label %bb13.preheader

bb13.preheader:                                   ; preds = %alloca
  br label %bb13

bb13:                                             ; preds = %bb13.preheader, %bb13
  %"var$3.055" = phi i64 [ %add18, %bb13 ], [ 1, %bb13.preheader ]
  %"sub_$A[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) %"sub_$A", i64 %"var$3.055")
  %"sub_$A[]_fetch" = load float, ptr %"sub_$A[]", align 4
  %"sub_$B[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) %"sub_$B", i64 %"var$3.055")
  %"sub_$B[]_fetch" = load float, ptr %"sub_$B[]", align 4
  %add = fadd float %"sub_$A[]_fetch", %"sub_$B[]_fetch"
  %add8 = fadd float %add, 1.000000e+00
  %"sub_$C[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) nonnull %"sub_$C", i64 %"var$3.055")
  store float %add8, ptr %"sub_$C[]", align 4
  %add18 = add nuw nsw i64 %"var$3.055", 1
  %exitcond = icmp eq i64 %"var$3.055", %int_sext
  br i1 %exitcond, label %bb22.loopexit, label %bb13

bb22.loopexit:                                    ; preds = %bb13
  br label %bb22

bb22:                                             ; preds = %bb22.loopexit, %alloca
  %.fca.0.gep = getelementptr inbounds [4 x i8], ptr %addressof, i64 0, i64 0
  store i8 26, ptr %.fca.0.gep, align 1
  %.fca.1.gep = getelementptr inbounds [4 x i8], ptr %addressof, i64 0, i64 1
  store i8 5, ptr %.fca.1.gep, align 1
  %.fca.2.gep = getelementptr inbounds [4 x i8], ptr %addressof, i64 0, i64 2
  store i8 1, ptr %.fca.2.gep, align 1
  %.fca.3.gep = getelementptr inbounds [4 x i8], ptr %addressof, i64 0, i64 3
  store i8 0, ptr %.fca.3.gep, align 1
  %BLKFIELD_ = getelementptr inbounds { i64, ptr }, ptr %ARGBLOCK_0, i64 0, i32 0
  store i64 %mul, ptr %BLKFIELD_, align 8
  %BLKFIELD_32 = getelementptr inbounds { i64, ptr }, ptr %ARGBLOCK_0, i64 0, i32 1
  %0 = bitcast ptr %BLKFIELD_32 to ptr
  store ptr %"sub_$A", ptr %0, align 8
  %ptr_cast34 = bitcast [8 x i64]* %"var$1" to ptr
  %ptr_cast38 = bitcast ptr %ARGBLOCK_0 to ptr
  %func_result40 = call i32 (ptr, i32, i64, ptr, ptr, ...) @for_write_seq_lis(ptr nonnull %ptr_cast34, i32 -1, i64 1239157112576, ptr nonnull %.fca.0.gep, ptr nonnull %ptr_cast38)
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #1

declare i32 @for_write_seq_lis(ptr, i32, i64, ptr, ptr, ...) local_unnamed_addr

attributes #0 = { "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+mpx,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #1 = { nounwind readnone speculatable }

;!omp_offload.info = !{}
