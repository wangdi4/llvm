; INTEL_CUSTOMIZATION
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -simplifycfg -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt,function(simplifycfg)' -S %s | FileCheck %s
;
; Regression test for CMPLRLLVM-25276. Check that paropt transform pass adds
; correct range metadata to omp_get_num_threads() call.
;
;      PROGRAM test_target_teams_distribute_parallel_for
;
;        USE omp_lib
;        implicit none
;
;        call target_teams_distribute_parallel_for()
;
;        CONTAINS
;          subroutine target_teams_distribute_parallel_for()
;            INTEGER :: i
;            INTEGER, DIMENSION(1024) :: num_threads
;
;!$omp teams distribute parallel do num_threads(8)
;            DO i = 1, 1024, 1
;              num_threads(i) = omp_get_num_threads()
;            END DO
;
;          END subroutine target_teams_distribute_parallel_for
;
;      END PROGRAM test_target_teams_distribute_parallel_for
;
; CHECK: call i32 @omp_get_num_threads(), !range [[RANGE:![0-9]+]]
; CHECK: [[RANGE]] = !{i32 1, i32 9}

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

declare i32 @omp_get_num_threads()

define void @test_target_teams_distribute_parallel_for_IP_target_teams_distribute_parallel_for_() {
alloca_1:
  %"var$2" = alloca [8 x i64], align 16
  %"target_teams_distribute_parallel_for$I$_228" = alloca i32, align 8
  %"target_teams_distribute_parallel_for$NUM_THREADS$_228" = alloca [1024 x i32], align 16
  br label %bb10

bb15:                                             ; preds = %bb13
  store i32 0, ptr %temp38, align 1
  %temp_fetch = load i32, ptr %temp, align 1
  %temp_fetch2 = load i32, ptr %temp36, align 1
  %temp_fetch4 = load i32, ptr %temp38, align 1
  %mul = mul nsw i32 %temp_fetch4, %temp_fetch2
  %add = add nsw i32 %mul, %temp_fetch
  store i32 %add, ptr %"target_teams_distribute_parallel_for$I$_228", align 1
  br label %bb19

bb19:                                             ; preds = %bb19, %bb15
  %temp_fetch6 = load i32, ptr %temp, align 1
  %temp_fetch8 = load i32, ptr %temp36, align 1
  %temp_fetch10 = load i32, ptr %temp38, align 1
  %mul12 = mul nsw i32 %temp_fetch10, %temp_fetch8
  %add14 = add nsw i32 %mul12, %temp_fetch6
  store i32 %add14, ptr %"target_teams_distribute_parallel_for$I$_228", align 1
  %func_result = call i32 @omp_get_num_threads()
  %"target_teams_distribute_parallel_for$I$_228_fetch" = load i32, ptr %"target_teams_distribute_parallel_for$I$_228", align 1
  %int_sext = sext i32 %"target_teams_distribute_parallel_for$I$_228_fetch" to i64
  %"target_teams_distribute_parallel_for$NUM_THREADS$_228[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %"target_teams_distribute_parallel_for$NUM_THREADS$_228", i64 %int_sext)
  store i32 %func_result, ptr %"target_teams_distribute_parallel_for$NUM_THREADS$_228[]", align 1
  %temp_fetch18 = load i32, ptr %temp36, align 1
  %temp_fetch20 = load i32, ptr %temp38, align 1
  %add22 = add nsw i32 %temp_fetch20, 1
  store i32 %add22, ptr %temp38, align 1
  %temp_fetch24 = load i32, ptr %temp39, align 1
  %temp_fetch26 = load i32, ptr %temp38, align 1
  %rel = icmp sle i32 %temp_fetch26, %temp_fetch24
  br i1 %rel, label %bb19, label %bb16

bb16:                                             ; preds = %bb19
  br label %bb17

bb13:                                             ; preds = %bb10
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(),
    "QUAL.OMP.NUM_THREADS"(i64 8),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %"target_teams_distribute_parallel_for$I$_228", i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %"target_teams_distribute_parallel_for$NUM_THREADS$_228", i32 0, i32 1024),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %temp38, i32 0),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %temp39, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %temp37, i32 0, i32 1) ]

  %temp_fetch31 = load i32, ptr %temp37, align 1
  store i32 %temp_fetch31, ptr %temp38, align 1
  %temp_fetch32 = load i32, ptr %temp38, align 1
  %temp_fetch33 = load i32, ptr %temp39, align 1
  %rel34 = icmp slt i32 %temp_fetch33, %temp_fetch32
  br i1 %rel34, label %bb17, label %bb15

bb17:                                             ; preds = %bb16, %bb13
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TEAMS"() ]
  ret void

bb10:                                             ; preds = %alloca_1
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"() ]
  %temp = alloca i32, align 1
  %temp35 = alloca i32, align 1
  %temp36 = alloca i32, align 1
  %temp37 = alloca i32, align 1
  %temp38 = alloca i32, align 1
  %temp39 = alloca i32, align 1
  store i32 1, ptr %temp, align 1
  store i32 1024, ptr %temp35, align 1
  store i32 1, ptr %temp36, align 1
  %temp_fetch27 = load i32, ptr %temp, align 1
  store i32 %temp_fetch27, ptr %"target_teams_distribute_parallel_for$I$_228", align 1
  store i32 0, ptr %temp37, align 1
  store i32 0, ptr %temp38, align 1
  %temp_fetch28 = load i32, ptr %temp36, align 1
  %temp_fetch29 = load i32, ptr %temp, align 1
  %temp_fetch30 = load i32, ptr %temp35, align 1
  %sub = sub nsw i32 %temp_fetch30, %temp_fetch29
  %div = sdiv i32 %sub, %temp_fetch28
  store i32 %div, ptr %temp39, align 1
  br label %bb13
}

declare i32 @for_set_reentrancy(ptr)
declare token @llvm.directive.region.entry()
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64)
declare void @llvm.directive.region.exit(token)
; end INTEL_CUSTOMIZATION
