; RUN: opt -bugpoint-enable-legacy-pm -switch-to-offload -vpo-paropt-enable-device-simd-codegen -vpo-paropt-emit-spirv-builtins -enable-device-simd -vpo-cfg-restructuring -vpo-paropt-loop-collapse -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -switch-to-offload -vpo-paropt-enable-device-simd-codegen -vpo-paropt-emit-spirv-builtins -enable-device-simd -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse,vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

;
; // This is a LLVM lit-test to check ND-range mapping to Loop nest level for 
; // OpenMP SIMD support on GPU 
;
; #include <omp.h>
; void update_stress(int Ny) {
;
;  int x_dim[128];
;  int y_dim[16];
;
; #pragma omp target teams distribute parallel for collapse(2) map(x_dim[0:128], y_dim[0:16])
;   for (int zbeg = 0; zbeg < 16; zbeg += 1) {
;     for (int iy = 0; iy < Ny; iy++) {
;       x_dim[iy] = iy;
;       y_dim[zbeg] = zbeg;
;     }
;   }
; }

; CHECK: [[S1:%[0-9A-Za-z._]+]] = load i64, ptr %loop0.upper.bnd, align 8
; CHECK: [[S2:%[0-9A-Za-z._]+]] = call spir_func i64 @_Z28__spirv_GlobalInvocationId_yv()
; CHECK: store i64 [[S2]], ptr %loop0.lower.bnd, align 8
; CHECK: [[S3:%[0-9A-Za-z._]+]] = icmp slt i64 [[S2]], [[S1]]

; ModuleID = 'target_teams_dist_par_simd_nd_range.cpp'
source_filename = "target_teams_dist_par_simd_nd_range.cpp"
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: convergent mustprogress noinline nounwind optnone
define protected spir_func void @_Z13update_stressi(i32 noundef %Ny) #0 {
entry:
  %Ny.addr = alloca i32, align 4
  %x_dim = alloca [128 x i32], align 4
  %y_dim = alloca [16 x i32], align 4
  %.capture_expr.0 = alloca i32, align 4
  %.capture_expr.1 = alloca i64, align 8
  %.omp.uncollapsed.lb = alloca i64, align 8
  %.omp.uncollapsed.ub = alloca i64, align 8
  %.omp.uncollapsed.lb3 = alloca i64, align 8
  %.omp.uncollapsed.ub4 = alloca i64, align 8
  %tmp = alloca i32, align 4
  %tmp12 = alloca i32, align 4
  %.omp.uncollapsed.iv = alloca i64, align 8
  %.omp.uncollapsed.iv13 = alloca i64, align 8
  %zbeg = alloca i32, align 4
  %iy = alloca i32, align 4
  %Ny.addr.ascast = addrspacecast ptr %Ny.addr to ptr addrspace(4)
  %x_dim.ascast = addrspacecast ptr %x_dim to ptr addrspace(4)
  %y_dim.ascast = addrspacecast ptr %y_dim to ptr addrspace(4)
  %.capture_expr.0.ascast = addrspacecast ptr %.capture_expr.0 to ptr addrspace(4)
  %.capture_expr.1.ascast = addrspacecast ptr %.capture_expr.1 to ptr addrspace(4)
  %.omp.uncollapsed.lb.ascast = addrspacecast ptr %.omp.uncollapsed.lb to ptr addrspace(4)
  %.omp.uncollapsed.ub.ascast = addrspacecast ptr %.omp.uncollapsed.ub to ptr addrspace(4)
  %.omp.uncollapsed.lb3.ascast = addrspacecast ptr %.omp.uncollapsed.lb3 to ptr addrspace(4)
  %.omp.uncollapsed.ub4.ascast = addrspacecast ptr %.omp.uncollapsed.ub4 to ptr addrspace(4)
  %tmp.ascast = addrspacecast ptr %tmp to ptr addrspace(4)
  %tmp12.ascast = addrspacecast ptr %tmp12 to ptr addrspace(4)
  %.omp.uncollapsed.iv.ascast = addrspacecast ptr %.omp.uncollapsed.iv to ptr addrspace(4)
  %.omp.uncollapsed.iv13.ascast = addrspacecast ptr %.omp.uncollapsed.iv13 to ptr addrspace(4)
  %zbeg.ascast = addrspacecast ptr %zbeg to ptr addrspace(4)
  %iy.ascast = addrspacecast ptr %iy to ptr addrspace(4)
  store i32 %Ny, ptr addrspace(4) %Ny.addr.ascast, align 4
  %0 = load i32, ptr addrspace(4) %Ny.addr.ascast, align 4
  store i32 %0, ptr addrspace(4) %.capture_expr.0.ascast, align 4
  %1 = load i32, ptr addrspace(4) %.capture_expr.0.ascast, align 4
  %sub = sub nsw i32 %1, 0
  %sub1 = sub nsw i32 %sub, 1
  %add = add nsw i32 %sub1, 1
  %div = sdiv i32 %add, 1
  %conv = sext i32 %div to i64
  %mul = mul nsw i64 16, %conv
  %sub2 = sub nsw i64 %mul, 1
  store i64 %sub2, ptr addrspace(4) %.capture_expr.1.ascast, align 8
  store i64 0, ptr addrspace(4) %.omp.uncollapsed.lb.ascast, align 8
  store i64 15, ptr addrspace(4) %.omp.uncollapsed.ub.ascast, align 8
  store i64 0, ptr addrspace(4) %.omp.uncollapsed.lb3.ascast, align 8
  %2 = load i32, ptr addrspace(4) %.capture_expr.0.ascast, align 4
  %sub5 = sub nsw i32 %2, 0
  %sub6 = sub nsw i32 %sub5, 1
  %add7 = add nsw i32 %sub6, 1
  %div8 = sdiv i32 %add7, 1
  %conv9 = sext i32 %div8 to i64
  %sub10 = sub nsw i64 %conv9, 1
  store i64 %sub10, ptr addrspace(4) %.omp.uncollapsed.ub4.ascast, align 8
  %arrayidx = getelementptr inbounds [128 x i32], ptr addrspace(4) %x_dim.ascast, i64 0, i64 0
  %arrayidx11 = getelementptr inbounds [16 x i32], ptr addrspace(4) %y_dim.ascast, i64 0, i64 0
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %x_dim.ascast, ptr addrspace(4) %arrayidx, i64 512, i64 35, ptr null, ptr null), ; MAP type: 35 = 0x23 = TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %y_dim.ascast, ptr addrspace(4) %arrayidx11, i64 64, i64 35, ptr null, ptr null), ; MAP type: 35 = 0x23 = TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.iv.ascast, i64 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.lb.ascast, i64 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.ub.ascast, i64 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %zbeg.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.iv13.ascast, i64 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.lb3.ascast, i64 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.ub4.ascast, i64 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %iy.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.capture_expr.0.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp12.ascast, i32 0, i32 1) ]

  %array.begin32 = getelementptr inbounds [128 x i32], ptr addrspace(4) %x_dim.ascast, i32 0, i32 0
  %array.begin33 = getelementptr inbounds [16 x i32], ptr addrspace(4) %y_dim.ascast, i32 0, i32 0
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %x_dim.ascast, i32 0, i64 128),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %y_dim.ascast, i32 0, i64 16),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.iv.ascast, i64 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %.omp.uncollapsed.lb.ascast, i64 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %.omp.uncollapsed.ub.ascast, i64 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %zbeg.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.iv13.ascast, i64 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %.omp.uncollapsed.lb3.ascast, i64 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %.omp.uncollapsed.ub4.ascast, i64 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %iy.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %.capture_expr.0.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp12.ascast, i32 0, i32 1) ]

  %5 = load i32, ptr addrspace(4) %.capture_expr.0.ascast, align 4
  %cmp = icmp slt i32 0, %5
  br i1 %cmp, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %entry
  %array.begin = getelementptr inbounds [128 x i32], ptr addrspace(4) %x_dim.ascast, i32 0, i32 0
  %array.begin31 = getelementptr inbounds [16 x i32], ptr addrspace(4) %y_dim.ascast, i32 0, i32 0
  %6 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(),
    "QUAL.OMP.COLLAPSE"(i32 2),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %x_dim.ascast, i32 0, i64 128),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %y_dim.ascast, i32 0, i64 16),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) %.omp.uncollapsed.iv.ascast, i64 0, ptr addrspace(4) %.omp.uncollapsed.iv13.ascast, i64 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.lb.ascast, i64 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) %.omp.uncollapsed.ub.ascast, i64 0, ptr addrspace(4) %.omp.uncollapsed.ub4.ascast, i64 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %zbeg.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.lb3.ascast, i64 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %iy.ascast, i32 0, i32 1) ]

  %7 = load i64, ptr addrspace(4) %.omp.uncollapsed.lb.ascast, align 8
  store i64 %7, ptr addrspace(4) %.omp.uncollapsed.iv.ascast, align 8
  br label %omp.uncollapsed.loop.cond

omp.uncollapsed.loop.cond:                        ; preds = %omp.uncollapsed.loop.inc28, %omp.precond.then
  %8 = load i64, ptr addrspace(4) %.omp.uncollapsed.iv.ascast, align 8
  %9 = load i64, ptr addrspace(4) %.omp.uncollapsed.ub.ascast, align 8
  %cmp14 = icmp sle i64 %8, %9
  br i1 %cmp14, label %omp.uncollapsed.loop.body, label %omp.uncollapsed.loop.end30

omp.uncollapsed.loop.body:                        ; preds = %omp.uncollapsed.loop.cond
  %10 = load i64, ptr addrspace(4) %.omp.uncollapsed.lb3.ascast, align 8
  store i64 %10, ptr addrspace(4) %.omp.uncollapsed.iv13.ascast, align 8
  br label %omp.uncollapsed.loop.cond15

omp.uncollapsed.loop.cond15:                      ; preds = %omp.uncollapsed.loop.inc, %omp.uncollapsed.loop.body
  %11 = load i64, ptr addrspace(4) %.omp.uncollapsed.iv13.ascast, align 8
  %12 = load i64, ptr addrspace(4) %.omp.uncollapsed.ub4.ascast, align 8
  %cmp16 = icmp sle i64 %11, %12
  br i1 %cmp16, label %omp.uncollapsed.loop.body17, label %omp.uncollapsed.loop.end

omp.uncollapsed.loop.body17:                      ; preds = %omp.uncollapsed.loop.cond15
  %13 = load i64, ptr addrspace(4) %.omp.uncollapsed.iv.ascast, align 8
  %mul18 = mul nsw i64 %13, 1
  %add19 = add nsw i64 0, %mul18
  %conv20 = trunc i64 %add19 to i32
  store i32 %conv20, ptr addrspace(4) %zbeg.ascast, align 4
  %14 = load i64, ptr addrspace(4) %.omp.uncollapsed.iv13.ascast, align 8
  %mul21 = mul nsw i64 %14, 1
  %add22 = add nsw i64 0, %mul21
  %conv23 = trunc i64 %add22 to i32
  store i32 %conv23, ptr addrspace(4) %iy.ascast, align 4
  %15 = load i32, ptr addrspace(4) %iy.ascast, align 4
  %16 = load i32, ptr addrspace(4) %iy.ascast, align 4
  %idxprom = sext i32 %16 to i64
  %arrayidx24 = getelementptr inbounds [128 x i32], ptr addrspace(4) %x_dim.ascast, i64 0, i64 %idxprom
  store i32 %15, ptr addrspace(4) %arrayidx24, align 4
  %17 = load i32, ptr addrspace(4) %zbeg.ascast, align 4
  %18 = load i32, ptr addrspace(4) %zbeg.ascast, align 4
  %idxprom25 = sext i32 %18 to i64
  %arrayidx26 = getelementptr inbounds [16 x i32], ptr addrspace(4) %y_dim.ascast, i64 0, i64 %idxprom25
  store i32 %17, ptr addrspace(4) %arrayidx26, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.uncollapsed.loop.body17
  br label %omp.uncollapsed.loop.inc

omp.uncollapsed.loop.inc:                         ; preds = %omp.body.continue
  %19 = load i64, ptr addrspace(4) %.omp.uncollapsed.iv13.ascast, align 8
  %add27 = add nsw i64 %19, 1
  store i64 %add27, ptr addrspace(4) %.omp.uncollapsed.iv13.ascast, align 8
  br label %omp.uncollapsed.loop.cond15

omp.uncollapsed.loop.end:                         ; preds = %omp.uncollapsed.loop.cond15
  br label %omp.uncollapsed.loop.inc28

omp.uncollapsed.loop.inc28:                       ; preds = %omp.uncollapsed.loop.end
  %20 = load i64, ptr addrspace(4) %.omp.uncollapsed.iv.ascast, align 8
  %add29 = add nsw i64 %20, 1
  store i64 %add29, ptr addrspace(4) %.omp.uncollapsed.iv.ascast, align 8
  br label %omp.uncollapsed.loop.cond

omp.uncollapsed.loop.end30:                       ; preds = %omp.uncollapsed.loop.cond
  call void @llvm.directive.region.exit(token %6) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]

  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.uncollapsed.loop.end30, %entry
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.TEAMS"() ]

  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.TARGET"() ]

  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { convergent mustprogress noinline nounwind optnone "approx-func-fp-math"="true" "contains-openmp-target"="true" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2, !3, !4, !5}
!opencl.compiler.options = !{!6}
!llvm.ident = !{!7}

!0 = !{i32 0, i32 2065, i32 263168, !"_Z13update_stressi", i32 7, i32 0, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 50}
!3 = !{i32 7, !"openmp-device", i32 50}
!4 = !{i32 8, !"PIC Level", i32 2}
!5 = !{i32 7, !"frame-pointer", i32 2}
!6 = !{}
!7 = !{!"clang version 16.0.0"}
