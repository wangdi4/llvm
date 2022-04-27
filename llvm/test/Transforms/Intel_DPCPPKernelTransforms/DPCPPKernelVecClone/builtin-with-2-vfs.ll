; RUN: opt -dpcpp-kernel-vec-clone -dpcpp-vector-variant-isa-encoding-override=AVX512Core -dpcpp-vect-info=%p/../Inputs/VectInfo64.gen %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=dpcpp-kernel-vec-clone -dpcpp-vector-variant-isa-encoding-override=AVX512Core -dpcpp-vect-info=%p/../Inputs/VectInfo64.gen %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -dpcpp-kernel-vec-clone -vplan-vec -dpcpp-vector-variant-isa-encoding-override=AVX512Core -dpcpp-vect-info=%p/../Inputs/VectInfo64.gen %s -S -o - | FileCheck %s
; RUN: opt -passes=dpcpp-kernel-vec-clone,vplan-vec -dpcpp-vector-variant-isa-encoding-override=AVX512Core -dpcpp-vect-info=%p/../Inputs/VectInfo64.gen %s -S -o - | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

declare i64 @_Z12get_local_idj(i32)

declare i32 @_Z28sub_group_scan_inclusive_addi(i32)

define internal fastcc i64 @foo_sg(i32 addrspace(1)* %a) #0 !recommended_vector_length !1 {
entry:
  %call = tail call i64 @_Z12get_local_idj(i32 0)
  %conv = trunc i64 %call to i32
  %call1 = tail call i32 @_Z28sub_group_scan_inclusive_addi(i32 %conv)
  %sext = shl i64 %call, 32
  %idxprom = ashr exact i64 %sext, 32
  %arrayidx = getelementptr inbounds i32, i32 addrspace(1)* %a, i64 %idxprom
  store i32 %call1, i32 addrspace(1)* %arrayidx, align 4
  %call2 = tail call i32 @_Z22get_sub_group_local_idv()
  %conv3 = zext i32 %call2 to i64
  ret i64 %conv3
}

; CHECK-LABEL: @_ZGVeM16v_foo_sg
; CHECK: tail call i32 @_Z28sub_group_scan_inclusive_addi(i32 %add) [[ATTR_16:#[0-9]+]]

; CHECK-LABEL: @_ZGVeN16v_foo_sg
; CHECK: tail call i32 @_Z28sub_group_scan_inclusive_addi(i32 %add) [[ATTR_16:#[0-9]+]]

; CHECK-LABEL: @_ZGVeM8v_foo_sg
; CHECK: tail call i32 @_Z28sub_group_scan_inclusive_addi(i32 %add) [[ATTR_8:#[0-9]+]]

; CHECK-LABEL: @_ZGVeN8v_foo_sg
; CHECK: tail call i32 @_Z28sub_group_scan_inclusive_addi(i32 %add) [[ATTR_8:#[0-9]+]]

; CHECK-LABEL: @_ZGVeN8uu_basic
; CHECK: tail call i32 @_Z28sub_group_scan_inclusive_addi(i32 %add) [[ATTR_8:#[0-9]+]]

; CHECK: attributes [[ATTR_16]] = { "has-vplan-mask" "kernel-call-once" "vector-variants"="_ZGVbM16v__Z28sub_group_scan_inclusive_addi(_Z28sub_group_scan_inclusive_addDv16_iDv16_j)" }
; CHECK: attributes [[ATTR_8]] = { "has-vplan-mask" "kernel-call-once" "vector-variants"="_ZGVbM8v__Z28sub_group_scan_inclusive_addi(_Z28sub_group_scan_inclusive_addDv8_iDv8_j)" }

declare i32 @_Z22get_sub_group_local_idv()

define dso_local void @basic(i64 addrspace(1)* %local_id, i32 addrspace(1)* %scan_add) !recommended_vector_length !2 {
entry:
  %call.i = tail call i64 @_Z12get_local_idj(i32 0)
  %conv.i = trunc i64 %call.i to i32
  %call1.i = tail call i32 @_Z28sub_group_scan_inclusive_addi(i32 %conv.i)
  %sext.i = shl i64 %call.i, 32
  %idxprom.i = ashr exact i64 %sext.i, 32
  %arrayidx.i = getelementptr inbounds i32, i32 addrspace(1)* %scan_add, i64 %idxprom.i
  store i32 %call1.i, i32 addrspace(1)* %arrayidx.i, align 4
  %arrayidx = getelementptr inbounds i64, i64 addrspace(1)* %local_id, i64 %idxprom.i
  store i64 %idxprom.i, i64 addrspace(1)* %arrayidx, align 8
  ret void
}

define dso_local void @basic_sg(i64 addrspace(1)* %sg_local_id, i32 addrspace(1)* %sg_scan_add) !recommended_vector_length !2 {
entry:
  %call = tail call i64 @_Z12get_local_idj(i32 0)
  %call1 = tail call fastcc i64 @foo_sg(i32 addrspace(1)* %sg_scan_add)
  %arrayidx = getelementptr inbounds i64, i64 addrspace(1)* %sg_local_id, i64 %call
  store i64 %call1, i64 addrspace(1)* %arrayidx, align 8
  ret void
}

declare i64 @_Z14get_local_sizej(i32)

declare i64 @get_base_global_id.(i32)

attributes #0 = { "vector-variants"="_ZGVeM16v_foo_sg,_ZGVeN16v_foo_sg,_ZGVeM8v_foo_sg,_ZGVeN8v_foo_sg" }

!sycl.kernels = !{!0}

!0 = !{void (i64 addrspace(1)*, i32 addrspace(1)*)* @basic, void (i64 addrspace(1)*, i32 addrspace(1)*)* @basic_sg}
!1 = !{i32 16}
!2 = !{i32 8}

; DEBUGIFY:      WARNING: Instruction with empty DebugLoc in function _ZGVeM16v_foo_sg {{.*}} alloca
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16v_foo_sg {{.*}} alloca
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16v_foo_sg {{.*}} alloca
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16v_foo_sg {{.*}} bitcast
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16v_foo_sg {{.*}} store
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16v_foo_sg {{.*}} bitcast
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16v_foo_sg {{.*}} bitcast
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16v_foo_sg {{.*}} store
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16v_foo_sg {{.*}} trunc
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16v_foo_sg {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16v_foo_sg {{.*}} call
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16v_foo_sg {{.*}} sext
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16v_foo_sg {{.*}} add
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16v_foo_sg {{.*}} sext
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16v_foo_sg {{.*}} getelementptr
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16v_foo_sg {{.*}} load
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16v_foo_sg {{.*}} icmp
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16v_foo_sg {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16v_foo_sg {{.*}} getelementptr
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16v_foo_sg {{.*}} load
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16v_foo_sg {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16v_foo_sg {{.*}} add
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16v_foo_sg {{.*}} icmp
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16v_foo_sg {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16v_foo_sg {{.*}} call
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16v_foo_sg {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16v_foo_sg {{.*}} bitcast
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16v_foo_sg {{.*}} load
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16v_foo_sg {{.*}} ret
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16v_foo_sg {{.*}} alloca
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16v_foo_sg {{.*}} alloca
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16v_foo_sg {{.*}} bitcast
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16v_foo_sg {{.*}} store
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16v_foo_sg {{.*}} bitcast
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16v_foo_sg {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16v_foo_sg {{.*}} call
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16v_foo_sg {{.*}} getelementptr
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16v_foo_sg {{.*}} load
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16v_foo_sg {{.*}} add
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16v_foo_sg {{.*}} icmp
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16v_foo_sg {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16v_foo_sg {{.*}} call
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16v_foo_sg {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16v_foo_sg {{.*}} bitcast
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16v_foo_sg {{.*}} load
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16v_foo_sg {{.*}} ret
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM8v_foo_sg {{.*}} alloca
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM8v_foo_sg {{.*}} alloca
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM8v_foo_sg {{.*}} alloca
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM8v_foo_sg {{.*}} bitcast
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM8v_foo_sg {{.*}} store
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM8v_foo_sg {{.*}} bitcast
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM8v_foo_sg {{.*}} bitcast
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM8v_foo_sg {{.*}} store
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM8v_foo_sg {{.*}} trunc
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM8v_foo_sg {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM8v_foo_sg {{.*}} call
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM8v_foo_sg {{.*}} sext
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM8v_foo_sg {{.*}} add
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM8v_foo_sg {{.*}} sext
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM8v_foo_sg {{.*}} getelementptr
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM8v_foo_sg {{.*}} load
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM8v_foo_sg {{.*}} icmp
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM8v_foo_sg {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM8v_foo_sg {{.*}} getelementptr
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM8v_foo_sg {{.*}} load
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM8v_foo_sg {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM8v_foo_sg {{.*}} add
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM8v_foo_sg {{.*}} icmp
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM8v_foo_sg {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM8v_foo_sg {{.*}} call
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM8v_foo_sg {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM8v_foo_sg {{.*}} bitcast
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM8v_foo_sg {{.*}} load
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM8v_foo_sg {{.*}} ret
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN8v_foo_sg {{.*}} alloca
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN8v_foo_sg {{.*}} alloca
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN8v_foo_sg {{.*}} bitcast
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN8v_foo_sg {{.*}} store
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN8v_foo_sg {{.*}} bitcast
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN8v_foo_sg {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN8v_foo_sg {{.*}} call
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN8v_foo_sg {{.*}} getelementptr
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN8v_foo_sg {{.*}} load
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN8v_foo_sg {{.*}} add
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN8v_foo_sg {{.*}} icmp
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN8v_foo_sg {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN8v_foo_sg {{.*}} call
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN8v_foo_sg {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN8v_foo_sg {{.*}} bitcast
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN8v_foo_sg {{.*}} load
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN8v_foo_sg {{.*}} ret
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN8uu_basic {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN8uu_basic {{.*}} call
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN8uu_basic {{.*}} add
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN8uu_basic {{.*}} icmp
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN8uu_basic {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN8uu_basic {{.*}} call
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN8uu_basic {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN8uu_basic_sg {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN8uu_basic_sg {{.*}} call
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN8uu_basic_sg {{.*}} add
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN8uu_basic_sg {{.*}} icmp
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN8uu_basic_sg {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN8uu_basic_sg {{.*}} call
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN8uu_basic_sg {{.*}} br
; DEBUGIFY-NEXT: CheckModuleDebugify: PASS
