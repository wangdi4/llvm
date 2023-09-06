; This test checks whether the special buffer size/offset for double3
; is calculated correctly.
;
; The IR is dumped before BarrierPass from source:
; __kernel void test( __global double3 *p )
; {
;    __private double3 data;
;    size_t i = get_global_id(0);
;    data = p[i];
; }
; RUN: opt -passes=sycl-kernel-barrier %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-barrier %s -S | FileCheck %s
;

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: convergent noinline norecurse nounwind
define dso_local void @test(ptr addrspace(1) %p) #0 !kernel_arg_addr_space !1 !kernel_arg_access_qual !2 !kernel_arg_type !3 !kernel_arg_base_type !4 !kernel_arg_type_qual !5 !kernel_arg_name !6 !kernel_arg_host_accessible !7 !kernel_arg_pipe_depth !8 !kernel_arg_pipe_io !5 !kernel_arg_buffer_location !5 !kernel_has_sub_groups !9 !kernel_execution_length !10 !no_barrier_path !9 !kernel_has_global_sync !9 {
entry:
; CHECK-LABEL: entry:
; CHECK: %data.addr = alloca ptr

; CHECK-LABEL: SyncBB1:
; CHECK: [[Offset_data:%SB_LocalId_Offset[0-9]+]] = add nuw i64 %SBIndex, 32
; CHECK-NEXT: [[Ptr_data:%pSB_LocalId[0-9]*]] = getelementptr inbounds i8, ptr %pSB, i64 [[Offset_data]]
; CHECK-NEXT: store ptr [[Ptr_data]], ptr %data.addr
; CHECK: [[Offset_i:%SB_LocalId_Offset[0-9]+]] = add nuw i64 %SBIndex, 64
; CHECK-NEXT: [[Ptr_i:%pSB_LocalId[0-9]*]] = getelementptr inbounds i8, ptr %pSB, i64 [[Offset_i]]
; CHECK-NEXT: store ptr [[Ptr_i]], ptr %i.addr

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
;; get_global_id resolve
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test -- %pCurrSBIndex = alloca i64, align 8
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test -- %pLocalIds = alloca [3 x i64], align 8
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test -- %pSB = call ptr @get_special_buffer.()
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test -- %LocalSize_0 = call i64 @_Z14get_local_sizej(i32 0)
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test -- %LocalSize_1 = call i64 @_Z14get_local_sizej(i32 1)
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test -- %LocalSize_2 = call i64 @_Z14get_local_sizej(i32 2)

;DEBUGIFY-NOT: WARNING
