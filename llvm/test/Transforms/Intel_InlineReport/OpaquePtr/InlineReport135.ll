; RUN: opt -opaque-pointers -passes='deadargelim,print<inline-report>' -disable-output -inline-report=0xe801 < %s 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers -passes='inlinereportsetup,deadargelim,inlinereportemitter' -inline-report=0xe880 -S < %s 2>&1 | FileCheck %s

; Check that dead arg elimination does not produce DELETEs in the inlining
; report

; CHECK-LABEL: COMPILE FUNC: test
; CHECK: COMPILE FUNC: hardertest
; CHECK: test
; CHECK-NOT: DELETE: test
; CHECK-LABEL: COMPILE FUNC: evenhardertest
; CHECK: evenhardertest
; CHECK-NOT: DELETE: evenhardertest
; CHECK-LABEL: COMPILE FUNC: needarg
; CHECK: needarg2
; CHECK-NOT: DELETE: needarg2
; CHECK-LABEL: COMPILE FUNC: needarg2
; CHECK-LABEL: COMPILE FUNC: needarg3
; CHECK: needarg
; CHECK-NOT: DELETE: needarg

; test - an obviously dead argument
define internal i32 @test(i32 %v, i32 %DEADARG1, ptr %p) {
        store i32 %v, ptr %p
        ret i32 %v
}

; hardertest - an argument which is only used by a call of a function with a 
; dead argument.
define internal i32 @hardertest(i32 %DEADARG2) {
        %p = alloca i32         ; <ptr> [#uses=1]
        %V = call i32 @test( i32 5, i32 %DEADARG2, ptr %p )            ; <i32> [#uses=1]
        ret i32 %V
}

; evenhardertest - recursive dead argument...
define internal void @evenhardertest(i32 %DEADARG3) {
        call void @evenhardertest( i32 %DEADARG3 )
        ret void
}

define internal void @needarg(i32 %TEST) {
        call i32 @needarg2( i32 %TEST )         ; <i32>:1 [#uses=0]
        ret void
}

define internal i32 @needarg2(i32 %TEST) {
        ret i32 %TEST
}

define internal void @needarg3(i32 %TEST3) {
        call void @needarg( i32 %TEST3 )
        ret void
}

