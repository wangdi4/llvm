; RUN: opt -passes='debugify,sycl-kernel-sg-emu-loop-construct,check-debugify' -S %s -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes='sycl-kernel-sg-emu-loop-construct' -S %s | FileCheck %s

; This test checks SGLoopConstruct:: updateTIDCalls(), resolveSGLIdCalls()
; 1. All uses of get_global_id(0) are replaced with gid + sglid
; 2. All uses of get_local_id(0) are replaced with lid + sglid
; 3. All uses of get_sub_group_local_id() are resolved as a load inst from the sg.lid.ptr.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

declare i64 @_Z13get_global_idj(i32)
declare i64 @_Z12get_local_idj(i32)
declare i64 @_Z12get_group_idj(i32)
declare i64 @_Z14get_local_sizej(i32)
declare i32 @_Z13sub_group_alli(i32)

declare void @dummybarrier.()
declare void @_Z7barrierj(i32)

declare void @_Z17sub_group_barrierj(i32)
declare void @dummy_sg_barrier()

define void @testKernel(ptr addrspace(1) %a, ptr addrspace(1) %b) !kernel_has_sub_groups !1 !sg_emu_size !2 !kernel_arg_base_type !3 !arg_type_null_val !4 {
sg.loop.exclude:
  call void @dummybarrier.()
  %w.call1 = alloca <16 x i64>, align 128
  %w.call2 = alloca <16 x i64>, align 128
  %w.call3 = alloca <16 x i64>, align 128
  %w. = alloca <16 x i32>, align 64
  %u.call4 = alloca i32, align 4
  br label %entry

entry:                                            ; preds = %sg.loop.exclude
  br label %sg.dummy.bb.

sg.dummy.bb.:                                     ; preds = %entry
; Every BB that uses SGLId, will begin with a load form sg.lid.ptr
; CHECK: [[SGLID:%sg.lid.*]] = load i32, ptr [[P_SGLID:%sg.lid.ptr.*]]

  call void @dummy_sg_barrier()
  %call = tail call i64 @_Z13get_global_idj(i32 0) #5
; GID(0) is added by SGLId
; CHECK: [[GID:%.*]] = tail call i64 @_Z13get_global_idj(i32 0)
; CHECK-NEXT: [[SGLID_EXT:%.*]] = zext i32 [[SGLID]] to i64
; CHECK-NEXT: [[SGLID_ADD:%.*]] = add i64 [[SGLID_EXT]], [[GID]]

  %call1 = tail call i64 @_Z12get_local_idj(i32 0) #5
; LID(0) is added by SGLId
; CHECK: [[LID:%.*]] = tail call i64 @_Z12get_local_idj(i32 0)
; CHECK-NEXT: [[SGLID_EXT1:%.*]] = zext i32 [[SGLID]] to i64
; CHECK-NEXT: [[SGLID_ADD1:%.*]] = add i64 [[SGLID_EXT1]], [[LID]]

  %sg.lid.1 = call i32 @_Z22get_sub_group_local_idv()
  %0 = getelementptr <16 x i64>, ptr %w.call1, i32 0, i32 %sg.lid.1
  store i64 %call1, ptr %0, align 8
; _Z22get_sub_group_local_idv() will be resolved
; CHECK-NOT: call i32 @_Z22get_sub_group_local_idv()
; CHECK: [[GEP:%.*]] = getelementptr <16 x i64>, ptr %w.call1, i32 0, i32 [[SGLID]]
; CHECK-NOT: store i64 %call1, ptr [[GEP]], align 8
; The use of LID is replaced with LID+SGLId
; CHECK: store i64 [[SGLID_ADD1]], ptr [[GEP]], align 8

  %call2 = tail call i64 @_Z12get_group_idj(i32 0) #5
  %sg.lid.3 = call i32 @_Z22get_sub_group_local_idv()
  %1 = getelementptr <16 x i64>, ptr %w.call2, i32 0, i32 %sg.lid.3
; CHECK-NOT: call i32 @_Z22get_sub_group_local_idv()
; CHECK: getelementptr <16 x i64>, ptr %w.call2, i32 0, i32 [[SGLID]]

  store i64 %call2, ptr %1, align 8
  %call3 = tail call i64 @_Z14get_local_sizej(i32 0) #5
  %sg.lid.5 = call i32 @_Z22get_sub_group_local_idv()
  %2 = getelementptr <16 x i64>, ptr %w.call3, i32 0, i32 %sg.lid.5
; CHECK-NOT: call i32 @_Z22get_sub_group_local_idv()
; CHECK: getelementptr <16 x i64>, ptr %w.call3, i32 0, i32 [[SGLID]]

  store i64 %call3, ptr %2, align 8
  %ptridx = getelementptr inbounds i32, ptr addrspace(1) %b, i64 %call
; The use of GID is replaced with GID+SGLId
; CHECK-NOT: %ptridx = getelementptr inbounds i32, ptr addrspace(1) %b, i64 %call
; CHECK: %ptridx = getelementptr inbounds i32, ptr addrspace(1) %b, i64 [[SGLID_ADD]]

  %3 = load i32, ptr addrspace(1) %ptridx, align 4
  %sg.lid.6 = call i32 @_Z22get_sub_group_local_idv()
  %4 = getelementptr <16 x i32>, ptr %w., i32 0, i32 %sg.lid.6
; CHECK-NOT: call i32 @_Z22get_sub_group_local_idv()
; CHECK: getelementptr <16 x i32>, ptr %w., i32 0, i32 [[SGLID]]
  store i32 %3, ptr %4, align 4
  br label %sg.barrier.bb.6

sg.barrier.bb.6:                                  ; preds = %sg.dummy.bb.
  call void @_Z17sub_group_barrierj(i32 1)
  %5 = load <16 x i32>, ptr %w., align 64
  %sg.size. = call i32 @_Z18get_sub_group_sizev()
  %6 = zext i32 %sg.size. to i64
  %.splatinsert = insertelement <16 x i64> undef, i64 %6, i32 0
  %.splat = shufflevector <16 x i64> %.splatinsert, <16 x i64> undef, <16 x i32> zeroinitializer
  %mask.i1 = icmp ult <16 x i64> <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>, %.splat
  %mask.i32 = sext <16 x i1> %mask.i1 to <16 x i32>
  %7 = call <16 x i32> @_Z13sub_group_allDv16_iDv16_j(<16 x i32> %5, <16 x i32> %mask.i32)
  %8 = extractelement <16 x i32> %7, i32 0
  store i32 %8, ptr %u.call4, align 4
  br label %sg.dummy.bb.8

sg.dummy.bb.8:                                    ; preds = %sg.barrier.bb.6
; CHECK-LABEL: sg.dummy.bb.8:
; CHECK: [[SGLID1:%sg.lid.*]] = load i32, ptr [[P_SGLID:%sg.lid.ptr.*]]
  call void @dummy_sg_barrier()
  %sg.lid.2 = call i32 @_Z22get_sub_group_local_idv()
  %9 = getelementptr <16 x i64>, ptr %w.call2, i32 0, i32 %sg.lid.2
; CHECK-NOT: call i32 @_Z22get_sub_group_local_idv()
; CHECK: getelementptr <16 x i64>, ptr %w.call2, i32 0, i32 [[SGLID1]]

  %10 = load i64, ptr %9, align 8
  %sg.lid.4 = call i32 @_Z22get_sub_group_local_idv()
  %11 = getelementptr <16 x i64>, ptr %w.call3, i32 0, i32 %sg.lid.4
; CHECK-NOT: call i32 @_Z22get_sub_group_local_idv()
; CHECK: getelementptr <16 x i64>, ptr %w.call3, i32 0, i32 [[SGLID1]]

  %12 = load i64, ptr %11, align 8
  %mul = mul i64 %12, %10
  %sg.lid. = call i32 @_Z22get_sub_group_local_idv()
  %13 = getelementptr <16 x i64>, ptr %w.call1, i32 0, i32 %sg.lid.
; CHECK-NOT: call i32 @_Z22get_sub_group_local_idv()
; CHECK: getelementptr <16 x i64>, ptr %w.call1, i32 0, i32 [[SGLID1]]

  %14 = load i64, ptr %13, align 8
  %add = add i64 %mul, %14
  %ptridx5 = getelementptr inbounds i32, ptr addrspace(1) %a, i64 %add
  %15 = load i32, ptr %u.call4, align 4
  store i32 %15, ptr addrspace(1) %ptridx5, align 4
  br label %sg.barrier.bb.

sg.barrier.bb.:                                   ; preds = %sg.dummy.bb.8
  call void @_Z17sub_group_barrierj(i32 1)
  call void @_Z7barrierj(i32 1)
  br label %sg.dummy.bb.7

sg.dummy.bb.7:                                    ; preds = %sg.barrier.bb.
  call void @dummy_sg_barrier()
  ret void
}

declare <16 x i32> @_Z13sub_group_allDv16_iDv16_j(<16 x i32>, <16 x i32>)
declare i32 @_Z22get_sub_group_local_idv()
declare i32 @_Z18get_sub_group_sizev()

!sycl.kernels = !{!0}

!0 = !{ptr @testKernel}
!1 = !{i1 true}
!2 = !{i32 16}
!3 = !{!"int*", !"int*"}
!4 = !{ptr addrspace(1) null, ptr addrspace(1) null}

; TODO: The get_sub_group_local_id function will be optimized by this pass. Will
; create tls storage and attach its debug info to the load instruction from
; tls storage.
; DEBUGIFY-COUNT-6: WARNING: Missing line

; DEBUGIFY-NOT: WARNING
