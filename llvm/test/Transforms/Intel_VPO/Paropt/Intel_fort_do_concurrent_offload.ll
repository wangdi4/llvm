; INTEL_CUSTOMIZATION
; RUN: opt -vpo-paropt-loop-mapping-scheme=1 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-loop-collapse -S %s | FileCheck %s
; RUN: opt -vpo-paropt-loop-mapping-scheme=1 -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse)' -S %s | FileCheck %s
; RUN: opt -vpo-paropt-loop-mapping-scheme=0 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s
; RUN: opt -vpo-paropt-loop-mapping-scheme=0 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s | FileCheck %s

; This test checks that the "genericloop qual.ext.do.concurrent" construct is
; mapped to "distribute parallel do"
; after prepare pass.

; integer, parameter :: n=64
; real a(n)
; real::b(n)=17.
; do concurrent(i=1:n)
;   a(i) = b(i)+i
; enddo
;
; print *,a
; end

; Verify that DIR.OMP.GENERICLOOP is mapped to DIR.OMP.DISTRIBUTE.PARLOOP
; CHECK-NOT: call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(), {{.*}}
; CHECK: call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(), "QUAL.EXT.DO.CONCURRENT"(),

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

@"_unnamed_main$$_$B" = internal global [64 x float] [float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01, float 1.700000e+01], align 16
@0 = internal unnamed_addr constant i32 65536
@1 = internal unnamed_addr constant i32 2
@"@tid.addr" = external global i32

define void @MAIN__() {
alloca_0:
  %"$io_ctx" = alloca [8 x i64], align 16
  %"_unnamed_main$$_$I$_1" = alloca i32, align 8
  %"_unnamed_main$$_$A" = alloca [64 x float], align 16
  %"var$1" = alloca i32, align 4
  %"(&)val$" = alloca [4 x i8], align 1
  %argblock = alloca <{ i64, ptr }>, align 8
  %func_result = call i32 @for_set_fpe_(ptr @0)
  %func_result2 = call i32 @for_set_reentrancy(ptr @1)
  %do.start = alloca i32, align 4
  store i32 1, ptr %do.start, align 1
  %do.end = alloca i32, align 4
  store i32 64, ptr %do.end, align 1
  %do.step = alloca i32, align 4
  store i32 1, ptr %do.step, align 1
  %do.norm.lb = alloca i64, align 8
  store i64 0, ptr %do.norm.lb, align 1
  %do.norm.ub = alloca i64, align 8
  %do.end_fetch.1 = load i32, ptr %do.end, align 1
  %do.start_fetch.2 = load i32, ptr %do.start, align 1
  %sub.1 = sub nsw i32 %do.end_fetch.1, %do.start_fetch.2
  %do.step_fetch.3 = load i32, ptr %do.step, align 1
  %div.1 = sdiv i32 %sub.1, %do.step_fetch.3
  %int_sext = sext i32 %div.1 to i64
  store i64 %int_sext, ptr %do.norm.ub, align 1
  %do.norm.iv = alloca i64, align 8
  br label %bb_new6

do.cond10:                                        ; preds = %DIR.OMP.GENERICLOOP.4, %do.body11
  %do.norm.iv_fetch.5 = load i64, ptr %do.norm.iv, align 1
  %do.norm.ub_fetch.6 = load i64, ptr %do.norm.ub, align 1
  %rel.1 = icmp sle i64 %do.norm.iv_fetch.5, %do.norm.ub_fetch.6
  br i1 %rel.1, label %do.body11, label %do.epilog12

do.body11:                                        ; preds = %do.cond10
  %do.norm.iv_fetch.7 = load i64, ptr %do.norm.iv, align 1
  %int_sext4 = trunc i64 %do.norm.iv_fetch.7 to i32
  %do.step_fetch.8 = load i32, ptr %do.step, align 1
  %mul.1 = mul nsw i32 %int_sext4, %do.step_fetch.8
  %do.start_fetch.9 = load i32, ptr %do.start, align 1
  %add.1 = add nsw i32 %mul.1, %do.start_fetch.9
  store i32 %add.1, ptr %"_unnamed_main$$_$I$_1", align 1
  %"_unnamed_main$$_$I$_1_fetch.10" = load i32, ptr %"_unnamed_main$$_$I$_1", align 1
  %int_sext5 = sext i32 %"_unnamed_main$$_$I$_1_fetch.10" to i64
  %0 = sub nsw i64 %int_sext5, 1
  %1 = getelementptr inbounds float, ptr @"_unnamed_main$$_$B", i64 %0
  %"_unnamed_main$$_$B[]_fetch.11" = load float, ptr %1, align 1
  %"_unnamed_main$$_$I$_1_fetch.12" = load i32, ptr %"_unnamed_main$$_$I$_1", align 1
  %"(float)_unnamed_main$$_$I$_1_fetch.12$" = sitofp i32 %"_unnamed_main$$_$I$_1_fetch.12" to float
  %add.2 = fadd reassoc ninf nsz arcp contract afn float %"_unnamed_main$$_$B[]_fetch.11", %"(float)_unnamed_main$$_$I$_1_fetch.12$"
  %"_unnamed_main$$_$I$_1_fetch.13" = load i32, ptr %"_unnamed_main$$_$I$_1", align 1
  %int_sext6 = sext i32 %"_unnamed_main$$_$I$_1_fetch.13" to i64
  %2 = sub nsw i64 %int_sext6, 1
  %3 = getelementptr inbounds float, ptr %"_unnamed_main$$_$A", i64 %2
  store float %add.2, ptr %3, align 1
  %do.norm.iv_fetch.14 = load i64, ptr %do.norm.iv, align 1
  %add.3 = add nsw i64 %do.norm.iv_fetch.14, 1
  store i64 %add.3, ptr %do.norm.iv, align 1
  br label %do.cond10

do.epilog12:                                      ; preds = %do.cond10
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.GENERICLOOP"() ]
  br label %DIR.OMP.END.GENERICLOOP.1

DIR.OMP.END.GENERICLOOP.1:                        ; preds = %do.epilog12
  call void @llvm.directive.region.exit(token %6) [ "DIR.OMP.END.TEAMS"() ]
  br label %DIR.OMP.END.TEAMS.2

DIR.OMP.END.TEAMS.2:                              ; preds = %DIR.OMP.END.GENERICLOOP.1
  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.TARGET"() ]
  br label %DIR.OMP.END.TARGET.3

DIR.OMP.END.TARGET.3:                             ; preds = %DIR.OMP.END.TEAMS.2
  store [4 x i8] c"\1A\05\01\00", ptr %"(&)val$", align 1
  %BLKFIELD_i64_ = getelementptr inbounds <{ i64, ptr }>, ptr %argblock, i32 0, i32 0
  store i64 256, ptr %BLKFIELD_i64_, align 1
  %"BLKFIELD_ptr_" = getelementptr inbounds <{ i64, ptr }>, ptr %argblock, i32 0, i32 1
  store ptr %"_unnamed_main$$_$A", ptr %"BLKFIELD_ptr_", align 1
  %func_result8 = call i32 (ptr, i32, i64, ptr, ptr, ...) @for_write_seq_lis(ptr %"$io_ctx", i32 -1, i64 1239157112576, ptr %"(&)val$", ptr %argblock)
  ret void

bb_new8:                                          ; preds = %DIR.OMP.TEAMS.2

  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(),
    "QUAL.EXT.DO.CONCURRENT"(),
    "QUAL.OMP.COLLAPSE"(i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %"_unnamed_main$$_$A", float 0.000000e+00, i64 64),
    "QUAL.OMP.SHARED:TYPED"(ptr @"_unnamed_main$$_$B", float 0.000000e+00, i64 64),
    "QUAL.OMP.SHARED:TYPED"(ptr %do.step, i32 0, i64 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %do.start, i32 0, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %"_unnamed_main$$_$I$_1", i32 0, i64 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %do.norm.lb, i64 0, i64 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %do.norm.iv, i64 0),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %do.norm.ub, i64 0),
    "QUAL.OMP.OFFLOAD.KNOWN.NDRANGE"(i1 true) ]
  br label %DIR.OMP.GENERICLOOP.4

DIR.OMP.GENERICLOOP.4:                            ; preds = %bb_new8
  %do.norm.lb_fetch.4 = load i64, ptr %do.norm.lb, align 1
  store i64 %do.norm.lb_fetch.4, ptr %do.norm.iv, align 1
  br label %do.cond10

bb_new6:                                          ; preds = %alloca_0
  br label %DIR.OMP.TARGET.5

DIR.OMP.TARGET.5:                                 ; preds = %bb_new6
  br label %DIR.OMP.TARGET.1

DIR.OMP.TARGET.1:                                 ; preds = %DIR.OMP.TARGET.5
  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.EXT.DO.CONCURRENT"(),
    "QUAL.OMP.MAP.TOFROM"(ptr @"_unnamed_main$$_$B", ptr @"_unnamed_main$$_$B", i64 256, i64 39, ptr null, ptr null),
    "QUAL.OMP.MAP.TOFROM"(ptr %"_unnamed_main$$_$A", ptr %"_unnamed_main$$_$A", i64 256, i64 39, ptr null, ptr null),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %do.norm.iv, i64 0, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %"_unnamed_main$$_$I$_1", i32 0, i64 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %do.norm.ub, i64 0, i64 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %do.norm.lb, i64 0, i64 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %do.step, i32 0, i64 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %do.start, i32 0, i64 1),
    "QUAL.OMP.OFFLOAD.NDRANGE"(ptr %do.norm.ub, i64 0) ]
  br label %bb_new7

bb_new7:                                          ; preds = %DIR.OMP.TARGET.1
  br label %DIR.OMP.TEAMS.6

DIR.OMP.TEAMS.6:                                  ; preds = %bb_new7
  br label %DIR.OMP.TEAMS.2

DIR.OMP.TEAMS.2:                                  ; preds = %DIR.OMP.TEAMS.6
  %6 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.EXT.DO.CONCURRENT"(),
    "QUAL.OMP.SHARED:TYPED"(ptr %"_unnamed_main$$_$A", float 0.000000e+00, i64 64),
    "QUAL.OMP.SHARED:TYPED"(ptr @"_unnamed_main$$_$B", float 0.000000e+00, i64 64),
    "QUAL.OMP.SHARED:TYPED"(ptr %do.norm.ub, i64 0, i64 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %do.norm.lb, i64 0, i64 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %do.step, i32 0, i64 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %do.start, i32 0, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %do.norm.iv, i64 0, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %"_unnamed_main$$_$I$_1", i32 0, i64 1) ]
  br label %bb_new8
}

declare i32 @for_set_fpe_(ptr nocapture readonly)
declare i32 @for_set_reentrancy(ptr nocapture readonly)
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64)
declare void @llvm.directive.region.exit(token)
declare i32 @for_write_seq_lis(ptr, i32, i64, ptr, ptr, ...)
declare token @llvm.directive.region.entry()

!omp_offload.info = !{!0}
!0 = !{i32 0, i32 2050, i32 56484600, !"MAIN__", i32 4, i32 0, i32 0}
; end INTEL_CUSTOMIZATION
