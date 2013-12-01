; Rregression test. Currently 28May this kernel fails compilation
; block variable call in for loop should be resolved to static call
; RUN: opt -resolve-block-call -S < %s | FileCheck %s

;kernel void block_for_cond(__global int* res)
;{
;  int multiplier = 3;
;  int (^kernelBlock)(int) = ^(int num)
;  {
;    return num * multiplier;
;  };
;  int tid = get_global_id(0);
;  res[tid] = 39;
;  for(int i=0; i<kernelBlock(13); i++)
;  {
;       res[tid]--;
;  }
;}

; ModuleID = 'Program'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-pc-win32"

%struct.__block_descriptor.10 = type { i64, i64 }
%struct.__block_literal_generic.11 = type { i8*, i32, i32, i8*, %struct.__block_descriptor.10* }

@_NSConcreteStackBlock = external global i8*
@.str = private unnamed_addr constant [9 x i8] c"i12@?0i8\00", align 1
@__block_descriptor_tmp = internal constant { i64, i64, i8*, i8* } { i64 0, i64 36, i8* getelementptr inbounds ([9 x i8]* @.str, i32 0, i32 0), i8* null }

define void @block_for_cond(i32 addrspace(1)* %res) nounwind {
entry:
  %res.addr = alloca i32 addrspace(1)*, align 8
  %multiplier = alloca i32, align 4
  %kernelBlock = alloca i32 (i32)*, align 8
  %block = alloca <{ i8*, i32, i32, i8*, %struct.__block_descriptor.10*, i32 }>, align 8
  %tid = alloca i32, align 4
  %i = alloca i32, align 4
  store i32 addrspace(1)* %res, i32 addrspace(1)** %res.addr, align 8
  store i32 3, i32* %multiplier, align 4
  %block.isa = getelementptr inbounds <{ i8*, i32, i32, i8*, %struct.__block_descriptor.10*, i32 }>* %block, i32 0, i32 0
  store i8* bitcast (i8** @_NSConcreteStackBlock to i8*), i8** %block.isa
  %block.flags = getelementptr inbounds <{ i8*, i32, i32, i8*, %struct.__block_descriptor.10*, i32 }>* %block, i32 0, i32 1
  store i32 1073741824, i32* %block.flags
  %block.reserved = getelementptr inbounds <{ i8*, i32, i32, i8*, %struct.__block_descriptor.10*, i32 }>* %block, i32 0, i32 2
  store i32 0, i32* %block.reserved
  %block.invoke = getelementptr inbounds <{ i8*, i32, i32, i8*, %struct.__block_descriptor.10*, i32 }>* %block, i32 0, i32 3
  store i8* bitcast (i32 (i8*, i32)* @__block_for_cond_block_invoke to i8*), i8** %block.invoke
  %block.descriptor = getelementptr inbounds <{ i8*, i32, i32, i8*, %struct.__block_descriptor.10*, i32 }>* %block, i32 0, i32 4
  store %struct.__block_descriptor.10* bitcast ({ i64, i64, i8*, i8* }* @__block_descriptor_tmp to %struct.__block_descriptor.10*), %struct.__block_descriptor.10** %block.descriptor
  %block.captured = getelementptr inbounds <{ i8*, i32, i32, i8*, %struct.__block_descriptor.10*, i32 }>* %block, i32 0, i32 5
  %0 = load i32* %multiplier, align 4
  store i32 %0, i32* %block.captured, align 4
  %1 = bitcast <{ i8*, i32, i32, i8*, %struct.__block_descriptor.10*, i32 }>* %block to i32 (i32)*
  store i32 (i32)* %1, i32 (i32)** %kernelBlock, align 8
  %call = call i64 @get_global_id(i32 0) nounwind readnone
  %conv = trunc i64 %call to i32
  store i32 %conv, i32* %tid, align 4
  %2 = load i32* %tid, align 4
  %idxprom = sext i32 %2 to i64
  %3 = load i32 addrspace(1)** %res.addr, align 8
  %arrayidx = getelementptr inbounds i32 addrspace(1)* %3, i64 %idxprom
  store i32 39, i32 addrspace(1)* %arrayidx, align 4
  store i32 0, i32* %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %4 = load i32* %i, align 4
  %5 = load i32 (i32)** %kernelBlock, align 8
  %block.literal = bitcast i32 (i32)* %5 to %struct.__block_literal_generic.11*
  %6 = getelementptr inbounds %struct.__block_literal_generic.11* %block.literal, i32 0, i32 3
  %7 = bitcast %struct.__block_literal_generic.11* %block.literal to i8*
  %8 = load i8** %6
  %9 = bitcast i8* %8 to i32 (i8*, i32)*
; CHECK: call {{.*}}  @__block_for_cond_block_invoke
; CHECK-NOT: call i32 %
  %call1 = call i32 %9(i8* %7, i32 13)
  %cmp = icmp slt i32 %4, %call1
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %10 = load i32* %tid, align 4
  %idxprom3 = sext i32 %10 to i64
  %11 = load i32 addrspace(1)** %res.addr, align 8
  %arrayidx4 = getelementptr inbounds i32 addrspace(1)* %11, i64 %idxprom3
  %12 = load i32 addrspace(1)* %arrayidx4, align 4
  %dec = add nsw i32 %12, -1
  store i32 %dec, i32 addrspace(1)* %arrayidx4, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %13 = load i32* %i, align 4
  %inc = add nsw i32 %13, 1
  store i32 %inc, i32* %i, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  ret void
}

define internal i32 @__block_for_cond_block_invoke(i8* %.block_descriptor, i32 %num) nounwind {
entry:
  %num.addr = alloca i32, align 4
  store i32 %num, i32* %num.addr, align 4
  %block = bitcast i8* %.block_descriptor to <{ i8*, i32, i32, i8*, %struct.__block_descriptor.10*, i32 }>*
  %0 = load i32* %num.addr, align 4
  %block.capture.addr = getelementptr inbounds <{ i8*, i32, i32, i8*, %struct.__block_descriptor.10*, i32 }>* %block, i32 0, i32 5
  %1 = load i32* %block.capture.addr, align 4
  %mul = mul nsw i32 %0, %1
  ret i32 %mul
}

declare i64 @get_global_id(i32) nounwind readnone

!opencl.kernels = !{!0}
!opencl.build.options = !{!2}

!0 = metadata !{void (i32 addrspace(1)*)* @block_for_cond, metadata !1}
!1 = metadata !{metadata !"argument_attribute", i32 0}
!2 = metadata !{metadata !"-cl-std=CL1.2"}
