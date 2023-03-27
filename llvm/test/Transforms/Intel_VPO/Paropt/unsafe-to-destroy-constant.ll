; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -switch-to-offload -vpo-paropt -S %s | FileCheck %s
; RUN: opt -opaque-pointers=0 -switch-to-offload -passes="vpo-paropt" -S %s | FileCheck %s
;
; CHECK-NOT: @"sigma_kernel_$AQSN"
; CHECK-NOT: @"sigma_kernel_$AQSM"
; CHECK-NOT: define void @MAIN__()

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64"
target device_triples = "spir64"

%"QNCA_a0$%complex_128bit addrspace(4)*$rank2$" = type { %complex_128bit addrspace(4)*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }
%complex_128bit = type { double, double }

@"sigma_kernel_$AQSN" = internal addrspace(1) global %"QNCA_a0$%complex_128bit addrspace(4)*$rank2$" { %complex_128bit addrspace(4)* null, i64 0, i64 0, i64 1073741952, i64 2, i64 0, [2 x { i64, i64, i64 }] zeroinitializer }
@"sigma_kernel_$AQSM" = internal addrspace(1) global %"QNCA_a0$%complex_128bit addrspace(4)*$rank2$" { %complex_128bit addrspace(4)* null, i64 0, i64 0, i64 1073741952, i64 2, i64 0, [2 x { i64, i64, i64 }] zeroinitializer }

define void @MAIN__() #0 {
alloca:
  %"var$2" = alloca [8 x i64], align 8
  ret void
}

attributes #0 = { "min-legal-vector-width"="0" }

!omp_offload.info = !{}
