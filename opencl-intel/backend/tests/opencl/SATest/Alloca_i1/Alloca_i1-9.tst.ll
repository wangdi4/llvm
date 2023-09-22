; ModuleID = 'main'
source_filename = "1"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "spir64-unknown-unknown-intelfpga"

@.str = private unnamed_addr addrspace(2) constant [3 x i8] c"%d\00", align 1

; Function Attrs: convergent nounwind
define spir_kernel void @set_false(ptr addrspace(1) %data) #0 !kernel_arg_addr_space !6 !kernel_arg_access_qual !7 !kernel_arg_type !8 !kernel_arg_base_type !8 !kernel_arg_type_qual !9 !kernel_arg_host_accessible !10 !kernel_arg_pipe_depth !11 !kernel_arg_pipe_io !9 !kernel_arg_buffer_location !9 !kernel_arg_name !12 {
entry:
  %data.addr = alloca ptr addrspace(1), align 8
  %i = alloca i32, align 4
  store ptr addrspace(1) %data, ptr %data.addr, align 8, !tbaa !13
  %0 = bitcast ptr %i to ptr
  call void @llvm.lifetime.start.p0i8(i64 4, ptr %0) #3
  store i32 0, ptr %i, align 4, !tbaa !17
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %1 = load i32, ptr %i, align 4, !tbaa !17
  %cmp = icmp slt i32 %1, 3
  br i1 %cmp, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  %2 = bitcast ptr %i to ptr
  call void @llvm.lifetime.end.p0i8(i64 4, ptr %2) #3
  br label %for.end

for.body:                                         ; preds = %for.cond
  %3 = load ptr addrspace(1), ptr %data.addr, align 8, !tbaa !13
  %4 = load i32, ptr %i, align 4, !tbaa !17
  %idx.ext = sext i32 %4 to i64
  %add.ptr = getelementptr inbounds i1, ptr addrspace(1) %3, i64 %idx.ext
  %5 = load i1, ptr addrspace(1) %add.ptr, align 8, !tbaa !19
  %cmp1 = icmp ne i1 %5, 0
  br i1 %cmp1, label %if.then, label %if.end

if.then:                                          ; preds = %for.body
  %6 = load ptr addrspace(1), ptr %data.addr, align 8, !tbaa !13
  %7 = load i32, ptr %i, align 4, !tbaa !17
  %idx.ext2 = sext i32 %7 to i64
  %add.ptr3 = getelementptr inbounds i1, ptr addrspace(1) %6, i64 %idx.ext2
  %8 = load i1, ptr addrspace(1) %add.ptr3, align 8, !tbaa !19
  %call = call i32 (ptr addrspace(2), ...) @printf(ptr addrspace(2) getelementptr inbounds ([3 x i8], ptr addrspace(2) @.str, i32 0, i32 0), i1 %8) #4
  %9 = load ptr addrspace(1), ptr %data.addr, align 8, !tbaa !13
  %10 = load i32, ptr %i, align 4, !tbaa !17
  %idx.ext4 = sext i32 %10 to i64
  %add.ptr5 = getelementptr inbounds i1, ptr addrspace(1) %9, i64 %idx.ext4
  store i1 0, ptr addrspace(1) %add.ptr5, align 8, !tbaa !19
  br label %if.end

if.end:                                           ; preds = %if.then, %for.body
  br label %for.inc

for.inc:                                          ; preds = %if.end
  %11 = load i32, ptr %i, align 4, !tbaa !17
  %inc = add nsw i32 %11, 1
  store i32 %inc, ptr %i, align 4, !tbaa !17
  br label %for.cond

for.end:                                          ; preds = %for.cond.cleanup
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, ptr nocapture) #1

; Function Attrs: convergent
declare i32 @printf(ptr addrspace(2), ...) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, ptr nocapture) #1

attributes #0 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { convergent "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }
attributes #4 = { convergent }

!llvm.linker.options = !{}
!llvm.module.flags = !{!0}
!opencl.enable.FP_CONTRACT = !{}
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
!5 = !{ptr @set_false}
!6 = !{i32 1}
!7 = !{!"none"}
!8 = !{!"long*"}
!9 = !{!""}
!10 = !{i1 false}
!11 = !{i32 0}
!12 = !{!"data"}
!13 = !{!14, !14, i64 0}
!14 = !{!"any pointer", !15, i64 0}
!15 = !{!"omnipotent char", !16, i64 0}
!16 = !{!"Simple C/C++ TBAA"}
!17 = !{!18, !18, i64 0}
!18 = !{!"int", !15, i64 0}
!19 = !{!20, !20, i1 0}
!20 = !{!"bool", !15, i1 0}
