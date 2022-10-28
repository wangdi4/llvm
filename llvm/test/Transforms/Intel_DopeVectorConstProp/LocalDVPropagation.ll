; REQUIRES: asserts
; RUN: opt < %s -opaque-pointers -disable-output -passes=localdvprop -debug-only=dopevectorconstprop -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 2>&1 | FileCheck %s

; CHECK: USES OF:   %"main_$C.reserved$_fetch.37" = load i64, ptr %"main_$C.reserved$", align 8, !tbaa !119, !llfort.type_idx !79
; CHECK-NEXT: IS REPLACED BY: i64 0
; CHECK: USES OF:   %"main_$C.dim_info$.lower_bound$152[]_fetch.42" = load i64, ptr %"main_$C.dim_info$.lower_bound$152[]", align 8, !tbaa !127, !llfort.type_idx !135
; CHECK-NEXT: IS REPLACED BY: i64 1
; CHECK: USES OF:   %"main_$C.dim_info$.lower_bound$156[]_fetch.43" = load i64, ptr %"main_$C.dim_info$.lower_bound$156[]", align 8, !tbaa !127, !llfort.type_idx !136
; CHECK-NEXT: IS REPLACED BY: i64 1
; CHECK: USES OF:   %"main_$C.dim_info$.spacing$160[]_fetch.44" = load i64, ptr %"main_$C.dim_info$.spacing$160[]", align 8, !tbaa !133, !range !97, !llfort.type_idx !137
; CHECK-NEXT: IS REPLACED BY:   %mul.10 = mul nsw i64 8, %slct.11
; CHECK: USES OF:   %"main_$B.reserved$_fetch.23" = load i64, ptr %"main_$B.reserved$", align 8, !tbaa !100, !llfort.type_idx !79
; CHECK-NEXT: IS REPLACED BY: i64 0
; CHECK: USES OF:   %"main_$B.dim_info$.lower_bound$97[]_fetch.28" = load i64, ptr %"main_$B.dim_info$.lower_bound$97[]", align 8, !tbaa !108, !llfort.type_idx !116
; CHECK-NEXT: IS REPLACED BY: i64 1
; CHECK: USES OF:   %"main_$B.dim_info$.lower_bound$101[]_fetch.29" = load i64, ptr %"main_$B.dim_info$.lower_bound$101[]", align 8, !tbaa !108, !llfort.type_idx !117
; CHECK-NEXT: IS REPLACED BY: i64 1
; CHECK: USES OF:   %"main_$B.dim_info$.spacing$105[]_fetch.30" = load i64, ptr %"main_$B.dim_info$.spacing$105[]", align 8, !tbaa !114, !range !97, !llfort.type_idx !118
; CHECK-NEXT: IS REPLACED BY:   %mul.6 = mul nsw i64 8, %slct.7
; CHECK: USES OF:   %"main_$A.reserved$_fetch.9" = load i64, ptr %"main_$A.reserved$", align 8, !tbaa !70, !llfort.type_idx !79
; CHECK-NEXT: IS REPLACED BY: i64 0
; CHECK: USES OF:   %"main_$A.dim_info$.lower_bound$42[]_fetch.14" = load i64, ptr %"main_$A.dim_info$.lower_bound$42[]", align 8, !tbaa !86, !llfort.type_idx !94
; CHECK-NEXT: IS REPLACED BY: i64 1
; CHECK: USES OF:   %"main_$A.dim_info$.lower_bound$46[]_fetch.15" = load i64, ptr %"main_$A.dim_info$.lower_bound$46[]", align 8, !tbaa !86, !llfort.type_idx !95
; CHECK-NEXT: IS REPLACED BY: i64 1
; CHECK: USES OF:   %"main_$A.dim_info$.spacing$50[]_fetch.16" = load i64, ptr %"main_$A.dim_info$.spacing$50[]", align 8, !tbaa !92, !range !97, !llfort.type_idx !96
; CHECK-NEXT: IS REPLACED BY:   %mul.2 = mul nsw i64 8, %slct.3


; ModuleID = 'simd_autovectorize.bc'
source_filename = "simd_autovectorize.bc"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

%"QNCA_a0$i8*$rank2$" = type { i8*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }
%"QNCA_a0$i64*$rank2$" = type { i64*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }

@strlit = internal unnamed_addr constant [27 x i8] c"kmp_get_cancellation_status", !llfort.type_idx !0
@strlit.1 = internal unnamed_addr constant [20 x i8] c"kmp_set_warnings_off", !llfort.type_idx !1
@strlit.2 = internal unnamed_addr constant [19 x i8] c"kmp_set_warnings_on", !llfort.type_idx !2
@strlit.3 = internal unnamed_addr constant [8 x i8] c"kmp_free", !llfort.type_idx !3
@strlit.4 = internal unnamed_addr constant [11 x i8] c"kmp_realloc", !llfort.type_idx !4
@strlit.5 = internal unnamed_addr constant [10 x i8] c"kmp_calloc", !llfort.type_idx !5
@strlit.6 = internal unnamed_addr constant [18 x i8] c"kmp_aligned_malloc", !llfort.type_idx !6
@strlit.7 = internal unnamed_addr constant [10 x i8] c"kmp_malloc", !llfort.type_idx !5
@strlit.8 = internal unnamed_addr constant [26 x i8] c"kmp_get_affinity_mask_proc", !llfort.type_idx !7
@strlit.9 = internal unnamed_addr constant [28 x i8] c"kmp_unset_affinity_mask_proc", !llfort.type_idx !8
@strlit.10 = internal unnamed_addr constant [26 x i8] c"kmp_set_affinity_mask_proc", !llfort.type_idx !7
@strlit.11 = internal unnamed_addr constant [25 x i8] c"kmp_destroy_affinity_mask", !llfort.type_idx !9
@strlit.12 = internal unnamed_addr constant [24 x i8] c"kmp_create_affinity_mask", !llfort.type_idx !10
@strlit.13 = internal unnamed_addr constant [25 x i8] c"kmp_get_affinity_max_proc", !llfort.type_idx !9
@strlit.14 = internal unnamed_addr constant [16 x i8] c"kmp_get_affinity", !llfort.type_idx !11
@strlit.15 = internal unnamed_addr constant [16 x i8] c"kmp_set_affinity", !llfort.type_idx !11
@strlit.16 = internal unnamed_addr constant [24 x i8] c"kmp_set_disp_num_buffers", !llfort.type_idx !10
@strlit.17 = internal unnamed_addr constant [15 x i8] c"kmp_get_library", !llfort.type_idx !12
@strlit.18 = internal unnamed_addr constant [17 x i8] c"kmp_get_blocktime", !llfort.type_idx !13
@strlit.19 = internal unnamed_addr constant [19 x i8] c"kmp_get_stacksize_s", !llfort.type_idx !2
@strlit.20 = internal unnamed_addr constant [17 x i8] c"kmp_get_stacksize", !llfort.type_idx !13
@strlit.21 = internal unnamed_addr constant [16 x i8] c"kmp_set_defaults", !llfort.type_idx !11
@strlit.22 = internal unnamed_addr constant [15 x i8] c"kmp_set_library", !llfort.type_idx !12
@strlit.23 = internal unnamed_addr constant [26 x i8] c"kmp_set_library_throughput", !llfort.type_idx !7
@strlit.24 = internal unnamed_addr constant [26 x i8] c"kmp_set_library_turnaround", !llfort.type_idx !7
@strlit.25 = internal unnamed_addr constant [22 x i8] c"kmp_set_library_serial", !llfort.type_idx !14
@strlit.26 = internal unnamed_addr constant [17 x i8] c"kmp_set_blocktime", !llfort.type_idx !13
@strlit.27 = internal unnamed_addr constant [19 x i8] c"kmp_set_stacksize_s", !llfort.type_idx !2
@strlit.28 = internal unnamed_addr constant [17 x i8] c"kmp_set_stacksize", !llfort.type_idx !13
@strlit.29 = internal unnamed_addr constant [23 x i8] c"ompx_get_num_subdevices", !llfort.type_idx !15
@strlit.30 = internal unnamed_addr constant [32 x i8] c"ompx_target_aligned_alloc_shared", !llfort.type_idx !16
@strlit.31 = internal unnamed_addr constant [30 x i8] c"ompx_target_aligned_alloc_host", !llfort.type_idx !17
@strlit.32 = internal unnamed_addr constant [32 x i8] c"ompx_target_aligned_alloc_device", !llfort.type_idx !16
@strlit.33 = internal unnamed_addr constant [25 x i8] c"ompx_target_aligned_alloc", !llfort.type_idx !9
@strlit.34 = internal unnamed_addr constant [26 x i8] c"ompx_target_realloc_shared", !llfort.type_idx !7
@strlit.35 = internal unnamed_addr constant [24 x i8] c"ompx_target_realloc_host", !llfort.type_idx !10
@strlit.36 = internal unnamed_addr constant [26 x i8] c"ompx_target_realloc_device", !llfort.type_idx !7
@strlit.37 = internal unnamed_addr constant [19 x i8] c"ompx_target_realloc", !llfort.type_idx !2
@strlit.38 = internal unnamed_addr constant [23 x i8] c"omp_target_alloc_device", !llfort.type_idx !15
@strlit.39 = internal unnamed_addr constant [23 x i8] c"omp_target_alloc_shared", !llfort.type_idx !15
@strlit.40 = internal unnamed_addr constant [21 x i8] c"omp_target_alloc_host", !llfort.type_idx !18
@strlit.41 = internal unnamed_addr constant [20 x i8] c"omp_in_explicit_task", !llfort.type_idx !1
@strlit.42 = internal unnamed_addr constant [8 x i8] c"omp_free", !llfort.type_idx !3
@strlit.43 = internal unnamed_addr constant [11 x i8] c"omp_realloc", !llfort.type_idx !4
@strlit.44 = internal unnamed_addr constant [18 x i8] c"omp_aligned_calloc", !llfort.type_idx !6
@strlit.45 = internal unnamed_addr constant [10 x i8] c"omp_calloc", !llfort.type_idx !5
@strlit.46 = internal unnamed_addr constant [17 x i8] c"omp_aligned_alloc", !llfort.type_idx !13
@strlit.47 = internal unnamed_addr constant [9 x i8] c"omp_alloc", !llfort.type_idx !19
@strlit.48 = internal unnamed_addr constant [24 x i8] c"omp_target_is_accessible", !llfort.type_idx !10
@strlit.49 = internal unnamed_addr constant [27 x i8] c"omp_target_disassociate_ptr", !llfort.type_idx !0
@strlit.50 = internal unnamed_addr constant [18 x i8] c"omp_get_mapped_ptr", !llfort.type_idx !6
@strlit.51 = internal unnamed_addr constant [24 x i8] c"omp_target_associate_ptr", !llfort.type_idx !10
@strlit.52 = internal unnamed_addr constant [28 x i8] c"omp_target_memcpy_rect_async", !llfort.type_idx !8
@strlit.53 = internal unnamed_addr constant [23 x i8] c"omp_target_memcpy_async", !llfort.type_idx !15
@strlit.54 = internal unnamed_addr constant [22 x i8] c"omp_target_memcpy_rect", !llfort.type_idx !14
@strlit.55 = internal unnamed_addr constant [17 x i8] c"omp_target_memcpy", !llfort.type_idx !13
@strlit.56 = internal unnamed_addr constant [21 x i8] c"omp_target_is_present", !llfort.type_idx !18
@strlit.57 = internal unnamed_addr constant [15 x i8] c"omp_target_free", !llfort.type_idx !12
@strlit.58 = internal unnamed_addr constant [16 x i8] c"omp_target_alloc", !llfort.type_idx !11
@strlit.59 = internal unnamed_addr constant [15 x i8] c"omp_display_env", !llfort.type_idx !12
@strlit.60 = internal unnamed_addr constant [26 x i8] c"omp_get_teams_thread_limit", !llfort.type_idx !7
@strlit.61 = internal unnamed_addr constant [26 x i8] c"omp_set_teams_thread_limit", !llfort.type_idx !7
@strlit.62 = internal unnamed_addr constant [17 x i8] c"omp_get_max_teams", !llfort.type_idx !13
@strlit.63 = internal unnamed_addr constant [17 x i8] c"omp_set_num_teams", !llfort.type_idx !13
@strlit.64 = internal unnamed_addr constant [25 x i8] c"omp_get_default_allocator", !llfort.type_idx !9
@strlit.65 = internal unnamed_addr constant [25 x i8] c"omp_set_default_allocator", !llfort.type_idx !9
@strlit.66 = internal unnamed_addr constant [21 x i8] c"omp_destroy_allocator", !llfort.type_idx !18
@strlit.67 = internal unnamed_addr constant [16 x i8] c"omp_control_tool", !llfort.type_idx !11
@strlit.68 = internal unnamed_addr constant [28 x i8] c"omp_init_nest_lock_with_hint", !llfort.type_idx !8
@strlit.69 = internal unnamed_addr constant [23 x i8] c"omp_init_lock_with_hint", !llfort.type_idx !15
@strlit.70 = internal unnamed_addr constant [25 x i8] c"omp_get_max_task_priority", !llfort.type_idx !9
@strlit.71 = internal unnamed_addr constant [18 x i8] c"omp_test_nest_lock", !llfort.type_idx !6
@strlit.72 = internal unnamed_addr constant [19 x i8] c"omp_unset_nest_lock", !llfort.type_idx !2
@strlit.73 = internal unnamed_addr constant [17 x i8] c"omp_set_nest_lock", !llfort.type_idx !13
@strlit.74 = internal unnamed_addr constant [21 x i8] c"omp_destroy_nest_lock", !llfort.type_idx !18
@strlit.75 = internal unnamed_addr constant [18 x i8] c"omp_init_nest_lock", !llfort.type_idx !6
@strlit.76 = internal unnamed_addr constant [13 x i8] c"omp_test_lock", !llfort.type_idx !20
@strlit.77 = internal unnamed_addr constant [14 x i8] c"omp_unset_lock", !llfort.type_idx !21
@strlit.78 = internal unnamed_addr constant [12 x i8] c"omp_set_lock", !llfort.type_idx !22
@strlit.79 = internal unnamed_addr constant [16 x i8] c"omp_destroy_lock", !llfort.type_idx !11
@strlit.80 = internal unnamed_addr constant [13 x i8] c"omp_init_lock", !llfort.type_idx !20
@strlit.81 = internal unnamed_addr constant [17 x i8] c"omp_fulfill_event", !llfort.type_idx !13
@strlit.82 = internal unnamed_addr constant [31 x i8] c"omp_get_supported_active_levels", !llfort.type_idx !23
@strlit.83 = internal unnamed_addr constant [22 x i8] c"omp_pause_resource_all", !llfort.type_idx !14
@strlit.84 = internal unnamed_addr constant [18 x i8] c"omp_pause_resource", !llfort.type_idx !6
@strlit.85 = internal unnamed_addr constant [18 x i8] c"omp_get_device_num", !llfort.type_idx !6
@strlit.86 = internal unnamed_addr constant [22 x i8] c"omp_get_initial_device", !llfort.type_idx !14
@strlit.87 = internal unnamed_addr constant [21 x i8] c"omp_is_initial_device", !llfort.type_idx !18
@strlit.88 = internal unnamed_addr constant [20 x i8] c"omp_get_cancellation", !llfort.type_idx !1
@strlit.89 = internal unnamed_addr constant [16 x i8] c"omp_get_team_num", !llfort.type_idx !11
@strlit.90 = internal unnamed_addr constant [17 x i8] c"omp_get_num_teams", !llfort.type_idx !13
@strlit.91 = internal unnamed_addr constant [19 x i8] c"omp_get_num_devices", !llfort.type_idx !2
@strlit.92 = internal unnamed_addr constant [22 x i8] c"omp_set_default_device", !llfort.type_idx !14
@strlit.93 = internal unnamed_addr constant [22 x i8] c"omp_get_default_device", !llfort.type_idx !14
@strlit.94 = internal unnamed_addr constant [13 x i8] c"omp_get_wtick", !llfort.type_idx !20
@strlit.95 = internal unnamed_addr constant [13 x i8] c"omp_get_wtime", !llfort.type_idx !20
@strlit.96 = internal unnamed_addr constant [28 x i8] c"omp_get_partition_place_nums", !llfort.type_idx !8
@strlit.97 = internal unnamed_addr constant [28 x i8] c"omp_get_partition_num_places", !llfort.type_idx !8
@strlit.98 = internal unnamed_addr constant [17 x i8] c"omp_get_place_num", !llfort.type_idx !13
@strlit.99 = internal unnamed_addr constant [22 x i8] c"omp_get_place_proc_ids", !llfort.type_idx !14
@strlit.100 = internal unnamed_addr constant [23 x i8] c"omp_get_place_num_procs", !llfort.type_idx !15
@strlit.101 = internal unnamed_addr constant [18 x i8] c"omp_get_num_places", !llfort.type_idx !6
@strlit.102 = internal unnamed_addr constant [17 x i8] c"omp_get_proc_bind", !llfort.type_idx !13
@strlit.103 = internal unnamed_addr constant [16 x i8] c"omp_get_schedule", !llfort.type_idx !11
@strlit.104 = internal unnamed_addr constant [16 x i8] c"omp_set_schedule", !llfort.type_idx !11
@strlit.105 = internal unnamed_addr constant [17 x i8] c"omp_get_team_size", !llfort.type_idx !13
@strlit.106 = internal unnamed_addr constant [27 x i8] c"omp_get_ancestor_thread_num", !llfort.type_idx !0
@strlit.107 = internal unnamed_addr constant [20 x i8] c"omp_get_active_level", !llfort.type_idx !1
@strlit.108 = internal unnamed_addr constant [13 x i8] c"omp_get_level", !llfort.type_idx !20
@strlit.109 = internal unnamed_addr constant [25 x i8] c"omp_get_max_active_levels", !llfort.type_idx !9
@strlit.110 = internal unnamed_addr constant [25 x i8] c"omp_set_max_active_levels", !llfort.type_idx !9
@strlit.111 = internal unnamed_addr constant [20 x i8] c"omp_get_thread_limit", !llfort.type_idx !1
@strlit.112 = internal unnamed_addr constant [14 x i8] c"omp_get_nested", !llfort.type_idx !21
@strlit.113 = internal unnamed_addr constant [15 x i8] c"omp_get_dynamic", !llfort.type_idx !12
@strlit.114 = internal unnamed_addr constant [12 x i8] c"omp_in_final", !llfort.type_idx !22
@strlit.115 = internal unnamed_addr constant [15 x i8] c"omp_in_parallel", !llfort.type_idx !12
@strlit.116 = internal unnamed_addr constant [17 x i8] c"omp_get_num_procs", !llfort.type_idx !13
@strlit.117 = internal unnamed_addr constant [18 x i8] c"omp_get_thread_num", !llfort.type_idx !6
@strlit.118 = internal unnamed_addr constant [19 x i8] c"omp_get_max_threads", !llfort.type_idx !2
@strlit.119 = internal unnamed_addr constant [19 x i8] c"omp_get_num_threads", !llfort.type_idx !2
@strlit.120 = internal unnamed_addr constant [14 x i8] c"omp_set_nested", !llfort.type_idx !21
@strlit.121 = internal unnamed_addr constant [15 x i8] c"omp_set_dynamic", !llfort.type_idx !12
@strlit.122 = internal unnamed_addr constant [19 x i8] c"omp_set_num_threads", !llfort.type_idx !2
@strlit.123 = internal unnamed_addr constant [12 x i8] c"BW (GB/s) = ", !llfort.type_idx !22
@strlit.124 = internal unnamed_addr constant [7 x i8] c"time = ", !llfort.type_idx !24
@strlit.125 = internal unnamed_addr constant [2 x i8] c" B", !llfort.type_idx !25
@strlit.126 = internal unnamed_addr constant [8 x i8] c"bytes = ", !llfort.type_idx !3
@strlit.127 = internal unnamed_addr constant [2 x i8] c" B", !llfort.type_idx !25
@strlit.128 = internal unnamed_addr constant [8 x i8] c"bytes = ", !llfort.type_idx !3
@strlit.129 = internal unnamed_addr constant [17 x i8] c"incorrect results", !llfort.type_idx !13
@"main_$WARMUP" = internal global i32 10, align 8, !llfort.type_idx !26
@"main_$NO_MAX_REP" = internal global i32 100, align 8, !llfort.type_idx !27
@"main_$N" = internal global i64 10000, align 8, !llfort.type_idx !28
@"var$2" = internal global %"QNCA_a0$i8*$rank2$" { i8* null, i64 0, i64 0, i64 128, i64 2, i64 0, [2 x { i64, i64, i64 }] zeroinitializer }, !llfort.type_idx !29
@0 = internal unnamed_addr constant i32 65536
@1 = internal unnamed_addr constant i32 2


; Function Attrs: nounwind uwtable
define void @MAIN__() #0 !llfort.type_idx !32 {
alloca_0:
  %"$io_ctx" = alloca [8 x i64], align 16, !llfort.type_idx !33
  %"main_$IREP" = alloca i32, align 8, !llfort.type_idx !34
  %"main_$ERR" = alloca i32, align 8, !llfort.type_idx !35
  %"main_$TIME" = alloca double, align 8, !llfort.type_idx !36
  %"main_$BYTES" = alloca i64, align 8, !llfort.type_idx !37
  %"main_$J" = alloca i64, align 8, !llfort.type_idx !38
  %"main_$I" = alloca i64, align 8, !llfort.type_idx !39
  %"main_$M" = alloca i64, align 8, !llfort.type_idx !40
  %"main_$C" = alloca %"QNCA_a0$i64*$rank2$", align 8, !llfort.type_idx !41
  %"main_$B" = alloca %"QNCA_a0$i64*$rank2$", align 8, !llfort.type_idx !41
  %"main_$A" = alloca %"QNCA_a0$i64*$rank2$", align 8, !llfort.type_idx !41
  %"var$3" = alloca i64, align 8, !llfort.type_idx !42
  %"var$4" = alloca i64, align 8, !llfort.type_idx !42
  %"var$5" = alloca i64, align 8, !llfort.type_idx !42
  %"var$6" = alloca i64, align 8, !llfort.type_idx !42
  %"var$7" = alloca i64, align 8, !llfort.type_idx !42
  %"var$8" = alloca i64, align 8, !llfort.type_idx !42
  %"var$9" = alloca i64, align 8, !llfort.type_idx !42
  %"var$10" = alloca i64, align 8, !llfort.type_idx !42
  %"var$11" = alloca i64, align 8, !llfort.type_idx !42
  %"$loop_ctr" = alloca i64, align 8, !llfort.type_idx !42
  %"$loop_ctr164" = alloca i64, align 8, !llfort.type_idx !42
  %"var$14" = alloca i64, align 8, !llfort.type_idx !42
  %"var$15" = alloca i64, align 8, !llfort.type_idx !42
  %"var$16" = alloca i64, align 8, !llfort.type_idx !42
  %"var$17" = alloca i64, align 8, !llfort.type_idx !42
  %"var$18" = alloca i32, align 4, !llfort.type_idx !43
  %"var$23" = alloca i64, align 8, !llfort.type_idx !42
  %"var$24" = alloca i64, align 8, !llfort.type_idx !42
  %"var$25" = alloca i32, align 4, !llfort.type_idx !43
  %"(&)val$" = alloca [4 x i8], align 1, !llfort.type_idx !44
  %argblock = alloca <{ i64, i8* }>, align 8, !llfort.type_idx !45
  %"var$26" = alloca i32, align 4, !llfort.type_idx !43
  %"(&)val$505" = alloca [4 x i8], align 1, !llfort.type_idx !46
  %argblock506 = alloca <{ i64, i8* }>, align 8, !llfort.type_idx !45
  %"(&)val$513" = alloca [4 x i8], align 1, !llfort.type_idx !47
  %argblock514 = alloca <{ i64 }>, align 8, !llfort.type_idx !48
  %"(&)val$520" = alloca [4 x i8], align 1, !llfort.type_idx !49
  %argblock521 = alloca <{ i64, i8* }>, align 8, !llfort.type_idx !45
  %"var$27" = alloca i32, align 4, !llfort.type_idx !43
  %"(&)val$529" = alloca [4 x i8], align 1, !llfort.type_idx !50
  %argblock530 = alloca <{ i64, i8* }>, align 8, !llfort.type_idx !45
  %"(&)val$537" = alloca [4 x i8], align 1, !llfort.type_idx !51
  %argblock538 = alloca <{ double }>, align 8, !llfort.type_idx !52
  %"var$28" = alloca i32, align 4, !llfort.type_idx !43
  %"(&)val$544" = alloca [4 x i8], align 1, !llfort.type_idx !53
  %argblock545 = alloca <{ i64, i8* }>, align 8, !llfort.type_idx !45
  %"(&)val$552" = alloca [4 x i8], align 1, !llfort.type_idx !54
  %argblock553 = alloca <{ double }>, align 8, !llfort.type_idx !52
  %strlit.129_fetch.227 = load [17 x i8], [17 x i8]* @strlit.129, align 1, !tbaa !55, !llfort.type_idx !13
  %strlit.126_fetch.238 = load [8 x i8], [8 x i8]* @strlit.126, align 1, !tbaa !60, !llfort.type_idx !3
  %strlit.125_fetch.240 = load [2 x i8], [2 x i8]* @strlit.125, align 1, !tbaa !62, !llfort.type_idx !25
  %strlit.124_fetch.241 = load [7 x i8], [7 x i8]* @strlit.124, align 1, !tbaa !64, !llfort.type_idx !24
  %strlit.123_fetch.243 = load [12 x i8], [12 x i8]* @strlit.123, align 1, !tbaa !66, !llfort.type_idx !22
  %func_result = call i32 @for_set_fpe_(i32* @0), !llfort.type_idx !43
  %func_result2 = call i32 @for_set_reentrancy(i32* @1), !llfort.type_idx !43
  %fetch.1 = load %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* bitcast (%"QNCA_a0$i8*$rank2$"* @"var$2" to %"QNCA_a0$i64*$rank2$"*), align 1, !tbaa !68, !llfort.type_idx !41
  store %"QNCA_a0$i64*$rank2$" %fetch.1, %"QNCA_a0$i64*$rank2$"* %"main_$C", align 8, !tbaa !68
  %fetch.2 = load %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* bitcast (%"QNCA_a0$i8*$rank2$"* @"var$2" to %"QNCA_a0$i64*$rank2$"*), align 1, !tbaa !68, !llfort.type_idx !41
  store %"QNCA_a0$i64*$rank2$" %fetch.2, %"QNCA_a0$i64*$rank2$"* %"main_$B", align 8, !tbaa !68
  %fetch.3 = load %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* bitcast (%"QNCA_a0$i8*$rank2$"* @"var$2" to %"QNCA_a0$i64*$rank2$"*), align 1, !tbaa !68, !llfort.type_idx !41
  store %"QNCA_a0$i64*$rank2$" %fetch.3, %"QNCA_a0$i64*$rank2$"* %"main_$A", align 8, !tbaa !68
  %"main_$A.reserved$53" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$A", i32 0, i32 5
  store i64 0, i64* %"main_$A.reserved$53", align 8, !tbaa !70
  %"main_$N_fetch.4" = load i64, i64* @"main_$N", align 8, !tbaa !73, !llfort.type_idx !28
  %sub.1 = sub nsw i64 %"main_$N_fetch.4", 1
  %add.1 = add nsw i64 1, %sub.1
  %rel.1 = icmp sle i64 1, %"main_$N_fetch.4"
  %rel.2 = icmp sle i64 1, %"main_$N_fetch.4"
  %rel.3 = icmp ne i1 %rel.1, false
  %slct.1 = select i1 %rel.3, i64 %add.1, i64 0
  %"main_$N_fetch.5" = load i64, i64* @"main_$N", align 8, !tbaa !73, !llfort.type_idx !28
  %sub.2 = sub nsw i64 %"main_$N_fetch.5", 1
  %add.2 = add nsw i64 1, %sub.2
  %rel.4 = icmp sle i64 1, %"main_$N_fetch.5"
  %rel.5 = icmp sle i64 1, %"main_$N_fetch.5"
  %rel.6 = icmp ne i1 %rel.4, false
  %slct.2 = select i1 %rel.6, i64 %add.2, i64 0
  %mul.1 = mul nsw i64 8, %slct.1
  %func_result4 = call i32 (i64*, i32, ...) @for_check_mult_overflow64(i64* %"var$4", i32 3, i64 %slct.1, i64 %slct.2, i64 8), !llfort.type_idx !43
  %"var$4_fetch.6" = load i64, i64* %"var$4", align 8, !tbaa !75, !llfort.type_idx !42
  %"main_$A.addr_a0$" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$A", i32 0, i32 0
  %"main_$A.flags$" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$A", i32 0, i32 3
  %"main_$A.flags$_fetch.7" = load i64, i64* %"main_$A.flags$", align 8, !tbaa !76, !llfort.type_idx !77
  %"main_$A.flags$6" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$A", i32 0, i32 3
  %"main_$A.flags$6_fetch.8" = load i64, i64* %"main_$A.flags$6", align 8, !tbaa !76, !llfort.type_idx !77
  %and.1 = and i64 %"main_$A.flags$6_fetch.8", -68451041281
  %or.1 = or i64 %and.1, 1073741824
  %"main_$A.flags$8" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$A", i32 0, i32 3
  store i64 %or.1, i64* %"main_$A.flags$8", align 8, !tbaa !76
  %and.2 = and i64 %"main_$A.flags$_fetch.7", 1
  %shl.2 = shl i64 %and.2, 1
  %int_zext = trunc i64 %shl.2 to i32, !llfort.type_idx !78
  %or.2 = or i32 1, %int_zext
  %and.4 = and i32 %func_result4, 1
  %and.5 = and i32 %or.2, -17
  %shl.3 = shl i32 %and.4, 4
  %or.3 = or i32 %and.5, %shl.3
  %and.6 = and i64 %"main_$A.flags$_fetch.7", 256
  %lshr.1 = lshr i64 %and.6, 8
  %and.7 = and i32 %or.3, -2097153
  %shl.4 = shl i64 %lshr.1, 21
  %int_zext10 = trunc i64 %shl.4 to i32, !llfort.type_idx !78
  %or.4 = or i32 %and.7, %int_zext10
  %and.8 = and i64 %"main_$A.flags$_fetch.7", 1030792151040
  %lshr.2 = lshr i64 %and.8, 36
  %and.9 = and i32 %or.4, -31457281
  %shl.5 = shl i64 %lshr.2, 21
  %int_zext12 = trunc i64 %shl.5 to i32, !llfort.type_idx !78
  %or.5 = or i32 %and.9, %int_zext12
  %and.10 = and i64 %"main_$A.flags$_fetch.7", 1099511627776
  %lshr.3 = lshr i64 %and.10, 40
  %and.11 = and i32 %or.5, -33554433
  %shl.6 = shl i64 %lshr.3, 25
  %int_zext14 = trunc i64 %shl.6 to i32, !llfort.type_idx !78
  %or.6 = or i32 %and.11, %int_zext14
  %and.12 = and i32 %or.6, -2031617
  %or.7 = or i32 %and.12, 262144
  %"main_$A.reserved$" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$A", i32 0, i32 5
  %"main_$A.reserved$_fetch.9" = load i64, i64* %"main_$A.reserved$", align 8, !tbaa !70, !llfort.type_idx !79
  %"(i8*)main_$A.reserved$_fetch.9$" = inttoptr i64 %"main_$A.reserved$_fetch.9" to i8*, !llfort.type_idx !80
  %"(i8**)main_$A.addr_a0$$" = bitcast i64** %"main_$A.addr_a0$" to i8**, !llfort.type_idx !81
  %func_result16 = call i32 @for_alloc_allocatable_handle(i64 %"var$4_fetch.6", i8** %"(i8**)main_$A.addr_a0$$", i32 %or.7, i8* %"(i8*)main_$A.reserved$_fetch.9$"), !llfort.type_idx !43
  %int_sext = sext i32 %func_result16 to i64, !llfort.type_idx !42
  store i64 %int_sext, i64* %"var$3", align 8, !tbaa !75
  %rel.7 = icmp ne i32 %func_result16, 0
  br i1 %rel.7, label %bb_new13_uif_true, label %bb2_else

bb2_else:                                         ; preds = %alloca_0
  %"main_$A.flags$18" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$A", i32 0, i32 3
  %"main_$A.flags$18_fetch.10" = load i64, i64* %"main_$A.flags$18", align 8, !tbaa !76, !llfort.type_idx !77
  %and.13 = and i64 %"main_$A.flags$18_fetch.10", 256
  %lshr.4 = lshr i64 %and.13, 8
  %shl.8 = shl i64 %lshr.4, 8
  %or.8 = or i64 133, %shl.8
  %and.15 = and i64 %"main_$A.flags$18_fetch.10", 1030792151040
  %lshr.5 = lshr i64 %and.15, 36
  %and.16 = and i64 %or.8, -1030792151041
  %shl.9 = shl i64 %lshr.5, 36
  %or.9 = or i64 %and.16, %shl.9
  %"main_$A.flags$20" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$A", i32 0, i32 3
  store i64 %or.9, i64* %"main_$A.flags$20", align 8, !tbaa !76
  %"main_$A.addr_length$" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$A", i32 0, i32 1
  store i64 8, i64* %"main_$A.addr_length$", align 8, !tbaa !82
  %"main_$A.dim$" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$A", i32 0, i32 4
  store i64 2, i64* %"main_$A.dim$", align 8, !tbaa !83
  %"main_$A.codim$" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$A", i32 0, i32 2
  store i64 0, i64* %"main_$A.codim$", align 8, !tbaa !84
  %"main_$N_fetch.11" = load i64, i64* @"main_$N", align 8, !tbaa !73, !llfort.type_idx !28
  %"main_$A.dim_info$" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$A", i32 0, i32 6, i32 0
  %"main_$A.dim_info$.lower_bound$" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$A.dim_info$", i32 0, i32 2
  %"main_$A.dim_info$.lower_bound$[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$A.dim_info$.lower_bound$", i32 0), !llfort.type_idx !85
  store i64 1, i64* %"main_$A.dim_info$.lower_bound$[]", align 8, !tbaa !86
  %sub.3 = sub nsw i64 %"main_$N_fetch.11", 1
  %add.3 = add nsw i64 1, %sub.3
  %rel.8 = icmp sle i64 1, %"main_$N_fetch.11"
  %rel.9 = icmp sle i64 1, %"main_$N_fetch.11"
  %rel.10 = icmp ne i1 %rel.8, false
  %slct.3 = select i1 %rel.10, i64 %add.3, i64 0
  %"main_$A.dim_info$22" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$A", i32 0, i32 6, i32 0
  %"main_$A.dim_info$.extent$" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$A.dim_info$22", i32 0, i32 0
  %"main_$A.dim_info$.extent$[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$A.dim_info$.extent$", i32 0), !llfort.type_idx !87
  store i64 %slct.3, i64* %"main_$A.dim_info$.extent$[]", align 8, !tbaa !88
  %"main_$N_fetch.12" = load i64, i64* @"main_$N", align 8, !tbaa !73, !llfort.type_idx !28
  %"main_$A.dim_info$24" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$A", i32 0, i32 6, i32 0
  %"main_$A.dim_info$.lower_bound$26" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$A.dim_info$24", i32 0, i32 2
  %"main_$A.dim_info$.lower_bound$26[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$A.dim_info$.lower_bound$26", i32 1), !llfort.type_idx !89
  store i64 1, i64* %"main_$A.dim_info$.lower_bound$26[]", align 8, !tbaa !86
  %sub.4 = sub nsw i64 %"main_$N_fetch.12", 1
  %add.4 = add nsw i64 1, %sub.4
  %rel.11 = icmp sle i64 1, %"main_$N_fetch.12"
  %rel.12 = icmp sle i64 1, %"main_$N_fetch.12"
  %rel.13 = icmp ne i1 %rel.11, false
  %slct.4 = select i1 %rel.13, i64 %add.4, i64 0
  %"main_$A.dim_info$28" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$A", i32 0, i32 6, i32 0
  %"main_$A.dim_info$.extent$30" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$A.dim_info$28", i32 0, i32 0
  %"main_$A.dim_info$.extent$30[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$A.dim_info$.extent$30", i32 1), !llfort.type_idx !90
  store i64 %slct.4, i64* %"main_$A.dim_info$.extent$30[]", align 8, !tbaa !88
  %"main_$A.dim_info$32" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$A", i32 0, i32 6, i32 0
  %"main_$A.dim_info$.spacing$" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$A.dim_info$32", i32 0, i32 1
  %"main_$A.dim_info$.spacing$[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$A.dim_info$.spacing$", i32 0), !llfort.type_idx !91
  store i64 8, i64* %"main_$A.dim_info$.spacing$[]", align 8, !tbaa !92
  %mul.2 = mul nsw i64 8, %slct.3
  %"main_$A.dim_info$34" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$A", i32 0, i32 6, i32 0
  %"main_$A.dim_info$.spacing$36" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$A.dim_info$34", i32 0, i32 1
  %"main_$A.dim_info$.spacing$36[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$A.dim_info$.spacing$36", i32 1), !llfort.type_idx !93
  store i64 %mul.2, i64* %"main_$A.dim_info$.spacing$36[]", align 8, !tbaa !92
  %func_result38 = call i32 (i64*, i32, ...) @for_check_mult_overflow64(i64* %"var$5", i32 3, i64 %slct.3, i64 %slct.4, i64 8), !llfort.type_idx !43
  %"var$5_fetch.13" = load i64, i64* %"var$5", align 8, !tbaa !75, !llfort.type_idx !42
  %"main_$A.dim_info$40" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$A", i32 0, i32 6, i32 0
  %"main_$A.dim_info$.lower_bound$42" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$A.dim_info$40", i32 0, i32 2
  %"main_$A.dim_info$.lower_bound$42[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$A.dim_info$.lower_bound$42", i32 0), !llfort.type_idx !94
  %"main_$A.dim_info$.lower_bound$42[]_fetch.14" = load i64, i64* %"main_$A.dim_info$.lower_bound$42[]", align 8, !tbaa !86, !llfort.type_idx !94
  %mul.3 = mul nsw i64 %"main_$A.dim_info$.lower_bound$42[]_fetch.14", 8
  %"main_$A.dim_info$44" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$A", i32 0, i32 6, i32 0
  %"main_$A.dim_info$.lower_bound$46" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$A.dim_info$44", i32 0, i32 2
  %"main_$A.dim_info$.lower_bound$46[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$A.dim_info$.lower_bound$46", i32 1), !llfort.type_idx !95
  %"main_$A.dim_info$.lower_bound$46[]_fetch.15" = load i64, i64* %"main_$A.dim_info$.lower_bound$46[]", align 8, !tbaa !86, !llfort.type_idx !95
  %"main_$A.dim_info$48" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$A", i32 0, i32 6, i32 0
  %"main_$A.dim_info$.spacing$50" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$A.dim_info$48", i32 0, i32 1
  %"main_$A.dim_info$.spacing$50[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$A.dim_info$.spacing$50", i32 1), !llfort.type_idx !96
  %"main_$A.dim_info$.spacing$50[]_fetch.16" = load i64, i64* %"main_$A.dim_info$.spacing$50[]", align 8, !tbaa !92, !range !97, !llfort.type_idx !96
  %mul.4 = mul nsw i64 %"main_$A.dim_info$.lower_bound$46[]_fetch.15", %"main_$A.dim_info$.spacing$50[]_fetch.16"
  %add.5 = add nsw i64 %mul.3, %mul.4
  br label %alloc_fail7

bb_new13_uif_true:                                ; preds = %alloca_0
  br label %alloc_fail7

alloc_fail7:                                      ; preds = %bb_new13_uif_true, %bb2_else
  %"var$3_fetch.17" = load i64, i64* %"var$3", align 8, !tbaa !75, !llfort.type_idx !42
  %int_sext52 = trunc i64 %"var$3_fetch.17" to i32, !llfort.type_idx !43
  store i32 %int_sext52, i32* %"main_$ERR", align 8, !tbaa !98
  %"main_$B.reserved$108" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$B", i32 0, i32 5
  store i64 0, i64* %"main_$B.reserved$108", align 8, !tbaa !100
  %"main_$N_fetch.18" = load i64, i64* @"main_$N", align 8, !tbaa !73, !llfort.type_idx !28
  %sub.5 = sub nsw i64 %"main_$N_fetch.18", 1
  %add.6 = add nsw i64 1, %sub.5
  %rel.14 = icmp sle i64 1, %"main_$N_fetch.18"
  %rel.15 = icmp sle i64 1, %"main_$N_fetch.18"
  %rel.16 = icmp ne i1 %rel.14, false
  %slct.5 = select i1 %rel.16, i64 %add.6, i64 0
  %"main_$N_fetch.19" = load i64, i64* @"main_$N", align 8, !tbaa !73, !llfort.type_idx !28
  %sub.6 = sub nsw i64 %"main_$N_fetch.19", 1
  %add.7 = add nsw i64 1, %sub.6
  %rel.17 = icmp sle i64 1, %"main_$N_fetch.19"
  %rel.18 = icmp sle i64 1, %"main_$N_fetch.19"
  %rel.19 = icmp ne i1 %rel.17, false
  %slct.6 = select i1 %rel.19, i64 %add.7, i64 0
  %mul.5 = mul nsw i64 8, %slct.5
  %func_result55 = call i32 (i64*, i32, ...) @for_check_mult_overflow64(i64* %"var$7", i32 3, i64 %slct.5, i64 %slct.6, i64 8), !llfort.type_idx !43
  %"var$7_fetch.20" = load i64, i64* %"var$7", align 8, !tbaa !75, !llfort.type_idx !42
  %"main_$B.addr_a0$" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$B", i32 0, i32 0
  %"main_$B.flags$" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$B", i32 0, i32 3
  %"main_$B.flags$_fetch.21" = load i64, i64* %"main_$B.flags$", align 8, !tbaa !102, !llfort.type_idx !77
  %"main_$B.flags$57" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$B", i32 0, i32 3
  %"main_$B.flags$57_fetch.22" = load i64, i64* %"main_$B.flags$57", align 8, !tbaa !102, !llfort.type_idx !77
  %and.17 = and i64 %"main_$B.flags$57_fetch.22", -68451041281
  %or.10 = or i64 %and.17, 1073741824
  %"main_$B.flags$59" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$B", i32 0, i32 3
  store i64 %or.10, i64* %"main_$B.flags$59", align 8, !tbaa !102
  %and.18 = and i64 %"main_$B.flags$_fetch.21", 1
  %shl.11 = shl i64 %and.18, 1
  %int_zext61 = trunc i64 %shl.11 to i32, !llfort.type_idx !78
  %or.11 = or i32 1, %int_zext61
  %and.20 = and i32 %func_result55, 1
  %and.21 = and i32 %or.11, -17
  %shl.12 = shl i32 %and.20, 4
  %or.12 = or i32 %and.21, %shl.12
  %and.22 = and i64 %"main_$B.flags$_fetch.21", 256
  %lshr.6 = lshr i64 %and.22, 8
  %and.23 = and i32 %or.12, -2097153
  %shl.13 = shl i64 %lshr.6, 21
  %int_zext63 = trunc i64 %shl.13 to i32, !llfort.type_idx !78
  %or.13 = or i32 %and.23, %int_zext63
  %and.24 = and i64 %"main_$B.flags$_fetch.21", 1030792151040
  %lshr.7 = lshr i64 %and.24, 36
  %and.25 = and i32 %or.13, -31457281
  %shl.14 = shl i64 %lshr.7, 21
  %int_zext65 = trunc i64 %shl.14 to i32, !llfort.type_idx !78
  %or.14 = or i32 %and.25, %int_zext65
  %and.26 = and i64 %"main_$B.flags$_fetch.21", 1099511627776
  %lshr.8 = lshr i64 %and.26, 40
  %and.27 = and i32 %or.14, -33554433
  %shl.15 = shl i64 %lshr.8, 25
  %int_zext67 = trunc i64 %shl.15 to i32, !llfort.type_idx !78
  %or.15 = or i32 %and.27, %int_zext67
  %and.28 = and i32 %or.15, -2031617
  %or.16 = or i32 %and.28, 262144
  %"main_$B.reserved$" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$B", i32 0, i32 5
  %"main_$B.reserved$_fetch.23" = load i64, i64* %"main_$B.reserved$", align 8, !tbaa !100, !llfort.type_idx !79
  %"(i8*)main_$B.reserved$_fetch.23$" = inttoptr i64 %"main_$B.reserved$_fetch.23" to i8*, !llfort.type_idx !80
  %"(i8**)main_$B.addr_a0$$" = bitcast i64** %"main_$B.addr_a0$" to i8**, !llfort.type_idx !103
  %func_result69 = call i32 @for_alloc_allocatable_handle(i64 %"var$7_fetch.20", i8** %"(i8**)main_$B.addr_a0$$", i32 %or.16, i8* %"(i8*)main_$B.reserved$_fetch.23$"), !llfort.type_idx !43
  %int_sext71 = sext i32 %func_result69 to i64, !llfort.type_idx !42
  store i64 %int_sext71, i64* %"var$6", align 8, !tbaa !75
  %rel.20 = icmp ne i32 %func_result69, 0
  br i1 %rel.20, label %bb_new22_uif_true, label %bb5_else

bb5_else:                                         ; preds = %alloc_fail7
  %"main_$B.flags$73" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$B", i32 0, i32 3
  %"main_$B.flags$73_fetch.24" = load i64, i64* %"main_$B.flags$73", align 8, !tbaa !102, !llfort.type_idx !77
  %and.29 = and i64 %"main_$B.flags$73_fetch.24", 256
  %lshr.9 = lshr i64 %and.29, 8
  %shl.17 = shl i64 %lshr.9, 8
  %or.17 = or i64 133, %shl.17
  %and.31 = and i64 %"main_$B.flags$73_fetch.24", 1030792151040
  %lshr.10 = lshr i64 %and.31, 36
  %and.32 = and i64 %or.17, -1030792151041
  %shl.18 = shl i64 %lshr.10, 36
  %or.18 = or i64 %and.32, %shl.18
  %"main_$B.flags$75" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$B", i32 0, i32 3
  store i64 %or.18, i64* %"main_$B.flags$75", align 8, !tbaa !102
  %"main_$B.addr_length$" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$B", i32 0, i32 1
  store i64 8, i64* %"main_$B.addr_length$", align 8, !tbaa !104
  %"main_$B.dim$" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$B", i32 0, i32 4
  store i64 2, i64* %"main_$B.dim$", align 8, !tbaa !105
  %"main_$B.codim$" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$B", i32 0, i32 2
  store i64 0, i64* %"main_$B.codim$", align 8, !tbaa !106
  %"main_$N_fetch.25" = load i64, i64* @"main_$N", align 8, !tbaa !73, !llfort.type_idx !28
  %"main_$B.dim_info$" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$B", i32 0, i32 6, i32 0
  %"main_$B.dim_info$.lower_bound$" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$B.dim_info$", i32 0, i32 2
  %"main_$B.dim_info$.lower_bound$[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$B.dim_info$.lower_bound$", i32 0), !llfort.type_idx !107
  store i64 1, i64* %"main_$B.dim_info$.lower_bound$[]", align 8, !tbaa !108
  %sub.7 = sub nsw i64 %"main_$N_fetch.25", 1
  %add.8 = add nsw i64 1, %sub.7
  %rel.21 = icmp sle i64 1, %"main_$N_fetch.25"
  %rel.22 = icmp sle i64 1, %"main_$N_fetch.25"
  %rel.23 = icmp ne i1 %rel.21, false
  %slct.7 = select i1 %rel.23, i64 %add.8, i64 0
  %"main_$B.dim_info$77" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$B", i32 0, i32 6, i32 0
  %"main_$B.dim_info$.extent$" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$B.dim_info$77", i32 0, i32 0
  %"main_$B.dim_info$.extent$[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$B.dim_info$.extent$", i32 0), !llfort.type_idx !109
  store i64 %slct.7, i64* %"main_$B.dim_info$.extent$[]", align 8, !tbaa !110
  %"main_$N_fetch.26" = load i64, i64* @"main_$N", align 8, !tbaa !73, !llfort.type_idx !28
  %"main_$B.dim_info$79" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$B", i32 0, i32 6, i32 0
  %"main_$B.dim_info$.lower_bound$81" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$B.dim_info$79", i32 0, i32 2
  %"main_$B.dim_info$.lower_bound$81[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$B.dim_info$.lower_bound$81", i32 1), !llfort.type_idx !111
  store i64 1, i64* %"main_$B.dim_info$.lower_bound$81[]", align 8, !tbaa !108
  %sub.8 = sub nsw i64 %"main_$N_fetch.26", 1
  %add.9 = add nsw i64 1, %sub.8
  %rel.24 = icmp sle i64 1, %"main_$N_fetch.26"
  %rel.25 = icmp sle i64 1, %"main_$N_fetch.26"
  %rel.26 = icmp ne i1 %rel.24, false
  %slct.8 = select i1 %rel.26, i64 %add.9, i64 0
  %"main_$B.dim_info$83" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$B", i32 0, i32 6, i32 0
  %"main_$B.dim_info$.extent$85" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$B.dim_info$83", i32 0, i32 0
  %"main_$B.dim_info$.extent$85[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$B.dim_info$.extent$85", i32 1), !llfort.type_idx !112
  store i64 %slct.8, i64* %"main_$B.dim_info$.extent$85[]", align 8, !tbaa !110
  %"main_$B.dim_info$87" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$B", i32 0, i32 6, i32 0
  %"main_$B.dim_info$.spacing$" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$B.dim_info$87", i32 0, i32 1
  %"main_$B.dim_info$.spacing$[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$B.dim_info$.spacing$", i32 0), !llfort.type_idx !113
  store i64 8, i64* %"main_$B.dim_info$.spacing$[]", align 8, !tbaa !114
  %mul.6 = mul nsw i64 8, %slct.7
  %"main_$B.dim_info$89" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$B", i32 0, i32 6, i32 0
  %"main_$B.dim_info$.spacing$91" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$B.dim_info$89", i32 0, i32 1
  %"main_$B.dim_info$.spacing$91[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$B.dim_info$.spacing$91", i32 1), !llfort.type_idx !115
  store i64 %mul.6, i64* %"main_$B.dim_info$.spacing$91[]", align 8, !tbaa !114
  %func_result93 = call i32 (i64*, i32, ...) @for_check_mult_overflow64(i64* %"var$8", i32 3, i64 %slct.7, i64 %slct.8, i64 8), !llfort.type_idx !43
  %"var$8_fetch.27" = load i64, i64* %"var$8", align 8, !tbaa !75, !llfort.type_idx !42
  %"main_$B.dim_info$95" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$B", i32 0, i32 6, i32 0
  %"main_$B.dim_info$.lower_bound$97" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$B.dim_info$95", i32 0, i32 2
  %"main_$B.dim_info$.lower_bound$97[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$B.dim_info$.lower_bound$97", i32 0), !llfort.type_idx !116
  %"main_$B.dim_info$.lower_bound$97[]_fetch.28" = load i64, i64* %"main_$B.dim_info$.lower_bound$97[]", align 8, !tbaa !108, !llfort.type_idx !116
  %mul.7 = mul nsw i64 %"main_$B.dim_info$.lower_bound$97[]_fetch.28", 8
  %"main_$B.dim_info$99" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$B", i32 0, i32 6, i32 0
  %"main_$B.dim_info$.lower_bound$101" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$B.dim_info$99", i32 0, i32 2
  %"main_$B.dim_info$.lower_bound$101[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$B.dim_info$.lower_bound$101", i32 1), !llfort.type_idx !117
  %"main_$B.dim_info$.lower_bound$101[]_fetch.29" = load i64, i64* %"main_$B.dim_info$.lower_bound$101[]", align 8, !tbaa !108, !llfort.type_idx !117
  %"main_$B.dim_info$103" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$B", i32 0, i32 6, i32 0
  %"main_$B.dim_info$.spacing$105" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$B.dim_info$103", i32 0, i32 1
  %"main_$B.dim_info$.spacing$105[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$B.dim_info$.spacing$105", i32 1), !llfort.type_idx !118
  %"main_$B.dim_info$.spacing$105[]_fetch.30" = load i64, i64* %"main_$B.dim_info$.spacing$105[]", align 8, !tbaa !114, !range !97, !llfort.type_idx !118
  %mul.8 = mul nsw i64 %"main_$B.dim_info$.lower_bound$101[]_fetch.29", %"main_$B.dim_info$.spacing$105[]_fetch.30"
  %add.10 = add nsw i64 %mul.7, %mul.8
  br label %alloc_fail16

bb_new22_uif_true:                                ; preds = %alloc_fail7
  br label %alloc_fail16

alloc_fail16:                                     ; preds = %bb_new22_uif_true, %bb5_else
  %"var$6_fetch.31" = load i64, i64* %"var$6", align 8, !tbaa !75, !llfort.type_idx !42
  %int_sext107 = trunc i64 %"var$6_fetch.31" to i32, !llfort.type_idx !43
  store i32 %int_sext107, i32* %"main_$ERR", align 8, !tbaa !98
  %"main_$C.reserved$163" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$C", i32 0, i32 5
  store i64 0, i64* %"main_$C.reserved$163", align 8, !tbaa !119
  %"main_$N_fetch.32" = load i64, i64* @"main_$N", align 8, !tbaa !73, !llfort.type_idx !28
  %sub.9 = sub nsw i64 %"main_$N_fetch.32", 1
  %add.11 = add nsw i64 1, %sub.9
  %rel.27 = icmp sle i64 1, %"main_$N_fetch.32"
  %rel.28 = icmp sle i64 1, %"main_$N_fetch.32"
  %rel.29 = icmp ne i1 %rel.27, false
  %slct.9 = select i1 %rel.29, i64 %add.11, i64 0
  %"main_$N_fetch.33" = load i64, i64* @"main_$N", align 8, !tbaa !73, !llfort.type_idx !28
  %sub.10 = sub nsw i64 %"main_$N_fetch.33", 1
  %add.12 = add nsw i64 1, %sub.10
  %rel.30 = icmp sle i64 1, %"main_$N_fetch.33"
  %rel.31 = icmp sle i64 1, %"main_$N_fetch.33"
  %rel.32 = icmp ne i1 %rel.30, false
  %slct.10 = select i1 %rel.32, i64 %add.12, i64 0
  %mul.9 = mul nsw i64 8, %slct.9
  %func_result110 = call i32 (i64*, i32, ...) @for_check_mult_overflow64(i64* %"var$10", i32 3, i64 %slct.9, i64 %slct.10, i64 8), !llfort.type_idx !43
  %"var$10_fetch.34" = load i64, i64* %"var$10", align 8, !tbaa !75, !llfort.type_idx !42
  %"main_$C.addr_a0$" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$C", i32 0, i32 0
  %"main_$C.flags$" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$C", i32 0, i32 3
  %"main_$C.flags$_fetch.35" = load i64, i64* %"main_$C.flags$", align 8, !tbaa !121, !llfort.type_idx !77
  %"main_$C.flags$112" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$C", i32 0, i32 3
  %"main_$C.flags$112_fetch.36" = load i64, i64* %"main_$C.flags$112", align 8, !tbaa !121, !llfort.type_idx !77
  %and.33 = and i64 %"main_$C.flags$112_fetch.36", -68451041281
  %or.19 = or i64 %and.33, 1073741824
  %"main_$C.flags$114" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$C", i32 0, i32 3
  store i64 %or.19, i64* %"main_$C.flags$114", align 8, !tbaa !121
  %and.34 = and i64 %"main_$C.flags$_fetch.35", 1
  %shl.20 = shl i64 %and.34, 1
  %int_zext116 = trunc i64 %shl.20 to i32, !llfort.type_idx !78
  %or.20 = or i32 1, %int_zext116
  %and.36 = and i32 %func_result110, 1
  %and.37 = and i32 %or.20, -17
  %shl.21 = shl i32 %and.36, 4
  %or.21 = or i32 %and.37, %shl.21
  %and.38 = and i64 %"main_$C.flags$_fetch.35", 256
  %lshr.11 = lshr i64 %and.38, 8
  %and.39 = and i32 %or.21, -2097153
  %shl.22 = shl i64 %lshr.11, 21
  %int_zext118 = trunc i64 %shl.22 to i32, !llfort.type_idx !78
  %or.22 = or i32 %and.39, %int_zext118
  %and.40 = and i64 %"main_$C.flags$_fetch.35", 1030792151040
  %lshr.12 = lshr i64 %and.40, 36
  %and.41 = and i32 %or.22, -31457281
  %shl.23 = shl i64 %lshr.12, 21
  %int_zext120 = trunc i64 %shl.23 to i32, !llfort.type_idx !78
  %or.23 = or i32 %and.41, %int_zext120
  %and.42 = and i64 %"main_$C.flags$_fetch.35", 1099511627776
  %lshr.13 = lshr i64 %and.42, 40
  %and.43 = and i32 %or.23, -33554433
  %shl.24 = shl i64 %lshr.13, 25
  %int_zext122 = trunc i64 %shl.24 to i32, !llfort.type_idx !78
  %or.24 = or i32 %and.43, %int_zext122
  %and.44 = and i32 %or.24, -2031617
  %or.25 = or i32 %and.44, 262144
  %"main_$C.reserved$" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$C", i32 0, i32 5
  %"main_$C.reserved$_fetch.37" = load i64, i64* %"main_$C.reserved$", align 8, !tbaa !119, !llfort.type_idx !79
  %"(i8*)main_$C.reserved$_fetch.37$" = inttoptr i64 %"main_$C.reserved$_fetch.37" to i8*, !llfort.type_idx !80
  %"(i8**)main_$C.addr_a0$$" = bitcast i64** %"main_$C.addr_a0$" to i8**, !llfort.type_idx !122
  %func_result124 = call i32 @for_alloc_allocatable_handle(i64 %"var$10_fetch.34", i8** %"(i8**)main_$C.addr_a0$$", i32 %or.25, i8* %"(i8*)main_$C.reserved$_fetch.37$"), !llfort.type_idx !43
  %int_sext126 = sext i32 %func_result124 to i64, !llfort.type_idx !42
  store i64 %int_sext126, i64* %"var$9", align 8, !tbaa !75
  %rel.33 = icmp ne i32 %func_result124, 0
  br i1 %rel.33, label %bb_new31_uif_true, label %bb8_else

bb8_else:                                         ; preds = %alloc_fail16
  %"main_$C.flags$128" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$C", i32 0, i32 3
  %"main_$C.flags$128_fetch.38" = load i64, i64* %"main_$C.flags$128", align 8, !tbaa !121, !llfort.type_idx !77
  %and.45 = and i64 %"main_$C.flags$128_fetch.38", 256
  %lshr.14 = lshr i64 %and.45, 8
  %shl.26 = shl i64 %lshr.14, 8
  %or.26 = or i64 133, %shl.26
  %and.47 = and i64 %"main_$C.flags$128_fetch.38", 1030792151040
  %lshr.15 = lshr i64 %and.47, 36
  %and.48 = and i64 %or.26, -1030792151041
  %shl.27 = shl i64 %lshr.15, 36
  %or.27 = or i64 %and.48, %shl.27
  %"main_$C.flags$130" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$C", i32 0, i32 3
  store i64 %or.27, i64* %"main_$C.flags$130", align 8, !tbaa !121
  %"main_$C.addr_length$" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$C", i32 0, i32 1
  store i64 8, i64* %"main_$C.addr_length$", align 8, !tbaa !123
  %"main_$C.dim$" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$C", i32 0, i32 4
  store i64 2, i64* %"main_$C.dim$", align 8, !tbaa !124
  %"main_$C.codim$" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$C", i32 0, i32 2
  store i64 0, i64* %"main_$C.codim$", align 8, !tbaa !125
  %"main_$N_fetch.39" = load i64, i64* @"main_$N", align 8, !tbaa !73, !llfort.type_idx !28
  %"main_$C.dim_info$" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$C", i32 0, i32 6, i32 0
  %"main_$C.dim_info$.lower_bound$" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$C.dim_info$", i32 0, i32 2
  %"main_$C.dim_info$.lower_bound$[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$C.dim_info$.lower_bound$", i32 0), !llfort.type_idx !126
  store i64 1, i64* %"main_$C.dim_info$.lower_bound$[]", align 8, !tbaa !127
  %sub.11 = sub nsw i64 %"main_$N_fetch.39", 1
  %add.13 = add nsw i64 1, %sub.11
  %rel.34 = icmp sle i64 1, %"main_$N_fetch.39"
  %rel.35 = icmp sle i64 1, %"main_$N_fetch.39"
  %rel.36 = icmp ne i1 %rel.34, false
  %slct.11 = select i1 %rel.36, i64 %add.13, i64 0
  %"main_$C.dim_info$132" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$C", i32 0, i32 6, i32 0
  %"main_$C.dim_info$.extent$" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$C.dim_info$132", i32 0, i32 0
  %"main_$C.dim_info$.extent$[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$C.dim_info$.extent$", i32 0), !llfort.type_idx !128
  store i64 %slct.11, i64* %"main_$C.dim_info$.extent$[]", align 8, !tbaa !129
  %"main_$N_fetch.40" = load i64, i64* @"main_$N", align 8, !tbaa !73, !llfort.type_idx !28
  %"main_$C.dim_info$134" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$C", i32 0, i32 6, i32 0
  %"main_$C.dim_info$.lower_bound$136" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$C.dim_info$134", i32 0, i32 2
  %"main_$C.dim_info$.lower_bound$136[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$C.dim_info$.lower_bound$136", i32 1), !llfort.type_idx !130
  store i64 1, i64* %"main_$C.dim_info$.lower_bound$136[]", align 8, !tbaa !127
  %sub.12 = sub nsw i64 %"main_$N_fetch.40", 1
  %add.14 = add nsw i64 1, %sub.12
  %rel.37 = icmp sle i64 1, %"main_$N_fetch.40"
  %rel.38 = icmp sle i64 1, %"main_$N_fetch.40"
  %rel.39 = icmp ne i1 %rel.37, false
  %slct.12 = select i1 %rel.39, i64 %add.14, i64 0
  %"main_$C.dim_info$138" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$C", i32 0, i32 6, i32 0
  %"main_$C.dim_info$.extent$140" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$C.dim_info$138", i32 0, i32 0
  %"main_$C.dim_info$.extent$140[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$C.dim_info$.extent$140", i32 1), !llfort.type_idx !131
  store i64 %slct.12, i64* %"main_$C.dim_info$.extent$140[]", align 8, !tbaa !129
  %"main_$C.dim_info$142" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$C", i32 0, i32 6, i32 0
  %"main_$C.dim_info$.spacing$" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$C.dim_info$142", i32 0, i32 1
  %"main_$C.dim_info$.spacing$[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$C.dim_info$.spacing$", i32 0), !llfort.type_idx !132
  store i64 8, i64* %"main_$C.dim_info$.spacing$[]", align 8, !tbaa !133
  %mul.10 = mul nsw i64 8, %slct.11
  %"main_$C.dim_info$144" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$C", i32 0, i32 6, i32 0
  %"main_$C.dim_info$.spacing$146" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$C.dim_info$144", i32 0, i32 1
  %"main_$C.dim_info$.spacing$146[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$C.dim_info$.spacing$146", i32 1), !llfort.type_idx !134
  store i64 %mul.10, i64* %"main_$C.dim_info$.spacing$146[]", align 8, !tbaa !133
  %func_result148 = call i32 (i64*, i32, ...) @for_check_mult_overflow64(i64* %"var$11", i32 3, i64 %slct.11, i64 %slct.12, i64 8), !llfort.type_idx !43
  %"var$11_fetch.41" = load i64, i64* %"var$11", align 8, !tbaa !75, !llfort.type_idx !42
  %"main_$C.dim_info$150" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$C", i32 0, i32 6, i32 0
  %"main_$C.dim_info$.lower_bound$152" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$C.dim_info$150", i32 0, i32 2
  %"main_$C.dim_info$.lower_bound$152[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$C.dim_info$.lower_bound$152", i32 0), !llfort.type_idx !135
  %"main_$C.dim_info$.lower_bound$152[]_fetch.42" = load i64, i64* %"main_$C.dim_info$.lower_bound$152[]", align 8, !tbaa !127, !llfort.type_idx !135
  %mul.11 = mul nsw i64 %"main_$C.dim_info$.lower_bound$152[]_fetch.42", 8
  %"main_$C.dim_info$154" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$C", i32 0, i32 6, i32 0
  %"main_$C.dim_info$.lower_bound$156" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$C.dim_info$154", i32 0, i32 2
  %"main_$C.dim_info$.lower_bound$156[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$C.dim_info$.lower_bound$156", i32 1), !llfort.type_idx !136
  %"main_$C.dim_info$.lower_bound$156[]_fetch.43" = load i64, i64* %"main_$C.dim_info$.lower_bound$156[]", align 8, !tbaa !127, !llfort.type_idx !136
  %"main_$C.dim_info$158" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$C", i32 0, i32 6, i32 0
  %"main_$C.dim_info$.spacing$160" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$C.dim_info$158", i32 0, i32 1
  %"main_$C.dim_info$.spacing$160[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$C.dim_info$.spacing$160", i32 1), !llfort.type_idx !137
  %"main_$C.dim_info$.spacing$160[]_fetch.44" = load i64, i64* %"main_$C.dim_info$.spacing$160[]", align 8, !tbaa !133, !range !97, !llfort.type_idx !137
  %mul.12 = mul nsw i64 %"main_$C.dim_info$.lower_bound$156[]_fetch.43", %"main_$C.dim_info$.spacing$160[]_fetch.44"
  %add.15 = add nsw i64 %mul.11, %mul.12
  br label %alloc_fail25

bb_new31_uif_true:                                ; preds = %alloc_fail16
  br label %alloc_fail25

alloc_fail25:                                     ; preds = %bb_new31_uif_true, %bb8_else
  %"var$9_fetch.45" = load i64, i64* %"var$9", align 8, !tbaa !75, !llfort.type_idx !42
  %int_sext162 = trunc i64 %"var$9_fetch.45" to i32, !llfort.type_idx !43
  store i32 %int_sext162, i32* %"main_$ERR", align 8, !tbaa !98
  %"main_$C.addr_a0$165" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$C", i32 0, i32 0
  %"main_$C.addr_a0$_fetch.46" = load i64*, i64** %"main_$C.addr_a0$165", align 8, !tbaa !138, !llfort.type_idx !42
  %"main_$C.dim_info$166" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$C", i32 0, i32 6, i32 0
  %"main_$C.dim_info$.lower_bound$167" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$C.dim_info$166", i32 0, i32 2
  %"main_$C.dim_info$.lower_bound$[]168" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$C.dim_info$.lower_bound$167", i32 0), !llfort.type_idx !139
  %"main_$C.dim_info$.lower_bound$[]_fetch.47" = load i64, i64* %"main_$C.dim_info$.lower_bound$[]168", align 8, !tbaa !127, !llfort.type_idx !139
  %"main_$C.dim_info$169" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$C", i32 0, i32 6, i32 0
  %"main_$C.dim_info$.lower_bound$170" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$C.dim_info$169", i32 0, i32 2
  %"main_$C.dim_info$.lower_bound$[]171" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$C.dim_info$.lower_bound$170", i32 0), !llfort.type_idx !140
  %"main_$C.dim_info$.lower_bound$[]_fetch.48" = load i64, i64* %"main_$C.dim_info$.lower_bound$[]171", align 8, !tbaa !127, !llfort.type_idx !140
  %"main_$C.dim_info$172" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$C", i32 0, i32 6, i32 0
  %"main_$C.dim_info$.lower_bound$173" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$C.dim_info$172", i32 0, i32 2
  %"main_$C.dim_info$.lower_bound$[]174" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$C.dim_info$.lower_bound$173", i32 0), !llfort.type_idx !141
  %"main_$C.dim_info$.lower_bound$[]_fetch.49" = load i64, i64* %"main_$C.dim_info$.lower_bound$[]174", align 8, !tbaa !127, !llfort.type_idx !141
  %"main_$C.dim_info$175" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$C", i32 0, i32 6, i32 0
  %"main_$C.dim_info$.extent$176" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$C.dim_info$175", i32 0, i32 0
  %"main_$C.dim_info$.extent$[]177" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$C.dim_info$.extent$176", i32 0), !llfort.type_idx !142
  %"main_$C.dim_info$.extent$[]_fetch.50" = load i64, i64* %"main_$C.dim_info$.extent$[]177", align 8, !tbaa !129, !llfort.type_idx !142
  %add.16 = add nsw i64 %"main_$C.dim_info$.lower_bound$[]_fetch.49", %"main_$C.dim_info$.extent$[]_fetch.50"
  %sub.13 = sub nsw i64 %add.16, 1
  %"main_$C.dim_info$178" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$C", i32 0, i32 6, i32 0
  %"main_$C.dim_info$.extent$179" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$C.dim_info$178", i32 0, i32 0
  %"main_$C.dim_info$.extent$[]180" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$C.dim_info$.extent$179", i32 0), !llfort.type_idx !143
  %"main_$C.dim_info$.extent$[]_fetch.51" = load i64, i64* %"main_$C.dim_info$.extent$[]180", align 8, !tbaa !129, !llfort.type_idx !143
  %"main_$C.dim_info$181" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$C", i32 0, i32 6, i32 0
  %"main_$C.dim_info$.spacing$182" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$C.dim_info$181", i32 0, i32 1
  %"main_$C.dim_info$.spacing$[]183" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$C.dim_info$.spacing$182", i32 1), !llfort.type_idx !144
  %"main_$C.dim_info$.spacing$[]_fetch.52" = load i64, i64* %"main_$C.dim_info$.spacing$[]183", align 8, !tbaa !133, !range !97, !llfort.type_idx !144
  %"main_$C.dim_info$184" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$C", i32 0, i32 6, i32 0
  %"main_$C.dim_info$.lower_bound$185" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$C.dim_info$184", i32 0, i32 2
  %"main_$C.dim_info$.lower_bound$[]186" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$C.dim_info$.lower_bound$185", i32 1), !llfort.type_idx !145
  %"main_$C.dim_info$.lower_bound$[]_fetch.53" = load i64, i64* %"main_$C.dim_info$.lower_bound$[]186", align 8, !tbaa !127, !llfort.type_idx !145
  %"main_$C.dim_info$187" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$C", i32 0, i32 6, i32 0
  %"main_$C.dim_info$.lower_bound$188" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$C.dim_info$187", i32 0, i32 2
  %"main_$C.dim_info$.lower_bound$[]189" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$C.dim_info$.lower_bound$188", i32 1), !llfort.type_idx !146
  %"main_$C.dim_info$.lower_bound$[]_fetch.54" = load i64, i64* %"main_$C.dim_info$.lower_bound$[]189", align 8, !tbaa !127, !llfort.type_idx !146
  %"main_$C.dim_info$190" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$C", i32 0, i32 6, i32 0
  %"main_$C.dim_info$.lower_bound$191" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$C.dim_info$190", i32 0, i32 2
  %"main_$C.dim_info$.lower_bound$[]192" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$C.dim_info$.lower_bound$191", i32 1), !llfort.type_idx !147
  %"main_$C.dim_info$.lower_bound$[]_fetch.55" = load i64, i64* %"main_$C.dim_info$.lower_bound$[]192", align 8, !tbaa !127, !llfort.type_idx !147
  %"main_$C.dim_info$193" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$C", i32 0, i32 6, i32 0
  %"main_$C.dim_info$.extent$194" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$C.dim_info$193", i32 0, i32 0
  %"main_$C.dim_info$.extent$[]195" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$C.dim_info$.extent$194", i32 1), !llfort.type_idx !148
  %"main_$C.dim_info$.extent$[]_fetch.56" = load i64, i64* %"main_$C.dim_info$.extent$[]195", align 8, !tbaa !129, !llfort.type_idx !148
  %add.17 = add nsw i64 %"main_$C.dim_info$.lower_bound$[]_fetch.55", %"main_$C.dim_info$.extent$[]_fetch.56"
  %sub.14 = sub nsw i64 %add.17, 1
  %"main_$C.dim_info$196" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$C", i32 0, i32 6, i32 0
  %"main_$C.dim_info$.extent$197" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$C.dim_info$196", i32 0, i32 0
  %"main_$C.dim_info$.extent$[]198" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$C.dim_info$.extent$197", i32 1), !llfort.type_idx !149
  %"main_$C.dim_info$.extent$[]_fetch.57" = load i64, i64* %"main_$C.dim_info$.extent$[]198", align 8, !tbaa !129, !llfort.type_idx !149
  %"main_$C.dim_info$199" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$C", i32 0, i32 6, i32 0
  %"main_$C.dim_info$.lower_bound$200" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$C.dim_info$199", i32 0, i32 2
  %"main_$C.dim_info$.lower_bound$[]201" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$C.dim_info$.lower_bound$200", i32 0), !llfort.type_idx !150
  %"main_$C.dim_info$.lower_bound$[]_fetch.58" = load i64, i64* %"main_$C.dim_info$.lower_bound$[]201", align 8, !tbaa !127, !llfort.type_idx !150
  %mul.13 = mul nsw i64 %"main_$C.dim_info$.lower_bound$[]_fetch.58", 8
  %"main_$C.dim_info$202" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$C", i32 0, i32 6, i32 0
  %"main_$C.dim_info$.lower_bound$203" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$C.dim_info$202", i32 0, i32 2
  %"main_$C.dim_info$.lower_bound$[]204" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$C.dim_info$.lower_bound$203", i32 1), !llfort.type_idx !151
  %"main_$C.dim_info$.lower_bound$[]_fetch.59" = load i64, i64* %"main_$C.dim_info$.lower_bound$[]204", align 8, !tbaa !127, !llfort.type_idx !151
  %"main_$C.dim_info$205" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$C", i32 0, i32 6, i32 0
  %"main_$C.dim_info$.spacing$206" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$C.dim_info$205", i32 0, i32 1
  %"main_$C.dim_info$.spacing$[]207" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$C.dim_info$.spacing$206", i32 1), !llfort.type_idx !152
  %"main_$C.dim_info$.spacing$[]_fetch.60" = load i64, i64* %"main_$C.dim_info$.spacing$[]207", align 8, !tbaa !133, !range !97, !llfort.type_idx !152
  %mul.14 = mul nsw i64 %"main_$C.dim_info$.lower_bound$[]_fetch.59", %"main_$C.dim_info$.spacing$[]_fetch.60"
  %add.18 = add nsw i64 %mul.13, %mul.14
  store i64 %"main_$C.dim_info$.lower_bound$[]_fetch.54", i64* %"var$15", align 8, !tbaa !75
  store i64 1, i64* %"$loop_ctr164", align 8, !tbaa !75
  br label %loop_test39

loop_test35:                                      ; preds = %loop_body40, %loop_body36
  %"$loop_ctr_fetch.63" = load i64, i64* %"$loop_ctr", align 8, !tbaa !75, !llfort.type_idx !42
  %rel.40 = icmp sle i64 %"$loop_ctr_fetch.63", %"main_$C.dim_info$.extent$[]_fetch.51"
  br i1 %rel.40, label %loop_body36, label %loop_exit37

loop_body36:                                      ; preds = %loop_test35
  %"var$15_fetch.61" = load i64, i64* %"var$15", align 8, !tbaa !75, !llfort.type_idx !42
  %"main_$C.addr_a0$_fetch.46[]" = call i64* @llvm.intel.subscript.p0i64.i64.i64.p0i64.i64(i8 1, i64 %"main_$C.dim_info$.lower_bound$[]_fetch.53", i64 %"main_$C.dim_info$.spacing$[]_fetch.52", i64* elementtype(i64) %"main_$C.addr_a0$_fetch.46", i64 %"var$15_fetch.61"), !llfort.type_idx !153
  %"var$14_fetch.62" = load i64, i64* %"var$14", align 8, !tbaa !75, !llfort.type_idx !42
  %"main_$C.addr_a0$_fetch.46[][]" = call i64* @llvm.intel.subscript.p0i64.i64.i64.p0i64.i64(i8 0, i64 %"main_$C.dim_info$.lower_bound$[]_fetch.47", i64 8, i64* elementtype(i64) %"main_$C.addr_a0$_fetch.46[]", i64 %"var$14_fetch.62"), !llfort.type_idx !154
  store i64 0, i64* %"main_$C.addr_a0$_fetch.46[][]", align 8, !tbaa !155
  %"var$14_fetch.64" = load i64, i64* %"var$14", align 8, !tbaa !75, !llfort.type_idx !42
  %add.19 = add nsw i64 %"var$14_fetch.64", 1
  store i64 %add.19, i64* %"var$14", align 8, !tbaa !75
  %"$loop_ctr_fetch.65" = load i64, i64* %"$loop_ctr", align 8, !tbaa !75, !llfort.type_idx !42
  %add.20 = add nsw i64 %"$loop_ctr_fetch.65", 1
  store i64 %add.20, i64* %"$loop_ctr", align 8, !tbaa !75
  br label %loop_test35

loop_exit37:                                      ; preds = %loop_test35
  %"var$15_fetch.67" = load i64, i64* %"var$15", align 8, !tbaa !75, !llfort.type_idx !42
  %add.21 = add nsw i64 %"var$15_fetch.67", 1
  store i64 %add.21, i64* %"var$15", align 8, !tbaa !75
  %"$loop_ctr164_fetch.68" = load i64, i64* %"$loop_ctr164", align 8, !tbaa !75, !llfort.type_idx !42
  %add.22 = add nsw i64 %"$loop_ctr164_fetch.68", 1
  store i64 %add.22, i64* %"$loop_ctr164", align 8, !tbaa !75
  br label %loop_test39

loop_test39:                                      ; preds = %loop_exit37, %alloc_fail25
  %"$loop_ctr164_fetch.66" = load i64, i64* %"$loop_ctr164", align 8, !tbaa !75, !llfort.type_idx !42
  %rel.41 = icmp sle i64 %"$loop_ctr164_fetch.66", %"main_$C.dim_info$.extent$[]_fetch.57"
  br i1 %rel.41, label %loop_body40, label %loop_exit41

loop_body40:                                      ; preds = %loop_test39
  store i64 %"main_$C.dim_info$.lower_bound$[]_fetch.48", i64* %"var$14", align 8, !tbaa !75
  store i64 1, i64* %"$loop_ctr", align 8, !tbaa !75
  br label %loop_test35

loop_exit41:                                      ; preds = %loop_test39
  %"main_$N_fetch.69" = load i64, i64* @"main_$N", align 8, !tbaa !73, !llfort.type_idx !28
  store i64 %"main_$N_fetch.69", i64* %"var$16", align 8, !tbaa !75
  store i64 1, i64* %"main_$J", align 8, !tbaa !157
  %"var$16_fetch.70" = load i64, i64* %"var$16", align 8, !tbaa !75, !llfort.type_idx !42
  %rel.42 = icmp slt i64 %"var$16_fetch.70", 1
  br i1 %rel.42, label %bb17, label %bb18

bb18:                                             ; preds = %loop_exit41
  br label %bb16

bb16:                                             ; preds = %bb21, %bb18
  %"main_$N_fetch.71" = load i64, i64* @"main_$N", align 8, !tbaa !73, !llfort.type_idx !28
  store i64 %"main_$N_fetch.71", i64* %"var$17", align 8, !tbaa !75
  store i64 1, i64* %"main_$I", align 8, !tbaa !159
  %"var$17_fetch.72" = load i64, i64* %"var$17", align 8, !tbaa !75, !llfort.type_idx !42
  %rel.43 = icmp slt i64 %"var$17_fetch.72", 1
  br i1 %rel.43, label %bb21, label %bb22

bb22:                                             ; preds = %bb16
  br label %bb20

bb20:                                             ; preds = %bb20, %bb22
  %"main_$J_fetch.73" = load i64, i64* %"main_$J", align 8, !tbaa !157, !llfort.type_idx !38
  %add.23 = add nsw i64 %"main_$J_fetch.73", 1
  %"main_$I_fetch.74" = load i64, i64* %"main_$I", align 8, !tbaa !159, !llfort.type_idx !39
  %mul.15 = mul nsw i64 %add.23, %"main_$I_fetch.74"
  %"main_$I_fetch.75" = load i64, i64* %"main_$I", align 8, !tbaa !159, !llfort.type_idx !39
  %sub.15 = sub nsw i64 %"main_$I_fetch.75", 1
  %add.24 = add nsw i64 %mul.15, %sub.15
  %"main_$A.addr_a0$208" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$A", i32 0, i32 0
  %"main_$A.addr_a0$_fetch.76" = load i64*, i64** %"main_$A.addr_a0$208", align 8, !tbaa !161, !llfort.type_idx !42
  %"main_$A.dim_info$209" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$A", i32 0, i32 6, i32 0
  %"main_$A.dim_info$.lower_bound$210" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$A.dim_info$209", i32 0, i32 2
  %"main_$A.dim_info$.lower_bound$[]211" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$A.dim_info$.lower_bound$210", i32 0), !llfort.type_idx !162
  %"main_$A.dim_info$.lower_bound$[]_fetch.77" = load i64, i64* %"main_$A.dim_info$.lower_bound$[]211", align 8, !tbaa !86, !llfort.type_idx !162
  %"main_$I_fetch.78" = load i64, i64* %"main_$I", align 8, !tbaa !159, !llfort.type_idx !39
  %"main_$A.dim_info$212" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$A", i32 0, i32 6, i32 0
  %"main_$A.dim_info$.spacing$213" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$A.dim_info$212", i32 0, i32 1
  %"main_$A.dim_info$.spacing$[]214" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$A.dim_info$.spacing$213", i32 1), !llfort.type_idx !163
  %"main_$A.dim_info$.spacing$[]_fetch.79" = load i64, i64* %"main_$A.dim_info$.spacing$[]214", align 8, !tbaa !92, !range !97, !llfort.type_idx !163
  %"main_$A.dim_info$215" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$A", i32 0, i32 6, i32 0
  %"main_$A.dim_info$.lower_bound$216" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$A.dim_info$215", i32 0, i32 2
  %"main_$A.dim_info$.lower_bound$[]217" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$A.dim_info$.lower_bound$216", i32 1), !llfort.type_idx !164
  %"main_$A.dim_info$.lower_bound$[]_fetch.80" = load i64, i64* %"main_$A.dim_info$.lower_bound$[]217", align 8, !tbaa !86, !llfort.type_idx !164
  %"main_$J_fetch.81" = load i64, i64* %"main_$J", align 8, !tbaa !157, !llfort.type_idx !38
  %"main_$A.dim_info$218" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$A", i32 0, i32 6, i32 0
  %"main_$A.dim_info$.lower_bound$219" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$A.dim_info$218", i32 0, i32 2
  %"main_$A.dim_info$.lower_bound$[]220" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$A.dim_info$.lower_bound$219", i32 0), !llfort.type_idx !165
  %"main_$A.dim_info$.lower_bound$[]_fetch.82" = load i64, i64* %"main_$A.dim_info$.lower_bound$[]220", align 8, !tbaa !86, !llfort.type_idx !165
  %mul.16 = mul nsw i64 %"main_$A.dim_info$.lower_bound$[]_fetch.82", 8
  %"main_$A.dim_info$221" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$A", i32 0, i32 6, i32 0
  %"main_$A.dim_info$.lower_bound$222" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$A.dim_info$221", i32 0, i32 2
  %"main_$A.dim_info$.lower_bound$[]223" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$A.dim_info$.lower_bound$222", i32 1), !llfort.type_idx !166
  %"main_$A.dim_info$.lower_bound$[]_fetch.83" = load i64, i64* %"main_$A.dim_info$.lower_bound$[]223", align 8, !tbaa !86, !llfort.type_idx !166
  %"main_$A.dim_info$224" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$A", i32 0, i32 6, i32 0
  %"main_$A.dim_info$.spacing$225" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$A.dim_info$224", i32 0, i32 1
  %"main_$A.dim_info$.spacing$[]226" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$A.dim_info$.spacing$225", i32 1), !llfort.type_idx !167
  %"main_$A.dim_info$.spacing$[]_fetch.84" = load i64, i64* %"main_$A.dim_info$.spacing$[]226", align 8, !tbaa !92, !range !97, !llfort.type_idx !167
  %mul.17 = mul nsw i64 %"main_$A.dim_info$.lower_bound$[]_fetch.83", %"main_$A.dim_info$.spacing$[]_fetch.84"
  %add.25 = add nsw i64 %mul.16, %mul.17
  %"main_$A.addr_a0$_fetch.76[]" = call i64* @llvm.intel.subscript.p0i64.i64.i64.p0i64.i64(i8 1, i64 %"main_$A.dim_info$.lower_bound$[]_fetch.80", i64 %"main_$A.dim_info$.spacing$[]_fetch.79", i64* elementtype(i64) %"main_$A.addr_a0$_fetch.76", i64 %"main_$J_fetch.81"), !llfort.type_idx !168
  %"main_$A.addr_a0$_fetch.76[][]" = call i64* @llvm.intel.subscript.p0i64.i64.i64.p0i64.i64(i8 0, i64 %"main_$A.dim_info$.lower_bound$[]_fetch.77", i64 8, i64* elementtype(i64) %"main_$A.addr_a0$_fetch.76[]", i64 %"main_$I_fetch.78"), !llfort.type_idx !169
  store i64 %add.24, i64* %"main_$A.addr_a0$_fetch.76[][]", align 8, !tbaa !170
  %"main_$J_fetch.85" = load i64, i64* %"main_$J", align 8, !tbaa !157, !llfort.type_idx !38
  %add.26 = add nsw i64 %"main_$J_fetch.85", 1
  %"main_$I_fetch.86" = load i64, i64* %"main_$I", align 8, !tbaa !159, !llfort.type_idx !39
  %mul.18 = mul nsw i64 %add.26, %"main_$I_fetch.86"
  %"main_$I_fetch.87" = load i64, i64* %"main_$I", align 8, !tbaa !159, !llfort.type_idx !39
  %sub.16 = sub nsw i64 %"main_$I_fetch.87", 1
  %add.27 = add nsw i64 %mul.18, %sub.16
  %"main_$B.addr_a0$227" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$B", i32 0, i32 0
  %"main_$B.addr_a0$_fetch.88" = load i64*, i64** %"main_$B.addr_a0$227", align 8, !tbaa !172, !llfort.type_idx !42
  %"main_$B.dim_info$228" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$B", i32 0, i32 6, i32 0
  %"main_$B.dim_info$.lower_bound$229" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$B.dim_info$228", i32 0, i32 2
  %"main_$B.dim_info$.lower_bound$[]230" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$B.dim_info$.lower_bound$229", i32 0), !llfort.type_idx !173
  %"main_$B.dim_info$.lower_bound$[]_fetch.89" = load i64, i64* %"main_$B.dim_info$.lower_bound$[]230", align 8, !tbaa !108, !llfort.type_idx !173
  %"main_$I_fetch.90" = load i64, i64* %"main_$I", align 8, !tbaa !159, !llfort.type_idx !39
  %"main_$B.dim_info$231" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$B", i32 0, i32 6, i32 0
  %"main_$B.dim_info$.spacing$232" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$B.dim_info$231", i32 0, i32 1
  %"main_$B.dim_info$.spacing$[]233" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$B.dim_info$.spacing$232", i32 1), !llfort.type_idx !174
  %"main_$B.dim_info$.spacing$[]_fetch.91" = load i64, i64* %"main_$B.dim_info$.spacing$[]233", align 8, !tbaa !114, !range !97, !llfort.type_idx !174
  %"main_$B.dim_info$234" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$B", i32 0, i32 6, i32 0
  %"main_$B.dim_info$.lower_bound$235" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$B.dim_info$234", i32 0, i32 2
  %"main_$B.dim_info$.lower_bound$[]236" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$B.dim_info$.lower_bound$235", i32 1), !llfort.type_idx !175
  %"main_$B.dim_info$.lower_bound$[]_fetch.92" = load i64, i64* %"main_$B.dim_info$.lower_bound$[]236", align 8, !tbaa !108, !llfort.type_idx !175
  %"main_$J_fetch.93" = load i64, i64* %"main_$J", align 8, !tbaa !157, !llfort.type_idx !38
  %"main_$B.dim_info$237" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$B", i32 0, i32 6, i32 0
  %"main_$B.dim_info$.lower_bound$238" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$B.dim_info$237", i32 0, i32 2
  %"main_$B.dim_info$.lower_bound$[]239" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$B.dim_info$.lower_bound$238", i32 0), !llfort.type_idx !176
  %"main_$B.dim_info$.lower_bound$[]_fetch.94" = load i64, i64* %"main_$B.dim_info$.lower_bound$[]239", align 8, !tbaa !108, !llfort.type_idx !176
  %mul.19 = mul nsw i64 %"main_$B.dim_info$.lower_bound$[]_fetch.94", 8
  %"main_$B.dim_info$240" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$B", i32 0, i32 6, i32 0
  %"main_$B.dim_info$.lower_bound$241" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$B.dim_info$240", i32 0, i32 2
  %"main_$B.dim_info$.lower_bound$[]242" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$B.dim_info$.lower_bound$241", i32 1), !llfort.type_idx !177
  %"main_$B.dim_info$.lower_bound$[]_fetch.95" = load i64, i64* %"main_$B.dim_info$.lower_bound$[]242", align 8, !tbaa !108, !llfort.type_idx !177
  %"main_$B.dim_info$243" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$B", i32 0, i32 6, i32 0
  %"main_$B.dim_info$.spacing$244" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$B.dim_info$243", i32 0, i32 1
  %"main_$B.dim_info$.spacing$[]245" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$B.dim_info$.spacing$244", i32 1), !llfort.type_idx !178
  %"main_$B.dim_info$.spacing$[]_fetch.96" = load i64, i64* %"main_$B.dim_info$.spacing$[]245", align 8, !tbaa !114, !range !97, !llfort.type_idx !178
  %mul.20 = mul nsw i64 %"main_$B.dim_info$.lower_bound$[]_fetch.95", %"main_$B.dim_info$.spacing$[]_fetch.96"
  %add.28 = add nsw i64 %mul.19, %mul.20
  %"main_$B.addr_a0$_fetch.88[]" = call i64* @llvm.intel.subscript.p0i64.i64.i64.p0i64.i64(i8 1, i64 %"main_$B.dim_info$.lower_bound$[]_fetch.92", i64 %"main_$B.dim_info$.spacing$[]_fetch.91", i64* elementtype(i64) %"main_$B.addr_a0$_fetch.88", i64 %"main_$J_fetch.93"), !llfort.type_idx !179
  %"main_$B.addr_a0$_fetch.88[][]" = call i64* @llvm.intel.subscript.p0i64.i64.i64.p0i64.i64(i8 0, i64 %"main_$B.dim_info$.lower_bound$[]_fetch.89", i64 8, i64* elementtype(i64) %"main_$B.addr_a0$_fetch.88[]", i64 %"main_$I_fetch.90"), !llfort.type_idx !180
  store i64 %add.27, i64* %"main_$B.addr_a0$_fetch.88[][]", align 8, !tbaa !181
  %"main_$I_fetch.97" = load i64, i64* %"main_$I", align 8, !tbaa !159, !llfort.type_idx !39
  %add.29 = add nsw i64 %"main_$I_fetch.97", 1
  store i64 %add.29, i64* %"main_$I", align 8, !tbaa !159
  %"main_$I_fetch.98" = load i64, i64* %"main_$I", align 8, !tbaa !159, !llfort.type_idx !39
  %"var$17_fetch.99" = load i64, i64* %"var$17", align 8, !tbaa !75, !llfort.type_idx !42
  %rel.44 = icmp sle i64 %"main_$I_fetch.98", %"var$17_fetch.99"
  br i1 %rel.44, label %bb20, label %bb23

bb23:                                             ; preds = %bb20
  br label %bb21

bb21:                                             ; preds = %bb23, %bb16
  %"main_$J_fetch.100" = load i64, i64* %"main_$J", align 8, !tbaa !157, !llfort.type_idx !38
  %add.30 = add nsw i64 %"main_$J_fetch.100", 1
  store i64 %add.30, i64* %"main_$J", align 8, !tbaa !157
  %"main_$J_fetch.101" = load i64, i64* %"main_$J", align 8, !tbaa !157, !llfort.type_idx !38
  %"var$16_fetch.102" = load i64, i64* %"var$16", align 8, !tbaa !75, !llfort.type_idx !42
  %rel.45 = icmp sle i64 %"main_$J_fetch.101", %"var$16_fetch.102"
  br i1 %rel.45, label %bb16, label %bb19

bb19:                                             ; preds = %bb21
  br label %bb17

bb17:                                             ; preds = %bb19, %loop_exit41
  %"main_$A.addr_a0$247" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$A", i32 0, i32 0
  %"main_$A.addr_a0$247_fetch.103" = load i64*, i64** %"main_$A.addr_a0$247", align 8, !tbaa !161, !llfort.type_idx !42
  %"main_$A.addr_length$249" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$A", i32 0, i32 1
  %"main_$A.addr_length$251" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$A", i32 0, i32 1
  %"main_$A.addr_length$251_fetch.104" = load i64, i64* %"main_$A.addr_length$251", align 8, !tbaa !82, !llfort.type_idx !183
  %"main_$A.dim_info$253" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$A", i32 0, i32 6, i32 0
  %"main_$A.dim_info$.extent$255" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$A.dim_info$253", i32 0, i32 0
  %"main_$A.dim_info$.extent$255[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$A.dim_info$.extent$255", i32 0), !llfort.type_idx !184
  %"main_$A.dim_info$.extent$255[]_fetch.105" = load i64, i64* %"main_$A.dim_info$.extent$255[]", align 8, !tbaa !88, !llfort.type_idx !184
  %"main_$A.dim_info$257" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$A", i32 0, i32 6, i32 0
  %"main_$A.dim_info$.extent$259" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$A.dim_info$257", i32 0, i32 0
  %"main_$A.dim_info$.extent$259[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$A.dim_info$.extent$259", i32 1), !llfort.type_idx !185
  %"main_$A.dim_info$.extent$259[]_fetch.106" = load i64, i64* %"main_$A.dim_info$.extent$259[]", align 8, !tbaa !88, !llfort.type_idx !185
  %mul.21 = mul nsw i64 %"main_$A.dim_info$.extent$255[]_fetch.105", %"main_$A.dim_info$.extent$259[]_fetch.106"
  %mul.22 = mul nsw i64 %"main_$A.addr_length$251_fetch.104", %mul.21
  br label %bb_new44

bb_new44:                                         ; preds = %bb17
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.ENTER.DATA"(), "QUAL.OMP.MAP.TO"(%"QNCA_a0$i64*$rank2$"* %"main_$A", %"QNCA_a0$i64*$rank2$"* %"main_$A", i64 96, i64 0, i8* null, i8* null), "QUAL.OMP.MAP.TO:CHAIN"(%"QNCA_a0$i64*$rank2$"* %"main_$A", i64* %"main_$A.addr_a0$247_fetch.103", i64 %mul.22, i64 281474976710673, i8* null, i8* null), "QUAL.OMP.MAP.TO:CHAIN"(%"QNCA_a0$i64*$rank2$"* %"main_$A", i64* %"main_$A.addr_length$249", i64 88, i64 281474976710657, i8* null, i8* null) ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET.ENTER.DATA"() ]
  %"main_$B.addr_a0$261" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$B", i32 0, i32 0
  %"main_$B.addr_a0$261_fetch.107" = load i64*, i64** %"main_$B.addr_a0$261", align 8, !tbaa !172, !llfort.type_idx !42
  %"main_$B.addr_length$263" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$B", i32 0, i32 1
  %"main_$B.addr_length$265" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$B", i32 0, i32 1
  %"main_$B.addr_length$265_fetch.108" = load i64, i64* %"main_$B.addr_length$265", align 8, !tbaa !104, !llfort.type_idx !183
  %"main_$B.dim_info$267" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$B", i32 0, i32 6, i32 0
  %"main_$B.dim_info$.extent$269" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$B.dim_info$267", i32 0, i32 0
  %"main_$B.dim_info$.extent$269[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$B.dim_info$.extent$269", i32 0), !llfort.type_idx !186
  %"main_$B.dim_info$.extent$269[]_fetch.109" = load i64, i64* %"main_$B.dim_info$.extent$269[]", align 8, !tbaa !110, !llfort.type_idx !186
  %"main_$B.dim_info$271" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$B", i32 0, i32 6, i32 0
  %"main_$B.dim_info$.extent$273" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$B.dim_info$271", i32 0, i32 0
  %"main_$B.dim_info$.extent$273[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$B.dim_info$.extent$273", i32 1), !llfort.type_idx !187
  %"main_$B.dim_info$.extent$273[]_fetch.110" = load i64, i64* %"main_$B.dim_info$.extent$273[]", align 8, !tbaa !110, !llfort.type_idx !187
  %mul.23 = mul nsw i64 %"main_$B.dim_info$.extent$269[]_fetch.109", %"main_$B.dim_info$.extent$273[]_fetch.110"
  %mul.24 = mul nsw i64 %"main_$B.addr_length$265_fetch.108", %mul.23
  br label %bb_new45

bb_new45:                                         ; preds = %bb_new44
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.ENTER.DATA"(), "QUAL.OMP.MAP.TO"(%"QNCA_a0$i64*$rank2$"* %"main_$B", %"QNCA_a0$i64*$rank2$"* %"main_$B", i64 96, i64 0, i8* null, i8* null), "QUAL.OMP.MAP.TO:CHAIN"(%"QNCA_a0$i64*$rank2$"* %"main_$B", i64* %"main_$B.addr_a0$261_fetch.107", i64 %mul.24, i64 281474976710673, i8* null, i8* null), "QUAL.OMP.MAP.TO:CHAIN"(%"QNCA_a0$i64*$rank2$"* %"main_$B", i64* %"main_$B.addr_length$263", i64 88, i64 281474976710657, i8* null, i8* null) ]
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET.ENTER.DATA"() ]
  %"main_$C.addr_a0$275" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$C", i32 0, i32 0
  %"main_$C.addr_a0$275_fetch.111" = load i64*, i64** %"main_$C.addr_a0$275", align 8, !tbaa !138, !llfort.type_idx !42
  %"main_$C.addr_length$277" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$C", i32 0, i32 1
  %"main_$C.addr_length$279" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$C", i32 0, i32 1
  %"main_$C.addr_length$279_fetch.112" = load i64, i64* %"main_$C.addr_length$279", align 8, !tbaa !123, !llfort.type_idx !183
  %"main_$C.dim_info$281" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$C", i32 0, i32 6, i32 0
  %"main_$C.dim_info$.extent$283" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$C.dim_info$281", i32 0, i32 0
  %"main_$C.dim_info$.extent$283[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$C.dim_info$.extent$283", i32 0), !llfort.type_idx !188
  %"main_$C.dim_info$.extent$283[]_fetch.113" = load i64, i64* %"main_$C.dim_info$.extent$283[]", align 8, !tbaa !129, !llfort.type_idx !188
  %"main_$C.dim_info$285" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$C", i32 0, i32 6, i32 0
  %"main_$C.dim_info$.extent$287" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$C.dim_info$285", i32 0, i32 0
  %"main_$C.dim_info$.extent$287[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$C.dim_info$.extent$287", i32 1), !llfort.type_idx !189
  %"main_$C.dim_info$.extent$287[]_fetch.114" = load i64, i64* %"main_$C.dim_info$.extent$287[]", align 8, !tbaa !129, !llfort.type_idx !189
  %mul.25 = mul nsw i64 %"main_$C.dim_info$.extent$283[]_fetch.113", %"main_$C.dim_info$.extent$287[]_fetch.114"
  %mul.26 = mul nsw i64 %"main_$C.addr_length$279_fetch.112", %mul.25
  br label %bb_new46

bb_new46:                                         ; preds = %bb_new45
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.ENTER.DATA"(), "QUAL.OMP.MAP.TO"(%"QNCA_a0$i64*$rank2$"* %"main_$C", %"QNCA_a0$i64*$rank2$"* %"main_$C", i64 96, i64 0, i8* null, i8* null), "QUAL.OMP.MAP.TO:CHAIN"(%"QNCA_a0$i64*$rank2$"* %"main_$C", i64* %"main_$C.addr_a0$275_fetch.111", i64 %mul.26, i64 281474976710673, i8* null, i8* null), "QUAL.OMP.MAP.TO:CHAIN"(%"QNCA_a0$i64*$rank2$"* %"main_$C", i64* %"main_$C.addr_length$277", i64 88, i64 281474976710657, i8* null, i8* null) ]
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TARGET.ENTER.DATA"() ]
  %"main_$NO_MAX_REP_fetch.115" = load i32, i32* @"main_$NO_MAX_REP", align 8, !tbaa !190, !llfort.type_idx !27
  %"main_$WARMUP_fetch.116" = load i32, i32* @"main_$WARMUP", align 8, !tbaa !192, !llfort.type_idx !26
  %add.31 = add nsw i32 %"main_$NO_MAX_REP_fetch.115", %"main_$WARMUP_fetch.116"
  store i32 %add.31, i32* %"var$18", align 4, !tbaa !75
  store i32 1, i32* %"main_$IREP", align 8, !tbaa !194
  %"var$18_fetch.117" = load i32, i32* %"var$18", align 4, !tbaa !75, !llfort.type_idx !43
  %rel.46 = icmp slt i32 %"var$18_fetch.117", 1
  br i1 %rel.46, label %bb25, label %bb26

bb26:                                             ; preds = %bb_new46
  br label %bb24

bb24:                                             ; preds = %omp.pdo.epilog59, %bb26
  %"main_$IREP_fetch.118" = load i32, i32* %"main_$IREP", align 8, !tbaa !194, !llfort.type_idx !34
  %"main_$WARMUP_fetch.119" = load i32, i32* @"main_$WARMUP", align 8, !tbaa !192, !llfort.type_idx !26
  %add.32 = add nsw i32 %"main_$WARMUP_fetch.119", 1
  %rel.47 = icmp eq i32 %"main_$IREP_fetch.118", %add.32
  %int_zext290 = zext i1 %rel.47 to i32, !llfort.type_idx !43
  %int_zext291 = trunc i32 %int_zext290 to i1, !llfort.type_idx !196
  br i1 %int_zext291, label %call.pre.list48_then, label %bb28_else

call.pre.list48_then:                             ; preds = %bb24
  %func_result289 = call reassoc ninf nsz arcp contract afn double @omp_get_wtime(), !llfort.type_idx !197
  store double %func_result289, double* %"main_$TIME", align 8, !tbaa !198
  br label %bb29_endif

bb28_else:                                        ; preds = %bb24
  br label %bb29_endif

bb29_endif:                                       ; preds = %bb28_else, %call.pre.list48_then
  %"main_$C.addr_a0$293" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$C", i32 0, i32 0
  %"main_$C.addr_a0$293_fetch.120" = load i64*, i64** %"main_$C.addr_a0$293", align 8, !tbaa !138, !llfort.type_idx !42
  %"main_$C.addr_length$295" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$C", i32 0, i32 1
  %"main_$C.addr_length$297" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$C", i32 0, i32 1
  %"main_$C.addr_length$297_fetch.121" = load i64, i64* %"main_$C.addr_length$297", align 8, !tbaa !123, !llfort.type_idx !183
  %"main_$C.dim_info$299" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$C", i32 0, i32 6, i32 0
  %"main_$C.dim_info$.extent$301" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$C.dim_info$299", i32 0, i32 0
  %"main_$C.dim_info$.extent$301[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$C.dim_info$.extent$301", i32 0), !llfort.type_idx !200
  %"main_$C.dim_info$.extent$301[]_fetch.122" = load i64, i64* %"main_$C.dim_info$.extent$301[]", align 8, !tbaa !129, !llfort.type_idx !200
  %"main_$C.dim_info$303" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$C", i32 0, i32 6, i32 0
  %"main_$C.dim_info$.extent$305" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$C.dim_info$303", i32 0, i32 0
  %"main_$C.dim_info$.extent$305[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$C.dim_info$.extent$305", i32 1), !llfort.type_idx !201
  %"main_$C.dim_info$.extent$305[]_fetch.123" = load i64, i64* %"main_$C.dim_info$.extent$305[]", align 8, !tbaa !129, !llfort.type_idx !201
  %mul.27 = mul nsw i64 %"main_$C.dim_info$.extent$301[]_fetch.122", %"main_$C.dim_info$.extent$305[]_fetch.123"
  %mul.28 = mul nsw i64 %"main_$C.addr_length$297_fetch.121", %mul.27
  %"main_$A.addr_a0$307" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$A", i32 0, i32 0
  %"main_$A.addr_a0$307_fetch.124" = load i64*, i64** %"main_$A.addr_a0$307", align 8, !tbaa !161, !llfort.type_idx !42
  %"main_$A.addr_length$309" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$A", i32 0, i32 1
  %"main_$A.addr_length$311" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$A", i32 0, i32 1
  %"main_$A.addr_length$311_fetch.125" = load i64, i64* %"main_$A.addr_length$311", align 8, !tbaa !82, !llfort.type_idx !183
  %"main_$A.dim_info$313" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$A", i32 0, i32 6, i32 0
  %"main_$A.dim_info$.extent$315" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$A.dim_info$313", i32 0, i32 0
  %"main_$A.dim_info$.extent$315[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$A.dim_info$.extent$315", i32 0), !llfort.type_idx !202
  %"main_$A.dim_info$.extent$315[]_fetch.126" = load i64, i64* %"main_$A.dim_info$.extent$315[]", align 8, !tbaa !88, !llfort.type_idx !202
  %"main_$A.dim_info$317" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$A", i32 0, i32 6, i32 0
  %"main_$A.dim_info$.extent$319" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$A.dim_info$317", i32 0, i32 0
  %"main_$A.dim_info$.extent$319[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$A.dim_info$.extent$319", i32 1), !llfort.type_idx !203
  %"main_$A.dim_info$.extent$319[]_fetch.127" = load i64, i64* %"main_$A.dim_info$.extent$319[]", align 8, !tbaa !88, !llfort.type_idx !203
  %mul.29 = mul nsw i64 %"main_$A.dim_info$.extent$315[]_fetch.126", %"main_$A.dim_info$.extent$319[]_fetch.127"
  %mul.30 = mul nsw i64 %"main_$A.addr_length$311_fetch.125", %mul.29
  %"main_$B.addr_a0$321" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$B", i32 0, i32 0
  %"main_$B.addr_a0$321_fetch.128" = load i64*, i64** %"main_$B.addr_a0$321", align 8, !tbaa !172, !llfort.type_idx !42
  %"main_$B.addr_length$323" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$B", i32 0, i32 1
  %"main_$B.addr_length$325" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$B", i32 0, i32 1
  %"main_$B.addr_length$325_fetch.129" = load i64, i64* %"main_$B.addr_length$325", align 8, !tbaa !104, !llfort.type_idx !183
  %"main_$B.dim_info$327" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$B", i32 0, i32 6, i32 0
  %"main_$B.dim_info$.extent$329" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$B.dim_info$327", i32 0, i32 0
  %"main_$B.dim_info$.extent$329[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$B.dim_info$.extent$329", i32 0), !llfort.type_idx !204
  %"main_$B.dim_info$.extent$329[]_fetch.130" = load i64, i64* %"main_$B.dim_info$.extent$329[]", align 8, !tbaa !110, !llfort.type_idx !204
  %"main_$B.dim_info$331" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$B", i32 0, i32 6, i32 0
  %"main_$B.dim_info$.extent$333" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$B.dim_info$331", i32 0, i32 0
  %"main_$B.dim_info$.extent$333[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$B.dim_info$.extent$333", i32 1), !llfort.type_idx !205
  %"main_$B.dim_info$.extent$333[]_fetch.131" = load i64, i64* %"main_$B.dim_info$.extent$333[]", align 8, !tbaa !110, !llfort.type_idx !205
  %mul.31 = mul nsw i64 %"main_$B.dim_info$.extent$329[]_fetch.130", %"main_$B.dim_info$.extent$333[]_fetch.131"
  %mul.32 = mul nsw i64 %"main_$B.addr_length$325_fetch.129", %mul.31
  %"main_$N_fetch.132" = load i64, i64* @"main_$N", align 8, !tbaa !73, !llfort.type_idx !28
  %omp.pdo.end = alloca i64, align 8, !llfort.type_idx !42
  store i64 %"main_$N_fetch.132", i64* %omp.pdo.end, align 8, !tbaa !75
  %omp.pdo.norm.iv = alloca i64, align 8, !llfort.type_idx !42
  %omp.pdo.norm.lb = alloca i64, align 8, !llfort.type_idx !42
  store i64 0, i64* %omp.pdo.norm.lb, align 8, !tbaa !75
  %omp.pdo.norm.ub = alloca i64, align 8, !llfort.type_idx !42
  %omp.pdo.end_fetch.133 = load i64, i64* %omp.pdo.end, align 8, !tbaa !75, !llfort.type_idx !42
  %sub.17 = sub nsw i64 %omp.pdo.end_fetch.133, 1
  %add.33 = add nsw i64 %sub.17, 1
  %sub.18 = sub nsw i64 %add.33, 1
  store i64 %sub.18, i64* %omp.pdo.norm.ub, align 8, !tbaa !75
  br label %bb_new54

omp.pdo.cond57:                                   ; preds = %bb_new60, %loop_exit66
  %omp.pdo.norm.iv_fetch.135 = load i64, i64* %omp.pdo.norm.iv, align 8, !tbaa !75, !llfort.type_idx !42
  %omp.pdo.norm.ub_fetch.136 = load i64, i64* %omp.pdo.norm.ub, align 8, !tbaa !75, !llfort.type_idx !42
  %rel.48 = icmp sle i64 %omp.pdo.norm.iv_fetch.135, %omp.pdo.norm.ub_fetch.136
  br i1 %rel.48, label %omp.pdo.body58, label %omp.pdo.epilog59

omp.pdo.body58:                                   ; preds = %omp.pdo.cond57
  %omp.pdo.norm.iv_fetch.137 = load i64, i64* %omp.pdo.norm.iv, align 8, !tbaa !75, !llfort.type_idx !42
  %add.34 = add nsw i64 %omp.pdo.norm.iv_fetch.137, 1
  store i64 %add.34, i64* %"main_$J", align 8, !tbaa !157
  %"$loop_ctr334" = alloca i64, align 8, !llfort.type_idx !42
  %"main_$C.addr_a0$335" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$C", i32 0, i32 0
  %"main_$C.addr_a0$_fetch.138" = load i64*, i64** %"main_$C.addr_a0$335", align 8, !tbaa !138, !llfort.type_idx !42
  %"main_$C.dim_info$336" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$C", i32 0, i32 6, i32 0
  %"main_$C.dim_info$.lower_bound$337" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$C.dim_info$336", i32 0, i32 2
  %"main_$C.dim_info$.lower_bound$[]338" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$C.dim_info$.lower_bound$337", i32 0), !llfort.type_idx !206
  %"main_$C.dim_info$.lower_bound$[]_fetch.139" = load i64, i64* %"main_$C.dim_info$.lower_bound$[]338", align 8, !tbaa !127, !llfort.type_idx !206
  %"main_$C.dim_info$339" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$C", i32 0, i32 6, i32 0
  %"main_$C.dim_info$.lower_bound$340" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$C.dim_info$339", i32 0, i32 2
  %"main_$C.dim_info$.lower_bound$[]341" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$C.dim_info$.lower_bound$340", i32 0), !llfort.type_idx !207
  %"main_$C.dim_info$.lower_bound$[]_fetch.140" = load i64, i64* %"main_$C.dim_info$.lower_bound$[]341", align 8, !tbaa !127, !llfort.type_idx !207
  %"main_$C.dim_info$342" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$C", i32 0, i32 6, i32 0
  %"main_$C.dim_info$.lower_bound$343" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$C.dim_info$342", i32 0, i32 2
  %"main_$C.dim_info$.lower_bound$[]344" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$C.dim_info$.lower_bound$343", i32 0), !llfort.type_idx !208
  %"main_$C.dim_info$.lower_bound$[]_fetch.141" = load i64, i64* %"main_$C.dim_info$.lower_bound$[]344", align 8, !tbaa !127, !llfort.type_idx !208
  %"main_$C.dim_info$345" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$C", i32 0, i32 6, i32 0
  %"main_$C.dim_info$.extent$346" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$C.dim_info$345", i32 0, i32 0
  %"main_$C.dim_info$.extent$[]347" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$C.dim_info$.extent$346", i32 0), !llfort.type_idx !209
  %"main_$C.dim_info$.extent$[]_fetch.142" = load i64, i64* %"main_$C.dim_info$.extent$[]347", align 8, !tbaa !129, !llfort.type_idx !209
  %add.35 = add nsw i64 %"main_$C.dim_info$.lower_bound$[]_fetch.141", %"main_$C.dim_info$.extent$[]_fetch.142"
  %sub.19 = sub nsw i64 %add.35, 1
  %"main_$C.dim_info$348" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$C", i32 0, i32 6, i32 0
  %"main_$C.dim_info$.extent$349" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$C.dim_info$348", i32 0, i32 0
  %"main_$C.dim_info$.extent$[]350" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$C.dim_info$.extent$349", i32 0), !llfort.type_idx !210
  %"main_$C.dim_info$.extent$[]_fetch.143" = load i64, i64* %"main_$C.dim_info$.extent$[]350", align 8, !tbaa !129, !llfort.type_idx !210
  %"var$20" = alloca i64, align 8, !llfort.type_idx !42
  %"main_$C.dim_info$351" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$C", i32 0, i32 6, i32 0
  %"main_$C.dim_info$.spacing$352" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$C.dim_info$351", i32 0, i32 1
  %"main_$C.dim_info$.spacing$[]353" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$C.dim_info$.spacing$352", i32 1), !llfort.type_idx !211
  %"main_$C.dim_info$.spacing$[]_fetch.144" = load i64, i64* %"main_$C.dim_info$.spacing$[]353", align 8, !tbaa !133, !range !97, !llfort.type_idx !211
  %"main_$C.dim_info$354" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$C", i32 0, i32 6, i32 0
  %"main_$C.dim_info$.lower_bound$355" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$C.dim_info$354", i32 0, i32 2
  %"main_$C.dim_info$.lower_bound$[]356" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$C.dim_info$.lower_bound$355", i32 1), !llfort.type_idx !212
  %"main_$C.dim_info$.lower_bound$[]_fetch.145" = load i64, i64* %"main_$C.dim_info$.lower_bound$[]356", align 8, !tbaa !127, !llfort.type_idx !212
  %"main_$J_fetch.146" = load i64, i64* %"main_$J", align 8, !tbaa !157, !llfort.type_idx !38
  %"main_$C.dim_info$357" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$C", i32 0, i32 6, i32 0
  %"main_$C.dim_info$.lower_bound$358" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$C.dim_info$357", i32 0, i32 2
  %"main_$C.dim_info$.lower_bound$[]359" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$C.dim_info$.lower_bound$358", i32 0), !llfort.type_idx !213
  %"main_$C.dim_info$.lower_bound$[]_fetch.147" = load i64, i64* %"main_$C.dim_info$.lower_bound$[]359", align 8, !tbaa !127, !llfort.type_idx !213
  %mul.33 = mul nsw i64 %"main_$C.dim_info$.lower_bound$[]_fetch.147", 8
  %"main_$C.dim_info$360" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$C", i32 0, i32 6, i32 0
  %"main_$C.dim_info$.lower_bound$361" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$C.dim_info$360", i32 0, i32 2
  %"main_$C.dim_info$.lower_bound$[]362" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$C.dim_info$.lower_bound$361", i32 1), !llfort.type_idx !214
  %"main_$C.dim_info$.lower_bound$[]_fetch.148" = load i64, i64* %"main_$C.dim_info$.lower_bound$[]362", align 8, !tbaa !127, !llfort.type_idx !214
  %"main_$C.dim_info$363" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$C", i32 0, i32 6, i32 0
  %"main_$C.dim_info$.spacing$364" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$C.dim_info$363", i32 0, i32 1
  %"main_$C.dim_info$.spacing$[]365" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$C.dim_info$.spacing$364", i32 1), !llfort.type_idx !215
  %"main_$C.dim_info$.spacing$[]_fetch.149" = load i64, i64* %"main_$C.dim_info$.spacing$[]365", align 8, !tbaa !133, !range !97, !llfort.type_idx !215
  %mul.34 = mul nsw i64 %"main_$C.dim_info$.lower_bound$[]_fetch.148", %"main_$C.dim_info$.spacing$[]_fetch.149"
  %add.36 = add nsw i64 %mul.33, %mul.34
  %"main_$A.addr_a0$366" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$A", i32 0, i32 0
  %"main_$A.addr_a0$_fetch.151" = load i64*, i64** %"main_$A.addr_a0$366", align 8, !tbaa !161, !llfort.type_idx !42
  %"main_$A.dim_info$367" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$A", i32 0, i32 6, i32 0
  %"main_$A.dim_info$.lower_bound$368" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$A.dim_info$367", i32 0, i32 2
  %"main_$A.dim_info$.lower_bound$[]369" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$A.dim_info$.lower_bound$368", i32 0), !llfort.type_idx !216
  %"main_$A.dim_info$.lower_bound$[]_fetch.152" = load i64, i64* %"main_$A.dim_info$.lower_bound$[]369", align 8, !tbaa !86, !llfort.type_idx !216
  %"main_$A.dim_info$370" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$A", i32 0, i32 6, i32 0
  %"main_$A.dim_info$.lower_bound$371" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$A.dim_info$370", i32 0, i32 2
  %"main_$A.dim_info$.lower_bound$[]372" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$A.dim_info$.lower_bound$371", i32 0), !llfort.type_idx !217
  %"main_$A.dim_info$.lower_bound$[]_fetch.153" = load i64, i64* %"main_$A.dim_info$.lower_bound$[]372", align 8, !tbaa !86, !llfort.type_idx !217
  %"main_$A.dim_info$373" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$A", i32 0, i32 6, i32 0
  %"main_$A.dim_info$.lower_bound$374" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$A.dim_info$373", i32 0, i32 2
  %"main_$A.dim_info$.lower_bound$[]375" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$A.dim_info$.lower_bound$374", i32 0), !llfort.type_idx !218
  %"main_$A.dim_info$.lower_bound$[]_fetch.154" = load i64, i64* %"main_$A.dim_info$.lower_bound$[]375", align 8, !tbaa !86, !llfort.type_idx !218
  %"main_$A.dim_info$376" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$A", i32 0, i32 6, i32 0
  %"main_$A.dim_info$.extent$377" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$A.dim_info$376", i32 0, i32 0
  %"main_$A.dim_info$.extent$[]378" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$A.dim_info$.extent$377", i32 0), !llfort.type_idx !219
  %"main_$A.dim_info$.extent$[]_fetch.155" = load i64, i64* %"main_$A.dim_info$.extent$[]378", align 8, !tbaa !88, !llfort.type_idx !219
  %add.37 = add nsw i64 %"main_$A.dim_info$.lower_bound$[]_fetch.154", %"main_$A.dim_info$.extent$[]_fetch.155"
  %sub.20 = sub nsw i64 %add.37, 1
  %"main_$A.dim_info$379" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$A", i32 0, i32 6, i32 0
  %"main_$A.dim_info$.extent$380" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$A.dim_info$379", i32 0, i32 0
  %"main_$A.dim_info$.extent$[]381" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$A.dim_info$.extent$380", i32 0), !llfort.type_idx !220
  %"main_$A.dim_info$.extent$[]_fetch.156" = load i64, i64* %"main_$A.dim_info$.extent$[]381", align 8, !tbaa !88, !llfort.type_idx !220
  %"var$21" = alloca i64, align 8, !llfort.type_idx !42
  %"main_$A.dim_info$382" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$A", i32 0, i32 6, i32 0
  %"main_$A.dim_info$.spacing$383" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$A.dim_info$382", i32 0, i32 1
  %"main_$A.dim_info$.spacing$[]384" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$A.dim_info$.spacing$383", i32 1), !llfort.type_idx !221
  %"main_$A.dim_info$.spacing$[]_fetch.157" = load i64, i64* %"main_$A.dim_info$.spacing$[]384", align 8, !tbaa !92, !range !97, !llfort.type_idx !221
  %"main_$A.dim_info$385" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$A", i32 0, i32 6, i32 0
  %"main_$A.dim_info$.lower_bound$386" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$A.dim_info$385", i32 0, i32 2
  %"main_$A.dim_info$.lower_bound$[]387" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$A.dim_info$.lower_bound$386", i32 1), !llfort.type_idx !222
  %"main_$A.dim_info$.lower_bound$[]_fetch.158" = load i64, i64* %"main_$A.dim_info$.lower_bound$[]387", align 8, !tbaa !86, !llfort.type_idx !222
  %"main_$J_fetch.159" = load i64, i64* %"main_$J", align 8, !tbaa !157, !llfort.type_idx !38
  %"main_$A.dim_info$388" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$A", i32 0, i32 6, i32 0
  %"main_$A.dim_info$.lower_bound$389" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$A.dim_info$388", i32 0, i32 2
  %"main_$A.dim_info$.lower_bound$[]390" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$A.dim_info$.lower_bound$389", i32 0), !llfort.type_idx !223
  %"main_$A.dim_info$.lower_bound$[]_fetch.160" = load i64, i64* %"main_$A.dim_info$.lower_bound$[]390", align 8, !tbaa !86, !llfort.type_idx !223
  %mul.35 = mul nsw i64 %"main_$A.dim_info$.lower_bound$[]_fetch.160", 8
  %"main_$A.dim_info$391" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$A", i32 0, i32 6, i32 0
  %"main_$A.dim_info$.lower_bound$392" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$A.dim_info$391", i32 0, i32 2
  %"main_$A.dim_info$.lower_bound$[]393" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$A.dim_info$.lower_bound$392", i32 1), !llfort.type_idx !224
  %"main_$A.dim_info$.lower_bound$[]_fetch.161" = load i64, i64* %"main_$A.dim_info$.lower_bound$[]393", align 8, !tbaa !86, !llfort.type_idx !224
  %"main_$A.dim_info$394" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$A", i32 0, i32 6, i32 0
  %"main_$A.dim_info$.spacing$395" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$A.dim_info$394", i32 0, i32 1
  %"main_$A.dim_info$.spacing$[]396" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$A.dim_info$.spacing$395", i32 1), !llfort.type_idx !225
  %"main_$A.dim_info$.spacing$[]_fetch.162" = load i64, i64* %"main_$A.dim_info$.spacing$[]396", align 8, !tbaa !92, !range !97, !llfort.type_idx !225
  %mul.36 = mul nsw i64 %"main_$A.dim_info$.lower_bound$[]_fetch.161", %"main_$A.dim_info$.spacing$[]_fetch.162"
  %add.38 = add nsw i64 %mul.35, %mul.36
  %"main_$B.addr_a0$397" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$B", i32 0, i32 0
  %"main_$B.addr_a0$_fetch.165" = load i64*, i64** %"main_$B.addr_a0$397", align 8, !tbaa !172, !llfort.type_idx !42
  %"main_$B.dim_info$398" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$B", i32 0, i32 6, i32 0
  %"main_$B.dim_info$.lower_bound$399" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$B.dim_info$398", i32 0, i32 2
  %"main_$B.dim_info$.lower_bound$[]400" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$B.dim_info$.lower_bound$399", i32 0), !llfort.type_idx !226
  %"main_$B.dim_info$.lower_bound$[]_fetch.166" = load i64, i64* %"main_$B.dim_info$.lower_bound$[]400", align 8, !tbaa !108, !llfort.type_idx !226
  %"main_$B.dim_info$401" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$B", i32 0, i32 6, i32 0
  %"main_$B.dim_info$.lower_bound$402" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$B.dim_info$401", i32 0, i32 2
  %"main_$B.dim_info$.lower_bound$[]403" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$B.dim_info$.lower_bound$402", i32 0), !llfort.type_idx !227
  %"main_$B.dim_info$.lower_bound$[]_fetch.167" = load i64, i64* %"main_$B.dim_info$.lower_bound$[]403", align 8, !tbaa !108, !llfort.type_idx !227
  %"main_$B.dim_info$404" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$B", i32 0, i32 6, i32 0
  %"main_$B.dim_info$.lower_bound$405" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$B.dim_info$404", i32 0, i32 2
  %"main_$B.dim_info$.lower_bound$[]406" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$B.dim_info$.lower_bound$405", i32 0), !llfort.type_idx !228
  %"main_$B.dim_info$.lower_bound$[]_fetch.168" = load i64, i64* %"main_$B.dim_info$.lower_bound$[]406", align 8, !tbaa !108, !llfort.type_idx !228
  %"main_$B.dim_info$407" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$B", i32 0, i32 6, i32 0
  %"main_$B.dim_info$.extent$408" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$B.dim_info$407", i32 0, i32 0
  %"main_$B.dim_info$.extent$[]409" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$B.dim_info$.extent$408", i32 0), !llfort.type_idx !229
  %"main_$B.dim_info$.extent$[]_fetch.169" = load i64, i64* %"main_$B.dim_info$.extent$[]409", align 8, !tbaa !110, !llfort.type_idx !229
  %add.39 = add nsw i64 %"main_$B.dim_info$.lower_bound$[]_fetch.168", %"main_$B.dim_info$.extent$[]_fetch.169"
  %sub.21 = sub nsw i64 %add.39, 1
  %"main_$B.dim_info$410" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$B", i32 0, i32 6, i32 0
  %"main_$B.dim_info$.extent$411" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$B.dim_info$410", i32 0, i32 0
  %"main_$B.dim_info$.extent$[]412" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$B.dim_info$.extent$411", i32 0), !llfort.type_idx !230
  %"main_$B.dim_info$.extent$[]_fetch.170" = load i64, i64* %"main_$B.dim_info$.extent$[]412", align 8, !tbaa !110, !llfort.type_idx !230
  %"var$22" = alloca i64, align 8, !llfort.type_idx !42
  %"main_$B.dim_info$413" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$B", i32 0, i32 6, i32 0
  %"main_$B.dim_info$.spacing$414" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$B.dim_info$413", i32 0, i32 1
  %"main_$B.dim_info$.spacing$[]415" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$B.dim_info$.spacing$414", i32 1), !llfort.type_idx !231
  %"main_$B.dim_info$.spacing$[]_fetch.171" = load i64, i64* %"main_$B.dim_info$.spacing$[]415", align 8, !tbaa !114, !range !97, !llfort.type_idx !231
  %"main_$B.dim_info$416" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$B", i32 0, i32 6, i32 0
  %"main_$B.dim_info$.lower_bound$417" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$B.dim_info$416", i32 0, i32 2
  %"main_$B.dim_info$.lower_bound$[]418" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$B.dim_info$.lower_bound$417", i32 1), !llfort.type_idx !232
  %"main_$B.dim_info$.lower_bound$[]_fetch.172" = load i64, i64* %"main_$B.dim_info$.lower_bound$[]418", align 8, !tbaa !108, !llfort.type_idx !232
  %"main_$J_fetch.173" = load i64, i64* %"main_$J", align 8, !tbaa !157, !llfort.type_idx !38
  %"main_$B.dim_info$419" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$B", i32 0, i32 6, i32 0
  %"main_$B.dim_info$.lower_bound$420" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$B.dim_info$419", i32 0, i32 2
  %"main_$B.dim_info$.lower_bound$[]421" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$B.dim_info$.lower_bound$420", i32 0), !llfort.type_idx !233
  %"main_$B.dim_info$.lower_bound$[]_fetch.174" = load i64, i64* %"main_$B.dim_info$.lower_bound$[]421", align 8, !tbaa !108, !llfort.type_idx !233
  %mul.37 = mul nsw i64 %"main_$B.dim_info$.lower_bound$[]_fetch.174", 8
  %"main_$B.dim_info$422" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$B", i32 0, i32 6, i32 0
  %"main_$B.dim_info$.lower_bound$423" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$B.dim_info$422", i32 0, i32 2
  %"main_$B.dim_info$.lower_bound$[]424" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$B.dim_info$.lower_bound$423", i32 1), !llfort.type_idx !234
  %"main_$B.dim_info$.lower_bound$[]_fetch.175" = load i64, i64* %"main_$B.dim_info$.lower_bound$[]424", align 8, !tbaa !108, !llfort.type_idx !234
  %"main_$B.dim_info$425" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$B", i32 0, i32 6, i32 0
  %"main_$B.dim_info$.spacing$426" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$B.dim_info$425", i32 0, i32 1
  %"main_$B.dim_info$.spacing$[]427" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$B.dim_info$.spacing$426", i32 1), !llfort.type_idx !235
  %"main_$B.dim_info$.spacing$[]_fetch.176" = load i64, i64* %"main_$B.dim_info$.spacing$[]427", align 8, !tbaa !114, !range !97, !llfort.type_idx !235
  %mul.38 = mul nsw i64 %"main_$B.dim_info$.lower_bound$[]_fetch.175", %"main_$B.dim_info$.spacing$[]_fetch.176"
  %add.40 = add nsw i64 %mul.37, %mul.38
  store i64 %"main_$B.dim_info$.lower_bound$[]_fetch.167", i64* %"var$22", align 8, !tbaa !75
  store i64 %"main_$A.dim_info$.lower_bound$[]_fetch.153", i64* %"var$21", align 8, !tbaa !75
  store i64 %"main_$C.dim_info$.lower_bound$[]_fetch.140", i64* %"var$20", align 8, !tbaa !75
  store i64 1, i64* %"$loop_ctr334", align 8, !tbaa !75
  br label %loop_test64

loop_test64:                                      ; preds = %loop_body65, %omp.pdo.body58
  %"$loop_ctr_fetch.179" = load i64, i64* %"$loop_ctr334", align 8, !tbaa !75, !llfort.type_idx !42
  %rel.49 = icmp sle i64 %"$loop_ctr_fetch.179", %"main_$C.dim_info$.extent$[]_fetch.143"
  br i1 %rel.49, label %loop_body65, label %loop_exit66

loop_body65:                                      ; preds = %loop_test64
  %"main_$A.addr_a0$_fetch.151[]" = call i64* @llvm.intel.subscript.p0i64.i64.i64.p0i64.i64(i8 1, i64 %"main_$A.dim_info$.lower_bound$[]_fetch.158", i64 %"main_$A.dim_info$.spacing$[]_fetch.157", i64* elementtype(i64) %"main_$A.addr_a0$_fetch.151", i64 %"main_$J_fetch.159"), !llfort.type_idx !236
  %"var$21_fetch.163" = load i64, i64* %"var$21", align 8, !tbaa !75, !llfort.type_idx !42
  %"main_$A.addr_a0$_fetch.151[][]" = call i64* @llvm.intel.subscript.p0i64.i64.i64.p0i64.i64(i8 0, i64 %"main_$A.dim_info$.lower_bound$[]_fetch.152", i64 8, i64* elementtype(i64) %"main_$A.addr_a0$_fetch.151[]", i64 %"var$21_fetch.163"), !llfort.type_idx !237
  %"main_$A.addr_a0$_fetch.151[][]_fetch.164" = load i64, i64* %"main_$A.addr_a0$_fetch.151[][]", align 8, !tbaa !170, !llfort.type_idx !237
  %"main_$B.addr_a0$_fetch.165[]" = call i64* @llvm.intel.subscript.p0i64.i64.i64.p0i64.i64(i8 1, i64 %"main_$B.dim_info$.lower_bound$[]_fetch.172", i64 %"main_$B.dim_info$.spacing$[]_fetch.171", i64* elementtype(i64) %"main_$B.addr_a0$_fetch.165", i64 %"main_$J_fetch.173"), !llfort.type_idx !238
  %"var$22_fetch.177" = load i64, i64* %"var$22", align 8, !tbaa !75, !llfort.type_idx !42
  %"main_$B.addr_a0$_fetch.165[][]" = call i64* @llvm.intel.subscript.p0i64.i64.i64.p0i64.i64(i8 0, i64 %"main_$B.dim_info$.lower_bound$[]_fetch.166", i64 8, i64* elementtype(i64) %"main_$B.addr_a0$_fetch.165[]", i64 %"var$22_fetch.177"), !llfort.type_idx !239
  %"main_$B.addr_a0$_fetch.165[][]_fetch.178" = load i64, i64* %"main_$B.addr_a0$_fetch.165[][]", align 8, !tbaa !181, !llfort.type_idx !239
  %add.41 = add nsw i64 %"main_$A.addr_a0$_fetch.151[][]_fetch.164", %"main_$B.addr_a0$_fetch.165[][]_fetch.178"
  %"main_$C.addr_a0$_fetch.138[]" = call i64* @llvm.intel.subscript.p0i64.i64.i64.p0i64.i64(i8 1, i64 %"main_$C.dim_info$.lower_bound$[]_fetch.145", i64 %"main_$C.dim_info$.spacing$[]_fetch.144", i64* elementtype(i64) %"main_$C.addr_a0$_fetch.138", i64 %"main_$J_fetch.146"), !llfort.type_idx !240
  %"var$20_fetch.150" = load i64, i64* %"var$20", align 8, !tbaa !75, !llfort.type_idx !42
  %"main_$C.addr_a0$_fetch.138[][]" = call i64* @llvm.intel.subscript.p0i64.i64.i64.p0i64.i64(i8 0, i64 %"main_$C.dim_info$.lower_bound$[]_fetch.139", i64 8, i64* elementtype(i64) %"main_$C.addr_a0$_fetch.138[]", i64 %"var$20_fetch.150"), !llfort.type_idx !241
  store i64 %add.41, i64* %"main_$C.addr_a0$_fetch.138[][]", align 8, !tbaa !155
  %"var$22_fetch.180" = load i64, i64* %"var$22", align 8, !tbaa !75, !llfort.type_idx !42
  %add.42 = add nsw i64 %"var$22_fetch.180", 1
  store i64 %add.42, i64* %"var$22", align 8, !tbaa !75
  %"var$21_fetch.181" = load i64, i64* %"var$21", align 8, !tbaa !75, !llfort.type_idx !42
  %add.43 = add nsw i64 %"var$21_fetch.181", 1
  store i64 %add.43, i64* %"var$21", align 8, !tbaa !75
  %"var$20_fetch.182" = load i64, i64* %"var$20", align 8, !tbaa !75, !llfort.type_idx !42
  %add.44 = add nsw i64 %"var$20_fetch.182", 1
  store i64 %add.44, i64* %"var$20", align 8, !tbaa !75
  %"$loop_ctr_fetch.183" = load i64, i64* %"$loop_ctr334", align 8, !tbaa !75, !llfort.type_idx !42
  %add.45 = add nsw i64 %"$loop_ctr_fetch.183", 1
  store i64 %add.45, i64* %"$loop_ctr334", align 8, !tbaa !75
  br label %loop_test64

loop_exit66:                                      ; preds = %loop_test64
  %omp.pdo.norm.iv_fetch.184 = load i64, i64* %omp.pdo.norm.iv, align 8, !tbaa !75, !llfort.type_idx !42
  %add.46 = add nsw i64 %omp.pdo.norm.iv_fetch.184, 1
  store i64 %add.46, i64* %omp.pdo.norm.iv, align 8, !tbaa !75
  br label %omp.pdo.cond57

omp.pdo.epilog59:                                 ; preds = %omp.pdo.cond57
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]
  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.TARGET"() ]
  %"main_$IREP_fetch.185" = load i32, i32* %"main_$IREP", align 8, !tbaa !194, !llfort.type_idx !34
  %add.47 = add nsw i32 %"main_$IREP_fetch.185", 1
  store i32 %add.47, i32* %"main_$IREP", align 8, !tbaa !194
  %"main_$IREP_fetch.186" = load i32, i32* %"main_$IREP", align 8, !tbaa !194, !llfort.type_idx !34
  %"var$18_fetch.187" = load i32, i32* %"var$18", align 4, !tbaa !75, !llfort.type_idx !43
  %rel.50 = icmp sle i32 %"main_$IREP_fetch.186", %"var$18_fetch.187"
  br i1 %rel.50, label %bb24, label %bb27

bb_new60:                                         ; preds = %bb_new55
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(), "QUAL.OMP.SHARED"(%"QNCA_a0$i64*$rank2$"* %"main_$B"), "QUAL.OMP.SHARED"(%"QNCA_a0$i64*$rank2$"* %"main_$A"), "QUAL.OMP.SHARED"(%"QNCA_a0$i64*$rank2$"* %"main_$C"), "QUAL.OMP.SHARED"(i64* @"main_$N"), "QUAL.OMP.PRIVATE"(i64* %"main_$J"), "QUAL.OMP.FIRSTPRIVATE"(i64* %omp.pdo.norm.lb), "QUAL.OMP.NORMALIZED.IV"(i64* %omp.pdo.norm.iv), "QUAL.OMP.NORMALIZED.UB"(i64* %omp.pdo.norm.ub) ]
  %omp.pdo.norm.lb_fetch.134 = load i64, i64* %omp.pdo.norm.lb, align 8, !tbaa !75, !llfort.type_idx !42
  store i64 %omp.pdo.norm.lb_fetch.134, i64* %omp.pdo.norm.iv, align 8, !tbaa !75
  br label %omp.pdo.cond57

bb_new54:                                         ; preds = %bb29_endif
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.TOFROM"(%"QNCA_a0$i64*$rank2$"* %"main_$C", %"QNCA_a0$i64*$rank2$"* %"main_$C", i64 96, i64 32, i8* null, i8* null), "QUAL.OMP.MAP.TOFROM:CHAIN"(%"QNCA_a0$i64*$rank2$"* %"main_$C", i64* %"main_$C.addr_a0$293_fetch.120", i64 %mul.28, i64 281474976711187, i8* null, i8* null), "QUAL.OMP.MAP.TOFROM:CHAIN"(%"QNCA_a0$i64*$rank2$"* %"main_$C", i64* %"main_$C.addr_length$295", i64 88, i64 281474976710657, i8* null, i8* null), "QUAL.OMP.MAP.TOFROM"(%"QNCA_a0$i64*$rank2$"* %"main_$A", %"QNCA_a0$i64*$rank2$"* %"main_$A", i64 96, i64 32, i8* null, i8* null), "QUAL.OMP.MAP.TOFROM:CHAIN"(%"QNCA_a0$i64*$rank2$"* %"main_$A", i64* %"main_$A.addr_a0$307_fetch.124", i64 %mul.30, i64 1125899906843155, i8* null, i8* null), "QUAL.OMP.MAP.TOFROM:CHAIN"(%"QNCA_a0$i64*$rank2$"* %"main_$A", i64* %"main_$A.addr_length$309", i64 88, i64 1125899906842625, i8* null, i8* null), "QUAL.OMP.MAP.TOFROM"(%"QNCA_a0$i64*$rank2$"* %"main_$B", %"QNCA_a0$i64*$rank2$"* %"main_$B", i64 96, i64 32, i8* null, i8* null), "QUAL.OMP.MAP.TOFROM:CHAIN"(%"QNCA_a0$i64*$rank2$"* %"main_$B", i64* %"main_$B.addr_a0$321_fetch.128", i64 %mul.32, i64 1970324836975123, i8* null, i8* null), "QUAL.OMP.MAP.TOFROM:CHAIN"(%"QNCA_a0$i64*$rank2$"* %"main_$B", i64* %"main_$B.addr_length$323", i64 88, i64 1970324836974593, i8* null, i8* null), "QUAL.OMP.FIRSTPRIVATE"(i64* @"main_$N"), "QUAL.OMP.FIRSTPRIVATE"(i64* %"main_$J"), "QUAL.OMP.PRIVATE"(i64* %omp.pdo.norm.iv), "QUAL.OMP.FIRSTPRIVATE"(i64* %omp.pdo.norm.lb), "QUAL.OMP.FIRSTPRIVATE"(i64* %omp.pdo.norm.ub) ]
  br label %bb_new55

bb_new55:                                         ; preds = %bb_new54
  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(), "QUAL.OMP.SHARED"(i64* %omp.pdo.norm.lb), "QUAL.OMP.SHARED"(i64* %omp.pdo.norm.ub), "QUAL.OMP.SHARED"(%"QNCA_a0$i64*$rank2$"* %"main_$B"), "QUAL.OMP.SHARED"(%"QNCA_a0$i64*$rank2$"* %"main_$A"), "QUAL.OMP.SHARED"(%"QNCA_a0$i64*$rank2$"* %"main_$C"), "QUAL.OMP.SHARED"(i64* @"main_$N"), "QUAL.OMP.PRIVATE"(i64* %omp.pdo.norm.iv), "QUAL.OMP.PRIVATE"(i64* %"main_$J") ]
  br label %bb_new60

bb27:                                             ; preds = %omp.pdo.epilog59
  br label %bb25

bb25:                                             ; preds = %bb27, %bb_new46
  %func_result429 = call reassoc ninf nsz arcp contract afn double @omp_get_wtime(), !llfort.type_idx !197
  %"main_$TIME_fetch.188" = load double, double* %"main_$TIME", align 8, !tbaa !198, !llfort.type_idx !36
  %sub.22 = fsub reassoc ninf nsz arcp contract afn double %func_result429, %"main_$TIME_fetch.188"
  store double %sub.22, double* %"main_$TIME", align 8, !tbaa !198
  %"main_$C.addr_a0$431" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$C", i32 0, i32 0
  %"main_$C.addr_a0$431_fetch.189" = load i64*, i64** %"main_$C.addr_a0$431", align 8, !tbaa !138, !llfort.type_idx !42
  %"main_$C.addr_length$433" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$C", i32 0, i32 1
  %"main_$C.addr_length$435" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$C", i32 0, i32 1
  %"main_$C.addr_length$435_fetch.190" = load i64, i64* %"main_$C.addr_length$435", align 8, !tbaa !123, !llfort.type_idx !183
  %"main_$C.dim_info$437" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$C", i32 0, i32 6, i32 0
  %"main_$C.dim_info$.extent$439" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$C.dim_info$437", i32 0, i32 0
  %"main_$C.dim_info$.extent$439[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$C.dim_info$.extent$439", i32 0), !llfort.type_idx !242
  %"main_$C.dim_info$.extent$439[]_fetch.191" = load i64, i64* %"main_$C.dim_info$.extent$439[]", align 8, !tbaa !129, !llfort.type_idx !242
  %"main_$C.dim_info$441" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$C", i32 0, i32 6, i32 0
  %"main_$C.dim_info$.extent$443" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$C.dim_info$441", i32 0, i32 0
  %"main_$C.dim_info$.extent$443[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$C.dim_info$.extent$443", i32 1), !llfort.type_idx !243
  %"main_$C.dim_info$.extent$443[]_fetch.192" = load i64, i64* %"main_$C.dim_info$.extent$443[]", align 8, !tbaa !129, !llfort.type_idx !243
  %mul.39 = mul nsw i64 %"main_$C.dim_info$.extent$439[]_fetch.191", %"main_$C.dim_info$.extent$443[]_fetch.192"
  %mul.40 = mul nsw i64 %"main_$C.addr_length$435_fetch.190", %mul.39
  br label %bb_new73

bb_new73:                                         ; preds = %bb25
  %6 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.UPDATE"(), "QUAL.OMP.MAP.FROM"(i64* %"main_$C.addr_a0$431_fetch.189", i64* %"main_$C.addr_a0$431_fetch.189", i64 %mul.40, i64 34, i8* null, i8* null) ]
  call void @llvm.directive.region.exit(token %6) [ "DIR.OMP.END.TARGET.UPDATE"() ]
  %"main_$N_fetch.193" = load i64, i64* @"main_$N", align 8, !tbaa !73, !llfort.type_idx !28
  store i64 %"main_$N_fetch.193", i64* %"var$23", align 8, !tbaa !75
  store i64 1, i64* %"main_$J", align 8, !tbaa !157
  %"var$23_fetch.194" = load i64, i64* %"var$23", align 8, !tbaa !75, !llfort.type_idx !42
  %rel.51 = icmp slt i64 %"var$23_fetch.194", 1
  br i1 %rel.51, label %bb36, label %bb37

bb37:                                             ; preds = %bb_new73
  br label %bb35

bb35:                                             ; preds = %bb40, %bb37
  %"main_$N_fetch.195" = load i64, i64* @"main_$N", align 8, !tbaa !73, !llfort.type_idx !28
  store i64 %"main_$N_fetch.195", i64* %"var$24", align 8, !tbaa !75
  store i64 1, i64* %"main_$I", align 8, !tbaa !159
  %"var$24_fetch.196" = load i64, i64* %"var$24", align 8, !tbaa !75, !llfort.type_idx !42
  %rel.52 = icmp slt i64 %"var$24_fetch.196", 1
  br i1 %rel.52, label %bb40, label %bb41

bb41:                                             ; preds = %bb35
  br label %bb39

bb39:                                             ; preds = %bb45_endif, %bb41
  %"main_$C.addr_a0$446" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$C", i32 0, i32 0
  %"main_$C.addr_a0$_fetch.197" = load i64*, i64** %"main_$C.addr_a0$446", align 8, !tbaa !138, !llfort.type_idx !42
  %"main_$C.dim_info$447" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$C", i32 0, i32 6, i32 0
  %"main_$C.dim_info$.lower_bound$448" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$C.dim_info$447", i32 0, i32 2
  %"main_$C.dim_info$.lower_bound$[]449" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$C.dim_info$.lower_bound$448", i32 0), !llfort.type_idx !244
  %"main_$C.dim_info$.lower_bound$[]_fetch.198" = load i64, i64* %"main_$C.dim_info$.lower_bound$[]449", align 8, !tbaa !127, !llfort.type_idx !244
  %"main_$I_fetch.199" = load i64, i64* %"main_$I", align 8, !tbaa !159, !llfort.type_idx !39
  %"main_$C.dim_info$450" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$C", i32 0, i32 6, i32 0
  %"main_$C.dim_info$.spacing$451" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$C.dim_info$450", i32 0, i32 1
  %"main_$C.dim_info$.spacing$[]452" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$C.dim_info$.spacing$451", i32 1), !llfort.type_idx !245
  %"main_$C.dim_info$.spacing$[]_fetch.200" = load i64, i64* %"main_$C.dim_info$.spacing$[]452", align 8, !tbaa !133, !range !97, !llfort.type_idx !245
  %"main_$C.dim_info$453" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$C", i32 0, i32 6, i32 0
  %"main_$C.dim_info$.lower_bound$454" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$C.dim_info$453", i32 0, i32 2
  %"main_$C.dim_info$.lower_bound$[]455" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$C.dim_info$.lower_bound$454", i32 1), !llfort.type_idx !246
  %"main_$C.dim_info$.lower_bound$[]_fetch.201" = load i64, i64* %"main_$C.dim_info$.lower_bound$[]455", align 8, !tbaa !127, !llfort.type_idx !246
  %"main_$J_fetch.202" = load i64, i64* %"main_$J", align 8, !tbaa !157, !llfort.type_idx !38
  %"main_$C.dim_info$456" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$C", i32 0, i32 6, i32 0
  %"main_$C.dim_info$.lower_bound$457" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$C.dim_info$456", i32 0, i32 2
  %"main_$C.dim_info$.lower_bound$[]458" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$C.dim_info$.lower_bound$457", i32 0), !llfort.type_idx !247
  %"main_$C.dim_info$.lower_bound$[]_fetch.203" = load i64, i64* %"main_$C.dim_info$.lower_bound$[]458", align 8, !tbaa !127, !llfort.type_idx !247
  %mul.41 = mul nsw i64 %"main_$C.dim_info$.lower_bound$[]_fetch.203", 8
  %"main_$C.dim_info$459" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$C", i32 0, i32 6, i32 0
  %"main_$C.dim_info$.lower_bound$460" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$C.dim_info$459", i32 0, i32 2
  %"main_$C.dim_info$.lower_bound$[]461" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$C.dim_info$.lower_bound$460", i32 1), !llfort.type_idx !248
  %"main_$C.dim_info$.lower_bound$[]_fetch.204" = load i64, i64* %"main_$C.dim_info$.lower_bound$[]461", align 8, !tbaa !127, !llfort.type_idx !248
  %"main_$C.dim_info$462" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$C", i32 0, i32 6, i32 0
  %"main_$C.dim_info$.spacing$463" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$C.dim_info$462", i32 0, i32 1
  %"main_$C.dim_info$.spacing$[]464" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$C.dim_info$.spacing$463", i32 1), !llfort.type_idx !249
  %"main_$C.dim_info$.spacing$[]_fetch.205" = load i64, i64* %"main_$C.dim_info$.spacing$[]464", align 8, !tbaa !133, !range !97, !llfort.type_idx !249
  %mul.42 = mul nsw i64 %"main_$C.dim_info$.lower_bound$[]_fetch.204", %"main_$C.dim_info$.spacing$[]_fetch.205"
  %add.48 = add nsw i64 %mul.41, %mul.42
  %"main_$C.addr_a0$_fetch.197[]" = call i64* @llvm.intel.subscript.p0i64.i64.i64.p0i64.i64(i8 1, i64 %"main_$C.dim_info$.lower_bound$[]_fetch.201", i64 %"main_$C.dim_info$.spacing$[]_fetch.200", i64* elementtype(i64) %"main_$C.addr_a0$_fetch.197", i64 %"main_$J_fetch.202"), !llfort.type_idx !250
  %"main_$C.addr_a0$_fetch.197[][]" = call i64* @llvm.intel.subscript.p0i64.i64.i64.p0i64.i64(i8 0, i64 %"main_$C.dim_info$.lower_bound$[]_fetch.198", i64 8, i64* elementtype(i64) %"main_$C.addr_a0$_fetch.197[]", i64 %"main_$I_fetch.199"), !llfort.type_idx !251
  %"main_$C.addr_a0$_fetch.197[][]_fetch.206" = load i64, i64* %"main_$C.addr_a0$_fetch.197[][]", align 8, !tbaa !155, !llfort.type_idx !251
  %"main_$A.addr_a0$465" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$A", i32 0, i32 0
  %"main_$A.addr_a0$_fetch.207" = load i64*, i64** %"main_$A.addr_a0$465", align 8, !tbaa !161, !llfort.type_idx !42
  %"main_$A.dim_info$466" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$A", i32 0, i32 6, i32 0
  %"main_$A.dim_info$.lower_bound$467" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$A.dim_info$466", i32 0, i32 2
  %"main_$A.dim_info$.lower_bound$[]468" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$A.dim_info$.lower_bound$467", i32 0), !llfort.type_idx !252
  %"main_$A.dim_info$.lower_bound$[]_fetch.208" = load i64, i64* %"main_$A.dim_info$.lower_bound$[]468", align 8, !tbaa !86, !llfort.type_idx !252
  %"main_$I_fetch.209" = load i64, i64* %"main_$I", align 8, !tbaa !159, !llfort.type_idx !39
  %"main_$A.dim_info$469" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$A", i32 0, i32 6, i32 0
  %"main_$A.dim_info$.spacing$470" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$A.dim_info$469", i32 0, i32 1
  %"main_$A.dim_info$.spacing$[]471" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$A.dim_info$.spacing$470", i32 1), !llfort.type_idx !253
  %"main_$A.dim_info$.spacing$[]_fetch.210" = load i64, i64* %"main_$A.dim_info$.spacing$[]471", align 8, !tbaa !92, !range !97, !llfort.type_idx !253
  %"main_$A.dim_info$472" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$A", i32 0, i32 6, i32 0
  %"main_$A.dim_info$.lower_bound$473" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$A.dim_info$472", i32 0, i32 2
  %"main_$A.dim_info$.lower_bound$[]474" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$A.dim_info$.lower_bound$473", i32 1), !llfort.type_idx !254
  %"main_$A.dim_info$.lower_bound$[]_fetch.211" = load i64, i64* %"main_$A.dim_info$.lower_bound$[]474", align 8, !tbaa !86, !llfort.type_idx !254
  %"main_$J_fetch.212" = load i64, i64* %"main_$J", align 8, !tbaa !157, !llfort.type_idx !38
  %"main_$A.dim_info$475" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$A", i32 0, i32 6, i32 0
  %"main_$A.dim_info$.lower_bound$476" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$A.dim_info$475", i32 0, i32 2
  %"main_$A.dim_info$.lower_bound$[]477" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$A.dim_info$.lower_bound$476", i32 0), !llfort.type_idx !255
  %"main_$A.dim_info$.lower_bound$[]_fetch.213" = load i64, i64* %"main_$A.dim_info$.lower_bound$[]477", align 8, !tbaa !86, !llfort.type_idx !255
  %mul.43 = mul nsw i64 %"main_$A.dim_info$.lower_bound$[]_fetch.213", 8
  %"main_$A.dim_info$478" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$A", i32 0, i32 6, i32 0
  %"main_$A.dim_info$.lower_bound$479" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$A.dim_info$478", i32 0, i32 2
  %"main_$A.dim_info$.lower_bound$[]480" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$A.dim_info$.lower_bound$479", i32 1), !llfort.type_idx !256
  %"main_$A.dim_info$.lower_bound$[]_fetch.214" = load i64, i64* %"main_$A.dim_info$.lower_bound$[]480", align 8, !tbaa !86, !llfort.type_idx !256
  %"main_$A.dim_info$481" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$A", i32 0, i32 6, i32 0
  %"main_$A.dim_info$.spacing$482" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$A.dim_info$481", i32 0, i32 1
  %"main_$A.dim_info$.spacing$[]483" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$A.dim_info$.spacing$482", i32 1), !llfort.type_idx !257
  %"main_$A.dim_info$.spacing$[]_fetch.215" = load i64, i64* %"main_$A.dim_info$.spacing$[]483", align 8, !tbaa !92, !range !97, !llfort.type_idx !257
  %mul.44 = mul nsw i64 %"main_$A.dim_info$.lower_bound$[]_fetch.214", %"main_$A.dim_info$.spacing$[]_fetch.215"
  %add.49 = add nsw i64 %mul.43, %mul.44
  %"main_$A.addr_a0$_fetch.207[]" = call i64* @llvm.intel.subscript.p0i64.i64.i64.p0i64.i64(i8 1, i64 %"main_$A.dim_info$.lower_bound$[]_fetch.211", i64 %"main_$A.dim_info$.spacing$[]_fetch.210", i64* elementtype(i64) %"main_$A.addr_a0$_fetch.207", i64 %"main_$J_fetch.212"), !llfort.type_idx !258
  %"main_$A.addr_a0$_fetch.207[][]" = call i64* @llvm.intel.subscript.p0i64.i64.i64.p0i64.i64(i8 0, i64 %"main_$A.dim_info$.lower_bound$[]_fetch.208", i64 8, i64* elementtype(i64) %"main_$A.addr_a0$_fetch.207[]", i64 %"main_$I_fetch.209"), !llfort.type_idx !259
  %"main_$A.addr_a0$_fetch.207[][]_fetch.216" = load i64, i64* %"main_$A.addr_a0$_fetch.207[][]", align 8, !tbaa !170, !llfort.type_idx !259
  %"main_$B.addr_a0$484" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$B", i32 0, i32 0
  %"main_$B.addr_a0$_fetch.217" = load i64*, i64** %"main_$B.addr_a0$484", align 8, !tbaa !172, !llfort.type_idx !42
  %"main_$B.dim_info$485" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$B", i32 0, i32 6, i32 0
  %"main_$B.dim_info$.lower_bound$486" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$B.dim_info$485", i32 0, i32 2
  %"main_$B.dim_info$.lower_bound$[]487" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$B.dim_info$.lower_bound$486", i32 0), !llfort.type_idx !260
  %"main_$B.dim_info$.lower_bound$[]_fetch.218" = load i64, i64* %"main_$B.dim_info$.lower_bound$[]487", align 8, !tbaa !108, !llfort.type_idx !260
  %"main_$I_fetch.219" = load i64, i64* %"main_$I", align 8, !tbaa !159, !llfort.type_idx !39
  %"main_$B.dim_info$488" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$B", i32 0, i32 6, i32 0
  %"main_$B.dim_info$.spacing$489" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$B.dim_info$488", i32 0, i32 1
  %"main_$B.dim_info$.spacing$[]490" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$B.dim_info$.spacing$489", i32 1), !llfort.type_idx !261
  %"main_$B.dim_info$.spacing$[]_fetch.220" = load i64, i64* %"main_$B.dim_info$.spacing$[]490", align 8, !tbaa !114, !range !97, !llfort.type_idx !261
  %"main_$B.dim_info$491" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$B", i32 0, i32 6, i32 0
  %"main_$B.dim_info$.lower_bound$492" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$B.dim_info$491", i32 0, i32 2
  %"main_$B.dim_info$.lower_bound$[]493" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$B.dim_info$.lower_bound$492", i32 1), !llfort.type_idx !262
  %"main_$B.dim_info$.lower_bound$[]_fetch.221" = load i64, i64* %"main_$B.dim_info$.lower_bound$[]493", align 8, !tbaa !108, !llfort.type_idx !262
  %"main_$J_fetch.222" = load i64, i64* %"main_$J", align 8, !tbaa !157, !llfort.type_idx !38
  %"main_$B.dim_info$494" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$B", i32 0, i32 6, i32 0
  %"main_$B.dim_info$.lower_bound$495" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$B.dim_info$494", i32 0, i32 2
  %"main_$B.dim_info$.lower_bound$[]496" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$B.dim_info$.lower_bound$495", i32 0), !llfort.type_idx !263
  %"main_$B.dim_info$.lower_bound$[]_fetch.223" = load i64, i64* %"main_$B.dim_info$.lower_bound$[]496", align 8, !tbaa !108, !llfort.type_idx !263
  %mul.45 = mul nsw i64 %"main_$B.dim_info$.lower_bound$[]_fetch.223", 8
  %"main_$B.dim_info$497" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$B", i32 0, i32 6, i32 0
  %"main_$B.dim_info$.lower_bound$498" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$B.dim_info$497", i32 0, i32 2
  %"main_$B.dim_info$.lower_bound$[]499" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$B.dim_info$.lower_bound$498", i32 1), !llfort.type_idx !264
  %"main_$B.dim_info$.lower_bound$[]_fetch.224" = load i64, i64* %"main_$B.dim_info$.lower_bound$[]499", align 8, !tbaa !108, !llfort.type_idx !264
  %"main_$B.dim_info$500" = getelementptr inbounds %"QNCA_a0$i64*$rank2$", %"QNCA_a0$i64*$rank2$"* %"main_$B", i32 0, i32 6, i32 0
  %"main_$B.dim_info$.spacing$501" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"main_$B.dim_info$500", i32 0, i32 1
  %"main_$B.dim_info$.spacing$[]502" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"main_$B.dim_info$.spacing$501", i32 1), !llfort.type_idx !265
  %"main_$B.dim_info$.spacing$[]_fetch.225" = load i64, i64* %"main_$B.dim_info$.spacing$[]502", align 8, !tbaa !114, !range !97, !llfort.type_idx !265
  %mul.46 = mul nsw i64 %"main_$B.dim_info$.lower_bound$[]_fetch.224", %"main_$B.dim_info$.spacing$[]_fetch.225"
  %add.50 = add nsw i64 %mul.45, %mul.46
  %"main_$B.addr_a0$_fetch.217[]" = call i64* @llvm.intel.subscript.p0i64.i64.i64.p0i64.i64(i8 1, i64 %"main_$B.dim_info$.lower_bound$[]_fetch.221", i64 %"main_$B.dim_info$.spacing$[]_fetch.220", i64* elementtype(i64) %"main_$B.addr_a0$_fetch.217", i64 %"main_$J_fetch.222"), !llfort.type_idx !266
  %"main_$B.addr_a0$_fetch.217[][]" = call i64* @llvm.intel.subscript.p0i64.i64.i64.p0i64.i64(i8 0, i64 %"main_$B.dim_info$.lower_bound$[]_fetch.218", i64 8, i64* elementtype(i64) %"main_$B.addr_a0$_fetch.217[]", i64 %"main_$I_fetch.219"), !llfort.type_idx !267
  %"main_$B.addr_a0$_fetch.217[][]_fetch.226" = load i64, i64* %"main_$B.addr_a0$_fetch.217[][]", align 8, !tbaa !181, !llfort.type_idx !267
  %add.51 = add nsw i64 %"main_$A.addr_a0$_fetch.207[][]_fetch.216", %"main_$B.addr_a0$_fetch.217[][]_fetch.226"
  %rel.53 = icmp ne i64 %"main_$C.addr_a0$_fetch.197[][]_fetch.206", %add.51
  %int_zext503 = zext i1 %rel.53 to i32, !llfort.type_idx !43
  %int_zext504 = trunc i32 %int_zext503 to i1, !llfort.type_idx !196
  br i1 %int_zext504, label %bb_new75_then, label %bb44_else

bb_new75_then:                                    ; preds = %bb39
  %"(i64*)$io_ctx$" = bitcast [8 x i64]* %"$io_ctx" to i64*, !llfort.type_idx !268
  store i64 0, i64* %"(i64*)$io_ctx$", align 16, !tbaa !75
  store [4 x i8] c"8\04\01\00", [4 x i8]* %"(&)val$", align 1, !tbaa !75
  %BLKFIELD_i64_ = getelementptr inbounds <{ i64, i8* }>, <{ i64, i8* }>* %argblock, i32 0, i32 0
  store i64 17, i64* %BLKFIELD_i64_, align 8, !tbaa !269
  %"BLKFIELD_i8*_" = getelementptr inbounds <{ i64, i8* }>, <{ i64, i8* }>* %argblock, i32 0, i32 1
  store i8* getelementptr inbounds ([17 x i8], [17 x i8]* @strlit.129, i32 0, i32 0), i8** %"BLKFIELD_i8*_", align 8, !tbaa !271
  %"(i8*)$io_ctx$" = bitcast [8 x i64]* %"$io_ctx" to i8*, !llfort.type_idx !273
  %"(i8*)(&)val$$" = bitcast [4 x i8]* %"(&)val$" to i8*, !llfort.type_idx !274
  %"(i8*)argblock$" = bitcast <{ i64, i8* }>* %argblock to i8*, !llfort.type_idx !275
  %func_result445 = call i32 (i8*, i32, i64, i8*, i8*, ...) @for_write_seq_lis(i8* %"(i8*)$io_ctx$", i32 -1, i64 1239157112576, i8* %"(i8*)(&)val$$", i8* %"(i8*)argblock$"), !llfort.type_idx !43
  call void @abort_.void(), !llfort.type_idx !276
  br label %bb45_endif

bb44_else:                                        ; preds = %bb39
  br label %bb45_endif

bb45_endif:                                       ; preds = %bb44_else, %bb_new75_then
  %"main_$I_fetch.228" = load i64, i64* %"main_$I", align 8, !tbaa !159, !llfort.type_idx !39
  %add.52 = add nsw i64 %"main_$I_fetch.228", 1
  store i64 %add.52, i64* %"main_$I", align 8, !tbaa !159
  %"main_$I_fetch.229" = load i64, i64* %"main_$I", align 8, !tbaa !159, !llfort.type_idx !39
  %"var$24_fetch.230" = load i64, i64* %"var$24", align 8, !tbaa !75, !llfort.type_idx !42
  %rel.54 = icmp sle i64 %"main_$I_fetch.229", %"var$24_fetch.230"
  br i1 %rel.54, label %bb39, label %bb42

bb42:                                             ; preds = %bb45_endif
  br label %bb40

bb40:                                             ; preds = %bb42, %bb35
  %"main_$J_fetch.231" = load i64, i64* %"main_$J", align 8, !tbaa !157, !llfort.type_idx !38
  %add.53 = add nsw i64 %"main_$J_fetch.231", 1
  store i64 %add.53, i64* %"main_$J", align 8, !tbaa !157
  %"main_$J_fetch.232" = load i64, i64* %"main_$J", align 8, !tbaa !157, !llfort.type_idx !38
  %"var$23_fetch.233" = load i64, i64* %"var$23", align 8, !tbaa !75, !llfort.type_idx !42
  %rel.55 = icmp sle i64 %"main_$J_fetch.232", %"var$23_fetch.233"
  br i1 %rel.55, label %bb35, label %bb38

bb38:                                             ; preds = %bb40
  br label %bb36

bb36:                                             ; preds = %bb38, %bb_new73
  %"main_$N_fetch.234" = load i64, i64* @"main_$N", align 8, !tbaa !73, !llfort.type_idx !28
  %"main_$N_fetch.235" = load i64, i64* @"main_$N", align 8, !tbaa !73, !llfort.type_idx !28
  %mul.47 = mul nsw i64 %"main_$N_fetch.234", %"main_$N_fetch.235"
  %mul.48 = mul nsw i64 %mul.47, 8
  %mul.49 = mul nsw i64 %mul.48, 3
  store i64 %mul.49, i64* %"main_$BYTES", align 8, !tbaa !277
  %"main_$TIME_fetch.236" = load double, double* %"main_$TIME", align 8, !tbaa !198, !llfort.type_idx !36
  %"main_$NO_MAX_REP_fetch.237" = load i32, i32* @"main_$NO_MAX_REP", align 8, !tbaa !190, !llfort.type_idx !27
  %"(double)main_$NO_MAX_REP_fetch.237$" = sitofp i32 %"main_$NO_MAX_REP_fetch.237" to double, !llfort.type_idx !197
  %div.1 = fdiv reassoc ninf nsz arcp contract afn double %"main_$TIME_fetch.236", %"(double)main_$NO_MAX_REP_fetch.237$"
  store double %div.1, double* %"main_$TIME", align 8, !tbaa !198
  %"(i64*)$io_ctx$528" = bitcast [8 x i64]* %"$io_ctx" to i64*, !llfort.type_idx !268
  store i64 0, i64* %"(i64*)$io_ctx$528", align 16, !tbaa !75
  store [4 x i8] c"8\04\02\00", [4 x i8]* %"(&)val$505", align 1, !tbaa !75
  %BLKFIELD_i64_507 = getelementptr inbounds <{ i64, i8* }>, <{ i64, i8* }>* %argblock506, i32 0, i32 0
  store i64 8, i64* %BLKFIELD_i64_507, align 8, !tbaa !279
  %"BLKFIELD_i8*_508" = getelementptr inbounds <{ i64, i8* }>, <{ i64, i8* }>* %argblock506, i32 0, i32 1
  store i8* getelementptr inbounds ([8 x i8], [8 x i8]* @strlit.126, i32 0, i32 0), i8** %"BLKFIELD_i8*_508", align 8, !tbaa !281
  %"(i8*)$io_ctx$510" = bitcast [8 x i64]* %"$io_ctx" to i8*, !llfort.type_idx !283
  %"(i8*)(&)val$505$" = bitcast [4 x i8]* %"(&)val$505" to i8*, !llfort.type_idx !284
  %"(i8*)argblock506$" = bitcast <{ i64, i8* }>* %argblock506 to i8*, !llfort.type_idx !285
  %func_result512 = call i32 (i8*, i32, i64, i8*, i8*, ...) @for_write_seq_lis(i8* %"(i8*)$io_ctx$510", i32 -1, i64 1239157112576, i8* %"(i8*)(&)val$505$", i8* %"(i8*)argblock506$"), !llfort.type_idx !43
  %"main_$BYTES_fetch.239" = load i64, i64* %"main_$BYTES", align 8, !tbaa !277, !llfort.type_idx !37
  store [4 x i8] c"\0B\01\02\00", [4 x i8]* %"(&)val$513", align 1, !tbaa !75
  %BLKFIELD_i64_515 = getelementptr inbounds <{ i64 }>, <{ i64 }>* %argblock514, i32 0, i32 0
  store i64 %"main_$BYTES_fetch.239", i64* %BLKFIELD_i64_515, align 8, !tbaa !286
  %"(i8*)$io_ctx$517" = bitcast [8 x i64]* %"$io_ctx" to i8*, !llfort.type_idx !288
  %"(i8*)(&)val$513$" = bitcast [4 x i8]* %"(&)val$513" to i8*, !llfort.type_idx !289
  %"(i8*)argblock514$" = bitcast <{ i64 }>* %argblock514 to i8*, !llfort.type_idx !290
  %func_result519 = call i32 @for_write_seq_lis_xmit(i8* %"(i8*)$io_ctx$517", i8* %"(i8*)(&)val$513$", i8* %"(i8*)argblock514$"), !llfort.type_idx !43
  store [4 x i8] c"8\04\01\00", [4 x i8]* %"(&)val$520", align 1, !tbaa !75
  %BLKFIELD_i64_522 = getelementptr inbounds <{ i64, i8* }>, <{ i64, i8* }>* %argblock521, i32 0, i32 0
  store i64 2, i64* %BLKFIELD_i64_522, align 8, !tbaa !291
  %"BLKFIELD_i8*_523" = getelementptr inbounds <{ i64, i8* }>, <{ i64, i8* }>* %argblock521, i32 0, i32 1
  store i8* getelementptr inbounds ([2 x i8], [2 x i8]* @strlit.125, i32 0, i32 0), i8** %"BLKFIELD_i8*_523", align 8, !tbaa !293
  %"(i8*)$io_ctx$525" = bitcast [8 x i64]* %"$io_ctx" to i8*, !llfort.type_idx !295
  %"(i8*)(&)val$520$" = bitcast [4 x i8]* %"(&)val$520" to i8*, !llfort.type_idx !296
  %"(i8*)argblock521$" = bitcast <{ i64, i8* }>* %argblock521 to i8*, !llfort.type_idx !297
  %func_result527 = call i32 @for_write_seq_lis_xmit(i8* %"(i8*)$io_ctx$525", i8* %"(i8*)(&)val$520$", i8* %"(i8*)argblock521$"), !llfort.type_idx !43
  %"(i64*)$io_ctx$543" = bitcast [8 x i64]* %"$io_ctx" to i64*, !llfort.type_idx !268
  store i64 0, i64* %"(i64*)$io_ctx$543", align 16, !tbaa !75
  store [4 x i8] c"8\04\02\00", [4 x i8]* %"(&)val$529", align 1, !tbaa !75
  %BLKFIELD_i64_531 = getelementptr inbounds <{ i64, i8* }>, <{ i64, i8* }>* %argblock530, i32 0, i32 0
  store i64 7, i64* %BLKFIELD_i64_531, align 8, !tbaa !298
  %"BLKFIELD_i8*_532" = getelementptr inbounds <{ i64, i8* }>, <{ i64, i8* }>* %argblock530, i32 0, i32 1
  store i8* getelementptr inbounds ([7 x i8], [7 x i8]* @strlit.124, i32 0, i32 0), i8** %"BLKFIELD_i8*_532", align 8, !tbaa !300
  %"(i8*)$io_ctx$534" = bitcast [8 x i64]* %"$io_ctx" to i8*, !llfort.type_idx !302
  %"(i8*)(&)val$529$" = bitcast [4 x i8]* %"(&)val$529" to i8*, !llfort.type_idx !303
  %"(i8*)argblock530$" = bitcast <{ i64, i8* }>* %argblock530 to i8*, !llfort.type_idx !304
  %func_result536 = call i32 (i8*, i32, i64, i8*, i8*, ...) @for_write_seq_lis(i8* %"(i8*)$io_ctx$534", i32 -1, i64 1239157112576, i8* %"(i8*)(&)val$529$", i8* %"(i8*)argblock530$"), !llfort.type_idx !43
  %"main_$TIME_fetch.242" = load double, double* %"main_$TIME", align 8, !tbaa !198, !llfort.type_idx !36
  store [4 x i8] c"0\01\01\00", [4 x i8]* %"(&)val$537", align 1, !tbaa !75
  %BLKFIELD_double_ = getelementptr inbounds <{ double }>, <{ double }>* %argblock538, i32 0, i32 0
  store double %"main_$TIME_fetch.242", double* %BLKFIELD_double_, align 8, !tbaa !305
  %"(i8*)$io_ctx$540" = bitcast [8 x i64]* %"$io_ctx" to i8*, !llfort.type_idx !307
  %"(i8*)(&)val$537$" = bitcast [4 x i8]* %"(&)val$537" to i8*, !llfort.type_idx !308
  %"(i8*)argblock538$" = bitcast <{ double }>* %argblock538 to i8*, !llfort.type_idx !309
  %func_result542 = call i32 @for_write_seq_lis_xmit(i8* %"(i8*)$io_ctx$540", i8* %"(i8*)(&)val$537$", i8* %"(i8*)argblock538$"), !llfort.type_idx !43
  %"(i64*)$io_ctx$559" = bitcast [8 x i64]* %"$io_ctx" to i64*, !llfort.type_idx !268
  store i64 0, i64* %"(i64*)$io_ctx$559", align 16, !tbaa !75
  store [4 x i8] c"8\04\02\00", [4 x i8]* %"(&)val$544", align 1, !tbaa !75
  %BLKFIELD_i64_546 = getelementptr inbounds <{ i64, i8* }>, <{ i64, i8* }>* %argblock545, i32 0, i32 0
  store i64 12, i64* %BLKFIELD_i64_546, align 8, !tbaa !310
  %"BLKFIELD_i8*_547" = getelementptr inbounds <{ i64, i8* }>, <{ i64, i8* }>* %argblock545, i32 0, i32 1
  store i8* getelementptr inbounds ([12 x i8], [12 x i8]* @strlit.123, i32 0, i32 0), i8** %"BLKFIELD_i8*_547", align 8, !tbaa !312
  %"(i8*)$io_ctx$549" = bitcast [8 x i64]* %"$io_ctx" to i8*, !llfort.type_idx !314
  %"(i8*)(&)val$544$" = bitcast [4 x i8]* %"(&)val$544" to i8*, !llfort.type_idx !315
  %"(i8*)argblock545$" = bitcast <{ i64, i8* }>* %argblock545 to i8*, !llfort.type_idx !316
  %func_result551 = call i32 (i8*, i32, i64, i8*, i8*, ...) @for_write_seq_lis(i8* %"(i8*)$io_ctx$549", i32 -1, i64 1239157112576, i8* %"(i8*)(&)val$544$", i8* %"(i8*)argblock545$"), !llfort.type_idx !43
  %"main_$BYTES_fetch.244" = load i64, i64* %"main_$BYTES", align 8, !tbaa !277, !llfort.type_idx !37
  %"(double)main_$BYTES_fetch.244$" = sitofp i64 %"main_$BYTES_fetch.244" to double, !llfort.type_idx !197
  %"main_$TIME_fetch.245" = load double, double* %"main_$TIME", align 8, !tbaa !198, !llfort.type_idx !36
  %div.2 = fdiv reassoc ninf nsz arcp contract afn double %"(double)main_$BYTES_fetch.244$", %"main_$TIME_fetch.245"
  %mul.50 = fmul reassoc ninf nsz arcp contract afn double %div.2, 0x3E112E0BE0000000
  store [4 x i8] c"0\01\01\00", [4 x i8]* %"(&)val$552", align 1, !tbaa !75
  %BLKFIELD_double_554 = getelementptr inbounds <{ double }>, <{ double }>* %argblock553, i32 0, i32 0
  store double %mul.50, double* %BLKFIELD_double_554, align 8, !tbaa !317
  %"(i8*)$io_ctx$556" = bitcast [8 x i64]* %"$io_ctx" to i8*, !llfort.type_idx !319
  %"(i8*)(&)val$552$" = bitcast [4 x i8]* %"(&)val$552" to i8*, !llfort.type_idx !320
  %"(i8*)argblock553$" = bitcast <{ double }>* %argblock553 to i8*, !llfort.type_idx !321
  %func_result558 = call i32 @for_write_seq_lis_xmit(i8* %"(i8*)$io_ctx$556", i8* %"(i8*)(&)val$552$", i8* %"(i8*)argblock553$"), !llfort.type_idx !43
  ret void
}

declare !llfort.intrin_id !322 !llfort.type_idx !323 i32 @for_set_fpe_(i32* nocapture readonly)

declare !llfort.intrin_id !324 !llfort.type_idx !325 i32 @for_set_reentrancy(i32* nocapture readonly)

declare !llfort.intrin_id !326 !llfort.type_idx !327 i32 @for_check_mult_overflow64(i64* nocapture, i32, ...)

declare !llfort.intrin_id !328 !llfort.type_idx !329 i32 @for_alloc_allocatable_handle(i64, i8** nocapture, i32, i8*)

; Function Attrs: nounwind readnone speculatable
declare i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8, i64, i32, i64*, i32) #1

; Function Attrs: nounwind readnone speculatable
declare i64* @llvm.intel.subscript.p0i64.i64.i64.p0i64.i64(i8, i64, i64, i64*, i64) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

declare !llfort.type_idx !330 double @omp_get_wtime()

declare !llfort.intrin_id !331 !llfort.type_idx !332 i32 @for_write_seq_lis(i8*, i32, i64, i8*, i8*, ...)

; Function Attrs: nounwind uwtable
define internal void @abort_.void() #3 !llfort.type_idx !333 {
wrap_start103:
  call void (...) @abort_()
  ret void
}

declare !llfort.intrin_id !334 !llfort.type_idx !335 i32 @for_write_seq_lis_xmit(i8* nocapture readonly, i8* nocapture readonly, i8*)

declare !llfort.type_idx !336 void @abort_(...)

attributes #0 = { nounwind uwtable "denormal-fp-math"="preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { nounwind readnone speculatable }
attributes #2 = { nounwind }
attributes #3 = { nounwind uwtable "denormal-fp-math"="preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }

!llvm.module.flags = !{!30}
!omp_offload.info = !{!31}

!0 = !{i64 74}
!1 = !{i64 75}
!2 = !{i64 76}
!3 = !{i64 77}
!4 = !{i64 78}
!5 = !{i64 79}
!6 = !{i64 80}
!7 = !{i64 81}
!8 = !{i64 82}
!9 = !{i64 83}
!10 = !{i64 84}
!11 = !{i64 85}
!12 = !{i64 86}
!13 = !{i64 87}
!14 = !{i64 88}
!15 = !{i64 89}
!16 = !{i64 90}
!17 = !{i64 91}
!18 = !{i64 92}
!19 = !{i64 93}
!20 = !{i64 94}
!21 = !{i64 95}
!22 = !{i64 96}
!23 = !{i64 97}
!24 = !{i64 98}
!25 = !{i64 99}
!26 = !{i64 107}
!27 = !{i64 108}
!28 = !{i64 110}
!29 = !{i64 115}
!30 = !{i32 7, !"openmp", i32 50}
!31 = !{i32 0, i32 66313, i32 150669098, !"MAIN__", i32 33, i32 0, i32 0}
!32 = !{i64 73}
!33 = !{i64 69}
!34 = !{i64 101}
!35 = !{i64 102}
!36 = !{i64 103}
!37 = !{i64 104}
!38 = !{i64 105}
!39 = !{i64 106}
!40 = !{i64 109}
!41 = !{i64 31}
!42 = !{i64 3}
!43 = !{i64 2}
!44 = !{i64 300}
!45 = !{i64 302}
!46 = !{i64 313}
!47 = !{i64 322}
!48 = !{i64 324}
!49 = !{i64 332}
!50 = !{i64 342}
!51 = !{i64 351}
!52 = !{i64 352}
!53 = !{i64 361}
!54 = !{i64 370}
!55 = !{!56, !56, i64 0}
!56 = !{!"ifx$unique_sym$12", !57, i64 0}
!57 = !{!"Fortran Data Symbol", !58, i64 0}
!58 = !{!"Generic Fortran Symbol", !59, i64 0}
!59 = !{!"ifx$root$1$MAIN__"}
!60 = !{!61, !61, i64 0}
!61 = !{!"ifx$unique_sym$16", !57, i64 0}
!62 = !{!63, !63, i64 0}
!63 = !{!"ifx$unique_sym$20", !57, i64 0}
!64 = !{!65, !65, i64 0}
!65 = !{!"ifx$unique_sym$23", !57, i64 0}
!66 = !{!67, !67, i64 0}
!67 = !{!"ifx$unique_sym$27", !57, i64 0}
!68 = !{!69, !69, i64 0}
!69 = !{!"Fortran Dope Vector Symbol", !58, i64 0}
!70 = !{!71, !72, i64 40}
!71 = !{!"ifx$descr$1", !72, i64 0, !72, i64 8, !72, i64 16, !72, i64 24, !72, i64 32, !72, i64 40, !72, i64 48, !72, i64 56, !72, i64 64, !72, i64 72, !72, i64 80, !72, i64 88}
!72 = !{!"ifx$descr$field", !69, i64 0}
!73 = !{!74, !74, i64 0}
!74 = !{!"ifx$unique_sym$1", !57, i64 0}
!75 = !{!57, !57, i64 0}
!76 = !{!71, !72, i64 24}
!77 = !{i64 35}
!78 = !{i64 139}
!79 = !{i64 37}
!80 = !{i64 11}
!81 = !{i64 143}
!82 = !{!71, !72, i64 8}
!83 = !{!71, !72, i64 32}
!84 = !{!71, !72, i64 16}
!85 = !{i64 145}
!86 = !{!71, !72, i64 64}
!87 = !{i64 146}
!88 = !{!71, !72, i64 48}
!89 = !{i64 147}
!90 = !{i64 148}
!91 = !{i64 149}
!92 = !{!71, !72, i64 56}
!93 = !{i64 150}
!94 = !{i64 153}
!95 = !{i64 154}
!96 = !{i64 155}
!97 = !{i64 1, i64 -9223372036854775808}
!98 = !{!99, !99, i64 0}
!99 = !{!"ifx$unique_sym$2", !57, i64 0}
!100 = !{!101, !72, i64 40}
!101 = !{!"ifx$descr$2", !72, i64 0, !72, i64 8, !72, i64 16, !72, i64 24, !72, i64 32, !72, i64 40, !72, i64 48, !72, i64 56, !72, i64 64, !72, i64 72, !72, i64 80, !72, i64 88}
!102 = !{!101, !72, i64 24}
!103 = !{i64 161}
!104 = !{!101, !72, i64 8}
!105 = !{!101, !72, i64 32}
!106 = !{!101, !72, i64 16}
!107 = !{i64 163}
!108 = !{!101, !72, i64 64}
!109 = !{i64 164}
!110 = !{!101, !72, i64 48}
!111 = !{i64 165}
!112 = !{i64 166}
!113 = !{i64 167}
!114 = !{!101, !72, i64 56}
!115 = !{i64 168}
!116 = !{i64 171}
!117 = !{i64 172}
!118 = !{i64 173}
!119 = !{!120, !72, i64 40}
!120 = !{!"ifx$descr$3", !72, i64 0, !72, i64 8, !72, i64 16, !72, i64 24, !72, i64 32, !72, i64 40, !72, i64 48, !72, i64 56, !72, i64 64, !72, i64 72, !72, i64 80, !72, i64 88}
!121 = !{!120, !72, i64 24}
!122 = !{i64 179}
!123 = !{!120, !72, i64 8}
!124 = !{!120, !72, i64 32}
!125 = !{!120, !72, i64 16}
!126 = !{i64 181}
!127 = !{!120, !72, i64 64}
!128 = !{i64 182}
!129 = !{!120, !72, i64 48}
!130 = !{i64 183}
!131 = !{i64 184}
!132 = !{i64 185}
!133 = !{!120, !72, i64 56}
!134 = !{i64 186}
!135 = !{i64 189}
!136 = !{i64 190}
!137 = !{i64 191}
!138 = !{!120, !72, i64 0}
!139 = !{i64 192}
!140 = !{i64 193}
!141 = !{i64 194}
!142 = !{i64 195}
!143 = !{i64 196}
!144 = !{i64 197}
!145 = !{i64 198}
!146 = !{i64 199}
!147 = !{i64 200}
!148 = !{i64 201}
!149 = !{i64 202}
!150 = !{i64 203}
!151 = !{i64 204}
!152 = !{i64 205}
!153 = !{i64 206}
!154 = !{i64 207}
!155 = !{!156, !156, i64 0}
!156 = !{!"ifx$unique_sym$3", !57, i64 0}
!157 = !{!158, !158, i64 0}
!158 = !{!"ifx$unique_sym$4", !57, i64 0}
!159 = !{!160, !160, i64 0}
!160 = !{!"ifx$unique_sym$5", !57, i64 0}
!161 = !{!71, !72, i64 0}
!162 = !{i64 208}
!163 = !{i64 209}
!164 = !{i64 210}
!165 = !{i64 211}
!166 = !{i64 212}
!167 = !{i64 213}
!168 = !{i64 214}
!169 = !{i64 215}
!170 = !{!171, !171, i64 0}
!171 = !{!"ifx$unique_sym$6", !57, i64 0}
!172 = !{!101, !72, i64 0}
!173 = !{i64 216}
!174 = !{i64 217}
!175 = !{i64 218}
!176 = !{i64 219}
!177 = !{i64 220}
!178 = !{i64 221}
!179 = !{i64 222}
!180 = !{i64 223}
!181 = !{!182, !182, i64 0}
!182 = !{!"ifx$unique_sym$7", !57, i64 0}
!183 = !{i64 33}
!184 = !{i64 224}
!185 = !{i64 225}
!186 = !{i64 226}
!187 = !{i64 227}
!188 = !{i64 228}
!189 = !{i64 229}
!190 = !{!191, !191, i64 0}
!191 = !{!"ifx$unique_sym$8", !57, i64 0}
!192 = !{!193, !193, i64 0}
!193 = !{!"ifx$unique_sym$9", !57, i64 0}
!194 = !{!195, !195, i64 0}
!195 = !{!"ifx$unique_sym$10", !57, i64 0}
!196 = !{i64 136}
!197 = !{i64 6}
!198 = !{!199, !199, i64 0}
!199 = !{!"ifx$unique_sym$11", !57, i64 0}
!200 = !{i64 230}
!201 = !{i64 231}
!202 = !{i64 232}
!203 = !{i64 233}
!204 = !{i64 234}
!205 = !{i64 235}
!206 = !{i64 236}
!207 = !{i64 237}
!208 = !{i64 238}
!209 = !{i64 239}
!210 = !{i64 240}
!211 = !{i64 241}
!212 = !{i64 242}
!213 = !{i64 243}
!214 = !{i64 244}
!215 = !{i64 245}
!216 = !{i64 248}
!217 = !{i64 249}
!218 = !{i64 250}
!219 = !{i64 251}
!220 = !{i64 252}
!221 = !{i64 253}
!222 = !{i64 254}
!223 = !{i64 255}
!224 = !{i64 256}
!225 = !{i64 257}
!226 = !{i64 260}
!227 = !{i64 261}
!228 = !{i64 262}
!229 = !{i64 263}
!230 = !{i64 264}
!231 = !{i64 265}
!232 = !{i64 266}
!233 = !{i64 267}
!234 = !{i64 268}
!235 = !{i64 269}
!236 = !{i64 258}
!237 = !{i64 259}
!238 = !{i64 270}
!239 = !{i64 271}
!240 = !{i64 246}
!241 = !{i64 247}
!242 = !{i64 272}
!243 = !{i64 273}
!244 = !{i64 274}
!245 = !{i64 275}
!246 = !{i64 276}
!247 = !{i64 277}
!248 = !{i64 278}
!249 = !{i64 279}
!250 = !{i64 280}
!251 = !{i64 281}
!252 = !{i64 282}
!253 = !{i64 283}
!254 = !{i64 284}
!255 = !{i64 285}
!256 = !{i64 286}
!257 = !{i64 287}
!258 = !{i64 288}
!259 = !{i64 289}
!260 = !{i64 290}
!261 = !{i64 291}
!262 = !{i64 292}
!263 = !{i64 293}
!264 = !{i64 294}
!265 = !{i64 295}
!266 = !{i64 296}
!267 = !{i64 297}
!268 = !{i64 22}
!269 = !{!270, !270, i64 0}
!270 = !{!"ifx$unique_sym$13", !57, i64 0}
!271 = !{!272, !272, i64 0}
!272 = !{!"ifx$unique_sym$14", !57, i64 0}
!273 = !{i64 305}
!274 = !{i64 307}
!275 = !{i64 309}
!276 = !{i64 67}
!277 = !{!278, !278, i64 0}
!278 = !{!"ifx$unique_sym$15", !57, i64 0}
!279 = !{!280, !280, i64 0}
!280 = !{!"ifx$unique_sym$17", !57, i64 0}
!281 = !{!282, !282, i64 0}
!282 = !{!"ifx$unique_sym$18", !57, i64 0}
!283 = !{i64 316}
!284 = !{i64 318}
!285 = !{i64 320}
!286 = !{!287, !287, i64 0}
!287 = !{!"ifx$unique_sym$19", !57, i64 0}
!288 = !{i64 326}
!289 = !{i64 328}
!290 = !{i64 330}
!291 = !{!292, !292, i64 0}
!292 = !{!"ifx$unique_sym$21", !57, i64 0}
!293 = !{!294, !294, i64 0}
!294 = !{!"ifx$unique_sym$22", !57, i64 0}
!295 = !{i64 335}
!296 = !{i64 337}
!297 = !{i64 339}
!298 = !{!299, !299, i64 0}
!299 = !{!"ifx$unique_sym$24", !57, i64 0}
!300 = !{!301, !301, i64 0}
!301 = !{!"ifx$unique_sym$25", !57, i64 0}
!302 = !{i64 345}
!303 = !{i64 347}
!304 = !{i64 349}
!305 = !{!306, !306, i64 0}
!306 = !{!"ifx$unique_sym$26", !57, i64 0}
!307 = !{i64 354}
!308 = !{i64 356}
!309 = !{i64 358}
!310 = !{!311, !311, i64 0}
!311 = !{!"ifx$unique_sym$28", !57, i64 0}
!312 = !{!313, !313, i64 0}
!313 = !{!"ifx$unique_sym$29", !57, i64 0}
!314 = !{i64 364}
!315 = !{i64 366}
!316 = !{i64 368}
!317 = !{!318, !318, i64 0}
!318 = !{!"ifx$unique_sym$30", !57, i64 0}
!319 = !{i64 372}
!320 = !{i64 374}
!321 = !{i64 376}
!322 = !{i32 100}
!323 = !{i64 132}
!324 = !{i32 101}
!325 = !{i64 134}
!326 = !{i32 105}
!327 = !{i64 187}
!328 = !{i32 96}
!329 = !{i64 176}
!330 = !{i64 100}
!331 = !{i32 335}
!332 = !{i64 301}
!333 = !{i64 311}
!334 = !{i32 337}
!335 = !{i64 323}
!336 = !{i64 378}
