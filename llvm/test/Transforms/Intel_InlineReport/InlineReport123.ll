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

%"class.tbb::detail::d1::small_object_allocator" = type { ptr }
%"class.tbb::detail::d1::range_vector" = type { i8, i8, i8, [8 x i8], [5 x i8], %"class.tbb::detail::d0::aligned_space" }
%"class.tbb::detail::d0::aligned_space" = type { [128 x i8] }
%"struct.tbb::detail::d1::execution_data" = type <{ ptr, i16, i16, [4 x i8] }>
%"struct.tbb::detail::d1::start_for" = type { %"class.tbb::detail::d1::task", %"class.tbb::detail::d1::blocked_range", %class.ArraySummer, ptr, %"class.tbb::detail::d1::auto_partition_type", %"class.tbb::detail::d1::small_object_allocator", [56 x i8] }
%"class.tbb::detail::d1::task" = type { ptr, %"class.tbb::detail::d1::task_traits", [6 x i64] }
%"class.tbb::detail::d1::task_traits" = type { i64 }
%"class.tbb::detail::d1::blocked_range" = type { i32, i32, i64 }
%class.ArraySummer = type { ptr, ptr, ptr }
%"class.tbb::detail::d1::auto_partition_type" = type { %"struct.tbb::detail::d1::dynamic_grainsize_mode.base", [3 x i8] }
%"struct.tbb::detail::d1::dynamic_grainsize_mode.base" = type <{ %"struct.tbb::detail::d1::adaptive_mode", i32, i8 }>
%"struct.tbb::detail::d1::adaptive_mode" = type { i64 }
%"struct.tbb::detail::d1::node" = type <{ ptr, %"struct.std::atomic.5", [4 x i8] }>
%"struct.std::atomic.5" = type { %"struct.std::__atomic_base.6" }
%"struct.std::__atomic_base.6" = type { i32 }
%"struct.tbb::detail::d1::tree_node" = type <{ %"struct.tbb::detail::d1::node.base", [4 x i8], %"class.tbb::detail::d1::small_object_allocator", %"struct.std::atomic.12", [7 x i8] }>
%"struct.tbb::detail::d1::node.base" = type <{ ptr, %"struct.std::atomic.5" }>
%"struct.std::atomic.12" = type { %"struct.std::__atomic_base.13" }
%"struct.std::__atomic_base.13" = type { i8 }
%"class.tbb::detail::d1::task_group_context" = type { i64, %"struct.std::atomic", i8, %"struct.tbb::detail::d1::task_group_context::context_traits", %"struct.std::atomic.0", %"struct.std::atomic.2", %union.anon, ptr, %"struct.tbb::detail::d1::intrusive_list_node", %"struct.std::atomic.3", ptr, i64, [56 x i8] }
%"struct.std::atomic" = type { %"struct.std::__atomic_base" }
%"struct.std::__atomic_base" = type { i32 }
%"struct.tbb::detail::d1::task_group_context::context_traits" = type { i8 }
%"struct.std::atomic.0" = type { %"struct.std::__atomic_base.1" }
%"struct.std::__atomic_base.1" = type { i8 }
%"struct.std::atomic.2" = type { i8 }
%union.anon = type { ptr }
%"struct.tbb::detail::d1::intrusive_list_node" = type { ptr, ptr }
%"struct.std::atomic.3" = type { %"struct.std::__atomic_base.4" }
%"struct.std::__atomic_base.4" = type { ptr }

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

@_ZTVN3tbb6detail2d19start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEEE = linkonce_odr dso_local unnamed_addr constant { [6 x ptr] } { [6 x ptr] [ptr null, ptr @_ZTIN3tbb6detail2d19start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEEE, ptr @_ZN3tbb6detail2d14taskD2Ev, ptr @_ZN3tbb6detail2d19start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEED0Ev, ptr @_ZN3tbb6detail2d19start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEE7executeERNS1_14execution_dataE, ptr @_ZN3tbb6detail2d19start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEE6cancelERNS1_14execution_dataE] }, comdat, align 8
@_ZTVN10__cxxabiv120__si_class_type_infoE = external dso_local global ptr
@_ZTSN3tbb6detail2d19start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEEE = linkonce_odr dso_local constant [89 x i8] c"N3tbb6detail2d19start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEEE\00", comdat, align 1
@_ZTVN10__cxxabiv121__vmi_class_type_infoE = external dso_local global ptr
@_ZTSN3tbb6detail2d14taskE = linkonce_odr dso_local constant [22 x i8] c"N3tbb6detail2d14taskE\00", comdat, align 1
@_ZTVN10__cxxabiv117__class_type_infoE = external dso_local global ptr
@_ZTSN3tbb6detail2d111task_traitsE = linkonce_odr dso_local constant [30 x i8] c"N3tbb6detail2d111task_traitsE\00", comdat, align 1
@_ZTIN3tbb6detail2d111task_traitsE = linkonce_odr dso_local constant { ptr, ptr } { ptr getelementptr inbounds (ptr, ptr @_ZTVN10__cxxabiv117__class_type_infoE, i64 2), ptr @_ZTSN3tbb6detail2d111task_traitsE }, comdat, align 8
@_ZTIN3tbb6detail2d14taskE = linkonce_odr dso_local constant { ptr, ptr, i32, i32, ptr, i64 } { ptr getelementptr inbounds (ptr, ptr @_ZTVN10__cxxabiv121__vmi_class_type_infoE, i64 2), ptr @_ZTSN3tbb6detail2d14taskE, i32 0, i32 1, ptr @_ZTIN3tbb6detail2d111task_traitsE, i64 2050 }, comdat, align 8
@_ZTIN3tbb6detail2d19start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEEE = linkonce_odr dso_local constant { ptr, ptr, ptr } { ptr getelementptr inbounds (ptr, ptr @_ZTVN10__cxxabiv120__si_class_type_infoE, i64 2), ptr @_ZTSN3tbb6detail2d19start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEEE, ptr @_ZTIN3tbb6detail2d14taskE }, comdat, align 8

declare dso_local noundef nonnull ptr @_Znam(i64 noundef)

declare dso_local i32 @__gxx_personality_v0(...)

declare dso_local void @_ZN3tbb6detail2r110initializeERNS0_2d118task_group_contextE(ptr noundef nonnull align 8 dereferenceable(128))

declare dso_local noundef ptr @_ZN3tbb6detail2r18allocateERPNS0_2d117small_object_poolEm(ptr noundef nonnull align 8 dereferenceable(8), i64 noundef)

define linkonce_odr dso_local void @_ZN3tbb6detail2d14taskD2Ev(ptr noundef nonnull align 64 dereferenceable(64) %this) {
entry:
  ret void
}

define linkonce_odr dso_local void @_ZN3tbb6detail2d19start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEED0Ev(ptr noundef nonnull align 64 dereferenceable(136) %this) comdat align 2 {
entry:
  tail call void @_ZdlPv(ptr noundef nonnull %this)
  ret void
}

define linkonce_odr dso_local noundef ptr @_ZN3tbb6detail2d19start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEE7executeERNS1_14execution_dataE(ptr noundef nonnull align 64 dereferenceable(136) %this, ptr noundef nonnull align 8 dereferenceable(12) %ed) comdat align 2 personality ptr @__gxx_personality_v0 {
entry:
  %alloc.i.i.i.i = alloca %"class.tbb::detail::d1::small_object_allocator", align 8
  %range_pool.i.i = alloca %"class.tbb::detail::d1::range_vector", align 8
  %alloc.i.i.i = alloca %"class.tbb::detail::d1::small_object_allocator", align 8
  %affinity_slot.i.i = getelementptr inbounds %"struct.tbb::detail::d1::execution_data", ptr %ed, i64 0, i32 2, !intel-tbaa !3
  %i = load i16, ptr %affinity_slot.i.i, align 2, !tbaa !3
  %cmp.i = icmp eq i16 %i, -1
  br i1 %cmp.i, label %if.end, label %_ZN3tbb6detail2d116is_same_affinityERKNS1_14execution_dataE.exit

_ZN3tbb6detail2d116is_same_affinityERKNS1_14execution_dataE.exit: ; preds = %entry
  %call.i.i = tail call noundef zeroext i16 @_ZN3tbb6detail2r114execution_slotEPKNS0_2d114execution_dataE(ptr noundef nonnull %ed)
  %cmp5.i = icmp eq i16 %i, %call.i.i
  br i1 %cmp5.i, label %if.end, label %if.then

if.then:                                          ; preds = %_ZN3tbb6detail2d116is_same_affinityERKNS1_14execution_dataE.exit
  %call.i = tail call noundef zeroext i16 @_ZN3tbb6detail2r114execution_slotEPKNS0_2d114execution_dataE(ptr noundef nonnull %ed)
  br label %if.end

if.end:                                           ; preds = %if.then, %_ZN3tbb6detail2d116is_same_affinityERKNS1_14execution_dataE.exit, %entry
  %my_divisor.i = getelementptr inbounds %"struct.tbb::detail::d1::start_for", ptr %this, i64 0, i32 4, i32 0, i32 0, i32 0
  %i1 = load i64, ptr %my_divisor.i, align 16, !tbaa !9
  %tobool.not.i = icmp eq i64 %i1, 0
  br i1 %tobool.not.i, label %if.then.i, label %_ZN3tbb6detail2d122dynamic_grainsize_modeINS1_13adaptive_modeINS1_19auto_partition_typeEEEE18check_being_stolenINS1_9start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEEEEEbRT_RKNS1_14execution_dataE.exit

if.then.i:                                        ; preds = %if.end
  %sunkaddr = getelementptr inbounds i8, ptr %this, i64 112
  store i64 1, ptr %sunkaddr, align 16, !tbaa !9
  %call.i.i.i = tail call noundef zeroext i16 @_ZN3tbb6detail2r114execution_slotEPKNS0_2d114execution_dataE(ptr noundef nonnull %ed)
  %original_slot.i.i.i = getelementptr inbounds %"struct.tbb::detail::d1::execution_data", ptr %ed, i64 0, i32 1, !intel-tbaa !12
  %i4 = load i16, ptr %original_slot.i.i.i, align 8, !tbaa !12
  %cmp.i.not.i = icmp eq i16 %call.i.i.i, %i4
  br i1 %cmp.i.not.i, label %_ZN3tbb6detail2d122dynamic_grainsize_modeINS1_13adaptive_modeINS1_19auto_partition_typeEEEE18check_being_stolenINS1_9start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEEEEEbRT_RKNS1_14execution_dataE.exit, label %land.lhs.true.i

land.lhs.true.i:                                  ; preds = %if.then.i
  %my_parent.i = getelementptr inbounds %"struct.tbb::detail::d1::start_for", ptr %this, i64 0, i32 3, !intel-tbaa !13
  %i5 = load ptr, ptr %my_parent.i, align 8, !tbaa !13
  %_M_i.i.i.i = getelementptr inbounds %"struct.tbb::detail::d1::node", ptr %i5, i64 0, i32 1, i32 0, i32 0
  %i6 = load atomic i32, ptr %_M_i.i.i.i seq_cst, align 4
  %cmp.i10 = icmp sgt i32 %i6, 1
  br i1 %cmp.i10, label %if.then6.i, label %_ZN3tbb6detail2d122dynamic_grainsize_modeINS1_13adaptive_modeINS1_19auto_partition_typeEEEE18check_being_stolenINS1_9start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEEEEEbRT_RKNS1_14execution_dataE.exit

if.then6.i:                                       ; preds = %land.lhs.true.i
  %sunkaddr54 = getelementptr inbounds i8, ptr %this, i64 104
  %i9 = load ptr, ptr %sunkaddr54, align 8, !tbaa !13
  %_M_i.i.i.i.i = getelementptr inbounds %"struct.tbb::detail::d1::tree_node", ptr %i9, i64 0, i32 3, i32 0, i32 0, !intel-tbaa !23
  store atomic i8 1, ptr %_M_i.i.i.i.i monotonic, align 1
  %i10 = getelementptr inbounds %"struct.tbb::detail::d1::start_for", ptr %this, i64 0, i32 4, i32 0, i32 2
  %i11 = load i8, ptr %i10, align 4, !tbaa !28
  %tobool7.not.i = icmp eq i8 %i11, 0
  %.op.i = add i8 %i11, 1
  %add.i = select i1 %tobool7.not.i, i8 2, i8 %.op.i
  store i8 %add.i, ptr %i10, align 4, !tbaa !28
  br label %_ZN3tbb6detail2d122dynamic_grainsize_modeINS1_13adaptive_modeINS1_19auto_partition_typeEEEE18check_being_stolenINS1_9start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEEEEEbRT_RKNS1_14execution_dataE.exit

_ZN3tbb6detail2d122dynamic_grainsize_modeINS1_13adaptive_modeINS1_19auto_partition_typeEEEE18check_being_stolenINS1_9start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEEEEEbRT_RKNS1_14execution_dataE.exit: ; preds = %if.then6.i, %land.lhs.true.i, %if.then.i, %if.end
  %my_range = getelementptr inbounds %"struct.tbb::detail::d1::start_for", ptr %this, i64 0, i32 1, !intel-tbaa !31
  %my_grainsize.i.i = getelementptr inbounds %"struct.tbb::detail::d1::start_for", ptr %this, i64 0, i32 1, i32 2
  %i12 = load i64, ptr %my_grainsize.i.i, align 8, !tbaa !32
  %i13 = load i32, ptr %my_range, align 64, !tbaa !33
  %my_begin.i.i.i = getelementptr inbounds %"struct.tbb::detail::d1::start_for", ptr %this, i64 0, i32 1, i32 1
  %i14 = load i32, ptr %my_begin.i.i.i, align 4, !tbaa !34
  %sub.i.i.i = sub nsw i32 %i13, %i14
  %conv.i.i.i = sext i32 %sub.i.i.i to i64
  %cmp.i.i = icmp ult i64 %i12, %conv.i.i.i
  br i1 %cmp.i.i, label %if.then.i11, label %if.end9.i

if.then.i11:                                      ; preds = %_ZN3tbb6detail2d122dynamic_grainsize_modeINS1_13adaptive_modeINS1_19auto_partition_typeEEEE18check_being_stolenINS1_9start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEEEEEbRT_RKNS1_14execution_dataE.exit
  %sunkaddr56 = getelementptr inbounds i8, ptr %this, i64 112
  %i17 = load i64, ptr %sunkaddr56, align 16, !tbaa !9
  %cmp.i15.i = icmp ugt i64 %i17, 1
  br i1 %cmp.i15.i, label %_ZN3tbb6detail2d119auto_partition_type12is_divisibleEv.exit.i, label %if.end.i.i

if.end.i.i:                                       ; preds = %if.then.i11
  %tobool.not.i.i = icmp eq i64 %i17, 0
  br i1 %tobool.not.i.i, label %if.end9.i, label %land.lhs.true.i.i

land.lhs.true.i.i:                                ; preds = %if.end.i.i
  %i18 = getelementptr inbounds %"struct.tbb::detail::d1::start_for", ptr %this, i64 0, i32 4, i32 0, i32 2
  %i19 = load i8, ptr %i18, align 4, !tbaa !28
  %tobool3.not.i.i = icmp eq i8 %i19, 0
  br i1 %tobool3.not.i.i, label %if.end9.i, label %if.then4.i.i

if.then4.i.i:                                     ; preds = %land.lhs.true.i.i
  %dec.i.i = add i8 %i19, -1
  %sunkaddr57 = getelementptr inbounds i8, ptr %this, i64 124
  store i8 %dec.i.i, ptr %sunkaddr57, align 4, !tbaa !28
  %sunkaddr58 = getelementptr inbounds i8, ptr %this, i64 112
  store i64 0, ptr %sunkaddr58, align 16, !tbaa !9
  br label %_ZN3tbb6detail2d119auto_partition_type12is_divisibleEv.exit.i

_ZN3tbb6detail2d119auto_partition_type12is_divisibleEv.exit.i: ; preds = %if.then4.i.i, %if.then.i11
  %my_body4.i.i.i.i.i = getelementptr inbounds %"struct.tbb::detail::d1::start_for", ptr %this, i64 0, i32 2
  br label %do.body.i

do.body.i:                                        ; preds = %if.then4.i29.i, %land.rhs.i, %_ZN3tbb6detail2d119auto_partition_type12is_divisibleEv.exit.i
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %alloc.i.i.i)
  store ptr null, ptr %alloc.i.i.i, align 8, !tbaa !35
  %call.i.i.i.i = call noundef ptr @_ZN3tbb6detail2r18allocateERPNS0_2d117small_object_poolEmRKNS2_14execution_dataE(ptr noundef nonnull align 8 dereferenceable(8) %alloc.i.i.i, i64 noundef 192, ptr noundef nonnull align 8 dereferenceable(12) %ed)
  %m_version_and_traits.i.i.i.i.i.i.i = getelementptr inbounds i8, ptr %call.i.i.i.i, i64 8
  %arrayinit.end.i.i.i.i.i.i = getelementptr inbounds i8, ptr %call.i.i.i.i, i64 64
  store <2 x i64> zeroinitializer, ptr %m_version_and_traits.i.i.i.i.i.i.i, align 8, !tbaa !36
  %i28 = getelementptr inbounds i8, ptr %call.i.i.i.i, i64 24
  store <2 x i64> zeroinitializer, ptr %i28, align 8, !tbaa !36
  %i30 = getelementptr inbounds i8, ptr %call.i.i.i.i, i64 40
  store <2 x i64> zeroinitializer, ptr %i30, align 8, !tbaa !36
  %i32 = getelementptr inbounds i8, ptr %call.i.i.i.i, i64 56
  store i64 0, ptr %i32, align 8, !tbaa !36
  store ptr getelementptr inbounds ({ [6 x ptr] }, ptr @_ZTVN3tbb6detail2d19start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEEE, i64 0, inrange i32 0, i64 2), ptr %call.i.i.i.i, align 64, !tbaa !37
  %sunkaddr61 = getelementptr inbounds i8, ptr %this, i64 64
  %i38 = load i32, ptr %sunkaddr61, align 64, !tbaa !33
  store i32 %i38, ptr %arrayinit.end.i.i.i.i.i.i, align 64, !tbaa !33
  %my_begin.i.i.i.i.i.i = getelementptr inbounds i8, ptr %call.i.i.i.i, i64 68
  %sunkaddr62 = getelementptr inbounds i8, ptr %this, i64 68
  %i42 = load i32, ptr %sunkaddr62, align 4, !tbaa !34
  %i43 = load i32, ptr %sunkaddr61, align 64, !tbaa !33
  %sub.i.i.i.i.i.i.i = sub nsw i32 %i43, %i42
  %div8.i.i.i.i.i.i.i = lshr i32 %sub.i.i.i.i.i.i.i, 1
  %add.i.i.i.i.i.i.i = add i32 %div8.i.i.i.i.i.i.i, %i42
  store i32 %add.i.i.i.i.i.i.i, ptr %sunkaddr61, align 64, !tbaa !33
  store i32 %add.i.i.i.i.i.i.i, ptr %my_begin.i.i.i.i.i.i, align 4, !tbaa !34
  %my_grainsize.i.i.i.i.i.i = getelementptr inbounds i8, ptr %call.i.i.i.i, i64 72
  %sunkaddr63 = getelementptr inbounds i8, ptr %this, i64 72
  %i47 = load i64, ptr %sunkaddr63, align 8, !tbaa !32
  store i64 %i47, ptr %my_grainsize.i.i.i.i.i.i, align 8, !tbaa !32
  %my_body.i.i.i.i.i = getelementptr inbounds i8, ptr %call.i.i.i.i, i64 80
  call void @llvm.memcpy.p0.p0.i64(ptr noundef nonnull align 16 dereferenceable(24) %my_body.i.i.i.i.i, ptr noundef nonnull align 16 dereferenceable(24) %my_body4.i.i.i.i.i, i64 24, i1 false), !tbaa.struct !39
  %my_divisor.i.i.i.i.i.i.i.i = getelementptr inbounds i8, ptr %call.i.i.i.i, i64 112
  %sunkaddr64 = getelementptr inbounds i8, ptr %this, i64 112
  %i51 = load i64, ptr %sunkaddr64, align 16, !tbaa !9
  %div2.i.i.i.i.i.i.i.i.i = lshr i64 %i51, 1
  store i64 %div2.i.i.i.i.i.i.i.i.i, ptr %sunkaddr64, align 16, !tbaa !9
  store i64 %div2.i.i.i.i.i.i.i.i.i, ptr %my_divisor.i.i.i.i.i.i.i.i, align 16, !tbaa !9
  %i52 = getelementptr inbounds i8, ptr %call.i.i.i.i, i64 120
  store i32 2, ptr %i52, align 8, !tbaa !41
  %i54 = getelementptr inbounds i8, ptr %call.i.i.i.i, i64 124
  %sunkaddr65 = getelementptr inbounds i8, ptr %this, i64 124
  %i56 = load i8, ptr %sunkaddr65, align 4, !tbaa !28
  store i8 %i56, ptr %i54, align 4, !tbaa !28
  %i57 = getelementptr inbounds i8, ptr %call.i.i.i.i, i64 128
  %i59 = load ptr, ptr %alloc.i.i.i, align 8, !tbaa !35
  store ptr %i59, ptr %i57, align 64, !tbaa !42
  %call.i13.i.i.i = call noundef ptr @_ZN3tbb6detail2r18allocateERPNS0_2d117small_object_poolEmRKNS2_14execution_dataE(ptr noundef nonnull align 8 dereferenceable(8) %alloc.i.i.i, i64 noundef 32, ptr noundef nonnull align 8 dereferenceable(12) %ed)
  %sunkaddr66 = getelementptr inbounds i8, ptr %this, i64 104
  %i62 = load ptr, ptr %sunkaddr66, align 8, !tbaa !13
  store ptr %i62, ptr %call.i13.i.i.i, align 8, !tbaa !43
  %i63 = getelementptr inbounds i8, ptr %call.i13.i.i.i, i64 8
  store i32 2, ptr %i63, align 8, !tbaa !46
  %i65 = getelementptr inbounds i8, ptr %call.i13.i.i.i, i64 16
  %i67 = load ptr, ptr %alloc.i.i.i, align 8, !tbaa !35
  store ptr %i67, ptr %i65, align 8, !tbaa !42
  %i68 = getelementptr inbounds i8, ptr %call.i13.i.i.i, i64 24
  store i8 0, ptr %i68, align 8, !tbaa !23
  %sunkaddr67 = getelementptr inbounds i8, ptr %this, i64 104
  store ptr %call.i13.i.i.i, ptr %sunkaddr67, align 8, !tbaa !13
  %my_parent6.i.i.i = getelementptr inbounds i8, ptr %call.i.i.i.i, i64 104
  store ptr %call.i13.i.i.i, ptr %my_parent6.i.i.i, align 8, !tbaa !13
  %i73 = load ptr, ptr %ed, align 8, !tbaa !48
  call void @_ZN3tbb6detail2r15spawnERNS0_2d14taskERNS2_18task_group_contextE(ptr noundef nonnull align 64 dereferenceable(64) %call.i.i.i.i, ptr noundef nonnull align 8 dereferenceable(128) %i73)
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %alloc.i.i.i)
  %i74 = load i64, ptr %sunkaddr63, align 8, !tbaa !32
  %sunkaddr68 = getelementptr inbounds i8, ptr %this, i64 64
  %i77 = load i32, ptr %sunkaddr68, align 64, !tbaa !33
  %i78 = load i32, ptr %sunkaddr62, align 4, !tbaa !34
  %sub.i.i19.i = sub nsw i32 %i77, %i78
  %conv.i.i20.i = sext i32 %sub.i.i19.i to i64
  %cmp.i21.i = icmp ult i64 %i74, %conv.i.i20.i
  br i1 %cmp.i21.i, label %land.rhs.i, label %if.end9.i

land.rhs.i:                                       ; preds = %do.body.i
  %sunkaddr69 = getelementptr inbounds i8, ptr %this, i64 112
  %i81 = load i64, ptr %sunkaddr69, align 16, !tbaa !9
  %cmp.i23.i = icmp ugt i64 %i81, 1
  br i1 %cmp.i23.i, label %do.body.i, label %if.end.i25.i

if.end.i25.i:                                     ; preds = %land.rhs.i
  %tobool.not.i24.i = icmp eq i64 %i81, 0
  br i1 %tobool.not.i24.i, label %if.end9.i, label %land.lhs.true.i27.i

land.lhs.true.i27.i:                              ; preds = %if.end.i25.i
  %sunkaddr70 = getelementptr inbounds i8, ptr %this, i64 124
  %i83 = load i8, ptr %sunkaddr70, align 4, !tbaa !28
  %tobool3.not.i26.i = icmp eq i8 %i83, 0
  br i1 %tobool3.not.i26.i, label %if.end9.i, label %if.then4.i29.i

if.then4.i29.i:                                   ; preds = %land.lhs.true.i27.i
  %dec.i28.i = add i8 %i83, -1
  %sunkaddr71 = getelementptr inbounds i8, ptr %this, i64 124
  store i8 %dec.i28.i, ptr %sunkaddr71, align 4, !tbaa !28
  %sunkaddr72 = getelementptr inbounds i8, ptr %this, i64 112
  store i64 0, ptr %sunkaddr72, align 16, !tbaa !9
  br label %do.body.i

if.end9.i:                                        ; preds = %land.lhs.true.i27.i, %if.end.i25.i, %do.body.i, %land.lhs.true.i.i, %if.end.i.i, %_ZN3tbb6detail2d122dynamic_grainsize_modeINS1_13adaptive_modeINS1_19auto_partition_typeEEEE18check_being_stolenINS1_9start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEEEEEbRT_RKNS1_14execution_dataE.exit
  %conv.i.i.i.pre-phi.i = phi i64 [ %conv.i.i.i, %if.end.i.i ], [ %conv.i.i.i, %land.lhs.true.i.i ], [ %conv.i.i.i, %_ZN3tbb6detail2d122dynamic_grainsize_modeINS1_13adaptive_modeINS1_19auto_partition_typeEEEE18check_being_stolenINS1_9start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEEEEEbRT_RKNS1_14execution_dataE.exit ], [ %conv.i.i20.i, %do.body.i ], [ %conv.i.i20.i, %if.end.i25.i ], [ %conv.i.i20.i, %land.lhs.true.i27.i ]
  %i87 = phi i32 [ %i14, %if.end.i.i ], [ %i14, %land.lhs.true.i.i ], [ %i14, %_ZN3tbb6detail2d122dynamic_grainsize_modeINS1_13adaptive_modeINS1_19auto_partition_typeEEEE18check_being_stolenINS1_9start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEEEEEbRT_RKNS1_14execution_dataE.exit ], [ %i78, %do.body.i ], [ %i78, %if.end.i25.i ], [ %i78, %land.lhs.true.i27.i ]
  %i88 = phi i32 [ %i13, %if.end.i.i ], [ %i13, %land.lhs.true.i.i ], [ %i13, %_ZN3tbb6detail2d122dynamic_grainsize_modeINS1_13adaptive_modeINS1_19auto_partition_typeEEEE18check_being_stolenINS1_9start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEEEEEbRT_RKNS1_14execution_dataE.exit ], [ %i77, %do.body.i ], [ %i77, %if.end.i25.i ], [ %i77, %land.lhs.true.i27.i ]
  %i89 = phi i64 [ %i12, %if.end.i.i ], [ %i12, %land.lhs.true.i.i ], [ %i12, %_ZN3tbb6detail2d122dynamic_grainsize_modeINS1_13adaptive_modeINS1_19auto_partition_typeEEEE18check_being_stolenINS1_9start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEEEEEbRT_RKNS1_14execution_dataE.exit ], [ %i74, %do.body.i ], [ %i74, %if.end.i25.i ], [ %i74, %land.lhs.true.i27.i ]
  %cmp.i.i.i = icmp ult i64 %i89, %conv.i.i.i.pre-phi.i
  br i1 %cmp.i.i.i, label %lor.lhs.false.i.i, label %if.then.i.i

lor.lhs.false.i.i:                                ; preds = %if.end9.i
  %i90 = getelementptr inbounds %"struct.tbb::detail::d1::start_for", ptr %this, i64 0, i32 4, i32 0, i32 2
  %i91 = load i8, ptr %i90, align 4, !tbaa !28
  %tobool.not.i32.i = icmp eq i8 %i91, 0
  br i1 %tobool.not.i32.i, label %if.then.i.i, label %if.else.i.i

if.then.i.i:                                      ; preds = %lor.lhs.false.i.i, %if.end9.i
  %cmp.not17.i.i.i.i = icmp eq i32 %i88, %i87
  br i1 %cmp.not17.i.i.i.i, label %_ZN3tbb6detail2d119partition_type_baseINS1_19auto_partition_typeEE7executeINS1_9start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEEES8_EEvRT_RT0_RNS1_14execution_dataE.exit, label %for.body.lr.ph.i.i.i.i

for.body.lr.ph.i.i.i.i:                           ; preds = %if.then.i.i
  %p_array_a.i.i.i.i = getelementptr inbounds %"struct.tbb::detail::d1::start_for", ptr %this, i64 0, i32 2, i32 0, !intel-tbaa !49
  %p_array_b.i.i.i.i = getelementptr inbounds %"struct.tbb::detail::d1::start_for", ptr %this, i64 0, i32 2, i32 1, !intel-tbaa !50
  %p_array_sum.i.i.i.i = getelementptr inbounds %"struct.tbb::detail::d1::start_for", ptr %this, i64 0, i32 2, i32 2, !intel-tbaa !51
  %i92 = sext i32 %i87 to i64
  %.pre.i.i.i.i = load ptr, ptr %p_array_a.i.i.i.i, align 16, !tbaa !49
  %.pre19.i.i.i.i = load ptr, ptr %p_array_b.i.i.i.i, align 8, !tbaa !50
  %.pre20.i.i.i.i = load ptr, ptr %p_array_sum.i.i.i.i, align 32, !tbaa !51
  br label %for.body.i.i.i.i

for.body.i.i.i.i:                                 ; preds = %for.body.i.i.i.i, %for.body.lr.ph.i.i.i.i
  %i93 = phi ptr [ %.pre20.i.i.i.i, %for.body.lr.ph.i.i.i.i ], [ %i106, %for.body.i.i.i.i ]
  %i94 = phi ptr [ %.pre19.i.i.i.i, %for.body.lr.ph.i.i.i.i ], [ %i102, %for.body.i.i.i.i ]
  %i95 = phi ptr [ %.pre.i.i.i.i, %for.body.lr.ph.i.i.i.i ], [ %i98, %for.body.i.i.i.i ]
  %indvars.iv.i.i.i.i = phi i64 [ %i92, %for.body.lr.ph.i.i.i.i ], [ %indvars.iv.next.i.i.i.i, %for.body.i.i.i.i ]
  %lsr48 = trunc i64 %indvars.iv.i.i.i.i to i32
  call fastcc void @_ZL14delta_steppingPiS_S_i(ptr noundef %i95, ptr noundef %i94, ptr noundef %i93, i32 noundef %lsr48)
  %sunkaddr73 = getelementptr inbounds i8, ptr %this, i64 80
  %i98 = load ptr, ptr %sunkaddr73, align 16, !tbaa !49
  %scevgep47 = getelementptr i32, ptr %i98, i64 %indvars.iv.i.i.i.i
  %i99 = load i32, ptr %scevgep47, align 4, !tbaa !52
  %sunkaddr74 = getelementptr inbounds i8, ptr %this, i64 88
  %i102 = load ptr, ptr %sunkaddr74, align 8, !tbaa !50
  %scevgep46 = getelementptr i32, ptr %i102, i64 %indvars.iv.i.i.i.i
  %i103 = load i32, ptr %scevgep46, align 4, !tbaa !52
  %add.i.i.i.i = add nsw i32 %i103, %i99
  %sunkaddr75 = getelementptr inbounds i8, ptr %this, i64 96
  %i106 = load ptr, ptr %sunkaddr75, align 32, !tbaa !51
  %scevgep = getelementptr i32, ptr %i106, i64 %indvars.iv.i.i.i.i
  store i32 %add.i.i.i.i, ptr %scevgep, align 4, !tbaa !52
  %indvars.iv.next.i.i.i.i = add i64 %indvars.iv.i.i.i.i, 1
  %lsr = trunc i64 %indvars.iv.next.i.i.i.i to i32
  %sunkaddr76 = getelementptr inbounds i8, ptr %this, i64 64
  %i109 = load i32, ptr %sunkaddr76, align 64, !tbaa !33
  %cmp.not.i.i.i.i = icmp eq i32 %lsr, %i109
  br i1 %cmp.not.i.i.i.i, label %_ZN3tbb6detail2d119partition_type_baseINS1_19auto_partition_typeEE7executeINS1_9start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEEES8_EEvRT_RT0_RNS1_14execution_dataE.exit, label %for.body.i.i.i.i, !llvm.loop !53

if.else.i.i:                                      ; preds = %lor.lhs.false.i.i
  call void @llvm.lifetime.start.p0(i64 144, ptr nonnull %range_pool.i.i)
  store i8 0, ptr %range_pool.i.i, align 8, !tbaa !55
  %my_tail.i.i.i = getelementptr inbounds %"class.tbb::detail::d1::range_vector", ptr %range_pool.i.i, i64 0, i32 1, !intel-tbaa !60
  store i8 0, ptr %my_tail.i.i.i, align 1, !tbaa !60
  %my_size.i.i.i = getelementptr inbounds %"class.tbb::detail::d1::range_vector", ptr %range_pool.i.i, i64 0, i32 2, !intel-tbaa !61
  store i8 1, ptr %my_size.i.i.i, align 2, !tbaa !61
  %arrayidx.i.i.i = getelementptr inbounds %"class.tbb::detail::d1::range_vector", ptr %range_pool.i.i, i64 0, i32 3, i64 0, !intel-tbaa !62
  store i8 0, ptr %arrayidx.i.i.i, align 1, !tbaa !62
  %i111 = getelementptr inbounds %"class.tbb::detail::d1::range_vector", ptr %range_pool.i.i, i64 0, i32 5, i32 0, i64 0
  call void @llvm.memcpy.p0.p0.i64(ptr noundef nonnull align 8 dereferenceable(16) %i111, ptr noundef nonnull align 64 dereferenceable(16) %my_range, i64 16, i1 false), !tbaa.struct !63
  %my_pool.i.i.i.i.i = getelementptr inbounds %"class.tbb::detail::d1::range_vector", ptr %range_pool.i.i, i64 0, i32 5
  %my_body2.i.i.i.i.i.i = getelementptr inbounds %"struct.tbb::detail::d1::start_for", ptr %this, i64 0, i32 2
  br label %do.body.i.i

do.body.i.i:                                      ; preds = %land.rhs.i.i, %if.else.i.i
  %i113 = phi i8 [ %i212, %land.rhs.i.i ], [ 0, %if.else.i.i ]
  %.pr138.i.i = phi i8 [ %my_size.i.promoted.i104147.i.i, %land.rhs.i.i ], [ 1, %if.else.i.i ]
  %i114 = phi i8 [ %my_head.i.promoted.i107149.i.i, %land.rhs.i.i ], [ 0, %if.else.i.i ]
  %my_head.i.i.promoted.i.i.i = phi i8 [ %my_head.i.i.promoted.i135151.i.i, %land.rhs.i.i ], [ 0, %if.else.i.i ]
  %sunkaddr79 = getelementptr inbounds i8, ptr %this, i64 124
  %i116 = load i8, ptr %sunkaddr79, align 4, !tbaa !28
  %cmp37.i.i.i = icmp ult i8 %.pr138.i.i, 8
  br i1 %cmp37.i.i.i, label %land.rhs.lr.ph.i.i.i, label %invoke.cont6.i.i

land.rhs.lr.ph.i.i.i:                             ; preds = %do.body.i.i
  %idxprom.i.i.i120.i.i = zext i8 %my_head.i.i.promoted.i.i.i to i64
  %arrayidx.i.i.i121.i.i = getelementptr inbounds %"class.tbb::detail::d1::range_vector", ptr %range_pool.i.i, i64 0, i32 3, i64 %idxprom.i.i.i120.i.i, !intel-tbaa !62
  %i117 = load i8, ptr %arrayidx.i.i.i121.i.i, align 1, !tbaa !62
  %cmp.i.i122.i.i = icmp ult i8 %i117, %i116
  br i1 %cmp.i.i122.i.i, label %_ZN3tbb6detail2d112range_vectorINS1_13blocked_rangeIiEELh8EE12is_divisibleEh.exit.i.i.i.preheader, label %invoke.cont6.loopexit.i.i

_ZN3tbb6detail2d112range_vectorINS1_13blocked_rangeIiEELh8EE12is_divisibleEh.exit.i.i.i.preheader: ; preds = %land.rhs.lr.ph.i.i.i
  br label %_ZN3tbb6detail2d112range_vectorINS1_13blocked_rangeIiEELh8EE12is_divisibleEh.exit.i.i.i

land.rhs.i.i.i:                                   ; preds = %while.body.i.i.i
  %cmp.i.i.i.i = icmp ult i8 %inc.i.i.i, %i116
  %inc32.i.i.i = add i8 %i118, 1
  br i1 %cmp.i.i.i.i, label %_ZN3tbb6detail2d112range_vectorINS1_13blocked_rangeIiEELh8EE12is_divisibleEh.exit.i.i.i, label %invoke.cont6.loopexit.i.i, !llvm.loop !64

_ZN3tbb6detail2d112range_vectorINS1_13blocked_rangeIiEELh8EE12is_divisibleEh.exit.i.i.i: ; preds = %land.rhs.i.i.i, %_ZN3tbb6detail2d112range_vectorINS1_13blocked_rangeIiEELh8EE12is_divisibleEh.exit.i.i.i.preheader
  %arrayidx.i.i.i127.i.i = phi ptr [ %arrayidx30.i.i.i, %land.rhs.i.i.i ], [ %arrayidx.i.i.i121.i.i, %_ZN3tbb6detail2d112range_vectorINS1_13blocked_rangeIiEELh8EE12is_divisibleEh.exit.i.i.i.preheader ]
  %idxprom.i.i.i126.i.i = phi i64 [ %rem.i.i.i, %land.rhs.i.i.i ], [ %idxprom.i.i.i120.i.i, %_ZN3tbb6detail2d112range_vectorINS1_13blocked_rangeIiEELh8EE12is_divisibleEh.exit.i.i.i.preheader ]
  %i118 = phi i8 [ %inc32.i.i.i, %land.rhs.i.i.i ], [ %.pr138.i.i, %_ZN3tbb6detail2d112range_vectorINS1_13blocked_rangeIiEELh8EE12is_divisibleEh.exit.i.i.i.preheader ]
  %rem.i116124.i.i = phi i8 [ %promoted85, %land.rhs.i.i.i ], [ %my_head.i.i.promoted.i.i.i, %_ZN3tbb6detail2d112range_vectorINS1_13blocked_rangeIiEELh8EE12is_divisibleEh.exit.i.i.i.preheader ]
  %my_grainsize.i.i.i.i.i = getelementptr inbounds %"class.tbb::detail::d1::blocked_range", ptr %my_pool.i.i.i.i.i, i64 %idxprom.i.i.i126.i.i, i32 2
  %i120 = load i64, ptr %my_grainsize.i.i.i.i.i, align 8, !tbaa !65
  %my_end.i.i.i.i.i.i = getelementptr inbounds %"class.tbb::detail::d1::blocked_range", ptr %my_pool.i.i.i.i.i, i64 %idxprom.i.i.i126.i.i, i32 0
  %i121 = load i32, ptr %my_end.i.i.i.i.i.i, align 8, !tbaa !66
  %conv.i.i.i.i.i.i = sext i32 %i121 to i64
  %my_begin.i.i.i.i.i33.i = getelementptr inbounds %"class.tbb::detail::d1::blocked_range", ptr %my_pool.i.i.i.i.i, i64 %idxprom.i.i.i126.i.i, i32 1
  %i122 = load i32, ptr %my_begin.i.i.i.i.i33.i, align 4, !tbaa !67
  %promoted = sext i32 %i122 to i64
  %sub.i.i.i.i.i.i = sub nsw i64 %conv.i.i.i.i.i.i, %promoted
  %cmp.i.i.i.i.i = icmp ult i64 %i120, %sub.i.i.i.i.i.i
  br i1 %cmp.i.i.i.i.i, label %while.body.i.i.i, label %invoke.cont6.loopexit.i.i

while.body.i.i.i:                                 ; preds = %_ZN3tbb6detail2d112range_vectorINS1_13blocked_rangeIiEELh8EE12is_divisibleEh.exit.i.i.i
  %add.i.i.i = add i8 %rem.i116124.i.i, 1
  %idx.ext.i.i.i = zext i8 %add.i.i.i to i64
  %rem.i.i.i = and i64 %idx.ext.i.i.i, 7
  %promoted85 = trunc i64 %rem.i.i.i to i8
  %add.ptr.i.i.i = getelementptr inbounds %"class.tbb::detail::d1::blocked_range", ptr %my_pool.i.i.i.i.i, i64 %rem.i.i.i, !intel-tbaa !68
  %arrayidx.i46.i.i = getelementptr inbounds %"class.tbb::detail::d1::blocked_range", ptr %my_pool.i.i.i.i.i, i64 %idxprom.i.i.i126.i.i
  call void @llvm.memcpy.p0.p0.i64(ptr noundef nonnull align 8 dereferenceable(16) %add.ptr.i.i.i, ptr noundef nonnull align 8 dereferenceable(16) %arrayidx.i46.i.i, i64 16, i1 false), !tbaa.struct !63
  %i126 = load i32, ptr %add.ptr.i.i.i, align 8, !tbaa !66
  store i32 %i126, ptr %arrayidx.i46.i.i, align 8, !tbaa !66
  %my_begin.i.i.i48.i.i = getelementptr inbounds %"class.tbb::detail::d1::blocked_range", ptr %my_pool.i.i.i.i.i, i64 %rem.i.i.i, i32 1
  %i127 = load i32, ptr %my_begin.i.i.i48.i.i, align 4, !tbaa !67
  %i128 = load i32, ptr %add.ptr.i.i.i, align 8, !tbaa !66
  %sub.i.i.i.i.i = sub nsw i32 %i128, %i127
  %div8.i.i.i.i.i = lshr i32 %sub.i.i.i.i.i, 1
  %add.i.i.i.i.i = add i32 %div8.i.i.i.i.i, %i127
  store i32 %add.i.i.i.i.i, ptr %add.ptr.i.i.i, align 8, !tbaa !66
  store i32 %add.i.i.i.i.i, ptr %my_begin.i.i.i.i.i33.i, align 4, !tbaa !67
  %my_grainsize3.i.i.i.i = getelementptr inbounds %"class.tbb::detail::d1::blocked_range", ptr %my_pool.i.i.i.i.i, i64 %rem.i.i.i, i32 2
  %i129 = load i64, ptr %my_grainsize3.i.i.i.i, align 8, !tbaa !65
  store i64 %i129, ptr %my_grainsize.i.i.i.i.i, align 8, !tbaa !65
  %i130 = load i8, ptr %arrayidx.i.i.i127.i.i, align 1, !tbaa !62
  %inc.i.i.i = add i8 %i130, 1
  store i8 %inc.i.i.i, ptr %arrayidx.i.i.i127.i.i, align 1, !tbaa !62
  %arrayidx30.i.i.i = getelementptr inbounds %"class.tbb::detail::d1::range_vector", ptr %range_pool.i.i, i64 0, i32 3, i64 %rem.i.i.i
  store i8 %inc.i.i.i, ptr %arrayidx30.i.i.i, align 1, !tbaa !62
  %exitcond.not.i.i.i = icmp eq i8 %i118, 7
  br i1 %exitcond.not.i.i.i, label %invoke.cont6.loopexit.i.i, label %land.rhs.i.i.i, !llvm.loop !64

invoke.cont6.loopexit.i.i:                        ; preds = %while.body.i.i.i, %_ZN3tbb6detail2d112range_vectorINS1_13blocked_rangeIiEELh8EE12is_divisibleEh.exit.i.i.i, %land.rhs.i.i.i, %land.rhs.lr.ph.i.i.i
  %inc32.i119.i.i = phi i8 [ %.pr138.i.i, %land.rhs.lr.ph.i.i.i ], [ 8, %while.body.i.i.i ], [ %inc32.i.i.i, %land.rhs.i.i.i ], [ %i118, %_ZN3tbb6detail2d112range_vectorINS1_13blocked_rangeIiEELh8EE12is_divisibleEh.exit.i.i.i ]
  %rem.i117.i.i = phi i8 [ %my_head.i.i.promoted.i.i.i, %land.rhs.lr.ph.i.i.i ], [ %promoted85, %while.body.i.i.i ], [ %promoted85, %land.rhs.i.i.i ], [ %rem.i116124.i.i, %_ZN3tbb6detail2d112range_vectorINS1_13blocked_rangeIiEELh8EE12is_divisibleEh.exit.i.i.i ]
  store i8 %rem.i117.i.i, ptr %range_pool.i.i, align 8, !tbaa !55
  %sunkaddr86 = getelementptr inbounds i8, ptr %range_pool.i.i, i64 2
  store i8 %inc32.i119.i.i, ptr %sunkaddr86, align 2, !tbaa !61
  br label %invoke.cont6.i.i

invoke.cont6.i.i:                                 ; preds = %invoke.cont6.loopexit.i.i, %do.body.i.i
  %i133 = phi i8 [ %inc32.i119.i.i, %invoke.cont6.loopexit.i.i ], [ %.pr138.i.i, %do.body.i.i ]
  %i134 = phi i8 [ %rem.i117.i.i, %invoke.cont6.loopexit.i.i ], [ %i114, %do.body.i.i ]
  %my_head.i.i.promoted.i136.i.i = phi i8 [ %rem.i117.i.i, %invoke.cont6.loopexit.i.i ], [ %my_head.i.i.promoted.i.i.i, %do.body.i.i ]
  %sunkaddr87 = getelementptr inbounds i8, ptr %this, i64 104
  %i137 = load ptr, ptr %sunkaddr87, align 8, !tbaa !13
  %_M_i.i.i.i.i.i.i = getelementptr inbounds %"struct.tbb::detail::d1::tree_node", ptr %i137, i64 0, i32 3, i32 0, i32 0, !intel-tbaa !23
  %i138 = load atomic i8, ptr %_M_i.i.i.i.i.i.i monotonic, align 1
  %i139 = and i8 %i138, 1
  %tobool.i.i.i.i.not.i.i = icmp eq i8 %i139, 0
  br i1 %tobool.i.i.i.i.not.i.i, label %invoke.cont6.invoke.cont28_crit_edge.i.i, label %if.then10.i.i

invoke.cont6.invoke.cont28_crit_edge.i.i:         ; preds = %invoke.cont6.i.i
  %.pre.i.i = zext i8 %i134 to i64
  br label %invoke.cont28.i.i

if.then10.i.i:                                    ; preds = %invoke.cont6.i.i
  %add.i49.i.i = add i8 %i116, 1
  %sunkaddr88 = getelementptr inbounds i8, ptr %this, i64 124
  store i8 %add.i49.i.i, ptr %sunkaddr88, align 4, !tbaa !28
  %cmp.i34.i = icmp ugt i8 %i133, 1
  br i1 %cmp.i34.i, label %invoke.cont14.i.i, label %if.end.i37.i

invoke.cont14.i.i:                                ; preds = %if.then10.i.i
  %idxprom.i.i.i = zext i8 %i113 to i64
  %arrayidx.i55.i.i = getelementptr inbounds %"class.tbb::detail::d1::range_vector", ptr %range_pool.i.i, i64 0, i32 3, i64 %idxprom.i.i.i, !intel-tbaa !62
  %i146 = load i8, ptr %arrayidx.i55.i.i, align 1, !tbaa !62
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %alloc.i.i.i.i)
  store ptr null, ptr %alloc.i.i.i.i, align 8, !tbaa !35
  %call.i.i.i57.i.i = call noundef ptr @_ZN3tbb6detail2r18allocateERPNS0_2d117small_object_poolEmRKNS2_14execution_dataE(ptr noundef nonnull align 8 dereferenceable(8) %alloc.i.i.i.i, i64 noundef 192, ptr noundef nonnull align 8 dereferenceable(12) %ed)
  %arrayidx.i52.i.i = getelementptr inbounds %"class.tbb::detail::d1::blocked_range", ptr %my_pool.i.i.i.i.i, i64 %idxprom.i.i.i
  %m_version_and_traits.i.i.i.i.i.i.i.i = getelementptr inbounds i8, ptr %call.i.i.i57.i.i, i64 8
  %arrayinit.end.i.i.i.i.i.i.i = getelementptr inbounds i8, ptr %call.i.i.i57.i.i, i64 64
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(56) %m_version_and_traits.i.i.i.i.i.i.i.i, i8 0, i64 56, i1 false)
  store ptr getelementptr inbounds ({ [6 x ptr] }, ptr @_ZTVN3tbb6detail2d19start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEEE, i64 0, inrange i32 0, i64 2), ptr %call.i.i.i57.i.i, align 64, !tbaa !37
  call void @llvm.memcpy.p0.p0.i64(ptr noundef nonnull align 64 dereferenceable(16) %arrayinit.end.i.i.i.i.i.i.i, ptr noundef nonnull align 8 dereferenceable(16) %arrayidx.i52.i.i, i64 16, i1 false), !tbaa.struct !63
  %my_body.i.i.i.i.i.i = getelementptr inbounds i8, ptr %call.i.i.i57.i.i, i64 80
  call void @llvm.memcpy.p0.p0.i64(ptr noundef nonnull align 16 dereferenceable(24) %my_body.i.i.i.i.i.i, ptr noundef nonnull align 16 dereferenceable(24) %my_body2.i.i.i.i.i.i, i64 24, i1 false), !tbaa.struct !39
  %my_divisor.i.i.i.i.i.i.i.i35.i = getelementptr inbounds i8, ptr %call.i.i.i57.i.i, i64 112
  %sunkaddr89 = getelementptr inbounds i8, ptr %this, i64 112
  %i152 = load i64, ptr %sunkaddr89, align 16, !tbaa !9
  %div2.i.i.i.i.i.i.i.i.i.i = lshr i64 %i152, 1
  store i64 %div2.i.i.i.i.i.i.i.i.i.i, ptr %sunkaddr89, align 16, !tbaa !9
  store i64 %div2.i.i.i.i.i.i.i.i.i.i, ptr %my_divisor.i.i.i.i.i.i.i.i35.i, align 16, !tbaa !9
  %i153 = getelementptr inbounds i8, ptr %call.i.i.i57.i.i, i64 120
  store i32 2, ptr %i153, align 8, !tbaa !41
  %i155 = getelementptr inbounds i8, ptr %call.i.i.i57.i.i, i64 124
  %sunkaddr90 = getelementptr inbounds i8, ptr %this, i64 124
  %i157 = load i8, ptr %sunkaddr90, align 4, !tbaa !28
  %i158 = getelementptr inbounds i8, ptr %call.i.i.i57.i.i, i64 128
  %i160 = load ptr, ptr %alloc.i.i.i.i, align 8, !tbaa !35
  store ptr %i160, ptr %i158, align 64, !tbaa !42
  %sub.i.i.i.i.i.i36.i = sub i8 %i157, %i146
  store i8 %sub.i.i.i.i.i.i36.i, ptr %i155, align 4, !tbaa !28
  %call.i15.i.i58.i.i = call noundef ptr @_ZN3tbb6detail2r18allocateERPNS0_2d117small_object_poolEmRKNS2_14execution_dataE(ptr noundef nonnull align 8 dereferenceable(8) %alloc.i.i.i.i, i64 noundef 32, ptr noundef nonnull align 8 dereferenceable(12) %ed)
  %sunkaddr91 = getelementptr inbounds i8, ptr %this, i64 104
  %i163 = load ptr, ptr %sunkaddr91, align 8, !tbaa !13
  store ptr %i163, ptr %call.i15.i.i58.i.i, align 8, !tbaa !43
  %i164 = getelementptr inbounds i8, ptr %call.i15.i.i58.i.i, i64 8
  store i32 2, ptr %i164, align 8, !tbaa !46
  %i166 = getelementptr inbounds i8, ptr %call.i15.i.i58.i.i, i64 16
  %i168 = load ptr, ptr %alloc.i.i.i.i, align 8, !tbaa !35
  store ptr %i168, ptr %i166, align 8, !tbaa !42
  %i169 = getelementptr inbounds i8, ptr %call.i15.i.i58.i.i, i64 24
  store i8 0, ptr %i169, align 8, !tbaa !23
  %sunkaddr92 = getelementptr inbounds i8, ptr %this, i64 104
  store ptr %call.i15.i.i58.i.i, ptr %sunkaddr92, align 8, !tbaa !13
  %my_parent8.i.i.i.i = getelementptr inbounds i8, ptr %call.i.i.i57.i.i, i64 104
  store ptr %call.i15.i.i58.i.i, ptr %my_parent8.i.i.i.i, align 8, !tbaa !13
  %i174 = load ptr, ptr %ed, align 8, !tbaa !48
  call void @_ZN3tbb6detail2r15spawnERNS0_2d14taskERNS2_18task_group_contextE(ptr noundef nonnull align 64 dereferenceable(64) %call.i.i.i57.i.i, ptr noundef nonnull align 8 dereferenceable(128) %i174)
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %alloc.i.i.i.i)
  %dec.i.i.i = add i8 %i133, -1
  %sunkaddr93 = getelementptr inbounds i8, ptr %range_pool.i.i, i64 2
  store i8 %dec.i.i.i, ptr %sunkaddr93, align 2, !tbaa !61
  %i176 = add i8 %i113, 1
  %i177 = and i8 %i176, 7
  %sunkaddr94 = getelementptr inbounds i8, ptr %range_pool.i.i, i64 1
  store i8 %i177, ptr %sunkaddr94, align 1, !tbaa !60
  br label %land.rhs.i.i

if.end.i37.i:                                     ; preds = %if.then10.i.i
  %idxprom.i.i.i.i = zext i8 %i134 to i64
  %arrayidx.i.i65.i.i = getelementptr inbounds %"class.tbb::detail::d1::range_vector", ptr %range_pool.i.i, i64 0, i32 3, i64 %idxprom.i.i.i.i, !intel-tbaa !62
  %i179 = load i8, ptr %arrayidx.i.i65.i.i, align 1, !tbaa !62
  %cmp.i66.i.i = icmp ult i8 %i179, %add.i49.i.i
  br i1 %cmp.i66.i.i, label %invoke.cont23.i.i, label %invoke.cont28.i.i

invoke.cont23.i.i:                                ; preds = %if.end.i37.i
  %my_grainsize.i.i.i.i = getelementptr inbounds %"class.tbb::detail::d1::blocked_range", ptr %my_pool.i.i.i.i.i, i64 %idxprom.i.i.i.i, i32 2
  %i181 = load i64, ptr %my_grainsize.i.i.i.i, align 8, !tbaa !65
  %my_end.i.i.i67.i.i = getelementptr inbounds %"class.tbb::detail::d1::blocked_range", ptr %my_pool.i.i.i.i.i, i64 %idxprom.i.i.i.i, i32 0
  %i182 = load i32, ptr %my_end.i.i.i67.i.i, align 8, !tbaa !66
  %conv.i.i.i.i.i = sext i32 %i182 to i64
  %my_begin.i.i.i68.i.i = getelementptr inbounds %"class.tbb::detail::d1::blocked_range", ptr %my_pool.i.i.i.i.i, i64 %idxprom.i.i.i.i, i32 1
  %i183 = load i32, ptr %my_begin.i.i.i68.i.i, align 4, !tbaa !67
  %promoted95 = sext i32 %i183 to i64
  %sub.i.i.i69.i.i = sub nsw i64 %conv.i.i.i.i.i, %promoted95
  %cmp.i.i70.i.i = icmp ult i64 %i181, %sub.i.i.i69.i.i
  br i1 %cmp.i.i70.i.i, label %do.cond.i.i, label %invoke.cont28.i.i

invoke.cont28.i.i:                                ; preds = %invoke.cont23.i.i, %if.end.i37.i, %invoke.cont6.invoke.cont28_crit_edge.i.i
  %idxprom.i74.pre-phi.i.i = phi i64 [ %.pre.i.i, %invoke.cont6.invoke.cont28_crit_edge.i.i ], [ %idxprom.i.i.i.i, %if.end.i37.i ], [ %idxprom.i.i.i.i, %invoke.cont23.i.i ]
  %my_begin.i.i.i76.i.i = getelementptr inbounds %"class.tbb::detail::d1::blocked_range", ptr %my_pool.i.i.i.i.i, i64 %idxprom.i74.pre-phi.i.i, i32 1
  %i185 = load i32, ptr %my_begin.i.i.i76.i.i, align 4, !tbaa !67
  %i186 = sext i32 %i185 to i64
  %my_end.i.i.i77.i.i = getelementptr inbounds %"class.tbb::detail::d1::blocked_range", ptr %my_pool.i.i.i.i.i, i64 %idxprom.i74.pre-phi.i.i, i32 0
  %i187 = load i32, ptr %my_end.i.i.i77.i.i, align 8, !tbaa !66
  %cmp.not17.i.i78.i.i = icmp eq i32 %i185, %i187
  br i1 %cmp.not17.i.i78.i.i, label %invoke.cont30.i.i, label %for.body.lr.ph.i.i85.i.i

for.body.lr.ph.i.i85.i.i:                         ; preds = %invoke.cont28.i.i
  %sunkaddr96 = getelementptr inbounds i8, ptr %this, i64 80
  %.pre.i.i82.i.i = load ptr, ptr %sunkaddr96, align 16, !tbaa !49
  %sunkaddr97 = getelementptr inbounds i8, ptr %this, i64 88
  %.pre19.i.i83.i.i = load ptr, ptr %sunkaddr97, align 8, !tbaa !50
  %sunkaddr98 = getelementptr inbounds i8, ptr %this, i64 96
  %.pre20.i.i84.i.i = load ptr, ptr %sunkaddr98, align 32, !tbaa !51
  br label %for.body.i.i93.i.i

for.body.i.i93.i.i:                               ; preds = %for.body.i.i93.i.i, %for.body.lr.ph.i.i85.i.i
  %i194 = phi ptr [ %.pre20.i.i84.i.i, %for.body.lr.ph.i.i85.i.i ], [ %i207, %for.body.i.i93.i.i ]
  %i195 = phi ptr [ %.pre19.i.i83.i.i, %for.body.lr.ph.i.i85.i.i ], [ %i203, %for.body.i.i93.i.i ]
  %i196 = phi ptr [ %.pre.i.i82.i.i, %for.body.lr.ph.i.i85.i.i ], [ %i199, %for.body.i.i93.i.i ]
  %indvars.iv.i.i86.i.i = phi i64 [ %i186, %for.body.lr.ph.i.i85.i.i ], [ %indvars.iv.next.i.i91.i.i, %for.body.i.i93.i.i ]
  %lsr53 = trunc i64 %indvars.iv.i.i86.i.i to i32
  call fastcc void @_ZL14delta_steppingPiS_S_i(ptr noundef %i196, ptr noundef %i195, ptr noundef %i194, i32 noundef %lsr53)
  %sunkaddr99 = getelementptr inbounds i8, ptr %this, i64 80
  %i199 = load ptr, ptr %sunkaddr99, align 16, !tbaa !49
  %scevgep51 = getelementptr i32, ptr %i199, i64 %indvars.iv.i.i86.i.i
  %i200 = load i32, ptr %scevgep51, align 4, !tbaa !52
  %sunkaddr100 = getelementptr inbounds i8, ptr %this, i64 88
  %i203 = load ptr, ptr %sunkaddr100, align 8, !tbaa !50
  %scevgep50 = getelementptr i32, ptr %i203, i64 %indvars.iv.i.i86.i.i
  %i204 = load i32, ptr %scevgep50, align 4, !tbaa !52
  %add.i.i89.i.i = add nsw i32 %i204, %i200
  %sunkaddr101 = getelementptr inbounds i8, ptr %this, i64 96
  %i207 = load ptr, ptr %sunkaddr101, align 32, !tbaa !51
  %scevgep49 = getelementptr i32, ptr %i207, i64 %indvars.iv.i.i86.i.i
  store i32 %add.i.i89.i.i, ptr %scevgep49, align 4, !tbaa !52
  %indvars.iv.next.i.i91.i.i = add nsw i64 %indvars.iv.i.i86.i.i, 1
  %lsr52 = trunc i64 %indvars.iv.next.i.i91.i.i to i32
  %cmp.not.i.i92.i.i = icmp eq i32 %i187, %lsr52
  br i1 %cmp.not.i.i92.i.i, label %invoke.cont30.i.i, label %for.body.i.i93.i.i, !llvm.loop !53

invoke.cont30.i.i:                                ; preds = %for.body.i.i93.i.i, %invoke.cont28.i.i
  %dec.i97.i.i = add i8 %i133, -1
  %sunkaddr102 = getelementptr inbounds i8, ptr %range_pool.i.i, i64 2
  store i8 %dec.i97.i.i, ptr %sunkaddr102, align 2, !tbaa !61
  %i210 = add i8 %i134, 7
  %i211 = and i8 %i210, 7
  store i8 %i211, ptr %range_pool.i.i, align 8, !tbaa !55
  br label %do.cond.i.i

do.cond.i.i:                                      ; preds = %invoke.cont30.i.i, %invoke.cont23.i.i
  %my_size.i.promoted.i104.i.i = phi i8 [ %dec.i97.i.i, %invoke.cont30.i.i ], [ %i133, %invoke.cont23.i.i ]
  %my_head.i.promoted.i107.i.i = phi i8 [ %i211, %invoke.cont30.i.i ], [ %i134, %invoke.cont23.i.i ]
  %cmp.i99.i.i = icmp eq i8 %my_size.i.promoted.i104.i.i, 0
  br i1 %cmp.i99.i.i, label %_ZN3tbb6detail2d112range_vectorINS1_13blocked_rangeIiEELh8EED2Ev.exit114.i.i, label %land.rhs.i.i

land.rhs.i.i:                                     ; preds = %do.cond.i.i, %invoke.cont14.i.i
  %my_head.i.i.promoted.i135151.i.i = phi i8 [ %my_head.i.i.promoted.i136.i.i, %invoke.cont14.i.i ], [ %my_head.i.promoted.i107.i.i, %do.cond.i.i ]
  %my_head.i.promoted.i107149.i.i = phi i8 [ %i134, %invoke.cont14.i.i ], [ %my_head.i.promoted.i107.i.i, %do.cond.i.i ]
  %my_size.i.promoted.i104147.i.i = phi i8 [ %dec.i.i.i, %invoke.cont14.i.i ], [ %my_size.i.promoted.i104.i.i, %do.cond.i.i ]
  %i212 = phi i8 [ %i177, %invoke.cont14.i.i ], [ %i113, %do.cond.i.i ]
  %i214 = load ptr, ptr %ed, align 8, !tbaa !48
  %_M_i.i.i.i.i100.i.i = getelementptr inbounds %"class.tbb::detail::d1::task_group_context", ptr %i214, i64 0, i32 5, i32 0, !intel-tbaa !69
  %i215 = load atomic i8, ptr %_M_i.i.i.i.i100.i.i monotonic, align 1
  %cmp.i.i.i101.i.i = icmp eq i8 %i215, -1
  %my_actual_context.i.i.i.i = getelementptr inbounds %"class.tbb::detail::d1::task_group_context", ptr %i214, i64 0, i32 6, i32 0
  %i216 = load ptr, ptr %my_actual_context.i.i.i.i, align 8
  %i217 = select i1 %cmp.i.i.i101.i.i, ptr %i216, ptr %i214
  %call2.i102.i.i = call noundef zeroext i1 @_ZN3tbb6detail2r128is_group_execution_cancelledERNS0_2d118task_group_contextE(ptr noundef nonnull align 8 dereferenceable(128) %i217)
  br i1 %call2.i102.i.i, label %_ZN3tbb6detail2d112range_vectorINS1_13blocked_rangeIiEELh8EED2Ev.exit114.i.i, label %do.body.i.i, !llvm.loop !86

_ZN3tbb6detail2d112range_vectorINS1_13blocked_rangeIiEELh8EED2Ev.exit114.i.i: ; preds = %land.rhs.i.i, %do.cond.i.i
  call void @llvm.lifetime.end.p0(i64 144, ptr nonnull %range_pool.i.i)
  br label %_ZN3tbb6detail2d119partition_type_baseINS1_19auto_partition_typeEE7executeINS1_9start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEEES8_EEvRT_RT0_RNS1_14execution_dataE.exit

_ZN3tbb6detail2d119partition_type_baseINS1_19auto_partition_typeEE7executeINS1_9start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEEES8_EEvRT_RT0_RNS1_14execution_dataE.exit: ; preds = %_ZN3tbb6detail2d112range_vectorINS1_13blocked_rangeIiEELh8EED2Ev.exit114.i.i, %for.body.i.i.i.i, %if.then.i.i
  %my_parent.i12 = getelementptr inbounds %"struct.tbb::detail::d1::start_for", ptr %this, i64 0, i32 3, !intel-tbaa !13
  %i219 = load ptr, ptr %my_parent.i12, align 8, !tbaa !13
  %i220 = getelementptr inbounds %"struct.tbb::detail::d1::start_for", ptr %this, i64 0, i32 5, i32 0
  %i221 = load ptr, ptr %i220, align 64, !tbaa !42
  %vtable.i = load ptr, ptr %this, align 64, !tbaa !37
  %i223 = load ptr, ptr %vtable.i, align 8
  call void %i223(ptr noundef nonnull align 64 dereferenceable(136) %this)
  %_M_i.i18.i.i = getelementptr inbounds %"struct.tbb::detail::d1::node", ptr %i219, i64 0, i32 1, i32 0, i32 0
  %i224 = atomicrmw sub ptr %_M_i.i18.i.i, i32 1 seq_cst, align 4
  %i225 = add i32 %i224, -1
  %cmp19.i.i = icmp sgt i32 %i225, 0
  br i1 %cmp19.i.i, label %_ZN3tbb6detail2d19start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEE8finalizeERKNS1_14execution_dataE.exit, label %if.end.i.i14.preheader

if.end.i.i14.preheader:                           ; preds = %_ZN3tbb6detail2d119partition_type_baseINS1_19auto_partition_typeEE7executeINS1_9start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEEES8_EEvRT_RT0_RNS1_14execution_dataE.exit
  br label %if.end.i.i14

if.end.i.i14:                                     ; preds = %cleanup.i.i, %if.end.i.i14.preheader
  %n.addr.020.i.i = phi ptr [ %i226, %cleanup.i.i ], [ %i219, %if.end.i.i14.preheader ]
  %i226 = load ptr, ptr %n.addr.020.i.i, align 8, !tbaa !43
  %tobool.not.i.i13 = icmp eq ptr %i226, null
  br i1 %tobool.not.i.i13, label %for.end.i.i, label %cleanup.i.i

cleanup.i.i:                                      ; preds = %if.end.i.i14
  %m_allocator.i.i = getelementptr inbounds %"struct.tbb::detail::d1::node", ptr %n.addr.020.i.i, i64 1
  %i228 = load ptr, ptr %m_allocator.i.i, align 8, !tbaa !42
  call void @_ZN3tbb6detail2r110deallocateERNS0_2d117small_object_poolEPvmRKNS2_14execution_dataE(ptr noundef nonnull align 1 dereferenceable(1) %i228, ptr noundef %n.addr.020.i.i, i64 noundef 32, ptr noundef nonnull align 8 dereferenceable(12) %ed)
  %_M_i.i.i.i15 = getelementptr inbounds %"struct.tbb::detail::d1::node", ptr %i226, i64 0, i32 1, i32 0, i32 0
  %i230 = atomicrmw sub ptr %_M_i.i.i.i15, i32 1 seq_cst, align 4
  %i231 = add i32 %i230, -1
  %cmp.i.i16 = icmp sgt i32 %i231, 0
  br i1 %cmp.i.i16, label %_ZN3tbb6detail2d19start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEE8finalizeERKNS1_14execution_dataE.exit, label %if.end.i.i14

for.end.i.i:                                      ; preds = %if.end.i.i14
  %_M_i.i.i.i.i.i = getelementptr inbounds %"struct.tbb::detail::d1::node", ptr %n.addr.020.i.i, i64 1, i32 1
  %i233 = atomicrmw add ptr %_M_i.i.i.i.i.i, i64 -1 seq_cst, align 8
  %tobool.not.i.i.i.i = icmp eq i64 %i233, 1
  br i1 %tobool.not.i.i.i.i, label %if.then.i.i.i.i, label %_ZN3tbb6detail2d19start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEE8finalizeERKNS1_14execution_dataE.exit

if.then.i.i.i.i:                                  ; preds = %for.end.i.i
  %m_wait.i.i = getelementptr inbounds %"struct.tbb::detail::d1::node", ptr %n.addr.020.i.i, i64 1
  %i234 = ptrtoint ptr %m_wait.i.i to i64
  call void @_ZN3tbb6detail2r114notify_waitersEm(i64 noundef %i234)
  br label %_ZN3tbb6detail2d19start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEE8finalizeERKNS1_14execution_dataE.exit

_ZN3tbb6detail2d19start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEE8finalizeERKNS1_14execution_dataE.exit: ; preds = %if.then.i.i.i.i, %for.end.i.i, %cleanup.i.i, %_ZN3tbb6detail2d119partition_type_baseINS1_19auto_partition_typeEE7executeINS1_9start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEEES8_EEvRT_RT0_RNS1_14execution_dataE.exit
  call void @_ZN3tbb6detail2r110deallocateERNS0_2d117small_object_poolEPvmRKNS2_14execution_dataE(ptr noundef nonnull align 1 dereferenceable(1) %i221, ptr noundef nonnull %this, i64 noundef 192, ptr noundef nonnull align 8 dereferenceable(12) %ed)
  ret ptr null
}

define linkonce_odr dso_local noundef ptr @_ZN3tbb6detail2d19start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEE6cancelERNS1_14execution_dataE(ptr noundef nonnull align 64 dereferenceable(136) %this, ptr noundef nonnull align 8 dereferenceable(12) %ed) comdat align 2 {
entry:
  %my_parent.i = getelementptr inbounds %"struct.tbb::detail::d1::start_for", ptr %this, i64 0, i32 3, !intel-tbaa !13
  %i = load ptr, ptr %my_parent.i, align 8, !tbaa !13
  %i1 = getelementptr inbounds %"struct.tbb::detail::d1::start_for", ptr %this, i64 0, i32 5, i32 0
  %i2 = load ptr, ptr %i1, align 64, !tbaa !42
  %vtable.i = load ptr, ptr %this, align 64, !tbaa !37
  %i4 = load ptr, ptr %vtable.i, align 8
  tail call void %i4(ptr noundef nonnull align 64 dereferenceable(136) %this)
  %_M_i.i18.i.i = getelementptr inbounds %"struct.tbb::detail::d1::node", ptr %i, i64 0, i32 1, i32 0, i32 0
  %i5 = atomicrmw sub ptr %_M_i.i18.i.i, i32 1 seq_cst, align 4
  %i6 = add i32 %i5, -1
  %cmp19.i.i = icmp sgt i32 %i6, 0
  br i1 %cmp19.i.i, label %_ZN3tbb6detail2d19start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEE8finalizeERKNS1_14execution_dataE.exit, label %if.end.i.i.preheader

if.end.i.i.preheader:                             ; preds = %entry
  br label %if.end.i.i

if.end.i.i:                                       ; preds = %cleanup.i.i, %if.end.i.i.preheader
  %n.addr.020.i.i = phi ptr [ %i7, %cleanup.i.i ], [ %i, %if.end.i.i.preheader ]
  %i7 = load ptr, ptr %n.addr.020.i.i, align 8, !tbaa !43
  %tobool.not.i.i = icmp eq ptr %i7, null
  br i1 %tobool.not.i.i, label %for.end.i.i, label %cleanup.i.i

cleanup.i.i:                                      ; preds = %if.end.i.i
  %m_allocator.i.i = getelementptr inbounds %"struct.tbb::detail::d1::node", ptr %n.addr.020.i.i, i64 1
  %i9 = load ptr, ptr %m_allocator.i.i, align 8, !tbaa !42
  tail call void @_ZN3tbb6detail2r110deallocateERNS0_2d117small_object_poolEPvmRKNS2_14execution_dataE(ptr noundef nonnull align 1 dereferenceable(1) %i9, ptr noundef %n.addr.020.i.i, i64 noundef 32, ptr noundef nonnull align 8 dereferenceable(12) %ed)
  %_M_i.i.i.i = getelementptr inbounds %"struct.tbb::detail::d1::node", ptr %i7, i64 0, i32 1, i32 0, i32 0
  %i11 = atomicrmw sub ptr %_M_i.i.i.i, i32 1 seq_cst, align 4
  %i12 = add i32 %i11, -1
  %cmp.i.i = icmp sgt i32 %i12, 0
  br i1 %cmp.i.i, label %_ZN3tbb6detail2d19start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEE8finalizeERKNS1_14execution_dataE.exit, label %if.end.i.i

for.end.i.i:                                      ; preds = %if.end.i.i
  %_M_i.i.i.i.i.i = getelementptr inbounds %"struct.tbb::detail::d1::node", ptr %n.addr.020.i.i, i64 1, i32 1
  %i14 = atomicrmw add ptr %_M_i.i.i.i.i.i, i64 -1 seq_cst, align 8
  %tobool.not.i.i.i.i = icmp eq i64 %i14, 1
  br i1 %tobool.not.i.i.i.i, label %if.then.i.i.i.i, label %_ZN3tbb6detail2d19start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEE8finalizeERKNS1_14execution_dataE.exit

if.then.i.i.i.i:                                  ; preds = %for.end.i.i
  %m_wait.i.i = getelementptr inbounds %"struct.tbb::detail::d1::node", ptr %n.addr.020.i.i, i64 1
  %i15 = ptrtoint ptr %m_wait.i.i to i64
  tail call void @_ZN3tbb6detail2r114notify_waitersEm(i64 noundef %i15)
  br label %_ZN3tbb6detail2d19start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEE8finalizeERKNS1_14execution_dataE.exit

_ZN3tbb6detail2d19start_forINS1_13blocked_rangeIiEE11ArraySummerKNS1_16auto_partitionerEE8finalizeERKNS1_14execution_dataE.exit: ; preds = %if.then.i.i.i.i, %for.end.i.i, %cleanup.i.i, %entry
  tail call void @_ZN3tbb6detail2r110deallocateERNS0_2d117small_object_poolEPvmRKNS2_14execution_dataE(ptr noundef nonnull align 1 dereferenceable(1) %i2, ptr noundef nonnull %this, i64 noundef 192, ptr noundef nonnull align 8 dereferenceable(12) %ed)
  ret ptr null
}

declare dso_local noundef i32 @_ZN3tbb6detail2r115max_concurrencyEPKNS0_2d115task_arena_baseE(ptr noundef)

declare dso_local void @_ZdlPv(ptr noundef)

declare dso_local noundef zeroext i16 @_ZN3tbb6detail2r114execution_slotEPKNS0_2d114execution_dataE(ptr noundef)

define linkonce_odr hidden void @__clang_call_terminate(ptr %arg) comdat {
bb:
  %i = tail call ptr @__cxa_begin_catch(ptr %arg)
  tail call void @_ZSt9terminatev()
  unreachable
}

declare dso_local ptr @__cxa_begin_catch(ptr)

declare dso_local void @_ZSt9terminatev()

declare dso_local noundef ptr @_ZN3tbb6detail2r18allocateERPNS0_2d117small_object_poolEmRKNS2_14execution_dataE(ptr noundef nonnull align 8 dereferenceable(8), i64 noundef, ptr noundef nonnull align 8 dereferenceable(12))

declare dso_local void @_ZN3tbb6detail2r15spawnERNS0_2d14taskERNS2_18task_group_contextE(ptr noundef nonnull align 64 dereferenceable(64), ptr noundef nonnull align 8 dereferenceable(128))

define internal fastcc void @_ZL14delta_steppingPiS_S_i(ptr nocapture noundef readonly %p_array_a, ptr nocapture noundef readonly %p_array_b, ptr nocapture noundef writeonly %p_array_sum, i32 noundef %index) {
entry:
  tail call fastcc void @_ZL5relaxPiS_S_i(ptr noundef %p_array_a, ptr noundef %p_array_b, ptr noundef %p_array_sum, i32 noundef %index)
  tail call fastcc void @_ZL5relaxPiS_S_i(ptr noundef %p_array_a, ptr noundef %p_array_b, ptr noundef %p_array_sum, i32 noundef %index)
  tail call fastcc void @_ZL5relaxPiS_S_i(ptr noundef %p_array_a, ptr noundef %p_array_b, ptr noundef %p_array_sum, i32 noundef %index)
  ret void
}

define internal fastcc void @_ZL5relaxPiS_S_i(ptr nocapture noundef readonly %p_array_a, ptr nocapture noundef readonly %p_array_b, ptr nocapture noundef writeonly %p_array_sum, i32 noundef %index) {
entry:
  %cmp = icmp sgt i32 %index, 10
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %idxprom = zext i32 %index to i64
  %arrayidx = getelementptr inbounds i32, ptr %p_array_a, i64 %idxprom
  %i = load i32, ptr %arrayidx, align 4, !tbaa !52
  %arrayidx2 = getelementptr inbounds i32, ptr %p_array_b, i64 %idxprom
  %i1 = load i32, ptr %arrayidx2, align 4, !tbaa !52
  %add = add nsw i32 %i1, %i
  %arrayidx4 = getelementptr inbounds i32, ptr %p_array_sum, i64 %idxprom
  store i32 %add, ptr %arrayidx4, align 4, !tbaa !52
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  ret void
}

declare dso_local noundef zeroext i1 @_ZN3tbb6detail2r128is_group_execution_cancelledERNS0_2d118task_group_contextE(ptr noundef nonnull align 8 dereferenceable(128))

declare dso_local void @_ZN3tbb6detail2r110deallocateERNS0_2d117small_object_poolEPvmRKNS2_14execution_dataE(ptr noundef nonnull align 1 dereferenceable(1), ptr noundef, i64 noundef, ptr noundef nonnull align 8 dereferenceable(12))

declare dso_local void @_ZN3tbb6detail2r114notify_waitersEm(i64 noundef)

declare dso_local void @_ZN3tbb6detail2r116execute_and_waitERNS0_2d14taskERNS2_18task_group_contextERNS2_12wait_contextES6_(ptr noundef nonnull align 64 dereferenceable(64), ptr noundef nonnull align 8 dereferenceable(128), ptr noundef nonnull align 8 dereferenceable(16), ptr noundef nonnull align 8 dereferenceable(128))

declare dso_local void @_ZN3tbb6detail2r17destroyERNS0_2d118task_group_contextE(ptr noundef nonnull align 8 dereferenceable(128))

declare void @_Unwind_Resume(ptr)

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #0

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #0

; Function Attrs: argmemonly nofree nounwind willreturn
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #1

; Function Attrs: argmemonly nofree nounwind willreturn writeonly
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #2

attributes #0 = { argmemonly nocallback nofree nosync nounwind willreturn }
attributes #1 = { argmemonly nofree nounwind willreturn }
attributes #2 = { argmemonly nofree nounwind willreturn writeonly }

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
