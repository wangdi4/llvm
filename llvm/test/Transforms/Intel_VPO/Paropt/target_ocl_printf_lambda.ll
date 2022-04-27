; RUN: opt -switch-to-offload -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -S %s | FileCheck %s

; Test to check that we replace all printf's in spir64 code with ocl_printf,
; and not just those inside the target region.
; Test src:

; #include <stdio.h>
; #include <type_traits>
;
; template <typename Func> void run_on_tgt(Func &&func_body) {
;
;   using Body = typename std::remove_reference<decltype(func_body)>::type;
;   Body body = func_body;
;   int i = 10;
;
; #pragma omp target map(to : body, i)
;   {
;     printf("In tgt.\n");
;     body(i);
;   }
; }
;
; int main() {
;   run_on_tgt([=](int i) { printf("i = %d.\n", i); });
; }

; ModuleID = 'tgt_lambda.cpp'
source_filename = "tgt_lambda.cpp"
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64"
target device_triples = "spir64"

%"class._ZTSZ4mainE3$_0" = type { i8 }

@.str = private unnamed_addr addrspace(1) constant [9 x i8] c"In tgt.\0A\00", align 1
@.str.1 = private unnamed_addr addrspace(1) constant [9 x i8] c"i = %d.\0A\00", align 1

; Function Attrs: noinline nounwind optnone uwtable
define internal spir_func void @"_Z10run_on_tgtIZ4mainE3$_0EvOT_"(%"class._ZTSZ4mainE3$_0" addrspace(4)* dereferenceable(1) %func_body) #0 {
entry:
  %func_body.addr = alloca %"class._ZTSZ4mainE3$_0" addrspace(4)*, align 8
  %func_body.addr.ascast = addrspacecast %"class._ZTSZ4mainE3$_0" addrspace(4)** %func_body.addr to %"class._ZTSZ4mainE3$_0" addrspace(4)* addrspace(4)*
  %body = alloca %"class._ZTSZ4mainE3$_0", align 1
  %body.ascast = addrspacecast %"class._ZTSZ4mainE3$_0"* %body to %"class._ZTSZ4mainE3$_0" addrspace(4)*
  %i = alloca i32, align 4
  %i.ascast = addrspacecast i32* %i to i32 addrspace(4)*
  store %"class._ZTSZ4mainE3$_0" addrspace(4)* %func_body, %"class._ZTSZ4mainE3$_0" addrspace(4)* addrspace(4)* %func_body.addr.ascast, align 8
  %0 = load %"class._ZTSZ4mainE3$_0" addrspace(4)*, %"class._ZTSZ4mainE3$_0" addrspace(4)* addrspace(4)* %func_body.addr.ascast, align 8
  store i32 10, i32 addrspace(4)* %i.ascast, align 4

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.TO"(%"class._ZTSZ4mainE3$_0" addrspace(4)* %body.ascast, %"class._ZTSZ4mainE3$_0" addrspace(4)* %body.ascast, i64 1, i64 33), "QUAL.OMP.MAP.TO"(i32 addrspace(4)* %i.ascast, i32 addrspace(4)* %i.ascast, i64 4, i64 33) ]

  %call = call spir_func i32 (i8 addrspace(4)*, ...) @printf(i8 addrspace(4)* getelementptr inbounds ([9 x i8], [9 x i8] addrspace(4)* addrspacecast ([9 x i8] addrspace(1)* @.str to [9 x i8] addrspace(4)*), i64 0, i64 0)) #1
; Make sure that call to printf is replaced with ocl_printf
; CHECK-NOT: call {{.*}} (i8 addrspace(4)*, ...) @printf(i8 addrspace(4)* {{.*}})
; CHECK: call {{.*}} (i8 addrspace(2)*, ...) @_Z18__spirv_ocl_printfPU3AS2ci(i8 addrspace(2)* {{.*}})

  %2 = load i32, i32 addrspace(4)* %i.ascast, align 4
  call spir_func void @"_ZZ4mainENK3$_0clEi"(%"class._ZTSZ4mainE3$_0" addrspace(4)* %body.ascast, i32 %2) #1

  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

declare dso_local spir_func i32 @printf(i8 addrspace(4)*, ...) #2
; Check that the declaration of orifinal printf is deleted, and declaration of ocl_printf is added.
; CHECK-NOT: declare dso_local spir_func i32 @printf(i8 addrspace(4)*, ...)
; CHECK: declare dso_local spir_func i32 @_Z18__spirv_ocl_printfPU3AS2ci(i8 addrspace(2)*, ...)

; Function Attrs: noinline nounwind optnone uwtable
define internal spir_func void @"_ZZ4mainENK3$_0clEi"(%"class._ZTSZ4mainE3$_0" addrspace(4)* %this, i32 %i) #3 align 2 {
entry:
  %this.addr = alloca %"class._ZTSZ4mainE3$_0" addrspace(4)*, align 8
  %this.addr.ascast = addrspacecast %"class._ZTSZ4mainE3$_0" addrspace(4)** %this.addr to %"class._ZTSZ4mainE3$_0" addrspace(4)* addrspace(4)*
  %i.addr = alloca i32, align 4
  %i.addr.ascast = addrspacecast i32* %i.addr to i32 addrspace(4)*
  store %"class._ZTSZ4mainE3$_0" addrspace(4)* %this, %"class._ZTSZ4mainE3$_0" addrspace(4)* addrspace(4)* %this.addr.ascast, align 8
  store i32 %i, i32 addrspace(4)* %i.addr.ascast, align 4
  %this1 = load %"class._ZTSZ4mainE3$_0" addrspace(4)*, %"class._ZTSZ4mainE3$_0" addrspace(4)* addrspace(4)* %this.addr.ascast, align 8
  %0 = load i32, i32 addrspace(4)* %i.addr.ascast, align 4

  %call = call spir_func i32 (i8 addrspace(4)*, ...) @printf(i8 addrspace(4)* getelementptr inbounds ([9 x i8], [9 x i8] addrspace(4)* addrspacecast ([9 x i8] addrspace(1)* @.str.1 to [9 x i8] addrspace(4)*), i64 0, i64 0), i32 %0)
; Make sure that call to printf is replaced with ocl_printf
; CHECK-NOT: call {{.*}} (i8 addrspace(4)*, ...) @printf(i8 addrspace(4)* {{.*}})
; CHECK: call {{.*}} (i8 addrspace(2)*, ...) @_Z18__spirv_ocl_printfPU3AS2ci(i8 addrspace(2)* {{.*}})

  ret void
}

attributes #0 = { noinline nounwind optnone uwtable "contains-openmp-target"="true" "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "openmp-target-declare"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "openmp-target-declare"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!3}

!0 = !{i32 0, i32 2055, i32 12062966, !"_Z10run_on_tgtIZ4mainE3$_0EvOT_", i32 10, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{}
!3 = !{!"clang version 11.0.0"}
