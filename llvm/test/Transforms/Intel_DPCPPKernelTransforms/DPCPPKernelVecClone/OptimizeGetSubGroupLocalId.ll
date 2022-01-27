; RUN: opt -dpcpp-kernel-set-vf -dpcpp-kernel-vec-clone -dpcpp-vector-variant-isa-encoding-override=AVX512Core %s -S -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes="dpcpp-kernel-set-vf,dpcpp-kernel-vec-clone" -dpcpp-vector-variant-isa-encoding-override=AVX512Core %s -S -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -dpcpp-kernel-set-vf -dpcpp-kernel-vec-clone -dpcpp-vector-variant-isa-encoding-override=AVX512Core %s -S -o - | FileCheck %s
; RUN: opt -passes="dpcpp-kernel-set-vf,dpcpp-kernel-vec-clone" -dpcpp-vector-variant-isa-encoding-override=AVX512Core %s -S -o - | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define void @_ZTSZ4mainE9broadcast(i32 addrspace(1)* %_arg_, i32 addrspace(1)* %_arg_1) local_unnamed_addr #0 !kernel_has_sub_groups !1 !recommended_vector_length !2 {
; CHECK:  define void @_ZGVeN8uu__ZTSZ4mainE9broadcast(
; CHECK:       simd.loop.header:
; CHECK-NEXT:    [[INDEX0:%.*]] = phi i32 [ 0, [[SIMD_LOOP_PREHEADER0:%.*]] ], [ [[INDVAR0:%.*]], [[SIMD_LOOP_LATCH0:%.*]] ]
; CHECK-NEXT:    [[DOTSEXT0:%.*]] = sext i32 [[INDEX0]] to i64
; CHECK-NEXT:    [[ADD0:%.*]] = add nuw i32 [[TMP1:%.*]], [[INDEX0]]
; CHECK:         [[CALL2_I_I_I_I_I0:%.*]] = tail call i32 @_Z19sub_group_broadcastjj(i32 [[ADD0]], i32 0)
; CHECK-NEXT:    [[CONV3_I0:%.*]] = sext i32 [[CALL2_I_I_I_I_I0]] to i64
; CHECK-NEXT:    [[ADD_I0:%.*]] = add nsw i64 [[DOTSEXT0]], [[CONV3_I0]]
; CHECK-NEXT:    [[ARRAYIDX_I230:%.*]] = getelementptr inbounds i32, i32 addrspace(1)* [[LOAD__ARG_10:%.*]], i64 [[ADD_I0]]
; CHECK-NEXT:    [[TMP3:%.*]] = load i32, i32 addrspace(1)* [[ARRAYIDX_I230]], align 4
;
entry:
  %0 = tail call i64 @_Z13get_global_idj(i32 0)
  %cmp.i.i = icmp ult i64 %0, 2147483648
  tail call void @llvm.assume(i1 %cmp.i.i)
  %conv.i = trunc i64 %0 to i32
  %call2.i.i.i.i.i = tail call i32 @_Z19sub_group_broadcastjj(i32 %conv.i, i32 0)
  %conv3.i = sext i32 %call2.i.i.i.i.i to i64
  %1 = tail call i32 @_Z22get_sub_group_local_idv()
  %conv.i.i = zext i32 %1 to i64
  %add.i = add nsw i64 %conv.i.i, %conv3.i
  %arrayidx.i23 = getelementptr inbounds i32, i32 addrspace(1)* %_arg_1, i64 %add.i
  %2 = load i32, i32 addrspace(1)* %arrayidx.i23, align 4
  %arrayidx6.i24 = getelementptr inbounds i32, i32 addrspace(1)* %_arg_, i64 %0
  store i32 %2, i32 addrspace(1)* %arrayidx6.i24, align 4
  ret void
}

declare void @llvm.assume(i1 noundef) #1

declare i64 @_Z13get_global_idj(i32) local_unnamed_addr #2

declare i32 @_Z19sub_group_broadcastjj(i32, i32) local_unnamed_addr #3

declare i32 @_Z22get_sub_group_local_idv() local_unnamed_addr #2

!sycl.kernels = !{!0}

!0 = !{void (i32 addrspace(1)*, i32 addrspace(1)*)* @_ZTSZ4mainE9broadcast}
!1 = !{i1 true}
!2 = !{i32 8}

; DEBUGIFY: CheckModuleDebugify: PASS
