; REQUIRES: asserts

; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-fuse-collapse)' -vpo-paropt-loop-fuse-collapse --debug-only=vpo-paropt-loop-fuse-collapse -S %s 2>&1  | FileCheck %s

; Original code:
; int main() {
;         static constexpr int N = 1024;
;         static constexpr int B = 4;
; 
;         int a[B*N], b[B*(N+1)];
; #pragma omp target teams loop map(tofrom:a[:B*N],b[:B*(N+1)])
;         for (int i = 0; i < B; i++) {
;                 int j;
;                 b[1] = 1;
; #pragma omp loop
;                 for (j = 0; j < N; j++)
;                         a[i*N+j] = j;
; #pragma omp loop
;                 for (j = 0; j < N+1; j++)
;                         b[i*(N+1)+j] = j*2;
;         }
; 
;         return 0;
; }

; This test checks that the pass backs off because of the intra-nest code preceding the pair of loops to be fused
; and outputs correct debug message describing it

; CHECK: Candidate pairs fusion check failed

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

define protected noundef i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %a = alloca [4096 x i32], align 4
  %b = alloca [4100 x i32], align 4
  %N0 = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %i = alloca i32, align 4
  %j = alloca i32, align 4
  %tmp3 = alloca i32, align 4
  %.omp.iv4 = alloca i32, align 4
  %.omp.lb5 = alloca i32, align 4
  %.omp.ub6 = alloca i32, align 4
  %tmp17 = alloca i32, align 4
  %.omp.iv18 = alloca i32, align 4
  %.omp.lb19 = alloca i32, align 4
  %.omp.ub20 = alloca i32, align 4
  %retval.ascast = addrspacecast ptr %retval to ptr addrspace(4)
  %a.ascast = addrspacecast ptr %a to ptr addrspace(4)
  %b.ascast = addrspacecast ptr %b to ptr addrspace(4)
  %N0.ascast = addrspacecast ptr %N0 to ptr addrspace(4)
  %.omp.lb.ascast = addrspacecast ptr %.omp.lb to ptr addrspace(4)
  %.omp.ub.ascast = addrspacecast ptr %.omp.ub to ptr addrspace(4)
  %tmp.ascast = addrspacecast ptr %tmp to ptr addrspace(4)
  %.omp.iv.ascast = addrspacecast ptr %.omp.iv to ptr addrspace(4)
  %i.ascast = addrspacecast ptr %i to ptr addrspace(4)
  %j.ascast = addrspacecast ptr %j to ptr addrspace(4)
  %tmp3.ascast = addrspacecast ptr %tmp3 to ptr addrspace(4)
  %.omp.iv4.ascast = addrspacecast ptr %.omp.iv4 to ptr addrspace(4)
  %.omp.lb5.ascast = addrspacecast ptr %.omp.lb5 to ptr addrspace(4)
  %.omp.ub6.ascast = addrspacecast ptr %.omp.ub6 to ptr addrspace(4)
  %tmp17.ascast = addrspacecast ptr %tmp17 to ptr addrspace(4)
  %.omp.iv18.ascast = addrspacecast ptr %.omp.iv18 to ptr addrspace(4)
  %.omp.lb19.ascast = addrspacecast ptr %.omp.lb19 to ptr addrspace(4)
  %.omp.ub20.ascast = addrspacecast ptr %.omp.ub20 to ptr addrspace(4)
  store i32 0, ptr addrspace(4) %retval.ascast, align 4
  store i32 1024, ptr addrspace(4) %N0.ascast, align 4
  store i32 0, ptr addrspace(4) %.omp.lb.ascast, align 4
  store i32 3, ptr addrspace(4) %.omp.ub.ascast, align 4
  %arrayidx = getelementptr inbounds [4100 x i32], ptr addrspace(4) %b.ascast, i64 0, i64 0
  %arrayidx1 = getelementptr inbounds [4096 x i32], ptr addrspace(4) %a.ascast, i64 0, i64 0
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %b.ascast, ptr addrspace(4) %arrayidx, i64 16400, i64 35, ptr null, ptr null), ; MAP type: 35 = 0x23 = TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %a.ascast, ptr addrspace(4) %arrayidx1, i64 16384, i64 35, ptr null, ptr null), ; MAP type: 35 = 0x23 = TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %N0.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %j.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv4.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.lb5.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.ub6.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv18.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.lb19.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.ub20.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp3.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp17.ascast, i32 0, i32 1) ]

  %array.begin49 = getelementptr inbounds [4096 x i32], ptr addrspace(4) %a.ascast, i32 0, i32 0
  %array.begin50 = getelementptr inbounds [4100 x i32], ptr addrspace(4) %b.ascast, i32 0, i32 0
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %a.ascast, i32 0, i64 4096),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %b.ascast, i32 0, i64 4100),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %N0.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %j.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv4.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.lb5.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.ub6.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv18.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.lb19.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.ub20.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp3.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp17.ascast, i32 0, i32 1) ]

  %array.begin47 = getelementptr inbounds [4096 x i32], ptr addrspace(4) %a.ascast, i32 0, i32 0
  %array.begin48 = getelementptr inbounds [4100 x i32], ptr addrspace(4) %b.ascast, i32 0, i32 0
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %a.ascast, i32 0, i64 4096),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %b.ascast, i32 0, i64 4100),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %N0.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %j.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv4.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.lb5.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.ub6.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv18.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.lb19.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.ub20.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp3.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp17.ascast, i32 0, i32 1) ]

  %3 = load i32, ptr addrspace(4) %.omp.lb.ascast, align 4
  store i32 %3, ptr addrspace(4) %.omp.iv.ascast, align 4
  %4 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %5 = load i32, ptr addrspace(4) %.omp.ub.ascast, align 4
  %cmp = icmp sle i32 %4, %5
  br i1 %cmp, label %omp.inner.for.body.lh, label %omp.inner.for.end45

omp.inner.for.body.lh:                            ; preds = %entry
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.inc41, %omp.inner.for.body.lh
  %6 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %mul = mul nsw i32 %6, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr addrspace(4) %i.ascast, align 4
  %arrayidx2 = getelementptr inbounds [4100 x i32], ptr addrspace(4) %b.ascast, i64 0, i64 1
  store i32 1, ptr addrspace(4) %arrayidx2, align 4
  store i32 0, ptr addrspace(4) %.omp.lb5.ascast, align 4
  store i32 1023, ptr addrspace(4) %.omp.ub6.ascast, align 4
  %array.begin = getelementptr inbounds [4096 x i32], ptr addrspace(4) %a.ascast, i32 0, i32 0
  %7 = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %a.ascast, i32 0, i64 4096),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %N0.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %j.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) %.omp.iv4.ascast, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb5.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) %.omp.ub6.ascast, i32 0) ]

  %8 = load i32, ptr addrspace(4) %.omp.lb5.ascast, align 4
  store i32 %8, ptr addrspace(4) %.omp.iv4.ascast, align 4
  %9 = load i32, ptr addrspace(4) %.omp.iv4.ascast, align 4
  %10 = load i32, ptr addrspace(4) %.omp.ub6.ascast, align 4
  %cmp7 = icmp sle i32 %9, %10
  br i1 %cmp7, label %omp.inner.for.body.lh8, label %omp.inner.for.end

omp.inner.for.body.lh8:                           ; preds = %omp.inner.for.body
  br label %omp.inner.for.body9

omp.inner.for.body9:                              ; preds = %omp.inner.for.inc, %omp.inner.for.body.lh8
  %11 = load i32, ptr addrspace(4) %.omp.iv4.ascast, align 4
  %mul10 = mul nsw i32 %11, 1
  %add11 = add nsw i32 0, %mul10
  store i32 %add11, ptr addrspace(4) %j.ascast, align 4
  %12 = load i32, ptr addrspace(4) %j.ascast, align 4
  %13 = load i32, ptr addrspace(4) %i.ascast, align 4
  %14 = load i32, ptr addrspace(4) %N0.ascast, align 4
  %mul12 = mul nsw i32 %13, %14
  %15 = load i32, ptr addrspace(4) %j.ascast, align 4
  %add13 = add nsw i32 %mul12, %15
  %idxprom = sext i32 %add13 to i64
  %arrayidx14 = getelementptr inbounds [4096 x i32], ptr addrspace(4) %a.ascast, i64 0, i64 %idxprom
  store i32 %12, ptr addrspace(4) %arrayidx14, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body9
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %16 = load i32, ptr addrspace(4) %.omp.iv4.ascast, align 4
  %add15 = add nsw i32 %16, 1
  store i32 %add15, ptr addrspace(4) %.omp.iv4.ascast, align 4
  %17 = load i32, ptr addrspace(4) %.omp.iv4.ascast, align 4
  %18 = load i32, ptr addrspace(4) %.omp.ub6.ascast, align 4
  %cmp16 = icmp sle i32 %17, %18
  br i1 %cmp16, label %omp.inner.for.body9, label %omp.inner.for.end_crit_edge

omp.inner.for.end_crit_edge:                      ; preds = %omp.inner.for.inc
  br label %omp.inner.for.end

omp.inner.for.end:                                ; preds = %omp.inner.for.end_crit_edge, %omp.inner.for.body
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %7) [ "DIR.OMP.END.GENERICLOOP"() ]

  store i32 0, ptr addrspace(4) %.omp.lb19.ascast, align 4
  store i32 1024, ptr addrspace(4) %.omp.ub20.ascast, align 4
  %array.begin39 = getelementptr inbounds [4100 x i32], ptr addrspace(4) %b.ascast, i32 0, i32 0
  %19 = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %b.ascast, i32 0, i64 4100),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %N0.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %j.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) %.omp.iv18.ascast, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb19.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) %.omp.ub20.ascast, i32 0) ]

  %20 = load i32, ptr addrspace(4) %.omp.lb19.ascast, align 4
  store i32 %20, ptr addrspace(4) %.omp.iv18.ascast, align 4
  %21 = load i32, ptr addrspace(4) %.omp.iv18.ascast, align 4
  %22 = load i32, ptr addrspace(4) %.omp.ub20.ascast, align 4
  %cmp21 = icmp sle i32 %21, %22
  br i1 %cmp21, label %omp.inner.for.body.lh22, label %omp.inner.for.end37

omp.inner.for.body.lh22:                          ; preds = %omp.loop.exit
  br label %omp.inner.for.body23

omp.inner.for.body23:                             ; preds = %omp.inner.for.inc33, %omp.inner.for.body.lh22
  %23 = load i32, ptr addrspace(4) %.omp.iv18.ascast, align 4
  %mul24 = mul nsw i32 %23, 1
  %add25 = add nsw i32 0, %mul24
  store i32 %add25, ptr addrspace(4) %j.ascast, align 4
  %24 = load i32, ptr addrspace(4) %j.ascast, align 4
  %mul26 = mul nsw i32 %24, 2
  %25 = load i32, ptr addrspace(4) %i.ascast, align 4
  %26 = load i32, ptr addrspace(4) %N0.ascast, align 4
  %add27 = add nsw i32 %26, 1
  %mul28 = mul nsw i32 %25, %add27
  %27 = load i32, ptr addrspace(4) %j.ascast, align 4
  %add29 = add nsw i32 %mul28, %27
  %idxprom30 = sext i32 %add29 to i64
  %arrayidx31 = getelementptr inbounds [4100 x i32], ptr addrspace(4) %b.ascast, i64 0, i64 %idxprom30
  store i32 %mul26, ptr addrspace(4) %arrayidx31, align 4
  br label %omp.body.continue32

omp.body.continue32:                              ; preds = %omp.inner.for.body23
  br label %omp.inner.for.inc33

omp.inner.for.inc33:                              ; preds = %omp.body.continue32
  %28 = load i32, ptr addrspace(4) %.omp.iv18.ascast, align 4
  %add34 = add nsw i32 %28, 1
  store i32 %add34, ptr addrspace(4) %.omp.iv18.ascast, align 4
  %29 = load i32, ptr addrspace(4) %.omp.iv18.ascast, align 4
  %30 = load i32, ptr addrspace(4) %.omp.ub20.ascast, align 4
  %cmp35 = icmp sle i32 %29, %30
  br i1 %cmp35, label %omp.inner.for.body23, label %omp.inner.for.end_crit_edge36

omp.inner.for.end_crit_edge36:                    ; preds = %omp.inner.for.inc33
  br label %omp.inner.for.end37

omp.inner.for.end37:                              ; preds = %omp.inner.for.end_crit_edge36, %omp.loop.exit
  br label %omp.loop.exit38

omp.loop.exit38:                                  ; preds = %omp.inner.for.end37
  call void @llvm.directive.region.exit(token %19) [ "DIR.OMP.END.GENERICLOOP"() ]

  br label %omp.body.continue40

omp.body.continue40:                              ; preds = %omp.loop.exit38
  br label %omp.inner.for.inc41

omp.inner.for.inc41:                              ; preds = %omp.body.continue40
  %31 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %add42 = add nsw i32 %31, 1
  store i32 %add42, ptr addrspace(4) %.omp.iv.ascast, align 4
  %32 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %33 = load i32, ptr addrspace(4) %.omp.ub.ascast, align 4
  %cmp43 = icmp sle i32 %32, %33
  br i1 %cmp43, label %omp.inner.for.body, label %omp.inner.for.end_crit_edge44

omp.inner.for.end_crit_edge44:                    ; preds = %omp.inner.for.inc41
  br label %omp.inner.for.end45

omp.inner.for.end45:                              ; preds = %omp.inner.for.end_crit_edge44, %entry
  br label %omp.loop.exit46

omp.loop.exit46:                                  ; preds = %omp.inner.for.end45
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.GENERICLOOP"() ]

  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TEAMS"() ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]

  ret i32 0
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

attributes #0 = { "contains-openmp-target"="true" }

!omp_offload.info = !{!0}

!0 = !{i32 0, i32 2050, i32 49129119, !"_Z4main", i32 9, i32 0, i32 0, i32 0}
