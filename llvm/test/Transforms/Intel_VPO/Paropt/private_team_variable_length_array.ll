; RUN: opt -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -switch-to-offload -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; This test is used to check emitting loop of calling constructor for variable
; length array with team construct + private clause.

; Test src:
;
; class A
; {
; public:
; #pragma omp declare target
;   A();
; #pragma omp end declare target
; };
; void fn1(int n) {
;   A e[n];
; #pragma omp target teams private(e)
;   for(int d=0; d<n; d++);
; }

; Constructor
; CHECK:  %[[CONSTR_BEGIN:[^,]+]] = getelementptr inbounds %class.A, ptr %vla.ascast.priv{{.*}}, i32 0
; CHECK-NEXT:  %[[END:[^,]+]] = getelementptr %class.A, ptr %[[CONSTR_BEGIN]], i64 %[[VLA_LEN:[^,]+]]
; CHECK-NEXT:  %priv.constr.isempty{{.*}} = icmp eq ptr %[[CONSTR_BEGIN]], %[[END]]
; CHECK-NEXT:  br i1 %priv.constr.isempty{{.*}}, label %priv.constr.done{{.*}}, label %priv.constr.body{{.*}}
; CHECK-LABEL: priv.constr.body{{.*}}:
; CHECK-NEXT:  %priv.cpy.dest.ptr{{.*}} = phi ptr [ %[[CONSTR_BEGIN]], %{{.*}} ], [ %priv.cpy.dest.inc{{.*}}, %{{.*}} ]
; CHECK:  call spir_func ptr addrspace(4) @_ZTS1A.omp.def_constr(ptr addrspace(4) %{{.*}})
; CHECK:  %priv.cpy.dest.inc{{.*}} = getelementptr %class.A, ptr %priv.cpy.dest.ptr{{.*}}, i32 1
; CHECK-NEXT:  %priv.cpy.done{{.*}} = icmp eq ptr %priv.cpy.dest.inc{{.*}}, %[[END]]
; CHECK-NEXT:  br i1 %priv.cpy.done{{.*}}, label %priv.constr.done{{.*}}, label %priv.constr.body{{.*}}
; CHECK-LABEL: priv.constr.done{{.*}}:
; CHECK-NEXT:  br label %{{.*}}

; CHECK:  %[[CONSTR_BEGIN2:[^,]+]] = getelementptr inbounds %class.A, ptr %vla.ascast.priv, i32 0
; CHECK-NEXT:  %[[END2:[^,]+]] = getelementptr %class.A, ptr %[[CONSTR_BEGIN2]], i64 %[[VLA_LEN:[^,]+]]
; CHECK-NEXT:  %priv.constr.isempty = icmp eq ptr %[[CONSTR_BEGIN2]], %[[END2]]
; CHECK-NEXT:  br i1 %priv.constr.isempty, label %priv.constr.done, label %priv.constr.body
; CHECK-LABEL: priv.constr.body:
; CHECK-NEXT:  %priv.cpy.dest.ptr = phi ptr [ %[[CONSTR_BEGIN2]], %{{.*}} ], [ %priv.cpy.dest.inc, %{{.*}} ]
; CHECK:  call spir_func ptr addrspace(4) @_ZTS1A.omp.def_constr(ptr addrspace(4) %{{.*}})
; CHECK:  %priv.cpy.dest.inc = getelementptr %class.A, ptr %priv.cpy.dest.ptr, i32 1
; CHECK-NEXT:  %priv.cpy.done = icmp eq ptr %priv.cpy.dest.inc, %[[END2]]
; CHECK-NEXT:  br i1 %priv.cpy.done, label %priv.constr.done, label %priv.constr.body
; CHECK-LABEL: priv.constr.done:
; CHECK-NEXT:  call spir_func void @_Z22__spirv_ControlBarrieriii(i32 2, i32 2, i32 784)
; CHECK-NEXT:  br i1 %is.master.thread, label %master.thread.code{{[0-9]*}}, label %master.thread.fallthru{{[0-9]*}}

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

%class.A = type { i8 }

define protected spir_func void @_Z3fn1i(i32 noundef %n) {
entry:
  %n.addr = alloca i32, align 4
  %saved_stack = alloca ptr, align 8
  %__vla_expr0 = alloca i64, align 8
  %omp.vla.tmp = alloca i64, align 8
  %d = alloca i32, align 4
  %n.addr.ascast = addrspacecast ptr %n.addr to ptr addrspace(4)
  %saved_stack.ascast = addrspacecast ptr %saved_stack to ptr addrspace(4)
  %__vla_expr0.ascast = addrspacecast ptr %__vla_expr0 to ptr addrspace(4)
  %omp.vla.tmp.ascast = addrspacecast ptr %omp.vla.tmp to ptr addrspace(4)
  %d.ascast = addrspacecast ptr %d to ptr addrspace(4)
  store i32 %n, ptr addrspace(4) %n.addr.ascast, align 4
  %0 = load i32, ptr addrspace(4) %n.addr.ascast, align 4
  %1 = zext i32 %0 to i64
  %2 = call ptr @llvm.stacksave()
  store ptr %2, ptr addrspace(4) %saved_stack.ascast, align 8
  %vla = alloca %class.A, i64 %1, align 1
  %vla.ascast = addrspacecast ptr %vla to ptr addrspace(4)
  store i64 %1, ptr addrspace(4) %__vla_expr0.ascast, align 8
  %isempty = icmp eq i64 %1, 0
  br i1 %isempty, label %arrayctor.cont, label %new.ctorloop

new.ctorloop:                                     ; preds = %entry
  %arrayctor.end = getelementptr inbounds %class.A, ptr addrspace(4) %vla.ascast, i64 %1
  br label %arrayctor.loop

arrayctor.loop:                                   ; preds = %arrayctor.loop, %new.ctorloop
  %arrayctor.cur = phi ptr addrspace(4) [ %vla.ascast, %new.ctorloop ], [ %arrayctor.next, %arrayctor.loop ]
  call spir_func void @_ZN1AC1Ev(ptr addrspace(4) noundef align 1 dereferenceable_or_null(1) %arrayctor.cur)
  %arrayctor.next = getelementptr inbounds %class.A, ptr addrspace(4) %arrayctor.cur, i64 1
  %arrayctor.done = icmp eq ptr addrspace(4) %arrayctor.next, %arrayctor.end
  br i1 %arrayctor.done, label %arrayctor.cont, label %arrayctor.loop

arrayctor.cont:                                   ; preds = %entry, %arrayctor.loop
  store i64 %1, ptr addrspace(4) %omp.vla.tmp.ascast, align 8
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.PRIVATE:NONPOD.TYPED.VARLEN"(ptr addrspace(4) %vla.ascast, %class.A zeroinitializer, i64 %1, ptr @_ZTS1A.omp.def_constr, ptr @_ZTS1A.omp.destr),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %n.addr.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %d.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %omp.vla.tmp.ascast, i64 0, i32 1) ]

  %4 = load i64, ptr addrspace(4) %omp.vla.tmp.ascast, align 8
  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.PRIVATE:NONPOD.TYPED"(ptr addrspace(4) %vla.ascast, %class.A zeroinitializer, i64 %4, ptr @_ZTS1A.omp.def_constr, ptr @_ZTS1A.omp.destr),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %n.addr.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %d.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %omp.vla.tmp.ascast, i64 0, i32 1) ]

  %6 = load i64, ptr addrspace(4) %omp.vla.tmp.ascast, align 8
  store i32 0, ptr addrspace(4) %d.ascast, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %arrayctor.cont
  %7 = load i32, ptr addrspace(4) %d.ascast, align 4
  %8 = load i32, ptr addrspace(4) %n.addr.ascast, align 4
  %cmp = icmp slt i32 %7, %8
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %9 = load i32, ptr addrspace(4) %d.ascast, align 4
  %inc = add nsw i32 %9, 1
  store i32 %inc, ptr addrspace(4) %d.ascast, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.TEAMS"() ]

  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.TARGET"() ]

  %10 = load ptr, ptr addrspace(4) %saved_stack.ascast, align 8
  call void @llvm.stackrestore(ptr %10)
  ret void
}

declare ptr @llvm.stacksave()

declare spir_func void @_ZN1AC1Ev(ptr addrspace(4) noundef align 1 dereferenceable_or_null(1)) unnamed_addr

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

declare spir_func noundef ptr addrspace(4) @_ZTS1A.omp.def_constr(ptr addrspace(4) noundef %0)

declare spir_func void @_ZTS1A.omp.destr(ptr addrspace(4) noundef %0)

declare void @llvm.stackrestore(ptr)

!omp_offload.info = !{!0}

!0 = !{i32 0, i32 43, i32 -1929563031, !"_Z3fn1i", i32 10, i32 0, i32 0, i32 0}
