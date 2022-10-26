; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; RUN: opt -passes='cgscc(inline)'  -dtrans-inline-heuristics -intel-libirc-allowed -pre-lto-inline-cost=false -inline-report=0xe807 < %s -S 2>&1 | FileCheck --check-prefix=CHECK-NEW %s
; RUN: opt -inlinereportsetup -inline-report=0xe886 < %s -S | opt -passes='cgscc(inline)'  -dtrans-inline-heuristics -intel-libirc-allowed -pre-lto-inline-cost=false -inline-report=0x87 -S | opt -inlinereportemitter -inline-report=0xe886 -S 2>&1 | FileCheck --check-prefix=CHECK-OLD %s

target triple = "x86_64-unknown-linux-gnu"

; Check that with -dtrans-inline-heuristics -intel-libirc-allowed that @foo, which has a dynamic
; alloca, can be inlined.

; Checks for old pass manager with old inline report

; CHECK-OLD: COMPILE FUNC: foo_
; CHECK-OLD: COMPILE FUNC: MAIN__
; CHECK-OLD: INLINE: foo_{{.*}}Inlining is profitable>>

; CHECK-OLD: define float @foo_
; CHECK-OLD: define void @MAIN__
; CHECK-OLD-NOT: call float @foo_

; Checks for old pass manager with metadata inline report and new pass manager

; CHECK-NEW: define float @foo_
; CHECK-NEW: define void @MAIN__
; CHECK-NEW-NOT: call float @foo_

; CHECK-NEW: COMPILE FUNC: foo_
; CHECK-NEW: COMPILE FUNC: MAIN__
; CHECK-NEW: INLINE: foo_{{.*}}Inlining is profitable>>

declare float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8, i64, i64, float*, i64)
declare dso_local i32 @for_set_reentrancy(i32*) local_unnamed_addr
declare dso_local i32 @for_write_seq_lis(i8*, i32, i64, i8*, i8*, ...) local_unnamed_addr

@0 = internal unnamed_addr constant i32 2
@1 = internal unnamed_addr constant i32 1000

define float @foo_(i32* noalias %"foo_$N") local_unnamed_addr {
alloca:
  %"foo_$N_fetch" = load i32, i32* %"foo_$N", align 4
  %int_sext = sext i32 %"foo_$N_fetch" to i64
  %rel = icmp sgt i64 %int_sext, 0
  %slct = select i1 %rel, i64 %int_sext, i64 0
  %"foo_$A" = alloca float, i64 %slct, align 4
  %rel4 = icmp slt i32 %"foo_$N_fetch", 1
  br i1 %rel4, label %bb12, label %bb5

bb5:                                              ; preds = %alloca, %bb5
  %"foo_$I.0" = phi i32 [ 1, %alloca ], [ %add12, %bb5 ]
  %add = add nuw nsw i32 %"foo_$I.0", 45
  %int_cast = sitofp i32 %add to float
  %int_sext8 = zext i32 %"foo_$I.0" to i64
  %"foo_$A[]" = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* elementtype(float) nonnull %"foo_$A", i64 %int_sext8)
  store float %int_cast, float* %"foo_$A[]", align 4
  %add12 = add nuw nsw i32 %"foo_$I.0", 1
  %rel18 = icmp slt i32 %"foo_$I.0", %"foo_$N_fetch"
  br i1 %rel18, label %bb5, label %bb12

bb12:                                             ; preds = %alloca, %bb5
  %"foo_$N_fetch22" = load i32, i32* %"foo_$N", align 4
  %int_sext23 = sext i32 %"foo_$N_fetch22" to i64
  %"foo_$A[]21" = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* elementtype(float) nonnull %"foo_$A", i64 %int_sext23)
  %"foo_$A[]21_fetch" = load float, float* %"foo_$A[]21", align 4
  ret float %"foo_$A[]21_fetch"
}

define void @MAIN__() local_unnamed_addr {
alloca:
  %"var$4" = alloca [8 x i64], align 16
  %addressof = alloca [4 x i8], align 1
  %ARGBLOCK_0 = alloca { float }, align 8
  %func_result = call i32 @for_set_reentrancy(i32* nonnull @0)
  %func_result2 = call float @foo_(i32* nonnull @1)
  %.fca.0.gep = getelementptr inbounds [4 x i8], [4 x i8]* %addressof, i64 0, i64 0
  store i8 26, i8* %.fca.0.gep, align 1
  %.fca.1.gep = getelementptr inbounds [4 x i8], [4 x i8]* %addressof, i64 0, i64 1
  store i8 1, i8* %.fca.1.gep, align 1
  %.fca.2.gep = getelementptr inbounds [4 x i8], [4 x i8]* %addressof, i64 0, i64 2
  store i8 1, i8* %.fca.2.gep, align 1
  %.fca.3.gep = getelementptr inbounds [4 x i8], [4 x i8]* %addressof, i64 0, i64 3
  store i8 0, i8* %.fca.3.gep, align 1
  %BLKFIELD_ = getelementptr inbounds { float }, { float }* %ARGBLOCK_0, i64 0, i32 0
  store float %func_result2, float* %BLKFIELD_, align 8
  %ptr_cast = bitcast [8 x i64]* %"var$4" to i8*
  %ptr_cast4 = getelementptr inbounds [4 x i8], [4 x i8]* %addressof, i64 0, i64 0
  %ptr_cast6 = bitcast { float }* %ARGBLOCK_0 to i8*
  %func_result8 = call i32 (i8*, i32, i64, i8*, i8*, ...) @for_write_seq_lis(i8* nonnull %ptr_cast, i32 -1, i64 1239157112576, i8* nonnull %ptr_cast4, i8* nonnull %ptr_cast6)
  ret void
}

; end INTEL_FEATURE_SW_ADVANCED
