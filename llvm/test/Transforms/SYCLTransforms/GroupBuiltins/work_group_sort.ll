; RUN: opt -passes=sycl-kernel-group-builtin -S < %s | FileCheck %s

; key-only private close sort scalar version
define dso_local void @sort1(ptr addrspace(1) noundef align 1 %data1, ptr addrspace(1) noundef align 1 %scratch){
; CHECK:         [[LOCALID_0:%.*]] = call i64 @_Z12get_local_idj(i32 0)
; CHECK-NEXT:    [[LOCALID_1:%.*]] = call i64 @_Z12get_local_idj(i32 1)
; CHECK-NEXT:    [[LOCALID_2:%.*]] = call i64 @_Z12get_local_idj(i32 2)
; CHECK-NEXT:    [[LOCALSIZE_1:%.*]] = call i64 @_Z14get_local_sizej(i32 1)
; CHECK-NEXT:    [[LOCALSIZE_0:%.*]] = call i64 @_Z14get_local_sizej(i32 0)
; CHECK-NEXT:    [[LLID_P0:%.*]] = mul nuw nsw  i64 [[LOCALID_2]], [[LOCALSIZE_1]]
; CHECK-NEXT:    [[LLID_P1:%.*]] = add nuw nsw  i64 [[LLID_P0]], [[LOCALID_1]]
; CHECK-NEXT:    [[LLID_P2:%.*]] = mul nuw nsw  i64 [[LLID_P1]], [[LOCALSIZE_0]]
; CHECK-NEXT:    [[LLID_RES:%.*]] = add nuw nsw  i64 [[LLID_P2]], [[LOCALID_0]]
; CHECK-NEXT:    [[LOCALSIZE_2:%.*]] = call i64 @_Z14get_local_sizej(i32 2)
; CHECK-NEXT:    [[LOCALSIZE_11:%.*]] = call i64 @_Z14get_local_sizej(i32 1)
; CHECK-NEXT:    [[LOCALSIZE_02:%.*]] = call i64 @_Z14get_local_sizej(i32 0)
; CHECK-NEXT:    [[LLSIZE_P0:%.*]] = mul nuw nsw  i64 [[LOCALSIZE_2]], [[LOCALSIZE_11]]
; CHECK-NEXT:    [[LLSIZE_P1:%.*]] = mul nuw nsw  i64 [[LLSIZE_P0]], [[LOCALSIZE_02]]
; CHECK-NEXT:    call void @_Z48__devicelib_default_work_group_private_sort_copyPU3AS4cjS0_llb(ptr addrspace(4) [[NEW_DATA:%.*]], i32 4, ptr addrspace(4) [[NEW_SCRATCH:%.*]], i64 [[LLID_RES]], i64 [[LLSIZE_P1]], i1 true)
; CHECK-NEXT:    call void @_Z18work_group_barrierj(i32 1)
; CHECK-NEXT:    [[LLSIZE32:%.*]] = trunc i64 [[LLSIZE_P1]] to i32
; CHECK-NEXT:    [[SORT_SIZE:%.*]] = mul nuw nsw  i32 4, [[LLSIZE32]]
; CHECK-NEXT:    tail call void @_Z73__devicelib_default_work_group_private_sort_close_ascending_p1i8_u32_p1i8PU3AS4cjS0_(ptr addrspace(4) [[NEW_DATA]], i32 [[SORT_SIZE]], ptr addrspace(4) [[NEW_SCRATCH]])
; CHECK-NEXT:    call void @dummy_barrier.()
; CHECK-NEXT:    call void @_Z48__devicelib_default_work_group_private_sort_copyPU3AS4cjS0_llb(ptr addrspace(4) [[NEW_DATA]], i32 4, ptr addrspace(4) [[NEW_SCRATCH]], i64 [[LLID_RES]], i64 [[LLSIZE_P1]], i1 false)
  %new.scratch = addrspacecast ptr addrspace(1) %scratch to ptr addrspace(4)
  %new.data = addrspacecast ptr addrspace(1) %data1 to ptr addrspace(4)
  tail call void @_Z73__devicelib_default_work_group_private_sort_close_ascending_p1i8_u32_p1i8PU3AS4cjS0_(ptr addrspace(4) %new.data, i32 4, ptr addrspace(4) %new.scratch)

  ret void
}

; key-only private spread sort scalar version
define dso_local void @sort2(ptr addrspace(1) noundef align 1 %data1, ptr addrspace(1) noundef align 1 %scratch){
; CHECK:         [[LOCALID_0:%.*]] = call i64 @_Z12get_local_idj(i32 0)
; CHECK-NEXT:    [[LOCALID_1:%.*]] = call i64 @_Z12get_local_idj(i32 1)
; CHECK-NEXT:    [[LOCALID_2:%.*]] = call i64 @_Z12get_local_idj(i32 2)
; CHECK-NEXT:    [[LOCALSIZE_1:%.*]] = call i64 @_Z14get_local_sizej(i32 1)
; CHECK-NEXT:    [[LOCALSIZE_0:%.*]] = call i64 @_Z14get_local_sizej(i32 0)
; CHECK-NEXT:    [[LLID_P0:%.*]] = mul nuw nsw  i64 [[LOCALID_2]], [[LOCALSIZE_1]]
; CHECK-NEXT:    [[LLID_P1:%.*]] = add nuw nsw  i64 [[LLID_P0]], [[LOCALID_1]]
; CHECK-NEXT:    [[LLID_P2:%.*]] = mul nuw nsw  i64 [[LLID_P1]], [[LOCALSIZE_0]]
; CHECK-NEXT:    [[LLID_RES:%.*]] = add nuw nsw  i64 [[LLID_P2]], [[LOCALID_0]]
; CHECK-NEXT:    [[LOCALSIZE_2:%.*]] = call i64 @_Z14get_local_sizej(i32 2)
; CHECK-NEXT:    [[LOCALSIZE_11:%.*]] = call i64 @_Z14get_local_sizej(i32 1)
; CHECK-NEXT:    [[LOCALSIZE_02:%.*]] = call i64 @_Z14get_local_sizej(i32 0)
; CHECK-NEXT:    [[LLSIZE_P0:%.*]] = mul nuw nsw  i64 [[LOCALSIZE_2]], [[LOCALSIZE_11]]
; CHECK-NEXT:    [[LLSIZE_P1:%.*]] = mul nuw nsw  i64 [[LLSIZE_P0]], [[LOCALSIZE_02]]
; CHECK-NEXT:    call void @_Z48__devicelib_default_work_group_private_sort_copyPU3AS4sjPU3AS4cllb(ptr addrspace(4) [[NEW_DATA:%.*]], i32 4, ptr addrspace(4) [[NEW_SCRATCH:%.*]], i64 [[LLID_RES]], i64 [[LLSIZE_P1]], i1 true)
; CHECK-NEXT:    call void @_Z18work_group_barrierj(i32 1)
; CHECK-NEXT:    [[LLSIZE32:%.*]] = trunc i64 [[LLSIZE_P1]] to i32
; CHECK-NEXT:    [[SORT_SIZE:%.*]] = mul nuw nsw  i32 4, [[LLSIZE32]]
; CHECK-NEXT:    tail call void @_Z75__devicelib_default_work_group_private_sort_spread_ascending_p1i16_u32_p1i8PU3AS4sjPU3AS4c(ptr addrspace(4) [[NEW_DATA]], i32 [[SORT_SIZE]], ptr addrspace(4) [[NEW_SCRATCH]])
; CHECK-NEXT:    call void @dummy_barrier.()
; CHECK-NEXT:    call void @_Z55__devicelib_default_work_group_private_spread_sort_copyPU3AS4sjPU3AS4cllb(ptr addrspace(4) [[NEW_DATA]], i32 4, ptr addrspace(4) [[NEW_SCRATCH]], i64 [[LLID_RES]], i64 [[LLSIZE_P1]], i1 false)

  %new.scratch = addrspacecast ptr addrspace(1) %scratch to ptr addrspace(4)
  %new.data = addrspacecast ptr addrspace(1) %data1 to ptr addrspace(4)
  tail call void @_Z75__devicelib_default_work_group_private_sort_spread_ascending_p1i16_u32_p1i8PU3AS4sjPU3AS4c(ptr addrspace(4) %new.data, i32 4, ptr addrspace(4) %new.scratch)
  ret void
}

; key-only joint sort scalar version
define dso_local void @sort3(ptr addrspace(1) noundef align 1 %data1, ptr addrspace(1) noundef align 1 %scratch){
; CHECK:         call void @_Z18work_group_barrierj(i32 1)
; CHECK-NEXT:    tail call void @_Z66__devicelib_default_work_group_joint_sort_ascending_p1f32_u32_p1i8PU3AS4fjPU3AS4c(ptr addrspace(4) [[NEW_DATA:%.*]], i32 4, ptr addrspace(4) [[NEW_SCRATCH:%.*]])
; CHECK-NEXT:    call void @dummy_barrier.()
  %new.scratch = addrspacecast ptr addrspace(1) %scratch to ptr addrspace(4)
  %new.data = addrspacecast ptr addrspace(1) %data1 to ptr addrspace(4)
  tail call void @_Z66__devicelib_default_work_group_joint_sort_ascending_p1f32_u32_p1i8PU3AS4fjPU3AS4c(ptr addrspace(4) %new.data, i32 4, ptr addrspace(4) %new.scratch)
  ret void
}

; key-value private close sort scalar version
define dso_local void @sort4(ptr addrspace(1) noundef align 1 %data1, ptr addrspace(1) noundef align 1 %data2, ptr addrspace(1) noundef align 1 %scratch){
; CHECK:         [[LOCALID_0:%.*]] = call i64 @_Z12get_local_idj(i32 0)
; CHECK-NEXT:    [[LOCALID_1:%.*]] = call i64 @_Z12get_local_idj(i32 1)
; CHECK-NEXT:    [[LOCALID_2:%.*]] = call i64 @_Z12get_local_idj(i32 2)
; CHECK-NEXT:    [[LOCALSIZE_1:%.*]] = call i64 @_Z14get_local_sizej(i32 1)
; CHECK-NEXT:    [[LOCALSIZE_0:%.*]] = call i64 @_Z14get_local_sizej(i32 0)
; CHECK-NEXT:    [[LLID_P0:%.*]] = mul nuw nsw  i64 [[LOCALID_2]], [[LOCALSIZE_1]]
; CHECK-NEXT:    [[LLID_P1:%.*]] = add nuw nsw  i64 [[LLID_P0]], [[LOCALID_1]]
; CHECK-NEXT:    [[LLID_P2:%.*]] = mul nuw nsw  i64 [[LLID_P1]], [[LOCALSIZE_0]]
; CHECK-NEXT:    [[LLID_RES:%.*]] = add nuw nsw  i64 [[LLID_P2]], [[LOCALID_0]]
; CHECK-NEXT:    [[LOCALSIZE_2:%.*]] = call i64 @_Z14get_local_sizej(i32 2)
; CHECK-NEXT:    [[LOCALSIZE_11:%.*]] = call i64 @_Z14get_local_sizej(i32 1)
; CHECK-NEXT:    [[LOCALSIZE_02:%.*]] = call i64 @_Z14get_local_sizej(i32 0)
; CHECK-NEXT:    [[LLSIZE_P0:%.*]] = mul nuw nsw  i64 [[LOCALSIZE_2]], [[LOCALSIZE_11]]
; CHECK-NEXT:    [[LLSIZE_P1:%.*]] = mul nuw nsw  i64 [[LLSIZE_P0]], [[LOCALSIZE_02]]
; CHECK-NEXT:    call void @_Z48__devicelib_default_work_group_private_sort_copyPU3AS4sPU3AS4DhjPU3AS4cllb(ptr addrspace(4) [[NEW_DATA1:%.*]], ptr addrspace(4) [[NEW_DATA2:%.*]], i32 4, ptr addrspace(4) [[NEW_SCRATCH:%.*]], i64 [[LLID_RES]], i64 [[LLSIZE_P1]], i1 true)
; CHECK-NEXT:    call void @_Z18work_group_barrierj(i32 1)
; CHECK-NEXT:    [[LLSIZE32:%.*]] = trunc i64 [[LLSIZE_P1]] to i32
; CHECK-NEXT:    [[SORT_SIZE:%.*]] = mul nuw nsw  i32 4, [[LLSIZE32]]
; CHECK-NEXT:    tail call void @_Z80__devicelib_default_work_group_private_sort_close_ascending_p1i16_p1f16_u32_p1i8PU3AS4sPU3AS4DhjPU3AS4c(ptr addrspace(4) [[NEW_DATA1]], ptr addrspace(4) [[NEW_DATA2]], i32 [[SORT_SIZE]], ptr addrspace(4) [[NEW_SCRATCH]])
; CHECK-NEXT:    call void @dummy_barrier.()
; CHECK-NEXT:    call void @_Z48__devicelib_default_work_group_private_sort_copyPU3AS4sPU3AS4DhjPU3AS4cllb(ptr addrspace(4) [[NEW_DATA1]], ptr addrspace(4) [[NEW_DATA2]], i32 4, ptr addrspace(4) [[NEW_SCRATCH]], i64 [[LLID_RES]], i64 [[LLSIZE_P1]], i1 false)
  %new.scratch = addrspacecast ptr addrspace(1) %scratch to ptr addrspace(4)
  %new.data1 = addrspacecast ptr addrspace(1) %data1 to ptr addrspace(4)
  %new.data2 = addrspacecast ptr addrspace(1) %data2 to ptr addrspace(4)
  tail call void @_Z80__devicelib_default_work_group_private_sort_close_ascending_p1i16_p1f16_u32_p1i8PU3AS4sPU3AS4DhjPU3AS4c(ptr addrspace(4) %new.data1, ptr addrspace(4) %new.data2, i32 4, ptr addrspace(4) %new.scratch)
  ret void
}

;; key-value private spread sort scalar version
define dso_local void @sort5(ptr addrspace(1) noundef align 1 %data1, ptr addrspace(1) noundef align 1 %data2, ptr addrspace(1) noundef align 1 %scratch){
; CHECK:         [[LOCALID_0:%.*]] = call i64 @_Z12get_local_idj(i32 0)
; CHECK-NEXT:    [[LOCALID_1:%.*]] = call i64 @_Z12get_local_idj(i32 1)
; CHECK-NEXT:    [[LOCALID_2:%.*]] = call i64 @_Z12get_local_idj(i32 2)
; CHECK-NEXT:    [[LOCALSIZE_1:%.*]] = call i64 @_Z14get_local_sizej(i32 1)
; CHECK-NEXT:    [[LOCALSIZE_0:%.*]] = call i64 @_Z14get_local_sizej(i32 0)
; CHECK-NEXT:    [[LLID_P0:%.*]] = mul nuw nsw  i64 [[LOCALID_2]], [[LOCALSIZE_1]]
; CHECK-NEXT:    [[LLID_P1:%.*]] = add nuw nsw  i64 [[LLID_P0]], [[LOCALID_1]]
; CHECK-NEXT:    [[LLID_P2:%.*]] = mul nuw nsw  i64 [[LLID_P1]], [[LOCALSIZE_0]]
; CHECK-NEXT:    [[LLID_RES:%.*]] = add nuw nsw  i64 [[LLID_P2]], [[LOCALID_0]]
; CHECK-NEXT:    [[LOCALSIZE_2:%.*]] = call i64 @_Z14get_local_sizej(i32 2)
; CHECK-NEXT:    [[LOCALSIZE_11:%.*]] = call i64 @_Z14get_local_sizej(i32 1)
; CHECK-NEXT:    [[LOCALSIZE_02:%.*]] = call i64 @_Z14get_local_sizej(i32 0)
; CHECK-NEXT:    [[LLSIZE_P0:%.*]] = mul nuw nsw  i64 [[LOCALSIZE_2]], [[LOCALSIZE_11]]
; CHECK-NEXT:    [[LLSIZE_P1:%.*]] = mul nuw nsw  i64 [[LLSIZE_P0]], [[LOCALSIZE_02]]
; CHECK-NEXT:    call void @_Z48__devicelib_default_work_group_private_sort_copyPU3AS4dPU3AS4cjS2_llb(ptr addrspace(4) [[NEW_DATA1:%.*]], ptr addrspace(4) [[NEW_DATA2:%.*]], i32 4, ptr addrspace(4) [[NEW_SCRATCH:%.*]], i64 [[LLID_RES]], i64 [[LLSIZE_P1]], i1 true)
; CHECK-NEXT:    call void @_Z18work_group_barrierj(i32 1)
; CHECK-NEXT:    [[LLSIZE32:%.*]] = trunc i64 [[LLSIZE_P1]] to i32
; CHECK-NEXT:    [[SORT_SIZE:%.*]] = mul nuw nsw  i32 4, [[LLSIZE32]]
; CHECK-NEXT:    tail call void @_Z80__devicelib_default_work_group_private_sort_spread_ascending_p1f64_p1i8_u32_p1i8PU3AS4dPU3AS4cjS2_(ptr addrspace(4) [[NEW_DATA1]], ptr addrspace(4) [[NEW_DATA2]], i32 [[SORT_SIZE]], ptr addrspace(4) [[NEW_SCRATCH]])
; CHECK-NEXT:    call void @dummy_barrier.()
; CHECK-NEXT:    call void @_Z55__devicelib_default_work_group_private_spread_sort_copyPU3AS4dPU3AS4cjS2_llb(ptr addrspace(4) [[NEW_DATA1]], ptr addrspace(4) [[NEW_DATA2]], i32 4, ptr addrspace(4) [[NEW_SCRATCH]], i64 [[LLID_RES]], i64 [[LLSIZE_P1]], i1 false)
  %new.scratch = addrspacecast ptr addrspace(1) %scratch to ptr addrspace(4)
  %new.data1 = addrspacecast ptr addrspace(1) %data1 to ptr addrspace(4)
  %new.data2 = addrspacecast ptr addrspace(1) %data2 to ptr addrspace(4)
  tail call void @_Z80__devicelib_default_work_group_private_sort_spread_ascending_p1f64_p1i8_u32_p1i8PU3AS4dPU3AS4cjS2_(ptr addrspace(4) %new.data1, ptr addrspace(4) %new.data2, i32 4, ptr addrspace(4) %new.scratch)
  ret void
}
;
;; key-value joint sort scalar version
define dso_local void @sort6(ptr addrspace(1) noundef align 1 %data1, ptr addrspace(1) noundef align 1 %data2, ptr addrspace(1) noundef align 1 %scratch){
; CHECK:         call void @_Z18work_group_barrierj(i32 1)
; CHECK-NEXT:    tail call void @_Z72__devicelib_default_work_group_joint_sort_ascending_p3u32_p3i32_u32_p1i8PU3AS4jPU3AS4ijPU3AS4c(ptr addrspace(4) [[NEW_DATA1:%.*]], ptr addrspace(4) [[NEW_DATA2:%.*]], i32 4, ptr addrspace(4) [[NEW_SCRATCH:%.*]])
; CHECK-NEXT:    call void @dummy_barrier.()
  %new.scratch = addrspacecast ptr addrspace(1) %scratch to ptr addrspace(4)
  %new.data1 = addrspacecast ptr addrspace(1) %data1 to ptr addrspace(4)
  %new.data2 = addrspacecast ptr addrspace(1) %data2 to ptr addrspace(4)
  tail call void @_Z72__devicelib_default_work_group_joint_sort_ascending_p3u32_p3i32_u32_p1i8PU3AS4jPU3AS4ijPU3AS4c(ptr addrspace(4) %new.data1, ptr addrspace(4) %new.data2, i32 4, ptr addrspace(4) %new.scratch)
  ret void
}

; key-only private close sort vector version
define dso_local void @sort7(ptr addrspace(1) noundef align 1 %data1, ptr addrspace(1) noundef align 1 %scratch){
; CHECK:         [[LOCALID_0:%.*]] = call i64 @_Z12get_local_idj(i32 0)
; CHECK-NEXT:    [[LOCALID_1:%.*]] = call i64 @_Z12get_local_idj(i32 1)
; CHECK-NEXT:    [[LOCALID_2:%.*]] = call i64 @_Z12get_local_idj(i32 2)
; CHECK-NEXT:    [[LOCALSIZE_1:%.*]] = call i64 @_Z14get_local_sizej(i32 1)
; CHECK-NEXT:    [[LOCALSIZE_0:%.*]] = call i64 @_Z14get_local_sizej(i32 0)
; CHECK-NEXT:    [[LLID_P0:%.*]] = mul nuw nsw  i64 [[LOCALID_2]], [[LOCALSIZE_1]]
; CHECK-NEXT:    [[LLID_P1:%.*]] = add nuw nsw  i64 [[LLID_P0]], [[LOCALID_1]]
; CHECK-NEXT:    [[LLID_P2:%.*]] = mul nuw nsw  i64 [[LLID_P1]], [[LOCALSIZE_0]]
; CHECK-NEXT:    [[LLID_RES:%.*]] = add nuw nsw  i64 [[LLID_P2]], [[LOCALID_0]]
; CHECK-NEXT:    [[LOCALSIZE_2:%.*]] = call i64 @_Z14get_local_sizej(i32 2)
; CHECK-NEXT:    [[LOCALSIZE_11:%.*]] = call i64 @_Z14get_local_sizej(i32 1)
; CHECK-NEXT:    [[LOCALSIZE_02:%.*]] = call i64 @_Z14get_local_sizej(i32 0)
; CHECK-NEXT:    [[LLSIZE_P0:%.*]] = mul nuw nsw  i64 [[LOCALSIZE_2]], [[LOCALSIZE_11]]
; CHECK-NEXT:    [[LLSIZE_P1:%.*]] = mul nuw nsw  i64 [[LLSIZE_P0]], [[LOCALSIZE_02]]
; CHECK-NEXT:    call void @_Z48__devicelib_default_work_group_private_sort_copyDv4_PU3AS4cjS0_llb(<4 x ptr addrspace(4)> [[VECTOR_DATA:%.*]], i32 4, ptr addrspace(4) [[NEW_SCRATCH:%.*]], i64 [[LLID_RES]], i64 [[LLSIZE_P1]], i1 true)
; CHECK-NEXT:    call void @_Z18work_group_barrierj(i32 1)
; CHECK-NEXT:    [[LLSIZE32:%.*]] = trunc i64 [[LLSIZE_P1]] to i32
; CHECK-NEXT:    [[SORT_SIZE:%.*]] = mul nuw nsw  i32 4, [[LLSIZE32]]
; CHECK-NEXT:    tail call void @_Z73__devicelib_default_work_group_private_sort_close_ascending_p1i8_u32_p1i8Dv4_PU3AS4cjS0_Dv4_j(<4 x ptr addrspace(4)> [[VECTOR_DATA]], i32 [[SORT_SIZE]], ptr addrspace(4) [[NEW_SCRATCH]], <4 x i32> <i32 -1, i32 -1, i32 -1, i32 -1>)
; CHECK-NEXT:    call void @dummy_barrier.()
; CHECK-NEXT:    call void @_Z48__devicelib_default_work_group_private_sort_copyDv4_PU3AS4cjS0_llb(<4 x ptr addrspace(4)> [[VECTOR_DATA]], i32 4, ptr addrspace(4) [[NEW_SCRATCH]], i64 [[LLID_RES]], i64 [[LLSIZE_P1]], i1 false)
  %broadcast.splatinsert = insertelement <4 x ptr addrspace(1)> poison, ptr addrspace(1) %data1, i64 0
  %broadcast.splat = shufflevector <4 x ptr addrspace(1)> %broadcast.splatinsert, <4 x ptr addrspace(1)> poison, <4 x i32> zeroinitializer
  %new.scratch = addrspacecast ptr addrspace(1) %scratch to ptr addrspace(4)
  %vector.data = addrspacecast <4 x ptr addrspace(1)> %broadcast.splat to <4 x ptr addrspace(4)>
  tail call void @_Z73__devicelib_default_work_group_private_sort_close_ascending_p1i8_u32_p1i8Dv4_PU3AS4cjS0_Dv4_j(<4 x ptr addrspace(4)> %vector.data, i32 4, ptr addrspace(4) %new.scratch, <4 x i32> <i32 -1, i32 -1, i32 -1, i32 -1>)
  ret void
}

; key-only private spread sort vector version
define dso_local void @sort8(ptr addrspace(1) noundef align 1 %data1, ptr addrspace(1) noundef align 1 %scratch){
; CHECK:         [[LOCALID_0:%.*]] = call i64 @_Z12get_local_idj(i32 0)
; CHECK-NEXT:    [[LOCALID_1:%.*]] = call i64 @_Z12get_local_idj(i32 1)
; CHECK-NEXT:    [[LOCALID_2:%.*]] = call i64 @_Z12get_local_idj(i32 2)
; CHECK-NEXT:    [[LOCALSIZE_1:%.*]] = call i64 @_Z14get_local_sizej(i32 1)
; CHECK-NEXT:    [[LOCALSIZE_0:%.*]] = call i64 @_Z14get_local_sizej(i32 0)
; CHECK-NEXT:    [[LLID_P0:%.*]] = mul nuw nsw  i64 [[LOCALID_2]], [[LOCALSIZE_1]]
; CHECK-NEXT:    [[LLID_P1:%.*]] = add nuw nsw  i64 [[LLID_P0]], [[LOCALID_1]]
; CHECK-NEXT:    [[LLID_P2:%.*]] = mul nuw nsw  i64 [[LLID_P1]], [[LOCALSIZE_0]]
; CHECK-NEXT:    [[LLID_RES:%.*]] = add nuw nsw  i64 [[LLID_P2]], [[LOCALID_0]]
; CHECK-NEXT:    [[LOCALSIZE_2:%.*]] = call i64 @_Z14get_local_sizej(i32 2)
; CHECK-NEXT:    [[LOCALSIZE_11:%.*]] = call i64 @_Z14get_local_sizej(i32 1)
; CHECK-NEXT:    [[LOCALSIZE_02:%.*]] = call i64 @_Z14get_local_sizej(i32 0)
; CHECK-NEXT:    [[LLSIZE_P0:%.*]] = mul nuw nsw  i64 [[LOCALSIZE_2]], [[LOCALSIZE_11]]
; CHECK-NEXT:    [[LLSIZE_P1:%.*]] = mul nuw nsw  i64 [[LLSIZE_P0]], [[LOCALSIZE_02]]
; CHECK-NEXT:    call void @_Z48__devicelib_default_work_group_private_sort_copyDv8_PU3AS4ljPU3AS4cllb(<8 x ptr addrspace(4)> [[VECTOR_DATA]], i32 4, ptr addrspace(4) [[NEW_SCRATCH]], i64 [[LLID_RES]], i64 [[LLSIZE_P1]], i1 true)
; CHECK-NEXT:    call void @_Z18work_group_barrierj(i32 1)
; CHECK-NEXT:    [[LLSIZE32:%.*]] = trunc i64 [[LLSIZE_P1]] to i32
; CHECK-NEXT:    [[SORT_SIZE:%.*]] = mul nuw nsw  i32 4, [[LLSIZE32]]
; CHECK-NEXT:    tail call void @_Z75__devicelib_default_work_group_private_sort_spread_ascending_p1i64_u32_p1i8Dv8_PU3AS4ljPU3AS4cDv8_j(<8 x ptr addrspace(4)> [[VECTOR_DATA]], i32 [[SORT_SIZE]], ptr addrspace(4) [[NEW_SCRATCH]], <8 x i32> <i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1>)
; CHECK-NEXT:    call void @dummy_barrier.()
; CHECK-NEXT:    call void @_Z55__devicelib_default_work_group_private_spread_sort_copyDv8_PU3AS4ljPU3AS4cllb(<8 x ptr addrspace(4)> [[VECTOR_DATA]], i32 4, ptr addrspace(4) [[NEW_SCRATCH]], i64 [[LLID_RES]], i64 [[LLSIZE_P1]], i1 false)
  %broadcast.splatinsert = insertelement <8 x ptr addrspace(1)> poison, ptr addrspace(1) %data1, i64 0
  %broadcast.splat = shufflevector <8 x ptr addrspace(1)> %broadcast.splatinsert, <8 x ptr addrspace(1)> poison, <8 x i32> zeroinitializer
  %new.scratch = addrspacecast ptr addrspace(1) %scratch to ptr addrspace(4)
  %vector.data = addrspacecast <8 x ptr addrspace(1)> %broadcast.splat to <8 x ptr addrspace(4)>
  tail call void @_Z75__devicelib_default_work_group_private_sort_spread_ascending_p1i64_u32_p1i8Dv8_PU3AS4ljPU3AS4cDv8_j(<8 x ptr addrspace(4)> %vector.data, i32 4, ptr addrspace(4) %new.scratch, <8 x i32> <i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1>)
  ret void
}

; key-only joint sort vector version
define dso_local void @sort9(ptr addrspace(1) noundef align 1 %data1, ptr addrspace(1) noundef align 1 %scratch){
; CHECK:         call void @_Z18work_group_barrierj(i32 1)
; CHECK-NEXT:    tail call void @_Z67__devicelib_default_work_group_joint_sort_descending_p1f64_u32_p1i8Dv16_PU3AS4djPU3AS4cDv16_j(<16 x ptr addrspace(4)> [[VECTOR_DATA:%.*]], i32 4, ptr addrspace(4) [[NEW_SCRATCH:%.*]], <16 x i32> <i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1>)
; CHECK-NEXT:    call void @dummy_barrier.()
  %broadcast.splatinsert = insertelement <16 x ptr addrspace(1)> poison, ptr addrspace(1) %data1, i64 0
  %broadcast.splat = shufflevector <16 x ptr addrspace(1)> %broadcast.splatinsert, <16 x ptr addrspace(1)> poison, <16 x i32> zeroinitializer
  %new.scratch = addrspacecast ptr addrspace(1) %scratch to ptr addrspace(4)
  %vector.data = addrspacecast <16 x ptr addrspace(1)> %broadcast.splat to <16 x ptr addrspace(4)>
  tail call void @_Z67__devicelib_default_work_group_joint_sort_descending_p1f64_u32_p1i8Dv16_PU3AS4djPU3AS4cDv16_j(<16 x ptr addrspace(4)> %vector.data, i32 4, ptr addrspace(4) %new.scratch, <16 x i32> <i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1>)
  ret void
}

;; key-value private close sort vector version
define dso_local void @sort10(ptr addrspace(1) noundef align 1 %data1, ptr addrspace(1) noundef align 1 %data2, ptr addrspace(1) noundef align 1 %scratch){
; CHECK:         [[LOCALID_0:%.*]] = call i64 @_Z12get_local_idj(i32 0)
; CHECK-NEXT:    [[LOCALID_1:%.*]] = call i64 @_Z12get_local_idj(i32 1)
; CHECK-NEXT:    [[LOCALID_2:%.*]] = call i64 @_Z12get_local_idj(i32 2)
; CHECK-NEXT:    [[LOCALSIZE_1:%.*]] = call i64 @_Z14get_local_sizej(i32 1)
; CHECK-NEXT:    [[LOCALSIZE_0:%.*]] = call i64 @_Z14get_local_sizej(i32 0)
; CHECK-NEXT:    [[LLID_P0:%.*]] = mul nuw nsw  i64 [[LOCALID_2]], [[LOCALSIZE_1]]
; CHECK-NEXT:    [[LLID_P1:%.*]] = add nuw nsw  i64 [[LLID_P0]], [[LOCALID_1]]
; CHECK-NEXT:    [[LLID_P2:%.*]] = mul nuw nsw  i64 [[LLID_P1]], [[LOCALSIZE_0]]
; CHECK-NEXT:    [[LLID_RES:%.*]] = add nuw nsw  i64 [[LLID_P2]], [[LOCALID_0]]
; CHECK-NEXT:    [[LOCALSIZE_2:%.*]] = call i64 @_Z14get_local_sizej(i32 2)
; CHECK-NEXT:    [[LOCALSIZE_11:%.*]] = call i64 @_Z14get_local_sizej(i32 1)
; CHECK-NEXT:    [[LOCALSIZE_02:%.*]] = call i64 @_Z14get_local_sizej(i32 0)
; CHECK-NEXT:    [[LLSIZE_P0:%.*]] = mul nuw nsw  i64 [[LOCALSIZE_2]], [[LOCALSIZE_11]]
; CHECK-NEXT:    [[LLSIZE_P1:%.*]] = mul nuw nsw  i64 [[LLSIZE_P0]], [[LOCALSIZE_02]]
; CHECK-NEXT:    call void @_Z48__devicelib_default_work_group_private_sort_copyDv4_PU3AS4dDv4_PU3AS4cjS3_llb(<4 x ptr addrspace(4)> [[VECTOR_DATA1:%.*]], <4 x ptr addrspace(4)> [[VECTOR_DATA2:%.*]], i32 4, ptr addrspace(4) [[NEW_SCRATCH:%.*]], i64 [[LLID_RES]], i64 [[LLSIZE_P1]], i1 true)
; CHECK-NEXT:    call void @_Z18work_group_barrierj(i32 1)
; CHECK-NEXT:    [[LLSIZE32:%.*]] = trunc i64 [[LLSIZE_P1]] to i32
; CHECK-NEXT:    [[SORT_SIZE:%.*]] = mul nuw nsw  i32 4, [[LLSIZE32]]
; CHECK-NEXT:    tail call void @_Z79__devicelib_default_work_group_private_sort_close_ascending_p3f64_p3i8_u32_p1i8Dv4_PU3AS4dDv4_PU3AS4cjS3_Dv4_j(<4 x ptr addrspace(4)> [[VECTOR_DATA1]], <4 x ptr addrspace(4)> [[VECTOR_DATA2]], i32 [[SORT_SIZE]], ptr addrspace(4) [[NEW_SCRATCH]], <4 x i32> <i32 -1, i32 -1, i32 -1, i32 -1>)
; CHECK-NEXT:    call void @dummy_barrier.()
; CHECK-NEXT:    call void @_Z48__devicelib_default_work_group_private_sort_copyDv4_PU3AS4dDv4_PU3AS4cjS3_llb(<4 x ptr addrspace(4)> [[VECTOR_DATA1]], <4 x ptr addrspace(4)> [[VECTOR_DATA2]], i32 4, ptr addrspace(4) [[NEW_SCRATCH]], i64 [[LLID_RES]], i64 [[LLSIZE_P1]], i1 false)
  %broadcast.splatinsert1 = insertelement <4 x ptr addrspace(1)> poison, ptr addrspace(1) %data1, i64 0
  %broadcast.splat1 = shufflevector <4 x ptr addrspace(1)> %broadcast.splatinsert1, <4 x ptr addrspace(1)> poison, <4 x i32> zeroinitializer
  %vector.data1 = addrspacecast <4 x ptr addrspace(1)> %broadcast.splat1 to <4 x ptr addrspace(4)>

  %broadcast.splatinsert2 = insertelement <4 x ptr addrspace(1)> poison, ptr addrspace(1) %data2, i64 0
  %broadcast.splat2 = shufflevector <4 x ptr addrspace(1)> %broadcast.splatinsert2, <4 x ptr addrspace(1)> poison, <4 x i32> zeroinitializer
  %vector.data2 = addrspacecast <4 x ptr addrspace(1)> %broadcast.splat2 to <4 x ptr addrspace(4)>

  %new.scratch = addrspacecast ptr addrspace(1) %scratch to ptr addrspace(4)

  tail call void @_Z79__devicelib_default_work_group_private_sort_close_ascending_p3f64_p3i8_u32_p1i8Dv4_PU3AS4dDv4_PU3AS4cjS3_Dv4_j(<4 x ptr addrspace(4)> %vector.data1, <4 x ptr addrspace(4)> %vector.data2, i32 4, ptr addrspace(4) %new.scratch, <4 x i32> <i32 -1, i32 -1, i32 -1, i32 -1>)
  ret void
}
;
;; key-value private spread sort vector version
define dso_local void @sort11(ptr addrspace(1) noundef align 1 %data1, ptr addrspace(1) noundef align 1 %data2, ptr addrspace(1) noundef align 1 %scratch){
; CHECK:         [[LOCALID_0:%.*]] = call i64 @_Z12get_local_idj(i32 0)
; CHECK-NEXT:    [[LOCALID_1:%.*]] = call i64 @_Z12get_local_idj(i32 1)
; CHECK-NEXT:    [[LOCALID_2:%.*]] = call i64 @_Z12get_local_idj(i32 2)
; CHECK-NEXT:    [[LOCALSIZE_1:%.*]] = call i64 @_Z14get_local_sizej(i32 1)
; CHECK-NEXT:    [[LOCALSIZE_0:%.*]] = call i64 @_Z14get_local_sizej(i32 0)
; CHECK-NEXT:    [[LLID_P0:%.*]] = mul nuw nsw  i64 [[LOCALID_2]], [[LOCALSIZE_1]]
; CHECK-NEXT:    [[LLID_P1:%.*]] = add nuw nsw  i64 [[LLID_P0]], [[LOCALID_1]]
; CHECK-NEXT:    [[LLID_P2:%.*]] = mul nuw nsw  i64 [[LLID_P1]], [[LOCALSIZE_0]]
; CHECK-NEXT:    [[LLID_RES:%.*]] = add nuw nsw  i64 [[LLID_P2]], [[LOCALID_0]]
; CHECK-NEXT:    [[LOCALSIZE_2:%.*]] = call i64 @_Z14get_local_sizej(i32 2)
; CHECK-NEXT:    [[LOCALSIZE_11:%.*]] = call i64 @_Z14get_local_sizej(i32 1)
; CHECK-NEXT:    [[LOCALSIZE_02:%.*]] = call i64 @_Z14get_local_sizej(i32 0)
; CHECK-NEXT:    [[LLSIZE_P0:%.*]] = mul nuw nsw  i64 [[LOCALSIZE_2]], [[LOCALSIZE_11]]
; CHECK-NEXT:    [[LLSIZE_P1:%.*]] = mul nuw nsw  i64 [[LLSIZE_P0]], [[LOCALSIZE_02]]
; CHECK-NEXT:    call void @_Z48__devicelib_default_work_group_private_sort_copyDv8_PU3AS4mDv8_PU3AS4sjPU3AS4cllb(<4 x ptr addrspace(4)> [[VECTOR_DATA1:%.*]], <4 x ptr addrspace(4)> [[VECTOR_DATA2:%.*]], i32 4, ptr addrspace(4) [[NEW_SCRATCH:%.*]], i64 [[LLID_RES]], i64 [[LLSIZE_P1]], i1 true)
; CHECK-NEXT:    call void @_Z18work_group_barrierj(i32 1)
; CHECK-NEXT:    [[LLSIZE32:%.*]] = trunc i64 [[LLSIZE_P1]] to i32
; CHECK-NEXT:    [[SORT_SIZE:%.*]] = mul nuw nsw  i32 4, [[LLSIZE32]]
; CHECK-NEXT:    tail call void @_Z81__devicelib_default_work_group_private_sort_spread_ascending_p3u64_p3i16_u32_p1i8Dv8_PU3AS4mDv8_PU3AS4sjPU3AS4cDv8_j(<4 x ptr addrspace(4)> [[VECTOR_DATA1]], <4 x ptr addrspace(4)> [[VECTOR_DATA2]], i32 [[SORT_SIZE]], ptr addrspace(4) [[NEW_SCRATCH]], <8 x i32> <i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1>)
; CHECK-NEXT:    call void @dummy_barrier.()
; CHECK-NEXT:    call void @_Z55__devicelib_default_work_group_private_spread_sort_copyDv8_PU3AS4mDv8_PU3AS4sjPU3AS4cllb(<4 x ptr addrspace(4)> [[VECTOR_DATA1]], <4 x ptr addrspace(4)> [[VECTOR_DATA2]], i32 4, ptr addrspace(4) [[NEW_SCRATCH]], i64 [[LLID_RES]], i64 [[LLSIZE_P1]], i1 false)
  %broadcast.splatinsert1 = insertelement <4 x ptr addrspace(1)> poison, ptr addrspace(1) %data1, i64 0
  %broadcast.splat1 = shufflevector <4 x ptr addrspace(1)> %broadcast.splatinsert1, <4 x ptr addrspace(1)> poison, <4 x i32> zeroinitializer
  %vector.data1 = addrspacecast <4 x ptr addrspace(1)> %broadcast.splat1 to <4 x ptr addrspace(4)>

  %broadcast.splatinsert2 = insertelement <4 x ptr addrspace(1)> poison, ptr addrspace(1) %data2, i64 0
  %broadcast.splat2 = shufflevector <4 x ptr addrspace(1)> %broadcast.splatinsert2, <4 x ptr addrspace(1)> poison, <4 x i32> zeroinitializer
  %vector.data2 = addrspacecast <4 x ptr addrspace(1)> %broadcast.splat2 to <4 x ptr addrspace(4)>

  %new.scratch = addrspacecast ptr addrspace(1) %scratch to ptr addrspace(4)

  tail call void @_Z81__devicelib_default_work_group_private_sort_spread_ascending_p3u64_p3i16_u32_p1i8Dv8_PU3AS4mDv8_PU3AS4sjPU3AS4cDv8_j(<4 x ptr addrspace(4)> %vector.data1, <4 x ptr addrspace(4)> %vector.data2, i32 4, ptr addrspace(4) %new.scratch, <8 x i32> <i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1>)
  ret void
}
;
;; key-value joint sort vector version
define dso_local void @sort12(ptr addrspace(1) noundef align 1 %data1, ptr addrspace(1) noundef align 1 %data2, ptr addrspace(1) noundef align 1 %scratch){
; CHECK:         call void @_Z18work_group_barrierj(i32 1)
; CHECK-NEXT:    tail call void @_Z72__devicelib_default_work_group_joint_sort_ascending_p3u32_p3i32_u32_p1i8Dv16_PU3AS4jDv16_PU3AS4ijPU3AS4cDv16_j(<4 x ptr addrspace(4)> [[VECTOR_DATA1]], <4 x ptr addrspace(4)> [[VECTOR_DATA2]], i32 4, ptr addrspace(4) [[NEW_SCRATCH]], <16 x i32> <i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1>)
; CHECK-NEXT:    call void @dummy_barrier.()
  %broadcast.splatinsert1 = insertelement <4 x ptr addrspace(1)> poison, ptr addrspace(1) %data1, i64 0
  %broadcast.splat1 = shufflevector <4 x ptr addrspace(1)> %broadcast.splatinsert1, <4 x ptr addrspace(1)> poison, <4 x i32> zeroinitializer
  %vector.data1 = addrspacecast <4 x ptr addrspace(1)> %broadcast.splat1 to <4 x ptr addrspace(4)>

  %broadcast.splatinsert2 = insertelement <4 x ptr addrspace(1)> poison, ptr addrspace(1) %data2, i64 0
  %broadcast.splat2 = shufflevector <4 x ptr addrspace(1)> %broadcast.splatinsert2, <4 x ptr addrspace(1)> poison, <4 x i32> zeroinitializer
  %vector.data2 = addrspacecast <4 x ptr addrspace(1)> %broadcast.splat2 to <4 x ptr addrspace(4)>

  %new.scratch = addrspacecast ptr addrspace(1) %scratch to ptr addrspace(4)

  tail call void @_Z72__devicelib_default_work_group_joint_sort_ascending_p3u32_p3i32_u32_p1i8Dv16_PU3AS4jDv16_PU3AS4ijPU3AS4cDv16_j(<4 x ptr addrspace(4)> %vector.data1, <4 x ptr addrspace(4)> %vector.data2, i32 4, ptr addrspace(4) %new.scratch, <16 x i32> <i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1>)
  ret void
}

declare i64 @_Z12get_local_idj(i32)

declare i64 @_Z14get_local_sizej(i32 noundef)

declare void @_Z73__devicelib_default_work_group_private_sort_close_ascending_p1i8_u32_p1i8PU3AS4cjS0_(ptr addrspace(4), i32, ptr addrspace(4))

declare void @_Z75__devicelib_default_work_group_private_sort_spread_ascending_p1i16_u32_p1i8PU3AS4sjPU3AS4c(ptr addrspace(4), i32, ptr addrspace(4))

declare void @_Z66__devicelib_default_work_group_joint_sort_ascending_p1f32_u32_p1i8PU3AS4fjPU3AS4c(ptr addrspace(4), i32, ptr addrspace(4))

declare void @_Z73__devicelib_default_work_group_private_sort_close_ascending_p1i8_u32_p1i8Dv4_PU3AS4cjS0_Dv4_j(<4 x ptr addrspace(4)>, i32, ptr addrspace(4), <4 x i32>)

declare void @_Z75__devicelib_default_work_group_private_sort_spread_ascending_p1i64_u32_p1i8Dv8_PU3AS4ljPU3AS4cDv8_j(<8 x ptr addrspace(4)>, i32, ptr addrspace(4), <8 x i32>)

declare void @_Z67__devicelib_default_work_group_joint_sort_descending_p1f64_u32_p1i8Dv16_PU3AS4djPU3AS4cDv16_j(<16 x ptr addrspace(4)>, i32, ptr addrspace(4), <16 x i32>)

declare void @_Z80__devicelib_default_work_group_private_sort_close_ascending_p1i16_p1f16_u32_p1i8PU3AS4sPU3AS4DhjPU3AS4c(ptr addrspace(4), ptr addrspace(4), i32, ptr addrspace(4))

declare void @_Z80__devicelib_default_work_group_private_sort_spread_ascending_p1f64_p1i8_u32_p1i8PU3AS4dPU3AS4cjS2_(ptr addrspace(4), ptr addrspace(4),  i32, ptr addrspace(4))

declare void @_Z72__devicelib_default_work_group_joint_sort_ascending_p3u32_p3i32_u32_p1i8PU3AS4jPU3AS4ijPU3AS4c(ptr addrspace(4), ptr addrspace(4), i32, ptr addrspace(4))

declare void @_Z79__devicelib_default_work_group_private_sort_close_ascending_p3f64_p3i8_u32_p1i8Dv4_PU3AS4dDv4_PU3AS4cjS3_Dv4_j(<4 x ptr addrspace(4)>, <4 x ptr addrspace(4)>, i32, ptr addrspace(4), <4 x i32>)

declare void @_Z81__devicelib_default_work_group_private_sort_spread_ascending_p3u64_p3i16_u32_p1i8Dv8_PU3AS4mDv8_PU3AS4sjPU3AS4cDv8_j(<4 x ptr addrspace(4)>, <4 x ptr addrspace(4)>, i32, ptr addrspace(4), <8 x i32>)

declare void @_Z72__devicelib_default_work_group_joint_sort_ascending_p3u32_p3i32_u32_p1i8Dv16_PU3AS4jDv16_PU3AS4ijPU3AS4cDv16_j(<4 x ptr addrspace(4)>, <4 x ptr addrspace(4)>, i32, ptr addrspace(4), <16 x i32>)
