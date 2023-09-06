; This test checks the source correlation of the load and store instructions created by the Barrier pass. These instructions have a zero-source correlation because-
; 1) They are not directly related to user's code (artifact of the runtime chosen)
; 2) Assigning any source correlation to these instruction causes incorrect stepping behavior 
; RUN: opt -passes=sycl-kernel-barrier %s -S -enable-debugify  | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: convergent noinline norecurse nounwind
define dso_local void @test(ptr addrspace(1) %p) #0 !kernel_arg_addr_space !1 !kernel_arg_access_qual !2 !kernel_arg_type !3 !kernel_arg_base_type !4 !kernel_arg_type_qual !5 !kernel_arg_name !6 !kernel_arg_host_accessible !7 !kernel_arg_pipe_depth !8 !kernel_arg_pipe_io !5 !kernel_arg_buffer_location !5 !kernel_has_sub_groups !9 !kernel_execution_length !10 !no_barrier_path !9 !kernel_has_global_sync !9 {
entry:
; CHECK: %SBIndex = load i64, ptr %pCurrSBIndex, align 8, !dbg [[LOCATION_METADATA:![0-9]+]]
; CHECK: [[SB_LocalId_Offset4:%.*]] = add nuw i64 %SBIndex, 64, !dbg [[LOCATION_METADATA]]
; CHECK: [[GEP:%.*]] = getelementptr inbounds i8, ptr %pSB, i64 [[SB_LocalId_Offset4]], !dbg [[LOCATION_METADATA]]
; CHECK: store ptr [[GEP]], ptr %i.addr, align 8, !dbg [[LOCATION_METADATA]]
; CHECK: [[LOCATION_METADATA]] = !DILocation(line: 0, scope: [[SCOPE_METADATA:![0-9]+]])
; CHECK: [[SCOPE_METADATA]] = !DILexicalBlockFile(scope: !{{[0-9]+}}, file: [[FILE_METADATA:![0-9]+]], discriminator: 0)
; CHECK: [[FILE_METADATA]] = !DIFile(filename: "CPU_DEVICE_RT", directory: "/")

  call void @dummy_barrier.()
  %p.addr = alloca ptr addrspace(1), align 8
  %data = alloca <3 x double>, align 32
  %i = alloca i64, align 8
  store ptr addrspace(1) %p, ptr %p.addr, align 8
  %call = call i64 @_Z13get_global_idj(i32 0) #4
  store i64 %call, ptr %i, align 8
  %0 = load ptr addrspace(1), ptr %p.addr, align 8
  %1 = load i64, ptr %i, align 8
  %ptridx = getelementptr inbounds <3 x double>, ptr addrspace(1) %0, i64 %1
  %loadVec4 = load <4 x double>, ptr addrspace(1) %ptridx, align 32
  %extractVec = shufflevector <4 x double> %loadVec4, <4 x double> poison, <3 x i32> <i32 0, i32 1, i32 2>
  %extractVec1 = shufflevector <3 x double> %extractVec, <3 x double> poison, <4 x i32> <i32 0, i32 1, i32 2, i32 undef>
  store <4 x double> %extractVec1, ptr %data, align 32
  br label %"Barrier BB"

"Barrier BB":                                     ; preds = %entry
  call void @_Z18work_group_barrierj(i32 1)
  ret void
}

; Function Attrs: convergent nounwind readnone
declare i64 @_Z13get_global_idj(i32) #2

declare void @dummy_barrier.()

; Function Attrs: convergent
declare void @_Z18work_group_barrierj(i32) #3

attributes #0 = { convergent noinline norecurse nounwind "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="true" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nofree nosync nounwind readnone speculatable willreturn }
attributes #2 = { convergent nounwind readnone "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { convergent }
attributes #4 = { convergent nounwind readnone }

!sycl.kernels = !{!0}

!0 = !{ptr @test}
!1 = !{i32 1, i32 1}
!2 = !{!"none", !"none"}
!3 = !{!"double3*", !"ptr"}
!4 = !{!"double __attribute__((ext_vector_type(3)))*", !"ptr"}
!5 = !{!"", !""}
!6 = !{!"p", !"f"}
!7 = !{i1 false, i1 false}
!8 = !{i32 0, i32 0}
!9 = !{i1 false}
!10 = !{i32 22}
