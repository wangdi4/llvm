; RUN: opt -passes='debugify,sycl-kernel-resolve-wi-call,check-debugify' -S %s -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes='sycl-kernel-resolve-wi-call' -S %s | FileCheck %s

; Check that un-promoted and integer literals are stored into argument buffer
; according to their actual sizes.
; For instance, literal 321 is larger than size of '%hhd' modifier allowed, and
; literal 11121314 is smaller than size of '%ld' modifier allowed.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

@.str = private unnamed_addr addrspace(2) constant [36 x i8] c"%hhd, %2.2v4hlf, %hd, %zd, %d, %ld\0A\00", align 1

; Function Attrs: convergent
declare i32 @printf(ptr addrspace(2), ...) #0

; Function Attrs: nounwind readnone
declare ptr @get_special_buffer.() #1

; CHECK: %temp_arg_buf = alloca [88 x i8], align 4
; CHECK: [[GEP0:%[0-9]+]] = getelementptr inbounds [88 x i8], ptr %temp_arg_buf, i32 0, i32 0
; CHECK-NEXT: store i32 88, ptr [[GEP0]], align 4
; CHECK-NEXT: [[GEP1:%[0-9]+]] = getelementptr inbounds [88 x i8], ptr %temp_arg_buf, i32 0, i32 4
; CHECK-NEXT: store i32 4, ptr [[GEP1]], align 4
; CHECK-NEXT: [[GEP2:%[0-9]+]] = getelementptr inbounds [88 x i8], ptr %temp_arg_buf, i32 0, i32 8
; CHECK-NEXT: store i32 321, ptr [[GEP2]], align 1
; CHECK-NEXT: [[GEP3:%[0-9]+]] = getelementptr inbounds [88 x i8], ptr %temp_arg_buf, i32 0, i32 12
; CHECK-NEXT: store i32 8, ptr [[GEP3]], align 4
; CHECK-NEXT: [[GEP4:%[0-9]+]] = getelementptr inbounds [88 x i8], ptr %temp_arg_buf, i32 0, i32 16
; CHECK-NEXT: store <4 x double> %printf.promoted, ptr [[GEP4]], align 1
; CHECK-NEXT: [[GEP5:%[0-9]+]] = getelementptr inbounds [88 x i8], ptr %temp_arg_buf, i32 0, i32 48
; CHECK-NEXT: store i32 4, ptr [[GEP5]], align 4
; CHECK-NEXT: [[GEP6:%[0-9]+]] = getelementptr inbounds [88 x i8], ptr %temp_arg_buf, i32 0, i32 52
; CHECK-NEXT: store i32 %conv, ptr [[GEP6]], align 1
; CHECK-NEXT: [[GEP7:%[0-9]+]] = getelementptr inbounds [88 x i8], ptr %temp_arg_buf, i32 0, i32 56
; CHECK-NEXT: store i32 262152, ptr [[GEP7]], align 4
; CHECK-NEXT: [[GEP8:%[0-9]+]] = getelementptr inbounds [88 x i8], ptr %temp_arg_buf, i32 0, i32 64
; CHECK-NEXT: store i64 %{{[0-9]+}}, ptr [[GEP8]], align 1
; CHECK-NEXT: [[GEP9:%[0-9]+]] = getelementptr inbounds [88 x i8], ptr %temp_arg_buf, i32 0, i32 72
; CHECK-NEXT: store i32 4, ptr [[GEP9]], align 4
; CHECK-NEXT: [[GEP10:%[0-9]+]] = getelementptr inbounds [88 x i8], ptr %temp_arg_buf, i32 0, i32 76
; CHECK-NEXT: store i32 %conv1, ptr [[GEP10]], align 1
; CHECK-NEXT: [[GEP11:%[0-9]+]] = getelementptr inbounds [88 x i8], ptr %temp_arg_buf, i32 0, i32 80
; CHECK-NEXT: store i32 4, ptr [[GEP11]], align 4
; CHECK-NEXT: [[GEP12:%[0-9]+]] = getelementptr inbounds [88 x i8], ptr %temp_arg_buf, i32 0, i32 84
; CHECK-NEXT: store i32 11121314, ptr [[GEP12]], align 1
; CHECK-NEXT: %translated_opencl_printf_call = call i32 @__opencl_printf(ptr addrspace(2) @.str, ptr [[GEP0]] 

; Function Attrs: convergent noinline norecurse nounwind optnone
define dso_local void @test(ptr addrspace(3) noalias %pLocalMemBase, ptr noalias %pWorkDim, ptr noalias %pWGId, [4 x i64] %BaseGlbId, ptr noalias %pSpecialBuf, ptr noalias %RuntimeHandle) #2 !kernel_arg_addr_space !3 !kernel_arg_access_qual !3 !kernel_arg_type !3 !kernel_arg_base_type !3 !kernel_arg_type_qual !3 !kernel_arg_name !3 !kernel_arg_host_accessible !3 !kernel_arg_pipe_depth !3 !kernel_arg_pipe_io !3 !kernel_arg_buffer_location !3 !kernel_has_sub_groups !4 !barrier_buffer_size !5 !kernel_execution_length !6 !kernel_has_global_sync !4 !recommended_vector_length !7 !private_memory_size !8 {
entry:
  %hhd.addr = alloca ptr, align 8
  %hd.addr = alloca ptr, align 8
  %zd.addr = alloca ptr, align 8
  %tmp.addr = alloca ptr, align 8
  %.compoundliteral.addr = alloca ptr, align 8
  %pCurrSBIndex = alloca i64, align 8
  %pSB = call ptr @get_special_buffer.()
  br label %FirstBB

FirstBB:                                          ; preds = %entry
  store i64 0, ptr %pCurrSBIndex, align 8
  br label %SyncBB1

SyncBB1:                                          ; preds = %FirstBB
  %SBIndex9 = load i64, ptr %pCurrSBIndex, align 8
  %SB_LocalId_Offset10 = add nuw i64 %SBIndex9, 32
  %0 = getelementptr inbounds i8, ptr %pSB, i64 %SB_LocalId_Offset10
  store ptr %0, ptr %.compoundliteral.addr, align 8
  %1 = load ptr, ptr %.compoundliteral.addr, align 8
  %SBIndex6 = load i64, ptr %pCurrSBIndex, align 8
  %SB_LocalId_Offset7 = add nuw i64 %SBIndex6, 16
  %2 = getelementptr inbounds i8, ptr %pSB, i64 %SB_LocalId_Offset7
  store ptr %2, ptr %tmp.addr, align 8
  %3 = load ptr, ptr %tmp.addr, align 8
  %SBIndex3 = load i64, ptr %pCurrSBIndex, align 8
  %SB_LocalId_Offset4 = add nuw i64 %SBIndex3, 8
  %4 = getelementptr inbounds i8, ptr %pSB, i64 %SB_LocalId_Offset4
  store ptr %4, ptr %zd.addr, align 8
  %5 = load ptr, ptr %zd.addr, align 8
  %SBIndex1 = load i64, ptr %pCurrSBIndex, align 8
  %SB_LocalId_Offset2 = add nuw i64 %SBIndex1, 2
  %6 = getelementptr inbounds i8, ptr %pSB, i64 %SB_LocalId_Offset2
  store ptr %6, ptr %hd.addr, align 8
  %7 = load ptr, ptr %hd.addr, align 8
  %SBIndex = load i64, ptr %pCurrSBIndex, align 8
  %SB_LocalId_Offset = add nuw i64 %SBIndex, 0
  %8 = getelementptr inbounds i8, ptr %pSB, i64 %SB_LocalId_Offset
  store ptr %8, ptr %hhd.addr, align 8
  %9 = load ptr, ptr %hhd.addr, align 8
  store i8 123, ptr %9, align 1
  store i16 12345, ptr %7, align 2
  store i64 1234567891011121314, ptr %5, align 8
  store <4 x float> <float 1.000000e+00, float 2.000000e+00, float 3.000000e+00, float 4.000000e+00>, ptr %1, align 16
  %10 = load <4 x float>, ptr %1, align 16
  store <4 x float> %10, ptr %3, align 16
  %11 = load <4 x float>, ptr %3, align 16
  %12 = load i16, ptr %7, align 2
  %conv = sext i16 %12 to i32
  %13 = load i64, ptr %5, align 8
  %14 = load i8, ptr %9, align 1
  %conv1 = sext i8 %14 to i32
  %printf.promoted = fpext <4 x float> %11 to <4 x double>
  %call = call i32 (ptr addrspace(2), ...) @printf(ptr addrspace(2) @.str, i32 321, <4 x double> %printf.promoted, i32 %conv, i64 %13, i32 %conv1, i32 11121314) #3
  br label %SyncBB0

SyncBB0:                                          ; preds = %SyncBB1
  ret void
}

attributes #0 = { convergent "frame-pointer"="none" "no-trapping-math"="true" "prefer-vector-width"="512" "stack-protector-buffer-size"="8" "stackrealign" }
attributes #1 = { nounwind readnone }
attributes #2 = { convergent noinline norecurse nounwind optnone "frame-pointer"="none" "min-legal-vector-width"="128" "no-trapping-math"="true" "prefer-vector-width"="512" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="true" }
attributes #3 = { convergent nobuiltin }

!opencl.ocl.version = !{!0}
!opencl.spir.version = !{!0}
!opencl.compiler.options = !{!1}
!sycl.kernels = !{!2}

!0 = !{i32 1, i32 2}
!1 = !{!"-cl-opt-disable"}
!2 = !{ptr @test}
!3 = !{}
!4 = !{i1 false}
!5 = !{i32 48}
!6 = !{i32 20}
!7 = !{i32 1}
!8 = !{i32 88}

; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test --  %{{.*}} = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], ptr, ptr, [3 x i64], [2 x [3 x i64]], [3 x i64] }, ptr %pWorkDim, i32 0, i32 5
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test --  %RuntimeInterface = load ptr, ptr %{{.*}}, align 1
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test --  %temp_arg_buf = alloca [88 x i8], align 4
; DEBUGIFY: WARNING: Missing line 7
; DEBUGIFY-NOT: WARNING
