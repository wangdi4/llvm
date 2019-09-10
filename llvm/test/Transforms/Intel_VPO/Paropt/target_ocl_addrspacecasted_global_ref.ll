; RUN: opt < %s -prepare-switch-to-offload -switch-to-offload=true -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt  -S | FileCheck %s
; RUN: opt < %s -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -prepare-switch-to-offload -switch-to-offload  -S | FileCheck %s

; Original code:
; #pragma omp declare target
; int my_glob[100];
;
; void foo(char *);
; #pragma omp end declare target
;
; void bar() {
;   int i;
; #pragma omp target map(my_glob[:10])
;   foo((char *)my_glob);
; }

; Check that @my_glob reference inside the region is correctly replaced
; with the argument of the outline function:
; CHECK: @my_glob = common dso_local
; CHECK-NOT: @my_glob

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64"
target device_triples = "spir64"

@my_glob = common dso_local target_declare addrspace(1) global [100 x i32] zeroinitializer, align 4

; Function Attrs: noinline nounwind optnone uwtable
define dso_local spir_func void @bar() #0 {
entry:
  %i = alloca i32, align 4
  %i.ascast = addrspacecast i32* %i to i32 addrspace(4)*
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 1), "QUAL.OMP.MAP.TOFROM:AGGRHEAD"([100 x i32] addrspace(4)* addrspacecast ([100 x i32] addrspace(1)* @my_glob to [100 x i32] addrspace(4)*), i32 addrspace(4)* getelementptr inbounds ([100 x i32], [100 x i32] addrspace(4)* addrspacecast ([100 x i32] addrspace(1)* @my_glob to [100 x i32] addrspace(4)*), i64 0, i64 0), i64 40) ]
  call spir_func void @foo(i8 addrspace(4)* addrspacecast (i8 addrspace(1)* bitcast ([100 x i32] addrspace(1)* @my_glob to i8 addrspace(1)*) to i8 addrspace(4)*))
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

declare dso_local spir_func void @foo(i8 addrspace(4)*) #2

attributes #0 = { noinline nounwind optnone uwtable "contains-openmp-target"="true" "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "openmp-target-declare"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!omp_offload.info = !{!0, !1}
!llvm.module.flags = !{!2}
!opencl.used.extensions = !{!3}
!opencl.used.optional.core.features = !{!3}
!opencl.compiler.options = !{!3}
!llvm.ident = !{!4}

!0 = !{i32 0, i32 2052, i32 85998847, !"bar", i32 9, i32 1, i32 0}
!1 = !{i32 1, !"my_glob", i32 0, i32 0}
!2 = !{i32 1, !"wchar_size", i32 4}
!3 = !{}
!4 = !{!"clang version 9.0.0"}
