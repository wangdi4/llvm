; RUN: opt -passes='debugify,sycl-kernel-resolve-wi-call,check-debugify' -S %s -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes='sycl-kernel-resolve-wi-call' -S %s | FileCheck %s

; Check that arguments in sycl non-variadic printf are stored into argument
; buffer according to their actual size.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

@_ZZ11do_d_i_testvE4fmt1._AS2 = external hidden unnamed_addr addrspace(2) constant [108 x i8], align 1

; Function Attrs: nounwind
declare i32 @printf(ptr addrspace(2), ...) #0

; CHECK: %temp_arg_buf = alloca [72 x i8], align 4
; CHECK: [[GEP0:%[0-9]+]] = getelementptr inbounds [72 x i8], ptr %temp_arg_buf, i32 0, i32 0
; CHECK-NEXT: store i32 72, ptr [[GEP0]], align 4
; CHECK-NEXT: [[GEP1:%[0-9]+]] = getelementptr inbounds [72 x i8], ptr %temp_arg_buf, i32 0, i32 4
; CHECK-NEXT: store i32 1, ptr [[GEP1]], align 4
; CHECK-NEXT: [[GEP2:%[0-9]+]] = getelementptr inbounds [72 x i8], ptr %temp_arg_buf, i32 0, i32 8
; CHECK-NEXT: store i8 %{{.*}}, ptr [[GEP2]], align 1
; CHECK-NEXT: [[GEP3:%[0-9]+]] = getelementptr inbounds [72 x i8], ptr %temp_arg_buf, i32 0, i32 12
; CHECK-NEXT: store i32 2, ptr [[GEP3]], align 4
; CHECK-NEXT: [[GEP4:%[0-9]+]] = getelementptr inbounds [72 x i8], ptr %temp_arg_buf, i32 0, i32 16
; CHECK-NEXT: store i16 %{{.*}}, ptr [[GEP4]], align 1
; CHECK-NEXT: [[GEP5:%[0-9]+]] = getelementptr inbounds [72 x i8], ptr %temp_arg_buf, i32 0, i32 20
; CHECK-NEXT: store i32 4, ptr [[GEP5]], align 4
; CHECK-NEXT: [[GEP6:%[0-9]+]] = getelementptr inbounds [72 x i8], ptr %temp_arg_buf, i32 0, i32 24
; CHECK-NEXT: store i32 %{{.*}}, ptr [[GEP6]], align 1
; CHECK-NEXT: [[GEP7:%[0-9]+]] = getelementptr inbounds [72 x i8], ptr %temp_arg_buf, i32 0, i32 28
; CHECK-NEXT: store i32 8, ptr [[GEP7]], align 4
; CHECK-NEXT: [[GEP8:%[0-9]+]] = getelementptr inbounds [72 x i8], ptr %temp_arg_buf, i32 0, i32 32
; CHECK-NEXT: store i64 %{{.*}}, ptr [[GEP8]], align 1
; CHECK-NEXT: [[GEP9:%[0-9]+]] = getelementptr inbounds [72 x i8], ptr %temp_arg_buf, i32 0, i32 40
; CHECK-NEXT: store i32 262152, ptr [[GEP9]], align 4
; CHECK-NEXT: [[GEP10:%[0-9]+]] = getelementptr inbounds [72 x i8], ptr %temp_arg_buf, i32 0, i32 48
; CHECK-NEXT: store double %printf.promoted, ptr [[GEP10]], align 1
; CHECK-NEXT: [[GEP11:%[0-9]+]] = getelementptr inbounds [72 x i8], ptr %temp_arg_buf, i32 0, i32 56
; CHECK-NEXT: store i32 262152, ptr [[GEP11]], align 4
; CHECK-NEXT: [[GEP12:%[0-9]+]] = getelementptr inbounds [72 x i8], ptr %temp_arg_buf, i32 0, i32 64
; CHECK-NEXT: store i64 %{{.*}}, ptr [[GEP12]], align 1
; CHECK-NEXT: %translated_opencl_printf_call = call i32 @__opencl_printf(ptr addrspace(2) %7, ptr [[GEP0]]

; Function Attrs: noinline nounwind optnone
define void @_Z11do_d_i_testv(ptr addrspace(3) noalias %pLocalMemBase, ptr noalias %pWorkDim, ptr noalias %pWGId, [4 x i64] %BaseGlbId, ptr noalias %pSpecialBuf, ptr noalias %RuntimeHandle) #1 {
entry:
  %CHAR_VALUE = alloca i8, align 1
  %SHORT_VALUE = alloca i16, align 2
  %INT_VALUE = alloca i32, align 4
  %LONG_LONG_VALUE = alloca i64, align 8
  %hhd = alloca i8, align 1
  %hd = alloca i16, align 2
  %d = alloca i32, align 4
  %jd = alloca i64, align 8
  %td = alloca i64, align 8
  %f = alloca float, align 4
  %CHAR_VALUE.ascast = addrspacecast ptr %CHAR_VALUE to ptr addrspace(4)
  %SHORT_VALUE.ascast = addrspacecast ptr %SHORT_VALUE to ptr addrspace(4)
  %INT_VALUE.ascast = addrspacecast ptr %INT_VALUE to ptr addrspace(4)
  %LONG_LONG_VALUE.ascast = addrspacecast ptr %LONG_LONG_VALUE to ptr addrspace(4)
  %hhd.ascast = addrspacecast ptr %hhd to ptr addrspace(4)
  %hd.ascast = addrspacecast ptr %hd to ptr addrspace(4)
  %d.ascast = addrspacecast ptr %d to ptr addrspace(4)
  %jd.ascast = addrspacecast ptr %jd to ptr addrspace(4)
  %td.ascast = addrspacecast ptr %td to ptr addrspace(4)
  %f.ascast = addrspacecast ptr %f to ptr addrspace(4)
  store i8 123, ptr addrspace(4) %CHAR_VALUE.ascast, align 1
  store i16 12345, ptr addrspace(4) %SHORT_VALUE.ascast, align 2
  store i32 1234567891, ptr addrspace(4) %INT_VALUE.ascast, align 4
  store i64 1234567891011121314, ptr addrspace(4) %LONG_LONG_VALUE.ascast, align 8
  store i8 123, ptr addrspace(4) %hhd.ascast, align 1
  store i16 12345, ptr addrspace(4) %hd.ascast, align 2
  store i32 1234567891, ptr addrspace(4) %d.ascast, align 4
  store i64 1234567891011121314, ptr addrspace(4) %jd.ascast, align 8
  store i64 1234567891011121314, ptr addrspace(4) %td.ascast, align 8
  store float 0x40091EB860000000, ptr addrspace(4) %f.ascast, align 4
  %0 = load i8, ptr addrspace(4) %hhd.ascast, align 1
  %1 = load i16, ptr addrspace(4) %hd.ascast, align 2
  %2 = load i32, ptr addrspace(4) %d.ascast, align 4
  %3 = load i64, ptr addrspace(4) %jd.ascast, align 8
  %4 = load float, ptr addrspace(4) %f.ascast, align 4
  %5 = load i64, ptr addrspace(4) %td.ascast, align 8
  %6 = getelementptr inbounds [108 x i8], ptr addrspace(2) @_ZZ11do_d_i_testvE4fmt1._AS2, i32 0, i32 0
  %printf.promoted = fpext float %4 to double
  %call = call i32 (ptr addrspace(2), ...) @printf(ptr addrspace(2) %6, i8 %0, i16 %1, i32 %2, i64 %3, double %printf.promoted, i64 %5) #2
  ret void
}

attributes #0 = { nounwind "prefer-vector-width"="512" }
attributes #1 = { noinline nounwind optnone "prefer-vector-width"="512" }
attributes #2 = { nobuiltin nounwind }

!spirv.MemoryModel = !{!0}
!spirv.Source = !{!1}
!opencl.spir.version = !{!2}
!opencl.ocl.version = !{!3}
!spirv.Generator = !{!4}
!sycl.kernels = !{!5}

!0 = !{i32 2, i32 2}
!1 = !{i32 4, i32 100000}
!2 = !{i32 1, i32 2}
!3 = !{i32 1, i32 0}
!4 = !{i16 6, i16 14}
!5 = distinct !{null}

; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function _Z11do_d_i_testv --  %{{.*}} = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], ptr, ptr, [3 x i64], [2 x [3 x i64]], [3 x i64] }, ptr %pWorkDim, i32 0, i32 5
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function _Z11do_d_i_testv --  %RuntimeInterface = load ptr, ptr %{{.*}}, align 1
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function _Z11do_d_i_testv --  %temp_arg_buf = alloca [72 x i8], align 4
; DEBUGIFY-NOT: WARNING
