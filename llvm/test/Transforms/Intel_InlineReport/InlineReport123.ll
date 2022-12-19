; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: system-linux,intel_feature_sw_advanced
; RUN: opt -passes='cgscc(inline)' -inline-report=0xe807 -inline-threshold=0 -inline-tbb-parallel-for-min-funcs=0 -lto-inline-cost < %s -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-CL
; RUN: opt -passes='inlinereportsetup,cgscc(inline),inlinereportemitter' -inline-report=0xe886 -inline-threshold=0 -inline-tbb-parallel-for-min-funcs=0 -lto-inline-cost -S < %s 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-MD

; Show that _ZL5relaxPiS_S_i is inlined because it is selected as a candidate
; using the "TBB parallel for" inlining heuristic on Linux.

; CHECK-CL: define {{.*}} @_ZN3tbb6detail2d19start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEE7executeERNS1_14execution_dataE
; CHECK-CL: call {{.*}} @_ZL14delta_steppingPiS_S_i
; CHECK-CL: call {{.*}} @_ZL14delta_steppingPiS_S_i

; CHECK-CL: define {{.*}} @_ZL14delta_steppingPiS_S_i
; CHECK-CL-NOT: call {{.*}} @_ZL5relaxPiS_S_i

; CHECK-CL: COMPILE FUNC: _ZL14delta_steppingPiS_S_i
; CHECK-CL: INLINE: _ZL5relaxPiS_S_i {{.*}}Under TBB parallel for
; CHECK-CL: INLINE: _ZL5relaxPiS_S_i {{.*}}Under TBB parallel for
; CHECK-CL: INLINE: _ZL5relaxPiS_S_i {{.*}}Callee has single callsite and local linkage

; CHECK: COMPILE FUNC: _ZN3tbb6detail2d19start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEE7executeERNS1_14execution_dataE
; CHECK: _ZL14delta_steppingPiS_S_i {{.*}}Inlining is not profitable
; CHECK: _ZL14delta_steppingPiS_S_i {{.*}}Inlining is not profitable

; CHECK-MD: COMPILE FUNC: _ZL14delta_steppingPiS_S_i
; CHECK-MD: INLINE: _ZL5relaxPiS_S_i {{.*}}Under TBB parallel for
; CHECK-MD: INLINE: _ZL5relaxPiS_S_i {{.*}}Under TBB parallel for
; CHECK-MD: INLINE: _ZL5relaxPiS_S_i {{.*}}Callee has single callsite and local linkage

; CHECK-MD: define {{.*}} @_ZN3tbb6detail2d19start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEE7executeERNS1_14execution_dataE
; CHECK-MD: call {{.*}} @_ZL14delta_steppingPiS_S_i
; CHECK-MD: call {{.*}} @_ZL14delta_steppingPiS_S_i

; CHECK-MD: define {{.*}} @_ZL14delta_steppingPiS_S_i
; CHECK-MD-NOT: call {{.*}} @_ZL5relaxPiS_S_i

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"class.tbb::detail::d1::task_group_context" = type { i64, %"struct.std::atomic", i8, %"struct.tbb::detail::d1::task_group_context::context_traits", %"struct.std::atomic.0", %"struct.std::atomic.2", %union.anon, %"class.tbb::detail::r1::context_list"*, %"struct.tbb::detail::d1::intrusive_list_node", %"struct.std::atomic.3", i8*, i64, [56 x i8] }
%"struct.std::atomic" = type { %"struct.std::__atomic_base" }
%"struct.std::__atomic_base" = type { i32 }
%"struct.tbb::detail::d1::task_group_context::context_traits" = type { i8 }
%"struct.std::atomic.0" = type { %"struct.std::__atomic_base.1" }
%"struct.std::__atomic_base.1" = type { i8 }
%"struct.std::atomic.2" = type { i8 }
%union.anon = type { %"class.tbb::detail::d1::task_group_context"* }
%"class.tbb::detail::r1::context_list" = type opaque
%"struct.tbb::detail::d1::intrusive_list_node" = type { %"struct.tbb::detail::d1::intrusive_list_node"*, %"struct.tbb::detail::d1::intrusive_list_node"* }
%"struct.std::atomic.3" = type { %"struct.std::__atomic_base.4" }
%"struct.std::__atomic_base.4" = type { %"class.tbb::detail::r1::tbb_exception_ptr"* }
%"class.tbb::detail::r1::tbb_exception_ptr" = type opaque
%"class.tbb::detail::d1::small_object_pool" = type { i8 }
%"class.tbb::detail::d1::task" = type { i32 (...)**, %"class.tbb::detail::d1::task_traits", [6 x i64] }
%"class.tbb::detail::d1::task_traits" = type { i64 }
%"struct.tbb::detail::d1::start_for" = type { %"class.tbb::detail::d1::task", %"class.tbb::detail::d1::blocked_range", %class.ArraySummer, %"struct.tbb::detail::d1::node"*, %"class.tbb::detail::d1::auto_partition_type", %"class.tbb::detail::d1::small_object_allocator", [56 x i8] }
%"class.tbb::detail::d1::blocked_range" = type { i32, i32, i64 }
%class.ArraySummer = type { i32*, i32*, i32* }
%"struct.tbb::detail::d1::node" = type <{ %"struct.tbb::detail::d1::node"*, %"struct.std::atomic.5", [4 x i8] }>
%"struct.std::atomic.5" = type { %"struct.std::__atomic_base.6" }
%"struct.std::__atomic_base.6" = type { i32 }
%"class.tbb::detail::d1::auto_partition_type" = type { %"struct.tbb::detail::d1::dynamic_grainsize_mode.base", [3 x i8] }
%"struct.tbb::detail::d1::dynamic_grainsize_mode.base" = type <{ %"struct.tbb::detail::d1::adaptive_mode", i32, i8 }>
%"struct.tbb::detail::d1::adaptive_mode" = type { i64 }
%"class.tbb::detail::d1::small_object_allocator" = type { %"class.tbb::detail::d1::small_object_pool"* }
%"struct.tbb::detail::d1::execution_data" = type <{ %"class.tbb::detail::d1::task_group_context"*, i16, i16, [4 x i8] }>
%"class.tbb::detail::d1::range_vector" = type { i8, i8, i8, [8 x i8], [5 x i8], %"class.tbb::detail::d0::aligned_space" }
%"class.tbb::detail::d0::aligned_space" = type { [128 x i8] }
%"struct.tbb::detail::d1::tree_node" = type <{ %"struct.tbb::detail::d1::node.base", [4 x i8], %"class.tbb::detail::d1::small_object_allocator", %"struct.std::atomic.12", [7 x i8] }>
%"struct.tbb::detail::d1::node.base" = type <{ %"struct.tbb::detail::d1::node"*, %"struct.std::atomic.5" }>
%"struct.std::atomic.12" = type { %"struct.std::__atomic_base.13" }
%"struct.std::__atomic_base.13" = type { i8 }
%"class.tbb::detail::d1::task_arena_base" = type { i64, %"struct.std::atomic.9", %"struct.std::atomic.10", i32, i32, i32, i32, i32, i32 }
%"struct.std::atomic.9" = type { i32 }
%"struct.std::atomic.10" = type { %"struct.std::__atomic_base.11" }
%"struct.std::__atomic_base.11" = type { %"class.tbb::detail::r1::arena"* }
%"class.tbb::detail::r1::arena" = type opaque
%"class.tbb::detail::d1::wait_context" = type { i64, %"struct.std::atomic.7" }
%"struct.std::atomic.7" = type { %"struct.std::__atomic_base.8" }
%"struct.std::__atomic_base.8" = type { i64 }

$_ZN3tbb6detail2d14taskD2Ev = comdat any

$_ZN3tbb6detail2d19start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEED0Ev = comdat any

$_ZN3tbb6detail2d19start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEE7executeERNS1_14execution_dataE = comdat any

$_ZN3tbb6detail2d19start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEE6cancelERNS1_14execution_dataE = comdat any

$__clang_call_terminate = comdat any

$_ZTVN3tbb6detail2d19start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEEE = comdat any

$_ZTSN3tbb6detail2d19start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEEE = comdat any

$_ZTSN3tbb6detail2d14taskE = comdat any

$_ZTSN3tbb6detail2d111task_traitsE = comdat any

$_ZTIN3tbb6detail2d111task_traitsE = comdat any

$_ZTIN3tbb6detail2d14taskE = comdat any

$_ZTIN3tbb6detail2d19start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEEE = comdat any

@_ZTVN3tbb6detail2d19start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEEE = linkonce_odr dso_local unnamed_addr constant { [6 x i8*] } { [6 x i8*] [i8* null, i8* bitcast ({ i8*, i8*, i8* }* @_ZTIN3tbb6detail2d19start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEEE to i8*), i8* bitcast (void (%"class.tbb::detail::d1::task"*)* @_ZN3tbb6detail2d14taskD2Ev to i8*), i8* bitcast (void (%"struct.tbb::detail::d1::start_for"*)* @_ZN3tbb6detail2d19start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEED0Ev to i8*), i8* bitcast (%"class.tbb::detail::d1::task"* (%"struct.tbb::detail::d1::start_for"*, %"struct.tbb::detail::d1::execution_data"*)* @_ZN3tbb6detail2d19start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEE7executeERNS1_14execution_dataE to i8*), i8* bitcast (%"class.tbb::detail::d1::task"* (%"struct.tbb::detail::d1::start_for"*, %"struct.tbb::detail::d1::execution_data"*)* @_ZN3tbb6detail2d19start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEE6cancelERNS1_14execution_dataE to i8*)] }, comdat, align 8
@_ZTVN10__cxxabiv120__si_class_type_infoE = external dso_local global i8*
@_ZTSN3tbb6detail2d19start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEEE = linkonce_odr dso_local constant [89 x i8] c"N3tbb6detail2d19start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEEE\00", comdat, align 1
@_ZTVN10__cxxabiv121__vmi_class_type_infoE = external dso_local global i8*
@_ZTSN3tbb6detail2d14taskE = linkonce_odr dso_local constant [22 x i8] c"N3tbb6detail2d14taskE\00", comdat, align 1
@_ZTVN10__cxxabiv117__class_type_infoE = external dso_local global i8*
@_ZTSN3tbb6detail2d111task_traitsE = linkonce_odr dso_local constant [30 x i8] c"N3tbb6detail2d111task_traitsE\00", comdat, align 1
@_ZTIN3tbb6detail2d111task_traitsE = linkonce_odr dso_local constant { i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv117__class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([30 x i8], [30 x i8]* @_ZTSN3tbb6detail2d111task_traitsE, i32 0, i32 0) }, comdat, align 8
@_ZTIN3tbb6detail2d14taskE = linkonce_odr dso_local constant { i8*, i8*, i32, i32, i8*, i64 } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv121__vmi_class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([22 x i8], [22 x i8]* @_ZTSN3tbb6detail2d14taskE, i32 0, i32 0), i32 0, i32 1, i8* bitcast ({ i8*, i8* }* @_ZTIN3tbb6detail2d111task_traitsE to i8*), i64 2050 }, comdat, align 8
@_ZTIN3tbb6detail2d19start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEEE = linkonce_odr dso_local constant { i8*, i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv120__si_class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([89 x i8], [89 x i8]* @_ZTSN3tbb6detail2d19start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEEE, i32 0, i32 0), i8* bitcast ({ i8*, i8*, i32, i32, i8*, i64 }* @_ZTIN3tbb6detail2d14taskE to i8*) }, comdat, align 8

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture)

; Function Attrs: nobuiltin allocsize(0)
declare dso_local noundef nonnull i8* @_Znam(i64 noundef)

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture)

declare dso_local i32 @__gxx_personality_v0(...)

; Function Attrs: argmemonly nofree nounwind willreturn
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* noalias nocapture writeonly, i8* noalias nocapture readonly, i64, i1 immarg) #2

declare dso_local void @_ZN3tbb6detail2r110initializeERNS0_2d118task_group_contextE(%"class.tbb::detail::d1::task_group_context"* noundef nonnull align 8 dereferenceable(128))

; Function Attrs: argmemonly nofree nounwind willreturn writeonly
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1 immarg)

declare dso_local noundef i8* @_ZN3tbb6detail2r18allocateERPNS0_2d117small_object_poolEm(%"class.tbb::detail::d1::small_object_pool"** noundef nonnull align 8 dereferenceable(8), i64 noundef)

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local void @_ZN3tbb6detail2d14taskD2Ev(%"class.tbb::detail::d1::task"* noundef nonnull align 64 dereferenceable(64) %this) {
entry:
  ret void
}

; Function Attrs: inlinehint nounwind uwtable
define linkonce_odr dso_local void @_ZN3tbb6detail2d19start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEED0Ev(%"struct.tbb::detail::d1::start_for"* noundef nonnull align 64 dereferenceable(136) %this) comdat align 2 {
entry:
  %0 = bitcast %"struct.tbb::detail::d1::start_for"* %this to i8*
  tail call void @_ZdlPv(i8* noundef nonnull %0)
  ret void
}

; Function Attrs: uwtable
define linkonce_odr dso_local noundef %"class.tbb::detail::d1::task"* @_ZN3tbb6detail2d19start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEE7executeERNS1_14execution_dataE(%"struct.tbb::detail::d1::start_for"* noundef nonnull align 64 dereferenceable(136) %this, %"struct.tbb::detail::d1::execution_data"* noundef nonnull align 8 dereferenceable(12) %ed) comdat align 2 personality i32 (...)* @__gxx_personality_v0 {
entry:
  %alloc.i.i.i.i = alloca %"class.tbb::detail::d1::small_object_allocator", align 8
  %range_pool.i.i = alloca %"class.tbb::detail::d1::range_vector", align 8
  %alloc.i.i.i = alloca %"class.tbb::detail::d1::small_object_allocator", align 8
  %affinity_slot.i.i = getelementptr inbounds %"struct.tbb::detail::d1::execution_data", %"struct.tbb::detail::d1::execution_data"* %ed, i64 0, i32 2, !intel-tbaa !3
  %0 = load i16, i16* %affinity_slot.i.i, align 2, !tbaa !3
  %cmp.i = icmp eq i16 %0, -1
  br i1 %cmp.i, label %if.end, label %_ZN3tbb6detail2d116is_same_affinityERKNS1_14execution_dataE.exit

_ZN3tbb6detail2d116is_same_affinityERKNS1_14execution_dataE.exit: ; preds = %entry
  %call.i.i = tail call noundef zeroext i16 @_ZN3tbb6detail2r114execution_slotEPKNS0_2d114execution_dataE(%"struct.tbb::detail::d1::execution_data"* noundef nonnull %ed)
  %cmp5.i = icmp eq i16 %0, %call.i.i
  br i1 %cmp5.i, label %if.end, label %if.then

if.then:                                          ; preds = %_ZN3tbb6detail2d116is_same_affinityERKNS1_14execution_dataE.exit
  %call.i = tail call noundef zeroext i16 @_ZN3tbb6detail2r114execution_slotEPKNS0_2d114execution_dataE(%"struct.tbb::detail::d1::execution_data"* noundef nonnull %ed)
  br label %if.end

if.end:                                           ; preds = %if.then, %_ZN3tbb6detail2d116is_same_affinityERKNS1_14execution_dataE.exit, %entry
  %my_divisor.i = getelementptr inbounds %"struct.tbb::detail::d1::start_for", %"struct.tbb::detail::d1::start_for"* %this, i64 0, i32 4, i32 0, i32 0, i32 0
  %1 = load i64, i64* %my_divisor.i, align 16, !tbaa !9
  %tobool.not.i = icmp eq i64 %1, 0
  br i1 %tobool.not.i, label %if.then.i, label %_ZN3tbb6detail2d122dynamic_grainsize_modeINS1_13adaptive_modeINS1_19auto_partition_typeEEEE18check_being_stolenINS1_9start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEEEEEbRT_RKNS1_14execution_dataE.exit

if.then.i:                                        ; preds = %if.end
  %2 = bitcast %"struct.tbb::detail::d1::start_for"* %this to i8*
  %sunkaddr = getelementptr inbounds i8, i8* %2, i64 112
  %3 = bitcast i8* %sunkaddr to i64*
  store i64 1, i64* %3, align 16, !tbaa !9
  %call.i.i.i = tail call noundef zeroext i16 @_ZN3tbb6detail2r114execution_slotEPKNS0_2d114execution_dataE(%"struct.tbb::detail::d1::execution_data"* noundef nonnull %ed)
  %original_slot.i.i.i = getelementptr inbounds %"struct.tbb::detail::d1::execution_data", %"struct.tbb::detail::d1::execution_data"* %ed, i64 0, i32 1, !intel-tbaa !12
  %4 = load i16, i16* %original_slot.i.i.i, align 8, !tbaa !12
  %cmp.i.not.i = icmp eq i16 %call.i.i.i, %4
  br i1 %cmp.i.not.i, label %_ZN3tbb6detail2d122dynamic_grainsize_modeINS1_13adaptive_modeINS1_19auto_partition_typeEEEE18check_being_stolenINS1_9start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEEEEEbRT_RKNS1_14execution_dataE.exit, label %land.lhs.true.i

land.lhs.true.i:                                  ; preds = %if.then.i
  %my_parent.i = getelementptr inbounds %"struct.tbb::detail::d1::start_for", %"struct.tbb::detail::d1::start_for"* %this, i64 0, i32 3, !intel-tbaa !13
  %5 = load %"struct.tbb::detail::d1::node"*, %"struct.tbb::detail::d1::node"** %my_parent.i, align 8, !tbaa !13
  %_M_i.i.i.i = getelementptr inbounds %"struct.tbb::detail::d1::node", %"struct.tbb::detail::d1::node"* %5, i64 0, i32 1, i32 0, i32 0
  %6 = load atomic i32, i32* %_M_i.i.i.i seq_cst, align 4
  %cmp.i10 = icmp sgt i32 %6, 1
  br i1 %cmp.i10, label %if.then6.i, label %_ZN3tbb6detail2d122dynamic_grainsize_modeINS1_13adaptive_modeINS1_19auto_partition_typeEEEE18check_being_stolenINS1_9start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEEEEEbRT_RKNS1_14execution_dataE.exit

if.then6.i:                                       ; preds = %land.lhs.true.i
  %7 = bitcast %"struct.tbb::detail::d1::start_for"* %this to i8*
  %sunkaddr54 = getelementptr inbounds i8, i8* %7, i64 104
  %8 = bitcast i8* %sunkaddr54 to %"struct.tbb::detail::d1::tree_node"**
  %9 = load %"struct.tbb::detail::d1::tree_node"*, %"struct.tbb::detail::d1::tree_node"** %8, align 8, !tbaa !13
  %_M_i.i.i.i.i = getelementptr inbounds %"struct.tbb::detail::d1::tree_node", %"struct.tbb::detail::d1::tree_node"* %9, i64 0, i32 3, i32 0, i32 0, !intel-tbaa !23
  store atomic i8 1, i8* %_M_i.i.i.i.i monotonic, align 1
  %10 = getelementptr inbounds %"struct.tbb::detail::d1::start_for", %"struct.tbb::detail::d1::start_for"* %this, i64 0, i32 4, i32 0, i32 2
  %11 = load i8, i8* %10, align 4, !tbaa !28
  %tobool7.not.i = icmp eq i8 %11, 0
  %.op.i = add i8 %11, 1
  %add.i = select i1 %tobool7.not.i, i8 2, i8 %.op.i
  store i8 %add.i, i8* %10, align 4, !tbaa !28
  br label %_ZN3tbb6detail2d122dynamic_grainsize_modeINS1_13adaptive_modeINS1_19auto_partition_typeEEEE18check_being_stolenINS1_9start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEEEEEbRT_RKNS1_14execution_dataE.exit

_ZN3tbb6detail2d122dynamic_grainsize_modeINS1_13adaptive_modeINS1_19auto_partition_typeEEEE18check_being_stolenINS1_9start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEEEEEbRT_RKNS1_14execution_dataE.exit: ; preds = %if.then6.i, %land.lhs.true.i, %if.then.i, %if.end
  %my_range = getelementptr inbounds %"struct.tbb::detail::d1::start_for", %"struct.tbb::detail::d1::start_for"* %this, i64 0, i32 1, !intel-tbaa !31
  %my_grainsize.i.i = getelementptr inbounds %"struct.tbb::detail::d1::start_for", %"struct.tbb::detail::d1::start_for"* %this, i64 0, i32 1, i32 2
  %12 = load i64, i64* %my_grainsize.i.i, align 8, !tbaa !32
  %my_end.i.i.i55 = bitcast %"class.tbb::detail::d1::blocked_range"* %my_range to i32*
  %13 = load i32, i32* %my_end.i.i.i55, align 64, !tbaa !33
  %my_begin.i.i.i = getelementptr inbounds %"struct.tbb::detail::d1::start_for", %"struct.tbb::detail::d1::start_for"* %this, i64 0, i32 1, i32 1
  %14 = load i32, i32* %my_begin.i.i.i, align 4, !tbaa !34
  %sub.i.i.i = sub nsw i32 %13, %14
  %conv.i.i.i = sext i32 %sub.i.i.i to i64
  %cmp.i.i = icmp ult i64 %12, %conv.i.i.i
  br i1 %cmp.i.i, label %if.then.i11, label %if.end9.i

if.then.i11:                                      ; preds = %_ZN3tbb6detail2d122dynamic_grainsize_modeINS1_13adaptive_modeINS1_19auto_partition_typeEEEE18check_being_stolenINS1_9start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEEEEEbRT_RKNS1_14execution_dataE.exit
  %15 = bitcast %"struct.tbb::detail::d1::start_for"* %this to i8*
  %sunkaddr56 = getelementptr inbounds i8, i8* %15, i64 112
  %16 = bitcast i8* %sunkaddr56 to i64*
  %17 = load i64, i64* %16, align 16, !tbaa !9
  %cmp.i15.i = icmp ugt i64 %17, 1
  br i1 %cmp.i15.i, label %_ZN3tbb6detail2d119auto_partition_type12is_divisibleEv.exit.i, label %if.end.i.i

if.end.i.i:                                       ; preds = %if.then.i11
  %tobool.not.i.i = icmp eq i64 %17, 0
  br i1 %tobool.not.i.i, label %if.end9.i, label %land.lhs.true.i.i

land.lhs.true.i.i:                                ; preds = %if.end.i.i
  %18 = getelementptr inbounds %"struct.tbb::detail::d1::start_for", %"struct.tbb::detail::d1::start_for"* %this, i64 0, i32 4, i32 0, i32 2
  %19 = load i8, i8* %18, align 4, !tbaa !28
  %tobool3.not.i.i = icmp eq i8 %19, 0
  br i1 %tobool3.not.i.i, label %if.end9.i, label %if.then4.i.i

if.then4.i.i:                                     ; preds = %land.lhs.true.i.i
  %dec.i.i = add i8 %19, -1
  %20 = bitcast %"struct.tbb::detail::d1::start_for"* %this to i8*
  %sunkaddr57 = getelementptr inbounds i8, i8* %20, i64 124
  store i8 %dec.i.i, i8* %sunkaddr57, align 4, !tbaa !28
  %21 = bitcast %"struct.tbb::detail::d1::start_for"* %this to i8*
  %sunkaddr58 = getelementptr inbounds i8, i8* %21, i64 112
  %22 = bitcast i8* %sunkaddr58 to i64*
  store i64 0, i64* %22, align 16, !tbaa !9
  br label %_ZN3tbb6detail2d119auto_partition_type12is_divisibleEv.exit.i

_ZN3tbb6detail2d119auto_partition_type12is_divisibleEv.exit.i: ; preds = %if.then4.i.i, %if.then.i11
  %my_body4.i.i.i.i.i = getelementptr inbounds %"struct.tbb::detail::d1::start_for", %"struct.tbb::detail::d1::start_for"* %this, i64 0, i32 2
  br label %do.body.i

do.body.i:                                        ; preds = %if.then4.i29.i, %land.rhs.i, %_ZN3tbb6detail2d119auto_partition_type12is_divisibleEv.exit.i
  %23 = bitcast %"struct.tbb::detail::d1::execution_data"* %ed to %"class.tbb::detail::d1::task_group_context"**
  %24 = bitcast %class.ArraySummer* %my_body4.i.i.i.i.i to i8*
  %25 = bitcast %"class.tbb::detail::d1::small_object_allocator"* %alloc.i.i.i to %"class.tbb::detail::d1::small_object_pool"**
  %26 = bitcast %"class.tbb::detail::d1::small_object_allocator"* %alloc.i.i.i to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %26)
  store %"class.tbb::detail::d1::small_object_pool"* null, %"class.tbb::detail::d1::small_object_pool"** %25, align 8, !tbaa !35
  %call.i.i.i.i = call noundef i8* @_ZN3tbb6detail2r18allocateERPNS0_2d117small_object_poolEmRKNS2_14execution_dataE(%"class.tbb::detail::d1::small_object_pool"** noundef nonnull align 8 dereferenceable(8) %25, i64 noundef 192, %"struct.tbb::detail::d1::execution_data"* noundef nonnull align 8 dereferenceable(12) %ed)
  %m_version_and_traits.i.i.i.i.i.i.i = getelementptr inbounds i8, i8* %call.i.i.i.i, i64 8
  %arrayinit.end.i.i.i.i.i.i = getelementptr inbounds i8, i8* %call.i.i.i.i, i64 64
  %27 = bitcast i8* %m_version_and_traits.i.i.i.i.i.i.i to <2 x i64>*
  store <2 x i64> zeroinitializer, <2 x i64>* %27, align 8, !tbaa !36
  %28 = getelementptr inbounds i8, i8* %call.i.i.i.i, i64 24
  %29 = bitcast i8* %28 to <2 x i64>*
  store <2 x i64> zeroinitializer, <2 x i64>* %29, align 8, !tbaa !36
  %30 = getelementptr inbounds i8, i8* %call.i.i.i.i, i64 40
  %31 = bitcast i8* %30 to <2 x i64>*
  store <2 x i64> zeroinitializer, <2 x i64>* %31, align 8, !tbaa !36
  %32 = getelementptr inbounds i8, i8* %call.i.i.i.i, i64 56
  %33 = bitcast i8* %32 to i64*
  store i64 0, i64* %33, align 8, !tbaa !36
  %34 = bitcast i8* %call.i.i.i.i to i32 (...)***
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [6 x i8*] }, { [6 x i8*] }* @_ZTVN3tbb6detail2d19start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEEE, i64 0, inrange i32 0, i64 2) to i32 (...)**), i32 (...)*** %34, align 64, !tbaa !37
  %35 = bitcast i8* %arrayinit.end.i.i.i.i.i.i to i32*
  %36 = bitcast %"struct.tbb::detail::d1::start_for"* %this to i8*
  %sunkaddr61 = getelementptr inbounds i8, i8* %36, i64 64
  %37 = bitcast i8* %sunkaddr61 to i32*
  %38 = load i32, i32* %37, align 64, !tbaa !33
  store i32 %38, i32* %35, align 64, !tbaa !33
  %my_begin.i.i.i.i.i.i = getelementptr inbounds i8, i8* %call.i.i.i.i, i64 68
  %39 = bitcast i8* %my_begin.i.i.i.i.i.i to i32*
  %40 = bitcast %"struct.tbb::detail::d1::start_for"* %this to i8*
  %sunkaddr62 = getelementptr inbounds i8, i8* %40, i64 68
  %41 = bitcast i8* %sunkaddr62 to i32*
  %42 = load i32, i32* %41, align 4, !tbaa !34
  %43 = load i32, i32* %37, align 64, !tbaa !33
  %sub.i.i.i.i.i.i.i = sub nsw i32 %43, %42
  %div8.i.i.i.i.i.i.i = lshr i32 %sub.i.i.i.i.i.i.i, 1
  %add.i.i.i.i.i.i.i = add i32 %div8.i.i.i.i.i.i.i, %42
  store i32 %add.i.i.i.i.i.i.i, i32* %37, align 64, !tbaa !33
  store i32 %add.i.i.i.i.i.i.i, i32* %39, align 4, !tbaa !34
  %my_grainsize.i.i.i.i.i.i = getelementptr inbounds i8, i8* %call.i.i.i.i, i64 72
  %44 = bitcast i8* %my_grainsize.i.i.i.i.i.i to i64*
  %45 = bitcast %"struct.tbb::detail::d1::start_for"* %this to i8*
  %sunkaddr63 = getelementptr inbounds i8, i8* %45, i64 72
  %46 = bitcast i8* %sunkaddr63 to i64*
  %47 = load i64, i64* %46, align 8, !tbaa !32
  store i64 %47, i64* %44, align 8, !tbaa !32
  %my_body.i.i.i.i.i = getelementptr inbounds i8, i8* %call.i.i.i.i, i64 80
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* noundef nonnull align 16 dereferenceable(24) %my_body.i.i.i.i.i, i8* noundef nonnull align 16 dereferenceable(24) %24, i64 24, i1 false), !tbaa.struct !39
  %my_divisor.i.i.i.i.i.i.i.i = getelementptr inbounds i8, i8* %call.i.i.i.i, i64 112
  %48 = bitcast i8* %my_divisor.i.i.i.i.i.i.i.i to i64*
  %49 = bitcast %"struct.tbb::detail::d1::start_for"* %this to i8*
  %sunkaddr64 = getelementptr inbounds i8, i8* %49, i64 112
  %50 = bitcast i8* %sunkaddr64 to i64*
  %51 = load i64, i64* %50, align 16, !tbaa !9
  %div2.i.i.i.i.i.i.i.i.i = lshr i64 %51, 1
  store i64 %div2.i.i.i.i.i.i.i.i.i, i64* %50, align 16, !tbaa !9
  store i64 %div2.i.i.i.i.i.i.i.i.i, i64* %48, align 16, !tbaa !9
  %52 = getelementptr inbounds i8, i8* %call.i.i.i.i, i64 120
  %53 = bitcast i8* %52 to i32*
  store i32 2, i32* %53, align 8, !tbaa !41
  %54 = getelementptr inbounds i8, i8* %call.i.i.i.i, i64 124
  %55 = bitcast %"struct.tbb::detail::d1::start_for"* %this to i8*
  %sunkaddr65 = getelementptr inbounds i8, i8* %55, i64 124
  %56 = load i8, i8* %sunkaddr65, align 4, !tbaa !28
  store i8 %56, i8* %54, align 4, !tbaa !28
  %57 = getelementptr inbounds i8, i8* %call.i.i.i.i, i64 128
  %58 = bitcast i8* %57 to %"class.tbb::detail::d1::small_object_pool"**
  %59 = load %"class.tbb::detail::d1::small_object_pool"*, %"class.tbb::detail::d1::small_object_pool"** %25, align 8, !tbaa !35
  store %"class.tbb::detail::d1::small_object_pool"* %59, %"class.tbb::detail::d1::small_object_pool"** %58, align 64, !tbaa !42
  %call.i13.i.i.i = call noundef i8* @_ZN3tbb6detail2r18allocateERPNS0_2d117small_object_poolEmRKNS2_14execution_dataE(%"class.tbb::detail::d1::small_object_pool"** noundef nonnull align 8 dereferenceable(8) %25, i64 noundef 32, %"struct.tbb::detail::d1::execution_data"* noundef nonnull align 8 dereferenceable(12) %ed)
  %60 = bitcast %"struct.tbb::detail::d1::start_for"* %this to i8*
  %sunkaddr66 = getelementptr inbounds i8, i8* %60, i64 104
  %61 = bitcast i8* %sunkaddr66 to %"struct.tbb::detail::d1::node"**
  %62 = load %"struct.tbb::detail::d1::node"*, %"struct.tbb::detail::d1::node"** %61, align 8, !tbaa !13
  %my_parent.i.i.i.i.i.i = bitcast i8* %call.i13.i.i.i to %"struct.tbb::detail::d1::node"**
  store %"struct.tbb::detail::d1::node"* %62, %"struct.tbb::detail::d1::node"** %my_parent.i.i.i.i.i.i, align 8, !tbaa !43
  %63 = getelementptr inbounds i8, i8* %call.i13.i.i.i, i64 8
  %64 = bitcast i8* %63 to i32*
  store i32 2, i32* %64, align 8, !tbaa !46
  %65 = getelementptr inbounds i8, i8* %call.i13.i.i.i, i64 16
  %66 = bitcast i8* %65 to %"class.tbb::detail::d1::small_object_pool"**
  %67 = load %"class.tbb::detail::d1::small_object_pool"*, %"class.tbb::detail::d1::small_object_pool"** %25, align 8, !tbaa !35
  store %"class.tbb::detail::d1::small_object_pool"* %67, %"class.tbb::detail::d1::small_object_pool"** %66, align 8, !tbaa !42
  %68 = getelementptr inbounds i8, i8* %call.i13.i.i.i, i64 24
  store i8 0, i8* %68, align 8, !tbaa !23
  %69 = bitcast %"struct.tbb::detail::d1::start_for"* %this to i8*
  %sunkaddr67 = getelementptr inbounds i8, i8* %69, i64 104
  %70 = bitcast i8* %sunkaddr67 to i8**
  store i8* %call.i13.i.i.i, i8** %70, align 8, !tbaa !13
  %my_parent6.i.i.i = getelementptr inbounds i8, i8* %call.i.i.i.i, i64 104
  %71 = bitcast i8* %my_parent6.i.i.i to i8**
  store i8* %call.i13.i.i.i, i8** %71, align 8, !tbaa !13
  %72 = bitcast i8* %call.i.i.i.i to %"class.tbb::detail::d1::task"*
  %73 = load %"class.tbb::detail::d1::task_group_context"*, %"class.tbb::detail::d1::task_group_context"** %23, align 8, !tbaa !48
  call void @_ZN3tbb6detail2r15spawnERNS0_2d14taskERNS2_18task_group_contextE(%"class.tbb::detail::d1::task"* noundef nonnull align 64 dereferenceable(64) %72, %"class.tbb::detail::d1::task_group_context"* noundef nonnull align 8 dereferenceable(128) %73)
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %26)
  %74 = load i64, i64* %46, align 8, !tbaa !32
  %75 = bitcast %"struct.tbb::detail::d1::start_for"* %this to i8*
  %sunkaddr68 = getelementptr inbounds i8, i8* %75, i64 64
  %76 = bitcast i8* %sunkaddr68 to i32*
  %77 = load i32, i32* %76, align 64, !tbaa !33
  %78 = load i32, i32* %41, align 4, !tbaa !34
  %sub.i.i19.i = sub nsw i32 %77, %78
  %conv.i.i20.i = sext i32 %sub.i.i19.i to i64
  %cmp.i21.i = icmp ult i64 %74, %conv.i.i20.i
  br i1 %cmp.i21.i, label %land.rhs.i, label %if.end9.i

land.rhs.i:                                       ; preds = %do.body.i
  %79 = bitcast %"struct.tbb::detail::d1::start_for"* %this to i8*
  %sunkaddr69 = getelementptr inbounds i8, i8* %79, i64 112
  %80 = bitcast i8* %sunkaddr69 to i64*
  %81 = load i64, i64* %80, align 16, !tbaa !9
  %cmp.i23.i = icmp ugt i64 %81, 1
  br i1 %cmp.i23.i, label %do.body.i, label %if.end.i25.i

if.end.i25.i:                                     ; preds = %land.rhs.i
  %tobool.not.i24.i = icmp eq i64 %81, 0
  br i1 %tobool.not.i24.i, label %if.end9.i, label %land.lhs.true.i27.i

land.lhs.true.i27.i:                              ; preds = %if.end.i25.i
  %82 = bitcast %"struct.tbb::detail::d1::start_for"* %this to i8*
  %sunkaddr70 = getelementptr inbounds i8, i8* %82, i64 124
  %83 = load i8, i8* %sunkaddr70, align 4, !tbaa !28
  %tobool3.not.i26.i = icmp eq i8 %83, 0
  br i1 %tobool3.not.i26.i, label %if.end9.i, label %if.then4.i29.i

if.then4.i29.i:                                   ; preds = %land.lhs.true.i27.i
  %dec.i28.i = add i8 %83, -1
  %84 = bitcast %"struct.tbb::detail::d1::start_for"* %this to i8*
  %sunkaddr71 = getelementptr inbounds i8, i8* %84, i64 124
  store i8 %dec.i28.i, i8* %sunkaddr71, align 4, !tbaa !28
  %85 = bitcast %"struct.tbb::detail::d1::start_for"* %this to i8*
  %sunkaddr72 = getelementptr inbounds i8, i8* %85, i64 112
  %86 = bitcast i8* %sunkaddr72 to i64*
  store i64 0, i64* %86, align 16, !tbaa !9
  br label %do.body.i

if.end9.i:                                        ; preds = %land.lhs.true.i27.i, %if.end.i25.i, %do.body.i, %land.lhs.true.i.i, %if.end.i.i, %_ZN3tbb6detail2d122dynamic_grainsize_modeINS1_13adaptive_modeINS1_19auto_partition_typeEEEE18check_being_stolenINS1_9start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEEEEEbRT_RKNS1_14execution_dataE.exit
  %conv.i.i.i.pre-phi.i = phi i64 [ %conv.i.i.i, %if.end.i.i ], [ %conv.i.i.i, %land.lhs.true.i.i ], [ %conv.i.i.i, %_ZN3tbb6detail2d122dynamic_grainsize_modeINS1_13adaptive_modeINS1_19auto_partition_typeEEEE18check_being_stolenINS1_9start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEEEEEbRT_RKNS1_14execution_dataE.exit ], [ %conv.i.i20.i, %do.body.i ], [ %conv.i.i20.i, %if.end.i25.i ], [ %conv.i.i20.i, %land.lhs.true.i27.i ]
  %87 = phi i32 [ %14, %if.end.i.i ], [ %14, %land.lhs.true.i.i ], [ %14, %_ZN3tbb6detail2d122dynamic_grainsize_modeINS1_13adaptive_modeINS1_19auto_partition_typeEEEE18check_being_stolenINS1_9start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEEEEEbRT_RKNS1_14execution_dataE.exit ], [ %78, %do.body.i ], [ %78, %if.end.i25.i ], [ %78, %land.lhs.true.i27.i ]
  %88 = phi i32 [ %13, %if.end.i.i ], [ %13, %land.lhs.true.i.i ], [ %13, %_ZN3tbb6detail2d122dynamic_grainsize_modeINS1_13adaptive_modeINS1_19auto_partition_typeEEEE18check_being_stolenINS1_9start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEEEEEbRT_RKNS1_14execution_dataE.exit ], [ %77, %do.body.i ], [ %77, %if.end.i25.i ], [ %77, %land.lhs.true.i27.i ]
  %89 = phi i64 [ %12, %if.end.i.i ], [ %12, %land.lhs.true.i.i ], [ %12, %_ZN3tbb6detail2d122dynamic_grainsize_modeINS1_13adaptive_modeINS1_19auto_partition_typeEEEE18check_being_stolenINS1_9start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEEEEEbRT_RKNS1_14execution_dataE.exit ], [ %74, %do.body.i ], [ %74, %if.end.i25.i ], [ %74, %land.lhs.true.i27.i ]
  %cmp.i.i.i = icmp ult i64 %89, %conv.i.i.i.pre-phi.i
  br i1 %cmp.i.i.i, label %lor.lhs.false.i.i, label %if.then.i.i

lor.lhs.false.i.i:                                ; preds = %if.end9.i
  %90 = getelementptr inbounds %"struct.tbb::detail::d1::start_for", %"struct.tbb::detail::d1::start_for"* %this, i64 0, i32 4, i32 0, i32 2
  %91 = load i8, i8* %90, align 4, !tbaa !28
  %tobool.not.i32.i = icmp eq i8 %91, 0
  br i1 %tobool.not.i32.i, label %if.then.i.i, label %if.else.i.i

if.then.i.i:                                      ; preds = %lor.lhs.false.i.i, %if.end9.i
  %cmp.not17.i.i.i.i = icmp eq i32 %88, %87
  br i1 %cmp.not17.i.i.i.i, label %_ZN3tbb6detail2d119partition_type_baseINS1_19auto_partition_typeEE7executeINS1_9start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEEES8_EEvRT_RT0_RNS1_14execution_dataE.exit, label %for.body.lr.ph.i.i.i.i

for.body.lr.ph.i.i.i.i:                           ; preds = %if.then.i.i
  %p_array_a.i.i.i.i = getelementptr inbounds %"struct.tbb::detail::d1::start_for", %"struct.tbb::detail::d1::start_for"* %this, i64 0, i32 2, i32 0, !intel-tbaa !49
  %p_array_b.i.i.i.i = getelementptr inbounds %"struct.tbb::detail::d1::start_for", %"struct.tbb::detail::d1::start_for"* %this, i64 0, i32 2, i32 1, !intel-tbaa !50
  %p_array_sum.i.i.i.i = getelementptr inbounds %"struct.tbb::detail::d1::start_for", %"struct.tbb::detail::d1::start_for"* %this, i64 0, i32 2, i32 2, !intel-tbaa !51
  %92 = sext i32 %87 to i64
  %.pre.i.i.i.i = load i32*, i32** %p_array_a.i.i.i.i, align 16, !tbaa !49
  %.pre19.i.i.i.i = load i32*, i32** %p_array_b.i.i.i.i, align 8, !tbaa !50
  %.pre20.i.i.i.i = load i32*, i32** %p_array_sum.i.i.i.i, align 32, !tbaa !51
  br label %for.body.i.i.i.i

for.body.i.i.i.i:                                 ; preds = %for.body.i.i.i.i, %for.body.lr.ph.i.i.i.i
  %93 = phi i32* [ %.pre20.i.i.i.i, %for.body.lr.ph.i.i.i.i ], [ %106, %for.body.i.i.i.i ]
  %94 = phi i32* [ %.pre19.i.i.i.i, %for.body.lr.ph.i.i.i.i ], [ %102, %for.body.i.i.i.i ]
  %95 = phi i32* [ %.pre.i.i.i.i, %for.body.lr.ph.i.i.i.i ], [ %98, %for.body.i.i.i.i ]
  %indvars.iv.i.i.i.i = phi i64 [ %92, %for.body.lr.ph.i.i.i.i ], [ %indvars.iv.next.i.i.i.i, %for.body.i.i.i.i ]
  %lsr48 = trunc i64 %indvars.iv.i.i.i.i to i32
  call fastcc void @_ZL14delta_steppingPiS_S_i(i32* noundef %95, i32* noundef %94, i32* noundef %93, i32 noundef %lsr48)
  %96 = bitcast %"struct.tbb::detail::d1::start_for"* %this to i8*
  %sunkaddr73 = getelementptr inbounds i8, i8* %96, i64 80
  %97 = bitcast i8* %sunkaddr73 to i32**
  %98 = load i32*, i32** %97, align 16, !tbaa !49
  %scevgep47 = getelementptr i32, i32* %98, i64 %indvars.iv.i.i.i.i
  %99 = load i32, i32* %scevgep47, align 4, !tbaa !52
  %100 = bitcast %"struct.tbb::detail::d1::start_for"* %this to i8*
  %sunkaddr74 = getelementptr inbounds i8, i8* %100, i64 88
  %101 = bitcast i8* %sunkaddr74 to i32**
  %102 = load i32*, i32** %101, align 8, !tbaa !50
  %scevgep46 = getelementptr i32, i32* %102, i64 %indvars.iv.i.i.i.i
  %103 = load i32, i32* %scevgep46, align 4, !tbaa !52
  %add.i.i.i.i = add nsw i32 %103, %99
  %104 = bitcast %"struct.tbb::detail::d1::start_for"* %this to i8*
  %sunkaddr75 = getelementptr inbounds i8, i8* %104, i64 96
  %105 = bitcast i8* %sunkaddr75 to i32**
  %106 = load i32*, i32** %105, align 32, !tbaa !51
  %scevgep = getelementptr i32, i32* %106, i64 %indvars.iv.i.i.i.i
  store i32 %add.i.i.i.i, i32* %scevgep, align 4, !tbaa !52
  %indvars.iv.next.i.i.i.i = add i64 %indvars.iv.i.i.i.i, 1
  %lsr = trunc i64 %indvars.iv.next.i.i.i.i to i32
  %107 = bitcast %"struct.tbb::detail::d1::start_for"* %this to i8*
  %sunkaddr76 = getelementptr inbounds i8, i8* %107, i64 64
  %108 = bitcast i8* %sunkaddr76 to i32*
  %109 = load i32, i32* %108, align 64, !tbaa !33
  %cmp.not.i.i.i.i = icmp eq i32 %lsr, %109
  br i1 %cmp.not.i.i.i.i, label %_ZN3tbb6detail2d119partition_type_baseINS1_19auto_partition_typeEE7executeINS1_9start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEEES8_EEvRT_RT0_RNS1_14execution_dataE.exit, label %for.body.i.i.i.i, !llvm.loop !53

if.else.i.i:                                      ; preds = %lor.lhs.false.i.i
  %110 = bitcast %"class.tbb::detail::d1::range_vector"* %range_pool.i.i to i8*
  call void @llvm.lifetime.start.p0i8(i64 144, i8* nonnull %110)
  store i8 0, i8* %110, align 8, !tbaa !55
  %my_tail.i.i.i = getelementptr inbounds %"class.tbb::detail::d1::range_vector", %"class.tbb::detail::d1::range_vector"* %range_pool.i.i, i64 0, i32 1, !intel-tbaa !60
  store i8 0, i8* %my_tail.i.i.i, align 1, !tbaa !60
  %my_size.i.i.i = getelementptr inbounds %"class.tbb::detail::d1::range_vector", %"class.tbb::detail::d1::range_vector"* %range_pool.i.i, i64 0, i32 2, !intel-tbaa !61
  store i8 1, i8* %my_size.i.i.i, align 2, !tbaa !61
  %arrayidx.i.i.i = getelementptr inbounds %"class.tbb::detail::d1::range_vector", %"class.tbb::detail::d1::range_vector"* %range_pool.i.i, i64 0, i32 3, i64 0, !intel-tbaa !62
  store i8 0, i8* %arrayidx.i.i.i, align 1, !tbaa !62
  %111 = getelementptr inbounds %"class.tbb::detail::d1::range_vector", %"class.tbb::detail::d1::range_vector"* %range_pool.i.i, i64 0, i32 5, i32 0, i64 0
  %112 = bitcast %"class.tbb::detail::d1::blocked_range"* %my_range to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* noundef nonnull align 8 dereferenceable(16) %111, i8* noundef nonnull align 64 dereferenceable(16) %112, i64 16, i1 false), !tbaa.struct !63
  %my_pool.i.i.i.i.i = getelementptr inbounds %"class.tbb::detail::d1::range_vector", %"class.tbb::detail::d1::range_vector"* %range_pool.i.i, i64 0, i32 5
  %my_body2.i.i.i.i.i.i = getelementptr inbounds %"struct.tbb::detail::d1::start_for", %"struct.tbb::detail::d1::start_for"* %this, i64 0, i32 2
  br label %do.body.i.i

do.body.i.i:                                      ; preds = %land.rhs.i.i, %if.else.i.i
  %113 = phi i8 [ %212, %land.rhs.i.i ], [ 0, %if.else.i.i ]
  %.pr138.i.i = phi i8 [ %my_size.i.promoted.i104147.i.i, %land.rhs.i.i ], [ 1, %if.else.i.i ]
  %114 = phi i8 [ %my_head.i.promoted.i107149.i.i, %land.rhs.i.i ], [ 0, %if.else.i.i ]
  %my_head.i.i.promoted.i.i.i = phi i8 [ %my_head.i.i.promoted.i135151.i.i, %land.rhs.i.i ], [ 0, %if.else.i.i ]
  %115 = bitcast %"struct.tbb::detail::d1::start_for"* %this to i8*
  %sunkaddr79 = getelementptr inbounds i8, i8* %115, i64 124
  %116 = load i8, i8* %sunkaddr79, align 4, !tbaa !28
  %cmp37.i.i.i = icmp ult i8 %.pr138.i.i, 8
  br i1 %cmp37.i.i.i, label %land.rhs.lr.ph.i.i.i, label %invoke.cont6.i.i

land.rhs.lr.ph.i.i.i:                             ; preds = %do.body.i.i
  %idxprom.i.i.i120.i.i = zext i8 %my_head.i.i.promoted.i.i.i to i64
  %arrayidx.i.i.i121.i.i = getelementptr inbounds %"class.tbb::detail::d1::range_vector", %"class.tbb::detail::d1::range_vector"* %range_pool.i.i, i64 0, i32 3, i64 %idxprom.i.i.i120.i.i, !intel-tbaa !62
  %117 = load i8, i8* %arrayidx.i.i.i121.i.i, align 1, !tbaa !62
  %cmp.i.i122.i.i = icmp ult i8 %117, %116
  br i1 %cmp.i.i122.i.i, label %_ZN3tbb6detail2d112range_vectorINS1_13blocked_rangeIiEELh8EE12is_divisibleEh.exit.i.i.i.preheader, label %invoke.cont6.loopexit.i.i

_ZN3tbb6detail2d112range_vectorINS1_13blocked_rangeIiEELh8EE12is_divisibleEh.exit.i.i.i.preheader: ; preds = %land.rhs.lr.ph.i.i.i
  br label %_ZN3tbb6detail2d112range_vectorINS1_13blocked_rangeIiEELh8EE12is_divisibleEh.exit.i.i.i

land.rhs.i.i.i:                                   ; preds = %while.body.i.i.i
  %cmp.i.i.i.i = icmp ult i8 %inc.i.i.i, %116
  %inc32.i.i.i = add i8 %118, 1
  br i1 %cmp.i.i.i.i, label %_ZN3tbb6detail2d112range_vectorINS1_13blocked_rangeIiEELh8EE12is_divisibleEh.exit.i.i.i, label %invoke.cont6.loopexit.i.i, !llvm.loop !64

_ZN3tbb6detail2d112range_vectorINS1_13blocked_rangeIiEELh8EE12is_divisibleEh.exit.i.i.i: ; preds = %land.rhs.i.i.i, %_ZN3tbb6detail2d112range_vectorINS1_13blocked_rangeIiEELh8EE12is_divisibleEh.exit.i.i.i.preheader
  %arrayidx.i.i.i127.i.i = phi i8* [ %arrayidx30.i.i.i, %land.rhs.i.i.i ], [ %arrayidx.i.i.i121.i.i, %_ZN3tbb6detail2d112range_vectorINS1_13blocked_rangeIiEELh8EE12is_divisibleEh.exit.i.i.i.preheader ]
  %idxprom.i.i.i126.i.i = phi i64 [ %rem.i.i.i, %land.rhs.i.i.i ], [ %idxprom.i.i.i120.i.i, %_ZN3tbb6detail2d112range_vectorINS1_13blocked_rangeIiEELh8EE12is_divisibleEh.exit.i.i.i.preheader ]
  %118 = phi i8 [ %inc32.i.i.i, %land.rhs.i.i.i ], [ %.pr138.i.i, %_ZN3tbb6detail2d112range_vectorINS1_13blocked_rangeIiEELh8EE12is_divisibleEh.exit.i.i.i.preheader ]
  %rem.i116124.i.i = phi i8 [ %promoted85, %land.rhs.i.i.i ], [ %my_head.i.i.promoted.i.i.i, %_ZN3tbb6detail2d112range_vectorINS1_13blocked_rangeIiEELh8EE12is_divisibleEh.exit.i.i.i.preheader ]
  %119 = bitcast %"class.tbb::detail::d0::aligned_space"* %my_pool.i.i.i.i.i to %"class.tbb::detail::d1::blocked_range"*
  %my_grainsize.i.i.i.i.i = getelementptr inbounds %"class.tbb::detail::d1::blocked_range", %"class.tbb::detail::d1::blocked_range"* %119, i64 %idxprom.i.i.i126.i.i, i32 2
  %120 = load i64, i64* %my_grainsize.i.i.i.i.i, align 8, !tbaa !65
  %my_end.i.i.i.i.i.i = getelementptr inbounds %"class.tbb::detail::d1::blocked_range", %"class.tbb::detail::d1::blocked_range"* %119, i64 %idxprom.i.i.i126.i.i, i32 0
  %121 = load i32, i32* %my_end.i.i.i.i.i.i, align 8, !tbaa !66
  %conv.i.i.i.i.i.i = sext i32 %121 to i64
  %my_begin.i.i.i.i.i33.i = getelementptr inbounds %"class.tbb::detail::d1::blocked_range", %"class.tbb::detail::d1::blocked_range"* %119, i64 %idxprom.i.i.i126.i.i, i32 1
  %122 = load i32, i32* %my_begin.i.i.i.i.i33.i, align 4, !tbaa !67
  %promoted = sext i32 %122 to i64
  %sub.i.i.i.i.i.i = sub nsw i64 %conv.i.i.i.i.i.i, %promoted
  %cmp.i.i.i.i.i = icmp ult i64 %120, %sub.i.i.i.i.i.i
  br i1 %cmp.i.i.i.i.i, label %while.body.i.i.i, label %invoke.cont6.loopexit.i.i

while.body.i.i.i:                                 ; preds = %_ZN3tbb6detail2d112range_vectorINS1_13blocked_rangeIiEELh8EE12is_divisibleEh.exit.i.i.i
  %123 = bitcast %"class.tbb::detail::d0::aligned_space"* %my_pool.i.i.i.i.i to %"class.tbb::detail::d1::blocked_range"*
  %add.i.i.i = add i8 %rem.i116124.i.i, 1
  %idx.ext.i.i.i = zext i8 %add.i.i.i to i64
  %rem.i.i.i = and i64 %idx.ext.i.i.i, 7
  %promoted85 = trunc i64 %rem.i.i.i to i8
  %add.ptr.i.i.i = getelementptr inbounds %"class.tbb::detail::d1::blocked_range", %"class.tbb::detail::d1::blocked_range"* %123, i64 %rem.i.i.i, !intel-tbaa !68
  %124 = bitcast %"class.tbb::detail::d1::blocked_range"* %add.ptr.i.i.i to i8*
  %arrayidx.i46.i.i = getelementptr inbounds %"class.tbb::detail::d1::blocked_range", %"class.tbb::detail::d1::blocked_range"* %123, i64 %idxprom.i.i.i126.i.i
  %125 = bitcast %"class.tbb::detail::d1::blocked_range"* %arrayidx.i46.i.i to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* noundef nonnull align 8 dereferenceable(16) %124, i8* noundef nonnull align 8 dereferenceable(16) %125, i64 16, i1 false), !tbaa.struct !63
  %my_end.i.i47.i.i81 = bitcast %"class.tbb::detail::d1::blocked_range"* %arrayidx.i46.i.i to i32*
  %my_end2.i.i.i.i82 = bitcast %"class.tbb::detail::d1::blocked_range"* %add.ptr.i.i.i to i32*
  %126 = load i32, i32* %my_end2.i.i.i.i82, align 8, !tbaa !66
  store i32 %126, i32* %my_end.i.i47.i.i81, align 8, !tbaa !66
  %my_begin.i.i.i48.i.i = getelementptr inbounds %"class.tbb::detail::d1::blocked_range", %"class.tbb::detail::d1::blocked_range"* %123, i64 %rem.i.i.i, i32 1
  %127 = load i32, i32* %my_begin.i.i.i48.i.i, align 4, !tbaa !67
  %128 = load i32, i32* %my_end2.i.i.i.i82, align 8, !tbaa !66
  %sub.i.i.i.i.i = sub nsw i32 %128, %127
  %div8.i.i.i.i.i = lshr i32 %sub.i.i.i.i.i, 1
  %add.i.i.i.i.i = add i32 %div8.i.i.i.i.i, %127
  store i32 %add.i.i.i.i.i, i32* %my_end2.i.i.i.i82, align 8, !tbaa !66
  store i32 %add.i.i.i.i.i, i32* %my_begin.i.i.i.i.i33.i, align 4, !tbaa !67
  %my_grainsize3.i.i.i.i = getelementptr inbounds %"class.tbb::detail::d1::blocked_range", %"class.tbb::detail::d1::blocked_range"* %123, i64 %rem.i.i.i, i32 2
  %129 = load i64, i64* %my_grainsize3.i.i.i.i, align 8, !tbaa !65
  store i64 %129, i64* %my_grainsize.i.i.i.i.i, align 8, !tbaa !65
  %130 = load i8, i8* %arrayidx.i.i.i127.i.i, align 1, !tbaa !62
  %inc.i.i.i = add i8 %130, 1
  store i8 %inc.i.i.i, i8* %arrayidx.i.i.i127.i.i, align 1, !tbaa !62
  %arrayidx30.i.i.i = getelementptr inbounds %"class.tbb::detail::d1::range_vector", %"class.tbb::detail::d1::range_vector"* %range_pool.i.i, i64 0, i32 3, i64 %rem.i.i.i
  store i8 %inc.i.i.i, i8* %arrayidx30.i.i.i, align 1, !tbaa !62
  %exitcond.not.i.i.i = icmp eq i8 %118, 7
  br i1 %exitcond.not.i.i.i, label %invoke.cont6.loopexit.i.i, label %land.rhs.i.i.i, !llvm.loop !64

invoke.cont6.loopexit.i.i:                        ; preds = %while.body.i.i.i, %_ZN3tbb6detail2d112range_vectorINS1_13blocked_rangeIiEELh8EE12is_divisibleEh.exit.i.i.i, %land.rhs.i.i.i, %land.rhs.lr.ph.i.i.i
  %inc32.i119.i.i = phi i8 [ %.pr138.i.i, %land.rhs.lr.ph.i.i.i ], [ 8, %while.body.i.i.i ], [ %inc32.i.i.i, %land.rhs.i.i.i ], [ %118, %_ZN3tbb6detail2d112range_vectorINS1_13blocked_rangeIiEELh8EE12is_divisibleEh.exit.i.i.i ]
  %rem.i117.i.i = phi i8 [ %my_head.i.i.promoted.i.i.i, %land.rhs.lr.ph.i.i.i ], [ %promoted85, %while.body.i.i.i ], [ %promoted85, %land.rhs.i.i.i ], [ %rem.i116124.i.i, %_ZN3tbb6detail2d112range_vectorINS1_13blocked_rangeIiEELh8EE12is_divisibleEh.exit.i.i.i ]
  %131 = bitcast %"class.tbb::detail::d1::range_vector"* %range_pool.i.i to i8*
  store i8 %rem.i117.i.i, i8* %131, align 8, !tbaa !55
  %132 = bitcast %"class.tbb::detail::d1::range_vector"* %range_pool.i.i to i8*
  %sunkaddr86 = getelementptr inbounds i8, i8* %132, i64 2
  store i8 %inc32.i119.i.i, i8* %sunkaddr86, align 2, !tbaa !61
  br label %invoke.cont6.i.i

invoke.cont6.i.i:                                 ; preds = %invoke.cont6.loopexit.i.i, %do.body.i.i
  %133 = phi i8 [ %inc32.i119.i.i, %invoke.cont6.loopexit.i.i ], [ %.pr138.i.i, %do.body.i.i ]
  %134 = phi i8 [ %rem.i117.i.i, %invoke.cont6.loopexit.i.i ], [ %114, %do.body.i.i ]
  %my_head.i.i.promoted.i136.i.i = phi i8 [ %rem.i117.i.i, %invoke.cont6.loopexit.i.i ], [ %my_head.i.i.promoted.i.i.i, %do.body.i.i ]
  %135 = bitcast %"struct.tbb::detail::d1::start_for"* %this to i8*
  %sunkaddr87 = getelementptr inbounds i8, i8* %135, i64 104
  %136 = bitcast i8* %sunkaddr87 to %"struct.tbb::detail::d1::tree_node"**
  %137 = load %"struct.tbb::detail::d1::tree_node"*, %"struct.tbb::detail::d1::tree_node"** %136, align 8, !tbaa !13
  %_M_i.i.i.i.i.i.i = getelementptr inbounds %"struct.tbb::detail::d1::tree_node", %"struct.tbb::detail::d1::tree_node"* %137, i64 0, i32 3, i32 0, i32 0, !intel-tbaa !23
  %138 = load atomic i8, i8* %_M_i.i.i.i.i.i.i monotonic, align 1
  %139 = and i8 %138, 1
  %tobool.i.i.i.i.not.i.i = icmp eq i8 %139, 0
  br i1 %tobool.i.i.i.i.not.i.i, label %invoke.cont6.invoke.cont28_crit_edge.i.i, label %if.then10.i.i

invoke.cont6.invoke.cont28_crit_edge.i.i:         ; preds = %invoke.cont6.i.i
  %.pre.i.i = zext i8 %134 to i64
  br label %invoke.cont28.i.i

if.then10.i.i:                                    ; preds = %invoke.cont6.i.i
  %add.i49.i.i = add i8 %116, 1
  %140 = bitcast %"struct.tbb::detail::d1::start_for"* %this to i8*
  %sunkaddr88 = getelementptr inbounds i8, i8* %140, i64 124
  store i8 %add.i49.i.i, i8* %sunkaddr88, align 4, !tbaa !28
  %cmp.i34.i = icmp ugt i8 %133, 1
  br i1 %cmp.i34.i, label %invoke.cont14.i.i, label %if.end.i37.i

invoke.cont14.i.i:                                ; preds = %if.then10.i.i
  %141 = bitcast %"struct.tbb::detail::d1::execution_data"* %ed to %"class.tbb::detail::d1::task_group_context"**
  %142 = bitcast %class.ArraySummer* %my_body2.i.i.i.i.i.i to i8*
  %143 = bitcast %"class.tbb::detail::d1::small_object_allocator"* %alloc.i.i.i.i to %"class.tbb::detail::d1::small_object_pool"**
  %144 = bitcast %"class.tbb::detail::d1::small_object_allocator"* %alloc.i.i.i.i to i8*
  %145 = bitcast %"class.tbb::detail::d0::aligned_space"* %my_pool.i.i.i.i.i to %"class.tbb::detail::d1::blocked_range"*
  %idxprom.i.i.i = zext i8 %113 to i64
  %arrayidx.i55.i.i = getelementptr inbounds %"class.tbb::detail::d1::range_vector", %"class.tbb::detail::d1::range_vector"* %range_pool.i.i, i64 0, i32 3, i64 %idxprom.i.i.i, !intel-tbaa !62
  %146 = load i8, i8* %arrayidx.i55.i.i, align 1, !tbaa !62
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %144)
  store %"class.tbb::detail::d1::small_object_pool"* null, %"class.tbb::detail::d1::small_object_pool"** %143, align 8, !tbaa !35
  %call.i.i.i57.i.i = call noundef i8* @_ZN3tbb6detail2r18allocateERPNS0_2d117small_object_poolEmRKNS2_14execution_dataE(%"class.tbb::detail::d1::small_object_pool"** noundef nonnull align 8 dereferenceable(8) %143, i64 noundef 192, %"struct.tbb::detail::d1::execution_data"* noundef nonnull align 8 dereferenceable(12) %ed)
  %arrayidx.i52.i.i = getelementptr inbounds %"class.tbb::detail::d1::blocked_range", %"class.tbb::detail::d1::blocked_range"* %145, i64 %idxprom.i.i.i
  %m_version_and_traits.i.i.i.i.i.i.i.i = getelementptr inbounds i8, i8* %call.i.i.i57.i.i, i64 8
  %arrayinit.end.i.i.i.i.i.i.i = getelementptr inbounds i8, i8* %call.i.i.i57.i.i, i64 64
  %147 = bitcast i8* %call.i.i.i57.i.i to i32 (...)***
  call void @llvm.memset.p0i8.i64(i8* noundef nonnull align 8 dereferenceable(56) %m_version_and_traits.i.i.i.i.i.i.i.i, i8 0, i64 56, i1 false)
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [6 x i8*] }, { [6 x i8*] }* @_ZTVN3tbb6detail2d19start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEEE, i64 0, inrange i32 0, i64 2) to i32 (...)**), i32 (...)*** %147, align 64, !tbaa !37
  %148 = bitcast %"class.tbb::detail::d1::blocked_range"* %arrayidx.i52.i.i to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* noundef nonnull align 64 dereferenceable(16) %arrayinit.end.i.i.i.i.i.i.i, i8* noundef nonnull align 8 dereferenceable(16) %148, i64 16, i1 false), !tbaa.struct !63
  %my_body.i.i.i.i.i.i = getelementptr inbounds i8, i8* %call.i.i.i57.i.i, i64 80
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* noundef nonnull align 16 dereferenceable(24) %my_body.i.i.i.i.i.i, i8* noundef nonnull align 16 dereferenceable(24) %142, i64 24, i1 false), !tbaa.struct !39
  %my_divisor.i.i.i.i.i.i.i.i35.i = getelementptr inbounds i8, i8* %call.i.i.i57.i.i, i64 112
  %149 = bitcast i8* %my_divisor.i.i.i.i.i.i.i.i35.i to i64*
  %150 = bitcast %"struct.tbb::detail::d1::start_for"* %this to i8*
  %sunkaddr89 = getelementptr inbounds i8, i8* %150, i64 112
  %151 = bitcast i8* %sunkaddr89 to i64*
  %152 = load i64, i64* %151, align 16, !tbaa !9
  %div2.i.i.i.i.i.i.i.i.i.i = lshr i64 %152, 1
  store i64 %div2.i.i.i.i.i.i.i.i.i.i, i64* %151, align 16, !tbaa !9
  store i64 %div2.i.i.i.i.i.i.i.i.i.i, i64* %149, align 16, !tbaa !9
  %153 = getelementptr inbounds i8, i8* %call.i.i.i57.i.i, i64 120
  %154 = bitcast i8* %153 to i32*
  store i32 2, i32* %154, align 8, !tbaa !41
  %155 = getelementptr inbounds i8, i8* %call.i.i.i57.i.i, i64 124
  %156 = bitcast %"struct.tbb::detail::d1::start_for"* %this to i8*
  %sunkaddr90 = getelementptr inbounds i8, i8* %156, i64 124
  %157 = load i8, i8* %sunkaddr90, align 4, !tbaa !28
  %158 = getelementptr inbounds i8, i8* %call.i.i.i57.i.i, i64 128
  %159 = bitcast i8* %158 to %"class.tbb::detail::d1::small_object_pool"**
  %160 = load %"class.tbb::detail::d1::small_object_pool"*, %"class.tbb::detail::d1::small_object_pool"** %143, align 8, !tbaa !35
  store %"class.tbb::detail::d1::small_object_pool"* %160, %"class.tbb::detail::d1::small_object_pool"** %159, align 64, !tbaa !42
  %sub.i.i.i.i.i.i36.i = sub i8 %157, %146
  store i8 %sub.i.i.i.i.i.i36.i, i8* %155, align 4, !tbaa !28
  %call.i15.i.i58.i.i = call noundef i8* @_ZN3tbb6detail2r18allocateERPNS0_2d117small_object_poolEmRKNS2_14execution_dataE(%"class.tbb::detail::d1::small_object_pool"** noundef nonnull align 8 dereferenceable(8) %143, i64 noundef 32, %"struct.tbb::detail::d1::execution_data"* noundef nonnull align 8 dereferenceable(12) %ed)
  %161 = bitcast %"struct.tbb::detail::d1::start_for"* %this to i8*
  %sunkaddr91 = getelementptr inbounds i8, i8* %161, i64 104
  %162 = bitcast i8* %sunkaddr91 to %"struct.tbb::detail::d1::node"**
  %163 = load %"struct.tbb::detail::d1::node"*, %"struct.tbb::detail::d1::node"** %162, align 8, !tbaa !13
  %my_parent.i.i.i.i.i.i.i = bitcast i8* %call.i15.i.i58.i.i to %"struct.tbb::detail::d1::node"**
  store %"struct.tbb::detail::d1::node"* %163, %"struct.tbb::detail::d1::node"** %my_parent.i.i.i.i.i.i.i, align 8, !tbaa !43
  %164 = getelementptr inbounds i8, i8* %call.i15.i.i58.i.i, i64 8
  %165 = bitcast i8* %164 to i32*
  store i32 2, i32* %165, align 8, !tbaa !46
  %166 = getelementptr inbounds i8, i8* %call.i15.i.i58.i.i, i64 16
  %167 = bitcast i8* %166 to %"class.tbb::detail::d1::small_object_pool"**
  %168 = load %"class.tbb::detail::d1::small_object_pool"*, %"class.tbb::detail::d1::small_object_pool"** %143, align 8, !tbaa !35
  store %"class.tbb::detail::d1::small_object_pool"* %168, %"class.tbb::detail::d1::small_object_pool"** %167, align 8, !tbaa !42
  %169 = getelementptr inbounds i8, i8* %call.i15.i.i58.i.i, i64 24
  store i8 0, i8* %169, align 8, !tbaa !23
  %170 = bitcast %"struct.tbb::detail::d1::start_for"* %this to i8*
  %sunkaddr92 = getelementptr inbounds i8, i8* %170, i64 104
  %171 = bitcast i8* %sunkaddr92 to i8**
  store i8* %call.i15.i.i58.i.i, i8** %171, align 8, !tbaa !13
  %my_parent8.i.i.i.i = getelementptr inbounds i8, i8* %call.i.i.i57.i.i, i64 104
  %172 = bitcast i8* %my_parent8.i.i.i.i to i8**
  store i8* %call.i15.i.i58.i.i, i8** %172, align 8, !tbaa !13
  %173 = bitcast i8* %call.i.i.i57.i.i to %"class.tbb::detail::d1::task"*
  %174 = load %"class.tbb::detail::d1::task_group_context"*, %"class.tbb::detail::d1::task_group_context"** %141, align 8, !tbaa !48
  call void @_ZN3tbb6detail2r15spawnERNS0_2d14taskERNS2_18task_group_contextE(%"class.tbb::detail::d1::task"* noundef nonnull align 64 dereferenceable(64) %173, %"class.tbb::detail::d1::task_group_context"* noundef nonnull align 8 dereferenceable(128) %174)
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %144)
  %dec.i.i.i = add i8 %133, -1
  %175 = bitcast %"class.tbb::detail::d1::range_vector"* %range_pool.i.i to i8*
  %sunkaddr93 = getelementptr inbounds i8, i8* %175, i64 2
  store i8 %dec.i.i.i, i8* %sunkaddr93, align 2, !tbaa !61
  %176 = add i8 %113, 1
  %177 = and i8 %176, 7
  %178 = bitcast %"class.tbb::detail::d1::range_vector"* %range_pool.i.i to i8*
  %sunkaddr94 = getelementptr inbounds i8, i8* %178, i64 1
  store i8 %177, i8* %sunkaddr94, align 1, !tbaa !60
  br label %land.rhs.i.i

if.end.i37.i:                                     ; preds = %if.then10.i.i
  %idxprom.i.i.i.i = zext i8 %134 to i64
  %arrayidx.i.i65.i.i = getelementptr inbounds %"class.tbb::detail::d1::range_vector", %"class.tbb::detail::d1::range_vector"* %range_pool.i.i, i64 0, i32 3, i64 %idxprom.i.i.i.i, !intel-tbaa !62
  %179 = load i8, i8* %arrayidx.i.i65.i.i, align 1, !tbaa !62
  %cmp.i66.i.i = icmp ult i8 %179, %add.i49.i.i
  br i1 %cmp.i66.i.i, label %invoke.cont23.i.i, label %invoke.cont28.i.i

invoke.cont23.i.i:                                ; preds = %if.end.i37.i
  %180 = bitcast %"class.tbb::detail::d0::aligned_space"* %my_pool.i.i.i.i.i to %"class.tbb::detail::d1::blocked_range"*
  %my_grainsize.i.i.i.i = getelementptr inbounds %"class.tbb::detail::d1::blocked_range", %"class.tbb::detail::d1::blocked_range"* %180, i64 %idxprom.i.i.i.i, i32 2
  %181 = load i64, i64* %my_grainsize.i.i.i.i, align 8, !tbaa !65
  %my_end.i.i.i67.i.i = getelementptr inbounds %"class.tbb::detail::d1::blocked_range", %"class.tbb::detail::d1::blocked_range"* %180, i64 %idxprom.i.i.i.i, i32 0
  %182 = load i32, i32* %my_end.i.i.i67.i.i, align 8, !tbaa !66
  %conv.i.i.i.i.i = sext i32 %182 to i64
  %my_begin.i.i.i68.i.i = getelementptr inbounds %"class.tbb::detail::d1::blocked_range", %"class.tbb::detail::d1::blocked_range"* %180, i64 %idxprom.i.i.i.i, i32 1
  %183 = load i32, i32* %my_begin.i.i.i68.i.i, align 4, !tbaa !67
  %promoted95 = sext i32 %183 to i64
  %sub.i.i.i69.i.i = sub nsw i64 %conv.i.i.i.i.i, %promoted95
  %cmp.i.i70.i.i = icmp ult i64 %181, %sub.i.i.i69.i.i
  br i1 %cmp.i.i70.i.i, label %do.cond.i.i, label %invoke.cont28.i.i

invoke.cont28.i.i:                                ; preds = %invoke.cont23.i.i, %if.end.i37.i, %invoke.cont6.invoke.cont28_crit_edge.i.i
  %idxprom.i74.pre-phi.i.i = phi i64 [ %.pre.i.i, %invoke.cont6.invoke.cont28_crit_edge.i.i ], [ %idxprom.i.i.i.i, %if.end.i37.i ], [ %idxprom.i.i.i.i, %invoke.cont23.i.i ]
  %184 = bitcast %"class.tbb::detail::d0::aligned_space"* %my_pool.i.i.i.i.i to %"class.tbb::detail::d1::blocked_range"*
  %my_begin.i.i.i76.i.i = getelementptr inbounds %"class.tbb::detail::d1::blocked_range", %"class.tbb::detail::d1::blocked_range"* %184, i64 %idxprom.i74.pre-phi.i.i, i32 1
  %185 = load i32, i32* %my_begin.i.i.i76.i.i, align 4, !tbaa !67
  %186 = sext i32 %185 to i64
  %my_end.i.i.i77.i.i = getelementptr inbounds %"class.tbb::detail::d1::blocked_range", %"class.tbb::detail::d1::blocked_range"* %184, i64 %idxprom.i74.pre-phi.i.i, i32 0
  %187 = load i32, i32* %my_end.i.i.i77.i.i, align 8, !tbaa !66
  %cmp.not17.i.i78.i.i = icmp eq i32 %185, %187
  br i1 %cmp.not17.i.i78.i.i, label %invoke.cont30.i.i, label %for.body.lr.ph.i.i85.i.i

for.body.lr.ph.i.i85.i.i:                         ; preds = %invoke.cont28.i.i
  %188 = bitcast %"struct.tbb::detail::d1::start_for"* %this to i8*
  %sunkaddr96 = getelementptr inbounds i8, i8* %188, i64 80
  %189 = bitcast i8* %sunkaddr96 to i32**
  %.pre.i.i82.i.i = load i32*, i32** %189, align 16, !tbaa !49
  %190 = bitcast %"struct.tbb::detail::d1::start_for"* %this to i8*
  %sunkaddr97 = getelementptr inbounds i8, i8* %190, i64 88
  %191 = bitcast i8* %sunkaddr97 to i32**
  %.pre19.i.i83.i.i = load i32*, i32** %191, align 8, !tbaa !50
  %192 = bitcast %"struct.tbb::detail::d1::start_for"* %this to i8*
  %sunkaddr98 = getelementptr inbounds i8, i8* %192, i64 96
  %193 = bitcast i8* %sunkaddr98 to i32**
  %.pre20.i.i84.i.i = load i32*, i32** %193, align 32, !tbaa !51
  br label %for.body.i.i93.i.i

for.body.i.i93.i.i:                               ; preds = %for.body.i.i93.i.i, %for.body.lr.ph.i.i85.i.i
  %194 = phi i32* [ %.pre20.i.i84.i.i, %for.body.lr.ph.i.i85.i.i ], [ %207, %for.body.i.i93.i.i ]
  %195 = phi i32* [ %.pre19.i.i83.i.i, %for.body.lr.ph.i.i85.i.i ], [ %203, %for.body.i.i93.i.i ]
  %196 = phi i32* [ %.pre.i.i82.i.i, %for.body.lr.ph.i.i85.i.i ], [ %199, %for.body.i.i93.i.i ]
  %indvars.iv.i.i86.i.i = phi i64 [ %186, %for.body.lr.ph.i.i85.i.i ], [ %indvars.iv.next.i.i91.i.i, %for.body.i.i93.i.i ]
  %lsr53 = trunc i64 %indvars.iv.i.i86.i.i to i32
  call fastcc void @_ZL14delta_steppingPiS_S_i(i32* noundef %196, i32* noundef %195, i32* noundef %194, i32 noundef %lsr53)
  %197 = bitcast %"struct.tbb::detail::d1::start_for"* %this to i8*
  %sunkaddr99 = getelementptr inbounds i8, i8* %197, i64 80
  %198 = bitcast i8* %sunkaddr99 to i32**
  %199 = load i32*, i32** %198, align 16, !tbaa !49
  %scevgep51 = getelementptr i32, i32* %199, i64 %indvars.iv.i.i86.i.i
  %200 = load i32, i32* %scevgep51, align 4, !tbaa !52
  %201 = bitcast %"struct.tbb::detail::d1::start_for"* %this to i8*
  %sunkaddr100 = getelementptr inbounds i8, i8* %201, i64 88
  %202 = bitcast i8* %sunkaddr100 to i32**
  %203 = load i32*, i32** %202, align 8, !tbaa !50
  %scevgep50 = getelementptr i32, i32* %203, i64 %indvars.iv.i.i86.i.i
  %204 = load i32, i32* %scevgep50, align 4, !tbaa !52
  %add.i.i89.i.i = add nsw i32 %204, %200
  %205 = bitcast %"struct.tbb::detail::d1::start_for"* %this to i8*
  %sunkaddr101 = getelementptr inbounds i8, i8* %205, i64 96
  %206 = bitcast i8* %sunkaddr101 to i32**
  %207 = load i32*, i32** %206, align 32, !tbaa !51
  %scevgep49 = getelementptr i32, i32* %207, i64 %indvars.iv.i.i86.i.i
  store i32 %add.i.i89.i.i, i32* %scevgep49, align 4, !tbaa !52
  %indvars.iv.next.i.i91.i.i = add nsw i64 %indvars.iv.i.i86.i.i, 1
  %lsr52 = trunc i64 %indvars.iv.next.i.i91.i.i to i32
  %cmp.not.i.i92.i.i = icmp eq i32 %187, %lsr52
  br i1 %cmp.not.i.i92.i.i, label %invoke.cont30.i.i, label %for.body.i.i93.i.i, !llvm.loop !53

invoke.cont30.i.i:                                ; preds = %for.body.i.i93.i.i, %invoke.cont28.i.i
  %208 = bitcast %"class.tbb::detail::d1::range_vector"* %range_pool.i.i to i8*
  %dec.i97.i.i = add i8 %133, -1
  %209 = bitcast %"class.tbb::detail::d1::range_vector"* %range_pool.i.i to i8*
  %sunkaddr102 = getelementptr inbounds i8, i8* %209, i64 2
  store i8 %dec.i97.i.i, i8* %sunkaddr102, align 2, !tbaa !61
  %210 = add i8 %134, 7
  %211 = and i8 %210, 7
  store i8 %211, i8* %208, align 8, !tbaa !55
  br label %do.cond.i.i

do.cond.i.i:                                      ; preds = %invoke.cont30.i.i, %invoke.cont23.i.i
  %my_size.i.promoted.i104.i.i = phi i8 [ %dec.i97.i.i, %invoke.cont30.i.i ], [ %133, %invoke.cont23.i.i ]
  %my_head.i.promoted.i107.i.i = phi i8 [ %211, %invoke.cont30.i.i ], [ %134, %invoke.cont23.i.i ]
  %cmp.i99.i.i = icmp eq i8 %my_size.i.promoted.i104.i.i, 0
  br i1 %cmp.i99.i.i, label %_ZN3tbb6detail2d112range_vectorINS1_13blocked_rangeIiEELh8EED2Ev.exit114.i.i, label %land.rhs.i.i

land.rhs.i.i:                                     ; preds = %do.cond.i.i, %invoke.cont14.i.i
  %my_head.i.i.promoted.i135151.i.i = phi i8 [ %my_head.i.i.promoted.i136.i.i, %invoke.cont14.i.i ], [ %my_head.i.promoted.i107.i.i, %do.cond.i.i ]
  %my_head.i.promoted.i107149.i.i = phi i8 [ %134, %invoke.cont14.i.i ], [ %my_head.i.promoted.i107.i.i, %do.cond.i.i ]
  %my_size.i.promoted.i104147.i.i = phi i8 [ %dec.i.i.i, %invoke.cont14.i.i ], [ %my_size.i.promoted.i104.i.i, %do.cond.i.i ]
  %212 = phi i8 [ %177, %invoke.cont14.i.i ], [ %113, %do.cond.i.i ]
  %213 = bitcast %"struct.tbb::detail::d1::execution_data"* %ed to %"class.tbb::detail::d1::task_group_context"**
  %214 = load %"class.tbb::detail::d1::task_group_context"*, %"class.tbb::detail::d1::task_group_context"** %213, align 8, !tbaa !48
  %_M_i.i.i.i.i100.i.i = getelementptr inbounds %"class.tbb::detail::d1::task_group_context", %"class.tbb::detail::d1::task_group_context"* %214, i64 0, i32 5, i32 0, !intel-tbaa !69
  %215 = load atomic i8, i8* %_M_i.i.i.i.i100.i.i monotonic, align 1
  %cmp.i.i.i101.i.i = icmp eq i8 %215, -1
  %my_actual_context.i.i.i.i = getelementptr inbounds %"class.tbb::detail::d1::task_group_context", %"class.tbb::detail::d1::task_group_context"* %214, i64 0, i32 6, i32 0
  %216 = load %"class.tbb::detail::d1::task_group_context"*, %"class.tbb::detail::d1::task_group_context"** %my_actual_context.i.i.i.i, align 8
  %217 = select i1 %cmp.i.i.i101.i.i, %"class.tbb::detail::d1::task_group_context"* %216, %"class.tbb::detail::d1::task_group_context"* %214
  %call2.i102.i.i = call noundef zeroext i1 @_ZN3tbb6detail2r128is_group_execution_cancelledERNS0_2d118task_group_contextE(%"class.tbb::detail::d1::task_group_context"* noundef nonnull align 8 dereferenceable(128) %217)
  br i1 %call2.i102.i.i, label %_ZN3tbb6detail2d112range_vectorINS1_13blocked_rangeIiEELh8EED2Ev.exit114.i.i, label %do.body.i.i, !llvm.loop !86

_ZN3tbb6detail2d112range_vectorINS1_13blocked_rangeIiEELh8EED2Ev.exit114.i.i: ; preds = %land.rhs.i.i, %do.cond.i.i
  %218 = bitcast %"class.tbb::detail::d1::range_vector"* %range_pool.i.i to i8*
  call void @llvm.lifetime.end.p0i8(i64 144, i8* nonnull %218)
  br label %_ZN3tbb6detail2d119partition_type_baseINS1_19auto_partition_typeEE7executeINS1_9start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEEES8_EEvRT_RT0_RNS1_14execution_dataE.exit

_ZN3tbb6detail2d119partition_type_baseINS1_19auto_partition_typeEE7executeINS1_9start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEEES8_EEvRT_RT0_RNS1_14execution_dataE.exit: ; preds = %_ZN3tbb6detail2d112range_vectorINS1_13blocked_rangeIiEELh8EED2Ev.exit114.i.i, %for.body.i.i.i.i, %if.then.i.i
  %my_parent.i12 = getelementptr inbounds %"struct.tbb::detail::d1::start_for", %"struct.tbb::detail::d1::start_for"* %this, i64 0, i32 3, !intel-tbaa !13
  %219 = load %"struct.tbb::detail::d1::node"*, %"struct.tbb::detail::d1::node"** %my_parent.i12, align 8, !tbaa !13
  %220 = getelementptr inbounds %"struct.tbb::detail::d1::start_for", %"struct.tbb::detail::d1::start_for"* %this, i64 0, i32 5, i32 0
  %221 = load %"class.tbb::detail::d1::small_object_pool"*, %"class.tbb::detail::d1::small_object_pool"** %220, align 64, !tbaa !42
  %222 = bitcast %"struct.tbb::detail::d1::start_for"* %this to void (%"struct.tbb::detail::d1::start_for"*)***
  %vtable.i = load void (%"struct.tbb::detail::d1::start_for"*)**, void (%"struct.tbb::detail::d1::start_for"*)*** %222, align 64, !tbaa !37
  %223 = load void (%"struct.tbb::detail::d1::start_for"*)*, void (%"struct.tbb::detail::d1::start_for"*)** %vtable.i, align 8
  call void %223(%"struct.tbb::detail::d1::start_for"* noundef nonnull align 64 dereferenceable(136) %this)
  %_M_i.i18.i.i = getelementptr inbounds %"struct.tbb::detail::d1::node", %"struct.tbb::detail::d1::node"* %219, i64 0, i32 1, i32 0, i32 0
  %224 = atomicrmw sub i32* %_M_i.i18.i.i, i32 1 seq_cst, align 4
  %225 = add i32 %224, -1
  %cmp19.i.i = icmp sgt i32 %225, 0
  br i1 %cmp19.i.i, label %_ZN3tbb6detail2d19start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEE8finalizeERKNS1_14execution_dataE.exit, label %if.end.i.i14.preheader

if.end.i.i14.preheader:                           ; preds = %_ZN3tbb6detail2d119partition_type_baseINS1_19auto_partition_typeEE7executeINS1_9start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEEES8_EEvRT_RT0_RNS1_14execution_dataE.exit
  br label %if.end.i.i14

if.end.i.i14:                                     ; preds = %cleanup.i.i, %if.end.i.i14.preheader
  %n.addr.020.i.i = phi %"struct.tbb::detail::d1::node"* [ %226, %cleanup.i.i ], [ %219, %if.end.i.i14.preheader ]
  %my_parent.i.i103 = bitcast %"struct.tbb::detail::d1::node"* %n.addr.020.i.i to %"struct.tbb::detail::d1::node"**
  %226 = load %"struct.tbb::detail::d1::node"*, %"struct.tbb::detail::d1::node"** %my_parent.i.i103, align 8, !tbaa !43
  %tobool.not.i.i13 = icmp eq %"struct.tbb::detail::d1::node"* %226, null
  br i1 %tobool.not.i.i13, label %for.end.i.i, label %cleanup.i.i

cleanup.i.i:                                      ; preds = %if.end.i.i14
  %m_allocator.i.i = getelementptr inbounds %"struct.tbb::detail::d1::node", %"struct.tbb::detail::d1::node"* %n.addr.020.i.i, i64 1
  %227 = bitcast %"struct.tbb::detail::d1::node"* %m_allocator.i.i to %"class.tbb::detail::d1::small_object_pool"**
  %228 = load %"class.tbb::detail::d1::small_object_pool"*, %"class.tbb::detail::d1::small_object_pool"** %227, align 8, !tbaa !42
  %229 = bitcast %"struct.tbb::detail::d1::node"* %n.addr.020.i.i to i8*
  call void @_ZN3tbb6detail2r110deallocateERNS0_2d117small_object_poolEPvmRKNS2_14execution_dataE(%"class.tbb::detail::d1::small_object_pool"* noundef nonnull align 1 dereferenceable(1) %228, i8* noundef %229, i64 noundef 32, %"struct.tbb::detail::d1::execution_data"* noundef nonnull align 8 dereferenceable(12) %ed)
  %_M_i.i.i.i15 = getelementptr inbounds %"struct.tbb::detail::d1::node", %"struct.tbb::detail::d1::node"* %226, i64 0, i32 1, i32 0, i32 0
  %230 = atomicrmw sub i32* %_M_i.i.i.i15, i32 1 seq_cst, align 4
  %231 = add i32 %230, -1
  %cmp.i.i16 = icmp sgt i32 %231, 0
  br i1 %cmp.i.i16, label %_ZN3tbb6detail2d19start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEE8finalizeERKNS1_14execution_dataE.exit, label %if.end.i.i14

for.end.i.i:                                      ; preds = %if.end.i.i14
  %_M_i.i.i.i.i.i = getelementptr inbounds %"struct.tbb::detail::d1::node", %"struct.tbb::detail::d1::node"* %n.addr.020.i.i, i64 1, i32 1
  %232 = bitcast %"struct.std::atomic.5"* %_M_i.i.i.i.i.i to i64*
  %233 = atomicrmw add i64* %232, i64 -1 seq_cst, align 8
  %tobool.not.i.i.i.i = icmp eq i64 %233, 1
  br i1 %tobool.not.i.i.i.i, label %if.then.i.i.i.i, label %_ZN3tbb6detail2d19start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEE8finalizeERKNS1_14execution_dataE.exit

if.then.i.i.i.i:                                  ; preds = %for.end.i.i
  %m_wait.i.i = getelementptr inbounds %"struct.tbb::detail::d1::node", %"struct.tbb::detail::d1::node"* %n.addr.020.i.i, i64 1
  %234 = ptrtoint %"struct.tbb::detail::d1::node"* %m_wait.i.i to i64
  call void @_ZN3tbb6detail2r114notify_waitersEm(i64 noundef %234)
  br label %_ZN3tbb6detail2d19start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEE8finalizeERKNS1_14execution_dataE.exit

_ZN3tbb6detail2d19start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEE8finalizeERKNS1_14execution_dataE.exit: ; preds = %if.then.i.i.i.i, %for.end.i.i, %cleanup.i.i, %_ZN3tbb6detail2d119partition_type_baseINS1_19auto_partition_typeEE7executeINS1_9start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEEES8_EEvRT_RT0_RNS1_14execution_dataE.exit
  %235 = bitcast %"struct.tbb::detail::d1::start_for"* %this to i8*
  call void @_ZN3tbb6detail2r110deallocateERNS0_2d117small_object_poolEPvmRKNS2_14execution_dataE(%"class.tbb::detail::d1::small_object_pool"* noundef nonnull align 1 dereferenceable(1) %221, i8* noundef nonnull %235, i64 noundef 192, %"struct.tbb::detail::d1::execution_data"* noundef nonnull align 8 dereferenceable(12) %ed)
  ret %"class.tbb::detail::d1::task"* null
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local noundef %"class.tbb::detail::d1::task"* @_ZN3tbb6detail2d19start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEE6cancelERNS1_14execution_dataE(%"struct.tbb::detail::d1::start_for"* noundef nonnull align 64 dereferenceable(136) %this, %"struct.tbb::detail::d1::execution_data"* noundef nonnull align 8 dereferenceable(12) %ed) comdat align 2 {
entry:
  %my_parent.i = getelementptr inbounds %"struct.tbb::detail::d1::start_for", %"struct.tbb::detail::d1::start_for"* %this, i64 0, i32 3, !intel-tbaa !13
  %0 = load %"struct.tbb::detail::d1::node"*, %"struct.tbb::detail::d1::node"** %my_parent.i, align 8, !tbaa !13
  %1 = getelementptr inbounds %"struct.tbb::detail::d1::start_for", %"struct.tbb::detail::d1::start_for"* %this, i64 0, i32 5, i32 0
  %2 = load %"class.tbb::detail::d1::small_object_pool"*, %"class.tbb::detail::d1::small_object_pool"** %1, align 64, !tbaa !42
  %3 = bitcast %"struct.tbb::detail::d1::start_for"* %this to void (%"struct.tbb::detail::d1::start_for"*)***
  %vtable.i = load void (%"struct.tbb::detail::d1::start_for"*)**, void (%"struct.tbb::detail::d1::start_for"*)*** %3, align 64, !tbaa !37
  %4 = load void (%"struct.tbb::detail::d1::start_for"*)*, void (%"struct.tbb::detail::d1::start_for"*)** %vtable.i, align 8
  tail call void %4(%"struct.tbb::detail::d1::start_for"* noundef nonnull align 64 dereferenceable(136) %this)
  %_M_i.i18.i.i = getelementptr inbounds %"struct.tbb::detail::d1::node", %"struct.tbb::detail::d1::node"* %0, i64 0, i32 1, i32 0, i32 0
  %5 = atomicrmw sub i32* %_M_i.i18.i.i, i32 1 seq_cst, align 4
  %6 = add i32 %5, -1
  %cmp19.i.i = icmp sgt i32 %6, 0
  br i1 %cmp19.i.i, label %_ZN3tbb6detail2d19start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEE8finalizeERKNS1_14execution_dataE.exit, label %if.end.i.i.preheader

if.end.i.i.preheader:                             ; preds = %entry
  br label %if.end.i.i

if.end.i.i:                                       ; preds = %cleanup.i.i, %if.end.i.i.preheader
  %n.addr.020.i.i = phi %"struct.tbb::detail::d1::node"* [ %7, %cleanup.i.i ], [ %0, %if.end.i.i.preheader ]
  %my_parent.i.i10 = bitcast %"struct.tbb::detail::d1::node"* %n.addr.020.i.i to %"struct.tbb::detail::d1::node"**
  %7 = load %"struct.tbb::detail::d1::node"*, %"struct.tbb::detail::d1::node"** %my_parent.i.i10, align 8, !tbaa !43
  %tobool.not.i.i = icmp eq %"struct.tbb::detail::d1::node"* %7, null
  br i1 %tobool.not.i.i, label %for.end.i.i, label %cleanup.i.i

cleanup.i.i:                                      ; preds = %if.end.i.i
  %m_allocator.i.i = getelementptr inbounds %"struct.tbb::detail::d1::node", %"struct.tbb::detail::d1::node"* %n.addr.020.i.i, i64 1
  %8 = bitcast %"struct.tbb::detail::d1::node"* %m_allocator.i.i to %"class.tbb::detail::d1::small_object_pool"**
  %9 = load %"class.tbb::detail::d1::small_object_pool"*, %"class.tbb::detail::d1::small_object_pool"** %8, align 8, !tbaa !42
  %10 = bitcast %"struct.tbb::detail::d1::node"* %n.addr.020.i.i to i8*
  tail call void @_ZN3tbb6detail2r110deallocateERNS0_2d117small_object_poolEPvmRKNS2_14execution_dataE(%"class.tbb::detail::d1::small_object_pool"* noundef nonnull align 1 dereferenceable(1) %9, i8* noundef %10, i64 noundef 32, %"struct.tbb::detail::d1::execution_data"* noundef nonnull align 8 dereferenceable(12) %ed)
  %_M_i.i.i.i = getelementptr inbounds %"struct.tbb::detail::d1::node", %"struct.tbb::detail::d1::node"* %7, i64 0, i32 1, i32 0, i32 0
  %11 = atomicrmw sub i32* %_M_i.i.i.i, i32 1 seq_cst, align 4
  %12 = add i32 %11, -1
  %cmp.i.i = icmp sgt i32 %12, 0
  br i1 %cmp.i.i, label %_ZN3tbb6detail2d19start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEE8finalizeERKNS1_14execution_dataE.exit, label %if.end.i.i

for.end.i.i:                                      ; preds = %if.end.i.i
  %_M_i.i.i.i.i.i = getelementptr inbounds %"struct.tbb::detail::d1::node", %"struct.tbb::detail::d1::node"* %n.addr.020.i.i, i64 1, i32 1
  %13 = bitcast %"struct.std::atomic.5"* %_M_i.i.i.i.i.i to i64*
  %14 = atomicrmw add i64* %13, i64 -1 seq_cst, align 8
  %tobool.not.i.i.i.i = icmp eq i64 %14, 1
  br i1 %tobool.not.i.i.i.i, label %if.then.i.i.i.i, label %_ZN3tbb6detail2d19start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEE8finalizeERKNS1_14execution_dataE.exit

if.then.i.i.i.i:                                  ; preds = %for.end.i.i
  %m_wait.i.i = getelementptr inbounds %"struct.tbb::detail::d1::node", %"struct.tbb::detail::d1::node"* %n.addr.020.i.i, i64 1
  %15 = ptrtoint %"struct.tbb::detail::d1::node"* %m_wait.i.i to i64
  tail call void @_ZN3tbb6detail2r114notify_waitersEm(i64 noundef %15)
  br label %_ZN3tbb6detail2d19start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEE8finalizeERKNS1_14execution_dataE.exit

_ZN3tbb6detail2d19start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEE8finalizeERKNS1_14execution_dataE.exit: ; preds = %if.then.i.i.i.i, %for.end.i.i, %cleanup.i.i, %entry
  %16 = bitcast %"struct.tbb::detail::d1::start_for"* %this to i8*
  tail call void @_ZN3tbb6detail2r110deallocateERNS0_2d117small_object_poolEPvmRKNS2_14execution_dataE(%"class.tbb::detail::d1::small_object_pool"* noundef nonnull align 1 dereferenceable(1) %2, i8* noundef nonnull %16, i64 noundef 192, %"struct.tbb::detail::d1::execution_data"* noundef nonnull align 8 dereferenceable(12) %ed)
  ret %"class.tbb::detail::d1::task"* null
}

declare dso_local noundef i32 @_ZN3tbb6detail2r115max_concurrencyEPKNS0_2d115task_arena_baseE(%"class.tbb::detail::d1::task_arena_base"* noundef)

; Function Attrs: nobuiltin nounwind
declare dso_local void @_ZdlPv(i8* noundef)

declare dso_local noundef zeroext i16 @_ZN3tbb6detail2r114execution_slotEPKNS0_2d114execution_dataE(%"struct.tbb::detail::d1::execution_data"* noundef)

; Function Attrs: noinline noreturn nounwind
define linkonce_odr hidden void @__clang_call_terminate(i8* %0) comdat {
  %2 = tail call i8* @__cxa_begin_catch(i8* %0)
  tail call void @_ZSt9terminatev()
  unreachable
}

; Function Attrs: nofree
declare dso_local i8* @__cxa_begin_catch(i8*)

; Function Attrs: nofree noreturn nounwind
declare dso_local void @_ZSt9terminatev()

declare dso_local noundef i8* @_ZN3tbb6detail2r18allocateERPNS0_2d117small_object_poolEmRKNS2_14execution_dataE(%"class.tbb::detail::d1::small_object_pool"** noundef nonnull align 8 dereferenceable(8), i64 noundef, %"struct.tbb::detail::d1::execution_data"* noundef nonnull align 8 dereferenceable(12))

declare dso_local void @_ZN3tbb6detail2r15spawnERNS0_2d14taskERNS2_18task_group_contextE(%"class.tbb::detail::d1::task"* noundef nonnull align 64 dereferenceable(64), %"class.tbb::detail::d1::task_group_context"* noundef nonnull align 8 dereferenceable(128))

; Function Attrs: argmemonly mustprogress nofree norecurse nosync nounwind willreturn uwtable
define internal fastcc void @_ZL14delta_steppingPiS_S_i(i32* nocapture noundef readonly %p_array_a, i32* nocapture noundef readonly %p_array_b, i32* nocapture noundef writeonly %p_array_sum, i32 noundef %index) {
entry:
  tail call fastcc void @_ZL5relaxPiS_S_i(i32* noundef %p_array_a, i32* noundef %p_array_b, i32* noundef %p_array_sum, i32 noundef %index)
  tail call fastcc void @_ZL5relaxPiS_S_i(i32* noundef %p_array_a, i32* noundef %p_array_b, i32* noundef %p_array_sum, i32 noundef %index)
  tail call fastcc void @_ZL5relaxPiS_S_i(i32* noundef %p_array_a, i32* noundef %p_array_b, i32* noundef %p_array_sum, i32 noundef %index)
  ret void
}

; Function Attrs: argmemonly mustprogress nofree norecurse nosync nounwind willreturn uwtable
define internal fastcc void @_ZL5relaxPiS_S_i(i32* nocapture noundef readonly %p_array_a, i32* nocapture noundef readonly %p_array_b, i32* nocapture noundef writeonly %p_array_sum, i32 noundef %index) {
entry:
  %cmp = icmp sgt i32 %index, 10
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %idxprom = zext i32 %index to i64
  %arrayidx = getelementptr inbounds i32, i32* %p_array_a, i64 %idxprom
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !52
  %arrayidx2 = getelementptr inbounds i32, i32* %p_array_b, i64 %idxprom
  %1 = load i32, i32* %arrayidx2, align 4, !tbaa !52
  %add = add nsw i32 %1, %0
  %arrayidx4 = getelementptr inbounds i32, i32* %p_array_sum, i64 %idxprom
  store i32 %add, i32* %arrayidx4, align 4, !tbaa !52
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  ret void
}

declare dso_local noundef zeroext i1 @_ZN3tbb6detail2r128is_group_execution_cancelledERNS0_2d118task_group_contextE(%"class.tbb::detail::d1::task_group_context"* noundef nonnull align 8 dereferenceable(128))

declare dso_local void @_ZN3tbb6detail2r110deallocateERNS0_2d117small_object_poolEPvmRKNS2_14execution_dataE(%"class.tbb::detail::d1::small_object_pool"* noundef nonnull align 1 dereferenceable(1), i8* noundef, i64 noundef, %"struct.tbb::detail::d1::execution_data"* noundef nonnull align 8 dereferenceable(12))

declare dso_local void @_ZN3tbb6detail2r114notify_waitersEm(i64 noundef)

declare dso_local void @_ZN3tbb6detail2r116execute_and_waitERNS0_2d14taskERNS2_18task_group_contextERNS2_12wait_contextES6_(%"class.tbb::detail::d1::task"* noundef nonnull align 64 dereferenceable(64), %"class.tbb::detail::d1::task_group_context"* noundef nonnull align 8 dereferenceable(128), %"class.tbb::detail::d1::wait_context"* noundef nonnull align 8 dereferenceable(16), %"class.tbb::detail::d1::task_group_context"* noundef nonnull align 8 dereferenceable(128))

declare dso_local void @_ZN3tbb6detail2r17destroyERNS0_2d118task_group_contextE(%"class.tbb::detail::d1::task_group_context"* noundef nonnull align 8 dereferenceable(128))

declare void @_Unwind_Resume(i8*)

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!3 = !{!4, !8, i64 10}
!4 = !{!"struct@_ZTSN3tbb6detail2d114execution_dataE", !5, i64 0, !8, i64 8, !8, i64 10}
!5 = !{!"pointer@_ZTSPN3tbb6detail2d118task_group_contextE", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C++ TBAA"}
!8 = !{!"short", !6, i64 0}
!9 = !{!10, !11, i64 0}
!10 = !{!"struct@_ZTSN3tbb6detail2d113adaptive_modeINS1_19auto_partition_typeEEE", !11, i64 0}
!11 = !{!"long", !6, i64 0}
!12 = !{!4, !8, i64 8}
!13 = !{!14, !19, i64 104}
!14 = !{!"struct@_ZTSN3tbb6detail2d19start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEEE", !15, i64 64, !17, i64 80, !19, i64 104, !20, i64 112, !21, i64 128}
!15 = !{!"struct@_ZTSN3tbb6detail2d113blocked_rangeIiEE", !16, i64 0, !16, i64 4, !11, i64 8}
!16 = !{!"int", !6, i64 0}
!17 = !{!"struct@_ZTS11ArraySummer", !18, i64 0, !18, i64 8, !18, i64 16}
!18 = !{!"pointer@_ZTSPi", !6, i64 0}
!19 = !{!"pointer@_ZTSPN3tbb6detail2d14nodeE", !6, i64 0}
!20 = !{!"struct@_ZTSN3tbb6detail2d119auto_partition_typeE"}
!21 = !{!"struct@_ZTSN3tbb6detail2d122small_object_allocatorE", !22, i64 0}
!22 = !{!"pointer@_ZTSPN3tbb6detail2d117small_object_poolE", !6, i64 0}
!23 = !{!24, !27, i64 24}
!24 = !{!"struct@_ZTSN3tbb6detail2d19tree_nodeE", !21, i64 16, !25, i64 24}
!25 = !{!"struct@_ZTSSt6atomicIbE", !26, i64 0}
!26 = !{!"struct@_ZTSSt13__atomic_baseIbE", !27, i64 0}
!27 = !{!"bool", !6, i64 0}
!28 = !{!29, !6, i64 12}
!29 = !{!"struct@_ZTSN3tbb6detail2d122dynamic_grainsize_modeINS1_13adaptive_modeINS1_19auto_partition_typeEEEEE", !30, i64 8, !6, i64 12}
!30 = !{!"_ZTSN3tbb6detail2d122dynamic_grainsize_modeINS1_13adaptive_modeINS1_19auto_partition_typeEEEEUt_E", !6, i64 0}
!31 = !{!14, !15, i64 64}
!32 = !{!14, !11, i64 72}
!33 = !{!14, !16, i64 64}
!34 = !{!14, !16, i64 68}
!35 = !{!21, !22, i64 0}
!36 = !{!11, !11, i64 0}
!37 = !{!38, !38, i64 0}
!38 = !{!"vtable pointer", !7, i64 0}
!39 = !{i64 0, i64 8, !40, i64 8, i64 8, !40, i64 16, i64 8, !40}
!40 = !{!18, !18, i64 0}
!41 = !{!29, !30, i64 8}
!42 = !{!22, !22, i64 0}
!43 = !{!44, !19, i64 0}
!44 = !{!"struct@_ZTSN3tbb6detail2d14nodeE", !19, i64 0, !45, i64 8}
!45 = !{!"struct@_ZTSSt6atomicIiE"}
!46 = !{!47, !16, i64 0}
!47 = !{!"struct@_ZTSSt13__atomic_baseIiE", !16, i64 0}
!48 = !{!4, !5, i64 0}
!49 = !{!14, !18, i64 80}
!50 = !{!14, !18, i64 88}
!51 = !{!14, !18, i64 96}
!52 = !{!16, !16, i64 0}
!53 = distinct !{!53, !54}
!54 = !{!"llvm.loop.mustprogress"}
!55 = !{!56, !6, i64 0}
!56 = !{!"struct@_ZTSN3tbb6detail2d112range_vectorINS1_13blocked_rangeIiEELh8EEE", !6, i64 0, !6, i64 1, !6, i64 2, !57, i64 3, !58, i64 16}
!57 = !{!"array@_ZTSA8_h", !6, i64 0}
!58 = !{!"struct@_ZTSN3tbb6detail2d013aligned_spaceINS0_2d113blocked_rangeIiEELm8EEE", !59, i64 0}
!59 = !{!"array@_ZTSA128_h", !6, i64 0}
!60 = !{!56, !6, i64 1}
!61 = !{!56, !6, i64 2}
!62 = !{!56, !6, i64 3}
!63 = !{i64 0, i64 4, !52, i64 4, i64 4, !52, i64 8, i64 8, !36}
!64 = distinct !{!64, !54}
!65 = !{!15, !11, i64 8}
!66 = !{!15, !16, i64 0}
!67 = !{!15, !16, i64 4}
!68 = !{!15, !15, i64 0}
!69 = !{!70, !76, i64 15}
!70 = !{!"struct@_ZTSN3tbb6detail2d118task_group_contextE", !11, i64 0, !71, i64 8, !72, i64 12, !73, i64 13, !74, i64 14, !75, i64 15, !6, i64 16, !77, i64 24, !78, i64 32, !80, i64 48, !83, i64 56, !84, i64 64, !85, i64 72}
!71 = !{!"struct@_ZTSSt6atomicIjE"}
!72 = !{!"_ZTSN3tbb6detail2d118task_group_context26task_group_context_versionE", !6, i64 0}
!73 = !{!"struct@_ZTSN3tbb6detail2d118task_group_context14context_traitsE", !6, i64 0, !6, i64 0, !6, i64 0, !6, i64 0, !6, i64 0, !6, i64 0, !6, i64 0, !6, i64 0}
!74 = !{!"struct@_ZTSSt6atomicIhE"}
!75 = !{!"struct@_ZTSSt6atomicIN3tbb6detail2d118task_group_context5stateEE", !76, i64 0}
!76 = !{!"_ZTSN3tbb6detail2d118task_group_context5stateE", !6, i64 0}
!77 = !{!"pointer@_ZTSPN3tbb6detail2r112context_listE", !6, i64 0}
!78 = !{!"struct@_ZTSN3tbb6detail2d119intrusive_list_nodeE", !79, i64 0, !79, i64 8}
!79 = !{!"pointer@_ZTSPN3tbb6detail2d119intrusive_list_nodeE", !6, i64 0}
!80 = !{!"struct@_ZTSSt6atomicIPN3tbb6detail2r117tbb_exception_ptrEE", !81, i64 0}
!81 = !{!"struct@_ZTSSt13__atomic_baseIPN3tbb6detail2r117tbb_exception_ptrEE", !82, i64 0}
!82 = !{!"pointer@_ZTSPN3tbb6detail2r117tbb_exception_ptrE", !6, i64 0}
!83 = !{!"pointer@_ZTSPv", !6, i64 0}
!84 = !{!"_ZTSN3tbb6detail2d021string_resource_indexE", !6, i64 0}
!85 = !{!"array@_ZTSA56_c", !6, i64 0}
!86 = distinct !{!86, !54}
; end INTEL_FEATURE_SW_ADVANCED
