; This test checks that the post-link tool generates a table file with IR files
; for OpenMP offload if parallel compilation is executed.
; First file in the table should contain externalized global variables and 
; indirectly callable functions.
; Each other IR file should contain a kernel functions with dependencies.

; RUN: sycl-post-link -ompoffload-link-entries -split=kernel -S %s -o %t.table
; RUN: FileCheck %s -input-file=%t.table --check-prefix=CHECK-TABLE
; RUN: FileCheck %s -input-file=%t_globals_0.ll --check-prefix=CHECK-GLOB
; RUN: FileCheck %s -input-file=%t_1.ll --check-prefix=CHECK-KERN1

; CHECK-TABLE: [Code]
; CHECK-TABLE-NEXT: {{.*}}_globals_0.ll
; CHECK-TABLE-NEXT: {{.*}}_1.ll

; CHECK-GLOB: @__omp_offloading_{{.*}}__Z4main_{{.*}}_kernel_info = weak target_declare local_unnamed_addr addrspace(1) constant
; CHECK-GLOB: @.omp_offloading.entry_name.2 = dso_local target_declare unnamed_addr addrspace(2) constant [40 x i8] c"__omp_offloading_805_3c7604__Z4main_l24\00"
; CHECK-GLOB: @__omp_offloading_entries_table = addrspace(1) constant [4 x %struct.__tgt_offload_entry.0]
; CHECK-GLOB-NEXT: @__omp_offloading_entries_table_size = addrspace(1) constant i64

; CHECK-GLOB: define{{.*}}spir_func void @_ZN1A4vfooEv{{.*}} #[[#Attr:]]
; CHECK-GLOB: attributes #[[#Attr]] = {{.*}} "openmp-target-declare"="true" "referenced-indirectly"

; CHECK-KERN1: @__omp_offloading_805_3c7604__Z4main_l24

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64_gen"

%0 = type { i32, i32, [1 x %1], i64, i64, i64 }
%1 = type { i32, i32 }
%struct.__tgt_offload_entry.0 = type { i8 addrspace(4)*, i8 addrspace(2)*, i64, i32, i32, i64 }
%struct.kmp_program_data = type { i32, i32, i32, i32, i32, i64, i64, i32, i8 addrspace(1)*, i32 }
%struct.__omp_offloading_fptr_map_t = type { i64, i64 }
%struct.A = type { i32 (...)* addrspace(4)* }
%struct.B = type { %struct.A }

$_ZN1A4vfooEv = comdat any

$_ZTV1B = comdat any

$_ZTS1B = comdat any

$_ZTS1A = comdat any

$_ZTI1A = comdat any

$_ZTI1B = comdat any

@count = protected target_declare local_unnamed_addr addrspace(1) global i32 0, align 4
@_ZTV1B = linkonce_odr protected target_declare unnamed_addr addrspace(1) constant { [3 x i8 addrspace(4)*] } { [3 x i8 addrspace(4)*] [i8 addrspace(4)* null, i8 addrspace(4)* addrspacecast (i8* bitcast ({ i8 addrspace(4)*, i8 addrspace(4)*, i8 addrspace(4)* }* @_ZTI1B to i8*) to i8 addrspace(4)*), i8 addrspace(4)* addrspacecast (i8* bitcast (void (%struct.A addrspace(4)*)* @_ZN1A4vfooEv to i8*) to i8 addrspace(4)*)] }, comdat, align 8
@_ZTVN10__cxxabiv120__si_class_type_infoE = external addrspace(1) global i8 addrspace(4)*
@_ZTS1B = linkonce_odr protected addrspace(1) constant [3 x i8] c"1B\00", comdat, align 1
@_ZTVN10__cxxabiv117__class_type_infoE = external addrspace(1) global i8 addrspace(4)*
@_ZTS1A = linkonce_odr protected addrspace(1) constant [3 x i8] c"1A\00", comdat, align 1
@_ZTI1A = linkonce_odr protected constant { i8 addrspace(4)*, i8 addrspace(4)* } { i8 addrspace(4)* bitcast (i8 addrspace(4)* addrspace(4)* getelementptr inbounds (i8 addrspace(4)*, i8 addrspace(4)* addrspace(4)* addrspacecast (i8 addrspace(4)* addrspace(1)* @_ZTVN10__cxxabiv117__class_type_infoE to i8 addrspace(4)* addrspace(4)*), i64 2) to i8 addrspace(4)*), i8 addrspace(4)* addrspacecast (i8 addrspace(1)* getelementptr inbounds ([3 x i8], [3 x i8] addrspace(1)* @_ZTS1A, i32 0, i32 0) to i8 addrspace(4)*) }, comdat, align 8
@_ZTI1B = linkonce_odr protected constant { i8 addrspace(4)*, i8 addrspace(4)*, i8 addrspace(4)* } { i8 addrspace(4)* bitcast (i8 addrspace(4)* addrspace(4)* getelementptr inbounds (i8 addrspace(4)*, i8 addrspace(4)* addrspace(4)* addrspacecast (i8 addrspace(4)* addrspace(1)* @_ZTVN10__cxxabiv120__si_class_type_infoE to i8 addrspace(4)* addrspace(4)*), i64 2) to i8 addrspace(4)*), i8 addrspace(4)* addrspacecast (i8 addrspace(1)* getelementptr inbounds ([3 x i8], [3 x i8] addrspace(1)* @_ZTS1B, i32 0, i32 0) to i8 addrspace(4)*), i8 addrspace(4)* addrspacecast (i8* bitcast ({ i8 addrspace(4)*, i8 addrspace(4)* }* @_ZTI1A to i8*) to i8 addrspace(4)*) }, comdat, align 8
@__omp_offloading_805_3c7604__Z4main_l24_kernel_info = weak target_declare local_unnamed_addr addrspace(1) constant %0 { i32 5, i32 1, [1 x %1] [%1 { i32 0, i32 8 }], i64 0, i64 0, i64 0 }
@.omp_offloading.entry_name = internal target_declare unnamed_addr addrspace(2) constant [9 x i8] c"_Z5count\00"
@.omp_offloading.entry._Z5count = weak target_declare local_unnamed_addr addrspace(1) constant %struct.__tgt_offload_entry.0 { i8 addrspace(4)* addrspacecast (i8 addrspace(1)* bitcast (i32 addrspace(1)* @count to i8 addrspace(1)*) to i8 addrspace(4)*), i8 addrspace(2)* getelementptr inbounds ([9 x i8], [9 x i8] addrspace(2)* @.omp_offloading.entry_name, i32 0, i32 0), i64 4, i32 0, i32 0, i64 9 }, section "omp_offloading_entries"
@.omp_offloading.entry_name.2 = internal target_declare unnamed_addr addrspace(2) constant [40 x i8] c"__omp_offloading_805_3c7604__Z4main_l24\00"
@.omp_offloading.entry.__omp_offloading_805_3c7604__Z4main_l24 = weak target_declare local_unnamed_addr addrspace(1) constant %struct.__tgt_offload_entry.0 { i8 addrspace(4)* null, i8 addrspace(2)* getelementptr inbounds ([40 x i8], [40 x i8] addrspace(2)* @.omp_offloading.entry_name.2, i32 0, i32 0), i64 0, i32 0, i32 0, i64 40 }, section "omp_offloading_entries"
@.omp_offloading.entry_name.3 = internal target_declare unnamed_addr addrspace(2) constant [7 x i8] c"_ZTV1B\00"
@.omp_offloading.entry._ZTV1B = weak target_declare local_unnamed_addr addrspace(1) constant %struct.__tgt_offload_entry.0 { i8 addrspace(4)* addrspacecast (i8 addrspace(1)* bitcast ({ [3 x i8 addrspace(4)*] } addrspace(1)* @_ZTV1B to i8 addrspace(1)*) to i8 addrspace(4)*), i8 addrspace(2)* getelementptr inbounds ([7 x i8], [7 x i8] addrspace(2)* @.omp_offloading.entry_name.3, i32 0, i32 0), i64 24, i32 0, i32 0, i64 7 }, section "omp_offloading_entries"
@.omp_offloading.entry_name.4 = internal target_declare unnamed_addr addrspace(2) constant [13 x i8] c"_ZN1A4vfooEv\00"
@.omp_offloading.entry._ZN1A4vfooEv = weak target_declare local_unnamed_addr addrspace(1) constant %struct.__tgt_offload_entry.0 { i8 addrspace(4)* addrspacecast (i8* bitcast (void (%struct.A addrspace(4)*)* @_ZN1A4vfooEv to i8*) to i8 addrspace(4)*), i8 addrspace(2)* getelementptr inbounds ([13 x i8], [13 x i8] addrspace(2)* @.omp_offloading.entry_name.4, i32 0, i32 0), i64 0, i32 8, i32 0, i64 13 }, section "omp_offloading_entries"
@__kmp_alloc.total_num_allocs = internal addrspace(1) global i32 0, align 4
@__omp_spirv_global_data = addrspace(1) global { { %1, [149184 x i8] }, i32, i32 } { { %1, [149184 x i8] } { %1 zeroinitializer, [149184 x i8] undef }, i32 1, i32 0 }, align 8
@__omp_spirv_program_data = addrspace(1) global %struct.kmp_program_data { i32 0, i32 0, i32 -1, i32 0, i32 0, i64 0, i64 0, i32 0, i8 addrspace(1)* null, i32 0 }, align 8
@__omp_offloading_fptr_map_p = weak addrspace(1) global %struct.__omp_offloading_fptr_map_t addrspace(1)* null, align 8
@__omp_offloading_fptr_map_size = weak addrspace(1) global i64 0, align 8

; Function Attrs: mustprogress nounwind
define linkonce_odr hidden spir_func void @_ZN1A4vfooEv(%struct.A addrspace(4)* noundef align 8 dereferenceable_or_null(8) %this) unnamed_addr #0 comdat align 2 {
entry:
  %0 = load i32, i32 addrspace(1)* @count, align 4, !tbaa !10
  %inc = add nsw i32 %0, 1
  store i32 %inc, i32 addrspace(1)* @count, align 4, !tbaa !10
  ret void
}

; Function Attrs: convergent norecurse nounwind
declare spir_func i8 addrspace(4)* @__kmpc_target_translate_fptr(i64 noundef) #2

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #3

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #3

; Function Attrs: mustprogress noinline norecurse nounwind
define weak dso_local spir_kernel void @__omp_offloading_805_3c7604__Z4main_l24(%struct.B addrspace(1)* %b.ascast) local_unnamed_addr #4 !sycl_kernel_omit_args !16 {
newFuncRoot:
  %0 = tail call spir_func i64 @_Z12get_local_idj(i32 0) #6
  %1 = tail call spir_func i64 @_Z12get_local_idj(i32 1) #6
  %2 = or i64 %0, %1
  %3 = tail call spir_func i64 @_Z12get_local_idj(i32 2) #6
  %4 = or i64 %2, %3
  %is.master.thread = icmp eq i64 %4, 0
  tail call spir_func void @_Z22__spirv_ControlBarrieriii(i32 2, i32 2, i32 784) #6
  br i1 %is.master.thread, label %master.thread.code, label %master.thread.fallthru

master.thread.code:                               ; preds = %newFuncRoot
  %5 = getelementptr %struct.B, %struct.B addrspace(1)* %b.ascast, i64 0, i32 0
  %6 = addrspacecast %struct.A addrspace(1)* %5 to %struct.A addrspace(4)*
  %7 = bitcast %struct.B addrspace(1)* %b.ascast to void (%struct.A addrspace(4)*)* addrspace(4)* addrspace(1)*
  %vtable.i = load void (%struct.A addrspace(4)*)* addrspace(4)*, void (%struct.A addrspace(4)*)* addrspace(4)* addrspace(1)* %7, align 8, !tbaa !14, !alias.scope !17
  %8 = load void (%struct.A addrspace(4)*)*, void (%struct.A addrspace(4)*)* addrspace(4)* %vtable.i, align 8, !alias.scope !20
  %9 = ptrtoint void (%struct.A addrspace(4)*)* %8 to i64
  %10 = tail call i8 addrspace(4)* @__kmpc_target_translate_fptr(i64 %9) #6
  %11 = addrspacecast i8 addrspace(4)* %10 to void (%struct.A addrspace(4)*)*
  tail call void %11(%struct.A addrspace(4)* %6) #6
  br label %master.thread.fallthru

master.thread.fallthru:                           ; preds = %master.thread.code, %newFuncRoot
  tail call spir_func void @_Z22__spirv_ControlBarrieriii(i32 2, i32 2, i32 784) #6
  ret void
}

declare spir_func i64 @_Z12get_local_idj(i32) local_unnamed_addr

; Function Attrs: convergent
declare spir_func void @_Z22__spirv_ControlBarrieriii(i32, i32, i32) #5

attributes #0 = { mustprogress nounwind "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="all" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "openmp-target-declare"="true" "referenced-indirectly" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #1 = { mustprogress nounwind "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="all" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "openmp-target-declare"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #2 = { convergent norecurse nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" }
attributes #3 = { nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #4 = { mustprogress noinline norecurse nounwind "approx-func-fp-math"="true" "contains-openmp-target"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="all" "may-have-openmp-directive"="false" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "processed-by-vpo" "stack-protector-buffer-size"="8" "target.declare"="true" "unsafe-fp-math"="true" }
attributes #5 = { convergent }
attributes #6 = { nounwind }

!opencl.compiler.options = !{!0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0}
!llvm.ident = !{!1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1, !1}
!spirv.Source = !{!2, !3, !3, !3, !3, !3, !2, !3, !2, !2, !2, !2, !2, !2, !2, !2, !2, !2, !2, !2, !2, !2}
!spirv.MemoryModel = !{!4, !4, !4, !4, !4, !4}
!spirv.ExecutionMode = !{}
!llvm.module.flags = !{!5, !6, !7, !8, !9}

!0 = !{}
!1 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2024.0.0 (2024.x.0.YYYYMMDD)"}
!2 = !{i32 4, i32 200000}
!3 = !{i32 3, i32 200000}
!4 = !{i32 2, i32 2}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{i32 7, !"openmp", i32 51}
!7 = !{i32 7, !"openmp-device", i32 51}
!8 = !{i32 8, !"PIC Level", i32 2}
!9 = !{i32 7, !"frame-pointer", i32 2}
!10 = !{!11, !11, i64 0}
!11 = !{!"int", !12, i64 0}
!12 = !{!"omnipotent char", !13, i64 0}
!13 = !{!"Simple C++ TBAA"}
!14 = !{!15, !15, i64 0}
!15 = !{!"vtable pointer", !13, i64 0}
!16 = !{i1 false}
!17 = !{!18}
!18 = distinct !{!18, !19, !"OMPAliasScope"}
!19 = distinct !{!19, !"OMPDomain"}
!20 = !{!21}
!21 = distinct !{!21, !19, !"OMPAliasScope"}
