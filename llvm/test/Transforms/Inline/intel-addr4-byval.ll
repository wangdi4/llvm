; RUN: opt -S -passes=always-inline %s | FileCheck %s
; The function "quux" is calling a byval arg with a different address space.
; When promoting the arg to stack memory, it must be casted to the target space
; for consistency.

source_filename = "test-reduce-tuplemaxloc.cpp"
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

%struct.barney = type { i64, i64 }

$_ZNK4RAJA15TargetReduceLocINS_3omp6maxlocId7Index2DEEdS3_E6reduceEdS3_ = comdat any

$_ZN4RAJA3omp6maxlocId7Index2DEclERdRS2_dS2_ = comdat any

define hidden spir_func void @quux() local_unnamed_addr #0 comdat($_ZNK4RAJA15TargetReduceLocINS_3omp6maxlocId7Index2DEEdS3_E6reduceEdS3_) align 2 {
; CHECK-LABEL: @quux(
; CHECK:         [[TMP0:%.*]] = alloca [[STRUCT_BARNEY:%.*]], align 8
; CHECK:         [[TMP1:%.*]] = addrspacecast %struct.barney* [[TMP0]] to [[STRUCT_BARNEY]] addrspace(4)*
; CHECK:         [[TMP3:%.*]] = bitcast [[STRUCT_BARNEY]] addrspace(4)* [[TMP1]] to i8 addrspace(4)*
; CHECK:         call void @llvm.memcpy.p4i8.p4i8.i64(i8 addrspace(4)* align 1 [[TMP3]], i8 addrspace(4)* align 1 undef, i64 16, i1 false)
; CHECK:         [[TMP_I:%.*]] = getelementptr inbounds [[STRUCT_BARNEY]], [[STRUCT_BARNEY]] addrspace(4)* [[TMP1]], i64 0, i32 0
;
bb:
  call spir_func void @wobble(%struct.barney addrspace(4)* byval(%struct.barney) align 8 undef) #2
  ret void
}

; Function Attrs: alwaysinline
define hidden spir_func void @wobble(%struct.barney addrspace(4)* %arg) local_unnamed_addr #1 comdat($_ZN4RAJA3omp6maxlocId7Index2DEclERdRS2_dS2_) align 2 {
bb:
  %tmp = getelementptr inbounds %struct.barney, %struct.barney addrspace(4)* %arg, i64 0, i32 0
  ret void
}

attributes #0 = { "unsafe-fp-math"="true" }
attributes #1 = { alwaysinline }
attributes #2 = { convergent }

!llvm.ident = !{!0}

!0 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2021.4.0 (2021.x.0.YYYYMMDD)"}
