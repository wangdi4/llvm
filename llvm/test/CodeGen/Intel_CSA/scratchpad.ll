; RUN: llc <%s | FileCheck %s
source_filename = "spad.c"
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

@spad = internal unnamed_addr addrspace(1024) constant [8 x float] [float 0.000000e+00, float -0.000000e+00, float 0x7FF0000000000000, float 0xFFF0000000000000, float 1.000000e+00, float -1.000000e+00, float 0x36A0000000000000, float 0x7FF8000000000000], align 16

; CHECK: .module
; CHECK: .rom .f32 spad[8]
; CHECK: .value 0x00000000
; CHECK: .value 0x80000000
; CHECK: .value 0x7f800000
; CHECK: .value 0xff800000
; CHECK: .value 0x3f800000
; CHECK: .value 0xbf800000
; CHECK: .value 0x00000001
; CHECK: .value 0x7fc00000

; There should be only one use of ldsp32
; CHECK: ldsp32 %[[OUT:[A-Za-z0-9_.]+]], spad
; CHECK-NOT: ldsp32

; Function Attrs: norecurse nounwind readnone uwtable
define dso_local float @test_pattern(i32 %i, i32 %j, i32 %k) local_unnamed_addr {
entry:
  %idxprom = sext i32 %i to i64
  %arrayidx = getelementptr inbounds [8 x float], [8 x float] addrspace(1024)* @spad, i64 0, i64 %idxprom
  %0 = load float, float addrspace(1024)* %arrayidx, align 4
  %idxprom1 = sext i32 %j to i64
  %arrayidx2 = getelementptr inbounds [8 x float], [8 x float] addrspace(1024)* @spad, i64 0, i64 %idxprom1
  %1 = load float, float addrspace(1024)* %arrayidx2, align 4
  %idxprom3 = sext i32 %k to i64
  %arrayidx4 = getelementptr inbounds [8 x float], [8 x float] addrspace(1024)* @spad, i64 0, i64 %idxprom3
  %2 = load float, float addrspace(1024)* %arrayidx4, align 4
  %mul = fmul float %1, %2
  %add = fadd float %0, %mul
  ret float %add
}

