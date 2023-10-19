; This test checks channel write functions transformation when type-coerced struct that contains pointer member is passed as argument.

; ----------------------------------------------------
; Compiled from:
; ----------------------------------------------------
; struct TS {
;   int *p;
;   char t[4];
; };
; #pragma OPENCL EXTENSION cl_intel_channels : enable
; channel struct TS chan;
; kernel void writer(__global struct TS *src) {
;   write_channel_intel(chan, *src);
; }
; ----------------------------------------------------

; RUN: opt -sycl-kernel-builtin-lib=%p/../Inputs/fpga-pipes.rtl -passes=sycl-kernel-channel-pipe-transformation -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -sycl-kernel-builtin-lib=%p/../Inputs/fpga-pipes.rtl -passes=sycl-kernel-channel-pipe-transformation -S %s -o - | FileCheck %s

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64-unknown-unknown-intelfpga"

%struct.TS = type { ptr addrspace(4), [4 x i8] }
%struct.TS.coerce = type { ptr addrspace(4), i64 }

@chan = addrspace(1) global ptr addrspace(1) null, align 8, !packet_size !0, !packet_align !1

; CHECK: %struct.channelpipetransformation.merge = type { ptr addrspace(4), i64 }

; Function Attrs: convergent norecurse nounwind
define dso_local void @writer(ptr addrspace(1) noundef align 8 %src) #0 !kernel_arg_addr_space !6 !kernel_arg_access_qual !7 !kernel_arg_type !8 !kernel_arg_base_type !8 !kernel_arg_type_qual !9 !kernel_arg_host_accessible !10 !kernel_arg_pipe_depth !11 !kernel_arg_pipe_io !9 !kernel_arg_buffer_location !9 !arg_type_null_val !12 {
entry:
  %src.addr = alloca ptr addrspace(1), align 8
  %byval-temp = alloca %struct.TS, align 8
  store ptr addrspace(1) %src, ptr %src.addr, align 8, !tbaa !13
  %0 = load ptr addrspace(1), ptr addrspace(1) @chan, align 8, !tbaa !17
  %1 = load ptr addrspace(1), ptr %src.addr, align 8, !tbaa !13
  call void @llvm.lifetime.start.p0(i64 16, ptr %byval-temp) #4
  call void @llvm.memcpy.p0.p1.i64(ptr align 8 %byval-temp, ptr addrspace(1) align 8 %1, i64 16, i1 false), !tbaa.struct !18
  %2 = getelementptr %struct.TS.coerce, ptr %byval-temp, i32 0, i32 0
  %3 = load ptr addrspace(4), ptr %2, align 8
  %4 = getelementptr %struct.TS.coerce, ptr %byval-temp, i32 0, i32 1
  %5 = load i64, ptr %4, align 8
  ; CHECK: [[GEP1:%[a-zA-Z0-9]+]] = getelementptr %struct.channelpipetransformation.merge, ptr %write.src, i32 0, i32 0
  ; CHECK-NEXT: store ptr addrspace(4) [[SRC1:%[a-zA-Z0-9]+]], ptr [[GEP1]]
  ; CHECK-NEXT: [[GEP2:%[a-zA-Z0-9]+]] = getelementptr %struct.channelpipetransformation.merge, ptr %write.src, i32 0, i32 1
  ; CHECK-NEXT: store i64 [[SRC2:%[a-zA-Z0-9]+]], ptr [[GEP2]]
  ; CHECK-NEXT: addrspacecast ptr %write.src to
  call void @_Z19write_channel_intel11ocl_channel2TSS_(ptr addrspace(1) %0, ptr addrspace(4) %3, i64 %5)
  call void @llvm.lifetime.end.p0(i64 16, ptr %byval-temp) #4
  ret void
}

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p1.i64(ptr noalias nocapture writeonly, ptr addrspace(1) noalias nocapture readonly, i64, i1 immarg) #2

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: convergent nounwind
declare void @___Z19write_channel_intel11ocl_channel2TSS__before.CoerceTypes(ptr addrspace(1), ptr noundef byval(%struct.TS) align 8) #3

; Function Attrs: convergent nounwind
declare void @_Z19write_channel_intel11ocl_channel2TSS_(ptr addrspace(1), ptr addrspace(4), i64) #3

attributes #0 = { convergent norecurse nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "uniform-work-group-size"="false" }
attributes #1 = { nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #2 = { nocallback nofree nounwind willreturn memory(argmem: readwrite) }
attributes #3 = { convergent nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" }
attributes #4 = { nounwind }

!opencl.ocl.version = !{!2}
!opencl.spir.version = !{!2}
!opencl.compiler.options = !{!3}
!llvm.ident = !{!4}
!sycl.kernels = !{!5}

!0 = !{i32 16}
!1 = !{i32 8}
!2 = !{i32 2, i32 0}
!3 = !{}
!4 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2024.0.0 (2024.x.0.YYYYMMDD)"}
!5 = !{ptr @writer}
!6 = !{i32 1}
!7 = !{!"none"}
!8 = !{!"struct TS*"}
!9 = !{!""}
!10 = !{i1 false}
!11 = !{i32 0}
!12 = !{ptr addrspace(1) null}
!13 = !{!14, !14, i64 0}
!14 = !{!"any pointer", !15, i64 0}
!15 = !{!"omnipotent char", !16, i64 0}
!16 = !{!"Simple C/C++ TBAA"}
!17 = !{!15, !15, i64 0}
!18 = !{i64 0, i64 8, !13, i64 8, i64 4, !17}

; DEBUGIFY-NOT: WARNING: Missing line
