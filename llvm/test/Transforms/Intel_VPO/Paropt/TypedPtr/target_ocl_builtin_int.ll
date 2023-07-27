; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -vpo-paropt -switch-to-offload -S %s | FileCheck %s
; RUN: opt -opaque-pointers=0 -passes='vpo-paropt' -switch-to-offload -S %s | FileCheck %s

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

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

declare dso_local spir_func i32 @abs(i32)
; CHECK: declare {{.*}} i32 @_Z17__spirv_ocl_s_absi(i32)

declare dso_local spir_func i64 @labs(i64)
; CHECK: declare {{.*}} i64 @_Z17__spirv_ocl_s_absl(i64)

define hidden spir_func void @_Z3foov() {
entry:
  %aa = alloca i32, align 4
  %aa.ascast = addrspacecast i32* %aa to i32 addrspace(4)*
  %bb = alloca i64, align 8
  %bb.ascast = addrspacecast i64* %bb to i64 addrspace(4)*
  br label %DIR.OMP.TARGET.1

DIR.OMP.TARGET.1:                                 ; preds = %entry
  br label %DIR.OMP.TARGET.2

DIR.OMP.TARGET.2:                                 ; preds = %DIR.OMP.TARGET.1
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.PRIVATE:WILOCAL"(i32 addrspace(4)* %aa.ascast),
    "QUAL.OMP.PRIVATE:WILOCAL"(i64 addrspace(4)* %bb.ascast) ]
  br label %DIR.OMP.TARGET.35

DIR.OMP.TARGET.35:                                ; preds = %DIR.OMP.TARGET.2
  br label %DIR.OMP.TARGET.3

DIR.OMP.TARGET.3:                                 ; preds = %DIR.OMP.TARGET.35

  %call = call spir_func i32 @abs(i32 123)
; CHECK:  call spir_func i32 @_Z17__spirv_ocl_s_absi(i32 123)

  store i32 %call, i32 addrspace(4)* %aa.ascast, align 4

  %call1 = call spir_func i64 @labs(i64 456)
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

!omp_offload.info = !{!0}
!0 = !{i32 0, i32 57, i32 -701343393, !"_Z3foov", i32 16, i32 0, i32 0}
