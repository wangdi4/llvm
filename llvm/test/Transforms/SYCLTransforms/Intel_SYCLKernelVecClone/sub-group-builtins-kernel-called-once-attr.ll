; RUN: opt -passes=sycl-kernel-vec-clone -sycl-vector-variant-isa-encoding-override=AVX512Core -sycl-vect-info=%p/../Inputs/VectInfo64.gen %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-vec-clone -sycl-vector-variant-isa-encoding-override=AVX512Core -sycl-vect-info=%p/../Inputs/VectInfo64.gen %s -S -o - | FileCheck %s

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: convergent nounwind
; CHECK-LABEL: @_ZGVeN4uu_a
define spir_kernel void @a(ptr addrspace(1) nocapture readonly %a, ptr addrspace(1) nocapture %b) #0 !recommended_vector_length !1 !kernel_arg_base_type !2 !arg_type_null_val !3 {
entry:
  %call = tail call spir_func i64 @_Z13get_global_idj(i32 0) #3
  %slid = tail call i32 @_Z22get_sub_group_local_idv() #3
  %slid.reverse = sub nuw i32 16, %slid
  %arrayidx = getelementptr inbounds i32, ptr addrspace(1) %a, i64 %call
  %load = load i32, ptr addrspace(1) %arrayidx, align 4
  %trunc.i16 = trunc i32 %load to i16
  %trunc.i8 = trunc i32 %load to i8
  %zext = zext i32 %load to i64
  %val = call i32 @_Z23intel_sub_group_shuffleij(i32 %load, i32 %slid.reverse)
; CHECK: call i32 @_Z23intel_sub_group_shuffleij(i32 %load, i32 %slid.reverse) #[[SHUFFLE_ATTR_1:[0-9]+]]
  %val2 = call i32 @_Z23intel_sub_group_shufflejj(i32 %load, i32 %slid.reverse)
; CHECK: call i32 @_Z23intel_sub_group_shufflejj(i32 %load, i32 %slid.reverse) #[[SHUFFLE_ATTR_2:[0-9]+]]
  %vec2 = insertelement <3 x i32> undef, i32 %load, i32 0
  %val3 = call <3 x i32> @_Z23intel_sub_group_shuffleDv3_ij(<3 x i32> %vec2, i32 %slid.reverse)
; CHECK: call <3 x i32> @_Z23intel_sub_group_shuffleDv3_ij(<3 x i32> %vec2, i32 %slid.reverse) #[[SHUFFLE_ATTR_3:[0-9]+]]
  %vec3 = insertelement <16 x i16> undef, i16 %trunc.i16, i32 0
  %val4 = call <16 x i16> @_Z23intel_sub_group_shuffleDv16_sj(<16 x i16> %vec3, i32 %slid.reverse)
; CHECK: call <16 x i16> @_Z23intel_sub_group_shuffleDv16_sj(<16 x i16> %vec3, i32 %slid.reverse) #[[SHUFFLE_ATTR_4:[0-9]+]]

  %call1 = tail call spir_func i32 @_Z13sub_group_alli(i32 %load) #0
  %call2 = tail call spir_func i32 @_Z16get_sub_group_idv() #0

  %call3 = tail call spir_func i64 @_Z19sub_group_broadcastlj(i64 %zext, i32 0) #0
; CHECK: tail call spir_func i64 @_Z19sub_group_broadcastlj(i64 %zext, i32 0)  #[[BROADCAST_ATTR_1:[0-9]+]]
  %call4 = tail call spir_func i32 @_Z19sub_group_broadcastij(i32 %load, i32 0) #0
; CHECK: tail call spir_func i32 @_Z19sub_group_broadcastij(i32 %load, i32 0) #[[BROADCAST_ATTR_2:[0-9]+]]
  %call5 = tail call spir_func i16 @_Z25intel_sub_group_broadcastsj(i16 %trunc.i16, i32 0) #0
; CHECK: tail call spir_func i16 @_Z25intel_sub_group_broadcastsj(i16 %trunc.i16, i32 0) #[[BROADCAST_ATTR_3:[0-9]+]]
  %call6 = tail call spir_func i8  @_Z25intel_sub_group_broadcastcj(i8 %trunc.i8, i32 0) #0
; CHECK: tail call spir_func i8  @_Z25intel_sub_group_broadcastcj(i8 %trunc.i8, i32 0) #[[BROADCAST_ATTR_4:[0-9]+]]

  %call7 = tail call spir_func i32 @_Z20sub_group_reduce_addi(i32 %load) #0
; CHECK: tail call spir_func i32 @_Z20sub_group_reduce_addi(i32 %load) #[[REDUCE_ATTR_1:[0-9]+]]
  %call8 = tail call spir_func i16 @_Z26intel_sub_group_reduce_mins(i16 %trunc.i16) #0
; CHECK: tail call spir_func i16 @_Z26intel_sub_group_reduce_mins(i16 %trunc.i16) #[[REDUCE_ATTR_2:[0-9]+]]
  %call9 = tail call spir_func i8  @_Z26intel_sub_group_reduce_maxc(i8 %trunc.i8) #0
; CHECK: tail call spir_func i8  @_Z26intel_sub_group_reduce_maxc(i8 %trunc.i8) #[[REDUCE_ATTR_3:[0-9]+]]

  %call10 = tail call spir_func i32 @_Z28sub_group_scan_exclusive_addi(i32 %load) #0
; CHECK: tail call spir_func i32 @_Z28sub_group_scan_exclusive_addi(i32 %load) #[[EXCL_SCAN_ATTR_1:[0-9]+]]
  %call11 = tail call spir_func i16 @_Z34intel_sub_group_scan_exclusive_mins(i16 %trunc.i16) #0
; CHECK: tail call spir_func i16 @_Z34intel_sub_group_scan_exclusive_mins(i16 %trunc.i16) #[[EXCL_SCAN_ATTR_2:[0-9]+]]
  %call12 = tail call spir_func i8  @_Z34intel_sub_group_scan_exclusive_maxc(i8 %trunc.i8) #0
; CHECK: tail call spir_func i8  @_Z34intel_sub_group_scan_exclusive_maxc(i8 %trunc.i8) #[[EXCL_SCAN_ATTR_3:[0-9]+]]

  %call13 = tail call spir_func i32 @_Z28sub_group_scan_inclusive_addi(i32 %load) #0
; CHECK: tail call spir_func i32 @_Z28sub_group_scan_inclusive_addi(i32 %load) #[[INCL_SCAN_ATTR_1:[0-9]+]]
  %call14 = tail call spir_func i16 @_Z34intel_sub_group_scan_inclusive_mins(i16 %trunc.i16) #0
; CHECK: tail call spir_func i16 @_Z34intel_sub_group_scan_inclusive_mins(i16 %trunc.i16) #[[INCL_SCAN_ATTR_2:[0-9]+]]
  %call15 = tail call spir_func i8  @_Z34intel_sub_group_scan_inclusive_maxc(i8 %trunc.i8) #0
; CHECK: tail call spir_func i8  @_Z34intel_sub_group_scan_inclusive_maxc(i8 %trunc.i8) #[[INCL_SCAN_ATTR_3:[0-9]+]]

  %call16 = tail call spir_func i64 @_Z28intel_sub_group_shuffle_downllj(i64 %call, i64 42, i32 %slid.reverse)
; CHECK: tail call spir_func i64 @_Z28intel_sub_group_shuffle_downllj(i64 {{.*}}, i64 42, i32 %slid.reverse) #[[SHUFFLE_ATTR_5:[0-9]+]]

  %call17 = tail call spir_func <4 x i32> @_Z28intel_sub_group_shuffle_downDv4_iS_j(<4 x i32> <i32 1, i32 2, i32 3, i32 4>, <4 x i32> <i32 41, i32 42, i32 43, i32 44>, i32 %slid.reverse)
; CHECK: tail call spir_func <4 x i32> @_Z28intel_sub_group_shuffle_downDv4_iS_j(<4 x i32> <i32 1, i32 2, i32 3, i32 4>, <4 x i32> <i32 41, i32 42, i32 43, i32 44>, i32 %slid.reverse) #[[SHUFFLE_ATTR_6:[0-9]+]]

  %blk_read = call <2 x i32> @_Z27intel_sub_group_block_read2PU3AS1Kj(ptr addrspace(1) %a)
; CHECK: call <2 x i32> @_Z27intel_sub_group_block_read2PU3AS1Kj(ptr addrspace(1) %load.a) #[[BLOCKREAD_ATTR_1:.*]]
  %blk_read.x2 = mul <2 x i32> %blk_read, <i32 2, i32 2>
  call void @_Z28intel_sub_group_block_write2PU3AS1jDv2_j(ptr addrspace(1) %b, <2 x i32> %blk_read.x2)
; CHECK: call void @_Z28intel_sub_group_block_write2PU3AS1jDv2_j(ptr addrspace(1) %load.b, <2 x i32> %blk_read.x2) #[[BLOCKREAD_ATTR_2:.*]]
  %blk_read_short = call <2 x i16> @_Z30intel_sub_group_block_read_us2PU3AS1Kt(ptr addrspace(1) %a)
; CHECK: call <2 x i16> @_Z30intel_sub_group_block_read_us2PU3AS1Kt(ptr addrspace(1) %load.a) #[[BLOCKREAD_ATTR_3:.*]]
  %blk_read_short.x2 = mul <2 x i16> %blk_read_short, <i16 2, i16 2>
  call void @_Z31intel_sub_group_block_write_us2PU3AS1tDv2_t(ptr addrspace(1) %a, <2 x i16> %blk_read_short.x2)
; CHECK: call void @_Z31intel_sub_group_block_write_us2PU3AS1tDv2_t(ptr addrspace(1) %load.a, <2 x i16> %blk_read_short.x2) #[[BLOCKREAD_ATTR_4:.*]]

  %call18 = call <4 x i32> @_Z22intel_sub_group_balloti(i32 %load)
; CHECK: call <4 x i32> @_Z22intel_sub_group_balloti(i32 %load) #[[BALLOT_ATTR_1:[0-9]+]]

  %tobool = icmp ne i32 %load, 0
  %call19 = call i32 @intel_sub_group_ballot(i1 zeroext %tobool)
; CHECK: call i32 @intel_sub_group_ballot(i1 zeroext %tobool) #[[BALLOT_ATTR_2:[0-9]+]]

  %mul = mul i32 %call4, 1000
  %conv = zext i32 %mul to i64
  %add = add i64 %call, %conv
  %arrayidx4 = getelementptr inbounds i32, ptr addrspace(1) %b, i64 %add
  store i32 %call1, ptr addrspace(1) %arrayidx4, align 4

  %cmp = icmp eq i32 %slid, 0
  br i1 %cmp, label %slid.zero, label %slid.nonzero

slid.zero:
  br label %end

slid.nonzero:
  %masked_load = load i32, ptr addrspace(1) %arrayidx, align 4
  %masked_shuffle = call i32 @_Z23intel_sub_group_shuffleij(i32 %load, i32 4)
  br label %end

end:
  ret void
}

; Function Attrs: convergent
declare spir_func i32 @_Z13sub_group_alli(i32)  #1

; Function Attrs: convergent
declare spir_func i64 @_Z19sub_group_broadcastlj(i64, i32)  #1
declare spir_func i32 @_Z19sub_group_broadcastij(i32, i32)  #1
declare spir_func i16 @_Z25intel_sub_group_broadcastsj(i16, i32)  #1
declare spir_func i8 @_Z25intel_sub_group_broadcastcj(i8, i32)  #1

; Function Attrs: convergent
declare spir_func i32 @_Z20sub_group_reduce_addi(i32)  #1
declare spir_func i16 @_Z26intel_sub_group_reduce_mins(i16)  #1
declare spir_func i8  @_Z26intel_sub_group_reduce_maxc(i8)  #1

; Function Attrs: convergent
declare spir_func i32 @_Z28sub_group_scan_exclusive_addi(i32)  #1
declare spir_func i16 @_Z34intel_sub_group_scan_exclusive_mins(i16)  #1
declare spir_func i8  @_Z34intel_sub_group_scan_exclusive_maxc(i8)  #1

; Function Attrs: convergent
declare spir_func i32 @_Z28sub_group_scan_inclusive_addi(i32)  #1
declare spir_func i16 @_Z34intel_sub_group_scan_inclusive_mins(i16)  #1
declare spir_func i8  @_Z34intel_sub_group_scan_inclusive_maxc(i8)  #1

; Function Attrs: convergent nounwind readnone
declare spir_func i64 @_Z13get_global_idj(i32)  #2

; Function Attrs: convergent
declare spir_func i32 @_Z16get_sub_group_idv()  #1

; Function Attrs: convergent
declare <4 x i32> @_Z22intel_sub_group_balloti(i32)  #1

; Function Attrs: convergent
declare i32 @intel_sub_group_ballot(i1 zeroext)  #1

declare spir_func i32 @_Z23intel_sub_group_shuffleij(i32, i32)  #1
declare spir_func i32 @_Z23intel_sub_group_shufflejj(i32, i32)  #1
declare spir_func <3 x i32> @_Z23intel_sub_group_shuffleDv3_ij(<3 x i32>, i32)  #1
declare spir_func <16 x i16> @_Z23intel_sub_group_shuffleDv16_sj(<16 x i16>, i32)  #1
declare spir_func i32 @_Z22get_sub_group_local_idv()  #1
declare spir_func i64 @_Z28intel_sub_group_shuffle_downllj(i64, i64, i32)  #1

declare spir_func <4 x i32> @_Z28intel_sub_group_shuffle_downDv4_iS_j(<4 x i32>, <4 x i32>, i32)  #1

declare <2 x i32> @_Z27intel_sub_group_block_read2PU3AS1Kj(ptr addrspace(1))  #1
declare void @_Z28intel_sub_group_block_write2PU3AS1jDv2_j(ptr addrspace(1), <2 x i32>)  #1
declare <2 x i16> @_Z30intel_sub_group_block_read_us2PU3AS1Kt(ptr addrspace(1))  #1
declare void @_Z31intel_sub_group_block_write_us2PU3AS1tDv2_t(ptr addrspace(1), <2 x i16>)  #1

attributes #0 = { convergent nounwind }

; CHECK: attributes #[[SHUFFLE_ATTR_1]] = { {{.*}} "kernel-call-once" {{.*}} }
; CHECK: attributes #[[SHUFFLE_ATTR_2]] = { {{.*}} "kernel-call-once" {{.*}} }
; CHECK: attributes #[[SHUFFLE_ATTR_3]] = { {{.*}} "kernel-call-once" {{.*}} }
; CHECK: attributes #[[SHUFFLE_ATTR_4]] = { {{.*}} "kernel-call-once" {{.*}} }

; CHECK: attributes #[[BROADCAST_ATTR_1]] = { {{.*}} "kernel-call-once" {{.*}} }
; CHECK: attributes #[[BROADCAST_ATTR_2]] = { {{.*}} "kernel-call-once" {{.*}} }
; CHECK: attributes #[[BROADCAST_ATTR_3]] = { {{.*}} "kernel-call-once" {{.*}} }
; CHECK: attributes #[[BROADCAST_ATTR_4]] = { {{.*}} "kernel-call-once" {{.*}} }

; CHECK: attributes #[[REDUCE_ATTR_1]] = { {{.*}} "kernel-call-once" {{.*}} }
; CHECK: attributes #[[REDUCE_ATTR_2]] = { {{.*}} "kernel-call-once" {{.*}} }
; CHECK: attributes #[[REDUCE_ATTR_3]] = { {{.*}} "kernel-call-once" {{.*}} }

; CHECK: attributes #[[EXCL_SCAN_ATTR_1]] = { {{.*}} "kernel-call-once" {{.*}} }
; CHECK: attributes #[[EXCL_SCAN_ATTR_2]] = { {{.*}} "kernel-call-once" {{.*}} }
; CHECK: attributes #[[EXCL_SCAN_ATTR_3]] = { {{.*}} "kernel-call-once" {{.*}} }

; CHECK: attributes #[[INCL_SCAN_ATTR_1]] = { {{.*}} "kernel-call-once" {{.*}} }
; CHECK: attributes #[[INCL_SCAN_ATTR_2]] = { {{.*}} "kernel-call-once" {{.*}} }
; CHECK: attributes #[[INCL_SCAN_ATTR_3]] = { {{.*}} "kernel-call-once" {{.*}} }

; CHECK: attributes #[[SHUFFLE_ATTR_5]] = { {{.*}} "kernel-call-once" {{.*}} }
; CHECK: attributes #[[SHUFFLE_ATTR_6]] = { {{.*}} "kernel-call-once" {{.*}} }

; CHECK: attributes #[[BLOCKREAD_ATTR_1]] = { {{.*}} "kernel-call-once" {{.*}} }
; CHECK: attributes #[[BLOCKREAD_ATTR_2]] = { {{.*}} "kernel-call-once" {{.*}} }
; CHECK: attributes #[[BLOCKREAD_ATTR_3]] = { {{.*}} "kernel-call-once" {{.*}} }
; CHECK: attributes #[[BLOCKREAD_ATTR_4]] = { {{.*}} "kernel-call-once" {{.*}} }

; CHECK: attributes #[[BALLOT_ATTR_1]] = { {{.*}} "kernel-call-once" {{.*}} }
; CHECK: attributes #[[BALLOT_ATTR_2]] = { {{.*}} "kernel-call-once" {{.*}} }

!sycl.kernels = !{!0}

!0 = !{ptr @a}
!1 = !{i32 4}
!2 = !{!"int*", !"int*"}
!3 = !{ptr addrspace(1) null, ptr addrspace(1) null}

; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function _ZGVeN4uu_a {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN4uu_a {{.*}} call
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN4uu_a {{.*}} add
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN4uu_a {{.*}} icmp
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN4uu_a {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN4uu_a {{.*}} call
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN4uu_a {{.*}} br
; DEBUGIFY-NOT: WARNING
