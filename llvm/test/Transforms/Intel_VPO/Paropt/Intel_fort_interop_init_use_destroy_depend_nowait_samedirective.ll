; INTEL_CUSTOMIZATION
; RUN: opt -enable-new-pm=0 -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s
;
;Test SRC:
;
;subroutine foo(obj1, obj2, obj3)
;    use omp_lib
;    implicit none
;    integer(kind=8)::obj1, obj2, obj3
;
;    !$omp interop init(targetsync:obj1) use(obj2) destroy(obj3) nowait &
;    !$omp& depend(in:obj3)
;
;  end subroutine

source_filename = "/tmp/ifxdvGp3R.i90"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@strlit = internal unnamed_addr constant [13 x i8] c"kmp_hw_switch"
@strlit.1 = internal unnamed_addr constant [27 x i8] c"kmp_get_cancellation_status"
@strlit.2 = internal unnamed_addr constant [20 x i8] c"kmp_set_warnings_off"
@strlit.3 = internal unnamed_addr constant [19 x i8] c"kmp_set_warnings_on"
@strlit.4 = internal unnamed_addr constant [8 x i8] c"kmp_free"
@strlit.5 = internal unnamed_addr constant [11 x i8] c"kmp_realloc"
@strlit.6 = internal unnamed_addr constant [10 x i8] c"kmp_calloc"
@strlit.7 = internal unnamed_addr constant [18 x i8] c"kmp_aligned_malloc"
@strlit.8 = internal unnamed_addr constant [10 x i8] c"kmp_malloc"
@strlit.9 = internal unnamed_addr constant [26 x i8] c"kmp_get_affinity_mask_proc"
@strlit.10 = internal unnamed_addr constant [28 x i8] c"kmp_unset_affinity_mask_proc"
@strlit.11 = internal unnamed_addr constant [26 x i8] c"kmp_set_affinity_mask_proc"
@strlit.12 = internal unnamed_addr constant [25 x i8] c"kmp_destroy_affinity_mask"
@strlit.13 = internal unnamed_addr constant [24 x i8] c"kmp_create_affinity_mask"
@strlit.14 = internal unnamed_addr constant [25 x i8] c"kmp_get_affinity_max_proc"
@strlit.15 = internal unnamed_addr constant [16 x i8] c"kmp_get_affinity"
@strlit.16 = internal unnamed_addr constant [16 x i8] c"kmp_set_affinity"
@strlit.17 = internal unnamed_addr constant [24 x i8] c"kmp_set_disp_num_buffers"
@strlit.18 = internal unnamed_addr constant [15 x i8] c"kmp_get_library"
@strlit.19 = internal unnamed_addr constant [17 x i8] c"kmp_get_blocktime"
@strlit.20 = internal unnamed_addr constant [19 x i8] c"kmp_get_stacksize_s"
@strlit.21 = internal unnamed_addr constant [17 x i8] c"kmp_get_stacksize"
@strlit.22 = internal unnamed_addr constant [16 x i8] c"kmp_set_defaults"
@strlit.23 = internal unnamed_addr constant [15 x i8] c"kmp_set_library"
@strlit.24 = internal unnamed_addr constant [26 x i8] c"kmp_set_library_throughput"
@strlit.25 = internal unnamed_addr constant [26 x i8] c"kmp_set_library_turnaround"
@strlit.26 = internal unnamed_addr constant [22 x i8] c"kmp_set_library_serial"
@strlit.27 = internal unnamed_addr constant [17 x i8] c"kmp_set_blocktime"
@strlit.28 = internal unnamed_addr constant [19 x i8] c"kmp_set_stacksize_s"
@strlit.29 = internal unnamed_addr constant [17 x i8] c"kmp_set_stacksize"
@strlit.30 = internal unnamed_addr constant [23 x i8] c"omp_target_alloc_device"
@strlit.31 = internal unnamed_addr constant [23 x i8] c"omp_target_alloc_shared"
@strlit.32 = internal unnamed_addr constant [21 x i8] c"omp_target_alloc_host"
@strlit.33 = internal unnamed_addr constant [8 x i8] c"omp_free"
@strlit.34 = internal unnamed_addr constant [11 x i8] c"omp_realloc"
@strlit.35 = internal unnamed_addr constant [18 x i8] c"omp_aligned_calloc"
@strlit.36 = internal unnamed_addr constant [10 x i8] c"omp_calloc"
@strlit.37 = internal unnamed_addr constant [17 x i8] c"omp_aligned_alloc"
@strlit.38 = internal unnamed_addr constant [9 x i8] c"omp_alloc"
@strlit.39 = internal unnamed_addr constant [27 x i8] c"omp_target_disassociate_ptr"
@strlit.40 = internal unnamed_addr constant [18 x i8] c"omp_get_mapped_ptr"
@strlit.41 = internal unnamed_addr constant [24 x i8] c"omp_target_associate_ptr"
@strlit.42 = internal unnamed_addr constant [28 x i8] c"omp_target_memcpy_rect_async"
@strlit.43 = internal unnamed_addr constant [23 x i8] c"omp_target_memcpy_async"
@strlit.44 = internal unnamed_addr constant [22 x i8] c"omp_target_memcpy_rect"
@strlit.45 = internal unnamed_addr constant [17 x i8] c"omp_target_memcpy"
@strlit.46 = internal unnamed_addr constant [21 x i8] c"omp_target_is_present"
@strlit.47 = internal unnamed_addr constant [15 x i8] c"omp_target_free"
@strlit.48 = internal unnamed_addr constant [16 x i8] c"omp_target_alloc"
@strlit.49 = internal unnamed_addr constant [15 x i8] c"omp_display_env"
@strlit.50 = internal unnamed_addr constant [26 x i8] c"omp_get_teams_thread_limit"
@strlit.51 = internal unnamed_addr constant [26 x i8] c"omp_set_teams_thread_limit"
@strlit.52 = internal unnamed_addr constant [17 x i8] c"omp_get_max_teams"
@strlit.53 = internal unnamed_addr constant [17 x i8] c"omp_set_num_teams"
@strlit.54 = internal unnamed_addr constant [25 x i8] c"omp_get_default_allocator"
@strlit.55 = internal unnamed_addr constant [25 x i8] c"omp_set_default_allocator"
@strlit.56 = internal unnamed_addr constant [21 x i8] c"omp_destroy_allocator"
@strlit.57 = internal unnamed_addr constant [16 x i8] c"omp_control_tool"
@strlit.58 = internal unnamed_addr constant [28 x i8] c"omp_init_nest_lock_with_hint"
@strlit.59 = internal unnamed_addr constant [23 x i8] c"omp_init_lock_with_hint"
@strlit.60 = internal unnamed_addr constant [25 x i8] c"omp_get_max_task_priority"
@strlit.61 = internal unnamed_addr constant [18 x i8] c"omp_test_nest_lock"
@strlit.62 = internal unnamed_addr constant [19 x i8] c"omp_unset_nest_lock"
@strlit.63 = internal unnamed_addr constant [17 x i8] c"omp_set_nest_lock"
@strlit.64 = internal unnamed_addr constant [21 x i8] c"omp_destroy_nest_lock"
@strlit.65 = internal unnamed_addr constant [18 x i8] c"omp_init_nest_lock"
@strlit.66 = internal unnamed_addr constant [13 x i8] c"omp_test_lock"
@strlit.67 = internal unnamed_addr constant [14 x i8] c"omp_unset_lock"
@strlit.68 = internal unnamed_addr constant [12 x i8] c"omp_set_lock"
@strlit.69 = internal unnamed_addr constant [16 x i8] c"omp_destroy_lock"
@strlit.70 = internal unnamed_addr constant [13 x i8] c"omp_init_lock"
@strlit.71 = internal unnamed_addr constant [17 x i8] c"omp_fulfill_event"
@strlit.72 = internal unnamed_addr constant [31 x i8] c"omp_get_supported_active_levels"
@strlit.73 = internal unnamed_addr constant [22 x i8] c"omp_pause_resource_all"
@strlit.74 = internal unnamed_addr constant [18 x i8] c"omp_pause_resource"
@strlit.75 = internal unnamed_addr constant [18 x i8] c"omp_get_device_num"
@strlit.76 = internal unnamed_addr constant [22 x i8] c"omp_get_initial_device"
@strlit.77 = internal unnamed_addr constant [21 x i8] c"omp_is_initial_device"
@strlit.78 = internal unnamed_addr constant [20 x i8] c"omp_get_cancellation"
@strlit.79 = internal unnamed_addr constant [16 x i8] c"omp_get_team_num"
@strlit.80 = internal unnamed_addr constant [17 x i8] c"omp_get_num_teams"
@strlit.81 = internal unnamed_addr constant [19 x i8] c"omp_get_num_devices"
@strlit.82 = internal unnamed_addr constant [22 x i8] c"omp_set_default_device"
@strlit.83 = internal unnamed_addr constant [22 x i8] c"omp_get_default_device"
@strlit.84 = internal unnamed_addr constant [13 x i8] c"omp_get_wtick"
@strlit.85 = internal unnamed_addr constant [13 x i8] c"omp_get_wtime"
@strlit.86 = internal unnamed_addr constant [28 x i8] c"omp_get_partition_place_nums"
@strlit.87 = internal unnamed_addr constant [28 x i8] c"omp_get_partition_num_places"
@strlit.88 = internal unnamed_addr constant [17 x i8] c"omp_get_place_num"
@strlit.89 = internal unnamed_addr constant [22 x i8] c"omp_get_place_proc_ids"
@strlit.90 = internal unnamed_addr constant [23 x i8] c"omp_get_place_num_procs"
@strlit.91 = internal unnamed_addr constant [18 x i8] c"omp_get_num_places"
@strlit.92 = internal unnamed_addr constant [17 x i8] c"omp_get_proc_bind"
@strlit.93 = internal unnamed_addr constant [16 x i8] c"omp_get_schedule"
@strlit.94 = internal unnamed_addr constant [16 x i8] c"omp_set_schedule"
@strlit.95 = internal unnamed_addr constant [17 x i8] c"omp_get_team_size"
@strlit.96 = internal unnamed_addr constant [27 x i8] c"omp_get_ancestor_thread_num"
@strlit.97 = internal unnamed_addr constant [20 x i8] c"omp_get_active_level"
@strlit.98 = internal unnamed_addr constant [13 x i8] c"omp_get_level"
@strlit.99 = internal unnamed_addr constant [25 x i8] c"omp_get_max_active_levels"
@strlit.100 = internal unnamed_addr constant [25 x i8] c"omp_set_max_active_levels"
@strlit.101 = internal unnamed_addr constant [20 x i8] c"omp_get_thread_limit"
@strlit.102 = internal unnamed_addr constant [14 x i8] c"omp_get_nested"
@strlit.103 = internal unnamed_addr constant [15 x i8] c"omp_get_dynamic"
@strlit.104 = internal unnamed_addr constant [12 x i8] c"omp_in_final"
@strlit.105 = internal unnamed_addr constant [15 x i8] c"omp_in_parallel"
@strlit.106 = internal unnamed_addr constant [17 x i8] c"omp_get_num_procs"
@strlit.107 = internal unnamed_addr constant [18 x i8] c"omp_get_thread_num"
@strlit.108 = internal unnamed_addr constant [19 x i8] c"omp_get_max_threads"
@strlit.109 = internal unnamed_addr constant [19 x i8] c"omp_get_num_threads"
@strlit.110 = internal unnamed_addr constant [14 x i8] c"omp_set_nested"
@strlit.111 = internal unnamed_addr constant [15 x i8] c"omp_set_dynamic"
@strlit.112 = internal unnamed_addr constant [19 x i8] c"omp_set_num_threads"

; Function Attrs: noinline nounwind uwtable
define void @foo_(i64* dereferenceable(8) %"foo_$OBJ1", i64* dereferenceable(8) %"foo_$OBJ2", i64* dereferenceable(8) %"foo_$OBJ3") #0 {
alloca_0:
  %"$io_ctx" = alloca [8 x i64], align 8
  br label %bb1

bb1:                                              ; preds = %alloca_0
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.INTEROP"(), "QUAL.OMP.INIT:TARGETSYNC"(i64* %"foo_$OBJ1"), "QUAL.OMP.USE"(i64* %"foo_$OBJ2"), "QUAL.OMP.DESTROY"(i64* %"foo_$OBJ3"), "QUAL.OMP.NOWAIT"(), "QUAL.OMP.DEPEND.IN"(i64* %"foo_$OBJ3") ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.INTEROP"() ]

;CHECK: call void @__kmpc_omp_task_begin_if0(%struct.ident_t* @{{[^ ,]+}}, i32 %{{[^ ,]+}}, i8* %{{[^ ,]+}})
;CHECK-NEXT:  %[[INTEROP_CAST:[^ ]+]] = bitcast i64* %{{[^ ,]+}} to i8**
;CHECK-NEXT:  %[[INTEROP_OBJ:[^ ]+]] = call i8* @__tgt_create_interop(i64 %{{[^ ,]+}}, i32 1, i32 0, i8* null)
;CHECK-NEXT:  store i8* %[[INTEROP_OBJ]], i8** %[[INTEROP_CAST]], align 8
;CHECK-NEXT:  %[[INTEROP_CAST2:[^ ]+]] = bitcast i64* %{{[^ ,]+}} to i8**
;CHECK-NEXT:  %[[INTEROP_OBJ_VAL1:[^ ]+]] = load i8*, i8** %[[INTEROP_CAST2]], align 8
;CHECK-NEXT:  %{{[^ ,]+}} = call i32 @__tgt_use_interop(i8* %[[INTEROP_OBJ_VAL1]])
;CHECK-NEXT:  %[[INTEROP_CAST3:[^ ]+]] = bitcast i64* %{{[^ ,]+}} to i8**
;CHECK-NEXT:  %[[INTEROP_OBJ_VAL2:[^ ]+]] = load i8*, i8** %[[INTEROP_CAST3]], align 8
;CHECK-NEXT:  %{{[^ ,]+}} = call i32 @__tgt_release_interop(i8* %[[INTEROP_OBJ_VAL2]])
;CHECK-NEXT:  store i8* null, i8** %[[INTEROP_CAST3]], align 8
;CHECK:  call void @__kmpc_omp_task_complete_if0(%struct.ident_t* @{{[^ ,]+}}, i32 %{{[^ ,]+}}, i8* %{{[^ ,]+}})

  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token %0) #1

attributes #0 = { noinline nounwind uwtable "frame-pointer"="all" "lang"="fortran" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { nounwind }

!omp_offload.info = !{}
; end INTEL_CUSTOMIZATION
