; RUN: llc %s -O2 -mtriple=x86_64-pc-windows-msvc19.20.0 -o - | FileCheck %s

%rtti.TypeDescriptor2 = type { i8**, i8*, [3 x i8] }
%eh.CatchableType = type { i32, i32, i32, i32, i32, i32, i32 }
%eh.CatchableTypeArray.1 = type { i32, [1 x i32] }
%eh.ThrowInfo = type { i32, i32, i32, i32 }

$"??_R0H@8" = comdat any

$"_CT??_R0H@84" = comdat any

$_CTA1H = comdat any

$_TI1H = comdat any

@"??_7type_info@@6B@" = external constant i8*
@"??_R0H@8" = linkonce_odr global %rtti.TypeDescriptor2 { i8** @"??_7type_info@@6B@", i8* null, [3 x i8] c".H\00" }, comdat
@__ImageBase = external dso_local constant i8
@"_CT??_R0H@84" = linkonce_odr unnamed_addr constant %eh.CatchableType { i32 1, i32 trunc (i64 sub nuw nsw (i64 ptrtoint (%rtti.TypeDescriptor2* @"??_R0H@8" to i64), i64 ptrtoint (i8* @__ImageBase to i64)) to i32), i32 0, i32 -1, i32 0, i32 4, i32 0 }, section ".xdata", comdat
@_CTA1H = linkonce_odr unnamed_addr constant %eh.CatchableTypeArray.1 { i32 1, [1 x i32] [i32 trunc (i64 sub nuw nsw (i64 ptrtoint (%eh.CatchableType* @"_CT??_R0H@84" to i64), i64 ptrtoint (i8* @__ImageBase to i64)) to i32)] }, section ".xdata", comdat
@_TI1H = linkonce_odr unnamed_addr constant %eh.ThrowInfo { i32 0, i32 0, i32 0, i32 trunc (i64 sub nuw nsw (i64 ptrtoint (%eh.CatchableTypeArray.1* @_CTA1H to i64), i64 ptrtoint (i8* @__ImageBase to i64)) to i32) }, section ".xdata", comdat

; Function Attrs: mustprogress nofree sspreq uwtable
define dso_local void @"?vuln@@YAXH@Z"(i32 noundef %op) local_unnamed_addr #0 {
; CHECK-LABEL: vuln@@YAXH@Z
entry:
  %tmp = alloca i32, align 4
  %cmp = icmp eq i32 %op, 1
  br i1 %cmp, label %if.then, label %if.end
; CHECK:        movq    __security_cookie(%rip), %rax
; CHECK-NEXT:   xorq    %rsp, %rax
; CHECK-NEXT:   movq    %rax, 48(%rsp)
; CHECK-NEXT:   cmpl    $1, %ecx
; CHECK-NEXT:   je      .LBB0_2

if.then:                                          ; preds = %entry
  store i32 1, i32* %tmp, align 4
  %0 = bitcast i32* %tmp to i8*
  call void @_CxxThrowException(i8* nonnull %0, %eh.ThrowInfo* nonnull @_TI1H) #2
  unreachable

if.end:                                           ; preds = %entry
  ret void
; CHECK:      # %bb.1:                                # %if.end
; CHECK-NEXT:   movq    48(%rsp), %rcx
; CHECK-NEXT:   xorq    %rsp, %rcx
; CHECK-NEXT:   callq   __security_check_cookie
; CHECK-NEXT:   nop
; CHECK-NEXT:   addq    $56, %rsp
; CHECK-NEXT:   retq
; CHECK-NEXT: .LBB0_2:                                # %if.then
; CHECK-NEXT:   movl    $1, 44(%rsp)
; CHECK-NEXT:   movq    48(%rsp), %rcx
; CHECK-NEXT:   xorq    %rsp, %rcx
; CHECK-NEXT:   callq   __security_check_cookie
; CHECK-NEXT:   leaq    _TI1H(%rip), %rdx
; CHECK-NEXT:   leaq    44(%rsp), %rcx
; CHECK-NEXT:   callq   _CxxThrowException

}

define linkonce_odr void @"?vuln2@@YAXH@Z"() #3 {
entry:
  %SaveInfo = alloca i8*, i32 0, align 8
  call void null() #2
  unreachable

; CHECK-LABEL: vuln2@@YAXH@Z
; CHECK:        movq    __security_cookie(%rip), %rax
; CHECK-NEXT:   xorq    %rsp, %rax
; CHECK-NEXT:   movq    %rax, 48(%rsp)
; CHECK-NEXT:   movq    48(%rsp), %rcx
; CHECK-NEXT:   xorq    %rsp, %rcx
; CHECK-NEXT:   callq   __security_check_cookie
; CHECK-NEXT:   xorl    %eax, %eax
; CHECK-NEXT:   callq   *%rax
}

; Note: Force mark tail call on noreturn function call
; remove me if it is illegal.
define linkonce_odr void @"?vuln3@@YAXH@Z"() #3 {
entry:
  %SaveInfo = alloca i8*, i32 0, align 8
  tail call void null() #2
  unreachable

; CHECK-LABEL: vuln3@@YAXH@Z
; CHECK:        movq    __security_cookie(%rip), %rax
; CHECK-NEXT:   xorq    %rsp, %rax
; CHECK-NEXT:   movq    %rax, 48(%rsp)
; CHECK-NEXT:   movq    48(%rsp), %rcx
; CHECK-NEXT:   xorq    %rsp, %rcx
; CHECK-NEXT:   callq   __security_check_cookie
; CHECK-NEXT:   xorl    %eax, %eax
; CHECK-NEXT:   callq   *%rax
}

define linkonce_odr void @"?vuln4@@YAXH@Z"() #3 {
entry:
  %SaveInfo = alloca i8*, i32 0, align 8
  tail call void null() #4
  unreachable

; CHECK-LABEL: vuln4@@YAXH@Z
; CHECK-NOT:   __security_cookie
}

; Function Attrs: nofree noreturn
declare dso_local void @_CxxThrowException(i8*, %eh.ThrowInfo*) local_unnamed_addr #1

attributes #0 = { mustprogress nofree sspreq uwtable "frame-pointer"="none" "stack-protector-buffer-size"="8" }
attributes #1 = { nofree noreturn }
attributes #2 = { noreturn }
attributes #3 = { sspstrong }
attributes #4 = { nounwind noreturn }
