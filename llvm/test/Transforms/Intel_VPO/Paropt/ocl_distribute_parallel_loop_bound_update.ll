; RUN: opt -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -S

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

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local spir_func void @kernel(float %c0, float %c1, float addrspace(4)* %A0, float addrspace(4)* %Anext, i32 %nx, i32 %ny, i32 %nz) #0 {
entry:
  %c0.addr = alloca float, align 4
  %0 = addrspacecast float* %c0.addr to float addrspace(4)*
  %c1.addr = alloca float, align 4
  %1 = addrspacecast float* %c1.addr to float addrspace(4)*
  %A0.addr = alloca float addrspace(4)*, align 8
  %2 = addrspacecast float addrspace(4)** %A0.addr to float addrspace(4)* addrspace(4)*
  %Anext.addr = alloca float addrspace(4)*, align 8
  %3 = addrspacecast float addrspace(4)** %Anext.addr to float addrspace(4)* addrspace(4)*
  %nx.addr = alloca i32, align 4
  %4 = addrspacecast i32* %nx.addr to i32 addrspace(4)*
  %ny.addr = alloca i32, align 4
  %5 = addrspacecast i32* %ny.addr to i32 addrspace(4)*
  %nz.addr = alloca i32, align 4
  %6 = addrspacecast i32* %nz.addr to i32 addrspace(4)*
  %i = alloca i32, align 4
  %7 = addrspacecast i32* %i to i32 addrspace(4)*
  %j = alloca i32, align 4
  %8 = addrspacecast i32* %j to i32 addrspace(4)*
  %k = alloca i32, align 4
  %9 = addrspacecast i32* %k to i32 addrspace(4)*
  %size = alloca i32, align 4
  %10 = addrspacecast i32* %size to i32 addrspace(4)*
  %.capture_expr. = alloca i32, align 4
  %11 = addrspacecast i32* %.capture_expr. to i32 addrspace(4)*
  %.capture_expr.2 = alloca i32, align 4
  %12 = addrspacecast i32* %.capture_expr.2 to i32 addrspace(4)*
  %.capture_expr.4 = alloca i32, align 4
  %13 = addrspacecast i32* %.capture_expr.4 to i32 addrspace(4)*
  %.capture_expr.6 = alloca i64, align 8
  %14 = addrspacecast i64* %.capture_expr.6 to i64 addrspace(4)*
  %.omp.lb = alloca i64, align 8
  %15 = addrspacecast i64* %.omp.lb to i64 addrspace(4)*
  %.omp.ub = alloca i64, align 8
  %16 = addrspacecast i64* %.omp.ub to i64 addrspace(4)*
  %.omp.iv = alloca i64, align 8
  %17 = addrspacecast i64* %.omp.iv to i64 addrspace(4)*
  %tmp = alloca i32, align 4
  %18 = addrspacecast i32* %tmp to i32 addrspace(4)*
  %tmp22 = alloca i32, align 4
  %19 = addrspacecast i32* %tmp22 to i32 addrspace(4)*
  %tmp23 = alloca i32, align 4
  %20 = addrspacecast i32* %tmp23 to i32 addrspace(4)*
  store float %c0, float addrspace(4)* %0, align 4
  store float %c1, float addrspace(4)* %1, align 4
  store float addrspace(4)* %A0, float addrspace(4)* addrspace(4)* %2, align 8
  store float addrspace(4)* %Anext, float addrspace(4)* addrspace(4)* %3, align 8
  store i32 %nx, i32 addrspace(4)* %4, align 4
  store i32 %ny, i32 addrspace(4)* %5, align 4
  store i32 %nz, i32 addrspace(4)* %6, align 4
  %21 = load i32, i32 addrspace(4)* %4, align 4
  %22 = load i32, i32 addrspace(4)* %5, align 4
  %mul = mul nsw i32 %21, %22
  %23 = load i32, i32 addrspace(4)* %6, align 4
  %mul1 = mul nsw i32 %mul, %23
  store i32 %mul1, i32 addrspace(4)* %10, align 4
  %24 = load i32, i32 addrspace(4)* %6, align 4
  %sub = sub nsw i32 %24, 1
  store i32 %sub, i32 addrspace(4)* %11, align 4
  %25 = load i32, i32 addrspace(4)* %5, align 4
  %sub3 = sub nsw i32 %25, 1
  store i32 %sub3, i32 addrspace(4)* %12, align 4
  %26 = load i32, i32 addrspace(4)* %4, align 4
  %sub5 = sub nsw i32 %26, 1
  store i32 %sub5, i32 addrspace(4)* %13, align 4
  %27 = load i32, i32 addrspace(4)* %11, align 4
  %sub7 = sub nsw i32 %27, -100
  %sub8 = sub nsw i32 %sub7, 1
  %add = add nsw i32 %sub8, 1
  %div = sdiv i32 %add, 1
  %conv = sext i32 %div to i64
  %28 = load i32, i32 addrspace(4)* %12, align 4
  %sub9 = sub nsw i32 %28, 1
  %sub10 = sub nsw i32 %sub9, 1
  %add11 = add nsw i32 %sub10, 1
  %div12 = sdiv i32 %add11, 1
  %conv13 = sext i32 %div12 to i64
  %mul14 = mul nsw i64 %conv, %conv13
  %29 = load i32, i32 addrspace(4)* %13, align 4
  %sub15 = sub nsw i32 %29, 1
  %sub16 = sub nsw i32 %sub15, 1
  %add17 = add nsw i32 %sub16, 1
  %div18 = sdiv i32 %add17, 1
  %conv19 = sext i32 %div18 to i64
  %mul20 = mul nsw i64 %mul14, %conv19
  %sub21 = sub nsw i64 %mul20, 1
  store i64 %sub21, i64 addrspace(4)* %14, align 8
  store i64 0, i64 addrspace(4)* %15, align 8
  %30 = load i64, i64 addrspace(4)* %14, align 8
  store i64 %30, i64 addrspace(4)* %16, align 8
  %31 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %11), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %12), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %13), "QUAL.OMP.FIRSTPRIVATE"(i64 addrspace(4)* %15), "QUAL.OMP.FIRSTPRIVATE"(i64 addrspace(4)* %16), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %9), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %8), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %7), "QUAL.OMP.PRIVATE"(i64 addrspace(4)* %17), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %18), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %19), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %20) ]
  %32 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(), "QUAL.OMP.SHARED"(i32 addrspace(4)* %11), "QUAL.OMP.SHARED"(i32 addrspace(4)* %12), "QUAL.OMP.SHARED"(i32 addrspace(4)* %13), "QUAL.OMP.SHARED"(i64 addrspace(4)* %15), "QUAL.OMP.SHARED"(i64 addrspace(4)* %16), "QUAL.OMP.SHARED"(i32 addrspace(4)* %9), "QUAL.OMP.SHARED"(i32 addrspace(4)* %8), "QUAL.OMP.SHARED"(i32 addrspace(4)* %7), "QUAL.OMP.PRIVATE"(i64 addrspace(4)* %17), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %18), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %19), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %20) ]
  %33 = load i32, i32 addrspace(4)* %11, align 4
  %cmp = icmp slt i32 -100, %33
  br i1 %cmp, label %land.lhs.true, label %omp.precond.end

land.lhs.true:                                    ; preds = %entry
  %34 = load i32, i32 addrspace(4)* %12, align 4
  %cmp26 = icmp slt i32 1, %34
  br i1 %cmp26, label %land.lhs.true29, label %omp.precond.end

land.lhs.true29:                                  ; preds = %land.lhs.true
  %35 = load i32, i32 addrspace(4)* %13, align 4
  %cmp30 = icmp slt i32 1, %35
  br i1 %cmp30, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %land.lhs.true29
  %36 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(), "QUAL.OMP.COLLAPSE"(i32 3), "QUAL.OMP.FIRSTPRIVATE"(i64 addrspace(4)* %15), "QUAL.OMP.NORMALIZED.IV"(i64 addrspace(4)* %17), "QUAL.OMP.NORMALIZED.UB"(i64 addrspace(4)* %16), "QUAL.OMP.SHARED"(i32 addrspace(4)* %12), "QUAL.OMP.SHARED"(i32 addrspace(4)* %13), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %9), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %8), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %7) ]
  %37 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.COLLAPSE"(i32 3) ]
  %38 = load i64, i64 addrspace(4)* %15, align 8
  store i64 %38, i64 addrspace(4)* %17, align 8
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %omp.precond.then
  %39 = load i64, i64 addrspace(4)* %17, align 8
  %40 = load i64, i64 addrspace(4)* %16, align 8
  %cmp33 = icmp sle i64 %39, %40
  br i1 %cmp33, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %41 = load i64, i64 addrspace(4)* %17, align 8
  %42 = load i32, i32 addrspace(4)* %12, align 4
  %sub35 = sub nsw i32 %42, 1
  %sub36 = sub nsw i32 %sub35, 1
  %add37 = add nsw i32 %sub36, 1
  %div38 = sdiv i32 %add37, 1
  %mul39 = mul nsw i32 1, %div38
  %43 = load i32, i32 addrspace(4)* %13, align 4
  %sub40 = sub nsw i32 %43, 1
  %sub41 = sub nsw i32 %sub40, 1
  %add42 = add nsw i32 %sub41, 1
  %div43 = sdiv i32 %add42, 1
  %mul44 = mul nsw i32 %mul39, %div43
  %conv45 = sext i32 %mul44 to i64
  %div46 = sdiv i64 %41, %conv45
  %mul47 = mul nsw i64 %div46, 1
  %add48 = add nsw i64 -100, %mul47
  %conv49 = trunc i64 %add48 to i32
  store i32 %conv49, i32 addrspace(4)* %9, align 4
  %44 = load i64, i64 addrspace(4)* %17, align 8
  %45 = load i64, i64 addrspace(4)* %17, align 8
  %46 = load i32, i32 addrspace(4)* %12, align 4
  %sub50 = sub nsw i32 %46, 1
  %sub51 = sub nsw i32 %sub50, 1
  %add52 = add nsw i32 %sub51, 1
  %div53 = sdiv i32 %add52, 1
  %mul54 = mul nsw i32 1, %div53
  %47 = load i32, i32 addrspace(4)* %13, align 4
  %sub55 = sub nsw i32 %47, 1
  %sub56 = sub nsw i32 %sub55, 1
  %add57 = add nsw i32 %sub56, 1
  %div58 = sdiv i32 %add57, 1
  %mul59 = mul nsw i32 %mul54, %div58
  %conv60 = sext i32 %mul59 to i64
  %div61 = sdiv i64 %45, %conv60
  %48 = load i32, i32 addrspace(4)* %12, align 4
  %sub62 = sub nsw i32 %48, 1
  %sub63 = sub nsw i32 %sub62, 1
  %add64 = add nsw i32 %sub63, 1
  %div65 = sdiv i32 %add64, 1
  %mul66 = mul nsw i32 1, %div65
  %49 = load i32, i32 addrspace(4)* %13, align 4
  %sub67 = sub nsw i32 %49, 1
  %sub68 = sub nsw i32 %sub67, 1
  %add69 = add nsw i32 %sub68, 1
  %div70 = sdiv i32 %add69, 1
  %mul71 = mul nsw i32 %mul66, %div70
  %conv72 = sext i32 %mul71 to i64
  %mul73 = mul nsw i64 %div61, %conv72
  %sub74 = sub nsw i64 %44, %mul73
  %50 = load i32, i32 addrspace(4)* %13, align 4
  %sub75 = sub nsw i32 %50, 1
  %sub76 = sub nsw i32 %sub75, 1
  %add77 = add nsw i32 %sub76, 1
  %div78 = sdiv i32 %add77, 1
  %mul79 = mul nsw i32 1, %div78
  %conv80 = sext i32 %mul79 to i64
  %div81 = sdiv i64 %sub74, %conv80
  %mul82 = mul nsw i64 %div81, 1
  %add83 = add nsw i64 1, %mul82
  %conv84 = trunc i64 %add83 to i32
  store i32 %conv84, i32 addrspace(4)* %8, align 4
  %51 = load i64, i64 addrspace(4)* %17, align 8
  %52 = load i64, i64 addrspace(4)* %17, align 8
  %53 = load i32, i32 addrspace(4)* %12, align 4
  %sub85 = sub nsw i32 %53, 1
  %sub86 = sub nsw i32 %sub85, 1
  %add87 = add nsw i32 %sub86, 1
  %div88 = sdiv i32 %add87, 1
  %mul89 = mul nsw i32 1, %div88
  %54 = load i32, i32 addrspace(4)* %13, align 4
  %sub90 = sub nsw i32 %54, 1
  %sub91 = sub nsw i32 %sub90, 1
  %add92 = add nsw i32 %sub91, 1
  %div93 = sdiv i32 %add92, 1
  %mul94 = mul nsw i32 %mul89, %div93
  %conv95 = sext i32 %mul94 to i64
  %div96 = sdiv i64 %52, %conv95
  %55 = load i32, i32 addrspace(4)* %12, align 4
  %sub97 = sub nsw i32 %55, 1
  %sub98 = sub nsw i32 %sub97, 1
  %add99 = add nsw i32 %sub98, 1
  %div100 = sdiv i32 %add99, 1
  %mul101 = mul nsw i32 1, %div100
  %56 = load i32, i32 addrspace(4)* %13, align 4
  %sub102 = sub nsw i32 %56, 1
  %sub103 = sub nsw i32 %sub102, 1
  %add104 = add nsw i32 %sub103, 1
  %div105 = sdiv i32 %add104, 1
  %mul106 = mul nsw i32 %mul101, %div105
  %conv107 = sext i32 %mul106 to i64
  %mul108 = mul nsw i64 %div96, %conv107
  %sub109 = sub nsw i64 %51, %mul108
  %57 = load i64, i64 addrspace(4)* %17, align 8
  %58 = load i64, i64 addrspace(4)* %17, align 8
  %59 = load i32, i32 addrspace(4)* %12, align 4
  %sub110 = sub nsw i32 %59, 1
  %sub111 = sub nsw i32 %sub110, 1
  %add112 = add nsw i32 %sub111, 1
  %div113 = sdiv i32 %add112, 1
  %mul114 = mul nsw i32 1, %div113
  %60 = load i32, i32 addrspace(4)* %13, align 4
  %sub115 = sub nsw i32 %60, 1
  %sub116 = sub nsw i32 %sub115, 1
  %add117 = add nsw i32 %sub116, 1
  %div118 = sdiv i32 %add117, 1
  %mul119 = mul nsw i32 %mul114, %div118
  %conv120 = sext i32 %mul119 to i64
  %div121 = sdiv i64 %58, %conv120
  %61 = load i32, i32 addrspace(4)* %12, align 4
  %sub122 = sub nsw i32 %61, 1
  %sub123 = sub nsw i32 %sub122, 1
  %add124 = add nsw i32 %sub123, 1
  %div125 = sdiv i32 %add124, 1
  %mul126 = mul nsw i32 1, %div125
  %62 = load i32, i32 addrspace(4)* %13, align 4
  %sub127 = sub nsw i32 %62, 1
  %sub128 = sub nsw i32 %sub127, 1
  %add129 = add nsw i32 %sub128, 1
  %div130 = sdiv i32 %add129, 1
  %mul131 = mul nsw i32 %mul126, %div130
  %conv132 = sext i32 %mul131 to i64
  %mul133 = mul nsw i64 %div121, %conv132
  %sub134 = sub nsw i64 %57, %mul133
  %63 = load i32, i32 addrspace(4)* %13, align 4
  %sub135 = sub nsw i32 %63, 1
  %sub136 = sub nsw i32 %sub135, 1
  %add137 = add nsw i32 %sub136, 1
  %div138 = sdiv i32 %add137, 1
  %mul139 = mul nsw i32 1, %div138
  %conv140 = sext i32 %mul139 to i64
  %div141 = sdiv i64 %sub134, %conv140
  %64 = load i32, i32 addrspace(4)* %13, align 4
  %sub142 = sub nsw i32 %64, 1
  %sub143 = sub nsw i32 %sub142, 1
  %add144 = add nsw i32 %sub143, 1
  %div145 = sdiv i32 %add144, 1
  %mul146 = mul nsw i32 1, %div145
  %conv147 = sext i32 %mul146 to i64
  %mul148 = mul nsw i64 %div141, %conv147
  %sub149 = sub nsw i64 %sub109, %mul148
  %mul150 = mul nsw i64 %sub149, 1
  %add151 = add nsw i64 1, %mul150
  %conv152 = trunc i64 %add151 to i32
  store i32 %conv152, i32 addrspace(4)* %7, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %65 = load i64, i64 addrspace(4)* %17, align 8
  %add153 = add nsw i64 %65, 1
  store i64 %add153, i64 addrspace(4)* %17, align 8
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %37) [ "DIR.OMP.END.SIMD"() ]
  call void @llvm.directive.region.exit(token %36) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %land.lhs.true29, %land.lhs.true, %entry
  call void @llvm.directive.region.exit(token %32) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %31) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind optnone uwtable "contains-openmp-target"="true" "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!3}

!0 = !{i32 0, i32 2052, i32 85985690, !"kernel", i32 5, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{}
!3 = !{!"clang version 9.0.0"}
