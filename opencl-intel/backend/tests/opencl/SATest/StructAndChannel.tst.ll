;; Compiled with:
;; clang -cc1 -triple spir64 -x cl -cl-std=cl2.0 -finclude-default-header -disable-llvm-passes -emit-llvm -o -
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

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"

%struct.STRUCT = type { i32, i32, i32 }

@CHANNEL = addrspace(1) global target("spirv.Channel") zeroinitializer, align 8, !packet_size !0, !packet_align !0

; Function Attrs: convergent norecurse nounwind
define dso_local spir_kernel void @dummy_kernel(ptr addrspace(1) noundef align 4 %ARG) #0 !kernel_arg_addr_space !3 !kernel_arg_access_qual !4 !kernel_arg_type !5 !kernel_arg_base_type !5 !kernel_arg_type_qual !6 !kernel_arg_host_accessible !7 !kernel_arg_pipe_depth !8 !kernel_arg_pipe_io !6 !kernel_arg_buffer_location !6 {
entry:
  %ARG.addr = alloca ptr addrspace(1), align 8
  %value = alloca %struct.STRUCT, align 4
  store ptr addrspace(1) %ARG, ptr %ARG.addr, align 8, !tbaa !9
  call void @llvm.lifetime.start.p0(i64 12, ptr %value) #4
  call void @llvm.memset.p0.i64(ptr align 4 %value, i8 0, i64 12, i1 false)
  %0 = load target("spirv.Channel"), ptr addrspace(1) @CHANNEL, align 8, !tbaa !13
  %VAL_1 = getelementptr inbounds %struct.STRUCT, ptr %value, i32 0, i32 0
  %1 = load i32, ptr %VAL_1, align 4, !tbaa !14
  call void @_Z19write_channel_intel11ocl_channelii(target("spirv.Channel") %0, i32 noundef %1) #5
  call void @llvm.lifetime.end.p0(i64 12, ptr %value) #4
  ret void
}

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: write)
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #2

; Function Attrs: convergent nounwind
declare void @_Z19write_channel_intel11ocl_channelii(target("spirv.Channel"), i32 noundef) #3

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

attributes #0 = { convergent norecurse nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "uniform-work-group-size"="false" }
attributes #1 = { nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #2 = { nocallback nofree nounwind willreturn memory(argmem: write) }
attributes #3 = { convergent nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" }
attributes #4 = { nounwind }
attributes #5 = { convergent nounwind }

!opencl.ocl.version = !{!1}
!opencl.spir.version = !{!1}
!llvm.ident = !{!2}

!0 = !{i32 4}
!1 = !{i32 2, i32 0}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2024.0.0 (2024.x.0.YYYYMMDD)"}
!3 = !{i32 1}
!4 = !{!"none"}
!5 = !{!"struct STRUCT*"}
!6 = !{!""}
!7 = !{i1 false}
!8 = !{i32 0}
!9 = !{!10, !10, i64 0}
!10 = !{!"any pointer", !11, i64 0}
!11 = !{!"omnipotent char", !12, i64 0}
!12 = !{!"Simple C/C++ TBAA"}
!13 = !{!11, !11, i64 0}
!14 = !{!15, !16, i64 0}
!15 = !{!"STRUCT", !16, i64 0, !16, i64 4, !16, i64 8}
!16 = !{!"int", !11, i64 0}
