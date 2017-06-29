;; Test packetization of gather/scatter prefetches.
;;
;; LLVM IR was generated using the following command:
;; bash$ ./clang -cc1 -x cl -cl-std=CL2.0 -triple spir64-unknown-unknown -include opencl-c.h -emit-llvm gather_scatter_prefetch.cl -o -

; RUN: opt -runtimelib %p/../../Full/runtime.bc -scalarize -predicate -mem2reg -dce -packetize -packet-size=16 -gather-scatter -gather-scatter-prefetch -verify %s -S -o - | FileCheck -check-prefix=CHECK_HW_PF %s
; RUN: opt -runtimelib %p/../../Full/runtime.bc -scalarize -predicate -mem2reg -dce -packetize -packet-size=16 -gather-scatter -verify %s -S -o - | FileCheck -check-prefix=CHECK_SW_PF %s

; CHECK_HW_PF: call void @"internal.prefetch.gather.v16f32[i64].m1"

; CHECK_SW_PF: call spir_func void {{.*}}prefetch{{.*}}

; ModuleID = 'test_basic.ocl_recorder.53.530.cl'
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown"

; Function Attrs: nounwind
define spir_kernel void @test_fn(<8 x float> addrspace(1)* %src, <8 x float> addrspace(1)* nocapture %dst, <8 x float> addrspace(3)* nocapture readnone %localBuffer, i32 %copiesPerWorkgroup, i32 %copiesPerWorkItem) #0 {
entry:
  %conv = sext i32 %copiesPerWorkItem to i64
  %call = tail call spir_func i64 @_Z13get_global_idj(i32 0) #3
  %mul = mul i64 %call, %conv
  %add.ptr = getelementptr inbounds <8 x float>, <8 x float> addrspace(1)* %src, i64 %mul
  tail call spir_func void @_Z8prefetchPU3AS1KDv8_fm(<8 x float> addrspace(1)* %add.ptr, i64 %conv) #4
  %cmp22 = icmp sgt i32 %copiesPerWorkItem, 0
  br i1 %cmp22, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %i.023 = phi i32 [ %inc, %for.body ], [ 0, %for.body.preheader ]
  %conv6 = sext i32 %i.023 to i64
  %add = add i64 %conv6, %mul
  %arrayidx = getelementptr inbounds <8 x float>, <8 x float> addrspace(1)* %src, i64 %add
  %0 = load <8 x float>, <8 x float> addrspace(1)* %arrayidx, align 32, !tbaa !9
  %arrayidx12 = getelementptr inbounds <8 x float>, <8 x float> addrspace(1)* %dst, i64 %add
  store <8 x float> %0, <8 x float> addrspace(1)* %arrayidx12, align 32, !tbaa !9
  %inc = add nuw nsw i32 %i.023, 1
  %cmp = icmp slt i32 %inc, %copiesPerWorkItem
  br i1 %cmp, label %for.body, label %for.end.loopexit

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

declare spir_func void @_Z8prefetchPU3AS1KDv8_fm(<8 x float> addrspace(1)*, i64) #1

; Function Attrs: nounwind readnone
declare spir_func i64 @_Z13get_global_idj(i32) #2

attributes #0 = { nounwind "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind readnone "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind readnone }
attributes #4 = { nounwind }

!opencl.kernels = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!6}
!opencl.spir.version = !{!6}
!opencl.used.extensions = !{!7}
!opencl.used.optional.core.features = !{!7}
!opencl.compiler.options = !{!7}

!0 = !{void (<8 x float> addrspace(1)*, <8 x float> addrspace(1)*, <8 x float> addrspace(3)*, i32, i32)* @test_fn, !1, !2, !3, !4, !5}
!1 = !{!"kernel_arg_addr_space", i32 1, i32 1, i32 3, i32 0, i32 0}
!2 = !{!"kernel_arg_access_qual", !"none", !"none", !"none", !"none", !"none"}
!3 = !{!"kernel_arg_type", !"float8*", !"float8*", !"float8*", !"int", !"int"}
!4 = !{!"kernel_arg_base_type", !"float8*", !"float8*", !"float8*", !"int", !"int"}
!5 = !{!"kernel_arg_type_qual", !"const", !"", !"", !"", !""}
!6 = !{i32 2, i32 0}
!7 = !{}
!9 = !{!10, !10, i64 0}
!10 = !{!"omnipotent char", !11, i64 0}
!11 = !{!"Simple C/C++ TBAA"}
