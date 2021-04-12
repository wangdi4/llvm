; RUN: %oclopt -debugify -resolve-block-call -check-debugify -add-implicit-args -S < %s -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: %oclopt -resolve-block-call -add-implicit-args -S < %s | FileCheck %s
;
; Regression test. Check if byval stuct paasing to block always have
; the same aligment on a call site and a callee.
;
; struct two_ints {
;       long x;
;       long y;
; };
; kernel void block_arg_struct()
; {
;     int (^kernelBlock)(struct two_ints) = ^int(struct two_ints ti)
;     {
;         return ti.x * ti.y;
;     };
;     struct two_ints i;
;     i.x = 2;
;     i.y = 3;
;     kernelBlock(i);
; }

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

%struct.two_ints = type { i64, i64 }
%struct.__opencl_block_literal_generic = type { i32, i32, i8 addrspace(4)* }

@__block_literal_global = internal addrspace(1) constant { i32, i32, i8 addrspace(4)* } { i32 16, i32 8, i8 addrspace(4)* addrspacecast (i8* bitcast (i32 (i8 addrspace(4)*, %struct.two_ints*)* @__block_arg_struct_block_invoke to i8*) to i8 addrspace(4)*) }, align 8

; Function Attrs: convergent nounwind
define void @block_arg_struct() #0 !kernel_arg_addr_space !2 !kernel_arg_access_qual !2 !kernel_arg_type !2 !kernel_arg_base_type !2 !kernel_arg_type_qual !2 !kernel_arg_host_accessible !2 !kernel_arg_pipe_depth !2 !kernel_arg_pipe_io !2 !kernel_arg_buffer_location !2 !kernel_arg_name !2 {
entry:
  %kernelBlock = alloca i32 (%struct.two_ints*) addrspace(4)*, align 8
  %i = alloca %struct.two_ints, align 8
  %0 = bitcast i32 (%struct.two_ints*) addrspace(4)** %kernelBlock to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %0) #3
  store i32 (%struct.two_ints*) addrspace(4)* addrspacecast (i32 (%struct.two_ints*) addrspace(1)* bitcast ({ i32, i32, i8 addrspace(4)* } addrspace(1)* @__block_literal_global to i32 (%struct.two_ints*) addrspace(1)*) to i32 (%struct.two_ints*) addrspace(4)*), i32 (%struct.two_ints*) addrspace(4)** %kernelBlock, align 8, !tbaa !6
  %1 = bitcast %struct.two_ints* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 16, i8* %1) #3
  %x = getelementptr inbounds %struct.two_ints, %struct.two_ints* %i, i32 0, i32 0
  store i64 2, i64* %x, align 8, !tbaa !9
  %y = getelementptr inbounds %struct.two_ints, %struct.two_ints* %i, i32 0, i32 1
  store i64 3, i64* %y, align 8, !tbaa !12
  %2 = load i8 addrspace(4)*, i8 addrspace(4)* addrspace(4)* getelementptr inbounds (%struct.__opencl_block_literal_generic, %struct.__opencl_block_literal_generic addrspace(4)* addrspacecast (%struct.__opencl_block_literal_generic addrspace(1)* bitcast ({ i32, i32, i8 addrspace(4)* } addrspace(1)* @__block_literal_global to %struct.__opencl_block_literal_generic addrspace(1)*) to %struct.__opencl_block_literal_generic addrspace(4)*), i32 0, i32 2), align 8
  %3 = addrspacecast i8 addrspace(4)* %2 to i32 (i8 addrspace(4)*, %struct.two_ints*)*
; CHECK: call i32 @__block_arg_struct_block_invoke
  %call = call i32 %3(i8 addrspace(4)* addrspacecast (i8 addrspace(1)* bitcast ({ i32, i32, i8 addrspace(4)* } addrspace(1)* @__block_literal_global to i8 addrspace(1)*) to i8 addrspace(4)*), %struct.two_ints* byval(%struct.two_ints) align 8 %i) #4
  %4 = bitcast %struct.two_ints* %i to i8*
  call void @llvm.lifetime.end.p0i8(i64 16, i8* %4) #3
  %5 = bitcast i32 (%struct.two_ints*) addrspace(4)** %kernelBlock to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %5) #3
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

; Function Attrs: convergent nounwind
; CHECK: define internal i32 @__block_arg_struct_block_invoke{{.*}} %struct.two_ints* byval(%struct.two_ints) align 8
define internal i32 @__block_arg_struct_block_invoke(i8 addrspace(4)* %.block_descriptor, %struct.two_ints* byval(%struct.two_ints) align 8 %ti) #2 {
entry:
  %.block_descriptor.addr = alloca i8 addrspace(4)*, align 8
  store i8 addrspace(4)* %.block_descriptor, i8 addrspace(4)** %.block_descriptor.addr, align 8
  %block = bitcast i8 addrspace(4)* %.block_descriptor to <{ i32, i32, i8 addrspace(4)* }> addrspace(4)*
  %x = getelementptr inbounds %struct.two_ints, %struct.two_ints* %ti, i32 0, i32 0
  %0 = load i64, i64* %x, align 8, !tbaa !9
  %y = getelementptr inbounds %struct.two_ints, %struct.two_ints* %ti, i32 0, i32 1
  %1 = load i64, i64* %y, align 8, !tbaa !12
  %mul = mul nsw i64 %0, %1
  %conv = trunc i64 %mul to i32
  ret i32 %conv
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
!5 = !{void ()* @block_arg_struct}
!6 = !{!7, !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C/C++ TBAA"}
!9 = !{!10, !11, i64 0}
!10 = !{!"two_ints", !11, i64 0, !11, i64 8}
!11 = !{!"long", !7, i64 0}
!12 = !{!10, !11, i64 8}


; DEBUGIFY-NOT: WARNING
