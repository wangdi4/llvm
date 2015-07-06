; ModuleID = 'test_memset.cl'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-pc-win32"

@.str = private unnamed_addr constant [4 x i8] c" %d\00", align 1

define void @test_memset() nounwind uwtable {
entry:
  %arr = alloca [8 x i32], align 16
  %i = alloca i32, align 4
  %0 = bitcast [8 x i32]* %arr to i8*
  call void @llvm.memset.p0i8.i64(i8* %0, i8 3, i64 32, i32 16, i1 false)
  store i32 0, i32* %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %1 = load i32* %i, align 4
  %cmp = icmp slt i32 %1, 8
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %2 = load i32* %i, align 4
  %idxprom = sext i32 %2 to i64
  %arrayidx = getelementptr inbounds [8 x i32]* %arr, i32 0, i64 %idxprom
  %3 = load i32* %arrayidx, align 4
  %call = call i32 (i8 addrspace(2)*, ...)* @printf(i8 addrspace(2)* addrspacecast ([4 x i8]* @.str to i8 addrspace(2)*), i32 %3)
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %4 = load i32* %i, align 4
  %inc = add nsw i32 %4, 1
  store i32 %inc, i32* %i, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  ret void
}

declare void @llvm.memset.p0i8.i64(i8* nocapture, i8, i64, i32, i1) nounwind

declare i32 @printf(i8 addrspace(2)*, ...)

!opencl.kernels = !{!0}
!opencl.compiler.options = !{!2}

!0 = !{void ()* @test_memset, !1}
!1 = !{!"image_access_qualifier"}
!2 = !{!"-cl-std=CL1.2"}
