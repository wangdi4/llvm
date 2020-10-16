; RUN: %oclopt -externalize-global-variables -S %s -o %t.ll
; RUN: FileCheck %s --input-file=%t.ll

; CHECK: @x1 = dso_local addrspace(1) global i32 0, align 4
; CHECK: @x2 = dso_local addrspace(1) global i32 0, align 4
; CHECK: @x1.3 = dso_local addrspace(1) global i32 1, align 4
; CHECK: @x2.2 = dso_local addrspace(1) global i32 2, align 4
; CHECK: @.omp_offloading.entry_name = dso_local unnamed_addr addrspace(2) constant [40 x i8] c"_ZL2x1_8bda62a21df3dfe585271c7a6a1d580d\00"
; CHECK: @.omp_offloading.entry_name.4 = dso_local unnamed_addr addrspace(2) constant [40 x i8] c"_ZL2x2_8bda62a21df3dfe585271c7a6a1d580d\00"
; CHECK: @.omp_offloading.entry_name.5 = dso_local unnamed_addr addrspace(2) constant [41 x i8] c"__omp_offloading_811_6e40112__Z4main_l19\00"
; CHECK: @.str.as2 = dso_local unnamed_addr addrspace(2) constant [12 x i8] c"foo: %d %d\0A\00"
; CHECK: @.str.2.as2 = dso_local unnamed_addr addrspace(2) constant [13 x i8] c"main: %d %d\0A\00"
; CHECK: @.omp_offloading.entry_name.6 = dso_local unnamed_addr addrspace(2) constant [40 x i8] c"_ZL2x1_002cf9b7666e54b7b348ad7bc7e73c7c\00"
; CHECK: @.omp_offloading.entry_name.1 = dso_local unnamed_addr addrspace(2) constant [40 x i8] c"_ZL2x2_002cf9b7666e54b7b348ad7bc7e73c7c\00"
; CHECK: @.str.as2.9 = dso_local unnamed_addr addrspace(2) constant [12 x i8] c"bar: %d %d\0A\00"

%struct.__tgt_offload_entry = type { i8 addrspace(4)*, i8 addrspace(2)*, i64, i32, i32, i64 }

@x1 = internal addrspace(1) global i32 0, align 4
@x2 = internal addrspace(1) global i32 0, align 4
@x1.3 = internal addrspace(1) global i32 1, align 4
@x2.2 = internal addrspace(1) global i32 2, align 4
@.omp_offloading.entry_name = internal unnamed_addr addrspace(2) constant [40 x i8] c"_ZL2x1_8bda62a21df3dfe585271c7a6a1d580d\00"
@.omp_offloading.entry_name.4 = internal unnamed_addr addrspace(2) constant [40 x i8] c"_ZL2x2_8bda62a21df3dfe585271c7a6a1d580d\00"
@.omp_offloading.entry_name.5 = internal unnamed_addr addrspace(2) constant [41 x i8] c"__omp_offloading_811_6e40112__Z4main_l19\00"
@.str.as2 = internal unnamed_addr addrspace(2) constant [12 x i8] c"foo: %d %d\0A\00"
@.str.2.as2 = internal unnamed_addr addrspace(2) constant [13 x i8] c"main: %d %d\0A\00"
@.omp_offloading.entry_name.6 = internal unnamed_addr addrspace(2) constant [40 x i8] c"_ZL2x1_002cf9b7666e54b7b348ad7bc7e73c7c\00"
@.omp_offloading.entry_name.1 = internal unnamed_addr addrspace(2) constant [40 x i8] c"_ZL2x2_002cf9b7666e54b7b348ad7bc7e73c7c\00"
@.str.as2.9 = internal unnamed_addr addrspace(2) constant [12 x i8] c"bar: %d %d\0A\00"
@__omp_offloading_entries_table = addrspace(2) constant [5 x %struct.__tgt_offload_entry] [%struct.__tgt_offload_entry { i8 addrspace(4)* addrspacecast (i8 addrspace(1)* bitcast (i32 addrspace(1)* @x1.3 to i8 addrspace(1)*) to i8 addrspace(4)*), i8 addrspace(2)* getelementptr inbounds ([40 x i8], [40 x i8] addrspace(2)* @.omp_offloading.entry_name.6, i32 0, i32 0), i64 4, i32 0, i32 0, i64 40 }, %struct.__tgt_offload_entry { i8 addrspace(4)* addrspacecast (i8 addrspace(1)* bitcast (i32 addrspace(1)* @x1 to i8 addrspace(1)*) to i8 addrspace(4)*), i8 addrspace(2)* getelementptr inbounds ([40 x i8], [40 x i8] addrspace(2)* @.omp_offloading.entry_name, i32 0, i32 0), i64 4, i32 0, i32 0, i64 40 }, %struct.__tgt_offload_entry { i8 addrspace(4)* addrspacecast (i8 addrspace(1)* bitcast (i32 addrspace(1)* @x2.2 to i8 addrspace(1)*) to i8 addrspace(4)*), i8 addrspace(2)* getelementptr inbounds ([40 x i8], [40 x i8] addrspace(2)* @.omp_offloading.entry_name.1, i32 0, i32 0), i64 4, i32 0, i32 0, i64 40 }, %struct.__tgt_offload_entry { i8 addrspace(4)* addrspacecast (i8 addrspace(1)* bitcast (i32 addrspace(1)* @x2 to i8 addrspace(1)*) to i8 addrspace(4)*), i8 addrspace(2)* getelementptr inbounds ([40 x i8], [40 x i8] addrspace(2)* @.omp_offloading.entry_name.4, i32 0, i32 0), i64 4, i32 0, i32 0, i64 40 }, %struct.__tgt_offload_entry { i8 addrspace(4)* null, i8 addrspace(2)* getelementptr inbounds ([41 x i8], [41 x i8] addrspace(2)* @.omp_offloading.entry_name.5, i32 0, i32 0), i64 0, i32 0, i32 0, i64 41 }]
@__omp_offloading_entries_table_size = addrspace(2) constant i64 200

; Function Attrs: nounwind
declare spir_func i64 @_Z12get_local_idj(i32) #0

; Function Attrs: nounwind
declare spir_func void @_Z18work_group_barrierj(i32) #0

; Function Attrs: nounwind
define spir_func void @foo() #0 {
entry:
  %0 = addrspacecast i32 addrspace(1)* @x1 to i32 addrspace(4)*
  %1 = load i32, i32 addrspace(4)* %0, align 4
  %2 = addrspacecast i32 addrspace(1)* @x2 to i32 addrspace(4)*
  %3 = load i32, i32 addrspace(4)* %2, align 4
  %4 = getelementptr inbounds [12 x i8], [12 x i8] addrspace(2)* @.str.as2, i64 0, i64 0
  %oclPrint = call spir_func i32 (i8 addrspace(2)*, ...) @printf(i8 addrspace(2)* %4, i32 %1, i32 %3) #0
  ret void
}

; Function Attrs: nounwind
declare spir_func i32 @printf(i8 addrspace(2)*, ...) #0

; Function Attrs: nounwind
define spir_kernel void @__omp_offloading_811_6e40112__Z4main_l19(i32 addrspace(1)* %x1, i32 addrspace(1)* %x2) #0 !kernel_arg_addr_space !11 !kernel_arg_access_qual !12 !kernel_arg_type !13 !kernel_arg_type_qual !14 !kernel_arg_base_type !13 {
newFuncRoot:
  %0 = getelementptr inbounds [13 x i8], [13 x i8] addrspace(2)* @.str.2.as2, i64 0, i64 0
  br label %DIR.OMP.TARGET.59.split

DIR.OMP.END.TARGET.8.exitStub:                    ; preds = %DIR.OMP.END.TARGET.7
  ret void

DIR.OMP.TARGET.59.split:                          ; preds = %newFuncRoot
  %1 = call spir_func i64 @_Z12get_local_idj(i32 0) #0
  %2 = icmp eq i64 %1, 0
  %3 = call spir_func i64 @_Z12get_local_idj(i32 1) #0
  %4 = icmp eq i64 %3, 0
  %5 = and i1 %2, %4
  %6 = call spir_func i64 @_Z12get_local_idj(i32 2) #0
  %7 = icmp eq i64 %6, 0
  %is.master.thread = and i1 %5, %7
  br label %DIR.OMP.TARGET.6

DIR.OMP.TARGET.6:                                 ; preds = %DIR.OMP.TARGET.59.split
  br label %DIR.OMP.TARGET.5

DIR.OMP.TARGET.5:                                 ; preds = %DIR.OMP.TARGET.6
  call spir_func void @_Z18work_group_barrierj(i32 3) #0
  br i1 %is.master.thread, label %master.thread.code, label %master.thread.fallthru

master.thread.code:                               ; preds = %DIR.OMP.TARGET.5
  call spir_func void @foo() #0
  br label %master.thread.fallthru

master.thread.fallthru:                           ; preds = %master.thread.code, %DIR.OMP.TARGET.5
  call spir_func void @_Z18work_group_barrierj(i32 3) #0
  %8 = load i32, i32 addrspace(1)* %x1, align 4
  %9 = load i32, i32 addrspace(1)* %x2, align 4
  call spir_func void @_Z18work_group_barrierj(i32 3) #0
  br i1 %is.master.thread, label %master.thread.code1, label %master.thread.fallthru2

master.thread.code1:                              ; preds = %master.thread.fallthru
  %oclPrint10 = call spir_func i32 (i8 addrspace(2)*, ...) @printf(i8 addrspace(2)* %0, i32 %8, i32 %9) #0
  call spir_func void @bar() #0
  br label %master.thread.fallthru2

master.thread.fallthru2:                          ; preds = %master.thread.code1, %master.thread.fallthru
  call spir_func void @_Z18work_group_barrierj(i32 3) #0
  br label %DIR.OMP.END.TARGET.6.split

DIR.OMP.END.TARGET.6.split:                       ; preds = %master.thread.fallthru2
  br label %DIR.OMP.END.TARGET.7

DIR.OMP.END.TARGET.7:                             ; preds = %DIR.OMP.END.TARGET.6.split
  br label %DIR.OMP.END.TARGET.8.exitStub
}

; Function Attrs: nounwind
define spir_func void @bar() #0 {
entry:
  %0 = addrspacecast i32 addrspace(1)* @x1.3 to i32 addrspace(4)*
  %1 = load i32, i32 addrspace(4)* %0, align 4
  %2 = getelementptr inbounds [12 x i8], [12 x i8] addrspace(2)* @.str.as2.9, i64 0, i64 0
  %oclPrint = call spir_func i32 (i8 addrspace(2)*, ...) @printf(i8 addrspace(2)* %2, i32 %1, i32 2) #0
  ret void
}

attributes #0 = { nounwind }

!spirv.MemoryModel = !{!0}
!spirv.Source = !{!1}
!opencl.spir.version = !{!2}
!opencl.ocl.version = !{!2}
!opencl.used.extensions = !{!3}
!opencl.used.optional.core.features = !{!4}
!spirv.Generator = !{!5}
!opencl.stat.type = !{!6}
!opencl.stat.exec_time = !{!7}
!opencl.stat.run_time_version = !{!8}
!opencl.stat.workload_name = !{!9}
!opencl.stat.module_name = !{!10}

!0 = !{i32 2, i32 2}
!1 = !{i32 4, i32 200000}
!2 = !{i32 2, i32 0}
!3 = !{!"cl_khr_int64_extended_atomics", !"cl_khr_subgroups"}
!4 = !{!"cl_doubles"}
!5 = !{i16 6, i16 14}
!6 = !{!""}
!7 = !{!"2020-10-11 15:46:14"}
!8 = !{!"2020.11.10.0"}
!9 = !{!"omp_declare_target.exe"}
!10 = !{!"omp_declare_target.exe"}
!11 = !{i32 1, i32 1}
!12 = !{!"none", !"none"}
!13 = !{!"int*", !"int*"}
!14 = !{!"", !""}
