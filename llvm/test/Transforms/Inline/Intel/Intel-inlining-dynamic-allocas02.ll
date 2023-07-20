; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; RUN: opt -passes='cgscc(inline)'  -dtrans-inline-heuristics=false -pre-lto-inline-cost=false -inline-report=0xe807 < %s -S 2>&1 | FileCheck --check-prefix=CHECK-NEW %s
; RUN: opt -passes='inlinereportsetup,cgscc(inline),inlinereportemitter' -dtrans-inline-heuristics=false -pre-lto-inline-cost=false -inline-report=0xe886 -S < %s 2>&1 | FileCheck --check-prefix=CHECK-OLD %s

; Check that without -dtrans-inline-heuristics that @foo, which has a dynamic
; alloca, is not inlined.

; Checks for old pass manager with old inline report

; CHECK-OLD: COMPILE FUNC: foo_
; CHECK-OLD: COMPILE FUNC: MAIN__
; CHECK-OLD-NOT: INLINE: foo_{{.*}}Inlining is profitable>>

; CHECK-OLD: define float @foo_
; CHECK-OLD: define void @MAIN__
; CHECK-OLD: call float @foo_

; Checks for old pass manager with metadata inline report and new pass manager

; CHECK-NEW: define float @foo_
; CHECK-NEW: define void @MAIN__
; CHECK-NEW: call float @foo_

; CHECK-NEW: COMPILE FUNC: foo_
; CHECK-NEW: COMPILE FUNC: MAIN__
; CHECK-NEW-NOT: INLINE: foo_{{.*}}Inlining is profitable>>

@0 = internal unnamed_addr constant i32 2
@1 = internal unnamed_addr constant i32 1000

declare dso_local i32 @for_set_reentrancy(ptr) local_unnamed_addr

declare dso_local i32 @for_write_seq_lis(ptr, i32, i64, ptr, ptr, ...) local_unnamed_addr

define float @foo_(ptr noalias %"foo_$N") local_unnamed_addr {
alloca:
  %"foo_$N_fetch" = load i32, ptr %"foo_$N", align 4
  %int_sext = sext i32 %"foo_$N_fetch" to i64
  %rel = icmp sgt i64 %int_sext, 0
  %slct = select i1 %rel, i64 %int_sext, i64 0
  %"foo_$A" = alloca float, i64 %slct, align 4
  %rel4 = icmp slt i32 %"foo_$N_fetch", 1
  br i1 %rel4, label %bb12, label %bb5

bb5:                                              ; preds = %bb5, %alloca
  %"foo_$I.0" = phi i32 [ 1, %alloca ], [ %add12, %bb5 ]
  %add = add nuw nsw i32 %"foo_$I.0", 45
  %int_cast = sitofp i32 %add to float
  %int_sext8 = zext i32 %"foo_$I.0" to i64
  %"foo_$A[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"foo_$A", i64 %int_sext8)
  store float %int_cast, ptr %"foo_$A[]", align 4
  %add12 = add nuw nsw i32 %"foo_$I.0", 1
  %rel18 = icmp slt i32 %"foo_$I.0", %"foo_$N_fetch"
  br i1 %rel18, label %bb5, label %bb12

bb12:                                             ; preds = %bb5, %alloca
  %"foo_$N_fetch22" = load i32, ptr %"foo_$N", align 4
  %int_sext23 = sext i32 %"foo_$N_fetch22" to i64
  %"foo_$A[]21" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"foo_$A", i64 %int_sext23)
  %"foo_$A[]21_fetch" = load float, ptr %"foo_$A[]21", align 4
  ret float %"foo_$A[]21_fetch"
}

define void @MAIN__() local_unnamed_addr {
alloca:
  %"var$4" = alloca [8 x i64], align 16
  %addressof = alloca [4 x i8], align 1
  %ARGBLOCK_0 = alloca { float }, align 8
  %func_result = call i32 @for_set_reentrancy(ptr nonnull @0)
  %func_result2 = call float @foo_(ptr nonnull @1)
  %.fca.0.gep = getelementptr inbounds [4 x i8], ptr %addressof, i64 0, i64 0
  store i8 26, ptr %.fca.0.gep, align 1
  %.fca.1.gep = getelementptr inbounds [4 x i8], ptr %addressof, i64 0, i64 1
  store i8 1, ptr %.fca.1.gep, align 1
  %.fca.2.gep = getelementptr inbounds [4 x i8], ptr %addressof, i64 0, i64 2
  store i8 1, ptr %.fca.2.gep, align 1
  %.fca.3.gep = getelementptr inbounds [4 x i8], ptr %addressof, i64 0, i64 3
  store i8 0, ptr %.fca.3.gep, align 1
  %BLKFIELD_ = getelementptr inbounds { float }, ptr %ARGBLOCK_0, i64 0, i32 0
  store float %func_result2, ptr %BLKFIELD_, align 8
  %ptr_cast4 = getelementptr inbounds [4 x i8], ptr %addressof, i64 0, i64 0
  %func_result8 = call i32 (ptr, i32, i64, ptr, ptr, ...) @for_write_seq_lis(ptr nonnull %"var$4", i32 -1, i64 1239157112576, ptr nonnull %ptr_cast4, ptr nonnull %ARGBLOCK_0)
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #0

attributes #0 = { nounwind readnone speculatable }
; end INTEL_FEATURE_SW_ADVANCED
