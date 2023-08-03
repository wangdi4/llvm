; RUN: opt -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -switch-to-offload -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; This test is used to check emitting loop of calling constructor for fixed size
; array with team construct + private clause.

; Test src:
;
; class A
; {
; public:
; #pragma omp declare target
;   A();
; #pragma omp end declare target
; };
; void fn1() {
;   A e[100];
; #pragma omp target teams private(e)
;   for(int d=0; d<100; d++);
; }

; Constructor
; CHECK:  %[[CONSTR_BEGIN:[^,]+]] = getelementptr inbounds [100 x %class.A], ptr %e.ascast.priv{{.*}}.gep, i32 0
; CHECK-NEXT:  %[[CONSTR_END:[^,]+]] = getelementptr %class.A, ptr %[[CONSTR_BEGIN]], i64 100
; CHECK-NEXT:  %priv.constr.isempty{{.*}} = icmp eq ptr %[[CONSTR_BEGIN]], %[[CONSTR_END]]
; CHECK-NEXT:  br i1 %priv.constr.isempty{{.*}}, label %priv.constr.done{{.*}}, label %priv.constr.body{{.*}}
; CHECK-LABEL: priv.constr.body{{.*}}:
; CHECK-NEXT:  %priv.cpy.dest.ptr{{.*}} = phi ptr [ %[[CONSTR_BEGIN]], %{{.*}} ], [ %priv.cpy.dest.inc{{.*}}, %{{.*}} ]
; CHECK-NEXT:  %[[DEST_ASCAST:[^,]+]] = addrspacecast ptr %priv.cpy.dest.ptr{{.*}} to ptr addrspace(4)
; CHECK:  call spir_func ptr addrspace(4) @_ZTS1A.omp.def_constr(ptr addrspace(4) %[[DEST_ASCAST]])
; CHECK:  %priv.cpy.dest.inc{{.*}} = getelementptr %class.A, ptr %priv.cpy.dest.ptr{{.*}}, i32 1
; CHECK-NEXT:  %priv.cpy.done{{.*}} = icmp eq ptr %priv.cpy.dest.inc{{.*}}, %[[CONSTR_END]]
; CHECK-NEXT:  br i1 %priv.cpy.done{{.*}}, label %priv.constr.done{{.*}}, label %priv.constr.body{{.*}}
; CHECK-LABEL: priv.constr.done{{.*}}:
; CHECK-NEXT:  br label %{{.*}}

; CHECK:  %[[CONSTR_BEGIN2:[^,]+]] = getelementptr inbounds [100 x %class.A], ptr %e.ascast.priv.gep, i32 0
; CHECK-NEXT:  %[[CONSTR_END2:[^,]+]] = getelementptr %class.A, ptr %[[CONSTR_BEGIN2]], i64 100
; CHECK-NEXT:  %priv.constr.isempty = icmp eq ptr %[[CONSTR_BEGIN2]], %[[CONSTR_END2]]
; CHECK-NEXT:  br i1 %priv.constr.isempty, label %priv.constr.done, label %priv.constr.body
; CHECK-LABEL: priv.constr.body:
; CHECK-NEXT:  %priv.cpy.dest.ptr = phi ptr [ %[[CONSTR_BEGIN2]], %{{.*}} ], [ %priv.cpy.dest.inc, %{{.*}} ]
; CHECK-NEXT:  %[[DEST_ASCAST:[^,]+]] = addrspacecast ptr %priv.cpy.dest.ptr to ptr addrspace(4)
; CHECK:  call spir_func ptr addrspace(4) @_ZTS1A.omp.def_constr(ptr addrspace(4) %[[DEST_ASCAST]])
; CHECK:  %priv.cpy.dest.inc = getelementptr %class.A, ptr %priv.cpy.dest.ptr, i32 1
; CHECK-NEXT:  %priv.cpy.done = icmp eq ptr %priv.cpy.dest.inc, %[[CONSTR_END2]]
; CHECK-NEXT:  br i1 %priv.cpy.done, label %priv.constr.done, label %priv.constr.body
; CHECK-LABEL: priv.constr.done:
; CHECK-NEXT:  call spir_func void @_Z22__spirv_ControlBarrieriii(i32 2, i32 2, i32 784)
; CHECK-NEXT:  br i1 %is.master.thread, label %master.thread.code{{[0-9]*}}, label %master.thread.fallthru{{[0-9]*}}

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

%class.A = type { i8 }

define protected spir_func void @_Z3fn1v() {
entry:
  %e = alloca [100 x %class.A], align 1
  %d = alloca i32, align 4
  %e.ascast = addrspacecast ptr %e to ptr addrspace(4)
  %d.ascast = addrspacecast ptr %d to ptr addrspace(4)
  %array.begin = getelementptr inbounds [100 x %class.A], ptr addrspace(4) %e.ascast, i32 0, i32 0
  %arrayctor.end = getelementptr inbounds %class.A, ptr addrspace(4) %array.begin, i64 100
  br label %arrayctor.loop

arrayctor.loop:                                   ; preds = %arrayctor.loop, %entry
  %arrayctor.cur = phi ptr addrspace(4) [ %array.begin, %entry ], [ %arrayctor.next, %arrayctor.loop ]
  call spir_func void @_ZN1AC1Ev(ptr addrspace(4) noundef align 1 dereferenceable_or_null(1) %arrayctor.cur)
  %arrayctor.next = getelementptr inbounds %class.A, ptr addrspace(4) %arrayctor.cur, i64 1
  %arrayctor.done = icmp eq ptr addrspace(4) %arrayctor.next, %arrayctor.end
  br i1 %arrayctor.done, label %arrayctor.cont, label %arrayctor.loop

arrayctor.cont:                                   ; preds = %arrayctor.loop
  %array.begin1 = getelementptr inbounds [100 x %class.A], ptr addrspace(4) %e.ascast, i32 0, i32 0
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.PRIVATE:NONPOD.TYPED"(ptr addrspace(4) %e.ascast, %class.A zeroinitializer, i64 100, ptr @_ZTS1A.omp.def_constr, ptr @_ZTS1A.omp.destr),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %d.ascast, i32 0, i32 1) ]

  %array.begin2 = getelementptr inbounds [100 x %class.A], ptr addrspace(4) %e.ascast, i32 0, i32 0
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.PRIVATE:NONPOD.TYPED"(ptr addrspace(4) %e.ascast, %class.A zeroinitializer, i64 100, ptr @_ZTS1A.omp.def_constr, ptr @_ZTS1A.omp.destr),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %d.ascast, i32 0, i32 1) ]

  store i32 0, ptr addrspace(4) %d.ascast, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %arrayctor.cont
  %2 = load i32, ptr addrspace(4) %d.ascast, align 4
  %cmp = icmp slt i32 %2, 100
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %3 = load i32, ptr addrspace(4) %d.ascast, align 4
  %inc = add nsw i32 %3, 1
  store i32 %inc, ptr addrspace(4) %d.ascast, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TEAMS"() ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]

  ret void
}

declare spir_func void @_ZN1AC1Ev(ptr addrspace(4) noundef align 1 dereferenceable_or_null(1)) unnamed_addr

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

declare spir_func noundef ptr addrspace(4) @_ZTS1A.omp.def_constr(ptr addrspace(4) noundef %0)

declare spir_func void @_ZTS1A.omp.destr(ptr addrspace(4) noundef %0)

!omp_offload.info = !{!0}

!0 = !{i32 0, i32 43, i32 -1934034804, !"_Z3fn1v", i32 10, i32 0, i32 0, i32 0}
