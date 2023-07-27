; RUN: opt -bugpoint-enable-legacy-pm -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='vpo-paropt' -S %s | FileCheck %s
;
; This test checks that VPOParopt translates the calls
;   sincosf(a,&s,&c)   into   s = @_Z18__spirv_ocl_sincosfPf(a,&c)
;   sincos (a,&s,&c)   into   s = @_Z18__spirv_ocl_sincosdPd(a,&c)

;
; #include <stdio.h>
; #include <math.h>
; void foo() {
;   #pragma omp target
;   {
;     float  fs, fc;
;     double ds, dc;
;     ::sincosf(2.0f, &fs, &fc);
;     ::sincos (3.0,  &ds, &dc);
;   }
; }

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

define hidden spir_func void @_Z3foov() #0 {
entry:
  %fs = alloca float, align 4
  %fc = alloca float, align 4
  %ds = alloca double, align 8
  %dc = alloca double, align 8
  %fs.ascast = addrspacecast ptr %fs to ptr addrspace(4)
  %fc.ascast = addrspacecast ptr %fc to ptr addrspace(4)
  %ds.ascast = addrspacecast ptr %ds to ptr addrspace(4)
  %dc.ascast = addrspacecast ptr %dc to ptr addrspace(4)

  call spir_func void @sincosf(float noundef 2.000000e+00, ptr addrspace(4) noundef %fs.ascast, ptr addrspace(4) noundef %fc.ascast)
;
; CHECK: [[SINE0:%[^ ]+]] = call spir_func float @_Z18__spirv_ocl_sincosfPf(float 2.000000e+00, ptr addrspace(4) %fc.ascast)
; CHECK: store float [[SINE0]], ptr addrspace(4) %fs.ascast, align 4

  call spir_func void @sincos(double noundef 3.000000e+00, ptr addrspace(4) noundef %ds.ascast, ptr addrspace(4) noundef %dc.ascast)
;
; CHECK: [[SINE1:%[^ ]+]] = call spir_func double @_Z18__spirv_ocl_sincosdPd(double 3.000000e+00, ptr addrspace(4) %dc.ascast)
; CHECK: store double [[SINE1]], ptr addrspace(4) %ds.ascast, align 8

  ret void
}

declare spir_func void @sincosf(float noundef, ptr addrspace(4) noundef, ptr addrspace(4) noundef)
;
; CHECK: declare dso_local spir_func float @_Z18__spirv_ocl_sincosfPf(float, ptr addrspace(4))
;

declare spir_func void @sincos(double noundef, ptr addrspace(4) noundef, ptr addrspace(4) noundef)
;
; CHECK: declare dso_local spir_func double @_Z18__spirv_ocl_sincosdPd(double, ptr addrspace(4))
;

attributes #0 = { "openmp-target-declare"="true" }
