; RUN: opt -passes=sycl-kernel-vec-clone -sycl-vector-variant-isa-encoding-override=AVX512Core %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-vec-clone -sycl-vector-variant-isa-encoding-override=AVX512Core %s -S -o - | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: convergent nounwind
; CHECK-LABEL: @_ZGVeN4uuuuuuuuuu_compute_sum
define void @compute_sum(ptr addrspace(1) %a, i32 %n, ptr addrspace(1) %tmp_sum, ptr addrspace(1) %sum, ptr addrspace(1) %out_pipe, ptr addrspace(1) %in_pipe, ptr addrspace(3) %localBuffer, ptr addrspace(1) %add.ptr, i64 %conv6, i64 %conv7) local_unnamed_addr #0 !recommended_vector_length !1 !kernel_arg_base_type !3 !arg_type_null_val !4 {
entry:
  %call = tail call i64 @_Z12get_local_idj(i32 0) #2
  %conv = trunc i64 %call to i32
  %call1 = tail call i64 @_Z14get_local_sizej(i32 0) #2
  %conv2 = trunc i64 %call1 to i32
  %sext = shl i64 %call, 32
  %idxprom = ashr exact i64 %sext, 32
  %arrayidx = getelementptr inbounds i32, ptr addrspace(1) %tmp_sum, i64 %idxprom
  store i32 0, ptr addrspace(1) %arrayidx, align 4
  %cmp28 = icmp slt i32 %conv, %n
  br i1 %cmp28, label %for.body.lr.ph, label %for.end

for.body.lr.ph:                                   ; preds = %entry
  %sext31 = shl i64 %call1, 32
  %0 = ashr exact i64 %sext31, 32
  %1 = sext i32 %n to i64
  br label %for.body

for.body:                                         ; preds = %for.body.lr.ph, %for.body
  %2 = phi i32 [ 0, %for.body.lr.ph ], [ %add, %for.body ]
  %indvars.iv = phi i64 [ %idxprom, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %arrayidx5 = getelementptr inbounds i32, ptr addrspace(1) %a, i64 %indvars.iv
  %3 = load i32, ptr addrspace(1) %arrayidx5, align 4
  %add = add nsw i32 %2, %3
  store i32 %add, ptr addrspace(1) %arrayidx, align 4
  %indvars.iv.next = add i64 %indvars.iv, %0
  %cmp = icmp slt i64 %indvars.iv.next, %1
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.body, %entry
  %call9 = tail call i32 @_Z4haddii(i32 %conv2, i32 1) #2
  %cmp1125 = icmp sgt i32 %conv2, 1
  br i1 %cmp1125, label %for.body13, label %for.end25

for.body13:                                       ; preds = %for.end, %if.end
  %i.127 = phi i32 [ %call24, %if.end ], [ %call9, %for.end ]
  %lsize.026 = phi i32 [ %i.127, %if.end ], [ %conv2, %for.end ]
  tail call void @_Z7barrierj(i32 2) #0
; CHECK: call void @_Z7barrierj(i32 2) #[[BARRIER_ATTR:[0-9]+]]
  tail call void @_Z18work_group_barrierj(i32 2) #0
; CHECK: call void @_Z18work_group_barrierj(i32 2) #[[BARRIER_ATTR]]
  tail call void @_Z18work_group_barrierj12memory_scope(i32 2, i32 4) #0
; CHECK: call void @_Z18work_group_barrierj12memory_scope(i32 2, i32 4) #[[BARRIER_ATTR]]
  tail call void @_Z17sub_group_barrierj(i32 2) #0
; CHECK: call void @_Z17sub_group_barrierj(i32 2) #[[BARRIER_ATTR]]
  tail call void @_Z17sub_group_barrierj12memory_scope(i32 2, i32 4) #0
; CHECK: call void @_Z17sub_group_barrierj12memory_scope(i32 2, i32 4) #[[BARRIER_ATTR]]
  tail call void @__builtin_IB_kmp_acquire_lock(ptr addrspace(1) %a) #0
; CHECK: call void @__builtin_IB_kmp_acquire_lock(ptr addrspace(1) %load.a) #[[BARRIER_ATTR]]
  tail call void @__builtin_IB_kmp_release_lock(ptr addrspace(1) %a) #0
; CHECK: call void @__builtin_IB_kmp_release_lock(ptr addrspace(1) %load.a) #[[BARRIER_ATTR]]
  %write_pipe = tail call ptr @__work_group_reserve_write_pipe(ptr addrspace(1) %out_pipe, i32 0, i32 4, i32 4) #0
; CHECK: call ptr @__work_group_reserve_write_pipe(ptr addrspace(1) %load.out_pipe, i32 0, i32 4, i32 4) #[[BARRIER_ATTR]]
  tail call void @__work_group_commit_write_pipe(ptr addrspace(1) %out_pipe, ptr %write_pipe, i32 4, i32 4) #0
; CHECK: call void @__work_group_commit_write_pipe(ptr addrspace(1) %load.out_pipe, ptr %write_pipe, i32 4, i32 4) #[[BARRIER_ATTR]]
  %read_pipe = tail call ptr @__work_group_reserve_read_pipe(ptr addrspace(1) %in_pipe, i32 0, i32 4, i32 4) #0
; CHECK: call ptr @__work_group_reserve_read_pipe(ptr addrspace(1) %load.in_pipe, i32 0, i32 4, i32 4) #[[BARRIER_ATTR]]
  tail call void @__work_group_commit_read_pipe(ptr addrspace(1) %in_pipe, ptr %read_pipe, i32 4, i32 4) #0
; CHECK: call void @__work_group_commit_read_pipe(ptr addrspace(1) %load.in_pipe, ptr %read_pipe, i32 4, i32 4) #[[BARRIER_ATTR]]
  %call8 = tail call ptr @_Z29async_work_group_strided_copyPU3AS3cPU3AS1Kcmm9ocl_event(ptr addrspace(3) %localBuffer, ptr addrspace(1) %add.ptr, i64 %conv6, i64 %conv7, ptr null) #0
; CHECK: tail call ptr @_Z29async_work_group_strided_copyPU3AS3cPU3AS1Kcmm9ocl_event(ptr addrspace(3) %load.localBuffer, ptr addrspace(1) %load.add.ptr, i64 %load.conv6, i64 %load.conv7, ptr null) #[[BARRIER_ATTR]]
  %call6 = tail call ptr @_Z21async_work_group_copyPU3AS3cPU3AS1Kcm9ocl_event(ptr addrspace(3) %localBuffer, ptr addrspace(1) %add.ptr, i64 %conv6, ptr null) #0
; CHECK: call ptr @_Z21async_work_group_copyPU3AS3cPU3AS1Kcm9ocl_event(ptr addrspace(3) %load.localBuffer, ptr addrspace(1) %load.add.ptr, i64 %load.conv6, ptr null) #[[BARRIER_ATTR]]
  %add14 = add nsw i32 %i.127, %conv
  %cmp15 = icmp slt i32 %add14, %lsize.026
  br i1 %cmp15, label %if.then, label %if.end

if.then:                                          ; preds = %for.body13
  %idxprom18 = sext i32 %add14 to i64
  %arrayidx19 = getelementptr inbounds i32, ptr addrspace(1) %tmp_sum, i64 %idxprom18
  %4 = load i32, ptr addrspace(1) %arrayidx19, align 4
  %5 = load i32, ptr addrspace(1) %arrayidx, align 4
  %add22 = add nsw i32 %5, %4
  store i32 %add22, ptr addrspace(1) %arrayidx, align 4
  br label %if.end

if.end:                                           ; preds = %if.then, %for.body13
  %call24 = tail call i32 @_Z4haddii(i32 %i.127, i32 1) #2
  %cmp11 = icmp sgt i32 %i.127, 1
  br i1 %cmp11, label %for.body13, label %for.end25

for.end25:                                        ; preds = %if.end, %for.end
  %cmp26 = icmp eq i32 %conv, 0
  br i1 %cmp26, label %if.then28, label %if.end30

if.then28:                                        ; preds = %for.end25
  %6 = load i32, ptr addrspace(1) %tmp_sum, align 4
  store i32 %6, ptr addrspace(1) %sum, align 4
  br label %if.end30

if.end30:                                         ; preds = %if.then28, %for.end25
  ret void
}

; Function Attrs: convergent nounwind readnone
declare i64 @_Z12get_local_idj(i32 %0) local_unnamed_addr #1

; Function Attrs: convergent nounwind readnone
declare i64 @_Z14get_local_sizej(i32 %0) local_unnamed_addr #1

; Function Attrs: convergent nounwind readnone
declare i32 @_Z4haddii(i32 %0, i32 %1) local_unnamed_addr #1

; Function Attrs: convergent
declare void @_Z7barrierj(i32 %0) local_unnamed_addr #2
declare void @_Z17sub_group_barrierj12memory_scope(i32, i32) #2
declare void @_Z17sub_group_barrierj(i32) #2
declare void @_Z18work_group_barrierj12memory_scope(i32, i32) #2
declare void @_Z18work_group_barrierj(i32) #2
declare void @__builtin_IB_kmp_acquire_lock(ptr addrspace(1)) #2
declare void @__builtin_IB_kmp_release_lock(ptr addrspace(1)) #2
declare ptr @__work_group_reserve_write_pipe(ptr addrspace(1), i32, i32, i32) #2
declare void @__work_group_commit_write_pipe(ptr addrspace(1), ptr, i32, i32) #2
declare ptr @__work_group_reserve_read_pipe(ptr addrspace(1), i32, i32, i32) #2
declare void @__work_group_commit_read_pipe(ptr addrspace(1), ptr, i32, i32) #2
declare ptr @_Z29async_work_group_strided_copyPU3AS3cPU3AS1Kcmm9ocl_event(ptr addrspace(3), ptr addrspace(1), i64, i64, ptr) #2
declare ptr @_Z21async_work_group_copyPU3AS3cPU3AS1Kcm9ocl_event(ptr addrspace(3), ptr addrspace(1), i64, ptr) #2

attributes #0 = { convergent nounwind }
attributes #1 = { convergent nounwind readnone }
attributes #2 = { convergent }
attributes #3 = { nounwind }

; CHECK: attributes #[[BARRIER_ATTR]] = { {{.*}} "kernel-uniform-call" }

!sycl.kernels = !{!0}
!opencl.ocl.version = !{!2}

!0 = !{ptr @compute_sum}
!1 = !{i32 4}
!2 = !{i32 2, i32 0}
!3 = !{!"int*", !"int", !"int*", !"int*", !"int", !"int", !"char*", !"char*", !"long", !"long"}
!4 = !{ptr addrspace(1) null, i32 0, ptr addrspace(1) null, ptr addrspace(1) null, target("spirv.Pipe", 1) zeroinitializer, target("spirv.Pipe", 0) zeroinitializer, ptr addrspace(3) null, ptr addrspace(1) null, i64 0, i64 0}

; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function _ZGVeN4uuuuuuuuuu_compute_sum {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN4uuuuuuuuuu_compute_sum {{.*}} call
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN4uuuuuuuuuu_compute_sum {{.*}} add
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN4uuuuuuuuuu_compute_sum {{.*}} icmp
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN4uuuuuuuuuu_compute_sum {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN4uuuuuuuuuu_compute_sum {{.*}} call
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN4uuuuuuuuuu_compute_sum {{.*}} br
; DEBUGIFY-NOT: WARNING
