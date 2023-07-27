; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -opaque-pointers=0 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -S %s | FileCheck %s
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

define dso_local i32 @main() {
entry:
  %retval = alloca i32, align 4
  %0 = addrspacecast i32* %retval to i32 addrspace(4)*
  %x = alloca i32, align 4
  %1 = addrspacecast i32* %x to i32 addrspace(4)*
  store i32 0, i32 addrspace(4)* %0, align 4
  store i32 1, i32 addrspace(4)* %1, align 4

  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.FROM"(i32 addrspace(4)* addrspacecast (i32 addrspace(1)* @y to i32 addrspace(4)*), i32 addrspace(4)* addrspacecast (i32 addrspace(1)* @y to i32 addrspace(4)*), i64 4, i64 34, i8* null, i8* null),
    "QUAL.OMP.MAP.TO"(i32 addrspace(4)* %1, i32 addrspace(4)* %1, i64 4, i64 33, i8* null, i8* null) ]

  %3 = load i32, i32 addrspace(4)* %1, align 4
  %add = add nsw i32 %3, 1
  store i32 %add, i32 addrspace(4)* addrspacecast (i32 addrspace(1)* @y to i32 addrspace(4)*), align 4
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TARGET"() ]

  %4 = load i32, i32 addrspace(4)* %1, align 4
  %5 = load i32, i32 addrspace(4)* addrspacecast (i32 addrspace(1)* @y to i32 addrspace(4)*), align 4
  %call = call spir_func i32 (i8 addrspace(4)*, ...) @printf(i8 addrspace(4)* getelementptr inbounds ([52 x i8], [52 x i8] addrspace(4)* addrspacecast ([52 x i8] addrspace(1)* @.str to [52 x i8] addrspace(4)*), i64 0, i64 0), i32 %4, i32 %5)
  ret i32 0
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare dso_local spir_func i32 @printf(i8 addrspace(4)*, ...)

!omp_offload.info = !{!0}
!0 = !{i32 0, i32 2052, i32 85985690, !"main", i32 6, i32 0, i32 0}
