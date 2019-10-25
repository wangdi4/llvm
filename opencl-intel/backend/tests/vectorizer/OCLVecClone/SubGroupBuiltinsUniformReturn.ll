; RUN: %oclopt --ocl-vecclone  --ocl-vec-clone-isa-encoding-override=AVX512Core < %s -S -o - | FileCheck %s

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "x86_64-unknown-linux-gnu"

; CHECK-NOT: opencl-vec-uniform-return
; CHECK: declare spir_func i32 @_Z13sub_group_alli(i32) local_unnamed_addr [[ATTR:#[0-9]*]]
; CHECK: declare spir_func i32 @_Z13sub_group_anyi(i32) local_unnamed_addr [[ATTR]]
; CHECK: declare spir_func i8 @_Z19sub_group_broadcastcj(i8, i32) local_unnamed_addr [[ATTR]]
; CHECK: declare spir_func i8 @_Z20sub_group_reduce_addc(i8) local_unnamed_addr [[ATTR]]
; CHECK: declare spir_func i8 @_Z20sub_group_reduce_minc(i8) local_unnamed_addr [[ATTR]]
; CHECK: declare spir_func i8 @_Z20sub_group_reduce_maxc(i8) local_unnamed_addr [[ATTR]]
; CHECK-NOT: opencl-vec-uniform-return
; CHECK: attributes [[ATTR]] = { {{.*}}opencl-vec-uniform-return{{.*}} }
; CHECK-NOT: opencl-vec-uniform-return


define spir_kernel void @a(i32 addrspace(1)* nocapture readonly %a, i32 addrspace(1)* nocapture %b) local_unnamed_addr #0 !kernel_arg_addr_space !4 !kernel_arg_access_qual !5 !kernel_arg_type !6 !kernel_arg_base_type !6 !kernel_arg_type_qual !7 !kernel_arg_host_accessible !8 !kernel_arg_pipe_depth !9 !kernel_arg_pipe_io !7 !kernel_arg_buffer_location !7 !ocl_recommended_vector_length !15 {
entry:
  %call = tail call spir_func i64 @_Z13get_global_idj(i32 0) #3
  %slid = tail call i32 @_Z22get_sub_group_local_idv() #3
  %slid.reverse = sub nuw i32 16, %slid
  %arrayidx = getelementptr inbounds i32, i32 addrspace(1)* %a, i64 %call
  %0 = load i32, i32 addrspace(1)* %arrayidx, align 4, !tbaa !10

  %val = call i32 @_Z23intel_sub_group_shuffleij(i32 %0, i32 %slid.reverse)
  %call1 = tail call spir_func i32 @_Z13sub_group_alli(i32 %0) #4
  %call2 = tail call spir_func i32 @_Z13sub_group_anyi(i32 %0) #4
  %call3 = tail call spir_func i32 @_Z16get_sub_group_idv() #4

  %1 = trunc i32 %0 to i8
  %call4 = tail call spir_func i8 @_Z19sub_group_broadcastcj(i8 %1, i32 0) #4
  %call5 = tail call spir_func i8 @_Z20sub_group_reduce_addc(i8 %1) #4
  %call6 = tail call spir_func i8 @_Z20sub_group_reduce_minc(i8 %1) #4
  %call7 = tail call spir_func i8 @_Z20sub_group_reduce_maxc(i8 %1) #4

  %mul = mul i32 %call3, 1000
  %conv = zext i32 %mul to i64
  %add = add i64 %call, %conv
  %arrayidx4 = getelementptr inbounds i32, i32 addrspace(1)* %b, i64 %add
  store i32 %call1, i32 addrspace(1)* %arrayidx4, align 4, !tbaa !10

  ret void
}

; Function Attrs: convergent
declare spir_func i32 @_Z13sub_group_alli(i32) local_unnamed_addr #1
declare spir_func i32 @_Z13sub_group_anyi(i32) local_unnamed_addr #1

; Function Attrs: convergent
declare spir_func i8 @_Z19sub_group_broadcastcj(i8, i32) local_unnamed_addr #1
declare spir_func i8 @_Z20sub_group_reduce_addc(i8) local_unnamed_addr #1
declare spir_func i8 @_Z20sub_group_reduce_minc(i8) local_unnamed_addr #1
declare spir_func i8 @_Z20sub_group_reduce_maxc(i8) local_unnamed_addr #1

; Function Attrs: convergent nounwind readnone
declare spir_func i64 @_Z13get_global_idj(i32) local_unnamed_addr #2

; Function Attrs: convergent
declare spir_func i32 @_Z16get_sub_group_idv() local_unnamed_addr #1

declare spir_func i32 @_Z23intel_sub_group_shuffleij(i32, i32) local_unnamed_addr #1
declare spir_func i32 @_Z22get_sub_group_local_idv() local_unnamed_addr #1

attributes #0 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "uniform-work-group-size"="false" "unsafe-fp-math"="false" "use-soft-float"="false" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87"}
attributes #1 = { convergent "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false"  "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87"}
attributes #2 = { convergent nounwind readnone "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false"  "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87"}
attributes #3 = { convergent nounwind readnone }
attributes #4 = { convergent nounwind }

!llvm.module.flags = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!1}
!opencl.spir.version = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!3}

!opencl.kernels = !{!14}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 2, i32 0}
!2 = !{}
!3 = !{!"icx (ICX) dev.8.x.0"}
!4 = !{i32 1, i32 1}
!5 = !{!"none", !"none"}
!6 = !{!"int*", !"int*"}
!7 = !{!"", !""}
!8 = !{i1 false, i1 false}
!9 = !{i32 0, i32 0}
!10 = !{!11, !11, i64 0}
!11 = !{!"int", !12, i64 0}
!12 = !{!"omnipotent char", !13, i64 0}
!13 = !{!"Simple C/C++ TBAA"}
!14 = !{void (i32 addrspace(1)*, i32 addrspace(1)*)* @a}
!15 = !{i32 4}
