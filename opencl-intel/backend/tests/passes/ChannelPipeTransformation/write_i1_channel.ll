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

; RUN: %oclopt -runtimelib=%p/../../vectorizer/Full/runtime.bc -channel-pipe-transformation -verify %s -S | FileCheck %s --implicit-check-not @_Z19write_channel_intel --implicit-check-not @_Z21read_channel_nb_intel
; ModuleID = 'main'
source_filename = "1"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

%opencl.channel_t = type opaque

@bitchannel = common addrspace(1) global [2 x %opencl.channel_t addrspace(1)*] zeroinitializer, align 1, !packet_size !0, !packet_align !0
@intch = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4, !packet_size !1, !packet_align !1

; Function Attrs: convergent nounwind
define void @send(i8 addrspace(1)* noalias %arr, i32 %size, i32 addrspace(1)* noalias %invalid) #0 !kernel_arg_addr_space !12 !kernel_arg_access_qual !13 !kernel_arg_type !14 !kernel_arg_base_type !14 !kernel_arg_type_qual !15 !kernel_arg_host_accessible !16 !kernel_arg_name !17 {
entry:
  %arr.addr = alloca i8 addrspace(1)*, align 8
  %size.addr = alloca i32, align 4
  %invalid.addr = alloca i32 addrspace(1)*, align 8
  %inval = alloca i32, align 4
  %i = alloca i32, align 4
  %ctrl = alloca i8, align 1
  %valid = alloca i8, align 1
  %tmp = alloca i8, align 1
  store i8 addrspace(1)* %arr, i8 addrspace(1)** %arr.addr, align 8, !tbaa !18
  store i32 %size, i32* %size.addr, align 4, !tbaa !22
  store i32 addrspace(1)* %invalid, i32 addrspace(1)** %invalid.addr, align 8, !tbaa !18
  %0 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* getelementptr inbounds ([2 x %opencl.channel_t addrspace(1)*], [2 x %opencl.channel_t addrspace(1)*] addrspace(1)* @bitchannel, i64 0, i64 0), align 1, !tbaa !24
  call void @_Z19write_channel_intel11ocl_channelbb(%opencl.channel_t addrspace(1)* %0, i1 zeroext false) #3
  %1 = bitcast i32* %inval to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %1) #4
  store i32 0, i32* %inval, align 4, !tbaa !22
  %2 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %2) #4
  store i32 0, i32* %i, align 4, !tbaa !22
  call void @llvm.lifetime.start.p0i8(i64 1, i8* %ctrl) #4
  store i8 0, i8* %ctrl, align 1, !tbaa !25
  br label %while.cond

while.cond:                                       ; preds = %if.end, %entry
  %3 = load i8, i8* %ctrl, align 1, !tbaa !25, !range !27
  %tobool = trunc i8 %3 to i1
  %conv = zext i1 %tobool to i32
  %cmp = icmp ne i32 %conv, 1
  br i1 %cmp, label %while.body, label %while.end

while.body:                                       ; preds = %while.cond
  call void @llvm.lifetime.start.p0i8(i64 1, i8* %valid) #4
  store i8 0, i8* %valid, align 1, !tbaa !25
  call void @llvm.lifetime.start.p0i8(i64 1, i8* %tmp) #4
  %4 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* getelementptr inbounds ([2 x %opencl.channel_t addrspace(1)*], [2 x %opencl.channel_t addrspace(1)*] addrspace(1)* @bitchannel, i64 0, i64 1), align 1, !tbaa !24
  %call = call zeroext i1 @_Z21read_channel_nb_intel11ocl_channelbPb(%opencl.channel_t addrspace(1)* %4, i8* %valid) #3
  %frombool = zext i1 %call to i8
  store i8 %frombool, i8* %tmp, align 1, !tbaa !25
  %5 = load i8, i8* %valid, align 1, !tbaa !25, !range !27
  %tobool2 = trunc i8 %5 to i1
  br i1 %tobool2, label %if.then, label %if.else

if.then:                                          ; preds = %while.body
  %6 = load i8, i8* %tmp, align 1, !tbaa !25, !range !27
  %tobool3 = trunc i8 %6 to i1
  %frombool4 = zext i1 %tobool3 to i8
  store i8 %frombool4, i8* %ctrl, align 1, !tbaa !25
  br label %if.end

if.else:                                          ; preds = %while.body
  %7 = load i32, i32* %inval, align 4, !tbaa !22
  %inc = add i32 %7, 1
  store i32 %inc, i32* %inval, align 4, !tbaa !22
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %8 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @intch, align 4, !tbaa !24
  %9 = load i8 addrspace(1)*, i8 addrspace(1)** %arr.addr, align 8, !tbaa !18
  %10 = load i32, i32* %i, align 4, !tbaa !22
  %inc5 = add nsw i32 %10, 1
  store i32 %inc5, i32* %i, align 4, !tbaa !22
  %idxprom = sext i32 %10 to i64
  %arrayidx = getelementptr inbounds i8, i8 addrspace(1)* %9, i64 %idxprom
  %11 = load i8, i8 addrspace(1)* %arrayidx, align 1, !tbaa !24
  %conv6 = sext i8 %11 to i32
  call void @_Z19write_channel_intel11ocl_channelii(%opencl.channel_t addrspace(1)* %8, i32 %conv6) #3
  call void @llvm.lifetime.end.p0i8(i64 1, i8* %tmp) #4
  call void @llvm.lifetime.end.p0i8(i64 1, i8* %valid) #4
  br label %while.cond

while.end:                                        ; preds = %while.cond
  %12 = load i32, i32* %inval, align 4, !tbaa !22
  %13 = load i32 addrspace(1)*, i32 addrspace(1)** %invalid.addr, align 8, !tbaa !18
  store i32 %12, i32 addrspace(1)* %13, align 4, !tbaa !22
  call void @llvm.lifetime.end.p0i8(i64 1, i8* %ctrl) #4
  %14 = bitcast i32* %i to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %14) #4
  %15 = bitcast i32* %inval to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %15) #4
  ret void
}

; Function Attrs: convergent
declare void @_Z19write_channel_intel11ocl_channelbb(%opencl.channel_t addrspace(1)*, i1 zeroext) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #2

; Function Attrs: convergent
declare zeroext i1 @_Z21read_channel_nb_intel11ocl_channelbPb(%opencl.channel_t addrspace(1)*, i8*) #1

; Function Attrs: convergent
declare void @_Z19write_channel_intel11ocl_channelii(%opencl.channel_t addrspace(1)*, i32) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #2

; Function Attrs: convergent nounwind
define void @recieve(i8 addrspace(1)* noalias %arr, i32 %size, i32 addrspace(1)* noalias %invalid) #0 !kernel_arg_addr_space !12 !kernel_arg_access_qual !13 !kernel_arg_type !14 !kernel_arg_base_type !14 !kernel_arg_type_qual !15 !kernel_arg_host_accessible !16 !kernel_arg_name !17 {
entry:
  %arr.addr = alloca i8 addrspace(1)*, align 8
  %size.addr = alloca i32, align 4
  %invalid.addr = alloca i32 addrspace(1)*, align 8
  %inval = alloca i32, align 4
  %ctrl = alloca i8, align 1
  %valid = alloca i8, align 1
  %tmp = alloca i8, align 1
  %i = alloca i32, align 4
  store i8 addrspace(1)* %arr, i8 addrspace(1)** %arr.addr, align 8, !tbaa !18
  store i32 %size, i32* %size.addr, align 4, !tbaa !22
  store i32 addrspace(1)* %invalid, i32 addrspace(1)** %invalid.addr, align 8, !tbaa !18
  %0 = bitcast i32* %inval to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #4
  store i32 0, i32* %inval, align 4, !tbaa !22
  call void @llvm.lifetime.start.p0i8(i64 1, i8* %ctrl) #4
  store i8 1, i8* %ctrl, align 1, !tbaa !25
  br label %while.cond

while.cond:                                       ; preds = %if.end, %entry
  %1 = load i8, i8* %ctrl, align 1, !tbaa !25, !range !27
  %tobool = trunc i8 %1 to i1
  %conv = zext i1 %tobool to i32
  %cmp = icmp ne i32 %conv, 0
  br i1 %cmp, label %while.body, label %while.end

while.body:                                       ; preds = %while.cond
  call void @llvm.lifetime.start.p0i8(i64 1, i8* %valid) #4
  store i8 0, i8* %valid, align 1, !tbaa !25
  call void @llvm.lifetime.start.p0i8(i64 1, i8* %tmp) #4
  %2 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* getelementptr inbounds ([2 x %opencl.channel_t addrspace(1)*], [2 x %opencl.channel_t addrspace(1)*] addrspace(1)* @bitchannel, i64 0, i64 0), align 1, !tbaa !24
  %call = call zeroext i1 @_Z21read_channel_nb_intel11ocl_channelbPb(%opencl.channel_t addrspace(1)* %2, i8* %valid) #3
  %frombool = zext i1 %call to i8
  store i8 %frombool, i8* %tmp, align 1, !tbaa !25
  %3 = load i8, i8* %valid, align 1, !tbaa !25, !range !27
  %tobool2 = trunc i8 %3 to i1
  br i1 %tobool2, label %if.then, label %if.else

if.then:                                          ; preds = %while.body
  %4 = load i8, i8* %tmp, align 1, !tbaa !25, !range !27
  %tobool3 = trunc i8 %4 to i1
  %frombool4 = zext i1 %tobool3 to i8
  store i8 %frombool4, i8* %ctrl, align 1, !tbaa !25
  br label %if.end

if.else:                                          ; preds = %while.body
  %5 = load i32, i32* %inval, align 4, !tbaa !22
  %inc = add i32 %5, 1
  store i32 %inc, i32* %inval, align 4, !tbaa !22
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  call void @llvm.lifetime.end.p0i8(i64 1, i8* %tmp) #4
  call void @llvm.lifetime.end.p0i8(i64 1, i8* %valid) #4
  br label %while.cond

while.end:                                        ; preds = %while.cond
  %6 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %6) #4
  store i32 0, i32* %i, align 4, !tbaa !22
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %while.end
  %7 = load i32, i32* %i, align 4, !tbaa !22
  %8 = load i32, i32* %size.addr, align 4, !tbaa !22
  %cmp5 = icmp ult i32 %7, %8
  br i1 %cmp5, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  %9 = bitcast i32* %i to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %9) #4
  br label %for.end

for.body:                                         ; preds = %for.cond
  %10 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @intch, align 4, !tbaa !24
  %call7 = call i32 @_Z18read_channel_intel11ocl_channeli(%opencl.channel_t addrspace(1)* %10) #3
  %conv8 = trunc i32 %call7 to i8
  %11 = load i8 addrspace(1)*, i8 addrspace(1)** %arr.addr, align 8, !tbaa !18
  %12 = load i32, i32* %i, align 4, !tbaa !22
  %idxprom = zext i32 %12 to i64
  %arrayidx = getelementptr inbounds i8, i8 addrspace(1)* %11, i64 %idxprom
  store i8 %conv8, i8 addrspace(1)* %arrayidx, align 1, !tbaa !24
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %13 = load i32, i32* %i, align 4, !tbaa !22
  %inc9 = add i32 %13, 1
  store i32 %inc9, i32* %i, align 4, !tbaa !22
  br label %for.cond

for.end:                                          ; preds = %for.cond.cleanup
  %14 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* getelementptr inbounds ([2 x %opencl.channel_t addrspace(1)*], [2 x %opencl.channel_t addrspace(1)*] addrspace(1)* @bitchannel, i64 0, i64 1), align 1, !tbaa !24
  call void @_Z19write_channel_intel11ocl_channelbb(%opencl.channel_t addrspace(1)* %14, i1 zeroext true) #3
  %15 = load i32, i32* %inval, align 4, !tbaa !22
  %16 = load i32 addrspace(1)*, i32 addrspace(1)** %invalid.addr, align 8, !tbaa !18
  store i32 %15, i32 addrspace(1)* %16, align 4, !tbaa !22
  call void @llvm.lifetime.end.p0i8(i64 1, i8* %ctrl) #4
  %17 = bitcast i32* %inval to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %17) #4
  ret void
}

; Function Attrs: convergent
declare i32 @_Z18read_channel_intel11ocl_channeli(%opencl.channel_t addrspace(1)*) #1

attributes #0 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { convergent "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { argmemonly nounwind }
attributes #3 = { convergent }
attributes #4 = { nounwind }

!llvm.linker.options = !{}
!llvm.module.flags = !{!6}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!7}
!opencl.spir.version = !{!7}
!opencl.used.extensions = !{!8}
!opencl.used.optional.core.features = !{!8}
!opencl.compiler.options = !{!9}
!llvm.ident = !{!10}
!opencl.kernels = !{!11}

!0 = !{i32 4}
!1 = !{i32 1}
!6 = !{i32 1, !"wchar_size", i32 4}
!7 = !{i32 1, i32 2}
!8 = !{}
!9 = !{!"-Idevice/"}
!10 = !{!"clang version 6.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 7fc81fc8922b226ea2e2c069ddae2e44619ea074) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm fcec820f4b80de4ed49d51d31292f09593b932e6)"}
!11 = !{void (i8 addrspace(1)*, i32, i32 addrspace(1)*)* @send, void (i8 addrspace(1)*, i32, i32 addrspace(1)*)* @recieve}
!12 = !{i32 1, i32 0, i32 1}
!13 = !{!"none", !"none", !"none"}
!14 = !{!"char*", !"int", !"uint*"}
!15 = !{!"restrict", !"", !"restrict"}
!16 = !{i1 false, i1 false, i1 false}
!17 = !{!"arr", !"size", !"invalid"}
!18 = !{!19, !19, i64 0}
!19 = !{!"any pointer", !20, i64 0}
!20 = !{!"omnipotent char", !21, i64 0}
!21 = !{!"Simple C/C++ TBAA"}
!22 = !{!23, !23, i64 0}
!23 = !{!"int", !20, i64 0}
!24 = !{!20, !20, i64 0}
!25 = !{!26, !26, i64 0}
!26 = !{!"bool", !20, i64 0}
!27 = !{i8 0, i8 2}

; CHECK: %write.src{{.*}} = alloca i1
; CHECK: store i1 false, i1* %write.src{{.*}}
; CHECK: %read.dst{{.*}} = alloca i1
