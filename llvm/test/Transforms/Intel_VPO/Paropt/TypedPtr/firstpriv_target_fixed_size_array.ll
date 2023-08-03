; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -opaque-pointers=0 -switch-to-offload -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; This test is used to check emitting loop of calling copy constructor for fixed
; size array type with target construct + firstprivate clause.
; class A
; {
; public:
; #pragma omp declare target
;   A();
; #pragma omp end declare target
; };
; void fn1() {
;   A e[100];
; #pragma omp target firstprivate(e)
;   for(int d=0; d<100; d++);
; }
;

; Copy Constructor
; CHECK:  %[[TO:[^,]+]] = getelementptr inbounds [100 x %class.A], [100 x %class.A]* %e.ascast.fpriv, i32 0, i32 0
; CHECK-NEXT:  %[[FROM:[^,]+]] = getelementptr inbounds [100 x %class.A], [100 x %class.A] addrspace(1)* %e.ascast, i32 0, i32 0
; CHECK-NEXT:  %[[END:[^,]+]] = getelementptr %class.A, %class.A* %[[TO]], i32 100
; CHECK-NEXT:  %priv.cpyctor.isempty = icmp eq %class.A* %[[TO]], %[[END]]
; CHECK-NEXT:  br i1 %priv.cpyctor.isempty, label %priv.cpyctor.done, label %priv.cpyctor.body
; CHECK-LABEL: priv.cpyctor.body:
; CHECK-NEXT:  %priv.cpy.dest.ptr = phi %class.A* [ %[[TO]], %{{.*}} ], [ %priv.cpy.dest.inc, %{{.*}} ]
; CHECK-NEXT:  %priv.cpy.src.ptr = phi %class.A addrspace(1)* [ %[[FROM]], %{{.*}} ], [ %priv.cpy.src.inc, %{{.*}} ]
; CHECK:  call spir_func void @_ZTS1A.omp.copy_constr(%class.A addrspace(4)* %{{.*}}, %class.A addrspace(4)* %{{.*}})
; CHECK:  %priv.cpy.dest.inc = getelementptr %class.A, %class.A* %priv.cpy.dest.ptr, i32 1
; CHECK-NEXT:  %priv.cpy.src.inc = getelementptr %class.A, %class.A addrspace(1)* %priv.cpy.src.ptr, i32 1
; CHECK-NEXT:  %priv.cpy.done = icmp eq %class.A* %priv.cpy.dest.inc, %[[END]]
; CHECK-NEXT:  br i1 %priv.cpy.done, label %priv.cpyctor.done, label %priv.cpyctor.body
; CHECK-LABEL: priv.cpyctor.done:
; CHECK-NEXT:  call spir_func void @_Z22__spirv_ControlBarrieriii(i32 2, i32 2, i32 784)
; CHECK-NEXT:  br i1 %is.master.thread, label %master.thread.code{{[0-9]*}}, label %master.thread.fallthru{{[0-9]*}}

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

%class.A = type { i8 }

define hidden spir_func void @_Z3fn1v() #0 {
entry:
  %e = alloca [100 x %class.A], align 1
  %e.ascast = addrspacecast [100 x %class.A]* %e to [100 x %class.A] addrspace(4)*
  %d = alloca i32, align 4
  %d.ascast = addrspacecast i32* %d to i32 addrspace(4)*
  %array.begin = getelementptr inbounds [100 x %class.A], [100 x %class.A] addrspace(4)* %e.ascast, i32 0, i32 0
  %arrayctor.end = getelementptr inbounds %class.A, %class.A addrspace(4)* %array.begin, i64 100
  br label %arrayctor.loop

arrayctor.loop:                                   ; preds = %arrayctor.loop, %entry
  %arrayctor.cur = phi %class.A addrspace(4)* [ %array.begin, %entry ], [ %arrayctor.next, %arrayctor.loop ]
  call spir_func void @_ZN1AC1Ev(%class.A addrspace(4)* %arrayctor.cur)
  %arrayctor.next = getelementptr inbounds %class.A, %class.A addrspace(4)* %arrayctor.cur, i64 1
  %arrayctor.done = icmp eq %class.A addrspace(4)* %arrayctor.next, %arrayctor.end
  br i1 %arrayctor.done, label %arrayctor.cont, label %arrayctor.loop

arrayctor.cont:                                   ; preds = %arrayctor.loop
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.FIRSTPRIVATE:NONPOD"([100 x %class.A] addrspace(4)* %e.ascast, void (%class.A addrspace(4)*, %class.A addrspace(4)*)* @_ZTS1A.omp.copy_constr, void (%class.A addrspace(4)*)* @_ZTS1A.omp.destr),
    "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %d.ascast) ]

  store i32 0, i32 addrspace(4)* %d.ascast, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %arrayctor.cont
  %1 = load i32, i32 addrspace(4)* %d.ascast, align 4
  %cmp = icmp slt i32 %1, 100
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %2 = load i32, i32 addrspace(4)* %d.ascast, align 4
  %inc = add nsw i32 %2, 1
  store i32 %inc, i32 addrspace(4)* %d.ascast, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

declare spir_func void @_ZN1AC1Ev(%class.A addrspace(4)*) unnamed_addr

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

define internal void @_ZTS1A.omp.copy_constr(%class.A addrspace(4)* %0, %class.A addrspace(4)* %1)#1 {
entry:
  %.addr = alloca %class.A addrspace(4)*, align 8
  %.addr.ascast = addrspacecast %class.A addrspace(4)** %.addr to %class.A addrspace(4)* addrspace(4)*
  %.addr1 = alloca %class.A addrspace(4)*, align 8
  %.addr1.ascast = addrspacecast %class.A addrspace(4)** %.addr1 to %class.A addrspace(4)* addrspace(4)*
  store %class.A addrspace(4)* %0, %class.A addrspace(4)* addrspace(4)* %.addr.ascast, align 8
  store %class.A addrspace(4)* %1, %class.A addrspace(4)* addrspace(4)* %.addr1.ascast, align 8
  %2 = load %class.A addrspace(4)*, %class.A addrspace(4)* addrspace(4)* %.addr.ascast, align 8
  %3 = load %class.A addrspace(4)*, %class.A addrspace(4)* addrspace(4)* %.addr1.ascast, align 8
  ret void
}

define internal void @_ZTS1A.omp.destr(%class.A addrspace(4)* %0)#1 {
entry:
  %.addr = alloca %class.A addrspace(4)*, align 8
  %.addr.ascast = addrspacecast %class.A addrspace(4)** %.addr to %class.A addrspace(4)* addrspace(4)*
  store %class.A addrspace(4)* %0, %class.A addrspace(4)* addrspace(4)* %.addr.ascast, align 8
  ret void
}

attributes #0 = { "contains-openmp-target"="true" }
attributes#1 = { "openmp-target-declare"="true" }

!omp_offload.info = !{!0}
!0 = !{i32 0, i32 2065, i32 28193777, !"_Z3fn1v", i32 10, i32 0, i32 0}
