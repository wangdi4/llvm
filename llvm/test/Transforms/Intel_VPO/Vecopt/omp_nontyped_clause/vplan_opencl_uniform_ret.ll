; RUN: opt -vplan-vec -S < %s | FileCheck %s
; RUN: opt -passes="vplan-vec" -S < %s | FileCheck %s
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: convergent nounwind
declare spir_kernel void @a(i32 addrspace(1)* nocapture readonly %a, i32 addrspace(1)* nocapture %b)

; Function Attrs: convergent
declare spir_func i32 @_Z13sub_group_alli(i32) local_unnamed_addr #1

; Function Attrs: convergent nounwind readnone
declare spir_func i64 @_Z13get_global_idj(i32) local_unnamed_addr #2

; Function Attrs: convergent
declare spir_func i32 @_Z22get_sub_group_local_idv() local_unnamed_addr #3

; Function Attrs: convergent nounwind
define spir_kernel void @_ZGVeN4uu_a(i32 addrspace(1)* nocapture readonly %a, i32 addrspace(1)* nocapture %b) local_unnamed_addr #0 !kernel_arg_addr_space !5 !kernel_arg_access_qual !6 !kernel_arg_type !7 !kernel_arg_base_type !7 !kernel_arg_type_qual !8 !kernel_arg_host_accessible !9 !kernel_arg_pipe_depth !10 !kernel_arg_pipe_io !8 !kernel_arg_buffer_location !8 !vectorized_kernel !13 !vectorized_width !18 !scalarized_kernel !4 !ocl_recommended_vector_length !18 !vectorization_dimension !19 !can_unite_workgroups !20 {
entry:
  %call = tail call spir_func i64 @_Z13get_global_idj(i32 0) #5
  %slid = tail call i32 @_Z22get_sub_group_local_idv() #5
  br label %simd.begin.region

simd.begin.region:                                ; preds = %entry
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4), "QUAL.OMP.UNIFORM"(i32 addrspace(1)* %a, i32 addrspace(1)* %b) ]
  br label %simd.loop

simd.loop:                                        ; preds = %simd.loop.exit, %simd.begin.region
  %index = phi i32 [ 0, %simd.begin.region ], [ %indvar, %simd.loop.exit ]
  %add1 = add nuw i32 %index, %slid
  %0 = sext i32 %index to i64
  %add = add nuw i64 %0, %call
  %arrayidx = getelementptr inbounds i32, i32 addrspace(1)* %a, i64 %add
  br label %loop

loop:                                             ; preds = %loop, %simd.loop
  %phi = phi i32 [ 0, %simd.loop ], [ %phi.next, %loop ]
  %phi.next = add i32 %phi, 1
  %1 = load i32, i32 addrspace(1)* %arrayidx, align 4, !tbaa !14
  ; Loop exit condition depends on this call. If we don't recognize it as
  ; uniform, LoopCFU will make the call masked which isn't correct (at least for
  ; now we don't have the masked versions with run-time all-zero bypass check
  ; for the mask).
  %all = tail call spir_func i32 @_Z13sub_group_alli(i32 %1) #7
  ; CHECK: call spir_func <4 x i32> @_Z13sub_group_allDv4_i(<4 x i32> %wide.load)
  %cond = icmp eq i32 %all, 0
  br i1 %cond, label %loop, label %exit

exit:                                             ; preds = %loop
  %phi.next.lcssa = phi i32 [ %phi.next, %loop ]
  store i32 %phi.next.lcssa, i32 addrspace(1)* %arrayidx, align 4, !tbaa !14
  br label %simd.loop.exit

simd.loop.exit:                                   ; preds = %exit
  %indvar = add nuw i32 %index, 1
  %vl.cond = icmp ult i32 %indvar, 4
  br i1 %vl.cond, label %simd.loop, label %simd.end.region, !llvm.loop !21

simd.end.region:                                  ; preds = %simd.loop.exit
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  br label %return

return:                                           ; preds = %simd.end.region
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #4

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #4

attributes #0 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "uniform-work-group-size"="false" "unsafe-fp-math"="false" "use-soft-float"="false" "vector-variants"="_ZGVeN4uu_a" }
attributes #1 = { convergent "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "opencl-vec-uniform-return" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { convergent nounwind readnone "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { convergent "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { nounwind }
attributes #5 = { convergent nounwind readnone }
attributes #6 = { convergent nounwind }
attributes #7 = { convergent nounwind "vector-variants"="_ZGVbN4v_Z13sub_group_alli(_Z13sub_group_allDv4_i)" }

!llvm.module.flags = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!1}
!opencl.spir.version = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!3}
!opencl.kernels = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 2, i32 0}
!2 = !{}
!3 = !{!"icx (ICX) dev.8.x.0"}
!4 = !{void (i32 addrspace(1)*, i32 addrspace(1)*)* @a}
!5 = !{i32 1, i32 1}
!6 = !{!"none", !"none"}
!7 = !{!"int*", !"int*"}
!8 = !{!"", !""}
!9 = !{i1 false, i1 false}
!10 = !{i32 0, i32 0}
!11 = !{void (i32 addrspace(1)*, i32 addrspace(1)*)* @_ZGVeN4uu_a}
!12 = !{i32 1}
!13 = !{null}
!14 = !{!15, !15, i64 0}
!15 = !{!"int", !16, i64 0}
!16 = !{!"omnipotent char", !17, i64 0}
!17 = !{!"Simple C/C++ TBAA"}
!18 = !{i32 4}
!19 = !{i32 0}
!20 = !{i1 false}
!21 = distinct !{!21, !22}
!22 = !{!"llvm.loop.unroll.disable"}
