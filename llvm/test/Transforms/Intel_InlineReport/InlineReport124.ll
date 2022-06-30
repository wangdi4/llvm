; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: system-windows,intel_feature_sw_advanced
; RUN: opt -passes='cgscc(inline)' -inline-report=0xe807 -inline-threshold=0 -inline-tbb-parallel-for-min-funcs=0 -lto-inline-cost < %s -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-CL
; RUN: opt -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='cgscc(inline)' -inline-report=0xe886 -inline-threshold=0 -inline-tbb-parallel-for-min-funcs=0 -lto-inline-cost -S | opt -passes='inlinereportemitter' -inline-report=0xe886 -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-MD

; Show that ?relax@@YAXPEAH00H@Z is inlined because it is selected as a
; candidate using the "TBB parallel for" inlining heuristic on Windows.

; CHECK-CL: define {{.*}} @"?execute@?$start_for@V?$blocked_range@H@d1@detail@tbb@@VArraySummer@@$$CBVauto_partitioner@234@@d1@detail@tbb@@UEAAPEAVtask@234@AEAUexecution_data@234@@Z"
; CHECK-CL: call {{.*}} @"?delta_stepping@@YAXPEAH00H@Z"
; CHECK-CL: call {{.*}} @"?delta_stepping@@YAXPEAH00H@Z"

; CHECK-CL: define {{.*}} @"?delta_stepping@@YAXPEAH00H@Z"
; CHECK-CL-NOT: call {{.*}} @?relax@@YAXPEAH00H@Z

; CHECK-CL: COMPILE FUNC: ?delta_stepping@@YAXPEAH00H@Z
; CHECK-CL: INLINE: ?relax@@YAXPEAH00H@Z {{.*}}Under TBB parallel for
; CHECK-CL: INLINE: ?relax@@YAXPEAH00H@Z {{.*}}Under TBB parallel for
; CHECK-CL: INLINE: ?relax@@YAXPEAH00H@Z {{.*}}Callee has single callsite and local linkage

; CHECK: COMPILE FUNC: ?execute@?$start_for@V?$blocked_range@H@d1@detail@tbb@@VArraySummer@@$$CBVauto_partitioner@234@@d1@detail@tbb@@UEAAPEAVtask@234@AEAUexecution_data@234@@Z
; CHECK: ?delta_stepping@@YAXPEAH00H@Z {{.*}}Inlining is not profitable
; CHECK: ?delta_stepping@@YAXPEAH00H@Z {{.*}}Inlining is not profitable

; CHECK-MD: COMPILE FUNC: ?delta_stepping@@YAXPEAH00H@Z
; CHECK-MD: INLINE: ?relax@@YAXPEAH00H@Z {{.*}}Under TBB parallel for
; CHECK-MD: INLINE: ?relax@@YAXPEAH00H@Z {{.*}}Under TBB parallel for
; CHECK-MD: INLINE: ?relax@@YAXPEAH00H@Z {{.*}}Callee has single callsite and local linkage

; CHECK-MD: define {{.*}} @"?execute@?$start_for@V?$blocked_range@H@d1@detail@tbb@@VArraySummer@@$$CBVauto_partitioner@234@@d1@detail@tbb@@UEAAPEAVtask@234@AEAUexecution_data@234@@Z"
; CHECK-MD: call {{.*}} @"?delta_stepping@@YAXPEAH00H@Z"
; CHECK-MD: call {{.*}} @"?delta_stepping@@YAXPEAH00H@Z"

; CHECK-MD: define {{.*}} @"?delta_stepping@@YAXPEAH00H@Z"
; CHECK-MD-NOT: call {{.*}} @?relax@@YAXPEAH00H@Z

target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc19.29.30133"

%rtti.CompleteObjectLocator = type { i32, i32, i32, i32, i32, i32 }
%rtti.TypeDescriptor106 = type { ptr, ptr, [107 x i8] }
%rtti.ClassHierarchyDescriptor = type { i32, i32, i32, i32 }
%rtti.BaseClassDescriptor = type { i32, i32, i32, i32, i32, i32, i32 }
%rtti.TypeDescriptor24 = type { ptr, ptr, [25 x i8] }
%rtti.TypeDescriptor31 = type { ptr, ptr, [32 x i8] }
%"class.tbb::detail::d1::small_object_allocator" = type { ptr }
%"class.tbb::detail::d1::range_vector" = type { i8, i8, i8, [8 x i8], [5 x i8], %"class.tbb::detail::d0::aligned_space" }
%"class.tbb::detail::d0::aligned_space" = type { [128 x i8] }
%"struct.tbb::detail::d1::execution_data" = type { ptr, i16, i16 }
%"struct.tbb::detail::d1::start_for" = type { %"class.tbb::detail::d1::task", %"class.tbb::detail::d1::blocked_range", %class.ArraySummer, ptr, %"class.tbb::detail::d1::auto_partition_type", %"class.tbb::detail::d1::small_object_allocator", [56 x i8] }
%"class.tbb::detail::d1::task" = type { ptr, %"class.tbb::detail::d1::task_traits", [6 x i64] }
%"class.tbb::detail::d1::task_traits" = type { i64 }
%"class.tbb::detail::d1::blocked_range" = type { i32, i32, i64 }
%class.ArraySummer = type { ptr, ptr, ptr }
%"class.tbb::detail::d1::auto_partition_type" = type { %"struct.tbb::detail::d1::dynamic_grainsize_mode" }
%"struct.tbb::detail::d1::dynamic_grainsize_mode" = type { %"struct.tbb::detail::d1::adaptive_mode", i32, i8 }
%"struct.tbb::detail::d1::adaptive_mode" = type { i64 }
%"struct.tbb::detail::d1::node" = type { ptr, %"struct.std::atomic.11" }
%"struct.std::atomic.11" = type { %"struct.std::_Atomic_integral_facade.12" }
%"struct.std::_Atomic_integral_facade.12" = type { %"struct.std::_Atomic_integral.13" }
%"struct.std::_Atomic_integral.13" = type { %"struct.std::_Atomic_storage.14" }
%"struct.std::_Atomic_storage.14" = type { %"struct.std::_Atomic_padded.15" }
%"struct.std::_Atomic_padded.15" = type { i32 }
%"struct.tbb::detail::d1::tree_node" = type { %"struct.tbb::detail::d1::node", %"class.tbb::detail::d1::small_object_allocator", %"struct.std::atomic.28" }
%"struct.std::atomic.28" = type { %"struct.std::_Atomic_storage.29" }
%"struct.std::_Atomic_storage.29" = type { %"struct.std::_Atomic_padded.30" }
%"struct.std::_Atomic_padded.30" = type { i8 }
%"class.tbb::detail::d1::task_group_context" = type { i64, %"struct.std::atomic", i8, %"struct.tbb::detail::d1::task_group_context::context_traits", %"struct.std::atomic.0", %"struct.std::atomic.5", %union.anon, ptr, %"struct.tbb::detail::d1::intrusive_list_node", %"struct.std::atomic.8", ptr, i64, [56 x i8] }
%"struct.std::atomic" = type { %"struct.std::_Atomic_integral_facade" }
%"struct.std::_Atomic_integral_facade" = type { %"struct.std::_Atomic_integral" }
%"struct.std::_Atomic_integral" = type { %"struct.std::_Atomic_storage" }
%"struct.std::_Atomic_storage" = type { %"struct.std::_Atomic_padded" }
%"struct.std::_Atomic_padded" = type { i32 }
%"struct.tbb::detail::d1::task_group_context::context_traits" = type { i8 }
%"struct.std::atomic.0" = type { %"struct.std::_Atomic_integral_facade.1" }
%"struct.std::_Atomic_integral_facade.1" = type { %"struct.std::_Atomic_integral.2" }
%"struct.std::_Atomic_integral.2" = type { %"struct.std::_Atomic_storage.3" }
%"struct.std::_Atomic_storage.3" = type { %"struct.std::_Atomic_padded.4" }
%"struct.std::_Atomic_padded.4" = type { i8 }
%"struct.std::atomic.5" = type { %"struct.std::_Atomic_storage.6" }
%"struct.std::_Atomic_storage.6" = type { %"struct.std::_Atomic_padded.7" }
%"struct.std::_Atomic_padded.7" = type { i8 }
%union.anon = type { ptr }
%"struct.tbb::detail::d1::intrusive_list_node" = type { ptr, ptr }
%"struct.std::atomic.8" = type { %"struct.std::_Atomic_pointer" }
%"struct.std::_Atomic_pointer" = type { %"struct.std::_Atomic_storage.9" }
%"struct.std::_Atomic_storage.9" = type { %"struct.std::_Atomic_padded.10" }
%"struct.std::_Atomic_padded.10" = type { ptr }

$"??_G?$start_for@V?$blocked_range@H@d1@detail@tbb@@VArraySummer@@$$CBVauto_partitioner@234@@d1@detail@tbb@@UEAAPEAXI@Z" = comdat any

$"?execute@?$start_for@V?$blocked_range@H@d1@detail@tbb@@VArraySummer@@$$CBVauto_partitioner@234@@d1@detail@tbb@@UEAAPEAVtask@234@AEAUexecution_data@234@@Z" = comdat any

$"?cancel@?$start_for@V?$blocked_range@H@d1@detail@tbb@@VArraySummer@@$$CBVauto_partitioner@234@@d1@detail@tbb@@UEAAPEAVtask@234@AEAUexecution_data@234@@Z" = comdat any

$"??_Gtask@d1@detail@tbb@@MEAAPEAXI@Z" = comdat any

$"??_7?$start_for@V?$blocked_range@H@d1@detail@tbb@@VArraySummer@@$$CBVauto_partitioner@234@@d1@detail@tbb@@6B@" = comdat largest

$"??_R4?$start_for@V?$blocked_range@H@d1@detail@tbb@@VArraySummer@@$$CBVauto_partitioner@234@@d1@detail@tbb@@6B@" = comdat any

$"??_R0?AU?$start_for@V?$blocked_range@H@d1@detail@tbb@@VArraySummer@@$$CBVauto_partitioner@234@@d1@detail@tbb@@@8" = comdat any

$"??_R3?$start_for@V?$blocked_range@H@d1@detail@tbb@@VArraySummer@@$$CBVauto_partitioner@234@@d1@detail@tbb@@8" = comdat any

$"??_R2?$start_for@V?$blocked_range@H@d1@detail@tbb@@VArraySummer@@$$CBVauto_partitioner@234@@d1@detail@tbb@@8" = comdat any

$"??_R1A@?0A@EA@?$start_for@V?$blocked_range@H@d1@detail@tbb@@VArraySummer@@$$CBVauto_partitioner@234@@d1@detail@tbb@@8" = comdat any

$"??_R1A@?0A@EA@task@d1@detail@tbb@@8" = comdat any

$"??_R0?AVtask@d1@detail@tbb@@@8" = comdat any

$"??_R3task@d1@detail@tbb@@8" = comdat any

$"??_R2task@d1@detail@tbb@@8" = comdat any

$"??_R17?0A@EA@task_traits@d1@detail@tbb@@8" = comdat any

$"??_R0?AVtask_traits@d1@detail@tbb@@@8" = comdat any

$"??_R3task_traits@d1@detail@tbb@@8" = comdat any

$"??_R2task_traits@d1@detail@tbb@@8" = comdat any

$"??_R1A@?0A@EA@task_traits@d1@detail@tbb@@8" = comdat any

$"??_7task@d1@detail@tbb@@6B@" = comdat largest

$"??_R4task@d1@detail@tbb@@6B@" = comdat any

@0 = private unnamed_addr constant { [4 x ptr] } { [4 x ptr] [ptr @"??_R4?$start_for@V?$blocked_range@H@d1@detail@tbb@@VArraySummer@@$$CBVauto_partitioner@234@@d1@detail@tbb@@6B@", ptr @"??_G?$start_for@V?$blocked_range@H@d1@detail@tbb@@VArraySummer@@$$CBVauto_partitioner@234@@d1@detail@tbb@@UEAAPEAXI@Z", ptr @"?execute@?$start_for@V?$blocked_range@H@d1@detail@tbb@@VArraySummer@@$$CBVauto_partitioner@234@@d1@detail@tbb@@UEAAPEAVtask@234@AEAUexecution_data@234@@Z", ptr @"?cancel@?$start_for@V?$blocked_range@H@d1@detail@tbb@@VArraySummer@@$$CBVauto_partitioner@234@@d1@detail@tbb@@UEAAPEAVtask@234@AEAUexecution_data@234@@Z"] }, comdat($"??_7?$start_for@V?$blocked_range@H@d1@detail@tbb@@VArraySummer@@$$CBVauto_partitioner@234@@d1@detail@tbb@@6B@")
@"??_R4?$start_for@V?$blocked_range@H@d1@detail@tbb@@VArraySummer@@$$CBVauto_partitioner@234@@d1@detail@tbb@@6B@" = linkonce_odr constant %rtti.CompleteObjectLocator { i32 1, i32 0, i32 0, i32 trunc (i64 sub nuw nsw (i64 ptrtoint (ptr @"??_R0?AU?$start_for@V?$blocked_range@H@d1@detail@tbb@@VArraySummer@@$$CBVauto_partitioner@234@@d1@detail@tbb@@@8" to i64), i64 ptrtoint (ptr @__ImageBase to i64)) to i32), i32 trunc (i64 sub nuw nsw (i64 ptrtoint (ptr @"??_R3?$start_for@V?$blocked_range@H@d1@detail@tbb@@VArraySummer@@$$CBVauto_partitioner@234@@d1@detail@tbb@@8" to i64), i64 ptrtoint (ptr @__ImageBase to i64)) to i32), i32 trunc (i64 sub nuw nsw (i64 ptrtoint (ptr @"??_R4?$start_for@V?$blocked_range@H@d1@detail@tbb@@VArraySummer@@$$CBVauto_partitioner@234@@d1@detail@tbb@@6B@" to i64), i64 ptrtoint (ptr @__ImageBase to i64)) to i32) }, comdat
@"??_7type_info@@6B@" = external constant ptr
@"??_R0?AU?$start_for@V?$blocked_range@H@d1@detail@tbb@@VArraySummer@@$$CBVauto_partitioner@234@@d1@detail@tbb@@@8" = linkonce_odr global %rtti.TypeDescriptor106 { ptr @"??_7type_info@@6B@", ptr null, [107 x i8] c".?AU?$start_for@V?$blocked_range@H@d1@detail@tbb@@VArraySummer@@$$CBVauto_partitioner@234@@d1@detail@tbb@@\00" }, comdat
@__ImageBase = external dso_local constant i8
@"??_R3?$start_for@V?$blocked_range@H@d1@detail@tbb@@VArraySummer@@$$CBVauto_partitioner@234@@d1@detail@tbb@@8" = linkonce_odr constant %rtti.ClassHierarchyDescriptor { i32 0, i32 0, i32 3, i32 trunc (i64 sub nuw nsw (i64 ptrtoint (ptr @"??_R2?$start_for@V?$blocked_range@H@d1@detail@tbb@@VArraySummer@@$$CBVauto_partitioner@234@@d1@detail@tbb@@8" to i64), i64 ptrtoint (ptr @__ImageBase to i64)) to i32) }, comdat
@"??_R2?$start_for@V?$blocked_range@H@d1@detail@tbb@@VArraySummer@@$$CBVauto_partitioner@234@@d1@detail@tbb@@8" = linkonce_odr constant [4 x i32] [i32 trunc (i64 sub nuw nsw (i64 ptrtoint (ptr @"??_R1A@?0A@EA@?$start_for@V?$blocked_range@H@d1@detail@tbb@@VArraySummer@@$$CBVauto_partitioner@234@@d1@detail@tbb@@8" to i64), i64 ptrtoint (ptr @__ImageBase to i64)) to i32), i32 trunc (i64 sub nuw nsw (i64 ptrtoint (ptr @"??_R1A@?0A@EA@task@d1@detail@tbb@@8" to i64), i64 ptrtoint (ptr @__ImageBase to i64)) to i32), i32 trunc (i64 sub nuw nsw (i64 ptrtoint (ptr @"??_R17?0A@EA@task_traits@d1@detail@tbb@@8" to i64), i64 ptrtoint (ptr @__ImageBase to i64)) to i32), i32 0], comdat
@"??_R1A@?0A@EA@?$start_for@V?$blocked_range@H@d1@detail@tbb@@VArraySummer@@$$CBVauto_partitioner@234@@d1@detail@tbb@@8" = linkonce_odr constant %rtti.BaseClassDescriptor { i32 trunc (i64 sub nuw nsw (i64 ptrtoint (ptr @"??_R0?AU?$start_for@V?$blocked_range@H@d1@detail@tbb@@VArraySummer@@$$CBVauto_partitioner@234@@d1@detail@tbb@@@8" to i64), i64 ptrtoint (ptr @__ImageBase to i64)) to i32), i32 2, i32 0, i32 -1, i32 0, i32 64, i32 trunc (i64 sub nuw nsw (i64 ptrtoint (ptr @"??_R3?$start_for@V?$blocked_range@H@d1@detail@tbb@@VArraySummer@@$$CBVauto_partitioner@234@@d1@detail@tbb@@8" to i64), i64 ptrtoint (ptr @__ImageBase to i64)) to i32) }, comdat
@"??_R1A@?0A@EA@task@d1@detail@tbb@@8" = linkonce_odr constant %rtti.BaseClassDescriptor { i32 trunc (i64 sub nuw nsw (i64 ptrtoint (ptr @"??_R0?AVtask@d1@detail@tbb@@@8" to i64), i64 ptrtoint (ptr @__ImageBase to i64)) to i32), i32 1, i32 0, i32 -1, i32 0, i32 64, i32 trunc (i64 sub nuw nsw (i64 ptrtoint (ptr @"??_R3task@d1@detail@tbb@@8" to i64), i64 ptrtoint (ptr @__ImageBase to i64)) to i32) }, comdat
@"??_R0?AVtask@d1@detail@tbb@@@8" = linkonce_odr global %rtti.TypeDescriptor24 { ptr @"??_7type_info@@6B@", ptr null, [25 x i8] c".?AVtask@d1@detail@tbb@@\00" }, comdat
@"??_R3task@d1@detail@tbb@@8" = linkonce_odr constant %rtti.ClassHierarchyDescriptor { i32 0, i32 0, i32 2, i32 trunc (i64 sub nuw nsw (i64 ptrtoint (ptr @"??_R2task@d1@detail@tbb@@8" to i64), i64 ptrtoint (ptr @__ImageBase to i64)) to i32) }, comdat
@"??_R2task@d1@detail@tbb@@8" = linkonce_odr constant [3 x i32] [i32 trunc (i64 sub nuw nsw (i64 ptrtoint (ptr @"??_R1A@?0A@EA@task@d1@detail@tbb@@8" to i64), i64 ptrtoint (ptr @__ImageBase to i64)) to i32), i32 trunc (i64 sub nuw nsw (i64 ptrtoint (ptr @"??_R17?0A@EA@task_traits@d1@detail@tbb@@8" to i64), i64 ptrtoint (ptr @__ImageBase to i64)) to i32), i32 0], comdat
@"??_R17?0A@EA@task_traits@d1@detail@tbb@@8" = linkonce_odr constant %rtti.BaseClassDescriptor { i32 trunc (i64 sub nuw nsw (i64 ptrtoint (ptr @"??_R0?AVtask_traits@d1@detail@tbb@@@8" to i64), i64 ptrtoint (ptr @__ImageBase to i64)) to i32), i32 0, i32 8, i32 -1, i32 0, i32 64, i32 trunc (i64 sub nuw nsw (i64 ptrtoint (ptr @"??_R3task_traits@d1@detail@tbb@@8" to i64), i64 ptrtoint (ptr @__ImageBase to i64)) to i32) }, comdat
@"??_R0?AVtask_traits@d1@detail@tbb@@@8" = linkonce_odr global %rtti.TypeDescriptor31 { ptr @"??_7type_info@@6B@", ptr null, [32 x i8] c".?AVtask_traits@d1@detail@tbb@@\00" }, comdat
@"??_R3task_traits@d1@detail@tbb@@8" = linkonce_odr constant %rtti.ClassHierarchyDescriptor { i32 0, i32 0, i32 1, i32 trunc (i64 sub nuw nsw (i64 ptrtoint (ptr @"??_R2task_traits@d1@detail@tbb@@8" to i64), i64 ptrtoint (ptr @__ImageBase to i64)) to i32) }, comdat
@"??_R2task_traits@d1@detail@tbb@@8" = linkonce_odr constant [2 x i32] [i32 trunc (i64 sub nuw nsw (i64 ptrtoint (ptr @"??_R1A@?0A@EA@task_traits@d1@detail@tbb@@8" to i64), i64 ptrtoint (ptr @__ImageBase to i64)) to i32), i32 0], comdat
@"??_R1A@?0A@EA@task_traits@d1@detail@tbb@@8" = linkonce_odr constant %rtti.BaseClassDescriptor { i32 trunc (i64 sub nuw nsw (i64 ptrtoint (ptr @"??_R0?AVtask_traits@d1@detail@tbb@@@8" to i64), i64 ptrtoint (ptr @__ImageBase to i64)) to i32), i32 0, i32 0, i32 -1, i32 0, i32 64, i32 trunc (i64 sub nuw nsw (i64 ptrtoint (ptr @"??_R3task_traits@d1@detail@tbb@@8" to i64), i64 ptrtoint (ptr @__ImageBase to i64)) to i32) }, comdat
@1 = private unnamed_addr constant { [4 x ptr] } { [4 x ptr] [ptr @"??_R4task@d1@detail@tbb@@6B@", ptr @"??_Gtask@d1@detail@tbb@@MEAAPEAXI@Z", ptr @_purecall, ptr @_purecall] }, comdat($"??_7task@d1@detail@tbb@@6B@")
@"??_R4task@d1@detail@tbb@@6B@" = linkonce_odr constant %rtti.CompleteObjectLocator { i32 1, i32 0, i32 0, i32 trunc (i64 sub nuw nsw (i64 ptrtoint (ptr @"??_R0?AVtask@d1@detail@tbb@@@8" to i64), i64 ptrtoint (ptr @__ImageBase to i64)) to i32), i32 trunc (i64 sub nuw nsw (i64 ptrtoint (ptr @"??_R3task@d1@detail@tbb@@8" to i64), i64 ptrtoint (ptr @__ImageBase to i64)) to i32), i32 trunc (i64 sub nuw nsw (i64 ptrtoint (ptr @"??_R4task@d1@detail@tbb@@6B@" to i64), i64 ptrtoint (ptr @__ImageBase to i64)) to i32) }, comdat

@"??_7?$start_for@V?$blocked_range@H@d1@detail@tbb@@VArraySummer@@$$CBVauto_partitioner@234@@d1@detail@tbb@@6B@" = unnamed_addr alias ptr, getelementptr inbounds ({ [4 x ptr] }, ptr @0, i32 0, i32 0, i32 1)
@"??_7task@d1@detail@tbb@@6B@" = unnamed_addr alias ptr, getelementptr inbounds ({ [4 x ptr] }, ptr @1, i32 0, i32 0, i32 1)

declare dso_local noundef nonnull ptr @"??_U@YAPEAX_K@Z"(i64 noundef)

declare dso_local void @"?initialize@r1@detail@tbb@@YAXAEAVtask_group_context@d1@23@@Z"(ptr noundef nonnull align 8 dereferenceable(128))

declare dso_local noundef ptr @"?allocate@r1@detail@tbb@@YAPEAXAEAPEAVsmall_object_pool@d1@23@_K@Z"(ptr noundef nonnull align 8 dereferenceable(8), i64 noundef)

define linkonce_odr dso_local noundef ptr @"??_G?$start_for@V?$blocked_range@H@d1@detail@tbb@@VArraySummer@@$$CBVauto_partitioner@234@@d1@detail@tbb@@UEAAPEAXI@Z"(ptr noundef nonnull align 64 dereferenceable(144) %this, i32 noundef %should_call_delete) comdat align 2 {
entry:
  %i = icmp eq i32 %should_call_delete, 0
  br i1 %i, label %dtor.continue, label %dtor.call_delete

dtor.call_delete:                                 ; preds = %entry
  %i1 = bitcast ptr %this to ptr
  tail call void @"??3@YAXPEAX@Z"(ptr noundef nonnull %i1)
  br label %dtor.continue

dtor.continue:                                    ; preds = %dtor.call_delete, %entry
  %i2 = bitcast ptr %this to ptr
  ret ptr %i2
}

define linkonce_odr dso_local noundef ptr @"?execute@?$start_for@V?$blocked_range@H@d1@detail@tbb@@VArraySummer@@$$CBVauto_partitioner@234@@d1@detail@tbb@@UEAAPEAVtask@234@AEAUexecution_data@234@@Z"(ptr noundef nonnull align 64 dereferenceable(144) %this, ptr noundef nonnull align 8 dereferenceable(16) %ed) comdat align 2 {
entry:
  %alloc.i.i.i.i = alloca %"class.tbb::detail::d1::small_object_allocator", align 8
  %range_pool.i.i = alloca %"class.tbb::detail::d1::range_vector", align 8
  %alloc.i.i.i = alloca %"class.tbb::detail::d1::small_object_allocator", align 8
  %affinity_slot.i.i = getelementptr inbounds %"struct.tbb::detail::d1::execution_data", ptr %ed, i64 0, i32 2, !intel-tbaa !17
  %i = load i16, ptr %affinity_slot.i.i, align 2, !tbaa !17
  %cmp.i = icmp eq i16 %i, -1
  br i1 %cmp.i, label %if.end, label %"?is_same_affinity@d1@detail@tbb@@YA_NAEBUexecution_data@123@@Z.exit"

"?is_same_affinity@d1@detail@tbb@@YA_NAEBUexecution_data@123@@Z.exit": ; preds = %entry
  %call.i.i = tail call noundef i16 @"?execution_slot@r1@detail@tbb@@YAGPEBUexecution_data@d1@23@@Z"(ptr noundef nonnull %ed)
  %cmp5.i = icmp eq i16 %i, %call.i.i
  br i1 %cmp5.i, label %if.end, label %if.then

if.then:                                          ; preds = %"?is_same_affinity@d1@detail@tbb@@YA_NAEBUexecution_data@123@@Z.exit"
  %call.i = tail call noundef i16 @"?execution_slot@r1@detail@tbb@@YAGPEBUexecution_data@d1@23@@Z"(ptr noundef nonnull %ed)
  br label %if.end

if.end:                                           ; preds = %if.then, %"?is_same_affinity@d1@detail@tbb@@YA_NAEBUexecution_data@123@@Z.exit", %entry
  %my_divisor.i = getelementptr inbounds %"struct.tbb::detail::d1::start_for", ptr %this, i64 0, i32 4, i32 0, i32 0, i32 0
  %i1 = load i64, ptr %my_divisor.i, align 16, !tbaa !23
  %tobool.not.i = icmp eq i64 %i1, 0
  br i1 %tobool.not.i, label %if.then.i, label %"??$check_being_stolen@U?$start_for@V?$blocked_range@H@d1@detail@tbb@@VArraySummer@@$$CBVauto_partitioner@234@@d1@detail@tbb@@@?$dynamic_grainsize_mode@U?$adaptive_mode@Vauto_partition_type@d1@detail@tbb@@@d1@detail@tbb@@@d1@detail@tbb@@QEAA_NAEAU?$start_for@V?$blocked_range@H@d1@detail@tbb@@VArraySummer@@$$CBVauto_partitioner@234@@123@AEBUexecution_data@123@@Z.exit"

if.then.i:                                        ; preds = %if.end
  %i2 = bitcast ptr %this to ptr
  %sunkaddr = getelementptr inbounds i8, ptr %i2, i64 112
  %i3 = bitcast ptr %sunkaddr to ptr
  store i64 1, ptr %i3, align 16, !tbaa !23
  %call.i.i.i = tail call noundef i16 @"?execution_slot@r1@detail@tbb@@YAGPEBUexecution_data@d1@23@@Z"(ptr noundef nonnull %ed)
  %original_slot.i.i.i = getelementptr inbounds %"struct.tbb::detail::d1::execution_data", ptr %ed, i64 0, i32 1, !intel-tbaa !26
  %i4 = load i16, ptr %original_slot.i.i.i, align 8, !tbaa !26
  %cmp.i.not.i = icmp eq i16 %call.i.i.i, %i4
  br i1 %cmp.i.not.i, label %"??$check_being_stolen@U?$start_for@V?$blocked_range@H@d1@detail@tbb@@VArraySummer@@$$CBVauto_partitioner@234@@d1@detail@tbb@@@?$dynamic_grainsize_mode@U?$adaptive_mode@Vauto_partition_type@d1@detail@tbb@@@d1@detail@tbb@@@d1@detail@tbb@@QEAA_NAEAU?$start_for@V?$blocked_range@H@d1@detail@tbb@@VArraySummer@@$$CBVauto_partitioner@234@@123@AEBUexecution_data@123@@Z.exit", label %land.lhs.true.i

land.lhs.true.i:                                  ; preds = %if.then.i
  %my_parent.i = getelementptr inbounds %"struct.tbb::detail::d1::start_for", ptr %this, i64 0, i32 3, !intel-tbaa !27
  %i5 = load ptr, ptr %my_parent.i, align 8, !tbaa !27
  %i6 = getelementptr inbounds %"struct.tbb::detail::d1::node", ptr %i5, i64 0, i32 1, i32 0, i32 0, i32 0, i32 0, i32 0
  %i7 = load volatile i32, ptr %i6, align 4
  fence syncscope("singlethread") seq_cst
  %cmp.i10 = icmp sgt i32 %i7, 1
  br i1 %cmp.i10, label %if.then6.i, label %"??$check_being_stolen@U?$start_for@V?$blocked_range@H@d1@detail@tbb@@VArraySummer@@$$CBVauto_partitioner@234@@d1@detail@tbb@@@?$dynamic_grainsize_mode@U?$adaptive_mode@Vauto_partition_type@d1@detail@tbb@@@d1@detail@tbb@@@d1@detail@tbb@@QEAA_NAEAU?$start_for@V?$blocked_range@H@d1@detail@tbb@@VArraySummer@@$$CBVauto_partitioner@234@@123@AEBUexecution_data@123@@Z.exit"

if.then6.i:                                       ; preds = %land.lhs.true.i
  %i8 = bitcast ptr %this to ptr
  %sunkaddr54 = getelementptr inbounds i8, ptr %i8, i64 104
  %i9 = bitcast ptr %sunkaddr54 to ptr
  %i10 = load ptr, ptr %i9, align 8, !tbaa !27
  %i11 = getelementptr inbounds %"struct.tbb::detail::d1::tree_node", ptr %i10, i64 0, i32 2, i32 0, i32 0, i32 0
  store volatile i8 1, ptr %i11, align 1
  %my_max_depth.i = getelementptr inbounds %"struct.tbb::detail::d1::start_for", ptr %this, i64 0, i32 4, i32 0, i32 2
  %i12 = load i8, ptr %my_max_depth.i, align 4, !tbaa !37
  %tobool7.not.i = icmp eq i8 %i12, 0
  %.op.i = add i8 %i12, 1
  %add.i = select i1 %tobool7.not.i, i8 2, i8 %.op.i
  store i8 %add.i, ptr %my_max_depth.i, align 4, !tbaa !37
  br label %"??$check_being_stolen@U?$start_for@V?$blocked_range@H@d1@detail@tbb@@VArraySummer@@$$CBVauto_partitioner@234@@d1@detail@tbb@@@?$dynamic_grainsize_mode@U?$adaptive_mode@Vauto_partition_type@d1@detail@tbb@@@d1@detail@tbb@@@d1@detail@tbb@@QEAA_NAEAU?$start_for@V?$blocked_range@H@d1@detail@tbb@@VArraySummer@@$$CBVauto_partitioner@234@@123@AEBUexecution_data@123@@Z.exit"

"??$check_being_stolen@U?$start_for@V?$blocked_range@H@d1@detail@tbb@@VArraySummer@@$$CBVauto_partitioner@234@@d1@detail@tbb@@@?$dynamic_grainsize_mode@U?$adaptive_mode@Vauto_partition_type@d1@detail@tbb@@@d1@detail@tbb@@@d1@detail@tbb@@QEAA_NAEAU?$start_for@V?$blocked_range@H@d1@detail@tbb@@VArraySummer@@$$CBVauto_partitioner@234@@123@AEBUexecution_data@123@@Z.exit": ; preds = %if.then6.i, %land.lhs.true.i, %if.then.i, %if.end
  %my_range = getelementptr inbounds %"struct.tbb::detail::d1::start_for", ptr %this, i64 0, i32 1, !intel-tbaa !40
  %my_grainsize.i.i = getelementptr inbounds %"struct.tbb::detail::d1::start_for", ptr %this, i64 0, i32 1, i32 2
  %i13 = load i64, ptr %my_grainsize.i.i, align 8, !tbaa !41
  %my_end.i.i.i55 = bitcast ptr %my_range to ptr
  %i14 = load i32, ptr %my_end.i.i.i55, align 64, !tbaa !42
  %my_begin.i.i.i = getelementptr inbounds %"struct.tbb::detail::d1::start_for", ptr %this, i64 0, i32 1, i32 1
  %i15 = load i32, ptr %my_begin.i.i.i, align 4, !tbaa !43
  %sub.i.i.i = sub nsw i32 %i14, %i15
  %conv.i.i.i = sext i32 %sub.i.i.i to i64
  %cmp.i.i = icmp ult i64 %i13, %conv.i.i.i
  br i1 %cmp.i.i, label %if.then.i11, label %if.end9.i

if.then.i11:                                      ; preds = %"??$check_being_stolen@U?$start_for@V?$blocked_range@H@d1@detail@tbb@@VArraySummer@@$$CBVauto_partitioner@234@@d1@detail@tbb@@@?$dynamic_grainsize_mode@U?$adaptive_mode@Vauto_partition_type@d1@detail@tbb@@@d1@detail@tbb@@@d1@detail@tbb@@QEAA_NAEAU?$start_for@V?$blocked_range@H@d1@detail@tbb@@VArraySummer@@$$CBVauto_partitioner@234@@123@AEBUexecution_data@123@@Z.exit"
  %i16 = bitcast ptr %this to ptr
  %sunkaddr56 = getelementptr inbounds i8, ptr %i16, i64 112
  %i17 = bitcast ptr %sunkaddr56 to ptr
  %i18 = load i64, ptr %i17, align 16, !tbaa !23
  %cmp.i15.i = icmp ugt i64 %i18, 1
  br i1 %cmp.i15.i, label %"?is_divisible@auto_partition_type@d1@detail@tbb@@QEAA_NXZ.exit.i", label %if.end.i.i

if.end.i.i:                                       ; preds = %if.then.i11
  %tobool.not.i.i = icmp eq i64 %i18, 0
  br i1 %tobool.not.i.i, label %if.end9.i, label %land.lhs.true.i.i

land.lhs.true.i.i:                                ; preds = %if.end.i.i
  %i19 = getelementptr inbounds %"struct.tbb::detail::d1::start_for", ptr %this, i64 0, i32 4, i32 0, i32 2
  %i20 = load i8, ptr %i19, align 4, !tbaa !37
  %tobool3.not.i.i = icmp eq i8 %i20, 0
  br i1 %tobool3.not.i.i, label %if.end9.i, label %if.then4.i.i

if.then4.i.i:                                     ; preds = %land.lhs.true.i.i
  %dec.i.i = add i8 %i20, -1
  %i21 = bitcast ptr %this to ptr
  %sunkaddr57 = getelementptr inbounds i8, ptr %i21, i64 124
  store i8 %dec.i.i, ptr %sunkaddr57, align 4, !tbaa !37
  %i22 = bitcast ptr %this to ptr
  %sunkaddr58 = getelementptr inbounds i8, ptr %i22, i64 112
  %i23 = bitcast ptr %sunkaddr58 to ptr
  store i64 0, ptr %i23, align 16, !tbaa !23
  br label %"?is_divisible@auto_partition_type@d1@detail@tbb@@QEAA_NXZ.exit.i"

"?is_divisible@auto_partition_type@d1@detail@tbb@@QEAA_NXZ.exit.i": ; preds = %if.then4.i.i, %if.then.i11
  %my_body5.i.i.i.i.i = getelementptr inbounds %"struct.tbb::detail::d1::start_for", ptr %this, i64 0, i32 2
  br label %do.body.i

do.body.i:                                        ; preds = %if.then4.i30.i, %land.rhs.i, %"?is_divisible@auto_partition_type@d1@detail@tbb@@QEAA_NXZ.exit.i"
  %i24 = bitcast ptr %ed to ptr
  %i25 = bitcast ptr %my_body5.i.i.i.i.i to ptr
  %i26 = bitcast ptr %alloc.i.i.i to ptr
  %i27 = bitcast ptr %alloc.i.i.i to ptr
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %i27)
  store ptr null, ptr %i26, align 8, !tbaa !44
  %call.i.i.i.i = call noundef ptr @"?allocate@r1@detail@tbb@@YAPEAXAEAPEAVsmall_object_pool@d1@23@_KAEBUexecution_data@523@@Z"(ptr noundef nonnull align 8 dereferenceable(8) %i26, i64 noundef 192, ptr noundef nonnull align 8 dereferenceable(16) %ed)
  %m_version_and_traits.i.i.i.i.i.i.i = getelementptr inbounds i8, ptr %call.i.i.i.i, i64 8
  %arrayinit.end.i.i.i.i.i.i = getelementptr inbounds i8, ptr %call.i.i.i.i, i64 64
  %i28 = bitcast ptr %m_version_and_traits.i.i.i.i.i.i.i to ptr
  store <2 x i64> zeroinitializer, ptr %i28, align 8, !tbaa !45
  %i29 = getelementptr inbounds i8, ptr %call.i.i.i.i, i64 24
  %i30 = bitcast ptr %i29 to ptr
  store <2 x i64> zeroinitializer, ptr %i30, align 8, !tbaa !45
  %i31 = getelementptr inbounds i8, ptr %call.i.i.i.i, i64 40
  %i32 = bitcast ptr %i31 to ptr
  store <2 x i64> zeroinitializer, ptr %i32, align 8, !tbaa !45
  %i33 = getelementptr inbounds i8, ptr %call.i.i.i.i, i64 56
  %i34 = bitcast ptr %i33 to ptr
  store i64 0, ptr %i34, align 8, !tbaa !45
  %i35 = bitcast ptr %call.i.i.i.i to ptr
  store ptr @"??_7?$start_for@V?$blocked_range@H@d1@detail@tbb@@VArraySummer@@$$CBVauto_partitioner@234@@d1@detail@tbb@@6B@", ptr %i35, align 64, !tbaa !46
  %i36 = bitcast ptr %arrayinit.end.i.i.i.i.i.i to ptr
  %i37 = bitcast ptr %this to ptr
  %sunkaddr61 = getelementptr inbounds i8, ptr %i37, i64 64
  %i38 = bitcast ptr %sunkaddr61 to ptr
  %i39 = load i32, ptr %i38, align 64, !tbaa !42
  store i32 %i39, ptr %i36, align 64, !tbaa !42
  %my_begin.i.i.i.i.i.i = getelementptr inbounds i8, ptr %call.i.i.i.i, i64 68
  %i40 = bitcast ptr %my_begin.i.i.i.i.i.i to ptr
  %i41 = bitcast ptr %this to ptr
  %sunkaddr62 = getelementptr inbounds i8, ptr %i41, i64 68
  %i42 = bitcast ptr %sunkaddr62 to ptr
  %i43 = load i32, ptr %i42, align 4, !tbaa !43
  %i44 = load i32, ptr %i38, align 64, !tbaa !42
  %sub.i.i.i.i.i.i.i = sub nsw i32 %i44, %i43
  %div8.i.i.i.i.i.i.i = lshr i32 %sub.i.i.i.i.i.i.i, 1
  %add.i.i.i.i.i.i.i = add i32 %div8.i.i.i.i.i.i.i, %i43
  store i32 %add.i.i.i.i.i.i.i, ptr %i38, align 64, !tbaa !42
  store i32 %add.i.i.i.i.i.i.i, ptr %i40, align 4, !tbaa !43
  %my_grainsize.i.i.i.i.i.i = getelementptr inbounds i8, ptr %call.i.i.i.i, i64 72
  %i45 = bitcast ptr %my_grainsize.i.i.i.i.i.i to ptr
  %i46 = bitcast ptr %this to ptr
  %sunkaddr63 = getelementptr inbounds i8, ptr %i46, i64 72
  %i47 = bitcast ptr %sunkaddr63 to ptr
  %i48 = load i64, ptr %i47, align 8, !tbaa !41
  store i64 %i48, ptr %i45, align 8, !tbaa !41
  %my_body.i.i.i.i.i = getelementptr inbounds i8, ptr %call.i.i.i.i, i64 80
  call void @llvm.memcpy.p0.p0.i64(ptr noundef nonnull align 16 dereferenceable(24) %my_body.i.i.i.i.i, ptr noundef nonnull align 16 dereferenceable(24) %i25, i64 24, i1 false)
  %my_divisor.i.i.i.i.i.i.i.i = getelementptr inbounds i8, ptr %call.i.i.i.i, i64 112
  %i49 = bitcast ptr %my_divisor.i.i.i.i.i.i.i.i to ptr
  %i50 = bitcast ptr %this to ptr
  %sunkaddr64 = getelementptr inbounds i8, ptr %i50, i64 112
  %i51 = bitcast ptr %sunkaddr64 to ptr
  %i52 = load i64, ptr %i51, align 16, !tbaa !23
  %div2.i.i.i.i.i.i.i.i.i = lshr i64 %i52, 1
  store i64 %div2.i.i.i.i.i.i.i.i.i, ptr %i51, align 16, !tbaa !23
  store i64 %div2.i.i.i.i.i.i.i.i.i, ptr %i49, align 16, !tbaa !23
  %my_delay.i.i.i.i.i.i.i = getelementptr inbounds i8, ptr %call.i.i.i.i, i64 120
  %i53 = bitcast ptr %my_delay.i.i.i.i.i.i.i to ptr
  store i32 2, ptr %i53, align 8, !tbaa !48
  %i54 = getelementptr inbounds i8, ptr %call.i.i.i.i, i64 124
  %i55 = bitcast ptr %this to ptr
  %sunkaddr65 = getelementptr inbounds i8, ptr %i55, i64 124
  %i56 = load i8, ptr %sunkaddr65, align 4, !tbaa !37
  store i8 %i56, ptr %i54, align 4, !tbaa !37
  %i57 = getelementptr inbounds i8, ptr %call.i.i.i.i, i64 128
  %i58 = bitcast ptr %i57 to ptr
  %i59 = load ptr, ptr %i26, align 8, !tbaa !44
  store ptr %i59, ptr %i58, align 64, !tbaa !49
  %call.i14.i.i.i = call noundef ptr @"?allocate@r1@detail@tbb@@YAPEAXAEAPEAVsmall_object_pool@d1@23@_KAEBUexecution_data@523@@Z"(ptr noundef nonnull align 8 dereferenceable(8) %i26, i64 noundef 32, ptr noundef nonnull align 8 dereferenceable(16) %ed)
  %i60 = bitcast ptr %this to ptr
  %sunkaddr66 = getelementptr inbounds i8, ptr %i60, i64 104
  %i61 = bitcast ptr %sunkaddr66 to ptr
  %i62 = load ptr, ptr %i61, align 8, !tbaa !27
  %my_parent.i.i.i.i.i.i = bitcast ptr %call.i14.i.i.i to ptr
  store ptr %i62, ptr %my_parent.i.i.i.i.i.i, align 8, !tbaa !50
  %_Value2.i.i.i.i.i.i.i.i.i.i = getelementptr inbounds i8, ptr %call.i14.i.i.i, i64 8
  %i63 = bitcast ptr %_Value2.i.i.i.i.i.i.i.i.i.i to ptr
  store i32 2, ptr %i63, align 8, !tbaa !53
  %i64 = getelementptr inbounds i8, ptr %call.i14.i.i.i, i64 16
  %i65 = bitcast ptr %i64 to ptr
  %i66 = load ptr, ptr %i26, align 8, !tbaa !44
  store ptr %i66, ptr %i65, align 8, !tbaa !49
  %i67 = getelementptr inbounds i8, ptr %call.i14.i.i.i, i64 24
  store i8 0, ptr %i67, align 8, !tbaa !56
  %i68 = bitcast ptr %this to ptr
  %sunkaddr67 = getelementptr inbounds i8, ptr %i68, i64 104
  %i69 = bitcast ptr %sunkaddr67 to ptr
  store ptr %call.i14.i.i.i, ptr %i69, align 8, !tbaa !27
  %my_parent7.i.i.i = getelementptr inbounds i8, ptr %call.i.i.i.i, i64 104
  %i70 = bitcast ptr %my_parent7.i.i.i to ptr
  store ptr %call.i14.i.i.i, ptr %i70, align 8, !tbaa !27
  %i71 = load ptr, ptr %i24, align 8, !tbaa !60
  %i72 = bitcast ptr %call.i.i.i.i to ptr
  call void @"?spawn@r1@detail@tbb@@YAXAEAVtask@d1@23@AEAVtask_group_context@523@@Z"(ptr noundef nonnull align 64 dereferenceable(64) %i72, ptr noundef nonnull align 8 dereferenceable(128) %i71)
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %i27)
  %i73 = load i64, ptr %i47, align 8, !tbaa !41
  %i74 = bitcast ptr %this to ptr
  %sunkaddr68 = getelementptr inbounds i8, ptr %i74, i64 64
  %i75 = bitcast ptr %sunkaddr68 to ptr
  %i76 = load i32, ptr %i75, align 64, !tbaa !42
  %i77 = load i32, ptr %i42, align 4, !tbaa !43
  %sub.i.i19.i = sub nsw i32 %i76, %i77
  %conv.i.i20.i = sext i32 %sub.i.i19.i to i64
  %cmp.i21.i = icmp ult i64 %i73, %conv.i.i20.i
  br i1 %cmp.i21.i, label %land.rhs.i, label %if.end9.i

land.rhs.i:                                       ; preds = %do.body.i
  %i78 = bitcast ptr %this to ptr
  %sunkaddr69 = getelementptr inbounds i8, ptr %i78, i64 112
  %i79 = bitcast ptr %sunkaddr69 to ptr
  %i80 = load i64, ptr %i79, align 16, !tbaa !23
  %cmp.i23.i = icmp ugt i64 %i80, 1
  br i1 %cmp.i23.i, label %do.body.i, label %if.end.i25.i

if.end.i25.i:                                     ; preds = %land.rhs.i
  %tobool.not.i24.i = icmp eq i64 %i80, 0
  br i1 %tobool.not.i24.i, label %if.end9.i, label %land.lhs.true.i28.i

land.lhs.true.i28.i:                              ; preds = %if.end.i25.i
  %i81 = bitcast ptr %this to ptr
  %sunkaddr70 = getelementptr inbounds i8, ptr %i81, i64 124
  %i82 = load i8, ptr %sunkaddr70, align 4, !tbaa !37
  %tobool3.not.i27.i = icmp eq i8 %i82, 0
  br i1 %tobool3.not.i27.i, label %if.end9.i, label %if.then4.i30.i

if.then4.i30.i:                                   ; preds = %land.lhs.true.i28.i
  %dec.i29.i = add i8 %i82, -1
  %i83 = bitcast ptr %this to ptr
  %sunkaddr71 = getelementptr inbounds i8, ptr %i83, i64 124
  store i8 %dec.i29.i, ptr %sunkaddr71, align 4, !tbaa !37
  %i84 = bitcast ptr %this to ptr
  %sunkaddr72 = getelementptr inbounds i8, ptr %i84, i64 112
  %i85 = bitcast ptr %sunkaddr72 to ptr
  store i64 0, ptr %i85, align 16, !tbaa !23
  br label %do.body.i

if.end9.i:                                        ; preds = %land.lhs.true.i28.i, %if.end.i25.i, %do.body.i, %land.lhs.true.i.i, %if.end.i.i, %"??$check_being_stolen@U?$start_for@V?$blocked_range@H@d1@detail@tbb@@VArraySummer@@$$CBVauto_partitioner@234@@d1@detail@tbb@@@?$dynamic_grainsize_mode@U?$adaptive_mode@Vauto_partition_type@d1@detail@tbb@@@d1@detail@tbb@@@d1@detail@tbb@@QEAA_NAEAU?$start_for@V?$blocked_range@H@d1@detail@tbb@@VArraySummer@@$$CBVauto_partitioner@234@@123@AEBUexecution_data@123@@Z.exit"
  %conv.i.i.i.pre-phi.i = phi i64 [ %conv.i.i.i, %if.end.i.i ], [ %conv.i.i.i, %land.lhs.true.i.i ], [ %conv.i.i.i, %"??$check_being_stolen@U?$start_for@V?$blocked_range@H@d1@detail@tbb@@VArraySummer@@$$CBVauto_partitioner@234@@d1@detail@tbb@@@?$dynamic_grainsize_mode@U?$adaptive_mode@Vauto_partition_type@d1@detail@tbb@@@d1@detail@tbb@@@d1@detail@tbb@@QEAA_NAEAU?$start_for@V?$blocked_range@H@d1@detail@tbb@@VArraySummer@@$$CBVauto_partitioner@234@@123@AEBUexecution_data@123@@Z.exit" ], [ %conv.i.i20.i, %do.body.i ], [ %conv.i.i20.i, %if.end.i25.i ], [ %conv.i.i20.i, %land.lhs.true.i28.i ]
  %i86 = phi i32 [ %i15, %if.end.i.i ], [ %i15, %land.lhs.true.i.i ], [ %i15, %"??$check_being_stolen@U?$start_for@V?$blocked_range@H@d1@detail@tbb@@VArraySummer@@$$CBVauto_partitioner@234@@d1@detail@tbb@@@?$dynamic_grainsize_mode@U?$adaptive_mode@Vauto_partition_type@d1@detail@tbb@@@d1@detail@tbb@@@d1@detail@tbb@@QEAA_NAEAU?$start_for@V?$blocked_range@H@d1@detail@tbb@@VArraySummer@@$$CBVauto_partitioner@234@@123@AEBUexecution_data@123@@Z.exit" ], [ %i77, %do.body.i ], [ %i77, %if.end.i25.i ], [ %i77, %land.lhs.true.i28.i ]
  %i87 = phi i32 [ %i14, %if.end.i.i ], [ %i14, %land.lhs.true.i.i ], [ %i14, %"??$check_being_stolen@U?$start_for@V?$blocked_range@H@d1@detail@tbb@@VArraySummer@@$$CBVauto_partitioner@234@@d1@detail@tbb@@@?$dynamic_grainsize_mode@U?$adaptive_mode@Vauto_partition_type@d1@detail@tbb@@@d1@detail@tbb@@@d1@detail@tbb@@QEAA_NAEAU?$start_for@V?$blocked_range@H@d1@detail@tbb@@VArraySummer@@$$CBVauto_partitioner@234@@123@AEBUexecution_data@123@@Z.exit" ], [ %i76, %do.body.i ], [ %i76, %if.end.i25.i ], [ %i76, %land.lhs.true.i28.i ]
  %i88 = phi i64 [ %i13, %if.end.i.i ], [ %i13, %land.lhs.true.i.i ], [ %i13, %"??$check_being_stolen@U?$start_for@V?$blocked_range@H@d1@detail@tbb@@VArraySummer@@$$CBVauto_partitioner@234@@d1@detail@tbb@@@?$dynamic_grainsize_mode@U?$adaptive_mode@Vauto_partition_type@d1@detail@tbb@@@d1@detail@tbb@@@d1@detail@tbb@@QEAA_NAEAU?$start_for@V?$blocked_range@H@d1@detail@tbb@@VArraySummer@@$$CBVauto_partitioner@234@@123@AEBUexecution_data@123@@Z.exit" ], [ %i73, %do.body.i ], [ %i73, %if.end.i25.i ], [ %i73, %land.lhs.true.i28.i ]
  %cmp.i.i.i = icmp ult i64 %i88, %conv.i.i.i.pre-phi.i
  br i1 %cmp.i.i.i, label %lor.lhs.false.i.i, label %if.then.i.i

lor.lhs.false.i.i:                                ; preds = %if.end9.i
  %i89 = getelementptr inbounds %"struct.tbb::detail::d1::start_for", ptr %this, i64 0, i32 4, i32 0, i32 2
  %i90 = load i8, ptr %i89, align 4, !tbaa !37
  %tobool.not.i33.i = icmp eq i8 %i90, 0
  br i1 %tobool.not.i33.i, label %if.then.i.i, label %if.else.i.i

if.then.i.i:                                      ; preds = %lor.lhs.false.i.i, %if.end9.i
  %cmp.not17.i.i.i.i = icmp eq i32 %i87, %i86
  br i1 %cmp.not17.i.i.i.i, label %"??$execute@U?$start_for@V?$blocked_range@H@d1@detail@tbb@@VArraySummer@@$$CBVauto_partitioner@234@@d1@detail@tbb@@V?$blocked_range@H@234@@?$partition_type_base@Vauto_partition_type@d1@detail@tbb@@@d1@detail@tbb@@QEAAXAEAU?$start_for@V?$blocked_range@H@d1@detail@tbb@@VArraySummer@@$$CBVauto_partitioner@234@@123@AEAV?$blocked_range@H@123@AEAUexecution_data@123@@Z.exit", label %for.body.lr.ph.i.i.i.i

for.body.lr.ph.i.i.i.i:                           ; preds = %if.then.i.i
  %p_array_sum.i.i.i.i = getelementptr inbounds %"struct.tbb::detail::d1::start_for", ptr %this, i64 0, i32 2, i32 2, !intel-tbaa !61
  %p_array_b.i.i.i.i = getelementptr inbounds %"struct.tbb::detail::d1::start_for", ptr %this, i64 0, i32 2, i32 1, !intel-tbaa !62
  %p_array_a.i.i.i.i = getelementptr inbounds %"struct.tbb::detail::d1::start_for", ptr %this, i64 0, i32 2, i32 0, !intel-tbaa !63
  %i91 = sext i32 %i86 to i64
  %.pre.i.i.i.i = load ptr, ptr %p_array_sum.i.i.i.i, align 32, !tbaa !61
  %.pre19.i.i.i.i = load ptr, ptr %p_array_b.i.i.i.i, align 8, !tbaa !62
  %.pre20.i.i.i.i = load ptr, ptr %p_array_a.i.i.i.i, align 16, !tbaa !63
  br label %for.body.i.i.i.i

for.body.i.i.i.i:                                 ; preds = %for.body.i.i.i.i, %for.body.lr.ph.i.i.i.i
  %i92 = phi ptr [ %.pre20.i.i.i.i, %for.body.lr.ph.i.i.i.i ], [ %i97, %for.body.i.i.i.i ]
  %i93 = phi ptr [ %.pre19.i.i.i.i, %for.body.lr.ph.i.i.i.i ], [ %i101, %for.body.i.i.i.i ]
  %i94 = phi ptr [ %.pre.i.i.i.i, %for.body.lr.ph.i.i.i.i ], [ %i105, %for.body.i.i.i.i ]
  %indvars.iv.i.i.i.i = phi i64 [ %i91, %for.body.lr.ph.i.i.i.i ], [ %indvars.iv.next.i.i.i.i, %for.body.i.i.i.i ]
  %lsr48 = trunc i64 %indvars.iv.i.i.i.i to i32
  call fastcc void @"?delta_stepping@@YAXPEAH00H@Z"(ptr noundef %i92, ptr noundef %i93, ptr noundef %i94, i32 noundef %lsr48)
  %i95 = bitcast ptr %this to ptr
  %sunkaddr73 = getelementptr inbounds i8, ptr %i95, i64 80
  %i96 = bitcast ptr %sunkaddr73 to ptr
  %i97 = load ptr, ptr %i96, align 16, !tbaa !63
  %scevgep47 = getelementptr i32, ptr %i97, i64 %indvars.iv.i.i.i.i
  %i98 = load i32, ptr %scevgep47, align 4, !tbaa !64
  %i99 = bitcast ptr %this to ptr
  %sunkaddr74 = getelementptr inbounds i8, ptr %i99, i64 88
  %i100 = bitcast ptr %sunkaddr74 to ptr
  %i101 = load ptr, ptr %i100, align 8, !tbaa !62
  %scevgep46 = getelementptr i32, ptr %i101, i64 %indvars.iv.i.i.i.i
  %i102 = load i32, ptr %scevgep46, align 4, !tbaa !64
  %add.i.i.i.i = add nsw i32 %i102, %i98
  %i103 = bitcast ptr %this to ptr
  %sunkaddr75 = getelementptr inbounds i8, ptr %i103, i64 96
  %i104 = bitcast ptr %sunkaddr75 to ptr
  %i105 = load ptr, ptr %i104, align 32, !tbaa !61
  %scevgep = getelementptr i32, ptr %i105, i64 %indvars.iv.i.i.i.i
  store i32 %add.i.i.i.i, ptr %scevgep, align 4, !tbaa !64
  %indvars.iv.next.i.i.i.i = add i64 %indvars.iv.i.i.i.i, 1
  %lsr = trunc i64 %indvars.iv.next.i.i.i.i to i32
  %i106 = bitcast ptr %this to ptr
  %sunkaddr76 = getelementptr inbounds i8, ptr %i106, i64 64
  %i107 = bitcast ptr %sunkaddr76 to ptr
  %i108 = load i32, ptr %i107, align 64, !tbaa !42
  %cmp.not.i.i.i.i = icmp eq i32 %lsr, %i108
  br i1 %cmp.not.i.i.i.i, label %"??$execute@U?$start_for@V?$blocked_range@H@d1@detail@tbb@@VArraySummer@@$$CBVauto_partitioner@234@@d1@detail@tbb@@V?$blocked_range@H@234@@?$partition_type_base@Vauto_partition_type@d1@detail@tbb@@@d1@detail@tbb@@QEAAXAEAU?$start_for@V?$blocked_range@H@d1@detail@tbb@@VArraySummer@@$$CBVauto_partitioner@234@@123@AEAV?$blocked_range@H@123@AEAUexecution_data@123@@Z.exit", label %for.body.i.i.i.i, !llvm.loop !65

if.else.i.i:                                      ; preds = %lor.lhs.false.i.i
  %i109 = bitcast ptr %range_pool.i.i to ptr
  call void @llvm.lifetime.start.p0(i64 144, ptr nonnull %i109)
  store i8 0, ptr %i109, align 8, !tbaa !67
  %my_tail.i.i.i = getelementptr inbounds %"class.tbb::detail::d1::range_vector", ptr %range_pool.i.i, i64 0, i32 1, !intel-tbaa !72
  store i8 0, ptr %my_tail.i.i.i, align 1, !tbaa !72
  %my_size.i.i.i = getelementptr inbounds %"class.tbb::detail::d1::range_vector", ptr %range_pool.i.i, i64 0, i32 2, !intel-tbaa !73
  store i8 1, ptr %my_size.i.i.i, align 2, !tbaa !73
  %arrayidx.i.i.i = getelementptr inbounds %"class.tbb::detail::d1::range_vector", ptr %range_pool.i.i, i64 0, i32 3, i64 0, !intel-tbaa !74
  store i8 0, ptr %arrayidx.i.i.i, align 1, !tbaa !74
  %i110 = getelementptr inbounds %"class.tbb::detail::d1::range_vector", ptr %range_pool.i.i, i64 0, i32 5, i32 0, i64 0
  %i111 = bitcast ptr %my_range to ptr
  call void @llvm.memcpy.p0.p0.i64(ptr noundef nonnull align 8 dereferenceable(16) %i110, ptr noundef nonnull align 64 dereferenceable(16) %i111, i64 16, i1 false)
  %my_pool.i.i.i.i.i = getelementptr inbounds %"class.tbb::detail::d1::range_vector", ptr %range_pool.i.i, i64 0, i32 5
  %my_body2.i.i.i.i.i.i = getelementptr inbounds %"struct.tbb::detail::d1::start_for", ptr %this, i64 0, i32 2
  br label %do.body.i.i

do.body.i.i:                                      ; preds = %land.rhs.i.i, %if.else.i.i
  %i112 = phi i8 [ %i201, %land.rhs.i.i ], [ 0, %if.else.i.i ]
  %rem.i87110.i.i = phi i8 [ %my_head.i.promoted.i116.i.i, %land.rhs.i.i ], [ 0, %if.else.i.i ]
  %inc33.i89106.i.i = phi i8 [ %inc33.i89104118.i.i, %land.rhs.i.i ], [ 1, %if.else.i.i ]
  %i113 = bitcast ptr %this to ptr
  %sunkaddr79 = getelementptr inbounds i8, ptr %i113, i64 124
  %i114 = load i8, ptr %sunkaddr79, align 4, !tbaa !37
  %cmp38.i.i.i = icmp ult i8 %inc33.i89106.i.i, 8
  br i1 %cmp38.i.i.i, label %land.rhs.lr.ph.i.i.i, label %"?split_to_fill@?$range_vector@V?$blocked_range@H@d1@detail@tbb@@$07@d1@detail@tbb@@QEAAXE@Z.exit.i.i"

land.rhs.lr.ph.i.i.i:                             ; preds = %do.body.i.i
  %idxprom.i.i.i90.i.i = zext i8 %rem.i87110.i.i to i64
  %arrayidx.i.i.i91.i.i = getelementptr inbounds %"class.tbb::detail::d1::range_vector", ptr %range_pool.i.i, i64 0, i32 3, i64 %idxprom.i.i.i90.i.i, !intel-tbaa !74
  %i115 = load i8, ptr %arrayidx.i.i.i91.i.i, align 1, !tbaa !74
  %cmp.i.i92.i.i = icmp ult i8 %i115, %i114
  br i1 %cmp.i.i92.i.i, label %"?is_divisible@?$range_vector@V?$blocked_range@H@d1@detail@tbb@@$07@d1@detail@tbb@@QEAA_NE@Z.exit.i.i.i.preheader", label %"?split_to_fill@?$range_vector@V?$blocked_range@H@d1@detail@tbb@@$07@d1@detail@tbb@@QEAAXE@Z.exit.i.i"

"?is_divisible@?$range_vector@V?$blocked_range@H@d1@detail@tbb@@$07@d1@detail@tbb@@QEAA_NE@Z.exit.i.i.i.preheader": ; preds = %land.rhs.lr.ph.i.i.i
  br label %"?is_divisible@?$range_vector@V?$blocked_range@H@d1@detail@tbb@@$07@d1@detail@tbb@@QEAA_NE@Z.exit.i.i.i"

land.rhs.i.i.i:                                   ; preds = %while.body.i.i.i
  %cmp.i.i.i.i = icmp ult i8 %inc.i.i.i, %i114
  %inc33.i.i.i = add i8 %i116, 1
  br i1 %cmp.i.i.i.i, label %"?is_divisible@?$range_vector@V?$blocked_range@H@d1@detail@tbb@@$07@d1@detail@tbb@@QEAA_NE@Z.exit.i.i.i", label %"?split_to_fill@?$range_vector@V?$blocked_range@H@d1@detail@tbb@@$07@d1@detail@tbb@@QEAAXE@Z.exit.i.i", !llvm.loop !75

"?is_divisible@?$range_vector@V?$blocked_range@H@d1@detail@tbb@@$07@d1@detail@tbb@@QEAA_NE@Z.exit.i.i.i": ; preds = %land.rhs.i.i.i, %"?is_divisible@?$range_vector@V?$blocked_range@H@d1@detail@tbb@@$07@d1@detail@tbb@@QEAA_NE@Z.exit.i.i.i.preheader"
  %arrayidx.i.i.i97.i.i = phi ptr [ %arrayidx31.i.i.i, %land.rhs.i.i.i ], [ %arrayidx.i.i.i91.i.i, %"?is_divisible@?$range_vector@V?$blocked_range@H@d1@detail@tbb@@$07@d1@detail@tbb@@QEAA_NE@Z.exit.i.i.i.preheader" ]
  %idxprom.i.i.i96.i.i = phi i64 [ %rem.i.i.i, %land.rhs.i.i.i ], [ %idxprom.i.i.i90.i.i, %"?is_divisible@?$range_vector@V?$blocked_range@H@d1@detail@tbb@@$07@d1@detail@tbb@@QEAA_NE@Z.exit.i.i.i.preheader" ]
  %rem3739.i95.i.i = phi i8 [ %promoted85, %land.rhs.i.i.i ], [ %rem.i87110.i.i, %"?is_divisible@?$range_vector@V?$blocked_range@H@d1@detail@tbb@@$07@d1@detail@tbb@@QEAA_NE@Z.exit.i.i.i.preheader" ]
  %i116 = phi i8 [ %inc33.i.i.i, %land.rhs.i.i.i ], [ %inc33.i89106.i.i, %"?is_divisible@?$range_vector@V?$blocked_range@H@d1@detail@tbb@@$07@d1@detail@tbb@@QEAA_NE@Z.exit.i.i.i.preheader" ]
  %i117 = bitcast ptr %my_pool.i.i.i.i.i to ptr
  %my_grainsize.i.i.i.i.i = getelementptr inbounds %"class.tbb::detail::d1::blocked_range", ptr %i117, i64 %idxprom.i.i.i96.i.i, i32 2
  %i118 = load i64, ptr %my_grainsize.i.i.i.i.i, align 8, !tbaa !76
  %my_end.i.i.i.i.i.i = getelementptr inbounds %"class.tbb::detail::d1::blocked_range", ptr %i117, i64 %idxprom.i.i.i96.i.i, i32 0
  %i119 = load i32, ptr %my_end.i.i.i.i.i.i, align 8, !tbaa !77
  %conv.i.i.i.i.i.i = sext i32 %i119 to i64
  %my_begin.i.i.i.i.i34.i = getelementptr inbounds %"class.tbb::detail::d1::blocked_range", ptr %i117, i64 %idxprom.i.i.i96.i.i, i32 1
  %i120 = load i32, ptr %my_begin.i.i.i.i.i34.i, align 4, !tbaa !78
  %promoted = sext i32 %i120 to i64
  %sub.i.i.i.i.i.i = sub nsw i64 %conv.i.i.i.i.i.i, %promoted
  %cmp.i.i.i.i.i = icmp ult i64 %i118, %sub.i.i.i.i.i.i
  br i1 %cmp.i.i.i.i.i, label %while.body.i.i.i, label %"?split_to_fill@?$range_vector@V?$blocked_range@H@d1@detail@tbb@@$07@d1@detail@tbb@@QEAAXE@Z.exit.i.i"

while.body.i.i.i:                                 ; preds = %"?is_divisible@?$range_vector@V?$blocked_range@H@d1@detail@tbb@@$07@d1@detail@tbb@@QEAA_NE@Z.exit.i.i.i"
  %i121 = bitcast ptr %my_pool.i.i.i.i.i to ptr
  %add.i.i.i = add i8 %rem3739.i95.i.i, 1
  %idx.ext.i.i.i = zext i8 %add.i.i.i to i64
  %rem.i.i.i = and i64 %idx.ext.i.i.i, 7
  %promoted85 = trunc i64 %rem.i.i.i to i8
  %add.ptr.i.i.i = getelementptr inbounds %"class.tbb::detail::d1::blocked_range", ptr %i121, i64 %rem.i.i.i, !intel-tbaa !79
  %i122 = bitcast ptr %add.ptr.i.i.i to ptr
  %arrayidx.i32.i.i = getelementptr inbounds %"class.tbb::detail::d1::blocked_range", ptr %i121, i64 %idxprom.i.i.i96.i.i
  %i123 = bitcast ptr %arrayidx.i32.i.i to ptr
  call void @llvm.memcpy.p0.p0.i64(ptr noundef nonnull align 8 dereferenceable(16) %i122, ptr noundef nonnull align 8 dereferenceable(16) %i123, i64 16, i1 false)
  %my_end.i.i33.i.i81 = bitcast ptr %arrayidx.i32.i.i to ptr
  %my_end2.i.i.i.i82 = bitcast ptr %add.ptr.i.i.i to ptr
  %i124 = load i32, ptr %my_end2.i.i.i.i82, align 8, !tbaa !77
  store i32 %i124, ptr %my_end.i.i33.i.i81, align 8, !tbaa !77
  %my_begin.i.i.i34.i.i = getelementptr inbounds %"class.tbb::detail::d1::blocked_range", ptr %i121, i64 %rem.i.i.i, i32 1
  %i125 = load i32, ptr %my_begin.i.i.i34.i.i, align 4, !tbaa !78
  %i126 = load i32, ptr %my_end2.i.i.i.i82, align 8, !tbaa !77
  %sub.i.i.i.i.i = sub nsw i32 %i126, %i125
  %div8.i.i.i.i.i = lshr i32 %sub.i.i.i.i.i, 1
  %add.i.i.i.i.i = add i32 %div8.i.i.i.i.i, %i125
  store i32 %add.i.i.i.i.i, ptr %my_end2.i.i.i.i82, align 8, !tbaa !77
  store i32 %add.i.i.i.i.i, ptr %my_begin.i.i.i.i.i34.i, align 4, !tbaa !78
  %my_grainsize4.i.i.i.i = getelementptr inbounds %"class.tbb::detail::d1::blocked_range", ptr %i121, i64 %rem.i.i.i, i32 2
  %i127 = load i64, ptr %my_grainsize4.i.i.i.i, align 8, !tbaa !76
  store i64 %i127, ptr %my_grainsize.i.i.i.i.i, align 8, !tbaa !76
  %i128 = load i8, ptr %arrayidx.i.i.i97.i.i, align 1, !tbaa !74
  %inc.i.i.i = add i8 %i128, 1
  store i8 %inc.i.i.i, ptr %arrayidx.i.i.i97.i.i, align 1, !tbaa !74
  %arrayidx31.i.i.i = getelementptr inbounds %"class.tbb::detail::d1::range_vector", ptr %range_pool.i.i, i64 0, i32 3, i64 %rem.i.i.i
  store i8 %inc.i.i.i, ptr %arrayidx31.i.i.i, align 1, !tbaa !74
  %exitcond.not.i.i.i = icmp eq i8 %i116, 7
  br i1 %exitcond.not.i.i.i, label %"?split_to_fill@?$range_vector@V?$blocked_range@H@d1@detail@tbb@@$07@d1@detail@tbb@@QEAAXE@Z.exit.i.i", label %land.rhs.i.i.i, !llvm.loop !75

"?split_to_fill@?$range_vector@V?$blocked_range@H@d1@detail@tbb@@$07@d1@detail@tbb@@QEAAXE@Z.exit.i.i": ; preds = %while.body.i.i.i, %"?is_divisible@?$range_vector@V?$blocked_range@H@d1@detail@tbb@@$07@d1@detail@tbb@@QEAA_NE@Z.exit.i.i.i", %land.rhs.i.i.i, %land.rhs.lr.ph.i.i.i, %do.body.i.i
  %rem.i87109.i.i = phi i8 [ %rem.i87110.i.i, %do.body.i.i ], [ %rem.i87110.i.i, %land.rhs.lr.ph.i.i.i ], [ %promoted85, %while.body.i.i.i ], [ %promoted85, %land.rhs.i.i.i ], [ %rem3739.i95.i.i, %"?is_divisible@?$range_vector@V?$blocked_range@H@d1@detail@tbb@@$07@d1@detail@tbb@@QEAA_NE@Z.exit.i.i.i" ]
  %inc33.i89105.i.i = phi i8 [ %inc33.i89106.i.i, %do.body.i.i ], [ %inc33.i89106.i.i, %land.rhs.lr.ph.i.i.i ], [ 8, %while.body.i.i.i ], [ %inc33.i.i.i, %land.rhs.i.i.i ], [ %i116, %"?is_divisible@?$range_vector@V?$blocked_range@H@d1@detail@tbb@@$07@d1@detail@tbb@@QEAA_NE@Z.exit.i.i.i" ]
  %i129 = bitcast ptr %this to ptr
  %sunkaddr86 = getelementptr inbounds i8, ptr %i129, i64 104
  %i130 = bitcast ptr %sunkaddr86 to ptr
  %i131 = load ptr, ptr %i130, align 8, !tbaa !27
  %i132 = getelementptr inbounds %"struct.tbb::detail::d1::tree_node", ptr %i131, i64 0, i32 2, i32 0, i32 0, i32 0
  %i133 = load volatile i8, ptr %i132, align 1
  %i134 = and i8 %i133, 1
  %tobool.i.i.i.not.i.i = icmp eq i8 %i134, 0
  br i1 %tobool.i.i.i.not.i.i, label %"?split_to_fill@?$range_vector@V?$blocked_range@H@d1@detail@tbb@@$07@d1@detail@tbb@@QEAAXE@Z.exit.if.end19_crit_edge.i.i", label %if.then9.i.i

"?split_to_fill@?$range_vector@V?$blocked_range@H@d1@detail@tbb@@$07@d1@detail@tbb@@QEAAXE@Z.exit.if.end19_crit_edge.i.i": ; preds = %"?split_to_fill@?$range_vector@V?$blocked_range@H@d1@detail@tbb@@$07@d1@detail@tbb@@QEAAXE@Z.exit.i.i"
  %.pre.i.i = zext i8 %rem.i87109.i.i to i64
  br label %if.end19.i.i

if.then9.i.i:                                     ; preds = %"?split_to_fill@?$range_vector@V?$blocked_range@H@d1@detail@tbb@@$07@d1@detail@tbb@@QEAAXE@Z.exit.i.i"
  %add.i36.i.i = add i8 %i114, 1
  %i135 = bitcast ptr %this to ptr
  %sunkaddr87 = getelementptr inbounds i8, ptr %i135, i64 124
  store i8 %add.i36.i.i, ptr %sunkaddr87, align 4, !tbaa !37
  %cmp.i35.i = icmp ugt i8 %inc33.i89105.i.i, 1
  br i1 %cmp.i35.i, label %do.cond.i.thread.i, label %if.end.i38.i

do.cond.i.thread.i:                               ; preds = %if.then9.i.i
  %i136 = bitcast ptr %ed to ptr
  %i137 = bitcast ptr %my_body2.i.i.i.i.i.i to ptr
  %i138 = bitcast ptr %alloc.i.i.i.i to ptr
  %i139 = bitcast ptr %alloc.i.i.i.i to ptr
  %i140 = bitcast ptr %my_pool.i.i.i.i.i to ptr
  %idxprom.i.i.i = zext i8 %i112 to i64
  %arrayidx.i39.i.i = getelementptr inbounds %"class.tbb::detail::d1::range_vector", ptr %range_pool.i.i, i64 0, i32 3, i64 %idxprom.i.i.i, !intel-tbaa !74
  %i141 = load i8, ptr %arrayidx.i39.i.i, align 1, !tbaa !74
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %i139)
  store ptr null, ptr %i138, align 8, !tbaa !44
  %call.i.i.i.i.i = call noundef ptr @"?allocate@r1@detail@tbb@@YAPEAXAEAPEAVsmall_object_pool@d1@23@_KAEBUexecution_data@523@@Z"(ptr noundef nonnull align 8 dereferenceable(8) %i138, i64 noundef 192, ptr noundef nonnull align 8 dereferenceable(16) %ed)
  %m_version_and_traits.i.i.i.i.i.i.i.i = getelementptr inbounds i8, ptr %call.i.i.i.i.i, i64 8
  %arrayinit.end.i.i.i.i.i.i.i = getelementptr inbounds i8, ptr %call.i.i.i.i.i, i64 64
  %arrayidx.i42.i.i = getelementptr inbounds %"class.tbb::detail::d1::blocked_range", ptr %i140, i64 %idxprom.i.i.i
  %i142 = bitcast ptr %call.i.i.i.i.i to ptr
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(56) %m_version_and_traits.i.i.i.i.i.i.i.i, i8 0, i64 56, i1 false)
  store ptr @"??_7?$start_for@V?$blocked_range@H@d1@detail@tbb@@VArraySummer@@$$CBVauto_partitioner@234@@d1@detail@tbb@@6B@", ptr %i142, align 64, !tbaa !46
  %i143 = bitcast ptr %arrayidx.i42.i.i to ptr
  call void @llvm.memcpy.p0.p0.i64(ptr noundef nonnull align 64 dereferenceable(16) %arrayinit.end.i.i.i.i.i.i.i, ptr noundef nonnull align 8 dereferenceable(16) %i143, i64 16, i1 false)
  %my_body.i.i.i.i.i.i = getelementptr inbounds i8, ptr %call.i.i.i.i.i, i64 80
  call void @llvm.memcpy.p0.p0.i64(ptr noundef nonnull align 16 dereferenceable(24) %my_body.i.i.i.i.i.i, ptr noundef nonnull align 16 dereferenceable(24) %i137, i64 24, i1 false)
  %my_divisor.i.i.i.i.i.i.i.i36.i = getelementptr inbounds i8, ptr %call.i.i.i.i.i, i64 112
  %i144 = bitcast ptr %my_divisor.i.i.i.i.i.i.i.i36.i to ptr
  %i145 = bitcast ptr %this to ptr
  %sunkaddr88 = getelementptr inbounds i8, ptr %i145, i64 112
  %i146 = bitcast ptr %sunkaddr88 to ptr
  %i147 = load i64, ptr %i146, align 16, !tbaa !23
  %div2.i.i.i.i.i.i.i.i.i.i = lshr i64 %i147, 1
  store i64 %div2.i.i.i.i.i.i.i.i.i.i, ptr %i146, align 16, !tbaa !23
  store i64 %div2.i.i.i.i.i.i.i.i.i.i, ptr %i144, align 16, !tbaa !23
  %my_delay.i.i.i.i.i.i.i.i = getelementptr inbounds i8, ptr %call.i.i.i.i.i, i64 120
  %i148 = bitcast ptr %my_delay.i.i.i.i.i.i.i.i to ptr
  store i32 2, ptr %i148, align 8, !tbaa !48
  %i149 = getelementptr inbounds i8, ptr %call.i.i.i.i.i, i64 124
  %i150 = bitcast ptr %this to ptr
  %sunkaddr89 = getelementptr inbounds i8, ptr %i150, i64 124
  %i151 = load i8, ptr %sunkaddr89, align 4, !tbaa !37
  %i152 = getelementptr inbounds i8, ptr %call.i.i.i.i.i, i64 128
  %i153 = bitcast ptr %i152 to ptr
  %i154 = load ptr, ptr %i138, align 8, !tbaa !44
  store ptr %i154, ptr %i153, align 64, !tbaa !49
  %sub.i.i.i.i.i.i37.i = sub i8 %i151, %i141
  store i8 %sub.i.i.i.i.i.i37.i, ptr %i149, align 4, !tbaa !37
  %call.i16.i.i.i.i = call noundef ptr @"?allocate@r1@detail@tbb@@YAPEAXAEAPEAVsmall_object_pool@d1@23@_KAEBUexecution_data@523@@Z"(ptr noundef nonnull align 8 dereferenceable(8) %i138, i64 noundef 32, ptr noundef nonnull align 8 dereferenceable(16) %ed)
  %i155 = bitcast ptr %this to ptr
  %sunkaddr90 = getelementptr inbounds i8, ptr %i155, i64 104
  %i156 = bitcast ptr %sunkaddr90 to ptr
  %i157 = load ptr, ptr %i156, align 8, !tbaa !27
  %my_parent.i.i.i.i.i.i.i = bitcast ptr %call.i16.i.i.i.i to ptr
  store ptr %i157, ptr %my_parent.i.i.i.i.i.i.i, align 8, !tbaa !50
  %_Value2.i.i.i.i.i.i.i.i.i.i.i = getelementptr inbounds i8, ptr %call.i16.i.i.i.i, i64 8
  %i158 = bitcast ptr %_Value2.i.i.i.i.i.i.i.i.i.i.i to ptr
  store i32 2, ptr %i158, align 8, !tbaa !53
  %i159 = getelementptr inbounds i8, ptr %call.i16.i.i.i.i, i64 16
  %i160 = bitcast ptr %i159 to ptr
  %i161 = load ptr, ptr %i138, align 8, !tbaa !44
  store ptr %i161, ptr %i160, align 8, !tbaa !49
  %i162 = getelementptr inbounds i8, ptr %call.i16.i.i.i.i, i64 24
  store i8 0, ptr %i162, align 8, !tbaa !56
  %i163 = bitcast ptr %this to ptr
  %sunkaddr91 = getelementptr inbounds i8, ptr %i163, i64 104
  %i164 = bitcast ptr %sunkaddr91 to ptr
  store ptr %call.i16.i.i.i.i, ptr %i164, align 8, !tbaa !27
  %my_parent9.i.i.i.i = getelementptr inbounds i8, ptr %call.i.i.i.i.i, i64 104
  %i165 = bitcast ptr %my_parent9.i.i.i.i to ptr
  store ptr %call.i16.i.i.i.i, ptr %i165, align 8, !tbaa !27
  %i166 = load ptr, ptr %i136, align 8, !tbaa !60
  %i167 = bitcast ptr %call.i.i.i.i.i to ptr
  call void @"?spawn@r1@detail@tbb@@YAXAEAVtask@d1@23@AEAVtask_group_context@523@@Z"(ptr noundef nonnull align 64 dereferenceable(64) %i167, ptr noundef nonnull align 8 dereferenceable(128) %i166)
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %i139)
  %i168 = add i8 %i112, 1
  %i169 = and i8 %i168, 7
  %inc33.i89104.i44.i = add i8 %inc33.i89105.i.i, -1
  br label %land.rhs.i.i

if.end.i38.i:                                     ; preds = %if.then9.i.i
  %idxprom.i.i.i.i = zext i8 %rem.i87109.i.i to i64
  %arrayidx.i.i47.i.i = getelementptr inbounds %"class.tbb::detail::d1::range_vector", ptr %range_pool.i.i, i64 0, i32 3, i64 %idxprom.i.i.i.i, !intel-tbaa !74
  %i170 = load i8, ptr %arrayidx.i.i47.i.i, align 1, !tbaa !74
  %cmp.i48.i.i = icmp ult i8 %i170, %add.i36.i.i
  br i1 %cmp.i48.i.i, label %"?is_divisible@?$range_vector@V?$blocked_range@H@d1@detail@tbb@@$07@d1@detail@tbb@@QEAA_NE@Z.exit.i.i", label %if.end19.i.i

"?is_divisible@?$range_vector@V?$blocked_range@H@d1@detail@tbb@@$07@d1@detail@tbb@@QEAA_NE@Z.exit.i.i": ; preds = %if.end.i38.i
  %i171 = bitcast ptr %my_pool.i.i.i.i.i to ptr
  %my_grainsize.i.i.i.i = getelementptr inbounds %"class.tbb::detail::d1::blocked_range", ptr %i171, i64 %idxprom.i.i.i.i, i32 2
  %i172 = load i64, ptr %my_grainsize.i.i.i.i, align 8, !tbaa !76
  %my_end.i.i.i49.i.i = getelementptr inbounds %"class.tbb::detail::d1::blocked_range", ptr %i171, i64 %idxprom.i.i.i.i, i32 0
  %i173 = load i32, ptr %my_end.i.i.i49.i.i, align 8, !tbaa !77
  %conv.i.i.i.i.i = sext i32 %i173 to i64
  %my_begin.i.i.i50.i.i = getelementptr inbounds %"class.tbb::detail::d1::blocked_range", ptr %i171, i64 %idxprom.i.i.i.i, i32 1
  %i174 = load i32, ptr %my_begin.i.i.i50.i.i, align 4, !tbaa !78
  %promoted92 = sext i32 %i174 to i64
  %sub.i.i.i51.i.i = sub nsw i64 %conv.i.i.i.i.i, %promoted92
  %cmp.i.i52.i.i = icmp ult i64 %i172, %sub.i.i.i51.i.i
  br i1 %cmp.i.i52.i.i, label %land.rhs.i.i, label %if.end19.i.i

if.end19.i.i:                                     ; preds = %"?is_divisible@?$range_vector@V?$blocked_range@H@d1@detail@tbb@@$07@d1@detail@tbb@@QEAA_NE@Z.exit.i.i", %if.end.i38.i, %"?split_to_fill@?$range_vector@V?$blocked_range@H@d1@detail@tbb@@$07@d1@detail@tbb@@QEAAXE@Z.exit.if.end19_crit_edge.i.i"
  %idxprom.i56.pre-phi.i.i = phi i64 [ %.pre.i.i, %"?split_to_fill@?$range_vector@V?$blocked_range@H@d1@detail@tbb@@$07@d1@detail@tbb@@QEAAXE@Z.exit.if.end19_crit_edge.i.i" ], [ %idxprom.i.i.i.i, %if.end.i38.i ], [ %idxprom.i.i.i.i, %"?is_divisible@?$range_vector@V?$blocked_range@H@d1@detail@tbb@@$07@d1@detail@tbb@@QEAA_NE@Z.exit.i.i" ]
  %i175 = bitcast ptr %my_pool.i.i.i.i.i to ptr
  %my_begin.i.i.i58.i.i = getelementptr inbounds %"class.tbb::detail::d1::blocked_range", ptr %i175, i64 %idxprom.i56.pre-phi.i.i, i32 1
  %i176 = load i32, ptr %my_begin.i.i.i58.i.i, align 4, !tbaa !78
  %i177 = sext i32 %i176 to i64
  %my_end.i.i.i59.i.i = getelementptr inbounds %"class.tbb::detail::d1::blocked_range", ptr %i175, i64 %idxprom.i56.pre-phi.i.i, i32 0
  %i178 = load i32, ptr %my_end.i.i.i59.i.i, align 8, !tbaa !77
  %cmp.not17.i.i60.i.i = icmp eq i32 %i176, %i178
  br i1 %cmp.not17.i.i60.i.i, label %do.cond.i.i, label %for.body.lr.ph.i.i67.i.i

for.body.lr.ph.i.i67.i.i:                         ; preds = %if.end19.i.i
  %i179 = bitcast ptr %this to ptr
  %sunkaddr93 = getelementptr inbounds i8, ptr %i179, i64 96
  %i180 = bitcast ptr %sunkaddr93 to ptr
  %.pre.i.i64.i.i = load ptr, ptr %i180, align 32, !tbaa !61
  %i181 = bitcast ptr %this to ptr
  %sunkaddr94 = getelementptr inbounds i8, ptr %i181, i64 88
  %i182 = bitcast ptr %sunkaddr94 to ptr
  %.pre19.i.i65.i.i = load ptr, ptr %i182, align 8, !tbaa !62
  %i183 = bitcast ptr %this to ptr
  %sunkaddr95 = getelementptr inbounds i8, ptr %i183, i64 80
  %i184 = bitcast ptr %sunkaddr95 to ptr
  %.pre20.i.i66.i.i = load ptr, ptr %i184, align 16, !tbaa !63
  br label %for.body.i.i75.i.i

for.body.i.i75.i.i:                               ; preds = %for.body.i.i75.i.i, %for.body.lr.ph.i.i67.i.i
  %i185 = phi ptr [ %.pre20.i.i66.i.i, %for.body.lr.ph.i.i67.i.i ], [ %i190, %for.body.i.i75.i.i ]
  %i186 = phi ptr [ %.pre19.i.i65.i.i, %for.body.lr.ph.i.i67.i.i ], [ %i194, %for.body.i.i75.i.i ]
  %i187 = phi ptr [ %.pre.i.i64.i.i, %for.body.lr.ph.i.i67.i.i ], [ %i198, %for.body.i.i75.i.i ]
  %indvars.iv.i.i68.i.i = phi i64 [ %i177, %for.body.lr.ph.i.i67.i.i ], [ %indvars.iv.next.i.i73.i.i, %for.body.i.i75.i.i ]
  %lsr53 = trunc i64 %indvars.iv.i.i68.i.i to i32
  call fastcc void @"?delta_stepping@@YAXPEAH00H@Z"(ptr noundef %i185, ptr noundef %i186, ptr noundef %i187, i32 noundef %lsr53)
  %i188 = bitcast ptr %this to ptr
  %sunkaddr96 = getelementptr inbounds i8, ptr %i188, i64 80
  %i189 = bitcast ptr %sunkaddr96 to ptr
  %i190 = load ptr, ptr %i189, align 16, !tbaa !63
  %scevgep51 = getelementptr i32, ptr %i190, i64 %indvars.iv.i.i68.i.i
  %i191 = load i32, ptr %scevgep51, align 4, !tbaa !64
  %i192 = bitcast ptr %this to ptr
  %sunkaddr97 = getelementptr inbounds i8, ptr %i192, i64 88
  %i193 = bitcast ptr %sunkaddr97 to ptr
  %i194 = load ptr, ptr %i193, align 8, !tbaa !62
  %scevgep50 = getelementptr i32, ptr %i194, i64 %indvars.iv.i.i68.i.i
  %i195 = load i32, ptr %scevgep50, align 4, !tbaa !64
  %add.i.i71.i.i = add nsw i32 %i195, %i191
  %i196 = bitcast ptr %this to ptr
  %sunkaddr98 = getelementptr inbounds i8, ptr %i196, i64 96
  %i197 = bitcast ptr %sunkaddr98 to ptr
  %i198 = load ptr, ptr %i197, align 32, !tbaa !61
  %scevgep49 = getelementptr i32, ptr %i198, i64 %indvars.iv.i.i68.i.i
  store i32 %add.i.i71.i.i, ptr %scevgep49, align 4, !tbaa !64
  %indvars.iv.next.i.i73.i.i = add nsw i64 %indvars.iv.i.i68.i.i, 1
  %lsr52 = trunc i64 %indvars.iv.next.i.i73.i.i to i32
  %cmp.not.i.i74.i.i = icmp eq i32 %i178, %lsr52
  br i1 %cmp.not.i.i74.i.i, label %do.cond.i.i, label %for.body.i.i75.i.i, !llvm.loop !65

do.cond.i.i:                                      ; preds = %for.body.i.i75.i.i, %if.end19.i.i
  %i199 = add i8 %rem.i87109.i.i, 7
  %i200 = and i8 %i199, 7
  %inc33.i89104.i.i = add i8 %inc33.i89105.i.i, -1
  %cmp.i81.i.i = icmp eq i8 %inc33.i89104.i.i, 0
  br i1 %cmp.i81.i.i, label %"??1?$range_vector@V?$blocked_range@H@d1@detail@tbb@@$07@d1@detail@tbb@@QEAA@XZ.exit.i.i", label %land.rhs.i.i

land.rhs.i.i:                                     ; preds = %do.cond.i.i, %"?is_divisible@?$range_vector@V?$blocked_range@H@d1@detail@tbb@@$07@d1@detail@tbb@@QEAA_NE@Z.exit.i.i", %do.cond.i.thread.i
  %inc33.i89104118.i.i = phi i8 [ %inc33.i89104.i.i, %do.cond.i.i ], [ 1, %"?is_divisible@?$range_vector@V?$blocked_range@H@d1@detail@tbb@@$07@d1@detail@tbb@@QEAA_NE@Z.exit.i.i" ], [ %inc33.i89104.i44.i, %do.cond.i.thread.i ]
  %my_head.i.promoted.i116.i.i = phi i8 [ %i200, %do.cond.i.i ], [ %rem.i87109.i.i, %"?is_divisible@?$range_vector@V?$blocked_range@H@d1@detail@tbb@@$07@d1@detail@tbb@@QEAA_NE@Z.exit.i.i" ], [ %rem.i87109.i.i, %do.cond.i.thread.i ]
  %i201 = phi i8 [ %i112, %do.cond.i.i ], [ %i112, %"?is_divisible@?$range_vector@V?$blocked_range@H@d1@detail@tbb@@$07@d1@detail@tbb@@QEAA_NE@Z.exit.i.i" ], [ %i169, %do.cond.i.thread.i ]
  %i202 = bitcast ptr %ed to ptr
  %i203 = load ptr, ptr %i202, align 8, !tbaa !60
  %i204 = getelementptr inbounds %"class.tbb::detail::d1::task_group_context", ptr %i203, i64 0, i32 5, i32 0, i32 0, i32 0
  %i205 = load volatile i8, ptr %i204, align 1
  %cmp.i.i.i82.i.i = icmp eq i8 %i205, -1
  %my_actual_context.i.i.i.i = getelementptr inbounds %"class.tbb::detail::d1::task_group_context", ptr %i203, i64 0, i32 6, i32 0
  %i206 = load ptr, ptr %my_actual_context.i.i.i.i, align 8
  %i207 = select i1 %cmp.i.i.i82.i.i, ptr %i206, ptr %i203
  %call2.i.i.i = call noundef zeroext i1 @"?is_group_execution_cancelled@r1@detail@tbb@@YA_NAEAVtask_group_context@d1@23@@Z"(ptr noundef nonnull align 8 dereferenceable(128) %i207)
  br i1 %call2.i.i.i, label %"??1?$range_vector@V?$blocked_range@H@d1@detail@tbb@@$07@d1@detail@tbb@@QEAA@XZ.exit.i.i", label %do.body.i.i, !llvm.loop !80

"??1?$range_vector@V?$blocked_range@H@d1@detail@tbb@@$07@d1@detail@tbb@@QEAA@XZ.exit.i.i": ; preds = %land.rhs.i.i, %do.cond.i.i
  %i208 = bitcast ptr %range_pool.i.i to ptr
  call void @llvm.lifetime.end.p0(i64 144, ptr nonnull %i208)
  br label %"??$execute@U?$start_for@V?$blocked_range@H@d1@detail@tbb@@VArraySummer@@$$CBVauto_partitioner@234@@d1@detail@tbb@@V?$blocked_range@H@234@@?$partition_type_base@Vauto_partition_type@d1@detail@tbb@@@d1@detail@tbb@@QEAAXAEAU?$start_for@V?$blocked_range@H@d1@detail@tbb@@VArraySummer@@$$CBVauto_partitioner@234@@123@AEAV?$blocked_range@H@123@AEAUexecution_data@123@@Z.exit"

"??$execute@U?$start_for@V?$blocked_range@H@d1@detail@tbb@@VArraySummer@@$$CBVauto_partitioner@234@@d1@detail@tbb@@V?$blocked_range@H@234@@?$partition_type_base@Vauto_partition_type@d1@detail@tbb@@@d1@detail@tbb@@QEAAXAEAU?$start_for@V?$blocked_range@H@d1@detail@tbb@@VArraySummer@@$$CBVauto_partitioner@234@@123@AEAV?$blocked_range@H@123@AEAUexecution_data@123@@Z.exit": ; preds = %"??1?$range_vector@V?$blocked_range@H@d1@detail@tbb@@$07@d1@detail@tbb@@QEAA@XZ.exit.i.i", %for.body.i.i.i.i, %if.then.i.i
  %my_parent.i12 = getelementptr inbounds %"struct.tbb::detail::d1::start_for", ptr %this, i64 0, i32 3, !intel-tbaa !27
  %i209 = load ptr, ptr %my_parent.i12, align 8, !tbaa !27
  %i210 = getelementptr inbounds %"struct.tbb::detail::d1::start_for", ptr %this, i64 0, i32 5, i32 0
  %i211 = load ptr, ptr %i210, align 64, !tbaa !49
  %i212 = bitcast ptr %this to ptr
  %vtable.i = load ptr, ptr %i212, align 64, !tbaa !46
  %i213 = load ptr, ptr %vtable.i, align 8
  %call.i13 = call noundef ptr %i213(ptr noundef nonnull align 64 dereferenceable(144) %this, i32 noundef 0)
  %i214 = getelementptr inbounds %"struct.tbb::detail::d1::node", ptr %i209, i64 0, i32 1, i32 0, i32 0, i32 0, i32 0, i32 0
  %i215 = atomicrmw sub ptr %i214, i32 1 seq_cst, align 4
  %i216 = add i32 %i215, -1
  %cmp18.i.i = icmp sgt i32 %i216, 0
  br i1 %cmp18.i.i, label %"?finalize@?$start_for@V?$blocked_range@H@d1@detail@tbb@@VArraySummer@@$$CBVauto_partitioner@234@@d1@detail@tbb@@QEAAXAEBUexecution_data@234@@Z.exit", label %if.end.i.i15.preheader

if.end.i.i15.preheader:                           ; preds = %"??$execute@U?$start_for@V?$blocked_range@H@d1@detail@tbb@@VArraySummer@@$$CBVauto_partitioner@234@@d1@detail@tbb@@V?$blocked_range@H@234@@?$partition_type_base@Vauto_partition_type@d1@detail@tbb@@@d1@detail@tbb@@QEAAXAEAU?$start_for@V?$blocked_range@H@d1@detail@tbb@@VArraySummer@@$$CBVauto_partitioner@234@@123@AEAV?$blocked_range@H@123@AEAUexecution_data@123@@Z.exit"
  br label %if.end.i.i15

if.end.i.i15:                                     ; preds = %cleanup.i.i, %if.end.i.i15.preheader
  %n.addr.019.i.i = phi ptr [ %i217, %cleanup.i.i ], [ %i209, %if.end.i.i15.preheader ]
  %my_parent.i.i99 = bitcast ptr %n.addr.019.i.i to ptr
  %i217 = load ptr, ptr %my_parent.i.i99, align 8, !tbaa !50
  %tobool.not.i.i14 = icmp eq ptr %i217, null
  br i1 %tobool.not.i.i14, label %for.end.i.i, label %cleanup.i.i

cleanup.i.i:                                      ; preds = %if.end.i.i15
  %m_allocator.i.i = getelementptr inbounds %"struct.tbb::detail::d1::node", ptr %n.addr.019.i.i, i64 1
  %i218 = bitcast ptr %m_allocator.i.i to ptr
  %i219 = load ptr, ptr %i218, align 8, !tbaa !49
  %i220 = bitcast ptr %n.addr.019.i.i to ptr
  call void @"?deallocate@r1@detail@tbb@@YAXAEAVsmall_object_pool@d1@23@PEAX_KAEBUexecution_data@523@@Z"(ptr noundef nonnull align 1 dereferenceable(1) %i219, ptr noundef %i220, i64 noundef 32, ptr noundef nonnull align 8 dereferenceable(16) %ed)
  %i221 = getelementptr inbounds %"struct.tbb::detail::d1::node", ptr %i217, i64 0, i32 1, i32 0, i32 0, i32 0, i32 0, i32 0
  %i222 = atomicrmw sub ptr %i221, i32 1 seq_cst, align 4
  %i223 = add i32 %i222, -1
  %cmp.i.i16 = icmp sgt i32 %i223, 0
  br i1 %cmp.i.i16, label %"?finalize@?$start_for@V?$blocked_range@H@d1@detail@tbb@@VArraySummer@@$$CBVauto_partitioner@234@@d1@detail@tbb@@QEAAXAEBUexecution_data@234@@Z.exit", label %if.end.i.i15

for.end.i.i:                                      ; preds = %if.end.i.i15
  %i224 = getelementptr inbounds %"struct.tbb::detail::d1::node", ptr %n.addr.019.i.i, i64 1, i32 1
  %i225 = bitcast ptr %i224 to ptr
  %i226 = atomicrmw add ptr %i225, i64 -1 seq_cst, align 8
  %tobool.not.i.i.i.i = icmp eq i64 %i226, 1
  br i1 %tobool.not.i.i.i.i, label %if.then.i.i.i.i, label %"?finalize@?$start_for@V?$blocked_range@H@d1@detail@tbb@@VArraySummer@@$$CBVauto_partitioner@234@@d1@detail@tbb@@QEAAXAEBUexecution_data@234@@Z.exit"

if.then.i.i.i.i:                                  ; preds = %for.end.i.i
  %m_wait.i.i = getelementptr inbounds %"struct.tbb::detail::d1::node", ptr %n.addr.019.i.i, i64 1
  %i227 = ptrtoint ptr %m_wait.i.i to i64
  call void @"?notify_waiters@r1@detail@tbb@@YAX_K@Z"(i64 noundef %i227)
  br label %"?finalize@?$start_for@V?$blocked_range@H@d1@detail@tbb@@VArraySummer@@$$CBVauto_partitioner@234@@d1@detail@tbb@@QEAAXAEBUexecution_data@234@@Z.exit"

"?finalize@?$start_for@V?$blocked_range@H@d1@detail@tbb@@VArraySummer@@$$CBVauto_partitioner@234@@d1@detail@tbb@@QEAAXAEBUexecution_data@234@@Z.exit": ; preds = %if.then.i.i.i.i, %for.end.i.i, %cleanup.i.i, %"??$execute@U?$start_for@V?$blocked_range@H@d1@detail@tbb@@VArraySummer@@$$CBVauto_partitioner@234@@d1@detail@tbb@@V?$blocked_range@H@234@@?$partition_type_base@Vauto_partition_type@d1@detail@tbb@@@d1@detail@tbb@@QEAAXAEAU?$start_for@V?$blocked_range@H@d1@detail@tbb@@VArraySummer@@$$CBVauto_partitioner@234@@123@AEAV?$blocked_range@H@123@AEAUexecution_data@123@@Z.exit"
  %i228 = bitcast ptr %this to ptr
  call void @"?deallocate@r1@detail@tbb@@YAXAEAVsmall_object_pool@d1@23@PEAX_KAEBUexecution_data@523@@Z"(ptr noundef nonnull align 1 dereferenceable(1) %i211, ptr noundef nonnull %i228, i64 noundef 192, ptr noundef nonnull align 8 dereferenceable(16) %ed)
  ret ptr null
}

define linkonce_odr dso_local noundef ptr @"?cancel@?$start_for@V?$blocked_range@H@d1@detail@tbb@@VArraySummer@@$$CBVauto_partitioner@234@@d1@detail@tbb@@UEAAPEAVtask@234@AEAUexecution_data@234@@Z"(ptr noundef nonnull align 64 dereferenceable(144) %this, ptr noundef nonnull align 8 dereferenceable(16) %ed) comdat align 2 {
entry:
  %my_parent.i = getelementptr inbounds %"struct.tbb::detail::d1::start_for", ptr %this, i64 0, i32 3, !intel-tbaa !27
  %i = load ptr, ptr %my_parent.i, align 8, !tbaa !27
  %i1 = getelementptr inbounds %"struct.tbb::detail::d1::start_for", ptr %this, i64 0, i32 5, i32 0
  %i2 = load ptr, ptr %i1, align 64, !tbaa !49
  %i3 = bitcast ptr %this to ptr
  %vtable.i = load ptr, ptr %i3, align 64, !tbaa !46
  %i4 = load ptr, ptr %vtable.i, align 8
  %call.i = tail call noundef ptr %i4(ptr noundef nonnull align 64 dereferenceable(144) %this, i32 noundef 0)
  %i5 = getelementptr inbounds %"struct.tbb::detail::d1::node", ptr %i, i64 0, i32 1, i32 0, i32 0, i32 0, i32 0, i32 0
  %i6 = atomicrmw sub ptr %i5, i32 1 seq_cst, align 4
  %i7 = add i32 %i6, -1
  %cmp18.i.i = icmp sgt i32 %i7, 0
  br i1 %cmp18.i.i, label %"?finalize@?$start_for@V?$blocked_range@H@d1@detail@tbb@@VArraySummer@@$$CBVauto_partitioner@234@@d1@detail@tbb@@QEAAXAEBUexecution_data@234@@Z.exit", label %if.end.i.i.preheader

if.end.i.i.preheader:                             ; preds = %entry
  br label %if.end.i.i

if.end.i.i:                                       ; preds = %cleanup.i.i, %if.end.i.i.preheader
  %n.addr.019.i.i = phi ptr [ %i8, %cleanup.i.i ], [ %i, %if.end.i.i.preheader ]
  %my_parent.i.i10 = bitcast ptr %n.addr.019.i.i to ptr
  %i8 = load ptr, ptr %my_parent.i.i10, align 8, !tbaa !50
  %tobool.not.i.i = icmp eq ptr %i8, null
  br i1 %tobool.not.i.i, label %for.end.i.i, label %cleanup.i.i

cleanup.i.i:                                      ; preds = %if.end.i.i
  %m_allocator.i.i = getelementptr inbounds %"struct.tbb::detail::d1::node", ptr %n.addr.019.i.i, i64 1
  %i9 = bitcast ptr %m_allocator.i.i to ptr
  %i10 = load ptr, ptr %i9, align 8, !tbaa !49
  %i11 = bitcast ptr %n.addr.019.i.i to ptr
  tail call void @"?deallocate@r1@detail@tbb@@YAXAEAVsmall_object_pool@d1@23@PEAX_KAEBUexecution_data@523@@Z"(ptr noundef nonnull align 1 dereferenceable(1) %i10, ptr noundef %i11, i64 noundef 32, ptr noundef nonnull align 8 dereferenceable(16) %ed)
  %i12 = getelementptr inbounds %"struct.tbb::detail::d1::node", ptr %i8, i64 0, i32 1, i32 0, i32 0, i32 0, i32 0, i32 0
  %i13 = atomicrmw sub ptr %i12, i32 1 seq_cst, align 4
  %i14 = add i32 %i13, -1
  %cmp.i.i = icmp sgt i32 %i14, 0
  br i1 %cmp.i.i, label %"?finalize@?$start_for@V?$blocked_range@H@d1@detail@tbb@@VArraySummer@@$$CBVauto_partitioner@234@@d1@detail@tbb@@QEAAXAEBUexecution_data@234@@Z.exit", label %if.end.i.i

for.end.i.i:                                      ; preds = %if.end.i.i
  %i15 = getelementptr inbounds %"struct.tbb::detail::d1::node", ptr %n.addr.019.i.i, i64 1, i32 1
  %i16 = bitcast ptr %i15 to ptr
  %i17 = atomicrmw add ptr %i16, i64 -1 seq_cst, align 8
  %tobool.not.i.i.i.i = icmp eq i64 %i17, 1
  br i1 %tobool.not.i.i.i.i, label %if.then.i.i.i.i, label %"?finalize@?$start_for@V?$blocked_range@H@d1@detail@tbb@@VArraySummer@@$$CBVauto_partitioner@234@@d1@detail@tbb@@QEAAXAEBUexecution_data@234@@Z.exit"

if.then.i.i.i.i:                                  ; preds = %for.end.i.i
  %m_wait.i.i = getelementptr inbounds %"struct.tbb::detail::d1::node", ptr %n.addr.019.i.i, i64 1
  %i18 = ptrtoint ptr %m_wait.i.i to i64
  tail call void @"?notify_waiters@r1@detail@tbb@@YAX_K@Z"(i64 noundef %i18)
  br label %"?finalize@?$start_for@V?$blocked_range@H@d1@detail@tbb@@VArraySummer@@$$CBVauto_partitioner@234@@d1@detail@tbb@@QEAAXAEBUexecution_data@234@@Z.exit"

"?finalize@?$start_for@V?$blocked_range@H@d1@detail@tbb@@VArraySummer@@$$CBVauto_partitioner@234@@d1@detail@tbb@@QEAAXAEBUexecution_data@234@@Z.exit": ; preds = %if.then.i.i.i.i, %for.end.i.i, %cleanup.i.i, %entry
  %i19 = bitcast ptr %this to ptr
  tail call void @"?deallocate@r1@detail@tbb@@YAXAEAVsmall_object_pool@d1@23@PEAX_KAEBUexecution_data@523@@Z"(ptr noundef nonnull align 1 dereferenceable(1) %i2, ptr noundef nonnull %i19, i64 noundef 192, ptr noundef nonnull align 8 dereferenceable(16) %ed)
  ret ptr null
}

define linkonce_odr dso_local noundef ptr @"??_Gtask@d1@detail@tbb@@MEAAPEAXI@Z"(ptr noundef nonnull align 64 dereferenceable(64) %this, i32 noundef %should_call_delete) comdat align 2 {
entry:
  tail call void @llvm.trap()
  unreachable
}

declare dso_local void @_purecall()

; Function Attrs: cold noreturn nounwind
declare void @llvm.trap() #0

declare dso_local noundef i32 @"?max_concurrency@r1@detail@tbb@@YAHPEBVtask_arena_base@d1@23@@Z"(ptr noundef)

declare dso_local void @"??3@YAXPEAX@Z"(ptr noundef)

declare dso_local noundef i16 @"?execution_slot@r1@detail@tbb@@YAGPEBUexecution_data@d1@23@@Z"(ptr noundef)

declare dso_local noundef ptr @"?allocate@r1@detail@tbb@@YAPEAXAEAPEAVsmall_object_pool@d1@23@_KAEBUexecution_data@523@@Z"(ptr noundef nonnull align 8 dereferenceable(8), i64 noundef, ptr noundef nonnull align 8 dereferenceable(16))

declare dso_local void @"?spawn@r1@detail@tbb@@YAXAEAVtask@d1@23@AEAVtask_group_context@523@@Z"(ptr noundef nonnull align 64 dereferenceable(64), ptr noundef nonnull align 8 dereferenceable(128))

define internal fastcc void @"?delta_stepping@@YAXPEAH00H@Z"(ptr nocapture noundef readonly %p_array_a, ptr nocapture noundef readonly %p_array_b, ptr nocapture noundef writeonly %p_array_sum, i32 noundef %index) {
entry:
  tail call fastcc void @"?relax@@YAXPEAH00H@Z"(ptr noundef %p_array_a, ptr noundef %p_array_b, ptr noundef %p_array_sum, i32 noundef %index)
  tail call fastcc void @"?relax@@YAXPEAH00H@Z"(ptr noundef %p_array_a, ptr noundef %p_array_b, ptr noundef %p_array_sum, i32 noundef %index)
  tail call fastcc void @"?relax@@YAXPEAH00H@Z"(ptr noundef %p_array_a, ptr noundef %p_array_b, ptr noundef %p_array_sum, i32 noundef %index)
  ret void
}

define internal fastcc void @"?relax@@YAXPEAH00H@Z"(ptr nocapture noundef readonly %p_array_a, ptr nocapture noundef readonly %p_array_b, ptr nocapture noundef writeonly %p_array_sum, i32 noundef %index) {
entry:
  %cmp = icmp sgt i32 %index, 10
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %idxprom = zext i32 %index to i64
  %arrayidx = getelementptr inbounds i32, ptr %p_array_a, i64 %idxprom
  %i = load i32, ptr %arrayidx, align 4, !tbaa !64
  %arrayidx2 = getelementptr inbounds i32, ptr %p_array_b, i64 %idxprom
  %i1 = load i32, ptr %arrayidx2, align 4, !tbaa !64
  %add = add nsw i32 %i1, %i
  %arrayidx4 = getelementptr inbounds i32, ptr %p_array_sum, i64 %idxprom
  store i32 %add, ptr %arrayidx4, align 4, !tbaa !64
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  ret void
}

declare dso_local noundef zeroext i1 @"?is_group_execution_cancelled@r1@detail@tbb@@YA_NAEAVtask_group_context@d1@23@@Z"(ptr noundef nonnull align 8 dereferenceable(128))

declare dso_local void @"?deallocate@r1@detail@tbb@@YAXAEAVsmall_object_pool@d1@23@PEAX_KAEBUexecution_data@523@@Z"(ptr noundef nonnull align 1 dereferenceable(1), ptr noundef, i64 noundef, ptr noundef nonnull align 8 dereferenceable(16))

declare dso_local void @"?notify_waiters@r1@detail@tbb@@YAX_K@Z"(i64 noundef)

declare dso_local void @"?execute_and_wait@r1@detail@tbb@@YAXAEAVtask@d1@23@AEAVtask_group_context@523@AEAVwait_context@523@1@Z"(ptr noundef nonnull align 64 dereferenceable(64), ptr noundef nonnull align 8 dereferenceable(128), ptr noundef nonnull align 8 dereferenceable(16), ptr noundef nonnull align 8 dereferenceable(128))

declare dso_local void @"?destroy@r1@detail@tbb@@YAXAEAVtask_group_context@d1@23@@Z"(ptr noundef nonnull align 8 dereferenceable(128))

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: argmemonly nofree nounwind willreturn
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #2

; Function Attrs: argmemonly nofree nounwind willreturn writeonly
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #3

attributes #0 = { cold noreturn nounwind }
attributes #1 = { argmemonly nocallback nofree nosync nounwind willreturn }
attributes #2 = { argmemonly nofree nounwind willreturn }
attributes #3 = { argmemonly nofree nounwind willreturn writeonly }

!llvm.linker.options = !{!0, !1, !2, !2, !3, !4, !5, !6, !7, !8, !9, !10, !11, !12}
!llvm.module.flags = !{!13, !14, !15}
!llvm.ident = !{!16}

!0 = !{!"/DEFAULTLIB:tbb12.lib"}
!1 = !{!"/DEFAULTLIB:libcpmt.lib"}
!2 = !{!"/DEFAULTLIB:uuid.lib"}
!3 = !{!"/DEFAULTLIB:libcmt.lib"}
!4 = !{!"/DEFAULTLIB:libircmt.lib"}
!5 = !{!"/DEFAULTLIB:svml_dispmt.lib"}
!6 = !{!"/DEFAULTLIB:libdecimal.lib"}
!7 = !{!"/DEFAULTLIB:libmmt.lib"}
!8 = !{!"/DEFAULTLIB:oldnames.lib"}
!9 = !{!"/FAILIFMISMATCH:\22_MSC_VER=1900\22"}
!10 = !{!"/FAILIFMISMATCH:\22_ITERATOR_DEBUG_LEVEL=0\22"}
!11 = !{!"/FAILIFMISMATCH:\22RuntimeLibrary=MT_StaticRelease\22"}
!12 = !{!"/FAILIFMISMATCH:\22_CRT_STDIO_ISO_WIDE_SPECIFIERS=0\22"}
!13 = !{i32 1, !"wchar_size", i32 2}
!14 = !{i32 7, !"PIC Level", i32 2}
!15 = !{i32 7, !"uwtable", i32 2}
!16 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!17 = !{!18, !22, i64 10}
!18 = !{!"struct@?AUexecution_data@d1@detail@tbb@@", !19, i64 0, !22, i64 8, !22, i64 10}
!19 = !{!"pointer@?APEAVtask_group_context@d1@detail@tbb@@", !20, i64 0}
!20 = !{!"omnipotent char", !21, i64 0}
!21 = !{!"Simple C++ TBAA"}
!22 = !{!"short", !20, i64 0}
!23 = !{!24, !25, i64 0}
!24 = !{!"struct@?AU?$adaptive_mode@Vauto_partition_type@d1@detail@tbb@@@d1@detail@tbb@@", !25, i64 0}
!25 = !{!"long long", !20, i64 0}
!26 = !{!18, !22, i64 8}
!27 = !{!28, !33, i64 104}
!28 = !{!"struct@?AU?$start_for@V?$blocked_range@H@d1@detail@tbb@@VArraySummer@@$$CBVauto_partitioner@234@@d1@detail@tbb@@", !29, i64 64, !31, i64 80, !33, i64 104, !34, i64 112, !35, i64 128}
!29 = !{!"struct@?AV?$blocked_range@H@d1@detail@tbb@@", !30, i64 0, !30, i64 4, !25, i64 8}
!30 = !{!"int", !20, i64 0}
!31 = !{!"struct@?AVArraySummer@@", !32, i64 0, !32, i64 8, !32, i64 16}
!32 = !{!"pointer@?APEAH", !20, i64 0}
!33 = !{!"pointer@?APEAUnode@d1@detail@tbb@@", !20, i64 0}
!34 = !{!"struct@?AVauto_partition_type@d1@detail@tbb@@"}
!35 = !{!"struct@?AVsmall_object_allocator@d1@detail@tbb@@", !36, i64 0}
!36 = !{!"pointer@?APEAVsmall_object_pool@d1@detail@tbb@@", !20, i64 0}
!37 = !{!38, !20, i64 12}
!38 = !{!"struct@?AU?$dynamic_grainsize_mode@U?$adaptive_mode@Vauto_partition_type@d1@detail@tbb@@@d1@detail@tbb@@@d1@detail@tbb@@", !39, i64 8, !20, i64 12}
!39 = !{!"?AW4<unnamed-type-my_delay>@?$dynamic_grainsize_mode@U?$adaptive_mode@Vauto_partition_type@d1@detail@tbb@@@d1@detail@tbb@@@d1@detail@tbb@@", !20, i64 0}
!40 = !{!28, !29, i64 64}
!41 = !{!28, !25, i64 72}
!42 = !{!28, !30, i64 64}
!43 = !{!28, !30, i64 68}
!44 = !{!35, !36, i64 0}
!45 = !{!25, !25, i64 0}
!46 = !{!47, !47, i64 0}
!47 = !{!"vtable pointer", !21, i64 0}
!48 = !{!38, !39, i64 8}
!49 = !{!36, !36, i64 0}
!50 = !{!51, !33, i64 0}
!51 = !{!"struct@?AUnode@d1@detail@tbb@@", !33, i64 0, !52, i64 8}
!52 = !{!"struct@?AU?$atomic@H@std@@"}
!53 = !{!54, !30, i64 0}
!54 = !{!"struct@?AU?$_Atomic_storage@H$03@std@@", !55, i64 0}
!55 = !{!"struct@?AU?$_Atomic_padded@H@std@@", !30, i64 0}
!56 = !{!57, !59, i64 0}
!57 = !{!"struct@?AU?$_Atomic_storage@_N$00@std@@", !58, i64 0}
!58 = !{!"struct@?AU?$_Atomic_padded@_N@std@@", !59, i64 0}
!59 = !{!"bool", !20, i64 0}
!60 = !{!18, !19, i64 0}
!61 = !{!28, !32, i64 96}
!62 = !{!28, !32, i64 88}
!63 = !{!28, !32, i64 80}
!64 = !{!30, !30, i64 0}
!65 = distinct !{!65, !66}
!66 = !{!"llvm.loop.mustprogress"}
!67 = !{!68, !20, i64 0}
!68 = !{!"struct@?AV?$range_vector@V?$blocked_range@H@d1@detail@tbb@@$07@d1@detail@tbb@@", !20, i64 0, !20, i64 1, !20, i64 2, !69, i64 3, !70, i64 16}
!69 = !{!"array@?AY07E", !20, i64 0}
!70 = !{!"struct@?AV?$aligned_space@V?$blocked_range@H@d1@detail@tbb@@$07@d0@detail@tbb@@", !71, i64 0}
!71 = !{!"array@?AY0IA@E", !20, i64 0}
!72 = !{!68, !20, i64 1}
!73 = !{!68, !20, i64 2}
!74 = !{!68, !20, i64 3}
!75 = distinct !{!75, !66}
!76 = !{!29, !25, i64 8}
!77 = !{!29, !30, i64 0}
!78 = !{!29, !30, i64 4}
!79 = !{!29, !29, i64 0}
!80 = distinct !{!80, !66}
; end INTEL_FEATURE_SW_ADVANCED
