; RUN: opt -disable-output 2>&1 -passes='print<dpcpp-kernel-data-per-value-analysis>' -S < %s | FileCheck %s
; RUN: opt -analyze -enable-new-pm=0 -dpcpp-kernel-data-per-value-analysis -S < %s | FileCheck %s

;;*****************************************************************************
;; This test checks the DataPerValue pass
;; The case: Functions are not inlined. Function with cross-barrier return value
;;           should have offset in special buffer.
;;           The IR is dumped before BarrierPass from following source with
;;           build option "-cl-opt-disable":
;;             size_t calc() {
;;               size_t gid = get_global_id(0);
;;               barrier(CLK_LOCAL_MEM_FENCE);
;;               return gid;
;;             }
;;             size_t inclusive() {
;;               return calc();
;;             }
;;             kernel void test(global size_t *dst) {
;;               size_t gid = get_global_id(0);
;;               dst[gid] = inclusive();
;;             }
;; The expected result:
;;  Group-A Values analysis data collected is as follows
;;      * No analysis data was collected to this group
;;  Group-B.1 Values analysis data collected is as follows
;;      * Data was collected only for kernel "test"
;;      * Only values "%x" and "%y" was collected to this group
;;      * "%x" value has offset 0 in the special buffer
;;      * "%y" value has offset 8 in the special buffer
;;  Group-B.2 Values analysis data collected is as follows
;;      * No analysis data was collected to this group
;;  Buffer Total Size is 32
;;*****************************************************************************

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: convergent noinline norecurse nounwind
define internal fastcc i64 @calc() unnamed_addr #0 {
entry:
  call void @dummy_barrier.()
  %w = tail call i64 @_Z13get_global_idj(i32 0) #4
  br label %"Barrier BB"

"Barrier BB":                                     ; preds = %entry
  tail call void @_Z18work_group_barrierj(i32 1) #5
  ret i64 %w
}

; Function Attrs: convergent nounwind readnone
declare i64 @_Z13get_global_idj(i32) local_unnamed_addr #1

; Function Attrs: convergent
declare void @_Z18work_group_barrierj(i32) local_unnamed_addr #2

; Function Attrs: convergent noinline norecurse nounwind
define internal fastcc i64 @inclusive() unnamed_addr #0 {
entry:
  call void @dummy_barrier.()
  br label %"Barrier BB1"

"Barrier BB1":                                    ; preds = %entry
  call void @_Z18work_group_barrierj(i32 1)
  %z = tail call fastcc i64 @calc() #6
  br label %"Barrier BB2"

"Barrier BB2":                                    ; preds = %"Barrier BB1"
  call void @dummy_barrier.()
  br label %"Barrier BB"

"Barrier BB":                                     ; preds = %"Barrier BB2"
  call void @_Z18work_group_barrierj(i32 1)
  ret i64 %z
}

; Function Attrs: convergent noinline norecurse nounwind
define void @test(i64 addrspace(1)* noalias %dst) local_unnamed_addr #3 !kernel_arg_addr_space !5 !kernel_arg_access_qual !6 !kernel_arg_type !7 !kernel_arg_base_type !8 !kernel_arg_type_qual !9 !kernel_arg_host_accessible !10 !kernel_arg_pipe_depth !11 !kernel_arg_pipe_io !9 !kernel_arg_buffer_location !9 !kernel_arg_name !12 !kernel_has_sub_groups !10 !kernel_execution_length !13 !kernel_has_barrier !10 !kernel_has_global_sync !10 {
entry:
  call void @dummy_barrier.()
  %x = tail call i64 @_Z13get_global_idj(i32 0) #4
  br label %"Barrier BB1"

"Barrier BB1":                                    ; preds = %entry
  call void @_Z18work_group_barrierj(i32 1)
  %y = tail call fastcc i64 @inclusive() #6
  br label %"Barrier BB2"

"Barrier BB2":                                    ; preds = %"Barrier BB1"
  call void @dummy_barrier.()
  %ptridx = getelementptr inbounds i64, i64 addrspace(1)* %dst, i64 %x
  store i64 %y, i64 addrspace(1)* %ptridx, align 8
  br label %"Barrier BB"

"Barrier BB":                                     ; preds = %"Barrier BB2"
  call void @_Z18work_group_barrierj(i32 1)
  ret void
}

; CHECK: Group-A Values
; CHECK-NOT: +
; CHECK-NOT: -
; CHECK-NOT: *

; CHECK: Group-B.1 Values
; CHECK-NOT: +
; CHECK: +test
; CHECK-NOT: +
; CHECK-NOT: -
; CHECK: -x (0)
; CHECK: -y (8)
; CHECK-NOT: -
; CHECK: *
; CHECK-NOT: +

; CHECK: Group-B.2 Values
; CHECK-NOT: +
; CHECK-NOT: -
; CHECK-NOT: *

; CHECK: Function Equivalence Classes:
; CHECK-NEXT: [test]: test inclusive calc

; CHECK-NEXT: Buffer Total Size:
; CHECK-NEXT: leader(test) : (32)
; CHECK-NEXT: DONE

declare void @dummy_barrier.()

attributes #0 = { convergent noinline norecurse nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "kernel-call-once" "kernel-convergent-call" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { convergent nounwind readnone "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { convergent "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "kernel-call-once" "kernel-convergent-call" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { convergent noinline norecurse nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "kernel-call-once" "kernel-convergent-call" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { convergent nounwind readnone }
attributes #5 = { convergent nounwind "kernel-call-once" "kernel-convergent-call" }
attributes #6 = { convergent }

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
!4 = !{void (i64 addrspace(1)*)* @test}
!5 = !{i32 1}
!6 = !{!"none"}
!7 = !{!"size_t*"}
!8 = !{!"ulong*"}
!9 = !{!""}
!10 = !{i1 false}
!11 = !{i32 0}
!12 = !{!"dst"}
!13 = !{i32 5}
