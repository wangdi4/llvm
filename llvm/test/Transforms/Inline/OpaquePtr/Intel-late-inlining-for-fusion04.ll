; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; RUN: opt -opaque-pointers -passes='cgscc(inline)' -pre-lto-inline-cost=false -inlining-for-fusion-heuristics=true -inline-threshold=20 -inline-for-fusion-min-arg-refs=3 -dtrans-inline-heuristics=true -inline-for-fusion-small-app-function-limit=0 -inline-report=0xe807 < %s -S 2>&1 | FileCheck --check-prefix=CHECK-NEW %s
; RUN: opt -opaque-pointers -inlinereportsetup -inline-report=0xe886 < %s -S | opt -passes='cgscc(inline)' -pre-lto-inline-cost=false -inlining-for-fusion-heuristics=true -inline-threshold=20 -inline-for-fusion-min-arg-refs=3 -dtrans-inline-heuristics=true -inline-for-fusion-small-app-function-limit=0 -inline-report=0xe886 | opt -inlinereportemitter -inline-report=0xe886 -S 2>&1 | FileCheck --check-prefix=CHECK-META %s

; Test link compile step loop fusion heuristic does not engage because
; -inline-for-fusion-small-app-function-limit=0, implying this is a small app.

; Checks for old pass manager with old inline report

; CHECK-OLD: define void @bar_
; CHECK-OLD: define void @foo_
; CHECK-OLD: call void @baz_
; CHECK-OLD: call void @bar_
; CHECK-OLD: call void @bar_
; CHECK-OLD: call void @bar_
; CHECK-OLD: define void @MAIN__
; CHECK-OLD: call void @foo_
; CHECK-OLD: define void @baz_

; CHECK-OLD: COMPILE FUNC: bar_
; CHECK-OLD: COMPILE FUNC: baz_
; CHECK-OLD: COMPILE FUNC: foo_
; CHECK-OLD: baz_{{.*}}Inlining is not profitable
; CHECK-OLD: bar_{{.*}}Inlining is not profitable
; CHECK-OLD: bar_{{.*}}Inlining is not profitable
; CHECK-OLD: COMPILE FUNC: MAIN__
; CHECK-OLD: foo_{{.*}}Inlining is not profitable

; Checks for new pass manager with old inline report

; CHECK-NEW: define void @bar_
; CHECK-NEW: define void @foo_
; CHECK-NEW: call void @baz_
; CHECK-NEW: call void @bar_
; CHECK-NEW: call void @bar_
; CHECK-NEW: call void @bar_
; CHECK-NEW: define void @MAIN__
; CHECK-NEW: call void @foo_
; CHECK-NEW: define void @baz_

; CHECK-NEW: COMPILE FUNC: bar_
; CHECK-NEW: COMPILE FUNC: baz_
; CHECK-NEW: COMPILE FUNC: foo_
; CHECK-NEW: baz_{{.*}}Inlining is not profitable
; CHECK-NEW: bar_{{.*}}Inlining is not profitable
; CHECK-NEW: bar_{{.*}}Inlining is not profitable
; CHECK-NEW: bar_{{.*}}Inlining is not profitable
; CHECK-NEW: COMPILE FUNC: MAIN__
; CHECK-NEW: foo_{{.*}}Inlining is not profitable

; Checks for old and new pass manager with metadata inline report

; CHECK-META: COMPILE FUNC: bar_
; CHECK-META: COMPILE FUNC: foo_
; CHECK-META: baz_{{.*}}Inlining is not profitable
; CHECK-META: bar_{{.*}}Inlining is not profitable
; CHECK-META: bar_{{.*}}Inlining is not profitable
; CHECK-META: COMPILE FUNC: MAIN__
; CHECK-META: foo_{{.*}}Inlining is not profitable
; CHECK-META: COMPILE FUNC: baz_

; CHECK-META: define void @bar_
; CHECK-META: define void @foo_
; CHECK-META: call void @baz_
; CHECK-META: call void @bar_
; CHECK-META: call void @bar_
; CHECK-META: call void @bar_
; CHECK-META: define void @MAIN__
; CHECK-META: call void @foo_
; CHECK-META: define void @baz_

@"foo_$D" = internal global [100 x [100 x [100 x float]]] zeroinitializer, align 16
@"foo_$C" = internal global [100 x [100 x [100 x float]]] zeroinitializer, align 16
@"foo_$B" = internal global [100 x [100 x [100 x float]]] zeroinitializer, align 16
@"foo_$A" = internal global [100 x [100 x [100 x float]]] zeroinitializer, align 16
@0 = internal unnamed_addr constant i32 100
@1 = internal unnamed_addr constant i32 100
@2 = internal unnamed_addr constant i32 100
@3 = internal unnamed_addr constant i32 100
@4 = internal unnamed_addr constant i32 2

declare i32 @for_write_seq_lis(ptr, i32, i64, ptr, ptr, ...)

declare i32 @for_write_seq_lis_xmit(ptr, ptr, ptr)

declare i32 @for_set_reentrancy(ptr)

define void @bar_(ptr noalias %"bar_$A", ptr noalias %"bar_$B", ptr noalias %"bar_$C", ptr noalias %"bar_$D", ptr noalias %"bar_$N") local_unnamed_addr {
alloca:
  %"bar_$N_fetch" = load i32, ptr %"bar_$N", align 4
  %int_sext = sext i32 %"bar_$N_fetch" to i64
  %mul = shl nsw i64 %int_sext, 2
  %mul17 = mul nsw i64 %mul, %int_sext
  %sub = add nsw i32 %"bar_$N_fetch", -1
  %rel = icmp slt i32 %"bar_$N_fetch", 3
  br i1 %rel, label %bb60, label %bb63

bb63:                                             ; preds = %bb68, %alloca
  %"bar_$I.0" = phi i32 [ 2, %alloca ], [ %add152, %bb68 ]
  %"bar_$N_fetch4" = load i32, ptr %"bar_$N", align 4
  %sub6 = add nsw i32 %"bar_$N_fetch4", -1
  %rel8 = icmp slt i32 %"bar_$N_fetch4", 3
  br i1 %rel8, label %bb68, label %bb67

bb67thread-pre-split:                             ; preds = %bb72
  %"bar_$N_fetch10.pr" = load i32, ptr %"bar_$N", align 4
  br label %bb67

bb67:                                             ; preds = %bb67thread-pre-split, %bb63
  %"bar_$N_fetch10" = phi i32 [ %"bar_$N_fetch10.pr", %bb67thread-pre-split ], [ %"bar_$N_fetch4", %bb63 ]
  %"bar_$J.0" = phi i32 [ 2, %bb63 ], [ %add142, %bb67thread-pre-split ]
  %sub12 = add nsw i32 %"bar_$N_fetch10", -1
  %rel14 = icmp slt i32 %"bar_$N_fetch10", 3
  br i1 %rel14, label %bb72, label %bb71

bb71:                                             ; preds = %bb71, %bb67
  %"bar_$K.0" = phi i32 [ 2, %bb67 ], [ %add132, %bb71 ]
  %int_sext121 = zext i32 %"bar_$I.0" to i64
  %int_sext124 = zext i32 %"bar_$J.0" to i64
  %int_sext127 = zext i32 %"bar_$K.0" to i64
  %"bar_$A[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 %mul17, ptr elementtype(float) %"bar_$A", i64 %int_sext127)
  %"bar_$A[][]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul, ptr elementtype(float) %"bar_$A[]", i64 %int_sext124)
  %"bar_$A[][][]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) %"bar_$A[][]", i64 %int_sext121)
  %"bar_$A[][][]_fetch" = load float, ptr %"bar_$A[][][]", align 4
  %"bar_$B[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 %mul17, ptr elementtype(float) %"bar_$B", i64 %int_sext121)
  %"bar_$B[][]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul, ptr elementtype(float) %"bar_$B[]", i64 %int_sext127)
  %"bar_$B[][][]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) %"bar_$B[][]", i64 %int_sext124)
  %"bar_$B[][][]_fetch" = load float, ptr %"bar_$B[][][]", align 4
  %add46 = fadd float %"bar_$A[][][]_fetch", %"bar_$B[][][]_fetch"
  %"bar_$C[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 %mul17, ptr elementtype(float) %"bar_$C", i64 %int_sext124)
  %"bar_$C[][]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul, ptr elementtype(float) %"bar_$C[]", i64 %int_sext121)
  %"bar_$C[][][]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) %"bar_$C[][]", i64 %int_sext127)
  %"bar_$C[][][]_fetch" = load float, ptr %"bar_$C[][][]", align 4
  %add75 = fadd float %add46, %"bar_$C[][][]_fetch"
  %"bar_$D[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 %mul17, ptr elementtype(float) %"bar_$D", i64 %int_sext127)
  %"bar_$D[][]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul, ptr elementtype(float) %"bar_$D[]", i64 %int_sext124)
  %"bar_$D[][][]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) %"bar_$D[][]", i64 %int_sext121)
  %"bar_$D[][][]_fetch" = load float, ptr %"bar_$D[][][]", align 4
  %add104 = fadd float %add75, %"bar_$D[][][]_fetch"
  store float %add104, ptr %"bar_$A[][][]", align 4
  %add132 = add nuw nsw i32 %"bar_$K.0", 1
  %rel138 = icmp slt i32 %"bar_$K.0", %sub12
  br i1 %rel138, label %bb71, label %bb72

bb72:                                             ; preds = %bb71, %bb67
  %add142 = add nuw nsw i32 %"bar_$J.0", 1
  %rel148 = icmp slt i32 %"bar_$J.0", %sub6
  br i1 %rel148, label %bb67thread-pre-split, label %bb68

bb68:                                             ; preds = %bb72, %bb63
  %add152 = add nuw nsw i32 %"bar_$I.0", 1
  %rel158 = icmp slt i32 %"bar_$I.0", %sub
  br i1 %rel158, label %bb63, label %bb60

bb60:                                             ; preds = %bb68, %alloca
  ret void
}

define void @foo_() local_unnamed_addr {
alloca:
  %"var$1" = alloca [8 x i64], align 16
  %addressof = alloca [4 x i8], align 1
  %ARGBLOCK_0 = alloca { float }, align 8
  %addressof11 = alloca [4 x i8], align 1
  %ARGBLOCK_1 = alloca { float }, align 8
  %addressof28 = alloca [4 x i8], align 1
  %ARGBLOCK_2 = alloca { float }, align 8
  %addressof45 = alloca [4 x i8], align 1
  %ARGBLOCK_3 = alloca { float }, align 8
  call void @baz_(ptr @"foo_$A", ptr @"foo_$B", ptr @"foo_$C", ptr @"foo_$D", ptr nonnull @0)
  call void @bar_(ptr @"foo_$A", ptr @"foo_$B", ptr @"foo_$C", ptr @"foo_$D", ptr nonnull @1)
  call void @bar_(ptr @"foo_$A", ptr @"foo_$B", ptr @"foo_$C", ptr @"foo_$D", ptr nonnull @2)
  call void @bar_(ptr @"foo_$A", ptr @"foo_$B", ptr @"foo_$C", ptr @"foo_$D", ptr nonnull @3)
  %"foo_$A[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 40000, ptr elementtype(float) @"foo_$A", i64 1)
  %"foo_$A[][]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 400, ptr elementtype(float) %"foo_$A[]", i64 1)
  %"foo_$A[][][]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) %"foo_$A[][]", i64 1)
  %"foo_$A[][][]_fetch68" = load i32, ptr %"foo_$A[][][]", align 4
  %.fca.0.gep64 = getelementptr inbounds [4 x i8], ptr %addressof, i64 0, i64 0
  store i8 26, ptr %.fca.0.gep64, align 1
  %.fca.1.gep65 = getelementptr inbounds [4 x i8], ptr %addressof, i64 0, i64 1
  store i8 1, ptr %.fca.1.gep65, align 1
  %.fca.2.gep66 = getelementptr inbounds [4 x i8], ptr %addressof, i64 0, i64 2
  store i8 2, ptr %.fca.2.gep66, align 1
  %.fca.3.gep67 = getelementptr inbounds [4 x i8], ptr %addressof, i64 0, i64 3
  store i8 0, ptr %.fca.3.gep67, align 1
  store i32 %"foo_$A[][][]_fetch68", ptr %ARGBLOCK_0, align 8
  %ptr_cast2 = getelementptr inbounds [4 x i8], ptr %addressof, i64 0, i64 0
  %func_result = call i32 (ptr, i32, i64, ptr, ptr, ...) @for_write_seq_lis(ptr nonnull %"var$1", i32 -1, i64 1239157112576, ptr nonnull %ptr_cast2, ptr nonnull %ARGBLOCK_0)
  %"foo_$B[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 40000, ptr elementtype(float) @"foo_$B", i64 2)
  %"foo_$B[][]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 400, ptr elementtype(float) %"foo_$B[]", i64 2)
  %"foo_$B[][][]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) %"foo_$B[][]", i64 2)
  %"foo_$B[][][]_fetch69" = load i32, ptr %"foo_$B[][][]", align 4
  %.fca.0.gep60 = getelementptr inbounds [4 x i8], ptr %addressof11, i64 0, i64 0
  store i8 26, ptr %.fca.0.gep60, align 1
  %.fca.1.gep61 = getelementptr inbounds [4 x i8], ptr %addressof11, i64 0, i64 1
  store i8 1, ptr %.fca.1.gep61, align 1
  %.fca.2.gep62 = getelementptr inbounds [4 x i8], ptr %addressof11, i64 0, i64 2
  store i8 2, ptr %.fca.2.gep62, align 1
  %.fca.3.gep63 = getelementptr inbounds [4 x i8], ptr %addressof11, i64 0, i64 3
  store i8 0, ptr %.fca.3.gep63, align 1
  store i32 %"foo_$B[][][]_fetch69", ptr %ARGBLOCK_1, align 8
  %ptr_cast17 = getelementptr inbounds [4 x i8], ptr %addressof11, i64 0, i64 0
  %func_result21 = call i32 @for_write_seq_lis_xmit(ptr nonnull %"var$1", ptr nonnull %ptr_cast17, ptr nonnull %ARGBLOCK_1)
  %"foo_$C[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 40000, ptr elementtype(float) @"foo_$C", i64 3)
  %"foo_$C[][]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 400, ptr elementtype(float) %"foo_$C[]", i64 3)
  %"foo_$C[][][]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) %"foo_$C[][]", i64 3)
  %"foo_$C[][][]_fetch70" = load i32, ptr %"foo_$C[][][]", align 4
  %.fca.0.gep56 = getelementptr inbounds [4 x i8], ptr %addressof28, i64 0, i64 0
  store i8 26, ptr %.fca.0.gep56, align 1
  %.fca.1.gep57 = getelementptr inbounds [4 x i8], ptr %addressof28, i64 0, i64 1
  store i8 1, ptr %.fca.1.gep57, align 1
  %.fca.2.gep58 = getelementptr inbounds [4 x i8], ptr %addressof28, i64 0, i64 2
  store i8 2, ptr %.fca.2.gep58, align 1
  %.fca.3.gep59 = getelementptr inbounds [4 x i8], ptr %addressof28, i64 0, i64 3
  store i8 0, ptr %.fca.3.gep59, align 1
  store i32 %"foo_$C[][][]_fetch70", ptr %ARGBLOCK_2, align 8
  %ptr_cast34 = getelementptr inbounds [4 x i8], ptr %addressof28, i64 0, i64 0
  %func_result38 = call i32 @for_write_seq_lis_xmit(ptr nonnull %"var$1", ptr nonnull %ptr_cast34, ptr nonnull %ARGBLOCK_2)
  %"foo_$D[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 40000, ptr elementtype(float) @"foo_$D", i64 4)
  %"foo_$D[][]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 400, ptr elementtype(float) %"foo_$D[]", i64 4)
  %"foo_$D[][][]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) %"foo_$D[][]", i64 4)
  %"foo_$D[][][]_fetch71" = load i32, ptr %"foo_$D[][][]", align 4
  %.fca.0.gep = getelementptr inbounds [4 x i8], ptr %addressof45, i64 0, i64 0
  store i8 26, ptr %.fca.0.gep, align 1
  %.fca.1.gep = getelementptr inbounds [4 x i8], ptr %addressof45, i64 0, i64 1
  store i8 1, ptr %.fca.1.gep, align 1
  %.fca.2.gep = getelementptr inbounds [4 x i8], ptr %addressof45, i64 0, i64 2
  store i8 1, ptr %.fca.2.gep, align 1
  %.fca.3.gep = getelementptr inbounds [4 x i8], ptr %addressof45, i64 0, i64 3
  store i8 0, ptr %.fca.3.gep, align 1
  store i32 %"foo_$D[][][]_fetch71", ptr %ARGBLOCK_3, align 8
  %ptr_cast51 = getelementptr inbounds [4 x i8], ptr %addressof45, i64 0, i64 0
  %func_result55 = call i32 @for_write_seq_lis_xmit(ptr nonnull %"var$1", ptr nonnull %ptr_cast51, ptr nonnull %ARGBLOCK_3)
  ret void
}

define void @MAIN__() local_unnamed_addr {
alloca:
  %func_result = call i32 @for_set_reentrancy(ptr nonnull @4)
  call void @foo_()
  ret void
}

define void @baz_(ptr noalias nocapture %"baz_$A", ptr noalias nocapture %"baz_$B", ptr noalias nocapture %"baz_$C", ptr noalias nocapture %"baz_$D", ptr noalias nocapture readonly %"baz_$N") local_unnamed_addr {
alloca:
  %"baz_$N_fetch" = load i32, ptr %"baz_$N", align 4
  %int_sext = sext i32 %"baz_$N_fetch" to i64
  %mul = shl nsw i64 %int_sext, 2
  %mul13 = mul nsw i64 %mul, %int_sext
  %rel = icmp slt i32 %"baz_$N_fetch", 1
  br i1 %rel, label %bb114, label %bb117

bb117thread-pre-split:                            ; preds = %bb122
  %"baz_$N_fetch4.pr" = load i32, ptr %"baz_$N", align 4
  br label %bb117

bb117:                                            ; preds = %bb117thread-pre-split, %alloca
  %"baz_$N_fetch4" = phi i32 [ %"baz_$N_fetch4.pr", %bb117thread-pre-split ], [ %"baz_$N_fetch", %alloca ]
  %"baz_$I.0" = phi i32 [ 1, %alloca ], [ %add107, %bb117thread-pre-split ]
  %rel6 = icmp slt i32 %"baz_$N_fetch4", 1
  br i1 %rel6, label %bb122, label %bb121

bb121:                                            ; preds = %bb126, %bb117
  %"baz_$J.0" = phi i32 [ 1, %bb117 ], [ %add97, %bb126 ]
  %"baz_$N_fetch8" = load i32, ptr %"baz_$N", align 4
  %rel10 = icmp slt i32 %"baz_$N_fetch8", 1
  br i1 %rel10, label %bb126, label %bb125

bb125:                                            ; preds = %bb125, %bb121
  %"baz_$K.0" = phi i32 [ 1, %bb121 ], [ %add87, %bb125 ]
  %int_sext20 = zext i32 %"baz_$I.0" to i64
  %int_sext22 = zext i32 %"baz_$J.0" to i64
  %int_sext23 = zext i32 %"baz_$K.0" to i64
  %"baz_$A[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 %mul13, ptr elementtype(float) %"baz_$A", i64 %int_sext23)
  %"baz_$A[][]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul, ptr elementtype(float) %"baz_$A[]", i64 %int_sext22)
  %"baz_$A[][][]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) %"baz_$A[][]", i64 %int_sext20)
  store float 1.000000e+00, ptr %"baz_$A[][][]", align 4
  %"baz_$B[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 %mul13, ptr elementtype(float) %"baz_$B", i64 %int_sext23)
  %"baz_$B[][]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul, ptr elementtype(float) %"baz_$B[]", i64 %int_sext22)
  %"baz_$B[][][]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) %"baz_$B[][]", i64 %int_sext20)
  store float 2.000000e+00, ptr %"baz_$B[][][]", align 4
  %"baz_$C[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 %mul13, ptr elementtype(float) %"baz_$C", i64 %int_sext23)
  %"baz_$C[][]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul, ptr elementtype(float) %"baz_$C[]", i64 %int_sext22)
  %"baz_$C[][][]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) %"baz_$C[][]", i64 %int_sext20)
  store float 3.000000e+00, ptr %"baz_$C[][][]", align 4
  %"baz_$D[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 %mul13, ptr elementtype(float) %"baz_$D", i64 %int_sext23)
  %"baz_$D[][]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul, ptr elementtype(float) %"baz_$D[]", i64 %int_sext22)
  %"baz_$D[][][]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) %"baz_$D[][]", i64 %int_sext20)
  store float 4.000000e+00, ptr %"baz_$D[][][]", align 4
  %add87 = add nuw nsw i32 %"baz_$K.0", 1
  %rel93 = icmp slt i32 %"baz_$K.0", %"baz_$N_fetch8"
  br i1 %rel93, label %bb125, label %bb126

bb126:                                            ; preds = %bb125, %bb121
  %add97 = add nuw nsw i32 %"baz_$J.0", 1
  %rel103 = icmp slt i32 %"baz_$J.0", %"baz_$N_fetch4"
  br i1 %rel103, label %bb121, label %bb122

bb122:                                            ; preds = %bb126, %bb117
  %add107 = add nuw nsw i32 %"baz_$I.0", 1
  %rel113 = icmp slt i32 %"baz_$I.0", %"baz_$N_fetch"
  br i1 %rel113, label %bb117thread-pre-split, label %bb114

bb114:                                            ; preds = %bb122, %alloca
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #0

attributes #0 = { nounwind readnone speculatable }
; end INTEL_FEATURE_SW_ADVANCED
