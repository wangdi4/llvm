; RUN: opt -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -S %s | FileCheck %s
;
; It tests whether the kernel is generate in the form of OpenCL form.
;
; #include<stdio.h>
; int y=2;
; main()
; {
; int x=1;
;    #pragma omp target map(from:y) map(to:x)
;    y = x + 1; // The copy of y on the device has a value of 2.
;    printf("After the target region is executed, x = %d y = %d\n", x, y);
;    return 0;
; }
;

; CHECK: store i32 %{{.*}}, i32 addrspace(1)* %{{.*}}
; CHECK:  !spirv.Source = !{!{{.*}}}

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64"
target device_triples = "spir64"

@y = external dso_local addrspace(1) global i32, align 4
@.str = private unnamed_addr addrspace(1) constant [52 x i8] c"After the target region is executed, x = %d y = %d\0A\00", align 1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %0 = addrspacecast i32* %retval to i32 addrspace(4)*
  %x = alloca i32, align 4
  %1 = addrspacecast i32* %x to i32 addrspace(4)*
  store i32 0, i32 addrspace(4)* %0, align 4
  store i32 1, i32 addrspace(4)* %1, align 4
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.FROM"(i32 addrspace(4)* addrspacecast (i32 addrspace(1)* @y to i32 addrspace(4)*), i32 addrspace(4)* addrspacecast (i32 addrspace(1)* @y to i32 addrspace(4)*), i64 4, i64 34, i8* null, i8* null), "QUAL.OMP.MAP.TO"(i32 addrspace(4)* %1, i32 addrspace(4)* %1, i64 4, i64 33, i8* null, i8* null) ]
  %3 = load i32, i32 addrspace(4)* %1, align 4
  %add = add nsw i32 %3, 1
  store i32 %add, i32 addrspace(4)* addrspacecast (i32 addrspace(1)* @y to i32 addrspace(4)*), align 4
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TARGET"() ]
  %4 = load i32, i32 addrspace(4)* %1, align 4
  %5 = load i32, i32 addrspace(4)* addrspacecast (i32 addrspace(1)* @y to i32 addrspace(4)*), align 4
  %call = call spir_func i32 (i8 addrspace(4)*, ...) @printf(i8 addrspace(4)* getelementptr inbounds ([52 x i8], [52 x i8] addrspace(4)* addrspacecast ([52 x i8] addrspace(1)* @.str to [52 x i8] addrspace(4)*), i64 0, i64 0), i32 %4, i32 %5)
  ret i32 0
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

declare dso_local spir_func i32 @printf(i8 addrspace(4)*, ...) #2

attributes #0 = { noinline nounwind optnone uwtable "contains-openmp-target"="true" "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!3}

!0 = !{i32 0, i32 2052, i32 85985690, !"main", i32 6, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{}
!3 = !{!"clang version 9.0.0"}
