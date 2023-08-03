; RUN: opt -passes=sycl-kernel-coerce-types -mtriple x86_64-pc-linux -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-coerce-types -mtriple x86_64-pc-linux -S %s -o - | FileCheck %s

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

  call void @single.half(ptr byval(%struct.single.half) align 2 %alloca.struct.single.half)
  call void @half.float(ptr byval(%struct.half.float) align 2 %alloca.struct.half.float)
  call void @half.double(ptr byval(%struct.half.double) align 2 %alloca.struct.half.double)
  call void @float.double(ptr byval(%struct.float.double) align 2 %alloca.struct.float.double)
  call void @half2(ptr byval(%struct.half2) align 2 %alloca.struct.half2)
  call void @half2.float(ptr byval(%struct.half2.float) align 2 %alloca.struct.half2.float)
  call void @half2.double(ptr byval(%struct.half2.double) align 2 %alloca.struct.half2.double)
  call void @half3(ptr byval(%struct.half3) align 2 %alloca.struct.half3)
  call void @half4(ptr byval(%struct.half4) align 2 %alloca.struct.half4)

  ret void
}

declare void @single.half(ptr byval(%struct.single.half) align 2)
; CHECK: declare void @single.half(half)

declare void @half.float(ptr byval(%struct.half.float) align 4)
; CHECK: declare void @half.float(double)

declare void @half.double(ptr byval(%struct.half.double) align 8)
; CHECK: declare void @half.double(half, double)

declare void @float.double(ptr byval(%struct.float.double) align 8)
; CHECK: declare void @float.double(float, double)

declare void @half2(ptr byval(%struct.half2) align 2)
; CHECK: declare void @half2(<2 x half>)

declare void @half2.float(ptr byval(%struct.half2.float) align 4)
; CHECK: declare void @half2.float(double)

declare void @half2.double(ptr byval(%struct.half2.double) align 8)
; CHECK: declare void @half2.double(<2 x half>, double)

declare void @half3(ptr byval(%struct.half3) align 2)
; CHECK: declare void @half3(<3 x half>)

declare void @half4(ptr byval(%struct.half4) align 2)
; CHECK: declare void @half4(<4 x half>)

; DEBUGIFY-NOT: WARNING
