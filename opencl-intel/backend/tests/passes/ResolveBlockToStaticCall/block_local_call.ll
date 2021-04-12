; RUN: %oclopt -resolve-block-call -S < %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: %oclopt -resolve-block-call -S < %s | FileCheck %s
;
;kernel void global_scope(__global int* res)
;{
;int (^globalBlock)(int) = ^int(int num)
;{
;   return 1;
;};
;
;*res = globalBlock(*res);
;}
;

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

%struct.__opencl_block_literal_generic = type { i32, i32, i8 addrspace(4)* }

@__block_literal_global = internal addrspace(1) constant { i32, i32, i8 addrspace(4)* } { i32 16, i32 8, i8 addrspace(4)* addrspacecast (i8* bitcast (i32 (i8 addrspace(4)*, i32)* @__global_scope_block_invoke to i8*) to i8 addrspace(4)*) }, align 8

; Function Attrs: convergent nounwind
define void @global_scope(i32 addrspace(1)* %res) #0 !kernel_arg_addr_space !6 !kernel_arg_access_qual !7 !kernel_arg_type !8 !kernel_arg_base_type !8 !kernel_arg_type_qual !9 !kernel_arg_host_accessible !10 !kernel_arg_pipe_depth !11 !kernel_arg_pipe_io !9 !kernel_arg_buffer_location !9 !kernel_arg_name !12 {
entry:
  %res.addr = alloca i32 addrspace(1)*, align 8
  %globalBlock = alloca i32 (i32) addrspace(4)*, align 8
  store i32 addrspace(1)* %res, i32 addrspace(1)** %res.addr, align 8, !tbaa !13
  %0 = bitcast i32 (i32) addrspace(4)** %globalBlock to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %0) #3
  store i32 (i32) addrspace(4)* addrspacecast (i32 (i32) addrspace(1)* bitcast ({ i32, i32, i8 addrspace(4)* } addrspace(1)* @__block_literal_global to i32 (i32) addrspace(1)*) to i32 (i32) addrspace(4)*), i32 (i32) addrspace(4)** %globalBlock, align 8, !tbaa !17
  %1 = load i32 addrspace(1)*, i32 addrspace(1)** %res.addr, align 8, !tbaa !13
  %2 = load i32, i32 addrspace(1)* %1, align 4, !tbaa !18
  %3 = load i8 addrspace(4)*, i8 addrspace(4)* addrspace(4)* getelementptr inbounds (%struct.__opencl_block_literal_generic, %struct.__opencl_block_literal_generic addrspace(4)* addrspacecast (%struct.__opencl_block_literal_generic addrspace(1)* bitcast ({ i32, i32, i8 addrspace(4)* } addrspace(1)* @__block_literal_global to %struct.__opencl_block_literal_generic addrspace(1)*) to %struct.__opencl_block_literal_generic addrspace(4)*), i32 0, i32 2), align 8
  %4 = addrspacecast i8 addrspace(4)* %3 to i32 (i8 addrspace(4)*, i32)*
; CHECK: call {{.*}} @__global_scope_block_invoke
; CHECK-NOT: call i32 %
  %call = call i32 %4(i8 addrspace(4)* addrspacecast (i8 addrspace(1)* bitcast ({ i32, i32, i8 addrspace(4)* } addrspace(1)* @__block_literal_global to i8 addrspace(1)*) to i8 addrspace(4)*), i32 %2) #4
  %5 = load i32 addrspace(1)*, i32 addrspace(1)** %res.addr, align 8, !tbaa !13
  store i32 %call, i32 addrspace(1)* %5, align 4, !tbaa !18
  %6 = bitcast i32 (i32) addrspace(4)** %globalBlock to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %6) #3
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

; Function Attrs: convergent nounwind
define internal i32 @__global_scope_block_invoke(i8 addrspace(4)* %.block_descriptor, i32 %num) #2 {
entry:
  %.block_descriptor.addr = alloca i8 addrspace(4)*, align 8
  %num.addr = alloca i32, align 4
  store i8 addrspace(4)* %.block_descriptor, i8 addrspace(4)** %.block_descriptor.addr, align 8
  %block = bitcast i8 addrspace(4)* %.block_descriptor to <{ i32, i32, i8 addrspace(4)* }> addrspace(4)*
  store i32 %num, i32* %num.addr, align 4, !tbaa !18
  ret i32 1
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

attributes #0 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
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
!opencl.kernels = !{!5}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 2, i32 0}
!2 = !{}
!3 = !{!"-cl-std=CL2.0"}
!4 = !{!"clang version 8.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang e272dc2059c2545d68f5a501e9bc2cc38138c7d3) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm bf01f1e4d64ee71eb4d0627ef0f9dda9273e65af)"}
!5 = !{void (i32 addrspace(1)*)* @global_scope}
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
!17 = !{!15, !15, i64 0}
!18 = !{!19, !19, i64 0}
!19 = !{!"int", !15, i64 0}


; DEBUGIFY-NOT: WARNING
