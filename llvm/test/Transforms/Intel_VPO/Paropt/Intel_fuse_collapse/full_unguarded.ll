; REQUIRES: asserts

; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-fuse-collapse)' -vpo-paropt-loop-fuse-collapse --debug-only=vpo-paropt-loop-fuse-collapse -S %s 2>&1  | FileCheck --check-prefix=CHECK_DBG %s

; The test checks that there're 2 fusion candidates picked
; when these loops are not guarded (i.e. don't have OMP lb<ub precondition
; emitted by the frontend)

; Original code:
; int main() {
;         static constexpr int N = 1024;
;         static constexpr int B = 4;
; 
;         int a[B*N], b[B*(N+1)];
; #pragma omp target teams loop map(tofrom:a[:B*N],b[:B*(N+1)])
;         for (int i = 0; i < B; i++) {
;                 int j;
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

; CHECK_DBG: OMP fusion candidate
; CHECK_DBG: OMP fusion candidate

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

define protected noundef i32 @main() {
entry:
  %retval = alloca i32, align 4
  %a = alloca [4096 x i32], align 4
  %b = alloca [4100 x i32], align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %i = alloca i32, align 4
  %j = alloca i32, align 4
  %tmp2 = alloca i32, align 4
  %.omp.iv3 = alloca i32, align 4
  %.omp.lb4 = alloca i32, align 4
  %.omp.ub5 = alloca i32, align 4
  %tmp15 = alloca i32, align 4
  %.omp.iv16 = alloca i32, align 4
  %.omp.lb17 = alloca i32, align 4
  %.omp.ub18 = alloca i32, align 4
  %retval.ascast = addrspacecast ptr %retval to ptr addrspace(4)
  %a.ascast = addrspacecast ptr %a to ptr addrspace(4)
  %b.ascast = addrspacecast ptr %b to ptr addrspace(4)
  %.omp.lb.ascast = addrspacecast ptr %.omp.lb to ptr addrspace(4)
  %.omp.ub.ascast = addrspacecast ptr %.omp.ub to ptr addrspace(4)
  %tmp.ascast = addrspacecast ptr %tmp to ptr addrspace(4)
  %.omp.iv.ascast = addrspacecast ptr %.omp.iv to ptr addrspace(4)
  %i.ascast = addrspacecast ptr %i to ptr addrspace(4)
  %j.ascast = addrspacecast ptr %j to ptr addrspace(4)
  %tmp2.ascast = addrspacecast ptr %tmp2 to ptr addrspace(4)
  %.omp.iv3.ascast = addrspacecast ptr %.omp.iv3 to ptr addrspace(4)
  %.omp.lb4.ascast = addrspacecast ptr %.omp.lb4 to ptr addrspace(4)
  %.omp.ub5.ascast = addrspacecast ptr %.omp.ub5 to ptr addrspace(4)
  %tmp15.ascast = addrspacecast ptr %tmp15 to ptr addrspace(4)
  %.omp.iv16.ascast = addrspacecast ptr %.omp.iv16 to ptr addrspace(4)
  %.omp.lb17.ascast = addrspacecast ptr %.omp.lb17 to ptr addrspace(4)
  %.omp.ub18.ascast = addrspacecast ptr %.omp.ub18 to ptr addrspace(4)
  store i32 0, ptr addrspace(4) %retval.ascast, align 4
  store i32 0, ptr addrspace(4) %.omp.lb.ascast, align 4
  store i32 3, ptr addrspace(4) %.omp.ub.ascast, align 4
  %arrayidx = getelementptr inbounds [4096 x i32], ptr addrspace(4) %a.ascast, i64 0, i64 0
  %arrayidx1 = getelementptr inbounds [4100 x i32], ptr addrspace(4) %b.ascast, i64 0, i64 0
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %a.ascast, ptr addrspace(4) %arrayidx, i64 16384, i64 35, ptr null, ptr null), ; MAP type: 35 = 0x23 = TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %b.ascast, ptr addrspace(4) %arrayidx1, i64 16400, i64 35, ptr null, ptr null), ; MAP type: 35 = 0x23 = TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %j.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv3.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.lb4.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.ub5.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv16.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.lb17.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.ub18.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp2.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp15.ascast, i32 0, i32 1) ]

  %array.begin42 = getelementptr inbounds [4096 x i32], ptr addrspace(4) %a.ascast, i32 0, i32 0
  %array.begin43 = getelementptr inbounds [4100 x i32], ptr addrspace(4) %b.ascast, i32 0, i32 0
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %a.ascast, i32 0, i64 4096),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %b.ascast, i32 0, i64 4100),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %j.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv3.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.lb4.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.ub5.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv16.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.lb17.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.ub18.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp2.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp15.ascast, i32 0, i32 1) ]

  %array.begin40 = getelementptr inbounds [4096 x i32], ptr addrspace(4) %a.ascast, i32 0, i32 0
  %array.begin41 = getelementptr inbounds [4100 x i32], ptr addrspace(4) %b.ascast, i32 0, i32 0
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %a.ascast, i32 0, i64 4096),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %b.ascast, i32 0, i64 4100),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %j.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv3.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.lb4.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.ub5.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv16.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.lb17.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.ub18.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp2.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp15.ascast, i32 0, i32 1) ]

  %3 = load i32, ptr addrspace(4) %.omp.lb.ascast, align 4
  store i32 %3, ptr addrspace(4) %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc36, %entry
  %4 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %5 = load i32, ptr addrspace(4) %.omp.ub.ascast, align 4
  %cmp = icmp sle i32 %4, %5
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end38

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %6 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %mul = mul nsw i32 %6, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr addrspace(4) %i.ascast, align 4
  store i32 0, ptr addrspace(4) %.omp.lb4.ascast, align 4
  store i32 1023, ptr addrspace(4) %.omp.ub5.ascast, align 4
  %array.begin = getelementptr inbounds [4096 x i32], ptr addrspace(4) %a.ascast, i32 0, i32 0
  %7 = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %a.ascast, i32 0, i64 4096),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %j.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) %.omp.iv3.ascast, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb4.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) %.omp.ub5.ascast, i32 0) ]

  %8 = load i32, ptr addrspace(4) %.omp.lb4.ascast, align 4
  store i32 %8, ptr addrspace(4) %.omp.iv3.ascast, align 4
  br label %omp.inner.for.cond6

omp.inner.for.cond6:                              ; preds = %omp.inner.for.inc, %omp.inner.for.body
  %9 = load i32, ptr addrspace(4) %.omp.iv3.ascast, align 4
  %10 = load i32, ptr addrspace(4) %.omp.ub5.ascast, align 4
  %cmp7 = icmp sle i32 %9, %10
  br i1 %cmp7, label %omp.inner.for.body8, label %omp.inner.for.end

omp.inner.for.body8:                              ; preds = %omp.inner.for.cond6
  %11 = load i32, ptr addrspace(4) %.omp.iv3.ascast, align 4
  %mul9 = mul nsw i32 %11, 1
  %add10 = add nsw i32 0, %mul9
  store i32 %add10, ptr addrspace(4) %j.ascast, align 4
  %12 = load i32, ptr addrspace(4) %j.ascast, align 4
  %13 = load i32, ptr addrspace(4) %i.ascast, align 4
  %mul11 = mul nsw i32 %13, 1024
  %14 = load i32, ptr addrspace(4) %j.ascast, align 4
  %add12 = add nsw i32 %mul11, %14
  %idxprom = sext i32 %add12 to i64
  %arrayidx13 = getelementptr inbounds [4096 x i32], ptr addrspace(4) %a.ascast, i64 0, i64 %idxprom
  store i32 %12, ptr addrspace(4) %arrayidx13, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body8
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %15 = load i32, ptr addrspace(4) %.omp.iv3.ascast, align 4
  %add14 = add nsw i32 %15, 1
  store i32 %add14, ptr addrspace(4) %.omp.iv3.ascast, align 4
  br label %omp.inner.for.cond6

omp.inner.for.end:                                ; preds = %omp.inner.for.cond6
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %7) [ "DIR.OMP.END.GENERICLOOP"() ]

  store i32 0, ptr addrspace(4) %.omp.lb17.ascast, align 4
  store i32 1024, ptr addrspace(4) %.omp.ub18.ascast, align 4
  %array.begin34 = getelementptr inbounds [4100 x i32], ptr addrspace(4) %b.ascast, i32 0, i32 0
  %16 = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %b.ascast, i32 0, i64 4100),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %j.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) %.omp.iv16.ascast, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb17.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) %.omp.ub18.ascast, i32 0) ]

  %17 = load i32, ptr addrspace(4) %.omp.lb17.ascast, align 4
  store i32 %17, ptr addrspace(4) %.omp.iv16.ascast, align 4
  br label %omp.inner.for.cond19

omp.inner.for.cond19:                             ; preds = %omp.inner.for.inc30, %omp.loop.exit
  %18 = load i32, ptr addrspace(4) %.omp.iv16.ascast, align 4
  %19 = load i32, ptr addrspace(4) %.omp.ub18.ascast, align 4
  %cmp20 = icmp sle i32 %18, %19
  br i1 %cmp20, label %omp.inner.for.body21, label %omp.inner.for.end32

omp.inner.for.body21:                             ; preds = %omp.inner.for.cond19
  %20 = load i32, ptr addrspace(4) %.omp.iv16.ascast, align 4
  %mul22 = mul nsw i32 %20, 1
  %add23 = add nsw i32 0, %mul22
  store i32 %add23, ptr addrspace(4) %j.ascast, align 4
  %21 = load i32, ptr addrspace(4) %j.ascast, align 4
  %mul24 = mul nsw i32 %21, 2
  %22 = load i32, ptr addrspace(4) %i.ascast, align 4
  %mul25 = mul nsw i32 %22, 1025
  %23 = load i32, ptr addrspace(4) %j.ascast, align 4
  %add26 = add nsw i32 %mul25, %23
  %idxprom27 = sext i32 %add26 to i64
  %arrayidx28 = getelementptr inbounds [4100 x i32], ptr addrspace(4) %b.ascast, i64 0, i64 %idxprom27
  store i32 %mul24, ptr addrspace(4) %arrayidx28, align 4
  br label %omp.body.continue29

omp.body.continue29:                              ; preds = %omp.inner.for.body21
  br label %omp.inner.for.inc30

omp.inner.for.inc30:                              ; preds = %omp.body.continue29
  %24 = load i32, ptr addrspace(4) %.omp.iv16.ascast, align 4
  %add31 = add nsw i32 %24, 1
  store i32 %add31, ptr addrspace(4) %.omp.iv16.ascast, align 4
  br label %omp.inner.for.cond19

omp.inner.for.end32:                              ; preds = %omp.inner.for.cond19
  br label %omp.loop.exit33

omp.loop.exit33:                                  ; preds = %omp.inner.for.end32
  call void @llvm.directive.region.exit(token %16) [ "DIR.OMP.END.GENERICLOOP"() ]

  br label %omp.body.continue35

omp.body.continue35:                              ; preds = %omp.loop.exit33
  br label %omp.inner.for.inc36

omp.inner.for.inc36:                              ; preds = %omp.body.continue35
  %25 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %add37 = add nsw i32 %25, 1
  store i32 %add37, ptr addrspace(4) %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.end38:                              ; preds = %omp.inner.for.cond
  br label %omp.loop.exit39

omp.loop.exit39:                                  ; preds = %omp.inner.for.end38
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.GENERICLOOP"() ]

  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TEAMS"() ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]

  ret i32 0
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

!omp_offload.info = !{!0}

!0 = !{i32 0, i32 2050, i32 49060407, !"_Z4main", i32 8, i32 0, i32 0, i32 0}
