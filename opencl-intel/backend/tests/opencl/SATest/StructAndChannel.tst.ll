;; Compiled with:
;; -cc1 -emit-llvm -triple spir64-unknown-unknown-intelfpga -disable-llvm-passes -x cl -cl-std=CL1.2
;;
;; from:
;;
;; #if !defined(cl_intel_channels)
;; #error cl_intel_channels extension is not supported
;; #endif
;;
;; #pragma OPENCL EXTENSION cl_intel_channels : enable
;;
;; struct STRUCT {
;;   int VAL_1;
;;   int VAL_2;
;;   int VAL_3;
;; };
;;
;; channel int CHANNEL;
;;
;; __kernel void dummy_kernel(
;;      global struct STRUCT * ARG) {
;;   struct STRUCT value = {0, 0, 0};
;;   write_channel_intel(CHANNEL, value.VAL_1);
;; }
;;
;; The test checks if pipe built-in can be successfully imported with a struct
;; of 3 ints being declared.

; ModuleID = '/localdisk/sidorovd/xmain/ics-ws/xmain/llvm/projects/opencl/backend/tests/opencl/SATest/StructAndChannel.tst.cl'
source_filename = "/localdisk/sidorovd/xmain/ics-ws/xmain/llvm/projects/opencl/backend/tests/opencl/SATest/StructAndChannel.tst.cl"
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown-intelfpga"

%opencl.channel_t = type opaque
%struct.STRUCT = type { i32, i32, i32 }

@CHANNEL = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4, !packet_size !0, !packet_align !0

; Function Attrs: convergent nounwind
define spir_kernel void @dummy_kernel(%struct.STRUCT addrspace(1)* %ARG) #0 !kernel_arg_addr_space !5 !kernel_arg_access_qual !6 !kernel_arg_type !7 !kernel_arg_base_type !7 !kernel_arg_type_qual !8 !kernel_arg_host_accessible !9 !kernel_arg_pipe_depth !10 !kernel_arg_pipe_io !8 !kernel_arg_buffer_location !8 {
entry:
  %ARG.addr = alloca %struct.STRUCT addrspace(1)*, align 8
  %value = alloca %struct.STRUCT, align 4
  store %struct.STRUCT addrspace(1)* %ARG, %struct.STRUCT addrspace(1)** %ARG.addr, align 8, !tbaa !11
  %0 = bitcast %struct.STRUCT* %value to i8*
  call void @llvm.lifetime.start.p0i8(i64 12, i8* %0) #3
  %1 = bitcast %struct.STRUCT* %value to i8*
  call void @llvm.memset.p0i8.i64(i8* align 4 %1, i8 0, i64 12, i1 false)
  %2 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @CHANNEL, align 4, !tbaa !15
  %VAL_1 = getelementptr inbounds %struct.STRUCT, %struct.STRUCT* %value, i32 0, i32 0
  %3 = load i32, i32* %VAL_1, align 4, !tbaa !16
  call void @_Z19write_channel_intel11ocl_channelii(%opencl.channel_t addrspace(1)* %2, i32 %3) #4
  %4 = bitcast %struct.STRUCT* %value to i8*
  call void @llvm.lifetime.end.p0i8(i64 12, i8* %4) #3
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1) #1

; Function Attrs: convergent
declare void @_Z19write_channel_intel11ocl_channelii(%opencl.channel_t addrspace(1)*, i32) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

attributes #0 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "uniform-work-group-size"="true" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { convergent "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }
attributes #4 = { convergent }

!llvm.module.flags = !{!1}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!2}
!opencl.spir.version = !{!2}
!opencl.used.extensions = !{!3}
!opencl.used.optional.core.features = !{!3}
!opencl.compiler.options = !{!3}
!llvm.ident = !{!4}

!0 = !{i32 4}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 1, i32 2}
!3 = !{}
!4 = !{!"clang version 8.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 4197242e87d4f39394d9fe8a5f4398b115ba1b42) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm bc30aa5d0043536d74de287f2ab67360b4e90752)"}
!5 = !{i32 1}
!6 = !{!"none"}
!7 = !{!"struct STRUCT*"}
!8 = !{!""}
!9 = !{i1 false}
!10 = !{i32 0}
!11 = !{!12, !12, i64 0}
!12 = !{!"any pointer", !13, i64 0}
!13 = !{!"omnipotent char", !14, i64 0}
!14 = !{!"Simple C/C++ TBAA"}
!15 = !{!13, !13, i64 0}
!16 = !{!17, !18, i64 0}
!17 = !{!"STRUCT", !18, i64 0, !18, i64 4, !18, i64 8}
!18 = !{!"int", !13, i64 0}
