;-----------------------------------------------------------------------------
; Compiled from:
;-----------------------------------------------------------------------------
; #pragma OPENCL EXTENSION cl_intel_channels : enable
; channel int intch;
; channel bool bitchannel[2];
;
; kernel void send( global char* restrict arr, int size, global unsigned* restrict invalid) {
;   write_channel_intel(bitchannel[0],0);// Go
;   unsigned inval=0;
;   int i=0;
;   bool ctrl=0;
;   while (ctrl != 1) { // Wait for stop
;     bool valid = 0;
;     bool tmp =  read_channel_nb_intel(bitchannel[1],&valid);
;     if (valid) {
;       ctrl=tmp;
;     } else {
;       inval++;
;     }
;     write_channel_intel(intch,arr[i++]);
;   }
;   *invalid = inval;
; }
;
; kernel void recieve( global char* restrict arr, int size, global unsigned* restrict invalid) {
;   unsigned inval=0;
;   bool ctrl=1;
;   while (ctrl != 0) { // Wait for start
;     bool valid = 0;
;     bool tmp =  read_channel_nb_intel(bitchannel[0],&valid);
;     if (valid) {
;       ctrl=tmp;
;     } else {
;       inval++;
;     }
;   }
;
;   for (unsigned i=0; i<size; i++) {
;     arr[i] =read_channel_intel(intch);
;   }
;   write_channel_intel(bitchannel[1],1);// Tell Sender to quit
;   *invalid = inval;
; }
;-----------------------------------------------------------------------------
; Compile options: -cc1 -emit-llvm -triple spir64-unknown-unknown-intelfpga -disable-llvm-passes -x cl -cl-std=CL1.2
; ----------------------------------------------------

; RUN: opt -sycl-kernel-builtin-lib=%p/../Inputs/fpga-pipes.rtl -passes=sycl-kernel-channel-pipe-transformation %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -sycl-kernel-builtin-lib=%p/../Inputs/fpga-pipes.rtl -passes=sycl-kernel-channel-pipe-transformation %s -S | FileCheck %s --implicit-check-not write_channel_intel --implicit-check-not read_channel_nb_intel

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64-unknown-unknown-intelfpga"

@bitchannel = addrspace(1) global [2 x ptr addrspace(1)] zeroinitializer, align 1, !packet_size !0, !packet_align !0
@intch = addrspace(1) global ptr addrspace(1) null, align 4, !packet_size !1, !packet_align !1

; CHECK: %write.src{{.*}} = alloca i1
; CHECK: store i1 false, ptr %write.src{{.*}}

; Function Attrs: convergent norecurse nounwind
define dso_local void @send(ptr addrspace(1) noalias noundef align 1 %arr, i32 noundef %size, ptr addrspace(1) noalias noundef align 4 %invalid) #0 !kernel_arg_addr_space !6 !kernel_arg_access_qual !7 !kernel_arg_type !8 !kernel_arg_base_type !8 !kernel_arg_type_qual !9 !kernel_arg_host_accessible !10 !kernel_arg_pipe_depth !11 !kernel_arg_pipe_io !12 !kernel_arg_buffer_location !12 !arg_type_null_val !13 {
entry:
  %arr.addr = alloca ptr addrspace(1), align 8
  %size.addr = alloca i32, align 4
  %invalid.addr = alloca ptr addrspace(1), align 8
  %inval = alloca i32, align 4
  %i = alloca i32, align 4
  %ctrl = alloca i8, align 1
  %valid = alloca i8, align 1
  %tmp = alloca i8, align 1
  store ptr addrspace(1) %arr, ptr %arr.addr, align 8, !tbaa !14
  store i32 %size, ptr %size.addr, align 4, !tbaa !18
  store ptr addrspace(1) %invalid, ptr %invalid.addr, align 8, !tbaa !14
  %0 = load ptr addrspace(1), ptr addrspace(1) @bitchannel, align 1, !tbaa !20
  call void @_Z19write_channel_intel11ocl_channelbb(ptr addrspace(1) %0, i1 noundef zeroext false) #3
  call void @llvm.lifetime.start.p0(i64 4, ptr %inval) #4
  store i32 0, ptr %inval, align 4, !tbaa !18
  call void @llvm.lifetime.start.p0(i64 4, ptr %i) #4
  store i32 0, ptr %i, align 4, !tbaa !18
  call void @llvm.lifetime.start.p0(i64 1, ptr %ctrl) #4
  store i8 0, ptr %ctrl, align 1, !tbaa !21
  br label %while.cond

while.cond:                                       ; preds = %if.end, %entry
  %1 = load i8, ptr %ctrl, align 1, !tbaa !21, !range !23, !noundef !3
  %tobool = trunc i8 %1 to i1
  %conv = zext i1 %tobool to i32
  %cmp = icmp ne i32 %conv, 1
  br i1 %cmp, label %while.body, label %while.end

while.body:                                       ; preds = %while.cond
  call void @llvm.lifetime.start.p0(i64 1, ptr %valid) #4
  store i8 0, ptr %valid, align 1, !tbaa !21
  call void @llvm.lifetime.start.p0(i64 1, ptr %tmp) #4
  %2 = load ptr addrspace(1), ptr addrspace(1) getelementptr inbounds ([2 x ptr addrspace(1)], ptr addrspace(1) @bitchannel, i64 0, i64 1), align 1, !tbaa !20
  %call = call zeroext i1 @_Z21read_channel_nb_intel11ocl_channelbPb(ptr addrspace(1) %2, ptr noundef %valid) #3
  %frombool = zext i1 %call to i8
  store i8 %frombool, ptr %tmp, align 1, !tbaa !21
  %3 = load i8, ptr %valid, align 1, !tbaa !21, !range !23, !noundef !3
  %tobool2 = trunc i8 %3 to i1
  br i1 %tobool2, label %if.then, label %if.else

if.then:                                          ; preds = %while.body
  %4 = load i8, ptr %tmp, align 1, !tbaa !21, !range !23, !noundef !3
  %tobool3 = trunc i8 %4 to i1
  %frombool4 = zext i1 %tobool3 to i8
  store i8 %frombool4, ptr %ctrl, align 1, !tbaa !21
  br label %if.end

if.else:                                          ; preds = %while.body
  %5 = load i32, ptr %inval, align 4, !tbaa !18
  %inc = add i32 %5, 1
  store i32 %inc, ptr %inval, align 4, !tbaa !18
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %6 = load ptr addrspace(1), ptr addrspace(1) @intch, align 4, !tbaa !20
  %7 = load ptr addrspace(1), ptr %arr.addr, align 8, !tbaa !14
  %8 = load i32, ptr %i, align 4, !tbaa !18
  %inc5 = add nsw i32 %8, 1
  store i32 %inc5, ptr %i, align 4, !tbaa !18
  %idxprom = sext i32 %8 to i64
  %arrayidx = getelementptr inbounds i8, ptr addrspace(1) %7, i64 %idxprom
  %9 = load i8, ptr addrspace(1) %arrayidx, align 1, !tbaa !20
  %conv6 = sext i8 %9 to i32
  call void @_Z19write_channel_intel11ocl_channelii(ptr addrspace(1) %6, i32 noundef %conv6) #3
  call void @llvm.lifetime.end.p0(i64 1, ptr %tmp) #4
  call void @llvm.lifetime.end.p0(i64 1, ptr %valid) #4
  br label %while.cond

while.end:                                        ; preds = %while.cond
  %10 = load i32, ptr %inval, align 4, !tbaa !18
  %11 = load ptr addrspace(1), ptr %invalid.addr, align 8, !tbaa !14
  store i32 %10, ptr addrspace(1) %11, align 4, !tbaa !18
  call void @llvm.lifetime.end.p0(i64 1, ptr %ctrl) #4
  call void @llvm.lifetime.end.p0(i64 4, ptr %i) #4
  call void @llvm.lifetime.end.p0(i64 4, ptr %inval) #4
  ret void
}

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: convergent norecurse nounwind
define dso_local void @recieve(ptr addrspace(1) noalias noundef align 1 %arr, i32 noundef %size, ptr addrspace(1) noalias noundef align 4 %invalid) #0 !kernel_arg_addr_space !6 !kernel_arg_access_qual !7 !kernel_arg_type !8 !kernel_arg_base_type !8 !kernel_arg_type_qual !9 !kernel_arg_host_accessible !10 !kernel_arg_pipe_depth !11 !kernel_arg_pipe_io !12 !kernel_arg_buffer_location !12 !arg_type_null_val !13 {
entry:
  %arr.addr = alloca ptr addrspace(1), align 8
  %size.addr = alloca i32, align 4
  %invalid.addr = alloca ptr addrspace(1), align 8
  %inval = alloca i32, align 4
  %ctrl = alloca i8, align 1
  %valid = alloca i8, align 1
  %tmp = alloca i8, align 1
  %i = alloca i32, align 4
  store ptr addrspace(1) %arr, ptr %arr.addr, align 8, !tbaa !14
  store i32 %size, ptr %size.addr, align 4, !tbaa !18
  store ptr addrspace(1) %invalid, ptr %invalid.addr, align 8, !tbaa !14
  call void @llvm.lifetime.start.p0(i64 4, ptr %inval) #4
  store i32 0, ptr %inval, align 4, !tbaa !18
  call void @llvm.lifetime.start.p0(i64 1, ptr %ctrl) #4
  store i8 1, ptr %ctrl, align 1, !tbaa !21
  br label %while.cond

while.cond:                                       ; preds = %if.end, %entry
  %0 = load i8, ptr %ctrl, align 1, !tbaa !21, !range !23, !noundef !3
  %tobool = trunc i8 %0 to i1
  %conv = zext i1 %tobool to i32
  %cmp = icmp ne i32 %conv, 0
  br i1 %cmp, label %while.body, label %while.end

while.body:                                       ; preds = %while.cond
  call void @llvm.lifetime.start.p0(i64 1, ptr %valid) #4
  store i8 0, ptr %valid, align 1, !tbaa !21
  call void @llvm.lifetime.start.p0(i64 1, ptr %tmp) #4
  %1 = load ptr addrspace(1), ptr addrspace(1) @bitchannel, align 1, !tbaa !20
; CHECK: void @recieve
; CHECK: [[WRITE_SRC:%.*]] = addrspacecast ptr %write.src to ptr addrspace(4)
; CHECK-NEXT: call i32 @__read_pipe_2_fpga(ptr addrspace(1) %1, ptr addrspace(4) [[WRITE_SRC]], i32 1, i32 1)
  %call = call zeroext i1 @_Z21read_channel_nb_intel11ocl_channelbPb(ptr addrspace(1) %1, ptr noundef %valid) #3
  %frombool = zext i1 %call to i8
  store i8 %frombool, ptr %tmp, align 1, !tbaa !21
  %2 = load i8, ptr %valid, align 1, !tbaa !21, !range !23, !noundef !3
  %tobool2 = trunc i8 %2 to i1
  br i1 %tobool2, label %if.then, label %if.else

if.then:                                          ; preds = %while.body
  %3 = load i8, ptr %tmp, align 1, !tbaa !21, !range !23, !noundef !3
  %tobool3 = trunc i8 %3 to i1
  %frombool4 = zext i1 %tobool3 to i8
  store i8 %frombool4, ptr %ctrl, align 1, !tbaa !21
  br label %if.end

if.else:                                          ; preds = %while.body
  %4 = load i32, ptr %inval, align 4, !tbaa !18
  %inc = add i32 %4, 1
  store i32 %inc, ptr %inval, align 4, !tbaa !18
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  call void @llvm.lifetime.end.p0(i64 1, ptr %tmp) #4
  call void @llvm.lifetime.end.p0(i64 1, ptr %valid) #4
  br label %while.cond

while.end:                                        ; preds = %while.cond
  call void @llvm.lifetime.start.p0(i64 4, ptr %i) #4
  store i32 0, ptr %i, align 4, !tbaa !18
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %while.end
  %5 = load i32, ptr %i, align 4, !tbaa !18
  %6 = load i32, ptr %size.addr, align 4, !tbaa !18
  %cmp5 = icmp ult i32 %5, %6
  br i1 %cmp5, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  call void @llvm.lifetime.end.p0(i64 4, ptr %i) #4
  br label %for.end

for.body:                                         ; preds = %for.cond
  %7 = load ptr addrspace(1), ptr addrspace(1) @intch, align 4, !tbaa !20
  %call7 = call i32 @_Z18read_channel_intel11ocl_channeli(ptr addrspace(1) %7) #3
  %conv8 = trunc i32 %call7 to i8
  %8 = load ptr addrspace(1), ptr %arr.addr, align 8, !tbaa !14
  %9 = load i32, ptr %i, align 4, !tbaa !18
  %idxprom = zext i32 %9 to i64
  %arrayidx = getelementptr inbounds i8, ptr addrspace(1) %8, i64 %idxprom
  store i8 %conv8, ptr addrspace(1) %arrayidx, align 1, !tbaa !20
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %10 = load i32, ptr %i, align 4, !tbaa !18
  %inc9 = add i32 %10, 1
  store i32 %inc9, ptr %i, align 4, !tbaa !18
  br label %for.cond

for.end:                                          ; preds = %for.cond.cleanup
  %11 = load ptr addrspace(1), ptr addrspace(1) getelementptr inbounds ([2 x ptr addrspace(1)], ptr addrspace(1) @bitchannel, i64 0, i64 1), align 1, !tbaa !20
  call void @_Z19write_channel_intel11ocl_channelbb(ptr addrspace(1) %11, i1 noundef zeroext true) #3
  %12 = load i32, ptr %inval, align 4, !tbaa !18
  %13 = load ptr addrspace(1), ptr %invalid.addr, align 8, !tbaa !14
  store i32 %12, ptr addrspace(1) %13, align 4, !tbaa !18
  call void @llvm.lifetime.end.p0(i64 1, ptr %ctrl) #4
  call void @llvm.lifetime.end.p0(i64 4, ptr %inval) #4
  ret void
}

; Function Attrs: convergent nounwind
declare void @_Z19write_channel_intel11ocl_channelbb(ptr addrspace(1), i1 noundef zeroext) #2

; Function Attrs: convergent nounwind
declare zeroext i1 @_Z21read_channel_nb_intel11ocl_channelbPb(ptr addrspace(1), ptr noundef) #2

; Function Attrs: convergent nounwind
declare void @_Z19write_channel_intel11ocl_channelii(ptr addrspace(1), i32 noundef) #2

; Function Attrs: convergent nounwind
declare i32 @_Z18read_channel_intel11ocl_channeli(ptr addrspace(1)) #2

attributes #0 = { convergent norecurse nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "uniform-work-group-size"="true" }
attributes #1 = { nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #2 = { convergent nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" }
attributes #3 = { convergent nounwind }
attributes #4 = { nounwind }

!opencl.ocl.version = !{!2}
!opencl.spir.version = !{!2}
!opencl.compiler.options = !{!3}
!llvm.ident = !{!4}
!sycl.kernels = !{!5}

!0 = !{i32 1}
!1 = !{i32 4}
!2 = !{i32 1, i32 2}
!3 = !{}
!4 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2024.0.0 (2024.x.0.YYYYMMDD)"}
!5 = !{ptr @send, ptr @recieve}
!6 = !{i32 1, i32 0, i32 1}
!7 = !{!"none", !"none", !"none"}
!8 = !{!"char*", !"int", !"uint*"}
!9 = !{!"restrict", !"", !"restrict"}
!10 = !{i1 false, i1 false, i1 false}
!11 = !{i32 0, i32 0, i32 0}
!12 = !{!"", !"", !""}
!13 = !{ptr addrspace(1) null, i32 0, ptr addrspace(1) null}
!14 = !{!15, !15, i64 0}
!15 = !{!"any pointer", !16, i64 0}
!16 = !{!"omnipotent char", !17, i64 0}
!17 = !{!"Simple C/C++ TBAA"}
!18 = !{!19, !19, i64 0}
!19 = !{!"int", !16, i64 0}
!20 = !{!16, !16, i64 0}
!21 = !{!22, !22, i64 0}
!22 = !{!"bool", !16, i64 0}
!23 = !{i8 0, i8 2}

; DEBUGIFY-NOT: WARNING: Missing line
