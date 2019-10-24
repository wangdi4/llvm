; RUN: %oclopt -runtimelib=%p/../../vectorizer/Full/runtime.bc -channel-pipe-transformation -S %s -o - | FileCheck %s
; This test checks channel write functions transformation when type-coerced struct that contains pointer member is passed as argument.

; ----------------------------------------------------
; Compiled from:
; ----------------------------------------------------
; struct TS {
;   int *p;
;   char t[4];
; };
; #pragma OPENCL EXTENSION cl_intel_channels : enable
; channel struct TS chan;
; kernel void writer(__global struct TS *src) {
;   write_channel_intel(chan, *src);
; }
; ----------------------------------------------------

%opencl.channel_t = type opaque
%struct.TS = type { i32*, [4 x i8] }
%struct.TS.coerce = type { i32*, i64 }
; CHECK: %struct.channelpipetransformation.merge = type { i32*, i64 }

@chan = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 8, !packet_size !0, !packet_align !1

; Function Attrs: convergent noinline nounwind
define void @writer(%struct.TS addrspace(1)* %src) {
entry:
  ; CHECK: %write.src = alloca %struct.channelpipetransformation.merge
  %src.addr = alloca %struct.TS addrspace(1)*, align 8
  %byval-temp = alloca %struct.TS, align 8
  store %struct.TS addrspace(1)* %src, %struct.TS addrspace(1)** %src.addr, align 8
  %0 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @chan, align 8
  %1 = load %struct.TS addrspace(1)*, %struct.TS addrspace(1)** %src.addr, align 8
  %2 = bitcast %struct.TS* %byval-temp to i8*
  %3 = bitcast %struct.TS addrspace(1)* %1 to i8 addrspace(1)*
  %4 = bitcast %struct.TS* %byval-temp to %struct.TS.coerce*
  %5 = getelementptr %struct.TS.coerce, %struct.TS.coerce* %4, i32 0, i32 0
  %6 = load i32*, i32** %5
  %7 = getelementptr %struct.TS.coerce, %struct.TS.coerce* %4, i32 0, i32 1
  %8 = load i64, i64* %7
  ; CHECK: [[GEP1:%[a-zA-Z0-9]+]] = getelementptr %struct.channelpipetransformation.merge, %struct.channelpipetransformation.merge* %write.src, i32 0, i32 0
  ; CHECK-NEXT: store i32* [[SRC1:%[a-zA-Z0-9]+]], i32** [[GEP1]]
  ; CHECK-NEXT: [[GEP2:%[a-zA-Z0-9]+]] = getelementptr %struct.channelpipetransformation.merge, %struct.channelpipetransformation.merge* %write.src, i32 0, i32 1
  ; CHECK-NEXT: store i64 [[SRC2:%[a-zA-Z0-9]+]], i64* [[GEP2]]
  ; CHECK-NEXT: bitcast %opencl.pipe_rw_t
  ; CHECK-NEXT: addrspacecast %struct.channelpipetransformation.merge* %write.src to
  call void @_Z19write_channel_intel11ocl_channel2TSS_(%opencl.channel_t addrspace(1)* %0, i32* %6, i64 %8)
  ret void
}

; Function Attrs: convergent
declare void @_Z19write_channel_intel11ocl_channel2TSS_(%opencl.channel_t addrspace(1)* %0, i32* %1, i64 %2)

!0 = !{i32 16}
!1 = !{i32 8}
