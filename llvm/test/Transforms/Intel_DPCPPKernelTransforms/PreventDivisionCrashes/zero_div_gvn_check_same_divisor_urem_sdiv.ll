; RUN: opt -passes=dpcpp-kernel-prevent-div-crashes,gvn -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=dpcpp-kernel-prevent-div-crashes,gvn -S %s -o - | FileCheck %s
; RUN: opt -enable-new-pm=0 -dpcpp-kernel-prevent-div-crashes -gvn -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -enable-new-pm=0 -dpcpp-kernel-prevent-div-crashes -gvn -S %s -o - | FileCheck %s

; CHECK: @sample_test
define void @sample_test(i32 %x, i32 %y, i32 addrspace(1)* nocapture %res) nounwind {
entry:
  %rem = urem i32 %x, %y
  %div = sdiv i32 %x, %y
  %add = add i32 %rem, %div
  store i32 %add, i32 addrspace(1)* %res
  ret void
}

; CHECK: 		[[IS_DIVISOR_ZERO:%[a-zA-Z0-9]+]] = icmp eq i32 %y, 0
; CHECK-NEXT: 	[[NEW_DIVISOR_UREM:%[a-zA-Z0-9]+]] = select i1 [[IS_DIVISOR_ZERO]], i32 1, i32 %y
; CHECK-NEXT: 	urem i32 %x, [[NEW_DIVISOR_UREM]]
; CHECK-NEXT: 	[[IS_DIVISOR_NEG_ONE:%[a-zA-Z0-9]+]] = icmp eq i32 %y, -1
; CHECK-NEXT: 	[[IS_DIVIDEND_MIN_INT:%[a-zA-Z0-9]+]] = icmp eq i32 %x, -2147483648
; CHECK-NEXT: 	[[IS_INTEGER_OVERFLOW:%[a-zA-Z0-9]+]] = and i1 [[IS_DIVISOR_NEG_ONE]], [[IS_DIVIDEND_MIN_INT]]
; CHECK-NEXT: 	[[IS_DIVISOR_BAD:%[a-zA-Z0-9]+]] = or i1 [[IS_INTEGER_OVERFLOW]], [[IS_DIVISOR_ZERO]]
; CHECK-NEXT: 	[[NEW_DIVISOR_SDIV:%[a-zA-Z0-9]+]] = select i1 [[IS_DIVISOR_BAD]], i32 1, i32 %y
; CHECK-NEXT: 	sdiv i32 %x, [[NEW_DIVISOR_SDIV]]



; DEBUGIFY-NOT: WARNING
