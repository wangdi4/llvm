; RUN: opt -passes=sycl-kernel-group-builtin -S < %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-group-builtin -S < %s | FileCheck %s

; Function Attrs: convergent norecurse nounwind
define dso_local void @test(i32 addrspace(1)* noundef align 4 %data, i32 addrspace(1)* noundef align 4 %result) {
entry:
  %call3 = tail call i64 @_Z13get_global_idj(i32 noundef 0)
  %conv4 = trunc i64 %call3 to i32
  %arrayidx = getelementptr inbounds i32, i32 addrspace(1)* %data, i64 %call3
  store i32 %conv4, i32 addrspace(1)* %arrayidx, align 4
; CHECK:         call void @_Z18work_group_barrierj(i32 1)
; CHECK-NEXT:    [[NUMSGROUPS_2:%.*]] = call i64 @_Z14get_num_groupsj(i32 2)
; CHECK-NEXT:    [[NUMSGROUPS_1:%.*]] = call i64 @_Z14get_num_groupsj(i32 1)
; CHECK-NEXT:    [[NUMSGROUPS_0:%.*]] = call i64 @_Z14get_num_groupsj(i32 0)
; CHECK-NEXT:    [[NUMSGROUP_P0:%.*]] = mul nuw nsw i64 [[NUMSGROUPS_2]], [[NUMSGROUPS_1]]
; CHECK-NEXT:    [[NUMSGROUP_P1:%.*]] = mul nuw nsw i64 [[NUMSGROUP_P0]], [[NUMSGROUPS_0]]
; CHECK-NEXT:    [[NUMS32:%.*]] = trunc i64 [[NUMSGROUP_P1]] to i32
; CHECK-NEXT:    call void @_Z20intel_device_barrierj(i32 [[NUMS32]])
; CHECK-NEXT:    call void @dummy_barrier.()
  tail call void @_Z20intel_device_barrierj12memory_scope(i32 noundef 2, i32 noundef 2)
  %0 = load i32, i32 addrspace(1)* %arrayidx, align 4
  %call8 = tail call i64 @_Z15get_global_sizej(i32 noundef 0) #6
  %1 = xor i64 %call3, -1
  %sub10 = add i64 %call8, %1
  %arrayidx11 = getelementptr inbounds i32, i32 addrspace(1)* %data, i64 %sub10
  %2 = load i32, i32 addrspace(1)* %arrayidx11, align 4
  %add = add nsw i32 %2, %0
; CHECK:    call void @_Z18work_group_barrierj(i32 1)
; CHECK-NEXT:    [[NUMSGROUPS_21:%.*]] = call i64 @_Z14get_num_groupsj(i32 2)
; CHECK-NEXT:    [[NUMSGROUPS_12:%.*]] = call i64 @_Z14get_num_groupsj(i32 1)
; CHECK-NEXT:    [[NUMSGROUPS_03:%.*]] = call i64 @_Z14get_num_groupsj(i32 0)
; CHECK-NEXT:    [[NUMSGROUP_P04:%.*]] = mul nuw nsw i64 [[NUMSGROUPS_21]], [[NUMSGROUPS_12]]
; CHECK-NEXT:    [[NUMSGROUP_P15:%.*]] = mul nuw nsw i64 [[NUMSGROUP_P04]], [[NUMSGROUPS_03]]
; CHECK-NEXT:    [[NUMS326:%.*]] = trunc i64 [[NUMSGROUP_P15]] to i32
; CHECK-NEXT:    call void @_Z20intel_device_barrierj(i32 [[NUMS326]])
; CHECK-NEXT:    call void @dummy_barrier.()
  tail call void @_Z20intel_device_barrierj12memory_scope(i32 noundef 2, i32 noundef 2)
  %arrayidx13 = getelementptr inbounds i32, i32 addrspace(1)* %result, i64 %call3
  store i32 %add, i32 addrspace(1)* %arrayidx13, align 4
  ret void
}

; Function Attrs: convergent mustprogress nofree nounwind willreturn memory(none)
declare i64 @_Z13get_global_idj(i32 noundef) 

; Function Attrs: convergent nounwind
declare void @_Z20intel_device_barrierj12memory_scope(i32 noundef, i32 noundef)

; Function Attrs: convergent mustprogress nofree nounwind willreturn memory(none)
declare i64 @_Z15get_global_sizej(i32 noundef)

; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test --  call void @dummy_barrier.()
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test --  call void @dummy_barrier.()
; DEBUGIFY-NOT: WARNING
