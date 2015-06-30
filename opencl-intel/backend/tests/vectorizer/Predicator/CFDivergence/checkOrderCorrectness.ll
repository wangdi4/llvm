; RUN: llvm-as %s -o %t.bc
; RUN: opt  -predicate %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'checkOrderCorrectness.ll'

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

@.str = private unnamed_addr constant [13 x i8] c"%d <= 0????\0A\00", align 1
@testKernel.a = internal addrspace(3) global i16 0, align 2
@testKernel.b = internal addrspace(3) global i32 0, align 4

; CHECK: @testFunction
; CHECK-NOT: No predecessors!

define void @testFunction(i16 addrspace(3)* %a, i32 addrspace(3)* %b) nounwind {
entry:
  %a.addr = alloca i16 addrspace(3)*, align 8
  %b.addr = alloca i32 addrspace(3)*, align 8
  %c = alloca i32, align 4
  store i16 addrspace(3)* %a, i16 addrspace(3)** %a.addr, align 8
  store i32 addrspace(3)* %b, i32 addrspace(3)** %b.addr, align 8
  %call = call i64 @_Z12get_local_idj(i32 0) nounwind readnone
  %cmp = icmp eq i64 %call, 0
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %0 = load i16 addrspace(3)** %a.addr, align 8
  store i16 50, i16 addrspace(3)* %0, align 2
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  call void @_Z7barrierj(i32 1)
  %1 = load i16 addrspace(3)** %a.addr, align 8
  %2 = load i16 addrspace(3)* %1, align 2
  %conv = sext i16 %2 to i32
  %div = sdiv i32 %conv, 32
  store i32 %div, i32* %c, align 4
  %3 = load i32* %c, align 4
  %cmp1 = icmp sgt i32 %3, 0
  br i1 %cmp1, label %if.then3, label %if.else

if.then3:                                         ; preds = %if.end
  %call4 = call i64 @_Z12get_local_idj(i32 0) nounwind readnone
  %cmp5 = icmp eq i64 %call4, 0
  br i1 %cmp5, label %if.then7, label %if.end8

if.then7:                                         ; preds = %if.then3
  %4 = load i32 addrspace(3)** %b.addr, align 8
  store i32 0, i32 addrspace(3)* %4, align 4
  br label %if.end8

if.end8:                                          ; preds = %if.then7, %if.then3
  br label %if.end10

if.else:                                          ; preds = %if.end
  %5 = load i32* %c, align 4
  %call9 = call i32 (i8 addrspace(2)*, ...)* @printf(i8 addrspace(2)* bitcast ([13 x i8]* @.str to i8 addrspace(2)*), i32 %5)
  br label %if.end10

if.end10:                                         ; preds = %if.else, %if.end8
  ret void
}

declare i64 @_Z12get_local_idj(i32) nounwind readnone

declare void @_Z7barrierj(i32)

declare i32 @printf(i8 addrspace(2)*, ...)

define void @testKernel() nounwind {
entry:
  call void @testFunction(i16 addrspace(3)* @testKernel.a, i32 addrspace(3)* @testKernel.b)
  ret void
}

!opencl.kernels = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.spir.version = !{!6}
!opencl.ocl.version = !{!7}
!opencl.used.extensions = !{!8}
!opencl.used.optional.core.features = !{!8}
!opencl.compiler.options = !{!8}

!0 = !{void ()* @testKernel, !1, !2, !3, !4, !5}
!1 = !{!"kernel_arg_addr_space"}
!2 = !{!"kernel_arg_access_qual"}
!3 = !{!"kernel_arg_type"}
!4 = !{!"kernel_arg_type_qual"}
!5 = !{!"kernel_arg_name"}
!6 = !{i32 1, i32 0}
!7 = !{i32 0, i32 0}
!8 = !{}
