; RUN: opt -passes=sycl-kernel-barrier -S < %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-barrier -S < %s | FileCheck %s

;;*****************************************************************************
;; This test checks the Barrier pass
;; The case:
;;     Functions are not inlined. Function "foo" call "work_group_broadcast",
;;     so its return value will be saved to special buffer and caller will load
;;     from special buffer. IR is dumped before BarrierPass from following source
;;     with build option "-cl-opt-disable":
;;         int foo(int a, size_t b){
;;           return work_group_broadcast(a, b);
;;         }
;;         __kernel void addVectors( __global int *src, __global int *dst) {
;;           size_t gid = get_global_id(0);
;;           int a = foo( src[gid], 3 );
;;           dst[gid] = a;
;;         }
;;*****************************************************************************

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: convergent noinline norecurse nounwind
define i32 @foo(i32 %a, i64 %b) #0 !use_fpga_pipes !5 {
entry:
  call void @dummy_barrier.()
  %AllocaWGResult = alloca i32, align 4
  store i32 0, ptr %AllocaWGResult, align 4
  br label %"Barrier BB3"

"Barrier BB3":                                    ; preds = %entry
  call void @dummy_barrier.()
  %a.addr = alloca i32, align 4
  %b.addr = alloca i64, align 8
  store i32 %a, ptr %a.addr, align 4
  store i64 %b, ptr %b.addr, align 8
  %0 = load i32, ptr %a.addr, align 4
  %1 = load i64, ptr %b.addr, align 8
  %WIcall = call i64 @_Z12get_local_idj(i32 0)
  %CallWGForItem = call i32 @_Z20work_group_broadcastimmPi(i32 %0, i64 %1, i64 %WIcall, ptr %AllocaWGResult) #6
; CHECK-LABEL: define i32 @foo
; CHECK: store i32 %CallWGForItem, ptr [[CallWGForItem:%CallWGForItem[0-9]+]], align 4
; CHECK-NEXT: br label %"Barrier BB1"
  br label %"Barrier BB1"

"Barrier BB1":                                    ; preds = %"Barrier BB3"
  call void @_Z18work_group_barrierj(i32 1)
  store i32 0, ptr %AllocaWGResult, align 4
  br label %"Barrier BB2"

"Barrier BB2":                                    ; preds = %"Barrier BB1"
  call void @dummy_barrier.()
  br label %"Barrier BB"

"Barrier BB":                                     ; preds = %"Barrier BB2"
  call void @_Z18work_group_barrierj(i32 1)
; CHECK: LoopBB:
; CHECK: [[SBIndex:%SBIndex[0-9]+]] = load i64, ptr %pCurrSBIndex, align 8
; CHECK-NEXT:  [[SBLocalIdOffset:%SB_LocalId_Offset[0-9]+]] = add nuw i64 [[SBIndex]], 56
; CHECK-NEXT:  [[GEP0:%pSB_LocalId[0-9]*]] = getelementptr inbounds i8, ptr %pSB, i64 [[SBLocalIdOffset]]
; CHECK-NEXT:  [[loadedValue:%loadedValue[0-9]+]] = load i32, ptr [[CallWGForItem]], align 4
; CHECK-NEXT:  store i32 [[loadedValue]], ptr [[GEP0]], align 4
  ret i32 %CallWGForItem
}

; Function Attrs: convergent
declare i32 @_Z20work_group_broadcastim(i32, i64) #1

; Function Attrs: convergent noinline norecurse nounwind
define void @addVectors(ptr addrspace(1) %src, ptr addrspace(1) %dst) #2 !kernel_arg_addr_space !6 !kernel_arg_access_qual !7 !kernel_arg_type !8 !kernel_arg_base_type !8 !kernel_arg_type_qual !9 !kernel_arg_name !10 !kernel_arg_host_accessible !11 !kernel_arg_pipe_depth !12 !kernel_arg_pipe_io !9 !kernel_arg_buffer_location !9 !use_fpga_pipes !5 !kernel_has_sub_groups !5 !kernel_execution_length !13 !no_barrier_path !5 !kernel_has_global_sync !5 {
entry:
  call void @dummy_barrier.()
  %src.addr = alloca ptr addrspace(1), align 8
  %dst.addr = alloca ptr addrspace(1), align 8
  %gid = alloca i64, align 8
  %a = alloca i32, align 4
  store ptr addrspace(1) %src, ptr %src.addr, align 8
  store ptr addrspace(1) %dst, ptr %dst.addr, align 8
  %call = call i64 @_Z13get_global_idj(i32 0) #7
  store i64 %call, ptr %gid, align 8
  %0 = load ptr addrspace(1), ptr %src.addr, align 8
  %1 = load i64, ptr %gid, align 8
  %ptridx = getelementptr inbounds i32, ptr addrspace(1) %0, i64 %1
  %2 = load i32, ptr addrspace(1) %ptridx, align 4
  br label %"Barrier BB1"

"Barrier BB1":                                    ; preds = %entry
  call void @_Z18work_group_barrierj(i32 1)
  %call1 = call i32 @foo(i32 %2, i64 3) #5
  br label %"Barrier BB2"

"Barrier BB2":                                    ; preds = %"Barrier BB1"
  call void @dummy_barrier.()
  store i32 %call1, ptr %a, align 4
  %3 = load i32, ptr %a, align 4
  %4 = load ptr addrspace(1), ptr %dst.addr, align 8
  %5 = load i64, ptr %gid, align 8
  %ptridx2 = getelementptr inbounds i32, ptr addrspace(1) %4, i64 %5
  store i32 %3, ptr addrspace(1) %ptridx2, align 4
  br label %"Barrier BB"

"Barrier BB":                                     ; preds = %"Barrier BB2"
  call void @_Z18work_group_barrierj(i32 1)
  ret void
}

; Function Attrs: convergent nounwind readnone
declare i64 @_Z13get_global_idj(i32) #3

declare void @dummy_barrier.()

declare i64 @_Z12get_local_idj(i32)

; Function Attrs: nofree norecurse nounwind
declare i32 @_Z20work_group_broadcastimmPi(i32, i64, i64, ptr nocapture) #4

; Function Attrs: convergent
declare void @_Z18work_group_barrierj(i32) #5

attributes #0 = { convergent noinline norecurse nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "kernel-call-once" "kernel-convergent-call" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { convergent "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "kernel-call-once" "kernel-convergent-call" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { convergent noinline norecurse nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "kernel-call-once" "kernel-convergent-call" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { convergent nounwind readnone "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { nofree norecurse nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "target-cpu"="skx" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #5 = { convergent }
attributes #6 = { convergent "kernel-call-once" "kernel-convergent-call" }
attributes #7 = { convergent nounwind readnone }

!llvm.linker.options = !{}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!0}
!opencl.spir.version = !{!0}
!opencl.used.extensions = !{!1}
!opencl.used.optional.core.features = !{!1}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!3}
!sycl.kernels = !{!4}

!0 = !{i32 2, i32 0}
!1 = !{}
!2 = !{!"-cl-std=CL2.0", !"-cl-opt-disable"}
!3 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!4 = !{ptr @addVectors}
!5 = !{i1 false}
!6 = !{i32 1, i32 1}
!7 = !{!"none", !"none"}
!8 = !{!"int*", !"int*"}
!9 = !{!"", !""}
!10 = !{!"src", !"dst"}
!11 = !{i1 false, i1 false}
!12 = !{i32 0, i32 0}
!13 = !{i32 20}

;; barrier key values
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function foo -- %pCurrBarrier = alloca i32, align 4
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function foo -- %pCurrSBIndex = alloca i64, align 8
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function foo -- %pLocalIds = alloca [3 x i64], align 8
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function foo -- %pSB = call ptr @get_special_buffer.()
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function foo -- %LocalSize_0 = call i64 @_Z14get_local_sizej(i32 0)
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function foo -- %LocalSize_1 = call i64 @_Z14get_local_sizej(i32 1)
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function foo -- %LocalSize_2 = call i64 @_Z14get_local_sizej(i32 2)
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function addVectors -- %pCurrBarrier = alloca i32, align 4
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function addVectors -- %pCurrSBIndex = alloca i64, align 8
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function addVectors -- %pLocalIds = alloca [3 x i64], align 8
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function addVectors -- %pSB = call ptr @get_special_buffer.()
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function addVectors -- %LocalSize_0 = call i64 @_Z14get_local_sizej(i32 0)
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function addVectors -- %LocalSize_1 = call i64 @_Z14get_local_sizej(i32 1)
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function addVectors -- %LocalSize_2 = call i64 @_Z14get_local_sizej(i32 2)

; DEBUGIFY-NOT: WARNING
