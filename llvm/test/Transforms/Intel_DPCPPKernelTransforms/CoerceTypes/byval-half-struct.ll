; RUN: opt -passes=dpcpp-kernel-coerce-types -mtriple x86_64-pc-linux -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=dpcpp-kernel-coerce-types -mtriple x86_64-pc-linux -S %s -o - | FileCheck %s --check-prefix=X64-LINUX
; RUN: opt -passes=dpcpp-kernel-coerce-win64-types -mtriple x86_64-w64-mingw32 -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=dpcpp-kernel-coerce-win64-types -mtriple x86_64-w64-mingw32 -S %s -o - | FileCheck %s --check-prefixes=X64-WIN,X64-WIN-NONOPAQUE
; RUN: opt -passes=dpcpp-kernel-coerce-types -opaque-pointers -mtriple x86_64-pc-linux -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=dpcpp-kernel-coerce-types -opaque-pointers -mtriple x86_64-pc-linux -S %s -o - | FileCheck %s --check-prefix=X64-LINUX
; RUN: opt -passes=dpcpp-kernel-coerce-win64-types -opaque-pointers -mtriple x86_64-w64-mingw32 -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=dpcpp-kernel-coerce-win64-types -opaque-pointers -mtriple x86_64-w64-mingw32 -S %s -o - | FileCheck %s --check-prefixes=X64-WIN,X64-WIN-OPAQUE

%struct.single.half = type { half } ; 2 bytes
%struct.half.float = type { half, float } ; 2 + 2 (padding) + 4 = 8 bytes
%struct.half.double = type { half, double } ; 2 + 6 (padding) + 8 = 16 bytes
%struct.float.double = type { float, double }
%struct.half2 = type { half, half } ; 4 bytes
%struct.half2.float = type { half, half, float } ; 2 + 2 + 4 = 8 bytes
%struct.half2.double = type { half, half, double } ; 2 + 2 + 4 (padding) + 8 = 16 bytes
%struct.half3 = type { half, half, half } ; 6 bytes
%struct.half4 = type { half, half, half, half } ; 8 bytes

define void @test() {
entry:
  %alloca.struct.single.half = alloca %struct.single.half, align 2
  %alloca.struct.half.float = alloca %struct.half.float, align 4
  %alloca.struct.half.double = alloca %struct.half.double, align 8
  %alloca.struct.float.double = alloca %struct.float.double, align 8
  %alloca.struct.half2 = alloca %struct.half2, align 2
  %alloca.struct.half2.float = alloca %struct.half2.float, align 4
  %alloca.struct.half2.double = alloca %struct.half2.double, align 8
  %alloca.struct.half3 = alloca %struct.half3, align 2
  %alloca.struct.half4 = alloca %struct.half4, align 2

  call void @single.half(%struct.single.half* byval(%struct.single.half) align 2 %alloca.struct.single.half)
  call void @half.float(%struct.half.float* byval(%struct.half.float) align 2 %alloca.struct.half.float)
  call void @half.double(%struct.half.double* byval(%struct.half.double) align 2 %alloca.struct.half.double)
  call void @float.double(%struct.float.double* byval(%struct.float.double) align 2 %alloca.struct.float.double)
  call void @half2(%struct.half2* byval(%struct.half2) align 2 %alloca.struct.half2)
  call void @half2.float(%struct.half2.float* byval(%struct.half2.float) align 2 %alloca.struct.half2.float)
  call void @half2.double(%struct.half2.double* byval(%struct.half2.double) align 2 %alloca.struct.half2.double)
  call void @half3(%struct.half3* byval(%struct.half3) align 2 %alloca.struct.half3)
  call void @half4(%struct.half4* byval(%struct.half4) align 2 %alloca.struct.half4)

  ret void
}

declare void @single.half(%struct.single.half* byval(%struct.single.half) align 2)
; X64-LINUX: declare void @single.half(half)
; X64-WIN: declare void @single.half(i16)

declare void @half.float(%struct.half.float* byval(%struct.half.float) align 4)
; X64-LINUX: declare void @half.float(double)
; X64-WIN: declare void @half.float(i64)

declare void @half.double(%struct.half.double* byval(%struct.half.double) align 8)
; X64-LINUX: declare void @half.double(half, double)
; X64-WIN-NONOPAQUE: declare void @half.double(%struct.half.double*)
; X64-WIN-OPAQUE: declare void @half.double(ptr)

declare void @float.double(%struct.float.double* byval(%struct.float.double) align 8)
; X64-LINUX: declare void @float.double(float, double)
; X64-WIN-NONOPAQUE: declare void @float.double(%struct.float.double*)
; X64-WIN-OPAQUE: declare void @float.double(ptr)

declare void @half2(%struct.half2* byval(%struct.half2) align 2)
; X64-LINUX: declare void @half2(<2 x half>)
; X64-WIN: declare void @half2(i32)

declare void @half2.float(%struct.half2.float* byval(%struct.half2.float) align 4)
; X64-LINUX: declare void @half2.float(double)
; X64-WIN: declare void @half2.float(i64)

declare void @half2.double(%struct.half2.double* byval(%struct.half2.double) align 8)
; X64-LINUX: declare void @half2.double(<2 x half>, double)
; X64-WIN-NONOPAQUE: declare void @half2.double(%struct.half2.double*)
; X64-WIN-OPAQUE: declare void @half2.double(ptr)

declare void @half3(%struct.half3* byval(%struct.half3) align 2)
; X64-LINUX: declare void @half3(<3 x half>)
; X64-WIN-NONOPAQUE: declare void @half3(%struct.half3*)
; X64-WIN-OPAQUE: declare void @half3(ptr)

declare void @half4(%struct.half4* byval(%struct.half4) align 2)
; X64-LINUX: declare void @half4(<4 x half>)
; X64-WIN: declare void @half4(i64)

; DEBUGIFY-NOT: WARNING
