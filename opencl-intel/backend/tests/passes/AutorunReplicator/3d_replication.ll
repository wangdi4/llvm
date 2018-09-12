; Compiled from:
; ----------------------------------------------------
; #pragma OPENCL EXTENSION cl_intel_channels: enable
;
; channel int ch[4];
;
; __attribute__((autorun))
; __attribute__((max_global_work_dim(0)))
; __attribute__((num_compute_units(2,2,2)))
; __kernel void plus() {
;   if (get_compute_id(2) == 0) {
;     int i = read_channel_intel(ch[0]);
;     write_channel_intel(ch[1], i + 1);
;   } else if (get_compute_id(1) == 0 || get_compute_id(0) == 0) {
;     int j = read_channel_intel(ch[1]);
;     write_channel_intel(ch[2], j + 2);
;   } else {
;     int k = read_channel_intel(ch[2]);
;     write_channel_intel(ch[3], k + 3);
;   }
; }
; ----------------------------------------------------
; Clang options: -cc1 -emit-llvm -triple spir64-unknown-unknown-intelfpga -disable-llvm-passes -x cl -cl-std=CL2.0
; ----------------------------------------------------
; Opt passes: -spir-materializer
; ----------------------------------------------------
; RUN: %oclopt -runtimelib=%p/../../vectorizer/Full/runtime.bc -autorun-replicator -verify %s -S | FileCheck %s --implicit-check-not get_compute_id
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown-intelfpga"

%opencl.channel_t = type opaque

@ch = common addrspace(1) global [4 x %opencl.channel_t addrspace(1)*] zeroinitializer, align 4

; CHECK: define {{.*}}@plus
; CHECK: %cmp = icmp eq i64 0, 0
; CHECK: %cmp1 = icmp eq i64 0, 0
; CHECK: %cmp2 = icmp eq i64 0, 0

; CHECK: define {{.*}}@plus.1
; CHECK: %cmp = icmp eq i64 1, 0
; CHECK: %cmp1 = icmp eq i64 0, 0
; CHECK: %cmp2 = icmp eq i64 0, 0

; CHECK: define {{.*}}@plus.2
; CHECK: %cmp = icmp eq i64 0, 0
; CHECK: %cmp1 = icmp eq i64 1, 0
; CHECK: %cmp2 = icmp eq i64 0, 0

; CHECK: define {{.*}}@plus.3
; CHECK: %cmp = icmp eq i64 1, 0
; CHECK: %cmp1 = icmp eq i64 1, 0
; CHECK: %cmp2 = icmp eq i64 0, 0

; CHECK: define {{.*}}@plus.4
; CHECK: %cmp = icmp eq i64 0, 0
; CHECK: %cmp1 = icmp eq i64 0, 0
; CHECK: %cmp2 = icmp eq i64 1, 0

; CHECK: define {{.*}}@plus.5
; CHECK: %cmp = icmp eq i64 1, 0
; CHECK: %cmp1 = icmp eq i64 0, 0
; CHECK: %cmp2 = icmp eq i64 1, 0

; CHECK: define {{.*}}@plus.6
; CHECK: %cmp = icmp eq i64 0, 0
; CHECK: %cmp1 = icmp eq i64 1, 0
; CHECK: %cmp2 = icmp eq i64 1, 0

; CHECK: define {{.*}}@plus.7
; CHECK: %cmp = icmp eq i64 1, 0
; CHECK: %cmp1 = icmp eq i64 1, 0
; CHECK: %cmp2 = icmp eq i64 1, 0

; CHECK-NOT: define {{.*}}@plus

; Function Attrs: nounwind
define void @plus() #0 !kernel_arg_addr_space !5 !kernel_arg_access_qual !5 !kernel_arg_type !5 !kernel_arg_base_type !5 !kernel_arg_type_qual !5 !task !8 !autorun !8 !num_compute_units !9 {
entry:
  %i = alloca i32, align 4
  %j = alloca i32, align 4
  %k = alloca i32, align 4
  %0 = call i64 @get_compute_id(i32 2)
  %cmp = icmp eq i64 %0, 0
  br i1 %cmp, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  %1 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %1) #3
  %2 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x %opencl.channel_t addrspace(1)*], [4 x %opencl.channel_t addrspace(1)*] addrspace(1)* @ch, i64 0, i64 0), align 4, !tbaa !10
  %call = call i32 @_Z18read_channel_intel11ocl_channeli(%opencl.channel_t addrspace(1)* %2)
  store i32 %call, i32* %i, align 4, !tbaa !13
  %3 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x %opencl.channel_t addrspace(1)*], [4 x %opencl.channel_t addrspace(1)*] addrspace(1)* @ch, i64 0, i64 1), align 4, !tbaa !10
  %4 = load i32, i32* %i, align 4, !tbaa !13
  %add = add nsw i32 %4, 1
  call void @_Z19write_channel_intel11ocl_channelii(%opencl.channel_t addrspace(1)* %3, i32 %add)
  %5 = bitcast i32* %i to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %5) #3
  br label %if.end9

if.else:                                          ; preds = %entry
  %6 = call i64 @get_compute_id(i32 1)
  %cmp1 = icmp eq i64 %6, 0
  br i1 %cmp1, label %if.then3, label %lor.lhs.false

lor.lhs.false:                                    ; preds = %if.else
  %7 = call i64 @get_compute_id(i32 0)
  %cmp2 = icmp eq i64 %7, 0
  br i1 %cmp2, label %if.then3, label %if.else6

if.then3:                                         ; preds = %lor.lhs.false, %if.else
  %8 = bitcast i32* %j to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %8) #3
  %9 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x %opencl.channel_t addrspace(1)*], [4 x %opencl.channel_t addrspace(1)*] addrspace(1)* @ch, i64 0, i64 1), align 4, !tbaa !10
  %call4 = call i32 @_Z18read_channel_intel11ocl_channeli(%opencl.channel_t addrspace(1)* %9)
  store i32 %call4, i32* %j, align 4, !tbaa !13
  %10 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x %opencl.channel_t addrspace(1)*], [4 x %opencl.channel_t addrspace(1)*] addrspace(1)* @ch, i64 0, i64 2), align 4, !tbaa !10
  %11 = load i32, i32* %j, align 4, !tbaa !13
  %add5 = add nsw i32 %11, 2
  call void @_Z19write_channel_intel11ocl_channelii(%opencl.channel_t addrspace(1)* %10, i32 %add5)
  %12 = bitcast i32* %j to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %12) #3
  br label %if.end

if.else6:                                         ; preds = %lor.lhs.false
  %13 = bitcast i32* %k to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %13) #3
  %14 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x %opencl.channel_t addrspace(1)*], [4 x %opencl.channel_t addrspace(1)*] addrspace(1)* @ch, i64 0, i64 2), align 4, !tbaa !10
  %call7 = call i32 @_Z18read_channel_intel11ocl_channeli(%opencl.channel_t addrspace(1)* %14)
  store i32 %call7, i32* %k, align 4, !tbaa !13
  %15 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x %opencl.channel_t addrspace(1)*], [4 x %opencl.channel_t addrspace(1)*] addrspace(1)* @ch, i64 0, i64 3), align 4, !tbaa !10
  %16 = load i32, i32* %k, align 4, !tbaa !13
  %add8 = add nsw i32 %16, 3
  call void @_Z19write_channel_intel11ocl_channelii(%opencl.channel_t addrspace(1)* %15, i32 %add8)
  %17 = bitcast i32* %k to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %17) #3
  br label %if.end

if.end:                                           ; preds = %if.else6, %if.then3
  br label %if.end9

if.end9:                                          ; preds = %if.end, %if.then
  ret void
}

declare i64 @get_compute_id(i32)

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

declare i32 @_Z18read_channel_intel11ocl_channeli(%opencl.channel_t addrspace(1)*) #2

declare void @_Z19write_channel_intel11ocl_channelii(%opencl.channel_t addrspace(1)*, i32) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

attributes #0 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }

!opencl.channels = !{!0}
!llvm.module.flags = !{!3}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!4}
!opencl.spir.version = !{!4}
!opencl.used.extensions = !{!5}
!opencl.used.optional.core.features = !{!5}
!opencl.compiler.options = !{!5}
!llvm.ident = !{!6}
!opencl.kernels = !{!7}

!0 = !{[4 x %opencl.channel_t addrspace(1)*] addrspace(1)* @ch, !1, !2}
!1 = !{!"packet_size", i32 4}
!2 = !{!"packet_align", i32 4}
!3 = !{i32 1, !"wchar_size", i32 4}
!4 = !{i32 2, i32 0}
!5 = !{}
!6 = !{!"clang version 5.0.0 "}
!7 = !{void ()* @plus}
!8 = !{i1 true}
!9 = !{i32 2, i32 2, i32 2}
!10 = !{!11, !11, i64 0}
!11 = !{!"omnipotent char", !12, i64 0}
!12 = !{!"Simple C/C++ TBAA"}
!13 = !{!14, !14, i64 0}
!14 = !{!"int", !11, i64 0}
