; RUN: llc < %s -mtriple=x86_64-apple-darwin -stack-symbol-ordering=0 | FileCheck %s -check-prefix=X64
; RUN: llc < %s -mtriple=i686-apple-darwin -stack-symbol-ordering=0 | FileCheck %s -check-prefix=X32

%struct.Baz = type { [17 x i8] }
%struct.__va_list_tag = type { i32, i32, i8*, i8* }

; Function Attrs: nounwind uwtable
define void @bar(%struct.Baz* byval nocapture readnone align 8 %x, ...) {
entry:
  %va = alloca [1 x %struct.__va_list_tag], align 16
  %arraydecay = getelementptr inbounds [1 x %struct.__va_list_tag], [1 x %struct.__va_list_tag]* %va, i64 0, i64 0
  %arraydecay1 = bitcast [1 x %struct.__va_list_tag]* %va to i8*
  call void @llvm.va_start(i8* %arraydecay1)
  %overflow_arg_area_p = getelementptr inbounds [1 x %struct.__va_list_tag], [1 x %struct.__va_list_tag]* %va, i64 0, i64 0, i32 2
  %overflow_arg_area = load i8*, i8** %overflow_arg_area_p, align 8
  %overflow_arg_area.next = getelementptr i8, i8* %overflow_arg_area, i64 24
  store i8* %overflow_arg_area.next, i8** %overflow_arg_area_p, align 8


; Check that we're taking the address of somewhere in the incoming arg list.
; We don't know the exact offset, because it can vary depending on stack
; layout. We just know that it's in the 60's somewhere (just to make it "less"
; flaky).
; X32: leal    6{{[0-9]}}(%esp), [[REG:%.*]]
; X32: movl    [[REG]], [[LOCAL_OFF:[0-9]*]](%esp)

; Check for taking the address of arraydecay to pass to qux.
; X32: leal    [[LOCAL_OFF]]

; Save the offset for the bottom of register save area
; X64: movq    %rdi, [[OFF_REG:[0-9]*]](%rsp)

; Check that we're taking the address of the register save area
; X64: leaq    [[OFF_REG]](%rsp)

; Check that we're taking the address of somewhere in the incoming arg list.
; We don't know the exact offset, because it can vary depending on stack
; layout. We just know that it's in the 200's somewhere (just to make it "less"
; flaky).
; X64: leaq    2{{[0-9][0-9]}}(%rsp)

; Check store of 0 to the beginning of arraydecay
; X64: movl    $0, [[ARR_OFF:[0-9]*]](%rsp)
; Check to see if that address is then passed to the call _qux
; X64: leaq    [[ARR_OFF]](%rsp), %rdi
  call void @qux(%struct.__va_list_tag* %arraydecay)
  ret void
}

; Function Attrs: nounwind
declare void @llvm.va_start(i8*)

declare void @qux(%struct.__va_list_tag*)
