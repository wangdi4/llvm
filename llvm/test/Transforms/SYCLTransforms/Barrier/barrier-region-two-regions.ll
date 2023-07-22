; RUN: opt -passes=sycl-kernel-barrier -S < %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-barrier -S < %s | FileCheck %s

; Check that %rhs's load from special buffer is within for.end which dominates
; its users within a barrier region.
;
; %rhs's users are in two regions:
; region 1:
;      for.end
;         |
;     if.then10
;
; region 2, which is a top-level region:
;     if.end12 and all other basic blocks

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: convergent nounwind
define void @__kmpc_reduction_add_int(i32 %Id, i32 %Size, ptr addrspace(4) %LocalResult, ptr addrspace(4) %Output) #0 {
entry:
  call void @dummy_barrier.()
  %Id.addr = alloca i32, align 4
  %rhs = alloca i32, align 4
  %stride = alloca i32, align 4
  store i32 %Id, ptr %Id.addr, align 4
  br label %do.end

do.end:                                           ; preds = %entry
  call void @_Z18work_group_barrierj12memory_scope(i32 3, i32 1) #0
  store i32 0, ptr %stride, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %do.end
  %0 = load i32, ptr %stride, align 4
  %cmp1 = icmp ugt i32 %0, 0
  br i1 %cmp1, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  br label %for.end

for.body:                                         ; preds = %for.cond
  call void @_Z18work_group_barrierj12memory_scope(i32 3, i32 1) #0
  br label %for.inc

for.inc:                                          ; preds = %if.end7
  br label %for.cond

for.end:                                          ; preds = %for.cond.cleanup
; CHECK-LABEL: for.end:
; CHECK-NEXT: [[Index:%SBIndex[0-9]*]] = load i64, ptr %pCurrSBIndex, align 8
; CHECK-NEXT: [[Offset:%SB_LocalId_Offset[0-9]*]] = add nuw i64 [[Index]], {{[0-9]+}}
; CHECK-NEXT: [[GEP:%pSB_LocalId[0-9]*]] = getelementptr inbounds i8, ptr %pSB, i64 [[Offset]]
; CHECK-NEXT: store ptr [[GEP]], ptr %rhs.addr, align 8
; CHECK-NEXT: [[LOAD:%[0-9]+]] = load ptr, ptr %rhs.addr, align 8

  %1 = load i32, ptr %Id.addr, align 4
  %cmp9 = icmp eq i32 %1, 0
  br i1 %cmp9, label %if.then10, label %if.end12

if.then10:                                        ; preds = %for.end
; CHECK-LABEL: if.then10:
; CHECK-NEXT: store i32 0, ptr [[LOAD]], align 4
; CHECK-NEXT: load i32, ptr [[LOAD]], align 4

  store i32 0, ptr %rhs, align 4
  %2 = load i32, ptr %rhs, align 4
  br label %if.end12

if.end12:                                         ; preds = %if.then10, %for.end
; CHECK-LABEL: if.end12:
; CHECK-NEXT: bitcast ptr [[LOAD]] to ptr

  %3 = bitcast ptr %rhs to ptr
  call void @llvm.lifetime.end.p0(i64 4, ptr %3)
  br label %Split.Barrier.BB

Split.Barrier.BB:                                 ; preds = %if.end12
  call void @_Z18work_group_barrierj(i32 1)
  ret void
}

; Function Attrs: convergent nounwind
declare void @_Z18work_group_barrierj12memory_scope(i32, i32) #0

declare void @dummy_barrier.()

; Function Attrs: convergent
declare void @_Z18work_group_barrierj(i32) #1

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #2

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #2

attributes #0 = { convergent nounwind "kernel-call-once" "kernel-convergent-call" }
attributes #1 = { convergent }
attributes #2 = { nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }

!spirv.Source = !{!0}
!spirv.Generator = !{!1}

!0 = !{i32 4, i32 200000}
!1 = !{i16 6, i16 14}

; DEBUGIFY-COUNT-7: WARNING: Instruction with empty DebugLoc in function __kmpc_reduction_add_int
; DEBUGIFY-NOT: WARNING
