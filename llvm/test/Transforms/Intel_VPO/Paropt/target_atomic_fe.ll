; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s | FileCheck %s
;
; Test src:
;
; #include <stdio.h>
;
; int main(void)
; {
;   int x = 0;
;   int y = 110;
;
; #pragma omp target map(tofrom:x) map(to: y)
;   {
; #pragma omp atomic read
;     x = y;
; #pragma omp atomic write
;     x = 100;
; #pragma omp atomic
;     x = x + 1;
;   }
;
;   printf("x = %d\n", (int) x);
;   return 0;
; }
;
; ModuleID = 'target_atomic_scalar.c'
source_filename = "target_atomic_scalar.c"
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64"
target device_triples = "spir64"

@.str = private unnamed_addr constant [8 x i8] c"x = %d\0A\00", align 1

; Function Attrs: noinline norecurse nounwind optnone uwtable
define dso_local i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %x = alloca i32, align 4
  %y = alloca i32, align 4
  %atomic-temp = alloca i32, align 4
  %atomic-temp1 = alloca i32, align 4
  %atomic-temp2 = alloca i32, align 4
  %atomic-temp3 = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  store i32 0, i32* %x, align 4
  store i32 110, i32* %y, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.TOFROM"(i32* %x), "QUAL.OMP.MAP.TO"(i32* %y), "QUAL.OMP.PRIVATE"(i32* %atomic-temp), "QUAL.OMP.PRIVATE"(i32* %atomic-temp1), "QUAL.OMP.PRIVATE"(i32* %atomic-temp2), "QUAL.OMP.PRIVATE"(i32* %atomic-temp3) ]
  %1 = bitcast i32* %y to i8*
  %2 = bitcast i32* %atomic-temp to i8*

  call void @__atomic_load(i64 4, i8* %1, i8* %2, i32 0)
; CHECK: call void @__kmpc_atomic_load(i64 4, i8 addrspace(4)* {{[^ ]+}}, i8 addrspace(4)* {{[^ ]+}}, i32 0)

  %3 = load i32, i32* %atomic-temp, align 4
  store i32 %3, i32* %x, align 4
  store i32 100, i32* %atomic-temp1, align 4
  %4 = bitcast i32* %x to i8*
  %5 = bitcast i32* %atomic-temp1 to i8*

  call void @__atomic_store(i64 4, i8* %4, i8* %5, i32 0)
; CHECK: call void @__kmpc_atomic_store(i64 4, i8 addrspace(4)* {{[^ ]+}}, i8 addrspace(4)* {{[^ ]+}}, i32 0)

  %6 = bitcast i32* %x to i8*
  %7 = bitcast i32* %atomic-temp2 to i8*

  call void @__atomic_load(i64 4, i8* %6, i8* %7, i32 0)
; CHECK: call void @__kmpc_atomic_load(i64 4, i8 addrspace(4)* {{[^ ]+}}, i8 addrspace(4)* {{[^ ]+}}, i32 0)

  br label %atomic_cont

atomic_cont:                                      ; preds = %atomic_cont, %entry
  %8 = load i32, i32* %atomic-temp2, align 4
  %add = add nsw i32 %8, 1
  store i32 %add, i32* %atomic-temp3, align 4
  %9 = bitcast i32* %x to i8*
  %10 = bitcast i32* %atomic-temp2 to i8*
  %11 = bitcast i32* %atomic-temp3 to i8*

  %call = call zeroext i1 @__atomic_compare_exchange(i64 4, i8* %9, i8* %10, i8* %11, i32 0, i32 0)
; CHECK: %call = call zeroext i1 @__kmpc_atomic_compare_exchange(i64 4, i8 addrspace(4)* {{[^ ]+}}, i8 addrspace(4)* {{[^ ]+}}, i8 addrspace(4)* {{[^ ]+}}, i32 0, i32 0)

  br i1 %call, label %atomic_exit, label %atomic_cont

atomic_exit:                                      ; preds = %atomic_cont
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  %12 = load i32, i32* %x, align 4
  %call4 = call spir_func i32 (i8*, ...) @printf(i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str, i64 0, i64 0), i32 %12)
  ret i32 0
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

declare dso_local void @__atomic_load(i64, i8*, i8*, i32)

declare dso_local void @__atomic_store(i64, i8*, i8*, i32)

declare dso_local i1 @__atomic_compare_exchange(i64, i8*, i8*, i8*, i32, i32)

declare dso_local spir_func i32 @printf(i8*, ...) #2

; CHECK: declare dso_local void @__kmpc_atomic_load(i64, i8 addrspace(4)*, i8 addrspace(4)*, i32)
; CHECK: declare dso_local void @__kmpc_atomic_store(i64, i8 addrspace(4)*, i8 addrspace(4)*, i32)
; CHECK: declare dso_local i1 @__kmpc_atomic_compare_exchange(i64, i8 addrspace(4)*, i8 addrspace(4)*, i8 addrspace(4)*, i32, i32)


attributes #0 = { noinline norecurse nounwind optnone uwtable "contains-openmp-target"="true" "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!3}

!0 = !{i32 0, i32 2065, i32 561966718, !"main", i32 10, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{}
!3 = !{!"clang version 9.0.0"}
