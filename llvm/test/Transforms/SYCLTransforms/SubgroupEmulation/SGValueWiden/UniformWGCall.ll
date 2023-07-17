; RUN: opt -passes=sycl-kernel-sg-emu-value-widen -S %s -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes=sycl-kernel-sg-emu-value-widen -S %s | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: convergent noinline norecurse nounwind
define void @basic(ptr addrspace(1) %scan_add, ptr addrspace(1) %wg_reduce_add) #0 !no_barrier_path !12 !kernel_has_sub_groups !13 !kernel_has_barrier !13 !sg_emu_size !15 {
; CHECK-LABEL: wg.loop.exclude:
; CHECK: %u.CallWGForItem = alloca i32, align 4
; CHECK-LABEL: sg.loop.exclude:
entry:
  call void @dummy_barrier.()
  %AllocaWGResult = alloca i32, align 4
  store i32 0, ptr %AllocaWGResult, align 4
  call void @dummy_barrier.()
  br label %sg.dummy.bb.

sg.dummy.bb.:                                     ; preds = %entry
  call void @dummy_sg_barrier()
  %scan_add.addr = alloca ptr addrspace(1), align 8
  %wg_reduce_add.addr = alloca ptr addrspace(1), align 8
  %lid = alloca i32, align 4
  store ptr addrspace(1) %scan_add, ptr %scan_add.addr, align 8
  store ptr addrspace(1) %wg_reduce_add, ptr %wg_reduce_add.addr, align 8
  %call = call i64 @_Z12get_local_idj(i32 0) #6
  %conv = trunc i64 %call to i32
  store i32 %conv, ptr %lid, align 4
  %0 = load i32, ptr %lid, align 4
  %CallWGForItem = call i32 @_Z21work_group_reduce_addiPi(i32 %0, ptr %AllocaWGResult) #7
; CHECK: %CallWGForItem = call i32 @_Z21work_group_reduce_addiPi
; CHECK-NEXT: store i32 %CallWGForItem, ptr %u.CallWGForItem
  br label %sg.barrier.bb.

sg.barrier.bb.:                                   ; preds = %sg.dummy.bb.
  call void @_Z17sub_group_barrierj(i32 1)
  call void @_Z7barrierj(i32 1)
  br label %sg.dummy.bb.4

sg.dummy.bb.4:                                    ; preds = %sg.barrier.bb.
  call void @dummy_sg_barrier()
  store i32 0, ptr %AllocaWGResult, align 4
  call void @dummy_barrier.()
  br label %sg.dummy.bb.3

sg.dummy.bb.3:                                    ; preds = %sg.dummy.bb.4
  call void @dummy_sg_barrier()
  %1 = load ptr addrspace(1), ptr %wg_reduce_add.addr, align 8
  %2 = load i32, ptr %lid, align 4
  %idxprom = sext i32 %2 to i64
  %ptridx = getelementptr inbounds i32, ptr addrspace(1) %1, i64 %idxprom
  store i32 %CallWGForItem, ptr addrspace(1) %ptridx, align 4
  %3 = load i32, ptr %lid, align 4
  br label %sg.barrier.bb.2

sg.barrier.bb.2:                                  ; preds = %sg.dummy.bb.3
  call void @_Z17sub_group_barrierj(i32 1)
  %call2 = call i32 @_Z28sub_group_scan_inclusive_addi(i32 %3) #8
  br label %sg.dummy.bb.6

sg.dummy.bb.6:                                    ; preds = %sg.barrier.bb.2
  call void @dummy_sg_barrier()
  %4 = load ptr addrspace(1), ptr %scan_add.addr, align 8
  %5 = load i32, ptr %lid, align 4
  %idxprom3 = sext i32 %5 to i64
  %ptridx4 = getelementptr inbounds i32, ptr addrspace(1) %4, i64 %idxprom3
  store i32 %call2, ptr addrspace(1) %ptridx4, align 4
  br label %sg.barrier.bb.1

sg.barrier.bb.1:                                  ; preds = %sg.dummy.bb.6
  call void @_Z17sub_group_barrierj(i32 1)
  call void @_Z7barrierj(i32 1)
  br label %sg.dummy.bb.5

sg.dummy.bb.5:                                    ; preds = %sg.barrier.bb.1
  call void @dummy_sg_barrier()
  ret void
}

; Function Attrs: convergent nounwind readnone
declare i64 @_Z12get_local_idj(i32) #1

; Function Attrs: convergent
declare i32 @_Z21work_group_reduce_addi(i32) #2

; Function Attrs: convergent
declare i32 @_Z28sub_group_scan_inclusive_addi(i32) #3

declare void @dummy_barrier.()

; Function Attrs: nofree norecurse nounwind
declare i32 @_Z21work_group_reduce_addiPi(i32, ptr nocapture) #4

; Function Attrs: convergent
declare void @_Z7barrierj(i32) #5

declare void @_Z17sub_group_barrierj(i32)

declare void @dummy_sg_barrier()

attributes #0 = { convergent noinline norecurse nounwind "disable-tail-calls"="false" "frame-pointer"="none" "kernel-call-once" "kernel-convergent-call" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { convergent nounwind readnone "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { convergent "disable-tail-calls"="false" "frame-pointer"="none" "kernel-call-once" "kernel-convergent-call" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { convergent "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" "vector-variants"="_ZGVbM4v__Z28sub_group_scan_inclusive_addi(_Z28sub_group_scan_inclusive_addDv4_iDv4_j)" }
attributes #4 = { nofree norecurse nounwind "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "target-cpu"="corei7" "target-features"="+cx16,+cx8,+fxsr,+mmx,+popcnt,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #5 = { convergent }
attributes #6 = { convergent nounwind readnone }
attributes #7 = { convergent "kernel-call-once" "kernel-convergent-call" }
attributes #8 = { convergent "has-vplan-mask" }

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
!2 = !{!"-cl-opt-disable", !"-cl-std=CL2.0"}
!3 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.2.0 (YYYY.x.0.MMDD)"}
!4 = !{ptr @basic}
!5 = !{i32 1, i32 1}
!6 = !{!"none", !"none"}
!7 = !{!"int*", !"int*"}
!8 = !{!"", !""}
!9 = !{!"scan_add", !"wg_reduce_add"}
!10 = !{i1 false, i1 false}
!11 = !{i32 0, i32 0}
!12 = !{i1 false}
!13 = !{i1 true}
!14 = !{i32 23}
!15 = !{i32 4}
!16 = !{!"int*", !"int*"}
!17 = !{ptr addrspace(1) null, ptr addrspace(1) null}

; DEBUGIFY-NOT: WARNING
; FIXME: SGValueWiden does not respect llvm.dbg.value and llvm.dbg.addr
; DEBUGIFY-COUNT-3: WARNING: Missing line
; DEBUGIFY-NOT: WARNING
