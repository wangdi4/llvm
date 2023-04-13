; RUN: opt -passes=sycl-kernel-builtin-call-to-inst -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-builtin-call-to-inst -S %s | FileCheck %s -check-prefix=CHECK-NONOPAQUE
; RUN: opt -opaque-pointers -passes=sycl-kernel-builtin-call-to-inst -S %s | FileCheck %s -check-prefix=CHECK-OPAQUE

; key-only private close sort
define dso_local void @sort1(i8 addrspace(1)* noundef align 1 %data1, i8 addrspace(1)* noundef align 1 %scratch){
; CHECK-NONOPAQUE:    [[NEW_DATA:%.*]] = addrspacecast i8 addrspace(1)* [[DATA1:%.*]] to i8 addrspace(4)*
; CHECK-NONOPAQUE:    [[NEW_DATA1:%.*]] = addrspacecast i8 addrspace(1)* [[SCRATCH:%.*]] to i8 addrspace(4)*
; CHECK-NONOPAQUE:    call void @_Z73__devicelib_default_work_group_private_sort_close_ascending_p1i8_u32_p1i8PU3AS4cjS0_(i8 addrspace(4)* [[NEW_DATA]], i32 4, i8 addrspace(4)* [[NEW_DATA1]])

; CHECK-OPAQUE:    [[NEW_DATA:%.*]] = addrspacecast ptr addrspace(1) [[DATA1:%.*]] to ptr addrspace(4)
; CHECK-OPAQUE:    [[NEW_DATA1:%.*]] = addrspacecast ptr addrspace(1) [[SCRATCH:%.*]] to ptr addrspace(4)
; CHECK-OPAQUE:    call void @_Z73__devicelib_default_work_group_private_sort_close_ascending_p1i8_u32_p1i8PU3AS4cjS0_(ptr addrspace(4) [[NEW_DATA]], i32 4, ptr addrspace(4) [[NEW_DATA1]])

  tail call void @__devicelib_default_work_group_private_sort_close_ascending_p1i8_u32_p1i8(i8 addrspace(1)* %data1, i32 4, i8 addrspace(1)* %scratch)
  ret void
}

; key-only private spread sort
define dso_local void @sort2(i16 addrspace(1)* noundef align 1 %data1, i8 addrspace(1)* noundef align 1 %scratch){
; CHECK-NONOPAQUE:    [[NEW_DATA:%.*]] = addrspacecast i16 addrspace(1)* [[DATA1:%.*]] to i16 addrspace(4)*
; CHECK-NONOPAQUE:    [[NEW_DATA1:%.*]] = addrspacecast i8 addrspace(1)* [[SCRATCH:%.*]] to i8 addrspace(4)*
; CHECK-NONOPAQUE:    call void @_Z75__devicelib_default_work_group_private_sort_spread_ascending_p1i16_u32_p1i8PU3AS4sjPU3AS4c(i16 addrspace(4)* [[NEW_DATA]], i32 4, i8 addrspace(4)* [[NEW_DATA1]])

; CHECK-OPAQUE:    [[NEW_DATA:%.*]] = addrspacecast ptr addrspace(1) [[DATA1:%.*]] to ptr addrspace(4)
; CHECK-OPAQUE:    [[NEW_DATA1:%.*]] = addrspacecast ptr addrspace(1) [[SCRATCH:%.*]] to ptr addrspace(4)
; CHECK-OPAQUE:    call void @_Z75__devicelib_default_work_group_private_sort_spread_ascending_p1i16_u32_p1i8PU3AS4sjPU3AS4c(ptr addrspace(4) [[NEW_DATA]], i32 4, ptr addrspace(4) [[NEW_DATA1]])

  tail call void @__devicelib_default_work_group_private_sort_spread_ascending_p1i16_u32_p1i8(i16 addrspace(1)* %data1, i32 4, i8 addrspace(1)* %scratch)
  ret void
}

; key-only joint sort
define dso_local void @sort3(float addrspace(1)* noundef align 1 %data1, i8 addrspace(1)* noundef align 1 %scratch){
; CHECK-NONOPAQUE:    [[NEW_DATA:%.*]] = addrspacecast float addrspace(1)* [[DATA1:%.*]] to float addrspace(4)*
; CHECK-NONOPAQUE:    [[NEW_DATA1:%.*]] = addrspacecast i8 addrspace(1)* [[SCRATCH:%.*]] to i8 addrspace(4)*
; CHECK-NONOPAQUE:    call void @_Z66__devicelib_default_work_group_joint_sort_ascending_p1f32_u32_p1i8PU3AS4fjPU3AS4c(float addrspace(4)* [[NEW_DATA]], i32 4, i8 addrspace(4)* [[NEW_DATA1]])

; CHECK-OPAQUE:    [[NEW_DATA:%.*]] = addrspacecast ptr addrspace(1) [[DATA1:%.*]] to ptr addrspace(4)
; CHECK-OPAQUE:    [[NEW_DATA1:%.*]] = addrspacecast ptr addrspace(1) [[SCRATCH:%.*]] to ptr addrspace(4)
; CHECK-OPAQUE:    call void @_Z66__devicelib_default_work_group_joint_sort_ascending_p1f32_u32_p1i8PU3AS4fjPU3AS4c(ptr addrspace(4) [[NEW_DATA]], i32 4, ptr addrspace(4) [[NEW_DATA1]])

  tail call void @__devicelib_default_work_group_joint_sort_ascending_p1f32_u32_p1i8(float addrspace(1)* %data1, i32 4, i8 addrspace(1)* %scratch)
  ret void
}

; key-value private close sort scalar version
define dso_local void @sort4(i16 addrspace(1)* noundef align 1 %data1, half addrspace(1)* noundef align 1 %data2, i8 addrspace(1)* noundef align 1 %scratch){
; CHECK-NONOPAQUE:    [[NEW_DATA:%.*]] = addrspacecast i16 addrspace(1)* [[DATA1:%.*]] to i16 addrspace(4)*
; CHECK-NONOPAQUE:    [[NEW_DATA1:%.*]] = addrspacecast half addrspace(1)* [[DATA2:%.*]] to half addrspace(4)*
; CHECK-NONOPAQUE:    [[NEW_DATA2:%.*]] = addrspacecast i8 addrspace(1)* [[SCRATCH:%.*]] to i8 addrspace(4)*
; CHECK-NONOPAQUE:    call void @_Z80__devicelib_default_work_group_private_sort_close_ascending_p1i16_p1f16_u32_p1i8PU3AS4sPU3AS4DhjPU3AS4c(i16 addrspace(4)* [[NEW_DATA]], half addrspace(4)* [[NEW_DATA1]], i32 4, i8 addrspace(4)* [[NEW_DATA2]])

; CHECK-OPAQUE:    [[NEW_DATA:%.*]] = addrspacecast ptr addrspace(1) [[DATA1:%.*]] to ptr addrspace(4)
; CHECK-OPAQUE:    [[NEW_DATA1:%.*]] = addrspacecast ptr addrspace(1) [[DATA2:%.*]] to ptr addrspace(4)
; CHECK-OPAQUE:    [[NEW_DATA2:%.*]] = addrspacecast ptr addrspace(1) [[SCRATCH:%.*]] to ptr addrspace(4)
; CHECK-OPAQUE:    call void @_Z80__devicelib_default_work_group_private_sort_close_ascending_p1i16_p1f16_u32_p1i8PU3AS4sPU3AS4DhjPU3AS4c(ptr addrspace(4) [[NEW_DATA]], ptr addrspace(4) [[NEW_DATA1]], i32 4, ptr addrspace(4) [[NEW_DATA2]])

  tail call void @__devicelib_default_work_group_private_sort_close_ascending_p1i16_p1f16_u32_p1i8(i16 addrspace(1)* %data1, half addrspace(1)* %data2, i32 4, i8 addrspace(1)* %scratch)
  ret void
}

; key-value private spread sort scalar version
define dso_local void @sort5(double addrspace(1)* noundef align 1 %data1, i8 addrspace(1)* noundef align 1 %data2, i8 addrspace(1)* noundef align 1 %scratch){
; CHECK-NONOPAQUE:  [[NEW_DATA:%.*]] = addrspacecast double addrspace(1)* [[DATA1:%.*]] to double addrspace(4)*
; CHECK-NONOPAQUE:  [[NEW_DATA1:%.*]] = addrspacecast i8 addrspace(1)* [[DATA2:%.*]] to i8 addrspace(4)*
; CHECK-NONOPAQUE:  [[NEW_DATA2:%.*]] = addrspacecast i8 addrspace(1)* [[SCRATCH:%.*]] to i8 addrspace(4)*
; CHECK-NONOPAQUE:  call void @_Z80__devicelib_default_work_group_private_sort_spread_ascending_p1f64_p1i8_u32_p1i8PU3AS4dPU3AS4cjS2_(double addrspace(4)* [[NEW_DATA]], i8 addrspace(4)* [[NEW_DATA1]], i32 4, i8 addrspace(4)* [[NEW_DATA2]])

; CHECK-OPAQUE:    [[NEW_DATA:%.*]] = addrspacecast ptr addrspace(1) [[DATA1:%.*]] to ptr addrspace(4)
; CHECK-OPAQUE:    [[NEW_DATA1:%.*]] = addrspacecast ptr addrspace(1) [[DATA2:%.*]] to ptr addrspace(4)
; CHECK-OPAQUE:    [[NEW_DATA2:%.*]] = addrspacecast ptr addrspace(1) [[SCRATCH:%.*]] to ptr addrspace(4)
; CHECK-OPAQUE:    call void @_Z80__devicelib_default_work_group_private_sort_spread_ascending_p1f64_p1i8_u32_p1i8PU3AS4dPU3AS4cjS2_(ptr addrspace(4) [[NEW_DATA]], ptr addrspace(4) [[NEW_DATA1]], i32 4, ptr addrspace(4) [[NEW_DATA2]])

  tail call void @__devicelib_default_work_group_private_sort_spread_ascending_p1f64_p1i8_u32_p1i8(double addrspace(1)* %data1, i8 addrspace(1)* %data2, i32 4, i8 addrspace(1)* %scratch)
  ret void
}

; key-value joint sort scalar version
define dso_local void @sort6(i32 addrspace(1)* noundef align 1 %data1, i32 addrspace(1)* noundef align 1 %data2, i8 addrspace(1)* noundef align 1 %scratch){
; CHECK-NONOPAQUE:    [[NEW_DATA:%.*]] = addrspacecast i32 addrspace(1)* [[DATA1:%.*]] to i32 addrspace(4)*
; CHECK-NONOPAQUE:    [[NEW_DATA1:%.*]] = addrspacecast i32 addrspace(1)* [[DATA2:%.*]] to i32 addrspace(4)*
; CHECK-NONOPAQUE:    [[NEW_DATA2:%.*]] = addrspacecast i8 addrspace(1)* [[SCRATCH:%.*]] to i8 addrspace(4)*
; CHECK-NONOPAQUE:    call void @_Z72__devicelib_default_work_group_joint_sort_ascending_p3u32_p3i32_u32_p1i8PU3AS4jPU3AS4ijPU3AS4c(i32 addrspace(4)* [[NEW_DATA]], i32 addrspace(4)* [[NEW_DATA1]], i32 4, i8 addrspace(4)* [[NEW_DATA2]])

; CHECK-OPAQUE:    [[NEW_DATA:%.*]] = addrspacecast ptr addrspace(1) [[DATA1:%.*]] to ptr addrspace(4)
; CHECK-OPAQUE:    [[NEW_DATA1:%.*]] = addrspacecast ptr addrspace(1) [[DATA2:%.*]] to ptr addrspace(4)
; CHECK-OPAQUE:    [[NEW_DATA2:%.*]] = addrspacecast ptr addrspace(1) [[SCRATCH:%.*]] to ptr addrspace(4)
; CHECK-OPAQUE:    call void @_Z72__devicelib_default_work_group_joint_sort_ascending_p3u32_p3i32_u32_p1i8PU3AS4jPU3AS4ijPU3AS4c(ptr addrspace(4) [[NEW_DATA]], ptr addrspace(4) [[NEW_DATA1]], i32 4, ptr addrspace(4) [[NEW_DATA2]])

  tail call void @__devicelib_default_work_group_joint_sort_ascending_p3u32_p3i32_u32_p1i8(i32 addrspace(1)* %data1, i32 addrspace(1)* %data2, i32 4, i8 addrspace(1)* %scratch)
  ret void
}

define dso_local void @sort7(i32 %data1){
; CHECK-NONOPAQUE:   call void @_Z56__devicelib_default_sub_group_private_sort_ascending_i32i(i32 [[DATA1:%.*]])
; CHECK-OPAQUE:  call void @_Z56__devicelib_default_sub_group_private_sort_ascending_i32i(i32 [[DATA1:%.*]])
  %call1 = call i32 @__devicelib_default_sub_group_private_sort_ascending_i32(i32 %data1)
  ret void
}

declare void @__devicelib_default_work_group_private_sort_close_ascending_p1i8_u32_p1i8(i8 addrspace(1)*, i32, i8 addrspace(1)*)

declare void @__devicelib_default_work_group_private_sort_spread_ascending_p1i16_u32_p1i8(i16 addrspace(1)*, i32, i8 addrspace(1)*)

declare void @__devicelib_default_work_group_joint_sort_ascending_p1f32_u32_p1i8(float addrspace(1)*, i32, i8 addrspace(1)*)

declare void @__devicelib_default_work_group_private_sort_close_ascending_p1i16_p1f16_u32_p1i8(i16 addrspace(1)*, half addrspace(1)*, i32, i8 addrspace(1)*)

declare void @__devicelib_default_work_group_private_sort_spread_ascending_p1f64_p1i8_u32_p1i8(double addrspace(1)*, i8 addrspace(1)*,  i32, i8 addrspace(1)*)

declare void @__devicelib_default_work_group_joint_sort_ascending_p3u32_p3i32_u32_p1i8(i32 addrspace(1)*, i32 addrspace(1)*, i32, i8 addrspace(1)*)

declare i32 @__devicelib_default_sub_group_private_sort_ascending_i32(i32)

; DEBUGIFY-NOT: WARNING
