; INTEL_CUSTOMIZATION
;RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -debug -S %s 2>&1 | FileCheck %s
;RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -debug -S %s 2>&1 | FileCheck %s
;
;Test SRC:
;    subroutine test(b)
;        integer, dimension(:) :: b
;        !$omp PAR FOR SMD reduction(+:b)
;        do i=1, 2
;        b = b + 1
;        end do
;    end subroutine test

;check the debug messages for finding the dope vector and for setting a VLA insertion point.
;CHECK: checkIfVLA: '  %{{[^,]+}} = load ptr, ptr %{{[^,]+}}, align 1' is a VLA clause operand.
;CHECK: setInsertionPtForVlaAllocas: Found a VLA operand. Setting VLA insertion point to
;
;check in the IR that the allocas and the stacksave call are inserted before the region entry and that the stackrestore is inserted after the region exit
;CHECK:   %"test_$B.1.red" = alloca %{{[^,]+}}, align 8
;CHECK:   %"test_$B.1.red.data" = alloca i32, i64 %{{[^,]+}}, align 4
;CHECK:  [[SS:%[^ ]+]] = call ptr @llvm.stacksave.p0()
;CHECK:  %{{[^,]+}}  = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"()
;CHECK:  call void @llvm.directive.region.exit(token %{{[^,]+}}) [ "DIR.OMP.END.SIMD"() ]
;CHECK:  %"test_$B.1.fast_red.data" = alloca i32, i64 %{{[^,]+}}, align 8
;CHECK:  call void @llvm.stackrestore.p0(ptr [[SS]])

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"QNCA_a0$i32*$rank1$" = type { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }

define void @test_(ptr dereferenceable(72) "assumed_shape" "ptrnoalias" %"test_$B$argptr") {
alloca_0:
  %"$io_ctx" = alloca [8 x i64], align 8
  %"test_$B$locptr" = alloca ptr, align 8
  %"test_$I" = alloca i32, align 8
  %"$loop_ctr" = alloca i64, align 8
  store ptr %"test_$B$argptr", ptr %"test_$B$locptr", align 1
  %"test_$B.1" = load ptr, ptr %"test_$B$locptr", align 1
  %omp.pdo.start = alloca i32, align 4
  store i32 1, ptr %omp.pdo.start, align 1
  %omp.pdo.end = alloca i32, align 4
  store i32 2, ptr %omp.pdo.end, align 1
  %omp.pdo.step = alloca i32, align 4
  store i32 1, ptr %omp.pdo.step, align 1
  %omp.pdo.norm.iv = alloca i64, align 8
  %omp.pdo.norm.lb = alloca i64, align 8
  store i64 0, ptr %omp.pdo.norm.lb, align 1
  %omp.pdo.norm.ub = alloca i64, align 8
  %omp.pdo.end_fetch.2 = load i32, ptr %omp.pdo.end, align 1
  %omp.pdo.start_fetch.3 = load i32, ptr %omp.pdo.start, align 1
  %sub.1 = sub nsw i32 %omp.pdo.end_fetch.2, %omp.pdo.start_fetch.3
  %omp.pdo.step_fetch.4 = load i32, ptr %omp.pdo.step, align 1
  %div.1 = sdiv i32 %sub.1, %omp.pdo.step_fetch.4
  %int_sext15 = sext i32 %div.1 to i64
  store i64 %int_sext15, ptr %omp.pdo.norm.ub, align 1
  br label %bb_new6

omp.pdo.cond3:                                    ; preds = %bb_new7, %loop_exit13
  %omp.pdo.norm.iv_fetch.6 = load i64, ptr %omp.pdo.norm.iv, align 1
  %omp.pdo.norm.ub_fetch.7 = load i64, ptr %omp.pdo.norm.ub, align 1
  %rel.1 = icmp sle i64 %omp.pdo.norm.iv_fetch.6, %omp.pdo.norm.ub_fetch.7
  br i1 %rel.1, label %omp.pdo.body4, label %omp.pdo.epilog5

omp.pdo.body4:                                    ; preds = %omp.pdo.cond3
  %omp.pdo.norm.iv_fetch.8 = load i64, ptr %omp.pdo.norm.iv, align 1
  %int_sext = trunc i64 %omp.pdo.norm.iv_fetch.8 to i32
  %omp.pdo.step_fetch.9 = load i32, ptr %omp.pdo.step, align 1
  %mul.1 = mul nsw i32 %int_sext, %omp.pdo.step_fetch.9
  %omp.pdo.start_fetch.10 = load i32, ptr %omp.pdo.start, align 1
  %add.1 = add nsw i32 %mul.1, %omp.pdo.start_fetch.10
  store i32 %add.1, ptr %"test_$I", align 1
  %"test_$B.1.addr_a0$" = getelementptr inbounds %"QNCA_a0$i32*$rank1$", ptr %"test_$B.1", i32 0, i32 0
  %"test_$B.1.addr_a0$_fetch.11" = load ptr, ptr %"test_$B.1.addr_a0$", align 1
  %"test_$B.1.dim_info$" = getelementptr inbounds %"QNCA_a0$i32*$rank1$", ptr %"test_$B.1", i32 0, i32 6, i32 0
  %"test_$B.1.dim_info$.spacing$" = getelementptr inbounds { i64, i64, i64 }, ptr %"test_$B.1.dim_info$", i32 0, i32 1
  %"test_$B.1.dim_info$.spacing$[]" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) %"test_$B.1.dim_info$.spacing$", i32 0)
  %"test_$B.1.dim_info$.spacing$[]_fetch.12" = load i64, ptr %"test_$B.1.dim_info$.spacing$[]", align 1
  %"test_$B.1.dim_info$1" = getelementptr inbounds %"QNCA_a0$i32*$rank1$", ptr %"test_$B.1", i32 0, i32 6, i32 0
  %"test_$B.1.dim_info$.extent$" = getelementptr inbounds { i64, i64, i64 }, ptr %"test_$B.1.dim_info$1", i32 0, i32 0
  %"test_$B.1.dim_info$.extent$[]" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) %"test_$B.1.dim_info$.extent$", i32 0)
  %"test_$B.1.dim_info$.extent$[]_fetch.13" = load i64, ptr %"test_$B.1.dim_info$.extent$[]", align 1
  %rel.2 = icmp sgt i64 0, %"test_$B.1.dim_info$.extent$[]_fetch.13"
  %slct.1 = select i1 %rel.2, i64 0, i64 %"test_$B.1.dim_info$.extent$[]_fetch.13"
  %"test_$B.1.dim_info$2" = getelementptr inbounds %"QNCA_a0$i32*$rank1$", ptr %"test_$B.1", i32 0, i32 6, i32 0
  %"test_$B.1.dim_info$.spacing$3" = getelementptr inbounds { i64, i64, i64 }, ptr %"test_$B.1.dim_info$2", i32 0, i32 1
  %"test_$B.1.dim_info$.spacing$[]4" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) %"test_$B.1.dim_info$.spacing$3", i32 0)
  %"test_$B.1.dim_info$.spacing$[]_fetch.14" = load i64, ptr %"test_$B.1.dim_info$.spacing$[]4", align 1
  %"test_$B.1.addr_a0$5" = getelementptr inbounds %"QNCA_a0$i32*$rank1$", ptr %"test_$B.1", i32 0, i32 0
  %"test_$B.1.addr_a0$_fetch.16" = load ptr, ptr %"test_$B.1.addr_a0$5", align 1
  %"test_$B.1.dim_info$6" = getelementptr inbounds %"QNCA_a0$i32*$rank1$", ptr %"test_$B.1", i32 0, i32 6, i32 0
  %"test_$B.1.dim_info$.spacing$7" = getelementptr inbounds { i64, i64, i64 }, ptr %"test_$B.1.dim_info$6", i32 0, i32 1
  %"test_$B.1.dim_info$.spacing$[]8" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) %"test_$B.1.dim_info$.spacing$7", i32 0)
  %"test_$B.1.dim_info$.spacing$[]_fetch.17" = load i64, ptr %"test_$B.1.dim_info$.spacing$[]8", align 1
  %"test_$B.1.dim_info$9" = getelementptr inbounds %"QNCA_a0$i32*$rank1$", ptr %"test_$B.1", i32 0, i32 6, i32 0
  %"test_$B.1.dim_info$.extent$10" = getelementptr inbounds { i64, i64, i64 }, ptr %"test_$B.1.dim_info$9", i32 0, i32 0
  %"test_$B.1.dim_info$.extent$[]11" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) %"test_$B.1.dim_info$.extent$10", i32 0)
  %"test_$B.1.dim_info$.extent$[]_fetch.18" = load i64, ptr %"test_$B.1.dim_info$.extent$[]11", align 1
  %rel.3 = icmp sgt i64 0, %"test_$B.1.dim_info$.extent$[]_fetch.18"
  %slct.2 = select i1 %rel.3, i64 0, i64 %"test_$B.1.dim_info$.extent$[]_fetch.18"
  %"test_$B.1.dim_info$12" = getelementptr inbounds %"QNCA_a0$i32*$rank1$", ptr %"test_$B.1", i32 0, i32 6, i32 0
  %"test_$B.1.dim_info$.spacing$13" = getelementptr inbounds { i64, i64, i64 }, ptr %"test_$B.1.dim_info$12", i32 0, i32 1
  %"test_$B.1.dim_info$.spacing$[]14" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) %"test_$B.1.dim_info$.spacing$13", i32 0)
  %"test_$B.1.dim_info$.spacing$[]_fetch.19" = load i64, ptr %"test_$B.1.dim_info$.spacing$[]14", align 1
  store i64 1, ptr %"$loop_ctr", align 1
  br label %loop_test11

loop_test11:                                      ; preds = %loop_body12, %omp.pdo.body4
  %"$loop_ctr_fetch.22" = load i64, ptr %"$loop_ctr", align 1
  %rel.4 = icmp sle i64 %"$loop_ctr_fetch.22", %"test_$B.1.dim_info$.extent$[]_fetch.13"
  br i1 %rel.4, label %loop_body12, label %loop_exit13

loop_body12:                                      ; preds = %loop_test11
  %"$loop_ctr_fetch.20" = load i64, ptr %"$loop_ctr", align 1
  %"test_$B.1.addr_a0$_fetch.16[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %"test_$B.1.dim_info$.spacing$[]_fetch.17", ptr elementtype(i32) %"test_$B.1.addr_a0$_fetch.16", i64 %"$loop_ctr_fetch.20")
  %"test_$B.1.addr_a0$_fetch.16[]_fetch.21" = load i32, ptr %"test_$B.1.addr_a0$_fetch.16[]", align 1
  %add.2 = add nsw i32 %"test_$B.1.addr_a0$_fetch.16[]_fetch.21", 1
  %"$loop_ctr_fetch.15" = load i64, ptr %"$loop_ctr", align 1
  %"test_$B.1.addr_a0$_fetch.11[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %"test_$B.1.dim_info$.spacing$[]_fetch.12", ptr elementtype(i32) %"test_$B.1.addr_a0$_fetch.11", i64 %"$loop_ctr_fetch.15")
  store i32 %add.2, ptr %"test_$B.1.addr_a0$_fetch.11[]", align 1
  %"$loop_ctr_fetch.23" = load i64, ptr %"$loop_ctr", align 1
  %add.3 = add nsw i64 %"$loop_ctr_fetch.23", 1
  store i64 %add.3, ptr %"$loop_ctr", align 1
  br label %loop_test11

loop_exit13:                                      ; preds = %loop_test11
  %omp.pdo.norm.iv_fetch.24 = load i64, ptr %omp.pdo.norm.iv, align 1
  %add.4 = add nsw i64 %omp.pdo.norm.iv_fetch.24, 1
  store i64 %add.4, ptr %omp.pdo.norm.iv, align 1
  br label %omp.pdo.cond3

bb_new6:                                          ; preds = %alloca_0
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(),
    "QUAL.OMP.LASTPRIVATE:TYPED"(ptr %"test_$I", i32 0, i64 1),
    "QUAL.OMP.REDUCTION.ADD:F90_DV.TYPED"(ptr %"test_$B.1", %"QNCA_a0$i32*$rank1$" zeroinitializer, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %omp.pdo.norm.lb, i64 0, i64 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %omp.pdo.norm.iv, i64 0),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %omp.pdo.norm.ub, i64 0) ]

  br label %bb_new7

bb_new7:                                          ; preds = %bb_new6
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(),
    "QUAL.OMP.LINEAR:IV.TYPED"(ptr %"test_$I", i32 0, i64 1, i32 1),
    "QUAL.OMP.REDUCTION.ADD:F90_DV.TYPED"(ptr %"test_$B.1", %"QNCA_a0$i32*$rank1$" zeroinitializer, i32 0) ]

  %omp.pdo.norm.lb_fetch.5 = load i64, ptr %omp.pdo.norm.lb, align 1
  store i64 %omp.pdo.norm.lb_fetch.5, ptr %omp.pdo.norm.iv, align 1
  br label %omp.pdo.cond3

omp.pdo.epilog5:                                  ; preds = %omp.pdo.cond3
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.SIMD"() ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.LOOP"() ]

  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8, i64, i32, ptr, i32)
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64)
; end INTEL_CUSTOMIZATION
