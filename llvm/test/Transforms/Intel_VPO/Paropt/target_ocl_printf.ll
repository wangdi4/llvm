; RUN: opt < %s -switch-to-offload=true -vpo-paropt  -S | FileCheck %s
; RUN: opt < %s -passes='vpo-paropt' -switch-to-offload  -S | FileCheck %s

; Checks that in the SPIR64 target compilation we replace printf()
; with the OCL builtin version _Z18__spirv_ocl_printfPU3AS2ci()
;
; #include <stdio.h>
; int main() {
;   int var = 123;
;   #pragma omp target map(var)
;   {
;      printf("\n\nHello %d %f %s \n\n\n", var, 456.0, "finally!");
;   }
; }

source_filename = "p6.cpp"
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64"
target device_triples = "spir64"

@.str = private unnamed_addr addrspace(1) constant [21 x i8] c"\0A\0AHello %d %f %s \0A\0A\0A\00", align 1
@.str.1 = private unnamed_addr addrspace(1) constant [9 x i8] c"finally!\00", align 1

; This is the original prototype
declare dso_local spir_func i32 @printf(i8 addrspace(4)*, ...) #2

; Paropt creates this prototype for OCL printf
; CHECK: declare dso_local spir_func i32 @_Z18__spirv_ocl_printfPU3AS2ci(i8 addrspace(1)*, ...)

; Function Attrs: noinline norecurse nounwind optnone uwtable
define dso_local i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %retval.ascast = addrspacecast i32* %retval to i32 addrspace(4)*
  %var = alloca i32, align 4
  %var.ascast = addrspacecast i32* %var to i32 addrspace(4)*
  store i32 123, i32 addrspace(4)* %var.ascast, align 4
  br label %DIR.OMP.TARGET.1

DIR.OMP.TARGET.1:                                 ; preds = %entry
  br label %DIR.OMP.TARGET.2

DIR.OMP.TARGET.2:                                 ; preds = %DIR.OMP.TARGET.1
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.TOFROM"(i32 addrspace(4)* %var.ascast) ]
  br label %DIR.OMP.TARGET.32

DIR.OMP.TARGET.32:                                ; preds = %DIR.OMP.TARGET.2
  br label %DIR.OMP.TARGET.3

DIR.OMP.TARGET.3:                                 ; preds = %DIR.OMP.TARGET.32
  %1 = load i32, i32 addrspace(4)* %var.ascast, align 4
  %call = call spir_func i32 (i8 addrspace(4)*, ...) @printf(i8 addrspace(4)* getelementptr inbounds ([21 x i8], [21 x i8] addrspace(4)* addrspacecast ([21 x i8] addrspace(1)* @.str to [21 x i8] addrspace(4)*), i64 0, i64 0), i32 %1, double 4.560000e+02, i8 addrspace(4)* getelementptr inbounds ([9 x i8], [9 x i8] addrspace(4)* addrspacecast ([9 x i8] addrspace(1)* @.str.1 to [9 x i8] addrspace(4)*), i64 0, i64 0))

; Make sure the original printf is removed
; CHECK-NOT: call {{.*}} (i8 addrspace(4)*, ...) @printf(i8 addrspace(4)* getelementptr

; Calling the OCL printf
; CHECK: call {{.*}} (i8 addrspace(1)*, ...) @_Z18__spirv_ocl_printfPU3AS2ci(i8 addrspace(1)* getelementptr

  br label %DIR.OMP.END.TARGET.4.split

DIR.OMP.END.TARGET.4.split:                       ; preds = %DIR.OMP.TARGET.3
  br label %DIR.OMP.END.TARGET.4

DIR.OMP.END.TARGET.4:                             ; preds = %DIR.OMP.END.TARGET.4.split
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  br label %DIR.OMP.END.TARGET.5

DIR.OMP.END.TARGET.5:                             ; preds = %DIR.OMP.END.TARGET.4
  ret i32 0
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline norecurse nounwind optnone uwtable "contains-openmp-target"="true" "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "openmp-target-declare"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!3}

!0 = !{i32 0, i32 58, i32 -692894678, !"main", i32 4, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{}
!3 = !{!"clang version 9.0.0"}
