; RUN: llc < %s -mtriple=x86_64-linux | FileCheck %s

%struct.__va_list_tag = type { i32, i32, i8*, i8* }

define void @foo(i32 %x, ...) nounwind {
entry:
  %ap = alloca [1 x %struct.__va_list_tag], align 8; <[1 x %struct.__va_list_tag]*> [#uses=2]
  %ap12 = bitcast [1 x %struct.__va_list_tag]* %ap to i8*; <i8*> [#uses=2]
  call void @llvm.va_start(i8* %ap12)

; CHECK-LABEL: foo

; CHECK: testb %al, %al

; These test for specific offsets, which is very fragile. Still, the test needs
; to ensure that va_list has the correct element types.
;
; Update : Changes were made to cause symbols to be allocated in different
; offsets, therefore, some changes were made to this test to make it
; "less" fragile.

; Check for the lea that grabs the address of the first local
; CHECK: leaq {{[0-9]+}}(%rsp), [[LOCAL_OFFS_REG:%.*]]

; Now check that we're storing this into the va_list struct.
; We don't know where the struct is located, so this is the best
; we can do.
; CHECK-NEXT:  movq [[LOCAL_OFFS_REG]], {{[0-9]+}}(%rsp)

; Now check for the lea of the arguments. This will be in the 200's
; somewhere, just to improve the flakiness of this test.
; CHECK: leaq 2{{[0-9][0-9]}}(%rsp), [[ARG_OFFS_REG:%.*]]

; And make sure we store that into the va_list struct.
; CHECK-NEXT: movq [[ARG_OFFS_REG]], {{[0-9]+}}(%rsp)

; Next 2 checks are for the fp/gp regs offsets, which we
; know to be 48 and 8, in this test. Again, we don't know where
; they will be stored.
; CHECK-NEXT: movl $48, {{[0-9]+}}(%rsp)
; CHECK-NEXT: movl $8, {{[0-9]+}}(%rsp)

  %ap3 = getelementptr inbounds [1 x %struct.__va_list_tag], [1 x %struct.__va_list_tag]* %ap, i64 0, i64 0; <%struct.__va_list_tag*> [#uses=1]
  call void @bar(%struct.__va_list_tag* %ap3) nounwind
  call void @llvm.va_end(i8* %ap12)
  ret void
}

declare void @llvm.va_start(i8*) nounwind

declare void @bar(%struct.__va_list_tag*)

declare void @llvm.va_end(i8*) nounwind
