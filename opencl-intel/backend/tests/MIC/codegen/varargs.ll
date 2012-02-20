; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF
;
; RUNc: llc < %s -mtriple=x86_64-pc-linux \
; RUNc:       -march=y86-64 -mcpu=knc \
; RUNc:     | FileCheck %s -check-prefix=KNC
;

target datalayout = "e-p:64:64"

%struct.__va_list_tag = type { i32, i32, i8*, i8* }

define i32 @va1(i8* nocapture %format, i8* nocapture %types, i32 %width, ...) nounwind {
; KNF: va1:
; KNF: vstored %v0,
;
; KNC: va1:
; KNC: vmovaps %zmm0
entry:
  %ap = alloca [1 x %struct.__va_list_tag], align 8
  %arraydecay2 = bitcast [1 x %struct.__va_list_tag]* %ap to i8*
  call void @llvm.va_start(i8* %arraydecay2)

  %gp_offset_p = getelementptr inbounds [1 x %struct.__va_list_tag]* %ap, i64 0, i64 0, i32 0
  %gp_offset = load i32* %gp_offset_p, align 4

  %arraydecay459460 = bitcast [1 x %struct.__va_list_tag]* %ap to i8*
  call void @llvm.va_end(i8* %arraydecay459460)

  ret i32 %gp_offset
}

declare void @llvm.va_start(i8*) nounwind
declare void @llvm.va_end(i8*) nounwind
