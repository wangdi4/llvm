; RUN: llc -mcpu=nehalem < %s | FileCheck %s

; XFAIL:
; CSSD100005336 - Bad scheduling of ldmxcsr instruction

; This is the valid ordering which we expect to see in the generated code:
; ...
; stmcsr
; ...
; ldmxcsr
; ...
; cvtpd2ps
; ...
; ldmxcsr

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-pc-win32"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>
%struct.LocalId = type <{ [3 x i64] }>
%struct.WorkDim = type { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }

declare void @__test_convert_float2_rtp_double2_original(<2 x double> addrspace(
1)* nocapture, <2 x float> addrspace(1)* nocapture) nounwind

declare i64 @get_global_id(i32)

define double @_Z18convert_float2_rtpU8__vector2d(<2 x double> %x) nounwind {
entry:
  %tmp1.i.i9 = alloca i32, align 4                ; <i32*> [#uses=2]
  %tmp1.i.i = alloca i32, align 4                 ; <i32*> [#uses=2]
  %tmp.i.i = alloca i32, align 4                  ; <i32*> [#uses=2]
  %0 = bitcast i32* %tmp.i.i to i8*               ; <i8*> [#uses=1]
  call void @llvm.x86.sse.stmxcsr(i8* %0) nounwind
  %stmxcsr.i.i = load i32* %tmp.i.i               ; <i32> [#uses=2]
  %and = and i32 %stmxcsr.i.i, -24577             ; <i32> [#uses=1]
  %or = or i32 %and, 16384                        ; <i32> [#uses=1]
  store i32 %or, i32* %tmp1.i.i9
  %1 = bitcast i32* %tmp1.i.i9 to i8*             ; <i8*> [#uses=1]
  call void @llvm.x86.sse.ldmxcsr(i8* %1) nounwind
  %2 = call <4 x float> @llvm.x86.sse2.cvtpd2ps(<2 x double> %x) nounwind ; <<4 x float>> [#uses=1]
  store i32 %stmxcsr.i.i, i32* %tmp1.i.i
  %3 = bitcast i32* %tmp1.i.i to i8*              ; <i8*> [#uses=1]
  call void @llvm.x86.sse.ldmxcsr(i8* %3) nounwind
  %tmp6 = shufflevector <4 x float> %2, <4 x float> undef, <2 x i32> <i32 0, i32 1> ; <<2 x float>> [#uses=1]
  %tmp8 = bitcast <2 x float> %tmp6 to <1 x double> ; <<1 x double>> [#uses=1]
  %tmp7 = extractelement <1 x double> %tmp8, i32 0 ; <double> [#uses=1]
  ret double %tmp7

; CHECK: _Z18convert_float2_rtpU8__vector2d:
; CHECK: stmxcsr
; CHECK: ldmxcsr
; CHECK: cvtpd2ps
; CHECK: ldmxcsr
; CHECK: ret
}

declare <4 x float> @llvm.x86.sse2.cvtpd2ps(<2 x double>) nounwind readnone

declare void @llvm.x86.sse.ldmxcsr(i8*) nounwind

declare void @llvm.x86.sse.stmxcsr(i8*) nounwind

define  void @test_convert_float2_rtp_double2(<2 x double> addrspace(1)* nocapture %src, <2 x float> addrspace(1)* nocapture %dest, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, i64* %pBaseGlbId, %struct.LocalId* byval %LocalIds) nounwind {
entry:
  %tmp1.i.i9.i = alloca i32, align 4              ; <i32*> [#uses=2]
  %tmp1.i.i.i = alloca i32, align 4               ; <i32*> [#uses=2]
  %tmp.i.i.i = alloca i32, align 4                ; <i32*> [#uses=2]
  %0 = getelementptr %struct.LocalId* %LocalIds, i64 0, i32 0, i64 0 ; <i64*> [#uses=1]
  %1 = load i64* %0                               ; <i64> [#uses=1]
  %2 = load i64* %pBaseGlbId                      ; <i64> [#uses=1]
  %3 = add i64 %1, %2                             ; <i64> [#uses=2]
  %arrayidx = getelementptr inbounds <2 x double> addrspace(1)* %src, i64 %3 ; <<2 x double> addrspace(1)*> [#uses=1]
  %tmp2 = load <2 x double> addrspace(1)* %arrayidx ; <<2 x double>> [#uses=1]
  %4 = bitcast i32* %tmp.i.i.i to i8*             ; <i8*> [#uses=1]
  call void @llvm.x86.sse.stmxcsr(i8* %4) nounwind
  %stmxcsr.i.i.i = load i32* %tmp.i.i.i           ; <i32> [#uses=2]
  %and.i = and i32 %stmxcsr.i.i.i, -24577         ; <i32> [#uses=1]
  %or.i = or i32 %and.i, 16384                    ; <i32> [#uses=1]
  store i32 %or.i, i32* %tmp1.i.i9.i
  %5 = bitcast i32* %tmp1.i.i9.i to i8*           ; <i8*> [#uses=1]
  call void @llvm.x86.sse.ldmxcsr(i8* %5) nounwind
  %6 = call <4 x float> @llvm.x86.sse2.cvtpd2ps(<2 x double> %tmp2) nounwind ; <<4 x float>> [#uses=1]
  store i32 %stmxcsr.i.i.i, i32* %tmp1.i.i.i
  %7 = bitcast i32* %tmp1.i.i.i to i8*            ; <i8*> [#uses=1]
  call void @llvm.x86.sse.ldmxcsr(i8* %7) nounwind
  %tmp6.i = shufflevector <4 x float> %6, <4 x float> undef, <2 x i32> <i32 0, i32 1> ; <<2 x float>> [#uses=1]
  %tmp8.i = bitcast <2 x float> %tmp6.i to <1 x double> ; <<1 x double>> [#uses=1]
  %tmp7.i = extractelement <1 x double> %tmp8.i, i32 0 ; <double> [#uses=1]
  %tmp1 = bitcast double %tmp7.i to <2 x float>   ; <<2 x float>> [#uses=1]
  %arrayidx7 = getelementptr inbounds <2 x float> addrspace(1)* %dest, i64 %3 ;<<2 x float> addrspace(1)*> [#uses=1]
  store <2 x float> %tmp1, <2 x float> addrspace(1)* %arrayidx7
  ret void

; CHECK: test_convert_float2_rtp_double2:
; CHECK: stmxcsr
; CHECK: ldmxcsr
; CHECK: cvtpd2ps
; CHECK: ldmxcsr
; CHECK: ret
}
