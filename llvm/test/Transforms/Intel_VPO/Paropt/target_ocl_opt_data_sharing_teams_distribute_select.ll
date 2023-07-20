; RUN: opt -bugpoint-enable-legacy-pm -switch-to-offload -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt-optimize-data-sharing -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-restore-operands,vpo-cfg-restructuring,vpo-paropt-optimize-data-sharing),vpo-paropt' -switch-to-offload -S %s | FileCheck %s

; Original code:
; const int& max(const int &a, const int &b) {
;   return a < b ? b : a;
; }
;
; void test1() {
; #pragma forceinline recursive
;   {
; #pragma omp target teams distribute
;     for (int i = 0; i < 3; i++) {
;       int var = max(i + 5, i + 7);
;     }
;   }
; }
;
; IR was captured after inlining pass. 'var' and temporaries created for passing
; arguments to 'max' should not be created in local address space.
;
; CHECK-NOT: @{{.+}} = internal addrspace(3) global
; CHECK-NOT: call spir_func void @_Z22__spirv_ControlBarrieriii

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

define hidden spir_func void @_Z5test1v() local_unnamed_addr {
DIR.OMP.TARGET.346:
  %.omp.lb = alloca i32, align 4
  %.omp.lb.ascast = addrspacecast ptr %.omp.lb to ptr addrspace(4)
  %.omp.ub = alloca i32, align 4
  %.omp.ub.ascast = addrspacecast ptr %.omp.ub to ptr addrspace(4)
  %tmp = alloca i32, align 4
  %tmp.ascast = addrspacecast ptr %tmp to ptr addrspace(4)
  %.omp.iv = alloca i32, align 4
  %.omp.iv.ascast = addrspacecast ptr %.omp.iv to ptr addrspace(4)
  %i = alloca i32, align 4
  %i.ascast = addrspacecast ptr %i to ptr addrspace(4)
  %var = alloca i32, align 4
  %var.ascast = addrspacecast ptr %var to ptr addrspace(4)
  %ref.tmp = alloca i32, align 4
  %ref.tmp.ascast = addrspacecast ptr %ref.tmp to ptr addrspace(4)
  %ref.tmp2 = alloca i32, align 4
  %ref.tmp2.ascast = addrspacecast ptr %ref.tmp2 to ptr addrspace(4)
  store i32 0, ptr addrspace(4) %.omp.lb.ascast, align 4
  store volatile i32 2, ptr addrspace(4) %.omp.ub.ascast, align 4
  %i.ascast.addr = alloca ptr addrspace(4), align 8
  %var.ascast.addr = alloca ptr addrspace(4), align 8
  %ref.tmp.ascast.addr = alloca ptr addrspace(4), align 8
  %ref.tmp2.ascast.addr = alloca ptr addrspace(4), align 8
  %.omp.lb.ascast.addr = alloca ptr addrspace(4), align 8
  %.omp.iv.ascast.addr = alloca ptr addrspace(4), align 8
  %i.ascast.addr12 = alloca ptr addrspace(4), align 8
  %var.ascast.addr14 = alloca ptr addrspace(4), align 8
  %tmp.ascast.addr = alloca ptr addrspace(4), align 8
  %ref.tmp.ascast.addr16 = alloca ptr addrspace(4), align 8
  %ref.tmp2.ascast.addr18 = alloca ptr addrspace(4), align 8
  %.omp.lb.ascast.addr20 = alloca ptr addrspace(4), align 8
  %.omp.ub.ascast.addr = alloca ptr addrspace(4), align 8
  %.omp.iv.ascast.addr23 = alloca ptr addrspace(4), align 8
  %i.ascast.addr25 = alloca ptr addrspace(4), align 8
  %var.ascast.addr27 = alloca ptr addrspace(4), align 8
  %tmp.ascast.addr29 = alloca ptr addrspace(4), align 8
  %ref.tmp.ascast.addr31 = alloca ptr addrspace(4), align 8
  %ref.tmp2.ascast.addr33 = alloca ptr addrspace(4), align 8
  %.omp.lb.ascast.addr35 = alloca ptr addrspace(4), align 8
  %.omp.ub.ascast.addr37 = alloca ptr addrspace(4), align 8
  store ptr addrspace(4) %.omp.iv.ascast, ptr %.omp.iv.ascast.addr23, align 8
  store ptr addrspace(4) %i.ascast, ptr %i.ascast.addr25, align 8
  store ptr addrspace(4) %var.ascast, ptr %var.ascast.addr27, align 8
  store ptr addrspace(4) %tmp.ascast, ptr %tmp.ascast.addr29, align 8
  store ptr addrspace(4) %ref.tmp.ascast, ptr %ref.tmp.ascast.addr31, align 8
  store ptr addrspace(4) %ref.tmp2.ascast, ptr %ref.tmp2.ascast.addr33, align 8
  store ptr addrspace(4) %.omp.lb.ascast, ptr %.omp.lb.ascast.addr35, align 8
  store ptr addrspace(4) %.omp.ub.ascast, ptr %.omp.ub.ascast.addr37, align 8
  %end.dir.temp43 = alloca i1, align 1
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %var.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %ref.tmp.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %ref.tmp2.ascast, i32 0, i32 1),
    "QUAL.OMP.OFFLOAD.NDRANGE"(ptr addrspace(4) %.omp.ub.ascast, i32 0),
    "QUAL.OMP.OPERAND.ADDR"(ptr addrspace(4) %.omp.iv.ascast, ptr %.omp.iv.ascast.addr23),
    "QUAL.OMP.OPERAND.ADDR"(ptr addrspace(4) %i.ascast, ptr %i.ascast.addr25),
    "QUAL.OMP.OPERAND.ADDR"(ptr addrspace(4) %var.ascast, ptr %var.ascast.addr27),
    "QUAL.OMP.OPERAND.ADDR"(ptr addrspace(4) %tmp.ascast, ptr %tmp.ascast.addr29),
    "QUAL.OMP.OPERAND.ADDR"(ptr addrspace(4) %ref.tmp.ascast, ptr %ref.tmp.ascast.addr31),
    "QUAL.OMP.OPERAND.ADDR"(ptr addrspace(4) %ref.tmp2.ascast, ptr %ref.tmp2.ascast.addr33),
    "QUAL.OMP.OPERAND.ADDR"(ptr addrspace(4) %.omp.lb.ascast, ptr %.omp.lb.ascast.addr35),
    "QUAL.OMP.OPERAND.ADDR"(ptr addrspace(4) %.omp.ub.ascast, ptr %.omp.ub.ascast.addr37),
    "QUAL.OMP.JUMP.TO.END.IF"(ptr %end.dir.temp43) ]

  %temp.load44 = load volatile i1, ptr %end.dir.temp43, align 1
  br i1 %temp.load44, label %DIR.OMP.END.TARGET.12, label %DIR.OMP.TEAMS.6

DIR.OMP.TEAMS.6:                                  ; preds = %DIR.OMP.TARGET.346
  %.omp.iv.ascast24 = load volatile ptr addrspace(4), ptr %.omp.iv.ascast.addr23, align 8
  %i.ascast26 = load volatile ptr addrspace(4), ptr %i.ascast.addr25, align 8
  %var.ascast28 = load volatile ptr addrspace(4), ptr %var.ascast.addr27, align 8
  %tmp.ascast30 = load volatile ptr addrspace(4), ptr %tmp.ascast.addr29, align 8
  %ref.tmp.ascast32 = load volatile ptr addrspace(4), ptr %ref.tmp.ascast.addr31, align 8
  %ref.tmp2.ascast34 = load volatile ptr addrspace(4), ptr %ref.tmp2.ascast.addr33, align 8
  %.omp.lb.ascast36 = load volatile ptr addrspace(4), ptr %.omp.lb.ascast.addr35, align 8
  %.omp.ub.ascast38 = load volatile ptr addrspace(4), ptr %.omp.ub.ascast.addr37, align 8
  store ptr addrspace(4) %.omp.iv.ascast24, ptr %.omp.iv.ascast.addr, align 8
  store ptr addrspace(4) %i.ascast26, ptr %i.ascast.addr12, align 8
  store ptr addrspace(4) %var.ascast28, ptr %var.ascast.addr14, align 8
  store ptr addrspace(4) %tmp.ascast30, ptr %tmp.ascast.addr, align 8
  store ptr addrspace(4) %ref.tmp.ascast32, ptr %ref.tmp.ascast.addr16, align 8
  store ptr addrspace(4) %ref.tmp2.ascast34, ptr %ref.tmp2.ascast.addr18, align 8
  store ptr addrspace(4) %.omp.lb.ascast36, ptr %.omp.lb.ascast.addr20, align 8
  store ptr addrspace(4) %.omp.ub.ascast38, ptr %.omp.ub.ascast.addr, align 8
  %end.dir.temp40 = alloca i1, align 1
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv.ascast24, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %.omp.lb.ascast36, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %.omp.ub.ascast38, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast26, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %var.ascast28, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast30, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %ref.tmp.ascast32, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %ref.tmp2.ascast34, i32 0, i32 1),
    "QUAL.OMP.OPERAND.ADDR"(ptr addrspace(4) %.omp.iv.ascast24, ptr %.omp.iv.ascast.addr),
    "QUAL.OMP.OPERAND.ADDR"(ptr addrspace(4) %i.ascast26, ptr %i.ascast.addr12),
    "QUAL.OMP.OPERAND.ADDR"(ptr addrspace(4) %var.ascast28, ptr %var.ascast.addr14),
    "QUAL.OMP.OPERAND.ADDR"(ptr addrspace(4) %tmp.ascast30, ptr %tmp.ascast.addr),
    "QUAL.OMP.OPERAND.ADDR"(ptr addrspace(4) %ref.tmp.ascast32, ptr %ref.tmp.ascast.addr16),
    "QUAL.OMP.OPERAND.ADDR"(ptr addrspace(4) %ref.tmp2.ascast34, ptr %ref.tmp2.ascast.addr18),
    "QUAL.OMP.OPERAND.ADDR"(ptr addrspace(4) %.omp.lb.ascast36, ptr %.omp.lb.ascast.addr20),
    "QUAL.OMP.OPERAND.ADDR"(ptr addrspace(4) %.omp.ub.ascast38, ptr %.omp.ub.ascast.addr),
    "QUAL.OMP.JUMP.TO.END.IF"(ptr %end.dir.temp40) ]

  %temp.load41 = load volatile i1, ptr %end.dir.temp40, align 1
  br i1 %temp.load41, label %DIR.OMP.END.TEAMS.10, label %DIR.OMP.DISTRIBUTE.8

DIR.OMP.DISTRIBUTE.8:                             ; preds = %DIR.OMP.TEAMS.6
  %.omp.iv.ascast11 = load volatile ptr addrspace(4), ptr %.omp.iv.ascast.addr, align 8
  %i.ascast13 = load volatile ptr addrspace(4), ptr %i.ascast.addr12, align 8
  %var.ascast15 = load volatile ptr addrspace(4), ptr %var.ascast.addr14, align 8
  %ref.tmp.ascast17 = load volatile ptr addrspace(4), ptr %ref.tmp.ascast.addr16, align 8
  %ref.tmp2.ascast19 = load volatile ptr addrspace(4), ptr %ref.tmp2.ascast.addr18, align 8
  %.omp.lb.ascast21 = load volatile ptr addrspace(4), ptr %.omp.lb.ascast.addr20, align 8
  %.omp.ub.ascast22 = load volatile ptr addrspace(4), ptr %.omp.ub.ascast.addr, align 8
  store ptr addrspace(4) %i.ascast13, ptr %i.ascast.addr, align 8
  store ptr addrspace(4) %var.ascast15, ptr %var.ascast.addr, align 8
  store ptr addrspace(4) %ref.tmp.ascast17, ptr %ref.tmp.ascast.addr, align 8
  store ptr addrspace(4) %ref.tmp2.ascast19, ptr %ref.tmp2.ascast.addr, align 8
  store ptr addrspace(4) %.omp.lb.ascast21, ptr %.omp.lb.ascast.addr, align 8
  %end.dir.temp = alloca i1, align 1
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE"(),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) %.omp.iv.ascast11, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast21, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) %.omp.ub.ascast22, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast13, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %var.ascast15, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %ref.tmp.ascast17, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %ref.tmp2.ascast19, i32 0, i32 1),
    "QUAL.OMP.OFFLOAD.KNOWN.NDRANGE"(i1 false),
    "QUAL.OMP.OPERAND.ADDR"(ptr addrspace(4) %i.ascast13, ptr %i.ascast.addr),
    "QUAL.OMP.OPERAND.ADDR"(ptr addrspace(4) %var.ascast15, ptr %var.ascast.addr),
    "QUAL.OMP.OPERAND.ADDR"(ptr addrspace(4) %ref.tmp.ascast17, ptr %ref.tmp.ascast.addr),
    "QUAL.OMP.OPERAND.ADDR"(ptr addrspace(4) %ref.tmp2.ascast19, ptr %ref.tmp2.ascast.addr),
    "QUAL.OMP.OPERAND.ADDR"(ptr addrspace(4) %.omp.lb.ascast21, ptr %.omp.lb.ascast.addr),
    "QUAL.OMP.JUMP.TO.END.IF"(ptr %end.dir.temp) ]

  %temp.load = load volatile i1, ptr %end.dir.temp, align 1
  br i1 %temp.load, label %DIR.OMP.END.DISTRIBUTE.9, label %DIR.OMP.DISTRIBUTE.6

DIR.OMP.DISTRIBUTE.6:                             ; preds = %DIR.OMP.DISTRIBUTE.8
  %i.ascast6 = load volatile ptr addrspace(4), ptr %i.ascast.addr, align 8
  %var.ascast7 = load volatile ptr addrspace(4), ptr %var.ascast.addr, align 8
  %ref.tmp.ascast8 = load volatile ptr addrspace(4), ptr %ref.tmp.ascast.addr, align 8
  %ref.tmp2.ascast9 = load volatile ptr addrspace(4), ptr %ref.tmp2.ascast.addr, align 8
  %.omp.lb.ascast10 = load volatile ptr addrspace(4), ptr %.omp.lb.ascast.addr, align 8
  %3 = load i32, ptr addrspace(4) %.omp.lb.ascast10, align 4
  store volatile i32 %3, ptr addrspace(4) %.omp.iv.ascast11, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.body, %DIR.OMP.DISTRIBUTE.6
  %4 = load volatile i32, ptr addrspace(4) %.omp.iv.ascast11, align 4
  %5 = load volatile i32, ptr addrspace(4) %.omp.ub.ascast22, align 4
  %cmp.not = icmp sgt i32 %4, %5
  br i1 %cmp.not, label %DIR.OMP.END.DISTRIBUTE.9, label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %6 = load volatile i32, ptr addrspace(4) %.omp.iv.ascast11, align 4
  store i32 %6, ptr addrspace(4) %i.ascast6, align 4
  %add1 = add nsw i32 %6, 5
  store i32 %add1, ptr addrspace(4) %ref.tmp.ascast8, align 4
  %7 = load i32, ptr addrspace(4) %i.ascast6, align 4
  %add3 = add nsw i32 %7, 7
  store i32 %add3, ptr addrspace(4) %ref.tmp2.ascast9, align 4
  %8 = load i32, ptr addrspace(4) %ref.tmp.ascast8, align 4
  %9 = load i32, ptr addrspace(4) %ref.tmp2.ascast9, align 4
  %cmp.i = icmp slt i32 %8, %9
  %10 = select i1 %cmp.i, ptr addrspace(4) %ref.tmp2.ascast9, ptr addrspace(4) %ref.tmp.ascast8
  %11 = load i32, ptr addrspace(4) %10, align 4
  store i32 %11, ptr addrspace(4) %var.ascast7, align 4
  %12 = load volatile i32, ptr addrspace(4) %.omp.iv.ascast11, align 4
  %add4 = add nsw i32 %12, 1
  store volatile i32 %add4, ptr addrspace(4) %.omp.iv.ascast11, align 4
  br label %omp.inner.for.cond

DIR.OMP.END.DISTRIBUTE.9:                         ; preds = %DIR.OMP.DISTRIBUTE.8, %omp.inner.for.cond
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.DISTRIBUTE"() ]

  br label %DIR.OMP.END.TEAMS.10

DIR.OMP.END.TEAMS.10:                             ; preds = %DIR.OMP.END.DISTRIBUTE.9, %DIR.OMP.TEAMS.6
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TEAMS"() ]

  br label %DIR.OMP.END.TARGET.12

DIR.OMP.END.TARGET.12:                            ; preds = %DIR.OMP.TARGET.346, %DIR.OMP.END.TEAMS.10
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]

  ret void
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

!omp_offload.info = !{!0}

!0 = !{i32 0, i32 53, i32 -687391223, !"_Z5test1v", i32 8, i32 0, i32 0}
