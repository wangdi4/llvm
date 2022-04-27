; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s

; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s | FileCheck %s

; GPU-offload test for Master. The test is created by compiling
; the C test below with:  icx -O0 -fiopenmp -fopenmp-targets=spir64 
; int main() {
;   int aaa = 10;
;   #pragma omp target map(tofrom:aaa)
;   {
;     #pragma omp master
;     aaa = aaa+2;
;   }
;   printf("aaa=%d",aaa);
;   return 0;
; }

; The GPU-offloading compilation will:
;
; 1. Remove the fence acquire/release that Clang added to the Master region
; CHECK-NOT: fence acquire
; CHECK-NOT: fence release
;
; 2. Emit the begin masked with tid = 0  and filter = 0  and emit end masked with tid =0.
; CHECK: %{{[0-9]+}} = call spir_func  i32 @__kmpc_masked(%struct.ident_t addrspace(4)* addrspacecast (%struct.ident_t addrspace(1)* @{{.*}} to %struct.ident_t addrspace(4)*), i32 0, i32 0)
; CHECK: call spir_func void @__kmpc_end_masked(%struct.ident_t addrspace(4)* addrspacecast (%struct.ident_t addrspace(1)* @{{.*}} to %struct.ident_t addrspace(4)*), i32 0)


; ModuleID = '<stdin>'
source_filename = "master.c"
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64"
target device_triples = "spir64"

@.str = private unnamed_addr constant [7 x i8] c"aaa=%d\00", align 1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %aaa = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  store i32 10, i32* %aaa, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.TOFROM"(i32* %aaa) ]
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.MASTER"() ]
  fence acquire
  %2 = load i32, i32* %aaa, align 4
  %add = add nsw i32 %2, 2
  store i32 %add, i32* %aaa, align 4
  fence release
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.MASTER"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  %3 = load i32, i32* %aaa, align 4
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i32 0, i32 0), i32 %3)
  ret i32 0
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

declare dso_local i32 @printf(i8*, ...) #2

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}
!llvm.ident = !{!2}

!0 = !{i32 0, i32 59, i32 -677974782, !"main", i32 4, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{!"clang version 8.0.0"}
