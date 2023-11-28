; RUN: opt -passes=sycl-kernel-wg-loop-bound %s -S -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes=sycl-kernel-wg-loop-bound %s -S -debug -disable-output 2>&1 | FileCheck %s
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; CHECK: WGLoopBoundaries
; CHECK: found 0 early exit boundaries
; CHECK: found 0 uniform early exit conditions

; Function Attrs: nounwind
define void @_ZTSZZ4mainENKUlRN2cl4sycl7handlerEE33_15clES2_EUlNS0_7nd_itemILi3EEEE36_8(i32 %_arg_, i32 %_arg_1, i32 %_arg_3, ptr addrspace(1) noalias %_arg_5) !no_barrier_path !11 {
entry:
  %0 = tail call i64 @_Z12get_local_idj(i32 0) #2
  %cmp.i.i.i = icmp ult i64 %0, 2147483648
  tail call void @llvm.assume(i1 %cmp.i.i.i)
  %div.lhs.trunc.i.i = trunc i64 %0 to i32
  %div14.i.i = udiv i32 %div.lhs.trunc.i.i, %_arg_3
  %1 = mul i32 %div14.i.i, %_arg_3
  %rem15.i.i.decomposed = sub i32 %div.lhs.trunc.i.i, %1
  %cmp.i.i = icmp ult i32 %rem15.i.i.decomposed, %_arg_1
  %mul.i.i = mul i32 %_arg_3, %_arg_
  %cmp4.i.i = icmp ugt i32 %mul.i.i, %div.lhs.trunc.i.i
  %or.cond.i.i = and i1 %cmp4.i.i, %cmp.i.i
  br i1 %or.cond.i.i, label %if.end.i.i, label %"_ZZZ4mainENK3$_0clERN2cl4sycl7handlerEENKUlNS1_7nd_itemILi3EEEE_clES5_.exit"

if.end.i.i:                                       ; preds = %entry
  %div.zext.i.i = zext i32 %div14.i.i to i64
  %ptridx.i.i67 = getelementptr inbounds float, ptr addrspace(1) %_arg_5, i64 %div.zext.i.i
  store float 0.000000e+00, ptr addrspace(1) %ptridx.i.i67, align 4
  br label %"_ZZZ4mainENK3$_0clERN2cl4sycl7handlerEENKUlNS1_7nd_itemILi3EEEE_clES5_.exit"

"_ZZZ4mainENK3$_0clERN2cl4sycl7handlerEENKUlNS1_7nd_itemILi3EEEE_clES5_.exit": ; preds = %if.end.i.i, %entry
  ret void
}

; Function Attrs: nofree nosync nounwind willreturn
declare void @llvm.assume(i1 noundef) #1

; Function Attrs: nounwind readnone
declare i64 @_Z12get_local_idj(i32) local_unnamed_addr #2

attributes #0 = { nounwind }
attributes #1 = { nofree nosync nounwind willreturn }
attributes #2 = { nounwind readnone }

!spirv.MemoryModel = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!spirv.Source = !{!1}
!opencl.spir.version = !{!2}
!opencl.ocl.version = !{!3}
!opencl.used.extensions = !{!4}
!opencl.used.optional.core.features = !{!4}
!spirv.Generator = !{!5}
!sycl.kernels = !{!6}

!0 = !{i32 2, i32 2}
!1 = !{i32 4, i32 100000}
!2 = !{i32 1, i32 2}
!3 = !{i32 1, i32 0}
!4 = !{}
!5 = !{i16 6, i16 14}
!6 = !{ptr @_ZTSZZ4mainENKUlRN2cl4sycl7handlerEE33_15clES2_EUlNS0_7nd_itemILi3EEEE36_8}
!7 = !{i32 0, i32 0, i32 0, i32 1}
!8 = !{!"none", !"none", !"none", !"none"}
!9 = !{!"int", !"int", !"int", !"ptr"}
!10 = !{!"", !"", !"", !""}
!11 = !{i1 true}

; DEBUGIFY-NOT: WARNING
