; REQUIRES: asserts
; RUN: opt -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -debug-only=WRegionUtils,vpo-paropt-transform -vpo-paropt-opt-scalar-fp=false -S %s 2>&1 | FileCheck %s
; RUN: opt -switch-to-offload -aa-pipeline=basic-aa -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -debug-only=WRegionUtils,vpo-paropt-transform -vpo-paropt-opt-scalar-fp=false -S %s 2>&1 | FileCheck %s

; Test src:

; #include <omp.h>
; #include <stdio.h>
; int main()
; {
;   int n_dims = 2;
;   int tmp[n_dims];
;   tmp[0] = 10;
;
; #pragma omp target defaultmap(tofrom:scalar)
;   {
; #pragma omp parallel private(tmp)
;     {
;       printf("%d\n", tmp[0]);
;     }
;   }
;   printf("%d\n", tmp[1]);
; }

; This test contains the host IR corresponding to target_par_priv_vla_host.ll.

; Check that we capture the VLA size expression at the target construct.
; CHECK: VPOParopt Transform: TARGET construct
; CHECK: collectNonPointerValuesToBeUsedInOutlinedRegion: Non-pointer values to be passed into the outlined region: 'i64 %1 '
; CHECK: captureAndAddCollectedNonPointerValuesToSharedClause: Added implicit shared/map(to) clause for: 'i64 addrspace(4)* [[SIZE_ADDR:%[^ ]+]]'

; Check that the captured VLA size is used in the target region for allocation of the private VLA.
; CHECK: define weak dso_local spir_kernel void @__omp_offloading{{.*}}main{{.*}}(i64 addrspace(1)* noalias [[SIZE_ADDR]], i32 addrspace(1)* %{{.*}}, i64 addrspace(1)* %{{.*}})
; CHECK: [[SIZE_VAL:%[^ ]+]] = load i64, i64 addrspace(1)* [[SIZE_ADDR]], align 8
; CHECK: %{{.*}} = alloca i32, i64 [[SIZE_VAL]], align 1

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

@.str = private unnamed_addr addrspace(1) constant [4 x i8] c"%d\0A\00", align 1

; Function Attrs: noinline nounwind optnone uwtable
define hidden i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %retval.ascast = addrspacecast i32* %retval to i32 addrspace(4)*
  %n_dims = alloca i32, align 4
  %n_dims.ascast = addrspacecast i32* %n_dims to i32 addrspace(4)*
  %saved_stack = alloca i8*, align 8
  %saved_stack.ascast = addrspacecast i8** %saved_stack to i8* addrspace(4)*
  %__vla_expr0 = alloca i64, align 8
  %__vla_expr0.ascast = addrspacecast i64* %__vla_expr0 to i64 addrspace(4)*
  %omp.vla.tmp = alloca i64, align 8
  %omp.vla.tmp.ascast = addrspacecast i64* %omp.vla.tmp to i64 addrspace(4)*
  %omp.vla.tmp1 = alloca i64, align 8
  %omp.vla.tmp1.ascast = addrspacecast i64* %omp.vla.tmp1 to i64 addrspace(4)*
  store i32 2, i32 addrspace(4)* %n_dims.ascast, align 4
  %0 = load i32, i32 addrspace(4)* %n_dims.ascast, align 4
  %1 = zext i32 %0 to i64
  %2 = call i8* @llvm.stacksave()
  store i8* %2, i8* addrspace(4)* %saved_stack.ascast, align 8
  %vla = alloca i32, i64 %1, align 4
  %vla.ascast = addrspacecast i32* %vla to i32 addrspace(4)*
  store i64 %1, i64 addrspace(4)* %__vla_expr0.ascast, align 8
  %ptridx = getelementptr inbounds i32, i32 addrspace(4)* %vla.ascast, i64 0
  store i32 10, i32 addrspace(4)* %ptridx, align 4
  store i64 %1, i64 addrspace(4)* %omp.vla.tmp.ascast, align 8


  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.DEFAULTMAP.TOFROM:SCALAR"(), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %vla.ascast), "QUAL.OMP.FIRSTPRIVATE"(i64 addrspace(4)* %omp.vla.tmp.ascast), "QUAL.OMP.PRIVATE"(i64 addrspace(4)* %omp.vla.tmp1.ascast) ]


  %4 = load i64, i64 addrspace(4)* %omp.vla.tmp.ascast, align 8
  store i64 %4, i64 addrspace(4)* %omp.vla.tmp1.ascast, align 8
  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %vla.ascast), "QUAL.OMP.SHARED"(i64 addrspace(4)* %omp.vla.tmp1.ascast) ]


  %6 = load i64, i64 addrspace(4)* %omp.vla.tmp1.ascast, align 8
  %ptridx2 = getelementptr inbounds i32, i32 addrspace(4)* %vla.ascast, i64 0
  %7 = load i32, i32 addrspace(4)* %ptridx2, align 4
  %call = call spir_func i32 (i8 addrspace(4)*, ...) @printf(i8 addrspace(4)* getelementptr inbounds ([4 x i8], [4 x i8] addrspace(4)* addrspacecast ([4 x i8] addrspace(1)* @.str to [4 x i8] addrspace(4)*), i64 0, i64 0), i32 %7) #1


  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.PARALLEL"() ]
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.TARGET"() ]


  %ptridx3 = getelementptr inbounds i32, i32 addrspace(4)* %vla.ascast, i64 1
  %8 = load i32, i32 addrspace(4)* %ptridx3, align 4
  %call4 = call spir_func i32 (i8 addrspace(4)*, ...) @printf(i8 addrspace(4)* getelementptr inbounds ([4 x i8], [4 x i8] addrspace(4)* addrspacecast ([4 x i8] addrspace(1)* @.str to [4 x i8] addrspace(4)*), i64 0, i64 0), i32 %8)
  %9 = load i8*, i8* addrspace(4)* %saved_stack.ascast, align 8
  call void @llvm.stackrestore(i8* %9)
  ret i32 0
}

; Function Attrs: nounwind
declare i8* @llvm.stacksave() #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

declare dso_local spir_func i32 @printf(i8 addrspace(4)*, ...) #2

; Function Attrs: nounwind
declare void @llvm.stackrestore(i8*) #1

attributes #0 = { noinline nounwind optnone uwtable "contains-openmp-target"="true" "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "openmp-target-declare"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!3}

!0 = !{i32 0, i32 2055, i32 150358973, !"_Z4main", i32 9, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{}
!3 = !{!"clang version 10.0.0"}
