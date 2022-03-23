; RUN: opt -switch-to-offload -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt-optimize-data-sharing -vpo-paropt -S %s | FileCheck %s
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
  %.omp.lb.ascast = addrspacecast i32* %.omp.lb to i32 addrspace(4)*
  %.omp.ub = alloca i32, align 4
  %.omp.ub.ascast = addrspacecast i32* %.omp.ub to i32 addrspace(4)*
  %tmp = alloca i32, align 4
  %tmp.ascast = addrspacecast i32* %tmp to i32 addrspace(4)*
  %.omp.iv = alloca i32, align 4
  %.omp.iv.ascast = addrspacecast i32* %.omp.iv to i32 addrspace(4)*
  %i = alloca i32, align 4
  %i.ascast = addrspacecast i32* %i to i32 addrspace(4)*
  %var = alloca i32, align 4
  %var.ascast = addrspacecast i32* %var to i32 addrspace(4)*
  %ref.tmp = alloca i32, align 4
  %ref.tmp.ascast = addrspacecast i32* %ref.tmp to i32 addrspace(4)*
  %ref.tmp2 = alloca i32, align 4
  %ref.tmp2.ascast = addrspacecast i32* %ref.tmp2 to i32 addrspace(4)*
  store i32 0, i32 addrspace(4)* %.omp.lb.ascast, align 4
  store volatile i32 2, i32 addrspace(4)* %.omp.ub.ascast, align 4
  %i.ascast.addr = alloca i32 addrspace(4)*, align 8
  %var.ascast.addr = alloca i32 addrspace(4)*, align 8
  %ref.tmp.ascast.addr = alloca i32 addrspace(4)*, align 8
  %ref.tmp2.ascast.addr = alloca i32 addrspace(4)*, align 8
  %.omp.lb.ascast.addr = alloca i32 addrspace(4)*, align 8
  %.omp.iv.ascast.addr = alloca i32 addrspace(4)*, align 8
  %i.ascast.addr12 = alloca i32 addrspace(4)*, align 8
  %var.ascast.addr14 = alloca i32 addrspace(4)*, align 8
  %tmp.ascast.addr = alloca i32 addrspace(4)*, align 8
  %ref.tmp.ascast.addr16 = alloca i32 addrspace(4)*, align 8
  %ref.tmp2.ascast.addr18 = alloca i32 addrspace(4)*, align 8
  %.omp.lb.ascast.addr20 = alloca i32 addrspace(4)*, align 8
  %.omp.ub.ascast.addr = alloca i32 addrspace(4)*, align 8
  %.omp.iv.ascast.addr23 = alloca i32 addrspace(4)*, align 8
  %i.ascast.addr25 = alloca i32 addrspace(4)*, align 8
  %var.ascast.addr27 = alloca i32 addrspace(4)*, align 8
  %tmp.ascast.addr29 = alloca i32 addrspace(4)*, align 8
  %ref.tmp.ascast.addr31 = alloca i32 addrspace(4)*, align 8
  %ref.tmp2.ascast.addr33 = alloca i32 addrspace(4)*, align 8
  %.omp.lb.ascast.addr35 = alloca i32 addrspace(4)*, align 8
  %.omp.ub.ascast.addr37 = alloca i32 addrspace(4)*, align 8
  store i32 addrspace(4)* %.omp.iv.ascast, i32 addrspace(4)** %.omp.iv.ascast.addr23, align 8
  store i32 addrspace(4)* %i.ascast, i32 addrspace(4)** %i.ascast.addr25, align 8
  store i32 addrspace(4)* %var.ascast, i32 addrspace(4)** %var.ascast.addr27, align 8
  store i32 addrspace(4)* %tmp.ascast, i32 addrspace(4)** %tmp.ascast.addr29, align 8
  store i32 addrspace(4)* %ref.tmp.ascast, i32 addrspace(4)** %ref.tmp.ascast.addr31, align 8
  store i32 addrspace(4)* %ref.tmp2.ascast, i32 addrspace(4)** %ref.tmp2.ascast.addr33, align 8
  store i32 addrspace(4)* %.omp.lb.ascast, i32 addrspace(4)** %.omp.lb.ascast.addr35, align 8
  store i32 addrspace(4)* %.omp.ub.ascast, i32 addrspace(4)** %.omp.ub.ascast.addr37, align 8
  %end.dir.temp43 = alloca i1, align 1
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %var.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %ref.tmp.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %ref.tmp2.ascast), "QUAL.OMP.OFFLOAD.NDRANGE"(i32 addrspace(4)* %.omp.ub.ascast, i32 0), "QUAL.OMP.OPERAND.ADDR"(i32 addrspace(4)* %.omp.iv.ascast, i32 addrspace(4)** %.omp.iv.ascast.addr23), "QUAL.OMP.OPERAND.ADDR"(i32 addrspace(4)* %i.ascast, i32 addrspace(4)** %i.ascast.addr25), "QUAL.OMP.OPERAND.ADDR"(i32 addrspace(4)* %var.ascast, i32 addrspace(4)** %var.ascast.addr27), "QUAL.OMP.OPERAND.ADDR"(i32 addrspace(4)* %tmp.ascast, i32 addrspace(4)** %tmp.ascast.addr29), "QUAL.OMP.OPERAND.ADDR"(i32 addrspace(4)* %ref.tmp.ascast, i32 addrspace(4)** %ref.tmp.ascast.addr31), "QUAL.OMP.OPERAND.ADDR"(i32 addrspace(4)* %ref.tmp2.ascast, i32 addrspace(4)** %ref.tmp2.ascast.addr33), "QUAL.OMP.OPERAND.ADDR"(i32 addrspace(4)* %.omp.lb.ascast, i32 addrspace(4)** %.omp.lb.ascast.addr35), "QUAL.OMP.OPERAND.ADDR"(i32 addrspace(4)* %.omp.ub.ascast, i32 addrspace(4)** %.omp.ub.ascast.addr37), "QUAL.OMP.JUMP.TO.END.IF"(i1* %end.dir.temp43) ]
  %temp.load44 = load volatile i1, i1* %end.dir.temp43, align 1
  br i1 %temp.load44, label %DIR.OMP.END.TARGET.12, label %DIR.OMP.TEAMS.6

DIR.OMP.TEAMS.6:                                  ; preds = %DIR.OMP.TARGET.346
  %.omp.iv.ascast24 = load volatile i32 addrspace(4)*, i32 addrspace(4)** %.omp.iv.ascast.addr23, align 8
  %i.ascast26 = load volatile i32 addrspace(4)*, i32 addrspace(4)** %i.ascast.addr25, align 8
  %var.ascast28 = load volatile i32 addrspace(4)*, i32 addrspace(4)** %var.ascast.addr27, align 8
  %tmp.ascast30 = load volatile i32 addrspace(4)*, i32 addrspace(4)** %tmp.ascast.addr29, align 8
  %ref.tmp.ascast32 = load volatile i32 addrspace(4)*, i32 addrspace(4)** %ref.tmp.ascast.addr31, align 8
  %ref.tmp2.ascast34 = load volatile i32 addrspace(4)*, i32 addrspace(4)** %ref.tmp2.ascast.addr33, align 8
  %.omp.lb.ascast36 = load volatile i32 addrspace(4)*, i32 addrspace(4)** %.omp.lb.ascast.addr35, align 8
  %.omp.ub.ascast38 = load volatile i32 addrspace(4)*, i32 addrspace(4)** %.omp.ub.ascast.addr37, align 8
  store i32 addrspace(4)* %.omp.iv.ascast24, i32 addrspace(4)** %.omp.iv.ascast.addr, align 8
  store i32 addrspace(4)* %i.ascast26, i32 addrspace(4)** %i.ascast.addr12, align 8
  store i32 addrspace(4)* %var.ascast28, i32 addrspace(4)** %var.ascast.addr14, align 8
  store i32 addrspace(4)* %tmp.ascast30, i32 addrspace(4)** %tmp.ascast.addr, align 8
  store i32 addrspace(4)* %ref.tmp.ascast32, i32 addrspace(4)** %ref.tmp.ascast.addr16, align 8
  store i32 addrspace(4)* %ref.tmp2.ascast34, i32 addrspace(4)** %ref.tmp2.ascast.addr18, align 8
  store i32 addrspace(4)* %.omp.lb.ascast36, i32 addrspace(4)** %.omp.lb.ascast.addr20, align 8
  store i32 addrspace(4)* %.omp.ub.ascast38, i32 addrspace(4)** %.omp.ub.ascast.addr, align 8
  %end.dir.temp40 = alloca i1, align 1
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv.ascast24), "QUAL.OMP.SHARED"(i32 addrspace(4)* %.omp.lb.ascast36), "QUAL.OMP.SHARED"(i32 addrspace(4)* %.omp.ub.ascast38), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast26), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %var.ascast28), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp.ascast30), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %ref.tmp.ascast32), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %ref.tmp2.ascast34), "QUAL.OMP.OPERAND.ADDR"(i32 addrspace(4)* %.omp.iv.ascast24, i32 addrspace(4)** %.omp.iv.ascast.addr), "QUAL.OMP.OPERAND.ADDR"(i32 addrspace(4)* %i.ascast26, i32 addrspace(4)** %i.ascast.addr12), "QUAL.OMP.OPERAND.ADDR"(i32 addrspace(4)* %var.ascast28, i32 addrspace(4)** %var.ascast.addr14), "QUAL.OMP.OPERAND.ADDR"(i32 addrspace(4)* %tmp.ascast30, i32 addrspace(4)** %tmp.ascast.addr), "QUAL.OMP.OPERAND.ADDR"(i32 addrspace(4)* %ref.tmp.ascast32, i32 addrspace(4)** %ref.tmp.ascast.addr16), "QUAL.OMP.OPERAND.ADDR"(i32 addrspace(4)* %ref.tmp2.ascast34, i32 addrspace(4)** %ref.tmp2.ascast.addr18), "QUAL.OMP.OPERAND.ADDR"(i32 addrspace(4)* %.omp.lb.ascast36, i32 addrspace(4)** %.omp.lb.ascast.addr20), "QUAL.OMP.OPERAND.ADDR"(i32 addrspace(4)* %.omp.ub.ascast38, i32 addrspace(4)** %.omp.ub.ascast.addr), "QUAL.OMP.JUMP.TO.END.IF"(i1* %end.dir.temp40) ]
  %temp.load41 = load volatile i1, i1* %end.dir.temp40, align 1
  br i1 %temp.load41, label %DIR.OMP.END.TEAMS.10, label %DIR.OMP.DISTRIBUTE.8

DIR.OMP.DISTRIBUTE.8:                             ; preds = %DIR.OMP.TEAMS.6
  %.omp.iv.ascast11 = load volatile i32 addrspace(4)*, i32 addrspace(4)** %.omp.iv.ascast.addr, align 8
  %i.ascast13 = load volatile i32 addrspace(4)*, i32 addrspace(4)** %i.ascast.addr12, align 8
  %var.ascast15 = load volatile i32 addrspace(4)*, i32 addrspace(4)** %var.ascast.addr14, align 8
  %ref.tmp.ascast17 = load volatile i32 addrspace(4)*, i32 addrspace(4)** %ref.tmp.ascast.addr16, align 8
  %ref.tmp2.ascast19 = load volatile i32 addrspace(4)*, i32 addrspace(4)** %ref.tmp2.ascast.addr18, align 8
  %.omp.lb.ascast21 = load volatile i32 addrspace(4)*, i32 addrspace(4)** %.omp.lb.ascast.addr20, align 8
  %.omp.ub.ascast22 = load volatile i32 addrspace(4)*, i32 addrspace(4)** %.omp.ub.ascast.addr, align 8
  store i32 addrspace(4)* %i.ascast13, i32 addrspace(4)** %i.ascast.addr, align 8
  store i32 addrspace(4)* %var.ascast15, i32 addrspace(4)** %var.ascast.addr, align 8
  store i32 addrspace(4)* %ref.tmp.ascast17, i32 addrspace(4)** %ref.tmp.ascast.addr, align 8
  store i32 addrspace(4)* %ref.tmp2.ascast19, i32 addrspace(4)** %ref.tmp2.ascast.addr, align 8
  store i32 addrspace(4)* %.omp.lb.ascast21, i32 addrspace(4)** %.omp.lb.ascast.addr, align 8
  %end.dir.temp = alloca i1, align 1
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE"(), "QUAL.OMP.NORMALIZED.IV"(i32 addrspace(4)* %.omp.iv.ascast11), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb.ascast21), "QUAL.OMP.NORMALIZED.UB"(i32 addrspace(4)* %.omp.ub.ascast22), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast13), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %var.ascast15), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %ref.tmp.ascast17), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %ref.tmp2.ascast19), "QUAL.OMP.OFFLOAD.KNOWN.NDRANGE"(), "QUAL.OMP.OPERAND.ADDR"(i32 addrspace(4)* %i.ascast13, i32 addrspace(4)** %i.ascast.addr), "QUAL.OMP.OPERAND.ADDR"(i32 addrspace(4)* %var.ascast15, i32 addrspace(4)** %var.ascast.addr), "QUAL.OMP.OPERAND.ADDR"(i32 addrspace(4)* %ref.tmp.ascast17, i32 addrspace(4)** %ref.tmp.ascast.addr), "QUAL.OMP.OPERAND.ADDR"(i32 addrspace(4)* %ref.tmp2.ascast19, i32 addrspace(4)** %ref.tmp2.ascast.addr), "QUAL.OMP.OPERAND.ADDR"(i32 addrspace(4)* %.omp.lb.ascast21, i32 addrspace(4)** %.omp.lb.ascast.addr), "QUAL.OMP.JUMP.TO.END.IF"(i1* %end.dir.temp) ]
  %temp.load = load volatile i1, i1* %end.dir.temp, align 1
  br i1 %temp.load, label %DIR.OMP.END.DISTRIBUTE.9, label %DIR.OMP.DISTRIBUTE.6

DIR.OMP.DISTRIBUTE.6:                             ; preds = %DIR.OMP.DISTRIBUTE.8
  %i.ascast6 = load volatile i32 addrspace(4)*, i32 addrspace(4)** %i.ascast.addr, align 8
  %var.ascast7 = load volatile i32 addrspace(4)*, i32 addrspace(4)** %var.ascast.addr, align 8
  %ref.tmp.ascast8 = load volatile i32 addrspace(4)*, i32 addrspace(4)** %ref.tmp.ascast.addr, align 8
  %ref.tmp2.ascast9 = load volatile i32 addrspace(4)*, i32 addrspace(4)** %ref.tmp2.ascast.addr, align 8
  %.omp.lb.ascast10 = load volatile i32 addrspace(4)*, i32 addrspace(4)** %.omp.lb.ascast.addr, align 8
  %3 = load i32, i32 addrspace(4)* %.omp.lb.ascast10, align 4
  store volatile i32 %3, i32 addrspace(4)* %.omp.iv.ascast11, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.body, %DIR.OMP.DISTRIBUTE.6
  %4 = load volatile i32, i32 addrspace(4)* %.omp.iv.ascast11, align 4
  %5 = load volatile i32, i32 addrspace(4)* %.omp.ub.ascast22, align 4
  %cmp.not = icmp sgt i32 %4, %5
  br i1 %cmp.not, label %DIR.OMP.END.DISTRIBUTE.9, label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %6 = load volatile i32, i32 addrspace(4)* %.omp.iv.ascast11, align 4
  store i32 %6, i32 addrspace(4)* %i.ascast6, align 4
  %add1 = add nsw i32 %6, 5
  store i32 %add1, i32 addrspace(4)* %ref.tmp.ascast8, align 4
  %7 = load i32, i32 addrspace(4)* %i.ascast6, align 4
  %add3 = add nsw i32 %7, 7
  store i32 %add3, i32 addrspace(4)* %ref.tmp2.ascast9, align 4
  %8 = load i32, i32 addrspace(4)* %ref.tmp.ascast8, align 4
  %9 = load i32, i32 addrspace(4)* %ref.tmp2.ascast9, align 4
  %cmp.i = icmp slt i32 %8, %9
  %10 = select i1 %cmp.i, i32 addrspace(4)* %ref.tmp2.ascast9, i32 addrspace(4)* %ref.tmp.ascast8
  %11 = load i32, i32 addrspace(4)* %10, align 4
  store i32 %11, i32 addrspace(4)* %var.ascast7, align 4
  %12 = load volatile i32, i32 addrspace(4)* %.omp.iv.ascast11, align 4
  %add4 = add nsw i32 %12, 1
  store volatile i32 %add4, i32 addrspace(4)* %.omp.iv.ascast11, align 4
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
