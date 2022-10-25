; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced,asserts
; RUN: opt < %s -S -passes='module(ip-cloning)' -ip-spec-cloning-min-loops=1 -ip-gen-cloning-force-enable-dtrans -debug-only=ipcloning 2>&1 | FileCheck %s

; Check that many loops specialization cloning was performed.

; Check the trace rejects non-matching conditions, but finds the desired
; matching condition.

; CHECK: MLSC: Testing aer_rad_props_mp_aer_rad_props_sw_:
; CHECK: MLSC: Arg(0): ArgUse(0): Missing minimal GEPI conditions
; CHECK: MLSC: Arg(1): ArgUse(0): Missing minimal GEPI conditions
; CHECK: MLSC: Arg(1): ArgUse(1): LoadUse(0): Missing CastInst or more than one use
; CHECK: MLSC: Arg(1): ArgUse(1): LoadUse(1): Good partial result
; CHECK: MLSC: Arg(1): ArgUse(1): LoadUse(2): Missing CastInst or more than one use
; CHECK: MLSC: Arg(1): ArgUse(1): FOUND MLSC CANDIDATE
; CHECK: Selected ManyLoopSpecialization cloning

; Check the specialization test in the output

; CHECK: define void @mymain(%"PHYSICS_TYPES$.btPHYSICS_STATE"* nonnull %0)
; CHECK: %[[S2:[0-9]+]] = getelementptr %"PHYSICS_TYPES$.btPHYSICS_STATE", %"PHYSICS_TYPES$.btPHYSICS_STATE"* %0, i32 0, i32 1
; CHECK: %[[S3:[0-9]+]] = load i32, i32* %[[S2]], align 4
; CHECK: %[[S4:[0-9]+]] = icmp eq i32 %[[S3]], 4
; CHECK: br i1 %[[S4]], label %[[L5:[0-9]+]], label %[[L6:[0-9]+]]
; CHECK: [[L5]]:
; CHECK:  call void @aer_rad_props_mp_aer_rad_props_sw_(i32* nonnull @anon, %"PHYSICS_TYPES$.btPHYSICS_STATE"* nonnull %0)
; CHECK:  br label %[[L7:[0-9]+]]
; CHECK: [[L6]]:
; CHECK:  call void @aer_rad_props_mp_aer_rad_props_sw_.1(i32* nonnull @anon, %"PHYSICS_TYPES$.btPHYSICS_STATE"* nonnull %0)
; CHECK:  br label %[[L7]]
; CHECK: [[L7]]:
; CHECK:  ret void

; Check that the original was optimized

; CHECK: define internal void @aer_rad_props_mp_aer_rad_props_sw_
; CHECK: %[[S25:[A-Za-z0-9]+]] = alloca i32, align 8
; CHECK: store i32 4, i32* %[[S25]], align 8
; CHECK: sext i32 4 to i64

; Check that the clone was not optimizied

; CHECK: define internal void @aer_rad_props_mp_aer_rad_props_sw_.1
; CHECK-NOT: store i32 4, i32* %[[S25]], align 8
; CHECK-NOT: sext i32 4 to i64

@anon = internal unnamed_addr constant i32 0
@physconst_mp_rga_ = internal global double 0x3FBA1B256713BBEC, align 8

declare double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 %0, i64 %1, i64 %2, double* %3, i64 %4)

%"PHYSICS_TYPES$.btPHYSICS_STATE" = type { i32, i32, [4 x double], [4 x double], [4 x double], [4 x double], [4 x double], [4 x double], [4 x double], [26 x [4 x double]], [26 x [4 x double]], [26 x [4 x double]], [26 x [4 x double]], [26 x [4 x double]], [26 x [4 x double]], [26 x [4 x double]], [26 x [4 x double]], [26 x [4 x double]], [26 x [4 x double]], [26 x [4 x double]], [26 x [4 x double]], [26 x [4 x double]], [26 x [4 x double]], [26 x [4 x double]], [26 x [4 x double]], [3 x [26 x [4 x double]]], [27 x [4 x double]], [27 x [4 x double]], [27 x [4 x double]], [27 x [4 x double]], [27 x [4 x double]], [4 x double], [4 x double], [4 x double], [4 x double], i32, [4 x i32], [4 x i32], [4 x i32], i32, i32 }

define void @mymain(%"PHYSICS_TYPES$.btPHYSICS_STATE"* nonnull %0) #0 {
  call void @aer_rad_props_mp_aer_rad_props_sw_(i32* nonnull @anon, %"PHYSICS_TYPES$.btPHYSICS_STATE"* nonnull %0)
  ret void
}

define internal void @aer_rad_props_mp_aer_rad_props_sw_(i32* noalias nocapture readonly dereferenceable(4) %0, %"PHYSICS_TYPES$.btPHYSICS_STATE"* noalias nocapture readonly dereferenceable(20552) %1) #0 {
L0:
  %t0 = load i32, i32* %0
  %t19 = alloca [26 x [4 x double]], align 32
  %t25 = alloca i32, align 8
  %t73 = getelementptr inbounds %"PHYSICS_TYPES$.btPHYSICS_STATE", %"PHYSICS_TYPES$.btPHYSICS_STATE"* %1, i64 0, i32 1
  %t74 = load i32, i32* %t73, align 1
  store i32 %t74, i32* %t25, align 8
  %t77 = sext i32 %t74 to i64
  %t78 = load double, double* @physconst_mp_rga_, align 8
  %t79 = getelementptr inbounds %"PHYSICS_TYPES$.btPHYSICS_STATE", %"PHYSICS_TYPES$.btPHYSICS_STATE"* %1, i64 0, i32 17, i64 0, i64 0
  %t80 = icmp slt i32 %t74, 1
  %t81 = getelementptr inbounds [26 x [4 x double]], [26 x [4 x double]]* %t19, i64 0, i64 0, i64 0
  %t82 = add nsw i64 %t77, 1
  br label %L83
L83:                                               ; preds = %L96, %L1
  %t84 = phi i64 [ %t97, %L96 ], [ 1, %L0 ]
  br i1 %t80, label %L96, label %L85

L85:                                               ; preds = %L83
  %t86 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 32, double* elementtype(double) nonnull %t79, i64 %t84)
  %t87 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 32, double* elementtype(double) nonnull %t81, i64 %t84)
  br label %L88

L88:                                               ; preds = %L88, %L85
  %t89 = phi i64 [ 1, %L85 ], [ %t94, %L88 ]
  %t90 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) nonnull %t86, i64 %t89)
  %t91 = load double, double* %t90, align 1
  %t92 = fmul fast double %t91, %t78
  %t93 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) nonnull %t87, i64 %t89)
  store double %t92, double* %t93, align 1
  %t94 = add nuw nsw i64 %t89, 1
  %t95 = icmp eq i64 %t94, %t82
  br i1 %t95, label %L96, label %L88

L96:                                               ; preds = %L88, %L83
  %t97 = add nuw nsw i64 %t84, 1
  %t98 = icmp eq i64 %t97, 27
  br i1 %t98, label %L99, label %L83

L99:                                               ; preds = %L96
  ret void
}

attributes #0 = { "intel-lang"="fortran" }
; end INTEL_FEATURE_SW_ADVANCED
