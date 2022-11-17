; RUN: opt -debugify -dpcpp-kernel-resolve-wi-call -check-debugify -S %s -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -dpcpp-kernel-resolve-wi-call -S %s | FileCheck %s
; RUN: opt -passes='debugify,dpcpp-kernel-resolve-wi-call,check-debugify' -S %s -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes='dpcpp-kernel-resolve-wi-call' -S %s | FileCheck %s

; Check that un-promoted and integer literals are stored into argument buffer
; according to their actual sizes.
; For instance, literal 321 is larger than size of '%hhd' modifier allowed, and
; literal 11121314 is smaller than size of '%ld' modifier allowed.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

@.str = private unnamed_addr addrspace(2) constant [36 x i8] c"%hhd, %2.2v4hlf, %hd, %zd, %d, %ld\0A\00", align 1

; Function Attrs: convergent
declare i32 @printf(i8 addrspace(2)*, ...) #0

; Function Attrs: nounwind readnone
declare i8* @get_special_buffer.() #1

; CHECK: %temp_arg_buf = alloca [88 x i8], align 4
; CHECK: [[GEP0:%[0-9]+]] = getelementptr inbounds [88 x i8], [88 x i8]* %temp_arg_buf, i32 0, i32 0
; CHECK-NEXT: %arg_buf_size = bitcast i8* [[GEP0]] to i32*
; CHECK-NEXT: store i32 88, i32* %arg_buf_size, align 4
; CHECK-NEXT: [[GEP1:%[0-9]+]] = getelementptr inbounds [88 x i8], [88 x i8]* %temp_arg_buf, i32 0, i32 4
; CHECK-NEXT: %arg_size = bitcast i8* [[GEP1]] to i32*
; CHECK-NEXT: store i32 4, i32* %arg_size, align 4
; CHECK-NEXT: [[GEP2:%[0-9]+]] = getelementptr inbounds [88 x i8], [88 x i8]* %temp_arg_buf, i32 0, i32 8
; CHECK-NEXT: %arg_val = bitcast i8* [[GEP2]] to i32*
; CHECK-NEXT: store i32 321, i32* %arg_val, align 1
; CHECK-NEXT: [[GEP3:%[0-9]+]] = getelementptr inbounds [88 x i8], [88 x i8]* %temp_arg_buf, i32 0, i32 12
; CHECK-NEXT: %arg_size1 = bitcast i8* [[GEP3]] to i32*
; CHECK-NEXT: store i32 8, i32* %arg_size1, align 4
; CHECK-NEXT: [[GEP4:%[0-9]+]] = getelementptr inbounds [88 x i8], [88 x i8]* %temp_arg_buf, i32 0, i32 16
; CHECK-NEXT: %arg_val2 = bitcast i8* [[GEP4]] to <4 x double>*
; CHECK-NEXT: store <4 x double> %printf.promoted, <4 x double>* %arg_val2, align 1
; CHECK-NEXT: [[GEP5:%[0-9]+]] = getelementptr inbounds [88 x i8], [88 x i8]* %temp_arg_buf, i32 0, i32 48
; CHECK-NEXT: %arg_size3 = bitcast i8* [[GEP5]] to i32*
; CHECK-NEXT: store i32 4, i32* %arg_size3, align 4
; CHECK-NEXT: [[GEP6:%[0-9]+]] = getelementptr inbounds [88 x i8], [88 x i8]* %temp_arg_buf, i32 0, i32 52
; CHECK-NEXT: %arg_val4 = bitcast i8* [[GEP6]] to i32*
; CHECK-NEXT: store i32 %conv, i32* %arg_val4, align 1
; CHECK-NEXT: [[GEP7:%[0-9]+]] = getelementptr inbounds [88 x i8], [88 x i8]* %temp_arg_buf, i32 0, i32 56
; CHECK-NEXT: %arg_size5 = bitcast i8* [[GEP7]] to i32*
; CHECK-NEXT: store i32 262152, i32* %arg_size5, align 4
; CHECK-NEXT: [[GEP8:%[0-9]+]] = getelementptr inbounds [88 x i8], [88 x i8]* %temp_arg_buf, i32 0, i32 64
; CHECK-NEXT: %arg_val6 = bitcast i8* [[GEP8]] to i64*
; CHECK-NEXT: store i64 %{{[0-9]+}}, i64* %arg_val6, align 1
; CHECK-NEXT: [[GEP9:%[0-9]+]] = getelementptr inbounds [88 x i8], [88 x i8]* %temp_arg_buf, i32 0, i32 72
; CHECK-NEXT: %arg_size7 = bitcast i8* [[GEP9]] to i32*
; CHECK-NEXT: store i32 4, i32* %arg_size7, align 4
; CHECK-NEXT: [[GEP10:%[0-9]+]] = getelementptr inbounds [88 x i8], [88 x i8]* %temp_arg_buf, i32 0, i32 76
; CHECK-NEXT: %arg_val8 = bitcast i8* [[GEP10]] to i32*
; CHECK-NEXT: store i32 %conv1, i32* %arg_val8, align 1
; CHECK-NEXT: [[GEP11:%[0-9]+]] = getelementptr inbounds [88 x i8], [88 x i8]* %temp_arg_buf, i32 0, i32 80
; CHECK-NEXT: %arg_size9 = bitcast i8* [[GEP11]] to i32*
; CHECK-NEXT: store i32 4, i32* %arg_size9, align 4
; CHECK-NEXT: [[GEP12:%[0-9]+]] = getelementptr inbounds [88 x i8], [88 x i8]* %temp_arg_buf, i32 0, i32 84
; CHECK-NEXT: %arg_val10 = bitcast i8* [[GEP12]] to i32*
; CHECK-NEXT: store i32 11121314, i32* %arg_val10, align 1
; CHECK-NEXT: %translated_opencl_printf_call = call i32 @__opencl_printf(i8 addrspace(2)* getelementptr inbounds ([36 x i8], [36 x i8] addrspace(2)* @.str, i64 0, i64 0), i8* [[GEP0]]

; Function Attrs: convergent noinline norecurse nounwind optnone
define dso_local void @test(i8 addrspace(3)* noalias %pLocalMemBase, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }* noalias %pWorkDim, i64* noalias %pWGId, [4 x i64] %BaseGlbId, i8* noalias %pSpecialBuf, {}* noalias %RuntimeHandle) #2 !kernel_arg_addr_space !3 !kernel_arg_access_qual !3 !kernel_arg_type !3 !kernel_arg_base_type !3 !kernel_arg_type_qual !3 !kernel_arg_name !3 !kernel_arg_host_accessible !3 !kernel_arg_pipe_depth !3 !kernel_arg_pipe_io !3 !kernel_arg_buffer_location !3 !kernel_has_sub_groups !4 !barrier_buffer_size !5 !kernel_execution_length !6 !kernel_has_global_sync !4 !recommended_vector_length !7 !private_memory_size !8 {
entry:
  %hhd.addr = alloca i8*, align 8
  %hd.addr = alloca i16*, align 8
  %zd.addr = alloca i64*, align 8
  %tmp.addr = alloca <4 x float>*, align 8
  %.compoundliteral.addr = alloca <4 x float>*, align 8
  %pCurrSBIndex = alloca i64, align 8
  %pSB = call i8* @get_special_buffer.()
  br label %FirstBB

FirstBB:                                          ; preds = %entry
  store i64 0, i64* %pCurrSBIndex, align 8
  br label %SyncBB1

SyncBB1:                                          ; preds = %FirstBB
  %SBIndex9 = load i64, i64* %pCurrSBIndex, align 8
  %SB_LocalId_Offset10 = add nuw i64 %SBIndex9, 32
  %0 = getelementptr inbounds i8, i8* %pSB, i64 %SB_LocalId_Offset10
  %pSB_LocalId11 = bitcast i8* %0 to <4 x float>*
  store <4 x float>* %pSB_LocalId11, <4 x float>** %.compoundliteral.addr, align 8
  %1 = load <4 x float>*, <4 x float>** %.compoundliteral.addr, align 8
  %SBIndex6 = load i64, i64* %pCurrSBIndex, align 8
  %SB_LocalId_Offset7 = add nuw i64 %SBIndex6, 16
  %2 = getelementptr inbounds i8, i8* %pSB, i64 %SB_LocalId_Offset7
  %pSB_LocalId8 = bitcast i8* %2 to <4 x float>*
  store <4 x float>* %pSB_LocalId8, <4 x float>** %tmp.addr, align 8
  %3 = load <4 x float>*, <4 x float>** %tmp.addr, align 8
  %SBIndex3 = load i64, i64* %pCurrSBIndex, align 8
  %SB_LocalId_Offset4 = add nuw i64 %SBIndex3, 8
  %4 = getelementptr inbounds i8, i8* %pSB, i64 %SB_LocalId_Offset4
  %pSB_LocalId5 = bitcast i8* %4 to i64*
  store i64* %pSB_LocalId5, i64** %zd.addr, align 8
  %5 = load i64*, i64** %zd.addr, align 8
  %SBIndex1 = load i64, i64* %pCurrSBIndex, align 8
  %SB_LocalId_Offset2 = add nuw i64 %SBIndex1, 2
  %6 = getelementptr inbounds i8, i8* %pSB, i64 %SB_LocalId_Offset2
  %pSB_LocalId = bitcast i8* %6 to i16*
  store i16* %pSB_LocalId, i16** %hd.addr, align 8
  %7 = load i16*, i16** %hd.addr, align 8
  %SBIndex = load i64, i64* %pCurrSBIndex, align 8
  %SB_LocalId_Offset = add nuw i64 %SBIndex, 0
  %8 = getelementptr inbounds i8, i8* %pSB, i64 %SB_LocalId_Offset
  store i8* %8, i8** %hhd.addr, align 8
  %9 = load i8*, i8** %hhd.addr, align 8
  store i8 123, i8* %9, align 1
  store i16 12345, i16* %7, align 2
  store i64 1234567891011121314, i64* %5, align 8
  store <4 x float> <float 1.000000e+00, float 2.000000e+00, float 3.000000e+00, float 4.000000e+00>, <4 x float>* %1, align 16
  %10 = load <4 x float>, <4 x float>* %1, align 16
  store <4 x float> %10, <4 x float>* %3, align 16
  %11 = load <4 x float>, <4 x float>* %3, align 16
  %12 = load i16, i16* %7, align 2
  %conv = sext i16 %12 to i32
  %13 = load i64, i64* %5, align 8
  %14 = load i8, i8* %9, align 1
  %conv1 = sext i8 %14 to i32
  %printf.promoted = fpext <4 x float> %11 to <4 x double>
  %call = call i32 (i8 addrspace(2)*, ...) @printf(i8 addrspace(2)* getelementptr inbounds ([36 x i8], [36 x i8] addrspace(2)* @.str, i64 0, i64 0), i32 321, <4 x double> %printf.promoted, i32 %conv, i64 %13, i32 %conv1, i32 11121314) #3
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
!2 = !{void (i8 addrspace(3)*, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }*, i64*, [4 x i64], i8*, {}*)* @test}
!3 = !{}
!4 = !{i1 false}
!5 = !{i32 48}
!6 = !{i32 20}
!7 = !{i32 1}
!8 = !{i32 88}

; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test --  %{{.*}} = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }* %pWorkDim, i32 0, i32 5
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test --  %RuntimeInterface = load {}*, {}** %{{.*}}, align 1
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test --  %temp_arg_buf = alloca [88 x i8], align 4
; DEBUGIFY: WARNING: Missing line 7
; DEBUGIFY-NOT: WARNING
