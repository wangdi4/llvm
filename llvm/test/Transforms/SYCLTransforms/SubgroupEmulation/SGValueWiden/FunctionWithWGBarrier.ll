; RUN: opt -passes=sycl-kernel-sg-emu-value-widen -S %s -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes=sycl-kernel-sg-emu-value-widen -S %s | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: convergent noinline norecurse nounwind
define i32 @foo(i32 %lid) #0 {
entry:
  call void @dummy_barrier.()
  br label %sg.dummy.bb.

sg.dummy.bb.:                                     ; preds = %entry
  call void @dummy_sg_barrier()
  %lid.addr = alloca i32, align 4
  store i32 %lid, ptr %lid.addr, align 4
  br label %sg.barrier.bb.

sg.barrier.bb.:                                   ; preds = %sg.dummy.bb.
  call void @_Z17sub_group_barrierj(i32 1)
  call void @_Z7barrierj(i32 1) #5
  br label %sg.dummy.bb.2

sg.dummy.bb.2:                                    ; preds = %sg.barrier.bb.
  call void @dummy_sg_barrier()
  %0 = load i32, ptr %lid.addr, align 4
  br label %sg.barrier.bb.1

sg.barrier.bb.1:                                  ; preds = %sg.dummy.bb.2
  call void @_Z17sub_group_barrierj(i32 1)
  %call = call i32 @_Z28sub_group_scan_inclusive_addi(i32 %0) #6
  call void @_Z7barrierj(i32 1)
  br label %sg.dummy.bb.3

sg.dummy.bb.3:                                    ; preds = %sg.barrier.bb.1
  call void @dummy_sg_barrier()
  ret i32 %call
}

;CHECK-LABEL: define <4 x i32> @_ZGVbN4v_foo
;CHECK-LABEL: sg.dummy.bb.3:
;CHECK: call void @_Z17sub_group_barrierj(i32 1)
;CHECK-NEXT: %[[#RET:]] = load <4 x i32>, ptr %w.{{.*}}, align 16
;CHECK-NEXT: call void @_Z18work_group_barrierj(i32 1)
;CHECK-NEXT: ret <4 x i32> %[[#RET]]

; Function Attrs: convergent
declare void @_Z7barrierj(i32) #1

; Function Attrs: convergent
declare i32 @_Z28sub_group_scan_inclusive_addi(i32) #2

; Function Attrs: convergent noinline norecurse nounwind
define void @basic(ptr addrspace(1) noalias %scan_add) #3 !no_barrier_path !10 !kernel_has_sub_groups !12 !kernel_has_barrier !10 !sg_emu_size !14 !kernel_arg_base_type !15 !arg_type_null_val !16 {
entry:
  call void @dummy_barrier.()
  br label %sg.dummy.bb.2

sg.dummy.bb.2:                                    ; preds = %entry
  call void @dummy_sg_barrier()
  %scan_add.addr = alloca ptr addrspace(1), align 8
  %lid = alloca i32, align 4
  store ptr addrspace(1) %scan_add, ptr %scan_add.addr, align 8
  %call = call i64 @_Z12get_local_idj(i32 0) #7
  %conv = trunc i64 %call to i32
  store i32 %conv, ptr %lid, align 4
  %0 = load i32, ptr %lid, align 4
  br label %sg.barrier.bb.

sg.barrier.bb.:                                   ; preds = %sg.dummy.bb.2
  call void @_Z17sub_group_barrierj(i32 1)
  call void @_Z7barrierj(i32 1)

; CHECK-LABEL: define void @basic
; CHECK: %[[#WIDE_ARG:]] = load <4 x i32>, ptr %{{w.*}}, align 16
; CHECK-NEXT: call void @_Z7barrierj(i32 1)
; CHECK-NEXT: %[[#WIDE_RET:]] = call <4 x i32> @_ZGVbN4v_foo(<4 x i32> %[[#WIDE_ARG]])
; CHECK-NEXT: call void @dummy_barrier.()
; CHECK-NEXT: store <4 x i32> %[[#WIDE_RET]], ptr %{{w.*}}, align 16

  %call1 = call i32 @foo(i32 %0) #8
  call void @dummy_barrier.()
  br label %sg.dummy.bb.

sg.dummy.bb.:                                     ; preds = %sg.barrier.bb.
  call void @dummy_sg_barrier()
  %1 = load ptr addrspace(1), ptr %scan_add.addr, align 8
  %2 = load i32, ptr %lid, align 4
  %idxprom = sext i32 %2 to i64
  %ptridx = getelementptr inbounds i32, ptr addrspace(1) %1, i64 %idxprom
  store i32 %call1, ptr addrspace(1) %ptridx, align 4
  br label %sg.barrier.bb.1

sg.barrier.bb.1:                                  ; preds = %sg.dummy.bb.
  call void @_Z17sub_group_barrierj(i32 1)
  call void @_Z7barrierj(i32 1)
  br label %sg.dummy.bb.3

sg.dummy.bb.3:                                    ; preds = %sg.barrier.bb.1
  call void @dummy_sg_barrier()
  ret void
}

; Function Attrs: convergent nounwind readnone
declare i64 @_Z12get_local_idj(i32) #4

declare void @dummy_barrier.()

declare void @_Z17sub_group_barrierj(i32)

declare void @dummy_sg_barrier()

attributes #0 = { convergent noinline norecurse nounwind "disable-tail-calls"="false" "frame-pointer"="none" "kernel-call-once" "kernel-convergent-call" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { convergent "disable-tail-calls"="false" "frame-pointer"="none" "kernel-call-once" "kernel-convergent-call" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { convergent "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" "vector-variants"="_ZGVbM4v__Z28sub_group_scan_inclusive_addi(_Z28sub_group_scan_inclusive_addDv4_iDv4_j)" }
attributes #3 = { convergent noinline norecurse nounwind "disable-tail-calls"="false" "frame-pointer"="none" "kernel-call-once" "kernel-convergent-call" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="true" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { convergent nounwind readnone "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #5 = { convergent "kernel-call-once" "kernel-convergent-call" }
attributes #6 = { convergent "has-vplan-mask" }
attributes #7 = { convergent nounwind readnone }
attributes #8 = { convergent }

!llvm.linker.options = !{}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!0}
!opencl.spir.version = !{!0}
!opencl.used.extensions = !{!1}
!opencl.used.optional.core.features = !{!1}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!3}
!sycl.kernels = !{!4}

!0 = !{i32 1, i32 2}
!1 = !{}
!2 = !{!"-cl-opt-disable"}
!3 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.2.0 (YYYY.x.0.MMDD)"}
!4 = !{ptr @basic}
!5 = !{i32 1}
!6 = !{!"none"}
!7 = !{!"int*"}
!8 = !{!""}
!9 = !{!"scan_add"}
!10 = !{i1 false}
!11 = !{i32 0}
!12 = !{i1 true}
!13 = !{i32 14}
!14 = !{i32 4}
!15 = !{!"int*"}
!16 = !{ptr addrspace(1) null}

; DEBUGIFY-NOT: WARNING
; FIXME: SGValueWiden does not respect llvm.dbg.value and llvm.dbg.addr
; DEBUGIFY-COUNT-2: WARNING: Missing line
; DEBUGIFY-NOT: WARNING
