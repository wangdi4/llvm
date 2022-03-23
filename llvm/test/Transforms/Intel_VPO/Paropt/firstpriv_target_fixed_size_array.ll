; RUN: opt -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -switch-to-offload -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

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


; ModuleID = 'firstpriv_target_fixed_size_array.cpp'
source_filename = "firstpriv_target_fixed_size_array.cpp"
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

%class.A = type { i8 }

; Function Attrs: noinline optnone
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
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.FIRSTPRIVATE:NONPOD"([100 x %class.A] addrspace(4)* %e.ascast, void (%class.A addrspace(4)*, %class.A addrspace(4)*)* @_ZTS1A.omp.copy_constr, void (%class.A addrspace(4)*)* @_ZTS1A.omp.destr), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %d.ascast) ]
  store i32 0, i32 addrspace(4)* %d.ascast, align 4
  br label %for.cond


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
; CHECK-NEXT:  br label %{{.*}}


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

declare spir_func void @_ZN1AC1Ev(%class.A addrspace(4)*) unnamed_addr #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: noinline
define internal void @_ZTS1A.omp.copy_constr(%class.A addrspace(4)* %0, %class.A addrspace(4)* %1) #3 {
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

; Function Attrs: noinline
define internal void @_ZTS1A.omp.destr(%class.A addrspace(4)* %0) #3 {
entry:
  %.addr = alloca %class.A addrspace(4)*, align 8
  %.addr.ascast = addrspacecast %class.A addrspace(4)** %.addr to %class.A addrspace(4)* addrspace(4)*
  store %class.A addrspace(4)* %0, %class.A addrspace(4)* addrspace(4)* %.addr.ascast, align 8
  ret void
}

attributes #0 = { noinline optnone "contains-openmp-target"="true" "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }
attributes #3 = { noinline "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "openmp-target-declare"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2}
!opencl.used.extensions = !{!3}
!opencl.used.optional.core.features = !{!3}
!opencl.compiler.options = !{!3}
!llvm.ident = !{!4}

!0 = !{i32 0, i32 2065, i32 28193777, !"_Z3fn1v", i32 10, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"PIC Level", i32 2}
!3 = !{}
!4 = !{!"clang version 9.0.0"}
