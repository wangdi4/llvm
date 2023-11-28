; RUN: opt -passes=sycl-kernel-sg-emu-value-widen -S %s | FileCheck %s
; RUN: opt -passes=sycl-kernel-sg-emu-value-widen -S %s -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY

%"class.sycl::_V1::detail::half_impl::half" = type { half }

define void @test_kernel() !kernel_has_sub_groups !1 !sg_emu_size !2 {
entry:
  call void @dummy_barrier.()
  br label %sg.dummy.bb.1

sg.dummy.bb.1:                                    ; preds = %entry
  call void @dummy_sg_barrier()
  %ret.addr = alloca %"class.sycl::_V1::detail::half_impl::half", align 2
  br label %sg.barrier.bb.

sg.barrier.bb.:                                   ; preds = %sg.dummy.bb.1
  call void @_Z17sub_group_barrierj(i32 1)
  call void @_Z18work_group_barrierj(i32 1)
  call void @foo(ptr %ret.addr)
  call void @dummy_barrier.()
  br label %sg.dummy.bb.

sg.dummy.bb.:                                     ; preds = %sg.barrier.bb.
  call void @dummy_sg_barrier()
  ret void
}

; Checks that the scalar arg use on the @bar call site is properly widened as the widened arg use.
; CHECK-LABEL: define void @_ZGVbN16v_foo(<16 x ptr> %ret.addr)
define void @foo(ptr %ret.addr) {
entry:
  call void @dummy_barrier.()
  call void @_Z18work_group_barrierj(i32 1)
; CHECK: call void @_ZGVbN16v_bar(<16 x ptr> %ret.addr)
  call void @bar(ptr %ret.addr)
  call void @dummy_barrier.()
  br label %sg.dummy.bb.

sg.dummy.bb.:                                     ; preds = %entry
  call void @dummy_sg_barrier()
  ret void
}

define void @bar(ptr %ret.addr) {
entry:
  call void @dummy_barrier.()
  %AllocaWGResult = alloca half, align 2
  store half 0xHFC00, ptr %AllocaWGResult, align 2
  call void @dummy_barrier.()
  br label %sg.dummy.bb.2

sg.dummy.bb.2:                                    ; preds = %entry
  call void @dummy_sg_barrier()
  %ptr = getelementptr %"class.sycl::_V1::detail::half_impl::half", ptr %ret.addr, i32 0, i32 0
  %v = load half, ptr %ptr, align 2
  %r = call half @_Z21work_group_reduce_maxDhPDh(half %v, ptr %AllocaWGResult)
  br label %sg.barrier.bb.1

sg.barrier.bb.1:                                  ; preds = %sg.dummy.bb.2
  call void @_Z17sub_group_barrierj(i32 1)
  call void @_Z18work_group_barrierj(i32 1)
  br label %sg.dummy.bb.4

sg.dummy.bb.4:                                    ; preds = %sg.barrier.bb.1
  call void @dummy_sg_barrier()
  %LoadWGFinalResult = load half, ptr %AllocaWGResult, align 2
  %CallFinalizeWG = call half @_Z30__finalize_work_group_identityDh(half %LoadWGFinalResult)
  call void @dummy_barrier.()
  br label %sg.dummy.bb.

sg.dummy.bb.:                                     ; preds = %sg.dummy.bb.4
  call void @dummy_sg_barrier()
  store half %CallFinalizeWG, ptr %ptr, align 2
  br label %sg.barrier.bb.

sg.barrier.bb.:                                   ; preds = %sg.dummy.bb.
  call void @_Z17sub_group_barrierj(i32 1)
  call void @_Z18work_group_barrierj(i32 1)
  br label %sg.dummy.bb.3

sg.dummy.bb.3:                                    ; preds = %sg.barrier.bb.
  call void @dummy_sg_barrier()
  ret void
}

declare void @dummy_barrier.()

declare void @_Z18work_group_barrierj(i32)

declare half @_Z21work_group_reduce_maxDhPDh(half noundef, ptr nocapture noundef)

declare half @_Z30__finalize_work_group_identityDh(half noundef returned)

declare void @_Z17sub_group_barrierj(i32)

declare void @dummy_sg_barrier()

!sycl.kernels = !{!0}

!0 = !{ptr @test_kernel}
!1 = !{i1 true}
!2 = !{i32 16}

; DEBUGIFY: WARNING: Missing line
; DEBUGIFY-NOT: WARNING
