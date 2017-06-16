; RUN: opt -spir-materializer -S %s -o - | FileCheck %s

; CHECK: !kernel_arg_addr_space ![[AS:[0-9]+]]
; CHECK: !kernel_arg_access_qual ![[AQ:[0-9]+]]
; CHECK: !kernel_arg_type ![[T:[0-9]+]]
; CHECK: !kernel_arg_type_qual ![[TQ:[0-9]+]]
; CHECK: !kernel_arg_base_type ![[T]]
; CHECK: @__to_global
; CHECK: @__to_local
; CHECK-NOT: @_Z9to_globalPU3AS4c
; CHECK-NOT: @_Z8to_localPU3AS4c
; CHECK: ![[AS]] = !{i32 1}
; CHECK: ![[AQ]] = !{!"none"}
; CHECK: ![[T]] = !{!"int*"}
; CHECK: ![[TQ]] = !{!""}

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

@gint = addrspace(1) global i32 1, align 4
@testKernel.lint = internal addrspace(3) global i32 undef, align 4

; Function Attrs: nounwind
define spir_func zeroext i1 @isFenceValid(i32 %fence) #0 {
entry:
  %cmp = icmp eq i32 %fence, 0
  br i1 %cmp, label %if.then, label %lor.lhs.false

lor.lhs.false:                                    ; preds = %entry
  %cmp1 = icmp eq i32 %fence, 2
  br i1 %cmp1, label %if.then, label %lor.lhs.false2

lor.lhs.false2:                                   ; preds = %lor.lhs.false
  %cmp3 = icmp eq i32 %fence, 1
  br i1 %cmp3, label %if.then, label %lor.lhs.false4

lor.lhs.false4:                                   ; preds = %lor.lhs.false2
  %cmp5 = icmp eq i32 %fence, 3
  br i1 %cmp5, label %if.then, label %if.else

if.then:                                          ; preds = %lor.lhs.false4, %lor.lhs.false2, %lor.lhs.false, %entry
  br label %return

if.else:                                          ; preds = %lor.lhs.false4
  br label %return

return:                                           ; preds = %if.else, %if.then
  %retval.0 = phi i1 [ true, %if.then ], [ false, %if.else ]
  ret i1 %retval.0
}

; Function Attrs: nounwind
define spir_kernel void @testKernel(i32 addrspace(1)* %results) #0 {
entry:
  %call = call spir_func i64 @_Z13get_global_idj(i32 0) #1
  %conv = trunc i64 %call to i32
  store i32 2, i32 addrspace(3)* @testKernel.lint, align 4
  %rem = urem i32 %conv, 2
  %tobool = icmp ne i32 %rem, 0
  %0 = bitcast i32 addrspace(1)* @gint to i8 addrspace(1)*
  %1 = addrspacecast i8 addrspace(1)* %0 to i8 addrspace(4)*
  %2 = bitcast i32 addrspace(3)* @testKernel.lint to i8 addrspace(3)*
  %3 = addrspacecast i8 addrspace(3)* %2 to i8 addrspace(4)*
  %cond = select i1 %tobool, i8 addrspace(4)* %1, i8 addrspace(4)* %3
  %4 = bitcast i8 addrspace(4)* %cond to i32 addrspace(4)*
  call spir_func void @_Z18work_group_barrierji(i32 2, i32 1) #0
  %rem1 = urem i32 %conv, 2
  %tobool2 = icmp ne i32 %rem1, 0
  br i1 %tobool2, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  %5 = bitcast i32 addrspace(4)* %4 to i8 addrspace(4)*
  %call3.tmp = call spir_func i32 @_Z9get_fencePKU3AS4v(i8 addrspace(4)* %5) #0
  %call3.tmp1 = shl i32 %call3.tmp, 8
  %call3.old = lshr exact i32 %call3.tmp1, 8
  %call4 = call spir_func zeroext i1 @isFenceValid(i32 %call3.old) #0
  br i1 %call4, label %land.lhs.true, label %land.end

land.lhs.true:                                    ; preds = %if.then
  %6 = bitcast i32 addrspace(4)* %4 to i8 addrspace(4)*
  %7 = bitcast i8 addrspace(4)* %6 to i8 addrspace(4)*
  %call6.tmp = call spir_func i8 addrspace(1)* @_Z9to_globalPU3AS4c(i8 addrspace(4)* %7) #0
  %call6.old = bitcast i8 addrspace(1)* %call6.tmp to i8 addrspace(1)*
  %tobool7 = icmp ne i8 addrspace(1)* %call6.old, null
  br i1 %tobool7, label %land.rhs, label %land.end

land.rhs:                                         ; preds = %land.lhs.true
  %8 = load i32, i32 addrspace(4)* %4, align 4
  %9 = load i32, i32 addrspace(1)* @gint, align 4
  %cmp = icmp eq i32 %8, %9
  br label %land.end

land.end:                                         ; preds = %land.rhs, %land.lhs.true, %if.then
  %10 = phi i1 [ false, %land.lhs.true ], [ false, %if.then ], [ %cmp, %land.rhs ]
  %land.ext = select i1 %10, i32 1, i32 0
  %idxprom = zext i32 %conv to i64
  %arrayidx = getelementptr inbounds i32, i32 addrspace(1)* %results, i64 %idxprom
  store i32 %land.ext, i32 addrspace(1)* %arrayidx, align 4
  br label %if.end

if.else:                                          ; preds = %entry
  %11 = bitcast i32 addrspace(4)* %4 to i8 addrspace(4)*
  %call9.tmp = call spir_func i32 @_Z9get_fencePKU3AS4v(i8 addrspace(4)* %11) #0
  %call9.tmp2 = shl i32 %call9.tmp, 8
  %call9.old = lshr exact i32 %call9.tmp2, 8
  %call10 = call spir_func zeroext i1 @isFenceValid(i32 %call9.old) #0
  br i1 %call10, label %land.lhs.true12, label %land.end18

land.lhs.true12:                                  ; preds = %if.else
  %12 = bitcast i32 addrspace(4)* %4 to i8 addrspace(4)*
  %13 = bitcast i8 addrspace(4)* %12 to i8 addrspace(4)*
  %call13.tmp = call spir_func i8 addrspace(3)* @_Z8to_localPU3AS4c(i8 addrspace(4)* %13) #0
  %call13.old = bitcast i8 addrspace(3)* %call13.tmp to i8 addrspace(3)*
  %tobool14 = icmp ne i8 addrspace(3)* %call13.old, null
  br i1 %tobool14, label %land.rhs15, label %land.end18

land.rhs15:                                       ; preds = %land.lhs.true12
  %14 = load i32, i32 addrspace(4)* %4, align 4
  %15 = load i32, i32 addrspace(3)* @testKernel.lint, align 4
  %cmp16 = icmp eq i32 %14, %15
  br label %land.end18

land.end18:                                       ; preds = %land.rhs15, %land.lhs.true12, %if.else
  %16 = phi i1 [ false, %land.lhs.true12 ], [ false, %if.else ], [ %cmp16, %land.rhs15 ]
  %land.ext19 = select i1 %16, i32 1, i32 0
  %idxprom20 = zext i32 %conv to i64
  %arrayidx21 = getelementptr inbounds i32, i32 addrspace(1)* %results, i64 %idxprom20
  store i32 %land.ext19, i32 addrspace(1)* %arrayidx21, align 4
  br label %if.end

if.end:                                           ; preds = %land.end18, %land.end
  ret void
}

; Function Attrs: nounwind
declare spir_func void @_Z18work_group_barrierji(i32, i32) #0

; Function Attrs: nounwind
declare spir_func i32 @_Z9get_fencePKU3AS4v(i8 addrspace(4)*) #0

; Function Attrs: nounwind
declare spir_func i8 addrspace(1)* @_Z9to_globalPU3AS4c(i8 addrspace(4)*) #0

; Function Attrs: nounwind
declare spir_func i8 addrspace(3)* @_Z8to_localPU3AS4c(i8 addrspace(4)*) #0

; Function Attrs: nounwind readnone
declare spir_func i64 @_Z13get_global_idj(i32) #1

attributes #0 = { nounwind }
attributes #1 = { nounwind readnone }

!opencl.kernels = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!spirv.Source = !{!6}
!opencl.spir.version = !{!7}
!opencl.ocl.version = !{!7}
!opencl.used.extensions = !{!8}
!opencl.used.optional.core.features = !{!8}
!spirv.Generator = !{!9}
!opencl.compiler.options = !{!10}

!0 = !{void (i32 addrspace(1)*)* @testKernel, !1, !2, !3, !4, !5}
!1 = !{!"kernel_arg_addr_space", i32 1}
!2 = !{!"kernel_arg_access_qual", !"none"}
!3 = !{!"kernel_arg_type", !"int*"}
!4 = !{!"kernel_arg_type_qual", !""}
!5 = !{!"kernel_arg_base_type", !"int*"}
!6 = !{i32 3, i32 200000}
!7 = !{i32 2, i32 0}
!8 = !{}
!9 = !{i16 6, i16 14}
!10 = !{!"-cl-std=CL2.0"}