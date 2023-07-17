; RUN: opt -passes=sycl-kernel-sg-emu-value-widen -S %s -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes=sycl-kernel-sg-emu-value-widen -S %s | FileCheck %s
; RUN: opt -passes=sycl-kernel-sg-emu-value-widen -S %s -disable-output -debug-only=sycl-kernel-sg-emu-value-widen 2>&1 | FileCheck %s -check-prefix=CHECK-DT

; Checks that DomTree is updated correctly after splitting the barrier-containing basic block.

; CHECK-DT: DomTree after splitting on sg barrier: 
; CHECK-DT:        [1] %sg.loop.exclude {{.*}} [0]
; CHECK-DT-NEXT:     [2] %entry {{.*}} [1]
; CHECK-DT-NEXT:       [3] %pre.sg.barrier.bb. {{.*}} [2]
; CHECK-DT-NEXT:         [4] %sg.barrier.bb. {{.*}} [3]
; CHECK-DT-NEXT: Roots: %sg.loop.exclude

define void @kernel() !kernel_has_sub_groups !1 !sg_emu_size !2 {
; CHECK-LABEL: @kernel(
; CHECK-NEXT:  sg.loop.exclude:
; CHECK:         br label %entry
; CHECK:       entry:
; CHECK:         br label %pre.sg.barrier.bb.
; CHECK:       pre.sg.barrier.bb.:
; CHECK:         br label %sg.barrier.bb.
; CHECK:       sg.barrier.bb.:
; CHECK:         ret void
;
entry:
  call void @dummy_sg_barrier()
  %call = call i32 @foo(i32 0)
  br label %sg.barrier.bb.

sg.barrier.bb.:                                   ; preds = %entry
  call void @_Z17sub_group_barrierj12memory_scope(i32 %call)
  ret void
}

declare void @_Z17sub_group_barrierj12memory_scope(i32)

declare i32 @foo(i32)

declare void @dummy_sg_barrier()

declare void @_Z17sub_group_barrierj(i32)

!sycl.kernels = !{!0}

!0 = !{ptr @kernel}
!1 = !{i1 true}
!2 = !{i32 16}

; DEBUGIFY-NOT: WARNING
