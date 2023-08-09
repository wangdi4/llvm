; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -S < %s | FileCheck %s

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

; Check that a complicated case with multiple nested loops is handled correctly.
;
; Original source:
;
; void nested_loops() {
; #pragma omp target teams distribute
;   for (int i = 0; i < N; ++i) {
;     foo();
;     for (int j = 0; j < N; ++j) {
;       foo();
;     }
;     for (int j = 0; j < N; ++j) {
;       foo();
; #pragma omp parallel
;       for (int k = 0; k < N; ++k) {
;         foo();
;       }
;     }
;     foo();
;   }
; }
define spir_func void @nested_loops() {
entry:
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %i = alloca i32, align 4
  %j = alloca i32, align 4
  %j2 = alloca i32, align 4
  %k = alloca i32, align 4
  %.omp.lb.ascast = addrspacecast ptr %.omp.lb to ptr addrspace(4)
  %.omp.ub.ascast = addrspacecast ptr %.omp.ub to ptr addrspace(4)
  %tmp.ascast = addrspacecast ptr %tmp to ptr addrspace(4)
  %.omp.iv.ascast = addrspacecast ptr %.omp.iv to ptr addrspace(4)
  %i.ascast = addrspacecast ptr %i to ptr addrspace(4)
  %j.ascast = addrspacecast ptr %j to ptr addrspace(4)
  %j2.ascast = addrspacecast ptr %j2 to ptr addrspace(4)
  %k.ascast = addrspacecast ptr %k to ptr addrspace(4)
  store i32 0, ptr addrspace(4) %.omp.lb.ascast, align 4
  store i32 1023, ptr addrspace(4) %.omp.ub.ascast, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %j.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %j2.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %k.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1) ]

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %j.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %j2.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %k.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1) ]

  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE"(),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %j.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %j2.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %k.ascast, i32 0, i32 1) ]

  %3 = load i32, ptr addrspace(4) %.omp.lb.ascast, align 4
  store i32 %3, ptr addrspace(4) %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %4 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %5 = load i32, ptr addrspace(4) %.omp.ub.ascast, align 4
  %cmp = icmp sle i32 %4, %5
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %6 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %mul = mul nsw i32 %6, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr addrspace(4) %i.ascast, align 4
  call spir_func void @foo()
  store i32 0, ptr addrspace(4) %j.ascast, align 4
  br label %for.cond

; CHECK:       omp.inner.for.body:
; CHECK-NEXT:    [[I_LOCAL:%.*]] = phi i32
; CHECK-NEXT:    call spir_func void @_Z22__spirv_ControlBarrieriii
; CHECK-NEXT:    br i1 %is.master.thread, label %[[CODE0:master.thread.code[0-9]*]], label %[[FALLTHRU0:master.thread.fallthru[0-9]*]]

; CHECK:       [[CODE0]]:
; CHECK-NEXT:    %mul = mul nsw i32 [[I_LOCAL]], 1
; CHECK-NEXT:    %add = add nsw i32 0, %mul
; CHECK-NEXT:    store i32 %add, ptr addrspace(3) @i.ascast.priv.__local, align 4
; CHECK-NEXT:    call spir_func void @foo()
; CHECK-NEXT:    store i32 0, ptr addrspace(3) @j.ascast.priv.__local, align 4

for.cond:                                         ; preds = %for.inc, %omp.inner.for.body
  %7 = load i32, ptr addrspace(4) %j.ascast, align 4
  %cmp1 = icmp slt i32 %7, 1024
  br i1 %cmp1, label %for.body, label %for.end

; CHECK:       for.cond:
; CHECK-NEXT:    [[J:%.*]] = load i32, ptr addrspace(3) @j.ascast.priv.__local, align 4
; CHECK-NEXT:    %cmp1 = icmp slt i32 [[J]], 1024
; CHECK-NEXT:    br i1 %cmp1, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  call spir_func void @foo() #3
  br label %for.inc

; CHECK:       for.body:
; CHECK-NEXT:    call spir_func void @foo()
; CHECK-NEXT:    br label %for.inc

for.inc:                                          ; preds = %for.body
  %8 = load i32, ptr addrspace(4) %j.ascast, align 4
  %inc = add nsw i32 %8, 1
  store i32 %inc, ptr addrspace(4) %j.ascast, align 4
  br label %for.cond

; CHECK:       for.inc:
; CHECK-NEXT:    [[J1:%.*]] = load i32, ptr addrspace(3) @j.ascast.priv.__local, align 4
; CHECK-NEXT:    %inc = add nsw i32 [[J1]], 1
; CHECK-NEXT:    store i32 %inc, ptr addrspace(3) @j.ascast.priv.__local, align 4
; CHECK-NEXT:    br label %for.cond

for.end:                                          ; preds = %for.cond
  store i32 0, ptr addrspace(4) %j2.ascast, align 4
  br label %for.cond3

; CHECK:       for.end:
; CHECK-NEXT:    store i32 0, ptr addrspace(3) @j2.ascast.priv.__local, align 4
; CHECK-NEXT:    br label %[[FALLTHRU0]]

; CHECK:       [[FALLTHRU0]]:
; CHECK-NEXT:    call spir_func void @_Z22__spirv_ControlBarrieriii
; CHECK-NEXT:    br label %for.cond3

for.cond3:                                        ; preds = %for.inc12, %for.end
  %9 = load i32, ptr addrspace(4) %j2.ascast, align 4
  %cmp4 = icmp slt i32 %9, 1024
  br i1 %cmp4, label %for.body5, label %for.end14

for.body5:                                        ; preds = %for.cond3
  call spir_func void @foo()

; CHECK:       for.body5:
; CHECK-NEXT:    call spir_func void @_Z22__spirv_ControlBarrieriii
; CHECK-NEXT:    br i1 %is.master.thread, label %[[CODE1:master.thread.code[0-9]*]], label %[[FALLTHRU1:master.thread.fallthru[0-9]*]]

; CHECK:       [[CODE1]]:
; CHECK-NEXT:    call spir_func void @foo()
; CHECK:         br label %[[FALLTHRU1]]

; CHECK:       [[FALLTHRU1]]:
; CHECK-NEXT:    call spir_func void @_Z22__spirv_ControlBarrieriii

  %10 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %k.ascast, i32 0, i32 1) ]

  store i32 0, ptr addrspace(4) %k.ascast, align 4
  br label %for.cond6

for.cond6:                                        ; preds = %for.inc9, %for.body5
  %11 = load i32, ptr addrspace(4) %k.ascast, align 4
  %cmp7 = icmp slt i32 %11, 1024
  br i1 %cmp7, label %for.body8, label %for.end11

for.body8:                                        ; preds = %for.cond6
  call spir_func void @foo()
  br label %for.inc9

for.inc9:                                         ; preds = %for.body8
  %12 = load i32, ptr addrspace(4) %k.ascast, align 4
  %inc10 = add nsw i32 %12, 1
  store i32 %inc10, ptr addrspace(4) %k.ascast, align 4
  br label %for.cond6

for.end11:                                        ; preds = %for.cond6
  call void @llvm.directive.region.exit(token %10) [ "DIR.OMP.END.PARALLEL"() ]

  br label %for.inc12

for.inc12:                                        ; preds = %for.end11
  %13 = load i32, ptr addrspace(4) %j2.ascast, align 4
  %inc13 = add nsw i32 %13, 1
  store i32 %inc13, ptr addrspace(4) %j2.ascast, align 4
  br label %for.cond3

; CHECK:       for.inc12:
; CHECK-NEXT:    call spir_func void @_Z22__spirv_ControlBarrieriii
; CHECK-NEXT:    br i1 %is.master.thread, label %[[CODE2:master.thread.code[0-9]*]], label %[[FALLTHRU2:master.thread.fallthru[0-9]*]]

; CHECK:       [[CODE2]]:
; CHECK-NEXT:    [[J2:%.*]] = load i32, ptr addrspace(3) @j2.ascast.priv.__local, align 4
; CHECK-NEXT:    %inc13 = add nsw i32 [[J2]], 1
; CHECK-NEXT:    store i32 %inc13, ptr addrspace(3) @j2.ascast.priv.__local, align 4
; CHECK-NEXT:    br label %[[FALLTHRU2]]

; CHECK:       [[FALLTHRU2]]:
; CHECK-NEXT:    call spir_func void @_Z22__spirv_ControlBarrieriii
; CHECK-NEXT:    br label %for.cond3

for.end14:                                        ; preds = %for.cond3
  call spir_func void @foo()
  br label %omp.body.continue

; CHECK:       for.end14:
; CHECK-NEXT:    call spir_func void @_Z22__spirv_ControlBarrieriii
; CHECK-NEXT:    br i1 %is.master.thread, label %[[CODE3:master.thread.code[0-9]*]], label %[[FALLTHRU3:master.thread.fallthru[0-9]*]]

; CHECK:       [[CODE3]]:
; CHECK-NEXT:    call spir_func void @foo()
; CHECK-NEXT:    br label %omp.body.continue

omp.body.continue:                                ; preds = %for.end14
  br label %omp.inner.for.inc

; CHECK:       omp.body.continue:
; CHECK-NEXT:    br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %14 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %add15 = add nsw i32 %14, 1
  store i32 %add15, ptr addrspace(4) %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

; CHECK:       omp.inner.for.inc:
; CHECK-NEXT:    br label %[[FALLTHRU3]]

; CHECK:       [[FALLTHRU3]]:
; CHECK-NEXT:    call spir_func void @_Z22__spirv_ControlBarrieriii
; CHECK-NEXT:    %add15 = add nsw i32 [[I_LOCAL]], 1

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.DISTRIBUTE"() ]

  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TEAMS"() ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]

  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare spir_func void @foo()

!omp_offload.info = !{!0}

!0 = !{i32 0, i32 66306, i32 12855403, !"_Z12nested_loops", i32 6, i32 0, i32 0, i32 0}
