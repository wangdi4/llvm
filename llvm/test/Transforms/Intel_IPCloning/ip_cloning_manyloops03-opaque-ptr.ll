; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced,asserts
; RUN: opt -opaque-pointers < %s -S -passes='module(ip-cloning)' -ip-spec-cloning-min-loops=1 -ip-gen-cloning-force-enable-dtrans -debug-only=ipcloning 2>&1 | FileCheck %s

; Check that many loops specialization cloning was NOT performed, because
; the call site is not unique.

; CHECK: MLSC: Testing aer_rad_props_mp_aer_rad_props_sw_:
; MLSC: Testing aer_rad_props_mp_aer_rad_props_sw_: Not a unique call site
; CHECK: Selected Specialization cloning
; CHECK-NOT: Selected ManyLoopSpecialization cloning

; CHECK: define void @mymain({{.*}})
; CHECK: define internal void @aer_rad_props_mp_aer_rad_props_sw_({{.*}})
; CHECK-NOT: define internal void @aer_rad_props_mp_aer_rad_props_sw_.1({{.*}})

@anon = internal unnamed_addr constant i32 0
@physconst_mp_rga_ = internal global double 0x3FBA1B256713BBEC, align 8

%"PHYSICS_TYPES$.btPHYSICS_STATE" = type { i32, i32, [4 x double], [4 x double], [4 x double], [4 x double], [4 x double], [4 x double], [4 x double], [26 x [4 x double]], [26 x [4 x double]], [26 x [4 x double]], [26 x [4 x double]], [26 x [4 x double]], [26 x [4 x double]], [26 x [4 x double]], [26 x [4 x double]], [26 x [4 x double]], [26 x [4 x double]], [26 x [4 x double]], [26 x [4 x double]], [26 x [4 x double]], [26 x [4 x double]], [26 x [4 x double]], [26 x [4 x double]], [3 x [26 x [4 x double]]], [27 x [4 x double]], [27 x [4 x double]], [27 x [4 x double]], [27 x [4 x double]], [27 x [4 x double]], [4 x double], [4 x double], [4 x double], [4 x double], i32, [4 x i32], [4 x i32], [4 x i32], i32, i32 }

define void @mymain(ptr nonnull %arg) #0 {
bb:
  call void @aer_rad_props_mp_aer_rad_props_sw_(ptr nonnull @anon, ptr nonnull %arg)
  call void @aer_rad_props_mp_aer_rad_props_sw_(ptr nonnull @anon, ptr nonnull %arg)
  ret void
}

define internal void @aer_rad_props_mp_aer_rad_props_sw_(ptr noalias nocapture readonly dereferenceable(4) %arg, ptr noalias nocapture readonly dereferenceable(20552) %arg1) #0 {
L0:
  %t19 = alloca [26 x [4 x double]], align 32
  %t25 = alloca i32, align 8
  %t73 = getelementptr inbounds %"PHYSICS_TYPES$.btPHYSICS_STATE", ptr %arg1, i64 0, i32 1
  %t74 = load i32, ptr %t73, align 1
  store i32 %t74, ptr %t25, align 8
  %t77 = sext i32 %t74 to i64
  %t78 = load double, ptr @physconst_mp_rga_, align 8
  %t79 = getelementptr inbounds %"PHYSICS_TYPES$.btPHYSICS_STATE", ptr %arg1, i64 0, i32 17, i64 0, i64 0
  %t80 = icmp slt i32 %t74, 1
  %t81 = getelementptr inbounds [26 x [4 x double]], ptr %t19, i64 0, i64 0, i64 0
  %t82 = add nsw i64 %t77, 2
  br label %L83

L83:                                              ; preds = %L96, %L0
  %t84 = phi i64 [ %t97, %L96 ], [ 1, %L0 ]
  br i1 %t80, label %L96, label %L85

L85:                                              ; preds = %L83
  %t86 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 32, ptr elementtype(i32) nonnull %t79, i64 %t84)
  %t87 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 32, ptr elementtype(i32) nonnull %t81, i64 %t84)
  br label %L88

L88:                                              ; preds = %L88, %L85
  %t89 = phi i64 [ 1, %L85 ], [ %t94, %L88 ]
  %t90 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(i32) nonnull %t86, i64 %t89)
  %t91 = load double, ptr %t90, align 1
  %t92 = fmul fast double %t91, %t78
  %t93 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(i32) nonnull %t87, i64 %t89)
  store double %t92, ptr %t93, align 1
  %t94 = add nuw nsw i64 %t89, 1
  %t95 = icmp eq i64 %t94, %t82
  br i1 %t95, label %L96, label %L88

L96:                                              ; preds = %L88, %L83
  %t97 = add nuw nsw i64 %t84, 1
  %t98 = icmp eq i64 %t97, 27
  br i1 %t98, label %L99, label %L83

L99:                                              ; preds = %L96
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #1

attributes #0 = { "intel-lang"="fortran" }
attributes #1 = { nounwind readnone speculatable }
; end INTEL_FEATURE_SW_ADVANCED
