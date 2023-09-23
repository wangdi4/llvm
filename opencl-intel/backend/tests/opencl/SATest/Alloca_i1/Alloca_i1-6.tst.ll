; ModuleID = 'main'
source_filename = "1"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "spir64-unknown-unknown-intelfpga"

@.str = private unnamed_addr addrspace(2) constant [3 x i8] c"%d\00", align 1

; Function Attrs: convergent nounwind
define spir_kernel void @set_false(ptr addrspace(1) %data) #0 !kernel_arg_addr_space !6 !kernel_arg_access_qual !7 !kernel_arg_type !8 !kernel_arg_base_type !8 !kernel_arg_type_qual !9 !kernel_arg_host_accessible !10 !kernel_arg_pipe_depth !11 !kernel_arg_pipe_io !9 !kernel_arg_buffer_location !9 !kernel_arg_name !12 {
entry:
  %data.addr = alloca ptr addrspace(1), align 8
  %tmp = alloca [3 x i1], align 1
  %i = alloca i32, align 4
  store ptr addrspace(1) %data, ptr %data.addr, align 8, !tbaa !13
  %0 = bitcast  ptr%tmp to ptr
  call void @llvm.lifetime.start.p0i8(i64 3, ptr %0) #3
  %1 = bitcast ptr %i to ptr
  call void @llvm.lifetime.start.p0i8(i64 4, ptr %1) #3
  store i32 0, ptr %i, align 4, !tbaa !17
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %2 = load i32, ptr %i, align 4, !tbaa !17
  %cmp = icmp slt i32 %2, 3
  br i1 %cmp, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  %3 = bitcast ptr %i to ptr
  call void @llvm.lifetime.end.p0i8(i64 4, ptr %3) #3
  br label %for.end

for.body:                                         ; preds = %for.cond
  %4 = load i32, ptr %i, align 4, !tbaa !17
  %idxprom = sext i32 %4 to i64
  %arrayidx = getelementptr inbounds [3 x i1],  ptr%tmp, i64 0, i64 %idxprom
  store i1 0, ptr %arrayidx, align 1, !tbaa !19
  %5 = load ptr addrspace(1), ptr %data.addr, align 8, !tbaa !13
  %6 = load i32, ptr %i, align 4, !tbaa !17
  %idxprom1 = sext i32 %6 to i64
  %arrayidx2 = getelementptr inbounds i1, ptr addrspace(1) %5, i64 %idxprom1
  %7 = load i1, ptr addrspace(1) %arrayidx2, align 1, !tbaa !19
  %conv = zext i1 %7 to i32
  %cmp3 = icmp ne i32 %conv, 0
  br i1 %cmp3, label %if.then, label %if.end

if.then:                                          ; preds = %for.body
  %8 = load ptr addrspace(1), ptr %data.addr, align 8, !tbaa !13
  %9 = load i32, ptr %i, align 4, !tbaa !17
  %idxprom5 = sext i32 %9 to i64
  %arrayidx6 = getelementptr inbounds i1, ptr addrspace(1) %8, i64 %idxprom5
  %10 = load i1, ptr addrspace(1) %arrayidx6, align 1, !tbaa !19
  %call = call i32 (ptr addrspace(2), ...) @printf(ptr addrspace(2) getelementptr inbounds ([3 x i8], ptr addrspace(2) @.str, i32 0, i32 0), i1 %10) #4
  %arrayidx9 = getelementptr inbounds [3 x i1],  ptr%tmp, i64 0, i64 0
  %11 = load i1, ptr %arrayidx9, align 1, !tbaa !19
  %12 = load ptr addrspace(1), ptr %data.addr, align 8, !tbaa !13
  %13 = load i32, ptr %i, align 4, !tbaa !17
  %idxprom11 = sext i32 %13 to i64
  %arrayidx12 = getelementptr inbounds i1, ptr addrspace(1) %12, i64 %idxprom11
  store i1 %10, ptr addrspace(1) %arrayidx12, align 1, !tbaa !19
  br label %if.end

if.end:                                           ; preds = %if.then, %for.body
  br label %for.inc

for.inc:                                          ; preds = %if.end
  %14 = load i32, ptr %i, align 4, !tbaa !17
  %inc = add nsw i32 %14, 1
  store i32 %inc, ptr %i, align 4, !tbaa !17
  br label %for.cond

for.end:                                          ; preds = %for.cond.cleanup
  %15 = bitcast  ptr%tmp to ptr
  call void @llvm.lifetime.end.p0i8(i64 3, ptr %15) #3
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
!8 = !{!"bool*"}
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
!19 = !{!20, !20, i64 0}
!20 = !{!"bool", !15, i64 0}
