; RUN: opt -ocl-asaa -gvn -dse -S < %s | FileCheck %s
; ModuleID = 'main'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Verify that %out and %temp_private are conservatively identified as
; may-alias (due to the private pointer escaping to int) by checking
; that all loads and stores remain (if these addresses are marked as
; anti-aliased the first store and two last loads are optimized away).

;CHECK: define void @test_escape
;CHECK: store i32 333
;CHECK: %1 = load i32 addrspace(3)* %arrayidx3, align 4
;CHECK: store i32 %sub, i32* %add.ptr, align 4
;CHECK: %3 = load i32 addrspace(3)* %arrayidx3, align 4
;CHECK: %4 = load i32 addrspace(1)* %arrayidx, align 4
;CHECK: store i32 %add, i32 addrspace(1)* %arrayidx, align 4
;CHECK: ret void

; Function Attrs: nounwind
define void @test_escape(i32 addrspace(1)* nocapture %out, i32 addrspace(3)* nocapture %in) #0 {
entry:
  %temp_private = alloca [3 x i32], align 4
  %call = call i64 @_Z13get_global_idj(i32 0) #2
  %0 = ptrtoint [3 x i32]* %temp_private to i64
  %sext = shl i64 %0, 32
  %conv1 = ashr exact i64 %sext, 32
  %sext8 = shl i64 %call, 32
  %idxprom = ashr exact i64 %sext8, 32
  %arrayidx = getelementptr inbounds i32 addrspace(1)* %out, i64 %idxprom
  store i32 333, i32 addrspace(1)* %arrayidx, align 4
  %arrayidx3 = getelementptr inbounds i32 addrspace(3)* %in, i64 %idxprom
  %1 = load i32 addrspace(3)* %arrayidx3, align 4
  %sub = add nsw i32 %1, -333
  %2 = inttoptr i64 %conv1 to i32*
  %add.ptr = getelementptr inbounds i32* %2, i64 2
  store i32 %sub, i32* %add.ptr, align 4
  %3 = load i32 addrspace(3)* %arrayidx3, align 4
  %mul = mul nsw i32 %3, 7
  %4 = load i32 addrspace(1)* %arrayidx, align 4
  %add = add nsw i32 %4, %mul
  store i32 %add, i32 addrspace(1)* %arrayidx, align 4
  ret void
}

; Function Attrs: nounwind readnone
declare i64 @_Z13get_global_idj(i32) #1

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-frame-pointer-elim-non-leaf"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-frame-pointer-elim-non-leaf"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind readnone }

!opencl.kernels = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.spir.version = !{!8}
!opencl.ocl.version = !{!8}
!opencl.used.extensions = !{!9}
!opencl.used.optional.core.features = !{!9}
!opencl.compiler.options = !{!10}

!0 = !{void (i32 addrspace(1)*, i32 addrspace(3)*)* @test_escape, !1, !2, !3, !4, !5, !6, !7}
!1 = !{!"kernel_arg_addr_space", i32 1, i32 3}
!2 = !{!"kernel_arg_access_qual", !"none", !"none"}
!3 = !{!"kernel_arg_type", !"int*", !"int*"}
!4 = !{!"kernel_arg_type_qual", !"", !""}
!5 = !{!"kernel_arg_base_type", !"int*", !"int*"}
!6 = !{!"kernel_arg_name", !"out", !"in"}
!7 = !{!"vec_type_hint", <4 x i32> undef, i32 1}
!8 = !{i32 1, i32 2}
!9 = !{}
!10 = !{!"-I."}