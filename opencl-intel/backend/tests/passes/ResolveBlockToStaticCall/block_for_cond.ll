; RUN: %oclopt -resolve-block-call -S < %s | FileCheck %s
;
; kernel void block_for_cond(__global int* res)
; {
;   int multiplier = 3;
;   int (^kernelBlock)(int) = ^(int num)
;   {
;     return num * multiplier;
;   };
;   int tid = get_global_id(0);
;   res[tid] = 39;
;   for(int i=0; i<kernelBlock(13); i++)
;   {
;     res[tid]--;
;   }
; }
;

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

%struct.__opencl_block_literal_generic = type { i32, i32, i8 addrspace(4)* }

; Function Attrs: convergent nounwind
define void @block_for_cond(i32 addrspace(1)* %res) #0 !kernel_arg_addr_space !6 !kernel_arg_access_qual !7 !kernel_arg_type !8 !kernel_arg_base_type !8 !kernel_arg_type_qual !9 !kernel_arg_host_accessible !10 !kernel_arg_pipe_depth !11 !kernel_arg_pipe_io !9 !kernel_arg_buffer_location !9 !kernel_arg_name !12 {
entry:
  %res.addr = alloca i32 addrspace(1)*, align 8
  %multiplier = alloca i32, align 4
  %kernelBlock = alloca %struct.__opencl_block_literal_generic addrspace(4)*, align 8
  %block = alloca <{ i32, i32, i8 addrspace(4)*, i32 }>, align 8
  %tid = alloca i32, align 4
  %i = alloca i32, align 4
  store i32 addrspace(1)* %res, i32 addrspace(1)** %res.addr, align 8, !tbaa !13
  %0 = bitcast i32* %multiplier to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #4
  store i32 3, i32* %multiplier, align 4, !tbaa !17
  %1 = bitcast %struct.__opencl_block_literal_generic addrspace(4)** %kernelBlock to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %1) #4
  %block.size = getelementptr inbounds <{ i32, i32, i8 addrspace(4)*, i32 }>, <{ i32, i32, i8 addrspace(4)*, i32 }>* %block, i32 0, i32 0
  store i32 20, i32* %block.size, align 8
  %block.align = getelementptr inbounds <{ i32, i32, i8 addrspace(4)*, i32 }>, <{ i32, i32, i8 addrspace(4)*, i32 }>* %block, i32 0, i32 1
  store i32 8, i32* %block.align, align 4
  %block.invoke = getelementptr inbounds <{ i32, i32, i8 addrspace(4)*, i32 }>, <{ i32, i32, i8 addrspace(4)*, i32 }>* %block, i32 0, i32 2
  store i8 addrspace(4)* addrspacecast (i8* bitcast (i32 (i8 addrspace(4)*, i32)* @__block_for_cond_block_invoke to i8*) to i8 addrspace(4)*), i8 addrspace(4)** %block.invoke, align 8
  %block.captured = getelementptr inbounds <{ i32, i32, i8 addrspace(4)*, i32 }>, <{ i32, i32, i8 addrspace(4)*, i32 }>* %block, i32 0, i32 3
  %2 = load i32, i32* %multiplier, align 4, !tbaa !17
  store i32 %2, i32* %block.captured, align 8, !tbaa !17
  %3 = bitcast <{ i32, i32, i8 addrspace(4)*, i32 }>* %block to %struct.__opencl_block_literal_generic*
  %4 = addrspacecast %struct.__opencl_block_literal_generic* %3 to %struct.__opencl_block_literal_generic addrspace(4)*
  store %struct.__opencl_block_literal_generic addrspace(4)* %4, %struct.__opencl_block_literal_generic addrspace(4)** %kernelBlock, align 8, !tbaa !19
  %5 = bitcast i32* %tid to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %5) #4
  %call = call i64 @_Z13get_global_idj(i32 0) #5
  %conv = trunc i64 %call to i32
  store i32 %conv, i32* %tid, align 4, !tbaa !17
  %6 = load i32 addrspace(1)*, i32 addrspace(1)** %res.addr, align 8, !tbaa !13
  %7 = load i32, i32* %tid, align 4, !tbaa !17
  %idxprom = sext i32 %7 to i64
  %arrayidx = getelementptr inbounds i32, i32 addrspace(1)* %6, i64 %idxprom
  store i32 39, i32 addrspace(1)* %arrayidx, align 4, !tbaa !17
  %8 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %8) #4
  store i32 0, i32* %i, align 4, !tbaa !17
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %9 = load i32, i32* %i, align 4, !tbaa !17
  %10 = load %struct.__opencl_block_literal_generic addrspace(4)*, %struct.__opencl_block_literal_generic addrspace(4)** %kernelBlock, align 8, !tbaa !19
  %11 = getelementptr inbounds %struct.__opencl_block_literal_generic, %struct.__opencl_block_literal_generic addrspace(4)* %10, i32 0, i32 2
  %12 = bitcast %struct.__opencl_block_literal_generic addrspace(4)* %10 to i8 addrspace(4)*
  %13 = load i8 addrspace(4)*, i8 addrspace(4)* addrspace(4)* %11, align 8
  %14 = addrspacecast i8 addrspace(4)* %13 to i32 (i8 addrspace(4)*, i32)*
; CHECK: call i32 @__block_for_cond_block_invoke
; CHECK-NOT: call i32 %
  %call1 = call i32 %14(i8 addrspace(4)* %12, i32 13) #6
  %cmp = icmp slt i32 %9, %call1
  br i1 %cmp, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  %15 = bitcast i32* %i to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %15) #4
  br label %for.end

for.body:                                         ; preds = %for.cond
  %16 = load i32 addrspace(1)*, i32 addrspace(1)** %res.addr, align 8, !tbaa !13
  %17 = load i32, i32* %tid, align 4, !tbaa !17
  %idxprom3 = sext i32 %17 to i64
  %arrayidx4 = getelementptr inbounds i32, i32 addrspace(1)* %16, i64 %idxprom3
  %18 = load i32, i32 addrspace(1)* %arrayidx4, align 4, !tbaa !17
  %dec = add nsw i32 %18, -1
  store i32 %dec, i32 addrspace(1)* %arrayidx4, align 4, !tbaa !17
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %19 = load i32, i32* %i, align 4, !tbaa !17
  %inc = add nsw i32 %19, 1
  store i32 %inc, i32* %i, align 4, !tbaa !17
  br label %for.cond

for.end:                                          ; preds = %for.cond.cleanup
  %20 = bitcast i32* %tid to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %20) #4
  %21 = bitcast %struct.__opencl_block_literal_generic addrspace(4)** %kernelBlock to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %21) #4
  %22 = bitcast i32* %multiplier to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %22) #4
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

; Function Attrs: convergent nounwind
define internal i32 @__block_for_cond_block_invoke(i8 addrspace(4)* %.block_descriptor, i32 %num) #2 {
entry:
  %.block_descriptor.addr = alloca i8 addrspace(4)*, align 8
  %num.addr = alloca i32, align 4
  store i8 addrspace(4)* %.block_descriptor, i8 addrspace(4)** %.block_descriptor.addr, align 8
  %block = bitcast i8 addrspace(4)* %.block_descriptor to <{ i32, i32, i8 addrspace(4)*, i32 }> addrspace(4)*
  store i32 %num, i32* %num.addr, align 4, !tbaa !17
  %0 = load i32, i32* %num.addr, align 4, !tbaa !17
  %block.capture.addr = getelementptr inbounds <{ i32, i32, i8 addrspace(4)*, i32 }>, <{ i32, i32, i8 addrspace(4)*, i32 }> addrspace(4)* %block, i32 0, i32 3
  %1 = load i32, i32 addrspace(4)* %block.capture.addr, align 8, !tbaa !17
  %mul = mul nsw i32 %0, %1
  ret i32 %mul
}

; Function Attrs: convergent nounwind readnone
declare i64 @_Z13get_global_idj(i32) #3

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

attributes #0 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { convergent nounwind readnone "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { nounwind }
attributes #5 = { convergent nounwind readnone }
attributes #6 = { convergent }

!llvm.linker.options = !{}
!llvm.module.flags = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!1}
!opencl.spir.version = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!3}
!llvm.ident = !{!4}
!opencl.kernels = !{!5}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 2, i32 0}
!2 = !{}
!3 = !{!"-cl-std=CL2.0"}
!4 = !{!"icx (ICX) 2019.8.2.0"}
!5 = !{void (i32 addrspace(1)*)* @block_for_cond}
!6 = !{i32 1}
!7 = !{!"none"}
!8 = !{!"int*"}
!9 = !{!""}
!10 = !{i1 false}
!11 = !{i32 0}
!12 = !{!"res"}
!13 = !{!14, !14, i64 0}
!14 = !{!"any pointer", !15, i64 0}
!15 = !{!"omnipotent char", !16, i64 0}
!16 = !{!"Simple C/C++ TBAA"}
!17 = !{!18, !18, i64 0}
!18 = !{!"int", !15, i64 0}
!19 = !{!15, !15, i64 0}

