; This test checks that the post-link tool generates a table file with IR files
; for OpenMP offload if parallel compilation is executed.
; First file in the table should contain externalized global variables.
; Each other IR file should contain a kernel functions with dependencies.

; RUN: sycl-post-link -ompoffload-link-entries -split=kernel -S %s -o %t.table
; RUN: FileCheck %s -input-file=%t.table --check-prefix=CHECK-TABLE
; RUN: FileCheck %s -input-file=%t_globals_0.ll --check-prefix=CHECK-GLOB
; RUN: FileCheck %s -input-file=%t_1.ll --check-prefix=CHECK-KERN2 --implicit-check-not=@brr
; RUN: FileCheck %s -input-file=%t_2.ll --check-prefix=CHECK-KERN1 --implicit-check-not=@arr

; CHECK-TABLE: [Code]
; CHECK-TABLE-NEXT: {{.*}}_globals_0.ll
; CHECK-TABLE-NEXT: {{.*}}_1.ll
; CHECK-TABLE-NEXT: {{.*}}_2.ll

; CHECK-GLOB: @arr = protected target_declare addrspace(1) global i32 0, align 4
; CHECK-GLOB-NEXT: @brr = dso_local addrspace(1) global i32 0, align 4
; CHECK-GLOB-NEXT: @__omp_offloading_fc09_26904c__Z4main_l19_kernel_info = weak target_declare local_unnamed_addr addrspace(1) constant
; CHECK-GLOB-NEXT: @__omp_offloading_fc09_26904c__Z4main_l24_kernel_info = weak target_declare local_unnamed_addr addrspace(1) constant
; CHECK-GLOB-NEXT: @.omp_offloading.entry_name = dso_local target_declare unnamed_addr addrspace(2) constant [7 x i8] c"_Z3arr\00"
; CHECK-GLOB-NEXT: @.omp_offloading.entry_name.1 = dso_local target_declare unnamed_addr addrspace(2) constant [7 x i8] c"_Z3brr\00"
; CHECK-GLOB-NEXT: @.omp_offloading.entry_name.2 = dso_local target_declare unnamed_addr addrspace(2) constant [41 x i8] c"__omp_offloading_fc09_26904c__Z4main_l19\00"
; CHECK-GLOB-NEXT: @.omp_offloading.entry_name.3 = dso_local target_declare unnamed_addr addrspace(2) constant [41 x i8] c"__omp_offloading_fc09_26904c__Z4main_l24\00"
; CHECK-GLOB-NEXT: @__omp_offloading_entries_table = addrspace(1) constant [4 x %struct.__tgt_offload_entry]
; CHECK-GLOB-NEXT: @__omp_offloading_entries_table_size = addrspace(1) constant i64
; CHECK-GLOB-NOT: define weak dso_local spir_kernel void @{{.*}}

; CHECK-KERN1-DAG: @arr = external protected target_declare addrspace(1) global i32, align 4
; CHECK-KERN1-DAG: define weak dso_local spir_kernel void @__omp_offloading_fc09_26904c__Z4main_l19
; CHECK-KERN1-DAG:   call void @start_wrapper()
; CHECK-KERN1-DAG:   call spir_func i64 @_Z12get_local_idj(ptr addrspace(1) @arr)
; CHECK-KERN1-DAG: declare spir_func i64 @_Z12get_local_idj(ptr addrspace(1))
; CHECK-KERN1-DAG: define weak protected spir_func void @start_wrapper()

; CHECK-KERN2-DAG: @brr = external dso_local addrspace(1) global i32, align 4
; CHECK-KERN2-DAG: define weak dso_local spir_kernel void @__omp_offloading_fc09_26904c__Z4main_l24
; CHECK-KERN2-DAG:   call void @start_wrapper()
; CHECK-KERN2-DAG:   call spir_func i64 @_Z12get_local_idj(ptr addrspace(1) @brr)
; CHECK-KERN2-DAG: declare spir_func i64 @_Z12get_local_idj(ptr addrspace(1))
; CHECK-KERN2-DAG: define weak protected spir_func void @start_wrapper()

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"

%0 = type { i32, i32, [2 x %1], i64, i64, i64 }
%1 = type { i32, i32 }
%struct.__tgt_offload_entry = type { ptr addrspace(4), ptr addrspace(2), i64, i32, i32, i64 }

@arr = protected target_declare addrspace(1) global i32 0, align 4
@brr = internal addrspace(1) global i32 0, align 4
@__omp_offloading_fc09_26904c__Z4main_l19_kernel_info = weak target_declare local_unnamed_addr addrspace(1) constant %0 { i32 5, i32 2, [2 x %1] [%1 { i32 0, i32 8 }, %1 { i32 0, i32 8 }], i64 0, i64 0, i64 0 }
@__omp_offloading_fc09_26904c__Z4main_l24_kernel_info = weak target_declare local_unnamed_addr addrspace(1) constant %0 { i32 5, i32 2, [2 x %1] [%1 { i32 0, i32 8 }, %1 { i32 0, i32 8 }], i64 0, i64 0, i64 0 }
@.omp_offloading.entry_name = internal target_declare unnamed_addr addrspace(2) constant [7 x i8] c"_Z3arr\00"
@.omp_offloading.entry._Z3arr = weak target_declare local_unnamed_addr addrspace(1) constant %struct.__tgt_offload_entry { ptr addrspace(4) addrspacecast (ptr addrspace(1) bitcast (ptr addrspace(1) @arr to ptr addrspace(1)) to ptr addrspace(4)), ptr addrspace(2) getelementptr inbounds ([7 x i8], ptr addrspace(2) @.omp_offloading.entry_name, i32 0, i32 0), i64 4, i32 0, i32 0, i64 7 }, section "omp_offloading_entries"
@.omp_offloading.entry_name.1 = internal target_declare unnamed_addr addrspace(2) constant [7 x i8] c"_Z3brr\00"
@.omp_offloading.entry._Z3brr = weak target_declare local_unnamed_addr addrspace(1) constant %struct.__tgt_offload_entry { ptr addrspace(4) addrspacecast (ptr addrspace(1) bitcast (ptr addrspace(1) @brr to ptr addrspace(1)) to ptr addrspace(4)), ptr addrspace(2) getelementptr inbounds ([7 x i8], ptr addrspace(2) @.omp_offloading.entry_name.1, i32 0, i32 0), i64 4, i32 0, i32 0, i64 7 }, section "omp_offloading_entries"
@.omp_offloading.entry_name.2 = internal target_declare unnamed_addr addrspace(2) constant [41 x i8] c"__omp_offloading_fc09_26904c__Z4main_l19\00"
@.omp_offloading.entry.__omp_offloading_fc09_26904c__Z4main_l19 = weak target_declare local_unnamed_addr addrspace(1) constant %struct.__tgt_offload_entry { ptr addrspace(4) null, ptr addrspace(2) getelementptr inbounds ([41 x i8], ptr addrspace(2) @.omp_offloading.entry_name.2, i32 0, i32 0), i64 0, i32 0, i32 0, i64 41 }, section "omp_offloading_entries"
@.omp_offloading.entry_name.3 = internal target_declare unnamed_addr addrspace(2) constant [41 x i8] c"__omp_offloading_fc09_26904c__Z4main_l24\00"
@.omp_offloading.entry.__omp_offloading_fc09_26904c__Z4main_l24 = weak target_declare local_unnamed_addr addrspace(1) constant %struct.__tgt_offload_entry { ptr addrspace(4) null, ptr addrspace(2) getelementptr inbounds ([41 x i8], ptr addrspace(2) @.omp_offloading.entry_name.3, i32 0, i32 0), i64 0, i32 0, i32 0, i64 41 }, section "omp_offloading_entries"

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn
define protected spir_func void @set_arr(i32 noundef %pos) local_unnamed_addr #0 {
entry:
  %0 = load i32, ptr addrspace(4) addrspacecast (ptr addrspace(1) @arr to ptr addrspace(4)), align 4, !tbaa !9
  %add = add nsw i32 %0, %pos
  store i32 %add, ptr addrspace(4) addrspacecast (ptr addrspace(1) @arr to ptr addrspace(4)), align 4, !tbaa !9
  %1 = load i32, ptr addrspace(4) addrspacecast (ptr addrspace(1) @brr to ptr addrspace(4)), align 4, !tbaa !9
  %add1 = add nsw i32 %1, %pos
  store i32 %add1, ptr addrspace(4) addrspacecast (ptr addrspace(1) @brr to ptr addrspace(4)), align 4, !tbaa !9
  ret void
}

; Function Attrs: noinline nounwind
define weak dso_local spir_kernel void @__omp_offloading_fc09_26904c__Z4main_l19(ptr addrspace(1) %arr, ptr addrspace(1) %brr) local_unnamed_addr #1 {
DIR.OMP.TARGET.3:
  call void @start_wrapper()
  %0 = tail call spir_func i64 @_Z12get_local_idj(ptr addrspace(1) @arr) #3
  ret void
}

declare spir_func i64 @_Z12get_local_idj(ptr addrspace(1)) local_unnamed_addr

; Function Attrs: convergent nounwind
declare spir_func void @_Z22__spirv_ControlBarrieriii(i32, i32, i32) #2

; Function Attrs: noinline nounwind
define weak dso_local spir_kernel void @__omp_offloading_fc09_26904c__Z4main_l24(ptr addrspace(1) %arr20, ptr addrspace(1) %brr21) local_unnamed_addr #1 {
DIR.OMP.TARGET.7:
  call void @start_wrapper()
  %0 = tail call spir_func i64 @_Z12get_local_idj(ptr addrspace(1) @brr) #3
  ret void
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn
define weak protected spir_func void @start_wrapper() #0 {
entry:
  %GroupID = alloca [3 x i64], align 8
  %call.i = call spir_func signext i8 @__spirv_SpecConstant(i32 noundef -9145239, i8 noundef signext 0) #2
  ret void
}

; Function Attrs: convergent
declare extern_weak spir_func signext i8 @__spirv_SpecConstant(i32 noundef, i8 noundef signext) local_unnamed_addr #2

attributes #0 = { mustprogress nofree norecurse nosync nounwind willreturn "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="all" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "openmp-target-declare"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #1 = { noinline nounwind "approx-func-fp-math"="true" "contains-openmp-target"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="all" "loopopt-pipeline"="light" "may-have-openmp-directive"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "processed-by-vpo" "stack-protector-buffer-size"="8" "target.declare"="true" "unsafe-fp-math"="true" }
attributes #2 = { convergent nounwind }
attributes #3 = { nounwind }

!opencl.compiler.options = !{!0}
!llvm.ident = !{!1}
!spirv.Source = !{!2}
!nvvm.annotations = !{}
!spirv.MemoryModel = !{!3}
!spirv.ExecutionMode = !{}
!llvm.module.flags = !{!4, !5, !6, !7, !8}

!0 = !{}
!1 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!2 = !{i32 4, i32 200000}
!3 = !{i32 2, i32 2}
!4 = !{i32 1, !"wchar_size", i32 4}
!5 = !{i32 7, !"openmp", i32 50}
!6 = !{i32 7, !"openmp-device", i32 50}
!7 = !{i32 7, !"PIC Level", i32 2}
!8 = !{i32 7, !"frame-pointer", i32 2}
!9 = !{!10, !10, i64 0}
!10 = !{!"int", !11, i64 0}
!11 = !{!"omnipotent char", !12, i64 0}
!12 = !{!"Simple C/C++ TBAA"}
