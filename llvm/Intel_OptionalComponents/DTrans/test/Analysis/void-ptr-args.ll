; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; This test verifies that we are correctly analyzing calls to functions with
; void pointer arguments. When a function has a void pointer argument, we need
; to analyze the uses of the argument within the function to determine the
; implied alias set for the argument. For example, if the argument is re-cast
; as a pointer to a structure type within the function, then that structure
; pointer type is added to the implied alias set of the function. This
; information will be used when the function is called to verify that the
; alias set of the value passed to the function has an alias set that is
; compatible with the argument's implied alias set.
;
; For this test to work correctly, the module must be a complete program.


; Test the simple case where the argument matches expectations.

%struct.test01 = type { i32, i32 }
define void @use_test01(i8* %p) {
  %p2 = bitcast i8* %p to %struct.test01*
  %gep = getelementptr %struct.test01, %struct.test01* %p2, i64 0, i32 0
  ret void
}
define void @test01() {
  %p = call i8* @malloc(i64 8)
  ; This is needed to establish %struct.test01* as an aliased type.
  %tmp = bitcast i8* %p to %struct.test01*
  ; This is the instruction we're actually interested in.
  call void @use_test01(i8* %p)
  call void @free(i8* %p)
  ret void
}

; CHECK-LABEL: LLVMType: %struct.test01 = type { i32, i32 }
; CHECK: Safety data: No issues found


; Test the simple negative case where the argument mismatches expectations.

; Call a function with an i64 aliased pointer.
%struct.test02.a = type { i32, i32 }
%struct.test02.b = type { i16, i16, i32 }
define void @use_test02b(i8* %p) {
  %p2 = bitcast i8* %p to %struct.test02.b*
  ret void
}
define void @test02() {
  %p = call i8* @malloc(i64 8)
  ; This is needed to establish %struct.test02.a* as an aliased type.
  %tmp = bitcast i8* %p to %struct.test02.a*
  ; This is the instruction we're actually interested in.
  call void @use_test02b(i8* %p)
  call void @free(i8* %p)
  ret void
}

; CHECK-LABEL: LLVMType: %struct.test02.a = type { i32, i32 }
; CHECK: Safety data: Mismatched argument use
; CHECK: LLVMType: %struct.test02.b = type { i16, i16, i32 }
; CHECK: Safety data: Mismatched argument use


; Test the case where a function is called with two types of values.

%struct.test03.a = type { i32, i32 }
%struct.test03.b = type { i16, i16, i32 }
define void @use_test03b(i8* %p) {
  %p2 = bitcast i8* %p to %struct.test03.b*
  ret void
}
define void @test03() {
  %p1 = call i8* @malloc(i64 8)
  %tmp = bitcast i8* %p1 to %struct.test03.a*
  call void @use_test03b(i8* %p1)
  call void @free(i8* %p1)
  %p2 = call i8* @malloc(i64 8)
  %tmp2 = bitcast i8* %p2 to %struct.test03.b*
  call void @use_test03b(i8* %p2)
  call void @free(i8* %p2)
  ret void
}

; CHECK-LABEL: LLVMType: %struct.test03.a = type { i32, i32 }
; CHECK: Safety data: Mismatched argument use
; CHECK: LLVMType: %struct.test03.b = type { i16, i16, i32 }
; CHECK: Safety data: Mismatched argument use


; Test the case where a function has two different uses for an argument.

%struct.test04.a = type { i32, i32 }
%struct.test04.b = type { i16, i16, i32 }
define void @use_test04ab(i8* %p) {
  %p1 = bitcast i8* %p to %struct.test04.a*
  %p2 = bitcast i8* %p to %struct.test04.b*
  %gep1 = getelementptr %struct.test04.a, %struct.test04.a* %p1, i64 0, i32 0
  %gep2 = getelementptr %struct.test04.b, %struct.test04.b* %p2, i64 0, i32 0
  ret void
}
define void @test04() {
  %p = call i8* @malloc(i64 8)
  %tmp = bitcast i8* %p to %struct.test04.a*
  call void @use_test04ab(i8* %p)
  call void @free(i8* %p)
  ret void
}

; CHECK-LABEL: LLVMType: %struct.test04.a = type { i32, i32 }
; CHECK: Safety data: Bad casting | Ambiguous GEP | Mismatched argument use
; CHECK: LLVMType: %struct.test04.b = type { i16, i16, i32 }
; CHECK: Safety data: Bad casting | Ambiguous GEP | Mismatched argument use


; Test the safe case where the argument is passed through an intermediate
; call that has no requirements.

%struct.test05 = type { i32, i32 }
define void @use_test05(i8* %p) {
  %p2 = bitcast i8* %p to %struct.test05*
  ret void
}
define void @passthru05(i8* %p) {
  call void @use_test05(i8* %p)
  ret void
}
define void @test05() {
  %p = call i8* @malloc(i64 8)
  %tmp = bitcast i8* %p to %struct.test05*
  call void @passthru05(i8* %p)
  call void @free(i8* %p)
  ret void
}

; CHECK-LABEL: LLVMType: %struct.test05 = type { i32, i32 }
; CHECK: Safety data: No issues found


; Test the unsafe case where the argument is passed through an intermediate
; call that has no requirements.

%struct.test06.a = type { i32, i32 }
%struct.test06.b = type { i16, i16, i32 }
define void @use_test06b(i8* %p) {
  %p2 = bitcast i8* %p to %struct.test06.b*
  ret void
}
define void @passthru06(i8* %p) {
  call void @use_test06b(i8* %p)
  ret void
}
define void @test06() {
  %p = call i8* @malloc(i64 8)
  %tmp = bitcast i8* %p to %struct.test06.a*
  call void @passthru06(i8* %p)
  call void @free(i8* %p)
  ret void
}

; CHECK-LABEL: LLVMType: %struct.test06.a = type { i32, i32 }
; CHECK: Safety data: Mismatched argument use
; CHECK: LLVMType: %struct.test06.b = type { i16, i16, i32 }
; CHECK: Safety data: Mismatched argument use


; Test another unsafe case where the argument is passed through an intermediate
; call that has no requirements.

%struct.test07.a = type { i32, i32 }
%struct.test07.b = type { i16, i16, i32 }
define void @use_test07b(i8* %p) {
  %p2 = bitcast i8* %p to %struct.test07.b*
  ret void
}
define void @passthru07a(i8* %p) {
  call void @use_test07b(i8* %p)
  ret void
}
define void @passthru07b(i8* %p) {
  call void @use_test07b(i8* %p)
  ret void
}
define void @test07() {
  %p1 = call i8* @malloc(i64 8)
  %tmp = bitcast i8* %p1 to %struct.test07.a*
  call void @passthru07a(i8* %p1)
  call void @free(i8* %p1)
  %p2 = call i8* @malloc(i64 8)
  %tmp2 = bitcast i8* %p2 to %struct.test07.b*
  call void @passthru07b(i8* %p2)
  call void @free(i8* %p2)
  ret void
}

; CHECK-LABEL: LLVMType: %struct.test07.a = type { i32, i32 }
; CHECK: Safety data: Mismatched argument use
; CHECK: LLVMType: %struct.test07.b = type { i16, i16, i32 }
; CHECK: Safety data: Mismatched argument use


; Test a safe case where the argument identity is established in an
; intermediate function.

%struct.test08 = type { i32, i32 }
define void @use_test08(i8* %p) {
  %p2 = bitcast i8* %p to %struct.test08*
  ret void
}
define void @passthru08(i8* %p) {
  call void @use_test08(i8* %p)
  bitcast i8* %p to %struct.test08*
  ret void
}
define void @test08() {
  %p = call i8* @malloc(i64 8)
  call void @passthru08(i8* %p)
  call void @free(i8* %p)
  ret void
}

; CHECK-LABEL: LLVMType: %struct.test08 = type { i32, i32 }
; CHECK: Safety data: No issues found


; Test an unsafe case where the argument identity is established in an
; intermediate function.

; Call a function with an i64 aliased pointer.
%struct.test09.a = type { i32, i32 }
%struct.test09.b = type { i16, i16, i32 }
define void @use_test09b(i8* %p) {
  %p2 = bitcast i8* %p to %struct.test09.b*
  ret void
}
define void @passthru09a(i8* %p) {
  %tmp = bitcast i8* %p to %struct.test09.a*
  %gep = getelementptr %struct.test09.a, %struct.test09.a* %tmp, i64 0, i32 0
  call void @use_test09b(i8* %p)
  ret void
}
define void @passthru09b(i8* %p) {
  %tmp2 = bitcast i8* %p to %struct.test09.b*
  %gep2 = getelementptr %struct.test09.b, %struct.test09.b* %tmp2, i64 0, i32 0
  call void @use_test09b(i8* %p)
  ret void
}
define void @test09() {
  %p1 = call i8* @malloc(i64 8)
  call void @passthru09a(i8* %p1)
  call void @free(i8* %p1)
  %p2 = call i8* @malloc(i64 8)
  call void @passthru09b(i8* %p2)
  call void @free(i8* %p1)
  ret void
}

; CHECK-LABEL: LLVMType: %struct.test09.a = type { i32, i32 }
; CHECK: Safety data: Bad casting | Ambiguous GEP | Mismatched argument use
; CHECK: LLVMType: %struct.test09.b = type { i16, i16, i32 }
; CHECK: Safety data: Bad casting | Ambiguous GEP | Mismatched argument use


; Test a safe case where the argument identity is conditionally established in
; an intermediate function.

%struct.test10.a = type { i32, i32 }
%struct.test10.b = type { i16, i16, i32 }
define void @use_test10a(i8* %p) {
  %p2 = bitcast i8* %p to %struct.test10.a*
  ret void
}
define void @use_test10b(i8* %p) {
  %p2 = bitcast i8* %p to %struct.test10.b*
  ret void
}
define void @choose10(i8* %p, i32 %n) {
entry:
  %cmp = icmp eq i32 0, %n
  br i1 %cmp, label %callA, label %callB

callA:
  %tmpA = bitcast i8* %p to %struct.test10.a*
  %gep = getelementptr %struct.test10.a, %struct.test10.a* %tmpA, i64 0, i32 0
  call void @use_test10a(i8* %p)
  br label %exit

callB:
  %tmpB = bitcast i8* %p to %struct.test10.b*
  %gep2 = getelementptr %struct.test10.b, %struct.test10.b* %tmpB, i64 0, i32 0
  call void @use_test10b(i8* %p)
  br label %exit

exit:
  ret void
}
define void @test10() {
  %p = call i8* @malloc(i64 8)
  call void @choose10(i8* %p, i32 0)
  call void @free(i8* %p)
  ret void
}

; Although this case is technically safe, proving that would require more
; control flow analysis than we are currently doing. Until this is needed
; we'll just conservatively treat it as unsafe.
; CHECK-LABEL: LLVMType: %struct.test10.a = type { i32, i32 }
; CHECK: Safety data: Bad casting | Ambiguous GEP | Mismatched argument use
; CHECK: LLVMType: %struct.test10.b = type { i16, i16, i32 }
; CHECK: Safety data: Bad casting | Ambiguous GEP | Mismatched argument use


; Test an unsafe case where the argument identity is conditionally established
; in an intermediate function.

%struct.test11.a = type { i32, i32 }
%struct.test11.b = type { i16, i16, i32 }
define void @use_test11a(i8* %p) {
  %p2 = bitcast i8* %p to %struct.test11.a*
  ret void
}
define void @use_test11b(i8* %p) {
  %p2 = bitcast i8* %p to %struct.test11.b*
  ret void
}
define void @choose11(i8* %p, i32 %n) {
entry:
  %cmp = icmp eq i32 0, %n
  br i1 %cmp, label %callA, label %callB

callA:
  %tmpA = bitcast i8* %p to %struct.test11.a*
  %gep1 = getelementptr %struct.test11.a, %struct.test11.a* %tmpA, i64 0, i32 0
  call void @use_test11b(i8* %p)
  br label %exit

callB:
  %tmpB = bitcast i8* %p to %struct.test11.b*
  %gep2 = getelementptr %struct.test11.b, %struct.test11.b* %tmpB, i64 0, i32 0
  call void @use_test11a(i8* %p)
  br label %exit

exit:
  ret void
}
define void @test11() {
  %p = call i8* @malloc(i64 8)
  call void @choose11(i8* %p, i32 1)
  call void @free(i8* %p)
  ret void
}

; CHECK-LABEL: LLVMType: %struct.test11.a = type { i32, i32 }
; CHECK: Safety data: Bad casting | Ambiguous GEP | Mismatched argument use
; CHECK: LLVMType: %struct.test11.b = type { i16, i16, i32 }
; CHECK: Safety data: Bad casting | Ambiguous GEP | Mismatched argument use


; Test the case where the argument is passed through an intermediate
; call that calls another function using a function bitcast.

%struct.test12 = type { i32, i32 }
define void @use_test12(%struct.test12* %p) {
  ret void
}
define void @passthru12(i8* %p) {
  call void bitcast (void (%struct.test12*)* @use_test12 to void (i8*)*)(i8* %p)
  ret void
}
define void @test12() {
  %p = call i8* @malloc(i64 8)
  %tmp = bitcast i8* %p to %struct.test12*
  call void @passthru12(i8* %p)
  call void @free(i8* %p)
  ret void
}

; CHECK-LABEL: LLVMType: %struct.test12 = type { i32, i32 }
; CHECK: Safety data: No issues found


; Test the case where the called function is varadic but the i8* argument
; is in a fixed location.

%struct.test13 = type { i32, i32 }
define void @test13_var(i32 %n, i8* %p, ...) {
  %p1 = bitcast i8* %p to %struct.test13*
  ret void
}
define void @test13() {
  %p = call i8* @malloc(i64 8)
  %tmp = bitcast i8* %p to %struct.test13*
  call void (i32, i8*, ...) @test13_var(i32 0, i8* %p, i32 3)
  ret void
}

; CHECK-LABEL: LLVMType: %struct.test13 = type { i32, i32 }
; CHECK: Safety data: No issues found


; Test the case where the called function is varadic and the i8* argument
; is in a vararg. This isn't supported at this time.

%struct.test14 = type { i32, i32 }
%struct.__va_list_tag = type { i32, i32, i8*, i8* }
define void @test14_var(i32 %n, ...) { ret void }
define void @test14() {
  %p = call i8* @malloc(i64 8)
  %tmp = bitcast i8* %p to %struct.test14*
  call void (i32, ...) @test14_var(i32 0, i8* %p, i32 3)
  ret void
}

; CHECK-LABEL: LLVMType: %struct.test14 = type { i32, i32 }
; CHECK: Safety data: Address taken


; Test a case where the argument is passed through an intermediate function
; that calls a varadic function with the i8* value in a fixed parameter.

%struct.test15 = type { i32, i32 }
define void @use_test15(i8* %p, ...) {
  %p2 = bitcast i8* %p to %struct.test15*
  ret void
}
define void @passthru15(i8* %p) {
  call void (i8*, ...) @use_test15(i8* %p, i32 0, i32 1)
  ret void
}
define void @test15() {
  %p = call i8* @malloc(i64 8)
  bitcast i8* %p to %struct.test15*
  call void @passthru15(i8* %p)
  call void @free(i8* %p)
  ret void
}

; CHECK-LABEL: LLVMType: %struct.test15 = type { i32, i32 }
; CHECK: Safety data: No issues found


; Test a case where the argument is passed through an intermediate function
; that calls a varadic function with the i8* value in a non-fixed parameter.

%struct.test16 = type { i32, i32 }
define void @use_test16(i32 %n, ...) {
  ret void
}
define void @passthru16(i8* %p) {
  call void (i32, ...) @use_test16(i32 1, i8* %p)
  ret void
}
define void @test16() {
  %p = call i8* @malloc(i64 8)
  bitcast i8* %p to %struct.test16*
  call void @passthru16(i8* %p)
  call void @free(i8* %p)
  ret void
}

; CHECK-LABEL: LLVMType: %struct.test16 = type { i32, i32 }
; CHECK: Safety data: Mismatched argument use


; Test a case where the argument is passed through an intermediate function
; that calls a varadic function with a bitcast and an i8* value in a fixed
; parameter.

%struct.test17 = type { i32, i32 }
define void @use_test17(%struct.test17* %p, ...) {
  ret void
}
define void @passthru17(i8* %p) {
  call void (i8*, ...) bitcast (void (%struct.test17 *, ...)*
                                    @use_test17
                                  to void (i8*, ...)*)
                         (i8* %p, i32 0, i32 1)
  ret void
}
define void @test17() {
  %p = call i8* @malloc(i64 8)
  bitcast i8* %p to %struct.test17*
  call void @passthru17(i8* %p)
  call void @free(i8* %p)
  ret void
}

; CHECK-LABEL: LLVMType: %struct.test17 = type { i32, i32 }
; CHECK: Safety data: No issues found


; Test a case where the argument is passed through an intermediate function
; that calls a varadic function with a bitcast and an i8* value in a non-fixed
; parameter.

%struct.test18 = type { i32, i32 }
define void @use_test18(%struct.test18* %p, ...) {
  ret void
}
define void @passthru18(i8* %p) {
  call void (i8*, ...) bitcast (void (%struct.test18 *, ...)*
                                    @use_test18
                                  to void (i8*, ...)*)
                         (i8* null, i8* %p)
  ret void
}
define void @test18() {
  %p = call i8* @malloc(i64 8)
  bitcast i8* %p to %struct.test18*
  call void @passthru18(i8* %p)
  call void @free(i8* %p)
  ret void
}

; CHECK-LABEL: LLVMType: %struct.test18 = type { i32, i32 }
; CHECK: Safety data: Mismatched argument use


; Test a case where a non-varadic function is bitcast as a varadic function.
; This actually happens in one of the cpu2017 benchmarks.

%struct.test19 = type { i32, i32 }
define void @use_test19() {
  ret void
}
define void @passthru19(i8* %p) {
  call void (i32, i8*, ...) bitcast (void ()* @use_test19
                                  to void (i32, i8*, ...)*)
                         (i32 0, i8* %p, i32 1)
  ret void
}
define void @test19() {
  %p = call i8* @malloc(i64 8)
  bitcast i8* %p to %struct.test19*
  call void @passthru19(i8* %p)
  call void @free(i8* %p)
  ret void
}

; CHECK-LABEL: LLVMType: %struct.test19 = type { i32, i32 }
; CHECK: Safety data: Mismatched argument use


; This is just here to complete the module.
define void @main() {
  call void @test01()
  call void @test02()
  call void @test03()
  call void @test04()
  call void @test05()
  call void @test06()
  call void @test07()
  call void @test08()
  call void @test09()
  call void @test10()
  call void @test11()
  call void @test12()
  call void @test13()
  call void @test14()
  call void @test15()
  call void @test16()
  call void @test17()
  call void @test18()
  call void @test19()
  ret void
}

declare i8* @malloc(i64)
declare void @free(i8*)
