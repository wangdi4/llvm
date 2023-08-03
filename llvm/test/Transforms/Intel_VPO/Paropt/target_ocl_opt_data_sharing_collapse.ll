; RUN: opt -bugpoint-enable-legacy-pm -switch-to-offload -vpo-paropt-optimize-data-sharing -S %s | FileCheck %s
; RUN: opt -switch-to-offload -passes='vpo-paropt-optimize-data-sharing' -S %s | FileCheck %s
;
; Original code:
;
; void foo(float (*P)[4][4], int N) {
; #pragma omp target teams distribute map(from: P[0:N])
;   for (int Z = 0; Z < N; ++Z) {
; #pragma omp parallel for collapse(2)
;     for (int Y = 0; Y < 4; ++Y)
;       for (int X = 0; X < 4; ++X)
;         P[Z][Y][X] = X + Y;
;   }
; }
;
; IR was captured before optimize data sharing pass.
;
; Check that collapsed LB and UB listed in one private clause on distribute
; region are made WI private.

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

define hidden spir_func void @foo(ptr addrspace(4) %P, i32 %N) {
; CHECK-LABEL: define hidden spir_func void @foo(
entry:
  %omp.collapsed.iv = alloca i64, align 8
  %omp.collapsed.iv.ascast = addrspacecast ptr %omp.collapsed.iv to ptr addrspace(4)
  %omp.collapsed.lb = alloca i64, align 8
  %omp.collapsed.lb.ascast = addrspacecast ptr %omp.collapsed.lb to ptr addrspace(4)
  %omp.collapsed.ub = alloca i64, align 8
  %omp.collapsed.ub.ascast = addrspacecast ptr %omp.collapsed.ub to ptr addrspace(4)
  %P.addr = alloca ptr addrspace(4), align 8
  %P.addr.ascast = addrspacecast ptr %P.addr to ptr addrspace(4)
  %N.addr = alloca i32, align 4
  %N.addr.ascast = addrspacecast ptr %N.addr to ptr addrspace(4)
  %.capture_expr.0 = alloca i32, align 4
  %.capture_expr.0.ascast = addrspacecast ptr %.capture_expr.0 to ptr addrspace(4)
  %.capture_expr.1 = alloca i32, align 4
  %.capture_expr.1.ascast = addrspacecast ptr %.capture_expr.1 to ptr addrspace(4)
  %.omp.lb = alloca i32, align 4
  %.omp.lb.ascast = addrspacecast ptr %.omp.lb to ptr addrspace(4)
  %.omp.ub = alloca i32, align 4
  %.omp.ub.ascast = addrspacecast ptr %.omp.ub to ptr addrspace(4)
  %P.map.ptr.tmp = alloca ptr addrspace(4), align 8
  %P.map.ptr.tmp.ascast = addrspacecast ptr %P.map.ptr.tmp to ptr addrspace(4)
  %tmp = alloca i32, align 4
  %tmp.ascast = addrspacecast ptr %tmp to ptr addrspace(4)
  %.omp.iv = alloca i32, align 4
  %.omp.iv.ascast = addrspacecast ptr %.omp.iv to ptr addrspace(4)
  %Z = alloca i32, align 4
  %Z.ascast = addrspacecast ptr %Z to ptr addrspace(4)
  %tmp7 = alloca i32, align 4
  %tmp7.ascast = addrspacecast ptr %tmp7 to ptr addrspace(4)
  %tmp8 = alloca i32, align 4
  %tmp8.ascast = addrspacecast ptr %tmp8 to ptr addrspace(4)
  %.omp.uncollapsed.iv = alloca i32, align 4
  %.omp.uncollapsed.iv.ascast = addrspacecast ptr %.omp.uncollapsed.iv to ptr addrspace(4)
  %.omp.uncollapsed.iv9 = alloca i32, align 4
  %.omp.uncollapsed.iv9.ascast = addrspacecast ptr %.omp.uncollapsed.iv9 to ptr addrspace(4)
  %.omp.uncollapsed.lb = alloca i32, align 4
  %.omp.uncollapsed.lb.ascast = addrspacecast ptr %.omp.uncollapsed.lb to ptr addrspace(4)
  %.omp.uncollapsed.ub = alloca i32, align 4
  %.omp.uncollapsed.ub.ascast = addrspacecast ptr %.omp.uncollapsed.ub to ptr addrspace(4)
  %.omp.uncollapsed.lb10 = alloca i32, align 4
  %.omp.uncollapsed.lb10.ascast = addrspacecast ptr %.omp.uncollapsed.lb10 to ptr addrspace(4)
  %.omp.uncollapsed.ub11 = alloca i32, align 4
  %.omp.uncollapsed.ub11.ascast = addrspacecast ptr %.omp.uncollapsed.ub11 to ptr addrspace(4)
  %Y = alloca i32, align 4
  %Y.ascast = addrspacecast ptr %Y to ptr addrspace(4)
  %X = alloca i32, align 4
  %X.ascast = addrspacecast ptr %X to ptr addrspace(4)
  store ptr addrspace(4) %P, ptr addrspace(4) %P.addr.ascast, align 8
  store i32 %N, ptr addrspace(4) %N.addr.ascast, align 4
  %0 = load i32, ptr addrspace(4) %N.addr.ascast, align 4
  store i32 %0, ptr addrspace(4) %.capture_expr.0.ascast, align 4
  %1 = load i32, ptr addrspace(4) %.capture_expr.0.ascast, align 4
  %sub = sub nsw i32 %1, 0
  %sub1 = sub nsw i32 %sub, 1
  %add = add nsw i32 %sub1, 1
  %div = sdiv i32 %add, 1
  %sub2 = sub nsw i32 %div, 1
  store i32 %sub2, ptr addrspace(4) %.capture_expr.1.ascast, align 4
  store i32 0, ptr addrspace(4) %.omp.lb.ascast, align 4
  %2 = load i32, ptr addrspace(4) %.capture_expr.1.ascast, align 4
  store volatile i32 %2, ptr addrspace(4) %.omp.ub.ascast, align 4
  %3 = load ptr addrspace(4), ptr addrspace(4) %P.addr.ascast, align 8
  %4 = load ptr addrspace(4), ptr addrspace(4) %P.addr.ascast, align 8
  %5 = load ptr addrspace(4), ptr addrspace(4) %P.addr.ascast, align 8
  %6 = load i32, ptr addrspace(4) %N.addr.ascast, align 4
  %conv = sext i32 %6 to i64
  %7 = mul nuw i64 %conv, 64
  br label %DIR.OMP.TARGET.1

DIR.OMP.TARGET.1:                                 ; preds = %entry
  br label %DIR.OMP.TARGET.2

DIR.OMP.TARGET.2:                                 ; preds = %DIR.OMP.TARGET.1
  br label %DIR.OMP.TARGET.14

DIR.OMP.TARGET.14:                                ; preds = %DIR.OMP.TARGET.2
  br label %DIR.OMP.TARGET.14.split

DIR.OMP.TARGET.14.split:                          ; preds = %DIR.OMP.TARGET.14
  %end.dir.temp122 = alloca i1, align 1
  br label %DIR.OMP.TARGET.1125

DIR.OMP.TARGET.1125:                              ; preds = %DIR.OMP.TARGET.14.split
  br label %DIR.OMP.TARGET.2126

DIR.OMP.TARGET.2126:                              ; preds = %DIR.OMP.TARGET.1125
  %8 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.FROM"(ptr addrspace(4) %4, ptr addrspace(4) %5, i64 %7, i64 34, ptr null, ptr null), ; MAP type: 34 = 0x22 = TARGET_PARAM (0x20) | FROM (0x2)
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %Z.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.capture_expr.0.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %Y.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.iv9.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.lb10.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.ub11.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %X.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %P.map.ptr.tmp.ascast, ptr addrspace(4) null, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp7.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp8.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:WILOCAL"(ptr addrspace(4) %omp.collapsed.lb.ascast, ptr addrspace(4) %omp.collapsed.ub.ascast),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %omp.collapsed.iv.ascast, i64 0, i32 1),
    "QUAL.OMP.OFFLOAD.NDRANGE"(ptr addrspace(4) %.omp.ub.ascast, i32 0),
    "QUAL.OMP.JUMP.TO.END.IF"(ptr %end.dir.temp122) ]

  br label %DIR.OMP.TARGET.3127

DIR.OMP.TARGET.3127:                              ; preds = %DIR.OMP.TARGET.2126
  %temp.load123 = load volatile i1, ptr %end.dir.temp122, align 1
  %cmp124 = icmp ne i1 %temp.load123, false
  br i1 %cmp124, label %DIR.OMP.END.TEAMS.13.split, label %9

9:                                                ; preds = %DIR.OMP.TARGET.3127
  br label %DIR.OMP.TARGET.3

DIR.OMP.TARGET.3:                                 ; preds = %9
  store ptr addrspace(4) %4, ptr addrspace(4) %P.map.ptr.tmp.ascast, align 8
  br label %DIR.OMP.TEAMS.4

DIR.OMP.TEAMS.4:                                  ; preds = %DIR.OMP.TARGET.3
  br label %DIR.OMP.TEAMS.5

DIR.OMP.TEAMS.5:                                  ; preds = %DIR.OMP.TEAMS.4
  br label %DIR.OMP.TEAMS.2

DIR.OMP.TEAMS.2:                                  ; preds = %DIR.OMP.TEAMS.5
  br label %DIR.OMP.TEAMS.2.split

DIR.OMP.TEAMS.2.split:                            ; preds = %DIR.OMP.TEAMS.2
  %end.dir.temp119 = alloca i1, align 1
  br label %DIR.OMP.TEAMS.4128

DIR.OMP.TEAMS.4128:                               ; preds = %DIR.OMP.TEAMS.2.split
  br label %DIR.OMP.TEAMS.5129

DIR.OMP.TEAMS.5129:                               ; preds = %DIR.OMP.TEAMS.4128
  %10 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %P.map.ptr.tmp.ascast, ptr addrspace(4) null, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %Z.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %.capture_expr.0.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %Y.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.iv9.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.lb10.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.ub11.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %X.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp7.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp8.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED.WILOCAL"(ptr addrspace(4) %omp.collapsed.lb.ascast, i64 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED.WILOCAL"(ptr addrspace(4) %omp.collapsed.ub.ascast, i64 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %omp.collapsed.iv.ascast, i64 0, i32 1),
    "QUAL.OMP.JUMP.TO.END.IF"(ptr %end.dir.temp119) ]

  br label %DIR.OMP.TEAMS.6130

DIR.OMP.TEAMS.6130:                               ; preds = %DIR.OMP.TEAMS.5129
  %temp.load120 = load volatile i1, ptr %end.dir.temp119, align 1
  %cmp121 = icmp ne i1 %temp.load120, false
  br i1 %cmp121, label %DIR.OMP.END.TEAMS.12.split, label %11

11:                                               ; preds = %DIR.OMP.TEAMS.6130
  br label %DIR.OMP.TEAMS.6

DIR.OMP.TEAMS.6:                                  ; preds = %11
  %12 = load i32, ptr addrspace(4) %.capture_expr.0.ascast, align 4
  %cmp = icmp slt i32 0, %12
  br i1 %cmp, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %DIR.OMP.TEAMS.6
  br label %omp.precond.then.split

omp.precond.then.split:                           ; preds = %omp.precond.then
  %end.dir.temp116 = alloca i1, align 1
  br label %DIR.OMP.DISTRIBUTE.7131

DIR.OMP.DISTRIBUTE.7131:                          ; preds = %omp.precond.then.split
  %13 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE"(),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %Z.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %Y.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.iv9.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.lb10.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.ub11.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %X.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp7.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp8.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %omp.collapsed.lb.ascast, i64 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %omp.collapsed.ub.ascast, i64 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %omp.collapsed.iv.ascast, i64 0, i32 1),
    "QUAL.OMP.OFFLOAD.KNOWN.NDRANGE"(i1 true),
    "QUAL.OMP.JUMP.TO.END.IF"(ptr %end.dir.temp116) ]

; CHECK: call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE"(),
; CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED.WILOCAL"(ptr addrspace(4) %omp.collapsed.lb.ascast, i64 0, i32 1)
; CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED.WILOCAL"(ptr addrspace(4) %omp.collapsed.ub.ascast, i64 0, i32 1)
  br label %DIR.OMP.DISTRIBUTE.8

DIR.OMP.DISTRIBUTE.8:                             ; preds = %DIR.OMP.DISTRIBUTE.7131
  %temp.load117 = load volatile i1, ptr %end.dir.temp116, align 1
  %cmp118 = icmp ne i1 %temp.load117, false
  br i1 %cmp118, label %omp.loop.exit.split, label %14

14:                                               ; preds = %DIR.OMP.DISTRIBUTE.8
  br label %DIR.OMP.DISTRIBUTE.7

DIR.OMP.DISTRIBUTE.7:                             ; preds = %14
  %15 = load i32, ptr addrspace(4) %.omp.lb.ascast, align 4
  store volatile i32 %15, ptr addrspace(4) %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %DIR.OMP.END.PARALLEL.LOOP.11, %DIR.OMP.DISTRIBUTE.7
  %16 = load volatile i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %17 = load volatile i32, ptr addrspace(4) %.omp.ub.ascast, align 4
  %cmp4 = icmp sle i32 %16, %17
  br i1 %cmp4, label %omp.inner.for.body, label %omp.loop.exit

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %18 = load volatile i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %mul = mul nsw i32 %18, 1
  %add6 = add nsw i32 0, %mul
  store i32 %add6, ptr addrspace(4) %Z.ascast, align 4
  store i32 0, ptr addrspace(4) %.omp.uncollapsed.lb.ascast, align 4
  store i32 3, ptr addrspace(4) %.omp.uncollapsed.ub.ascast, align 4
  store i32 0, ptr addrspace(4) %.omp.uncollapsed.lb10.ascast, align 4
  store i32 3, ptr addrspace(4) %.omp.uncollapsed.ub11.ascast, align 4
  br label %DIR.OMP.PARALLEL.LOOP.8

DIR.OMP.PARALLEL.LOOP.8:                          ; preds = %omp.inner.for.body
  br label %DIR.OMP.PARALLEL.LOOP.9

DIR.OMP.PARALLEL.LOOP.9:                          ; preds = %DIR.OMP.PARALLEL.LOOP.8
  %.omp.uncollapsed.ub.ascast.val = load i32, ptr addrspace(4) %.omp.uncollapsed.ub.ascast, align 4
  %.zext = zext i32 %.omp.uncollapsed.ub.ascast.val to i64
  %19 = add nuw nsw i64 %.zext, 1
  %.omp.uncollapsed.ub11.ascast.val = load i32, ptr addrspace(4) %.omp.uncollapsed.ub11.ascast, align 4
  %.zext1 = zext i32 %.omp.uncollapsed.ub11.ascast.val to i64
  %20 = add nuw nsw i64 %.zext1, 1
  %21 = mul nuw nsw i64 %19, %20
  %omp.collapsed.ub.value = sub nuw nsw i64 %21, 1
  store i64 0, ptr addrspace(4) %omp.collapsed.lb.ascast, align 8
  store volatile i64 %omp.collapsed.ub.value, ptr addrspace(4) %omp.collapsed.ub.ascast, align 8
  br label %DIR.OMP.PARALLEL.LOOP.3

DIR.OMP.PARALLEL.LOOP.3:                          ; preds = %DIR.OMP.PARALLEL.LOOP.9
  br label %DIR.OMP.PARALLEL.LOOP.4

DIR.OMP.PARALLEL.LOOP.4:                          ; preds = %DIR.OMP.PARALLEL.LOOP.3
  br label %DIR.OMP.PARALLEL.LOOP.4.split

DIR.OMP.PARALLEL.LOOP.4.split:                    ; preds = %DIR.OMP.PARALLEL.LOOP.4
  %end.dir.temp = alloca i1, align 1
  br label %DIR.OMP.PARALLEL.LOOP.9132

DIR.OMP.PARALLEL.LOOP.9132:                       ; preds = %DIR.OMP.PARALLEL.LOOP.4.split
  br label %DIR.OMP.PARALLEL.LOOP.10133

DIR.OMP.PARALLEL.LOOP.10133:                      ; preds = %DIR.OMP.PARALLEL.LOOP.9132
  %22 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.COLLAPSE"(i32 2),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %P.map.ptr.tmp.ascast, ptr addrspace(4) null, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %Z.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %Y.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.lb10.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %X.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) %omp.collapsed.iv.ascast, i64 0),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) %omp.collapsed.ub.ascast, i64 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.iv9.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %omp.collapsed.lb.ascast, i64 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.JUMP.TO.END.IF"(ptr %end.dir.temp) ]

  br label %DIR.OMP.PARALLEL.LOOP.11

DIR.OMP.PARALLEL.LOOP.11:                         ; preds = %DIR.OMP.PARALLEL.LOOP.10133
  %temp.load = load volatile i1, ptr %end.dir.temp, align 1
  %cmp115 = icmp ne i1 %temp.load, false
  br i1 %cmp115, label %omp.collapsed.loop.postexit.split, label %23

23:                                               ; preds = %DIR.OMP.PARALLEL.LOOP.11
  br label %DIR.OMP.PARALLEL.LOOP.9.split

DIR.OMP.PARALLEL.LOOP.9.split:                    ; preds = %23
  %.omp.uncollapsed.ub11.ascast.val2 = load i32, ptr addrspace(4) %.omp.uncollapsed.ub11.ascast, align 4
  %.zext3 = zext i32 %.omp.uncollapsed.ub11.ascast.val2 to i64
  %24 = add nuw nsw i64 %.zext3, 1
  %25 = mul nuw nsw i64 %24, 1
  br label %DIR.OMP.PARALLEL.LOOP.10

DIR.OMP.PARALLEL.LOOP.10:                         ; preds = %DIR.OMP.PARALLEL.LOOP.9.split
  %26 = load i32, ptr addrspace(4) %.omp.uncollapsed.lb.ascast, align 4
  store i32 %26, ptr addrspace(4) %.omp.uncollapsed.iv.ascast, align 4
  br label %DIR.OMP.PARALLEL.LOOP.10.split

DIR.OMP.PARALLEL.LOOP.10.split:                   ; preds = %DIR.OMP.PARALLEL.LOOP.10
  %27 = load i64, ptr addrspace(4) %omp.collapsed.lb.ascast, align 8
  store volatile i64 %27, ptr addrspace(4) %omp.collapsed.iv.ascast, align 8
  br label %omp.collapsed.loop.cond

omp.collapsed.loop.cond:                          ; preds = %omp.collapsed.loop.inc, %DIR.OMP.PARALLEL.LOOP.10.split
  %28 = load volatile i64, ptr addrspace(4) %omp.collapsed.iv.ascast, align 8
  %29 = load volatile i64, ptr addrspace(4) %omp.collapsed.ub.ascast, align 8
  %30 = icmp sle i64 %28, %29
  br i1 %30, label %omp.collapsed.loop.body, label %omp.collapsed.loop.exit

omp.collapsed.loop.body:                          ; preds = %omp.collapsed.loop.cond
  %omp.collapsed.iv.ascast.val = load volatile i64, ptr addrspace(4) %omp.collapsed.iv.ascast, align 8
  %31 = sdiv i64 %omp.collapsed.iv.ascast.val, %25
  %32 = trunc i64 %31 to i32
  store i32 %32, ptr addrspace(4) %.omp.uncollapsed.lb.ascast, align 4
  %33 = trunc i64 %31 to i32
  store i32 %33, ptr addrspace(4) %.omp.uncollapsed.ub.ascast, align 4
  %34 = srem i64 %omp.collapsed.iv.ascast.val, %25
  %35 = trunc i64 %31 to i32
  store i32 %35, ptr addrspace(4) %.omp.uncollapsed.iv.ascast, align 4
  %36 = sdiv i64 %34, 1
  %37 = trunc i64 %36 to i32
  store i32 %37, ptr addrspace(4) %.omp.uncollapsed.lb10.ascast, align 4
  %38 = trunc i64 %36 to i32
  store i32 %38, ptr addrspace(4) %.omp.uncollapsed.ub11.ascast, align 4
  %39 = srem i64 %34, 1
  br label %omp.uncollapsed.loop.cond

omp.collapsed.loop.exit:                          ; preds = %omp.collapsed.loop.cond
  br label %omp.collapsed.loop.postexit

omp.uncollapsed.loop.cond:                        ; preds = %omp.collapsed.loop.body, %omp.uncollapsed.loop.inc30
  %40 = load i32, ptr addrspace(4) %.omp.uncollapsed.iv.ascast, align 4
  %41 = load i32, ptr addrspace(4) %.omp.uncollapsed.ub.ascast, align 4
  %cmp12 = icmp sle i32 %40, %41
  br i1 %cmp12, label %omp.uncollapsed.loop.body, label %omp.collapsed.loop.inc

omp.uncollapsed.loop.body:                        ; preds = %omp.uncollapsed.loop.cond
  %42 = load i32, ptr addrspace(4) %.omp.uncollapsed.lb10.ascast, align 4
  store i32 %42, ptr addrspace(4) %.omp.uncollapsed.iv9.ascast, align 4
  br label %omp.uncollapsed.loop.cond14

omp.uncollapsed.loop.cond14:                      ; preds = %omp.uncollapsed.loop.body17, %omp.uncollapsed.loop.body
  %43 = load i32, ptr addrspace(4) %.omp.uncollapsed.iv9.ascast, align 4
  %44 = load i32, ptr addrspace(4) %.omp.uncollapsed.ub11.ascast, align 4
  %cmp15 = icmp sle i32 %43, %44
  br i1 %cmp15, label %omp.uncollapsed.loop.body17, label %omp.uncollapsed.loop.inc30

omp.uncollapsed.loop.body17:                      ; preds = %omp.uncollapsed.loop.cond14
  %45 = load i32, ptr addrspace(4) %.omp.uncollapsed.iv.ascast, align 4
  %mul18 = mul nsw i32 %45, 1
  %add19 = add nsw i32 0, %mul18
  store i32 %add19, ptr addrspace(4) %Y.ascast, align 4
  %46 = load i32, ptr addrspace(4) %.omp.uncollapsed.iv9.ascast, align 4
  %mul20 = mul nsw i32 %46, 1
  %add21 = add nsw i32 0, %mul20
  store i32 %add21, ptr addrspace(4) %X.ascast, align 4
  %47 = load i32, ptr addrspace(4) %X.ascast, align 4
  %48 = load i32, ptr addrspace(4) %Y.ascast, align 4
  %add22 = add nsw i32 %47, %48
  %conv23 = sitofp i32 %add22 to float
  %49 = load ptr addrspace(4), ptr addrspace(4) %P.map.ptr.tmp.ascast, align 8
  %50 = load i32, ptr addrspace(4) %Z.ascast, align 4
  %idxprom = sext i32 %50 to i64
  %arrayidx24 = getelementptr inbounds [4 x [4 x float]], ptr addrspace(4) %49, i64 %idxprom
  %51 = load i32, ptr addrspace(4) %Y.ascast, align 4
  %idxprom25 = sext i32 %51 to i64
  %arrayidx26 = getelementptr inbounds [4 x [4 x float]], ptr addrspace(4) %arrayidx24, i64 0, i64 %idxprom25
  %52 = load i32, ptr addrspace(4) %X.ascast, align 4
  %idxprom27 = sext i32 %52 to i64
  %arrayidx28 = getelementptr inbounds [4 x float], ptr addrspace(4) %arrayidx26, i64 0, i64 %idxprom27
  store float %conv23, ptr addrspace(4) %arrayidx28, align 4
  %53 = load i32, ptr addrspace(4) %.omp.uncollapsed.iv9.ascast, align 4
  %add29 = add nsw i32 %53, 1
  store i32 %add29, ptr addrspace(4) %.omp.uncollapsed.iv9.ascast, align 4
  br label %omp.uncollapsed.loop.cond14

omp.uncollapsed.loop.inc30:                       ; preds = %omp.uncollapsed.loop.cond14
  %54 = load i32, ptr addrspace(4) %.omp.uncollapsed.iv.ascast, align 4
  %add31 = add nsw i32 %54, 1
  store i32 %add31, ptr addrspace(4) %.omp.uncollapsed.iv.ascast, align 4
  br label %omp.uncollapsed.loop.cond

omp.collapsed.loop.inc:                           ; preds = %omp.uncollapsed.loop.cond
  %55 = load volatile i64, ptr addrspace(4) %omp.collapsed.iv.ascast, align 8
  %56 = add nuw nsw i64 %55, 1
  store volatile i64 %56, ptr addrspace(4) %omp.collapsed.iv.ascast, align 8
  br label %omp.collapsed.loop.cond

omp.collapsed.loop.postexit:                      ; preds = %omp.collapsed.loop.exit
  br label %omp.collapsed.loop.postexit.split

omp.collapsed.loop.postexit.split:                ; preds = %DIR.OMP.PARALLEL.LOOP.11, %omp.collapsed.loop.postexit
  br label %DIR.OMP.END.PARALLEL.LOOP.12

DIR.OMP.END.PARALLEL.LOOP.12:                     ; preds = %omp.collapsed.loop.postexit.split
  call void @llvm.directive.region.exit(token %22) [ "DIR.OMP.END.PARALLEL.LOOP"() ]

  br label %DIR.OMP.END.PARALLEL.LOOP.11

DIR.OMP.END.PARALLEL.LOOP.11:                     ; preds = %DIR.OMP.END.PARALLEL.LOOP.12
  %57 = load volatile i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %add34 = add nsw i32 %57, 1
  store volatile i32 %add34, ptr addrspace(4) %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.loop.exit:                                    ; preds = %omp.inner.for.cond
  br label %omp.loop.exit.split

omp.loop.exit.split:                              ; preds = %DIR.OMP.DISTRIBUTE.8, %omp.loop.exit
  br label %DIR.OMP.END.DISTRIBUTE.13

DIR.OMP.END.DISTRIBUTE.13:                        ; preds = %omp.loop.exit.split
  call void @llvm.directive.region.exit(token %13) [ "DIR.OMP.END.DISTRIBUTE"() ]

  br label %omp.precond.end

omp.precond.end:                                  ; preds = %DIR.OMP.END.DISTRIBUTE.13, %DIR.OMP.TEAMS.6
  br label %DIR.OMP.END.TEAMS.12

DIR.OMP.END.TEAMS.12:                             ; preds = %omp.precond.end
  br label %DIR.OMP.END.TEAMS.12.split

DIR.OMP.END.TEAMS.12.split:                       ; preds = %DIR.OMP.TEAMS.6130, %DIR.OMP.END.TEAMS.12
  br label %DIR.OMP.END.TEAMS.14

DIR.OMP.END.TEAMS.14:                             ; preds = %DIR.OMP.END.TEAMS.12.split
  call void @llvm.directive.region.exit(token %10) [ "DIR.OMP.END.TEAMS"() ]

  br label %DIR.OMP.END.TEAMS.13

DIR.OMP.END.TEAMS.13:                             ; preds = %DIR.OMP.END.TEAMS.14
  br label %DIR.OMP.END.TEAMS.13.split

DIR.OMP.END.TEAMS.13.split:                       ; preds = %DIR.OMP.TARGET.3127, %DIR.OMP.END.TEAMS.13
  br label %DIR.OMP.END.TARGET.15

DIR.OMP.END.TARGET.15:                            ; preds = %DIR.OMP.END.TEAMS.13.split
  call void @llvm.directive.region.exit(token %8) [ "DIR.OMP.END.TARGET"() ]

  br label %DIR.OMP.END.TARGET.14

DIR.OMP.END.TARGET.14:                            ; preds = %DIR.OMP.END.TARGET.15
  ret void
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

!omp_offload.info = !{!0}

!0 = !{i32 0, i32 53, i32 -676375550, !"_Z3foo", i32 2, i32 0, i32 0}
