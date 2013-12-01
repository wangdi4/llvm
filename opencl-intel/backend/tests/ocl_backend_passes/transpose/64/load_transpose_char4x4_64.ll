; XFAIL: i686-pc-win32
; RUN: oclopt -runtimelib=clbltfne9.rtl  -builtin-import -shuffle-call-to-inst  -instcombine -inline -scalarrepl -S %s -o %t1.ll
; RUN: llc < %t1.ll -mattr=+avx -mtriple=x86_64-pc-Win64 | FileCheck %s -check-prefix=CHECK
; RUN: oclopt -runtimelib=clbltfnl9.rtl  -builtin-import -shuffle-call-to-inst  -instcombine -inline -scalarrepl -S %s -o %t2.ll
; RUN: llc < %t2.ll -mattr=+avx2 -mtriple=x86_64-pc-Win64 | FileCheck %s -check-prefix=CHECK

define <4 x i8> @foo(<4 x i8>* %pLoadAdd){
entry:
   %xOut = alloca  <4 x i8>
   %yOut = alloca  <4 x i8>
   %zOut = alloca  <4 x i8>
   %wOut = alloca  <4 x i8>
   call void @__ocl_load_transpose_char_4x4(<4 x i8>* %pLoadAdd, <4 x i8>* nocapture %xOut, <4 x i8>* nocapture %yOut, <4 x i8>* nocapture %zOut, <4 x i8>* nocapture %wOut) nounwind
   %temp1 = load <4 x i8>* %xOut
   %temp2 = load <4 x i8>* %yOut
   %temp3 = load <4 x i8>* %zOut
   %temp4 = load <4 x i8>* %wOut
   %re0 = add <4 x i8> %temp1, %temp2
   %re1 = add <4 x i8> %temp3, %temp4
   %ret0 = add <4 x i8> %re0, %re1
   ret <4 x i8> %ret0
}

declare  void @__ocl_load_transpose_char_4x4(<4 x i8>* %pLoadAdd, <4 x i8>* nocapture %xOut, <4 x i8>* nocapture %yOut, <4 x i8>* nocapture %zOut, <4 x i8>* nocapture %wOut) nounwind




;CHECK:    .type    [[FOO:[_a-z]+]],@function
;CHECK:    vmovdqu	([[MEM:%[a-z]+]]), [[XMM0:%xmm[0-9]+]]
;CHECK:    vpsrldq	[[REG0:[$_0-9]+]], [[XMM0]], [[XMM1:%xmm[0-9]+]]
;CHECK:    vpsrldq	[[REG1:[$_0-9]+]], [[XMM1]], [[XMM2:%xmm[0-9]+]]
;CHECK:    vpsrldq	[[REG2:[$_0-9]+]], [[XMM2]], [[XMM3:%xmm[0-9]+]]
;CHECK:	.type	[[LOAD:[_a-z]+]]_transpose_char_4x4,@function
