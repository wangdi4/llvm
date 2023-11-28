; ModuleID = 'main'
source_filename = "1"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "spir64-unknown-unknown-intelfpga"

; Function Attrs: convergent nounwind
define spir_kernel void @set_zero(ptr addrspace(1) %data) #0 !kernel_arg_addr_space !6 !kernel_arg_access_qual !7 !kernel_arg_type !8 !kernel_arg_base_type !8 !kernel_arg_type_qual !9 !kernel_arg_host_accessible !10 !kernel_arg_name !11 {
entry:
  %data.addr = alloca ptr addrspace(1), align 32
  %i = alloca i32, align 4
  store ptr addrspace(1) %data, ptr %data.addr, align 32, !tbaa !12
  %0 = bitcast ptr %i to ptr
  call void @llvm.lifetime.start.p0i8(i64 4, ptr %0) #2
  store i32 0, ptr %i, align 4, !tbaa !16
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %1 = load i32, ptr %i, align 4, !tbaa !16
  %cmp = icmp slt i32 %1, 4
  br i1 %cmp, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  %2 = bitcast ptr %i to ptr
  call void @llvm.lifetime.end.p0i8(i64 4, ptr %2) #2
  br label %for.end

for.body:                                         ; preds = %for.cond
  %3 = load ptr addrspace(1), ptr %data.addr, align 32, !tbaa !12
  %4 = load i32, ptr %i, align 4, !tbaa !16
  %idxprom = sext i32 %4 to i64
  %arrayidx = getelementptr inbounds i239, ptr addrspace(1) %3, i64 %idxprom
  store i239 0, ptr addrspace(1) %arrayidx, align 32, !tbaa !18
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %5 = load i32, ptr %i, align 4, !tbaa !16
  %inc = add nsw i32 %5, 1
  store i32 %inc, ptr %i, align 4, !tbaa !16
  br label %for.cond

for.end:                                          ; preds = %for.cond.cleanup
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, ptr nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, ptr nocapture) #1

attributes #0 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind }

!llvm.linker.options = !{}
!llvm.module.flags = !{!0}
!opencl.ocl.version = !{!1}
!opencl.spir.version = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, i32 2}
!2 = !{}
!3 = !{!"-Idevice/"}
!4 = !{!"clang version 6.0.0"}
!5 = !{ptr @set_zero}
!6 = !{i32 1}
!7 = !{!"none"}
!8 = !{!"i239*"}
!9 = !{!""}
!10 = !{i1 false}
!11 = !{!"data"}
!12 = !{!13, !13, i64 0}
!13 = !{!"any pointer", !14, i64 0}
!14 = !{!"omnipotent char", !15, i64 0}
!15 = !{!"Simple C/C++ TBAA"}
!16 = !{!17, !17, i64 0}
!17 = !{!"int", !14, i64 0}
!18 = !{!19, !19, i64 0}
!19 = !{!"i239", !14, i64 0}
