; RUN: %oclopt --ocl-vecclone --ocl-vector-variant-isa-encoding-override=AVX512Core < %s -S -o - | FileCheck %s

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
; CHECK: tail call i32 @_Z28sub_group_scan_inclusive_addi(i32 %add) #4

; CHECK-LABEL: @_ZGVeN16v_foo_sg
; CHECK: tail call i32 @_Z28sub_group_scan_inclusive_addi(i32 %add) #4

; CHECK-LABEL: @_ZGVeM8v_foo_sg
; CHECK: tail call i32 @_Z28sub_group_scan_inclusive_addi(i32 %add) #5

; CHECK-LABEL: @_ZGVeN8v_foo_sg
; CHECK: tail call i32 @_Z28sub_group_scan_inclusive_addi(i32 %add) #5

; CHECK-LABEL: @_ZGVeN8uu_basic
; CHECK: tail call i32 @_Z28sub_group_scan_inclusive_addi(i32 %add) #5

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
