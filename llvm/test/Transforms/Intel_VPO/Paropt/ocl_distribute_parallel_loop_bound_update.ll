; RUN: opt < %s -switch-to-offload=true -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt  -S

; The test checks that bounds update for distribute parallel loop
; are generated properly for OpenCL target.
; The assertion was:
; opt: llvm/lib/IR/Instructions.cpp:2204: static llvm::BinaryOperator* llvm::BinaryOperator::Create(llvm::Instruction::BinaryOps, llvm::Value*, llvm::Value*, const llvm::Twine&, llvm::Instruction*): Assertion `S1->getType() == S2->getType() && "Cannot create binary operator with two operands of differing type!"' failed.

; Original code:
; void kernel(float c0,float c1, float *A0,float *Anext,const int nx, const int ny, const int nz)
; {
;   int i, j, k;
;   int size=nx*ny*nz;
; #pragma omp target teams distribute parallel for simd collapse(3)
;   for(k=-100;k<nz-1;k++) {
;     for(j=1;j<ny-1;j++) {
;       for(i=1;i<nx-1;i++) {
;       }
;     }
;   }
; }

; ModuleID = 'kernels.c'
source_filename = "kernels.c"
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local spir_func void @kernel(float %c0, float %c1, float* %A0, float* %Anext, i32 %nx, i32 %ny, i32 %nz) #0 {
entry:
  %c0.addr = alloca float, align 4
  %c1.addr = alloca float, align 4
  %A0.addr = alloca float*, align 8
  %Anext.addr = alloca float*, align 8
  %nx.addr = alloca i32, align 4
  %ny.addr = alloca i32, align 4
  %nz.addr = alloca i32, align 4
  %i = alloca i32, align 4
  %j = alloca i32, align 4
  %k = alloca i32, align 4
  %size = alloca i32, align 4
  %.omp.iv = alloca i64, align 8
  %tmp = alloca i32, align 4
  %tmp2 = alloca i32, align 4
  %tmp3 = alloca i32, align 4
  %.capture_expr. = alloca i32, align 4
  %.capture_expr.4 = alloca i32, align 4
  %.capture_expr.6 = alloca i32, align 4
  %.capture_expr.8 = alloca i64, align 8
  %.capture_expr.24 = alloca i32, align 4
  %.capture_expr.29 = alloca i32, align 4
  %.omp.lb = alloca i64, align 8
  %.omp.ub = alloca i64, align 8
  %.omp.stride = alloca i64, align 8
  %.omp.is_last = alloca i32, align 4
  store float %c0, float* %c0.addr, align 4
  store float %c1, float* %c1.addr, align 4
  store float* %A0, float** %A0.addr, align 8
  store float* %Anext, float** %Anext.addr, align 8
  store i32 %nx, i32* %nx.addr, align 4
  store i32 %ny, i32* %ny.addr, align 4
  store i32 %nz, i32* %nz.addr, align 4
  %0 = load i32, i32* %nx.addr, align 4
  %1 = load i32, i32* %ny.addr, align 4
  %mul = mul nsw i32 %0, %1
  %2 = load i32, i32* %nz.addr, align 4
  %mul1 = mul nsw i32 %mul, %2
  store i32 %mul1, i32* %size, align 4
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.PRIVATE"(i32* %.capture_expr.), "QUAL.OMP.PRIVATE"(i64* %.capture_expr.8), "QUAL.OMP.PRIVATE"(i32* %.capture_expr.24), "QUAL.OMP.PRIVATE"(i64* %.omp.stride), "QUAL.OMP.PRIVATE"(i32* %.capture_expr.4), "QUAL.OMP.PRIVATE"(i32* %.capture_expr.6), "QUAL.OMP.PRIVATE"(i32* %.capture_expr.29), "QUAL.OMP.PRIVATE"(i32* %.omp.is_last), "QUAL.OMP.PRIVATE"(i64* %.omp.lb), "QUAL.OMP.FIRSTPRIVATE"(i32* %nz.addr), "QUAL.OMP.FIRSTPRIVATE"(i32* %ny.addr), "QUAL.OMP.FIRSTPRIVATE"(i32* %nx.addr), "QUAL.OMP.PRIVATE"(i32* %tmp), "QUAL.OMP.PRIVATE"(i32* %tmp2), "QUAL.OMP.PRIVATE"(i32* %tmp3), "QUAL.OMP.PRIVATE"(i32* %tmp), "QUAL.OMP.PRIVATE"(i32* %tmp2), "QUAL.OMP.PRIVATE"(i32* %tmp3) ]
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(), "QUAL.OMP.PRIVATE"(i32* %.capture_expr.), "QUAL.OMP.SHARED"(i32* %nz.addr), "QUAL.OMP.PRIVATE"(i32* %.capture_expr.4), "QUAL.OMP.SHARED"(i32* %ny.addr), "QUAL.OMP.PRIVATE"(i32* %.capture_expr.6), "QUAL.OMP.SHARED"(i32* %nx.addr), "QUAL.OMP.PRIVATE"(i64* %.capture_expr.8), "QUAL.OMP.PRIVATE"(i32* %.capture_expr.24), "QUAL.OMP.PRIVATE"(i32* %.capture_expr.29), "QUAL.OMP.PRIVATE"(i64* %.omp.lb), "QUAL.OMP.PRIVATE"(i64* %.omp.stride), "QUAL.OMP.PRIVATE"(i32* %.omp.is_last), "QUAL.OMP.PRIVATE"(i32* %tmp), "QUAL.OMP.PRIVATE"(i32* %tmp2), "QUAL.OMP.PRIVATE"(i32* %tmp3) ]
  %5 = load i32, i32* %nz.addr, align 4
  %sub = sub nsw i32 %5, 1
  store i32 %sub, i32* %.capture_expr., align 4
  %6 = load i32, i32* %ny.addr, align 4
  %sub5 = sub nsw i32 %6, 1
  store i32 %sub5, i32* %.capture_expr.4, align 4
  %7 = load i32, i32* %nx.addr, align 4
  %sub7 = sub nsw i32 %7, 1
  store i32 %sub7, i32* %.capture_expr.6, align 4
  %8 = load i32, i32* %.capture_expr., align 4
  %sub9 = sub nsw i32 %8, -100
  %sub10 = sub nsw i32 %sub9, 1
  %add = add nsw i32 %sub10, 1
  %div = sdiv i32 %add, 1
  %conv = sext i32 %div to i64
  %9 = load i32, i32* %.capture_expr.4, align 4
  %sub11 = sub nsw i32 %9, 1
  %sub12 = sub nsw i32 %sub11, 1
  %add13 = add nsw i32 %sub12, 1
  %div14 = sdiv i32 %add13, 1
  %conv15 = sext i32 %div14 to i64
  %mul16 = mul nsw i64 %conv, %conv15
  %10 = load i32, i32* %.capture_expr.6, align 4
  %sub17 = sub nsw i32 %10, 1
  %sub18 = sub nsw i32 %sub17, 1
  %add19 = add nsw i32 %sub18, 1
  %div20 = sdiv i32 %add19, 1
  %conv21 = sext i32 %div20 to i64
  %mul22 = mul nsw i64 %mul16, %conv21
  %sub23 = sub nsw i64 %mul22, 1
  store i64 %sub23, i64* %.capture_expr.8, align 8
  %11 = load i32, i32* %.capture_expr.6, align 4
  %sub25 = sub nsw i32 %11, 1
  %sub26 = sub nsw i32 %sub25, 1
  %add27 = add nsw i32 %sub26, 1
  %div28 = sdiv i32 %add27, 1
  store i32 %div28, i32* %.capture_expr.24, align 4
  %12 = load i32, i32* %.capture_expr.24, align 4
  %13 = load i32, i32* %.capture_expr.4, align 4
  %sub30 = sub nsw i32 %13, 1
  %sub31 = sub nsw i32 %sub30, 1
  %add32 = add nsw i32 %sub31, 1
  %div33 = sdiv i32 %add32, 1
  %mul34 = mul nsw i32 %12, %div33
  store i32 %mul34, i32* %.capture_expr.29, align 4
  %14 = load i32, i32* %.capture_expr., align 4
  %cmp = icmp slt i32 -100, %14
  br i1 %cmp, label %land.lhs.true, label %omp.precond.end

land.lhs.true:                                    ; preds = %entry
  %15 = load i32, i32* %.capture_expr.4, align 4
  %cmp37 = icmp slt i32 1, %15
  br i1 %cmp37, label %land.lhs.true40, label %omp.precond.end

land.lhs.true40:                                  ; preds = %land.lhs.true
  %16 = load i32, i32* %.capture_expr.6, align 4
  %cmp41 = icmp slt i32 1, %16
  br i1 %cmp41, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %land.lhs.true40
  store i64 0, i64* %.omp.lb, align 8
  %17 = load i64, i64* %.capture_expr.8, align 8
  store i64 %17, i64* %.omp.ub, align 8
  store i64 1, i64* %.omp.stride, align 8
  store i32 0, i32* %.omp.is_last, align 4
  %18 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(), "QUAL.OMP.COLLAPSE"(i32 3), "QUAL.OMP.FIRSTPRIVATE"(i64* %.omp.lb), "QUAL.OMP.NORMALIZED.IV"(i64* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i64* %.omp.ub), "QUAL.OMP.SHARED"(i32* %.capture_expr.29), "QUAL.OMP.SHARED"(i32* %.capture_expr.24), "QUAL.OMP.SHARED"(i32* %.capture_expr.4), "QUAL.OMP.SHARED"(i32* %.capture_expr.6) ]
  %19 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.COLLAPSE"(i32 3) ]
  %20 = load i64, i64* %.omp.lb, align 8
  store i64 %20, i64* %.omp.iv, align 8
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %omp.precond.then
  %21 = load i64, i64* %.omp.iv, align 8
  %22 = load i64, i64* %.omp.ub, align 8
  %cmp44 = icmp sle i64 %21, %22
  br i1 %cmp44, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %23 = load i64, i64* %.omp.iv, align 8
  %24 = load i32, i32* %.capture_expr.29, align 4
  %conv46 = sext i32 %24 to i64
  %div47 = sdiv i64 %23, %conv46
  %mul48 = mul nsw i64 %div47, 1
  %add49 = add nsw i64 -100, %mul48
  %conv50 = trunc i64 %add49 to i32
  store i32 %conv50, i32* %k, align 4
  %25 = load i64, i64* %.omp.iv, align 8
  %26 = load i32, i32* %.capture_expr.24, align 4
  %conv51 = sext i32 %26 to i64
  %div52 = sdiv i64 %25, %conv51
  %27 = load i32, i32* %.capture_expr.4, align 4
  %sub53 = sub nsw i32 %27, 1
  %sub54 = sub nsw i32 %sub53, 1
  %add55 = add nsw i32 %sub54, 1
  %div56 = sdiv i32 %add55, 1
  %conv57 = sext i32 %div56 to i64
  %rem = srem i64 %div52, %conv57
  %mul58 = mul nsw i64 %rem, 1
  %add59 = add nsw i64 1, %mul58
  %conv60 = trunc i64 %add59 to i32
  store i32 %conv60, i32* %j, align 4
  %28 = load i64, i64* %.omp.iv, align 8
  %29 = load i32, i32* %.capture_expr.6, align 4
  %sub61 = sub nsw i32 %29, 1
  %sub62 = sub nsw i32 %sub61, 1
  %add63 = add nsw i32 %sub62, 1
  %div64 = sdiv i32 %add63, 1
  %conv65 = sext i32 %div64 to i64
  %rem66 = srem i64 %28, %conv65
  %mul67 = mul nsw i64 %rem66, 1
  %add68 = add nsw i64 1, %mul67
  %conv69 = trunc i64 %add68 to i32
  store i32 %conv69, i32* %i, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %30 = load i64, i64* %.omp.iv, align 8
  %add70 = add nsw i64 %30, 1
  store i64 %add70, i64* %.omp.iv, align 8
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %19) [ "DIR.OMP.END.SIMD"() ]
  call void @llvm.directive.region.exit(token %18) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %land.lhs.true40, %land.lhs.true, %entry
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind optnone uwtable "may-have-openmp-directive"="true" }
attributes #1 = { nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}

!0 = !{i32 0, i32 2052, i32 79699101, !"kernel", i32 12, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{}
