; RUN: opt -passes=sycl-kernel-equalizer -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-equalizer -S %s | FileCheck %s

; Thet test is to chack
; 1) sort builtin's mangling
; 2) casting pointer to generic
; 3) handling sort alisa info are correct (p3 suffix -> p1 suffix)

; key-only private close sort
define dso_local void @sort1(ptr addrspace(3) noundef align 1 %data1, ptr addrspace(1) noundef align 1 %scratch){
; CHECK-LABEL: define dso_local void @sort1
; CHECK-SAME: (ptr addrspace(3) noundef align 1 [[DATA1:%.*]], ptr addrspace(1) noundef align 1 [[SCRATCH:%.*]]) {
; CHECK-NEXT:    [[CAST_DATA:%.*]] = addrspacecast ptr addrspace(3) [[DATA1]] to ptr addrspace(4)
; CHECK-NEXT:    [[CAST_DATA1:%.*]] = addrspacecast ptr addrspace(1) [[SCRATCH]] to ptr addrspace(4)
; CHECK-NEXT:    call void @_Z73__devicelib_default_work_group_private_sort_close_ascending_p1i8_u32_p1i8PU3AS4cjS0_(ptr addrspace(4) [[CAST_DATA]], i32 4, ptr addrspace(4) [[CAST_DATA1]])
; CHECK-NEXT:    ret void
  tail call void @__devicelib_default_work_group_private_sort_close_ascending_p3i8_u32_p1i8(ptr addrspace(3) %data1, i32 4, ptr addrspace(1) %scratch)
  ret void
}

; key-only private spread sort
define dso_local void @sort2(ptr addrspace(1) noundef align 1 %data1, ptr addrspace(3) noundef align 1 %scratch){
; CHECK-LABEL: define dso_local void @sort2
; CHECK-SAME: (ptr addrspace(1) noundef align 1 [[DATA1:%.*]], ptr addrspace(3) noundef align 1 [[SCRATCH:%.*]]) {
; CHECK-NEXT:    [[CAST_DATA:%.*]] = addrspacecast ptr addrspace(1) [[DATA1]] to ptr addrspace(4)
; CHECK-NEXT:    [[CAST_DATA1:%.*]] = addrspacecast ptr addrspace(3) [[SCRATCH]] to ptr addrspace(4)
; CHECK-NEXT:    call void @_Z75__devicelib_default_work_group_private_sort_spread_ascending_p1i16_u32_p1i8PU3AS4sjPU3AS4c(ptr addrspace(4) [[CAST_DATA]], i32 4, ptr addrspace(4) [[CAST_DATA1]])
; CHECK-NEXT:    ret void
  tail call void @__devicelib_default_work_group_private_sort_spread_ascending_p1i16_u32_p3i8(ptr addrspace(1) %data1, i32 4, ptr addrspace(3) %scratch)
  ret void
}

; key-only joint sort
define dso_local void @sort3(ptr addrspace(3) noundef align 1 %data1, ptr addrspace(3) noundef align 1 %scratch){
; CHECK-LABEL: define dso_local void @sort3
; CHECK-SAME: (ptr addrspace(3) noundef align 1 [[DATA1:%.*]], ptr addrspace(3) noundef align 1 [[SCRATCH:%.*]]) {
; CHECK-NEXT:    [[CAST_DATA:%.*]] = addrspacecast ptr addrspace(3) [[DATA1]] to ptr addrspace(4)
; CHECK-NEXT:    [[CAST_DATA1:%.*]] = addrspacecast ptr addrspace(3) [[SCRATCH]] to ptr addrspace(4)
; CHECK-NEXT:    call void @_Z66__devicelib_default_work_group_joint_sort_ascending_p1f32_u32_p1i8PU3AS4fjPU3AS4c(ptr addrspace(4) [[CAST_DATA]], i32 4, ptr addrspace(4) [[CAST_DATA1]])
; CHECK-NEXT:    ret void
  tail call void @__devicelib_default_work_group_joint_sort_ascending_p3f32_u32_p3i8(ptr addrspace(3) %data1, i32 4, ptr addrspace(3) %scratch)
  ret void
}

; key-value private close sort scalar version
define dso_local void @sort4(ptr addrspace(3) noundef align 1 %data1, ptr addrspace(3) noundef align 1 %data2, ptr addrspace(1) noundef align 1 %scratch){
; CHECK-LABEL: define dso_local void @sort4
; CHECK-SAME: (ptr addrspace(3) noundef align 1 [[DATA1:%.*]], ptr addrspace(3) noundef align 1 [[DATA2:%.*]], ptr addrspace(1) noundef align 1 [[SCRATCH:%.*]]) {
; CHECK-NEXT:    [[CAST_DATA:%.*]] = addrspacecast ptr addrspace(3) [[DATA1]] to ptr addrspace(4)
; CHECK-NEXT:    [[CAST_DATA1:%.*]] = addrspacecast ptr addrspace(3) [[DATA2]] to ptr addrspace(4)
; CHECK-NEXT:    [[CAST_DATA2:%.*]] = addrspacecast ptr addrspace(1) [[SCRATCH]] to ptr addrspace(4)
; CHECK-NEXT:    call void @_Z80__devicelib_default_work_group_private_sort_close_ascending_p1i16_p1f16_u32_p1i8PU3AS4sPU3AS4DhjPU3AS4c(ptr addrspace(4) [[CAST_DATA]], ptr addrspace(4) [[CAST_DATA1]], i32 4, ptr addrspace(4) [[CAST_DATA2]])
; CHECK-NEXT:    ret void
  tail call void @__devicelib_default_work_group_private_sort_close_ascending_p3i16_p3f16_u32_p1i8(ptr addrspace(3) %data1, ptr addrspace(3) %data2, i32 4, ptr addrspace(1) %scratch)
  ret void
}

; key-value private spread sort scalar version
define dso_local void @sort5(ptr addrspace(3) noundef align 1 %data1, ptr addrspace(3) noundef align 1 %data2, ptr addrspace(1) noundef align 1 %scratch){
; CHECK-LABEL: define dso_local void @sort5
; CHECK-SAME: (ptr addrspace(3) noundef align 1 [[DATA1:%.*]], ptr addrspace(3) noundef align 1 [[DATA2:%.*]], ptr addrspace(1) noundef align 1 [[SCRATCH:%.*]]) {
; CHECK-NEXT:    [[CAST_DATA:%.*]] = addrspacecast ptr addrspace(3) [[DATA1]] to ptr addrspace(4)
; CHECK-NEXT:    [[CAST_DATA1:%.*]] = addrspacecast ptr addrspace(3) [[DATA2]] to ptr addrspace(4)
; CHECK-NEXT:    [[CAST_DATA2:%.*]] = addrspacecast ptr addrspace(1) [[SCRATCH]] to ptr addrspace(4)
; CHECK-NEXT:    call void @_Z80__devicelib_default_work_group_private_sort_spread_ascending_p1f64_p1i8_u32_p1i8PU3AS4dPU3AS4cjS2_(ptr addrspace(4) [[CAST_DATA]], ptr addrspace(4) [[CAST_DATA1]], i32 4, ptr addrspace(4) [[CAST_DATA2]])
; CHECK-NEXT:    ret void
  tail call void @__devicelib_default_work_group_private_sort_spread_ascending_p3f64_p3i8_u32_p1i8(ptr addrspace(3) %data1, ptr addrspace(3) %data2, i32 4, ptr addrspace(1) %scratch)
  ret void
}

; key-value joint sort scalar version
define dso_local void @sort6(ptr addrspace(1) noundef align 1 %data1, ptr addrspace(1) noundef align 1 %data2, ptr addrspace(3) noundef align 1 %scratch){
; CHECK-LABEL: define dso_local void @sort6
; CHECK-SAME: (ptr addrspace(1) noundef align 1 [[DATA1:%.*]], ptr addrspace(1) noundef align 1 [[DATA2:%.*]], ptr addrspace(3) noundef align 1 [[SCRATCH:%.*]]) {
; CHECK-NEXT:    [[CAST_DATA:%.*]] = addrspacecast ptr addrspace(1) [[DATA1]] to ptr addrspace(4)
; CHECK-NEXT:    [[CAST_DATA1:%.*]] = addrspacecast ptr addrspace(1) [[DATA2]] to ptr addrspace(4)
; CHECK-NEXT:    [[CAST_DATA2:%.*]] = addrspacecast ptr addrspace(3) [[SCRATCH]] to ptr addrspace(4)
; CHECK-NEXT:    call void @_Z72__devicelib_default_work_group_joint_sort_ascending_p1u32_p1i32_u32_p1i8PU3AS4jPU3AS4ijPU3AS4c(ptr addrspace(4) [[CAST_DATA]], ptr addrspace(4) [[CAST_DATA1]], i32 4, ptr addrspace(4) [[CAST_DATA2]])
; CHECK-NEXT:    ret void
  tail call void @__devicelib_default_work_group_joint_sort_ascending_p1u32_p1i32_u32_p3i8(ptr addrspace(1) %data1, ptr addrspace(1) %data2, i32 4, ptr addrspace(3) %scratch)
  ret void
}

; sub group sort (just check mangle)
define dso_local i32 @sort7(i32 %data1){
; CHECK-LABEL: define dso_local i32 @sort7
; CHECK-SAME: (i32 [[DATA1:%.*]]) {
; CHECK-NEXT:    [[TMP1:%.*]] = call i32 @_Z56__devicelib_default_sub_group_private_sort_ascending_i32i(i32 [[DATA1]])
; CHECK-NEXT:    ret i32 [[TMP1]]
;
  %call1 = call i32 @__devicelib_default_sub_group_private_sort_ascending_i32(i32 %data1)
  ret i32 %call1
}

declare void @__devicelib_default_work_group_private_sort_close_ascending_p3i8_u32_p1i8(ptr addrspace(3), i32, ptr addrspace(1))

declare void @__devicelib_default_work_group_private_sort_spread_ascending_p1i16_u32_p3i8(ptr addrspace(1), i32, ptr addrspace(3))

declare void @__devicelib_default_work_group_joint_sort_ascending_p3f32_u32_p3i8(ptr addrspace(3), i32, ptr addrspace(3))

declare void @__devicelib_default_work_group_private_sort_close_ascending_p3i16_p3f16_u32_p1i8(ptr addrspace(3), ptr addrspace(3), i32, ptr addrspace(1))

declare void @__devicelib_default_work_group_private_sort_spread_ascending_p3f64_p3i8_u32_p1i8(ptr addrspace(3), ptr addrspace(3),  i32, ptr addrspace(1))

declare void @__devicelib_default_work_group_joint_sort_ascending_p1u32_p1i32_u32_p3i8(ptr addrspace(1), ptr addrspace(1), i32, ptr addrspace(3))

declare i32 @__devicelib_default_sub_group_private_sort_ascending_i32(i32)

; DEBUGIFY-NOT: WARNING
