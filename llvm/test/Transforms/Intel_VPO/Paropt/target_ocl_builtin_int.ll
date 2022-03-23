; RUN: opt -vpo-paropt -switch-to-offload -S %s | FileCheck %s
; RUN: opt -passes='vpo-paropt' -switch-to-offload -S %s | FileCheck %s

; Check that integer math functions "abs" and "labs" are being replaced with
; OpenCL mangled versions "_Z3absi" and "_Z3absl", respectively.
;
; #include<math.h>
; void foo() {
;   #pragma omp target
;   {
;     int  aa = abs(123);
;     long bb = labs(456);
;   }
; }

; ModuleID = 'target_ocl_builtin_int.cpp'
source_filename = "target_ocl_builtin_int.cpp"
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: nounwind readnone
declare dso_local spir_func i32 @abs(i32) #2
; CHECK: declare {{.*}} i32 @_Z17__spirv_ocl_s_absi(i32) #0

; Function Attrs: nounwind readnone
declare dso_local spir_func i64 @labs(i64) #2
; CHECK: declare {{.*}} i64 @_Z17__spirv_ocl_s_absl(i64)

define hidden spir_func void @_Z3foov() #0 {
entry:
  %aa = alloca i32, align 4
  %aa.ascast = addrspacecast i32* %aa to i32 addrspace(4)*
  %bb = alloca i64, align 8
  %bb.ascast = addrspacecast i64* %bb to i64 addrspace(4)*
  br label %DIR.OMP.TARGET.1

DIR.OMP.TARGET.1:                                 ; preds = %entry
  br label %DIR.OMP.TARGET.2

DIR.OMP.TARGET.2:                                 ; preds = %DIR.OMP.TARGET.1
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.PRIVATE:WILOCAL"(i32 addrspace(4)* %aa.ascast), "QUAL.OMP.PRIVATE:WILOCAL"(i64 addrspace(4)* %bb.ascast) ]
  br label %DIR.OMP.TARGET.35

DIR.OMP.TARGET.35:                                ; preds = %DIR.OMP.TARGET.2
  br label %DIR.OMP.TARGET.3

DIR.OMP.TARGET.3:                                 ; preds = %DIR.OMP.TARGET.35

  %call = call spir_func i32 @abs(i32 123) #3
; CHECK:  call spir_func i32 @_Z17__spirv_ocl_s_absi(i32 123)

  store i32 %call, i32 addrspace(4)* %aa.ascast, align 4

  %call1 = call spir_func i64 @labs(i64 456) #3
; CHECK:   call spir_func i64 @_Z17__spirv_ocl_s_absl(i64 456)

  store i64 %call1, i64 addrspace(4)* %bb.ascast, align 8
  br label %DIR.OMP.END.TARGET.4.split

DIR.OMP.END.TARGET.4.split:                       ; preds = %DIR.OMP.TARGET.3
  br label %DIR.OMP.END.TARGET.4

DIR.OMP.END.TARGET.4:                             ; preds = %DIR.OMP.END.TARGET.4.split
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  br label %DIR.OMP.END.TARGET.5

DIR.OMP.END.TARGET.5:                             ; preds = %DIR.OMP.END.TARGET.4
  ret void
}

attributes #0 = { noinline nounwind optnone uwtable "contains-openmp-target"="true" "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { nounwind readnone "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "openmp-target-declare"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind readnone }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!3}

!0 = !{i32 0, i32 57, i32 -701343393, !"_Z3foov", i32 16, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{}
!3 = !{!"clang version 9.0.0"}
