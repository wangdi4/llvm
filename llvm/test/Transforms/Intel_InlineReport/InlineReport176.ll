; RUN: opt -aa-pipeline=basic-aa -passes='attributor,print<inline-report>' -attributor-manifest-internal -attributor-annotate-decl-cs  -attributor-allow-deep-wrappers -S -inline-report=0xf847 < %s 2>&1 | FileCheck --check-prefix=CHECK-CL %s
; RUN: opt -aa-pipeline=basic-aa -passes='inlinereportsetup,attributor,inlinereportemitter' -attributor-manifest-internal  -attributor-annotate-decl-cs  -attributor-allow-deep-wrappers -S -inline-report=0xf8c6 < %s 2>&1 | FileCheck --check-prefix=CHECK-MD %s

; This is modified from llvm/test/Transforms/Attributor/internalize.ll and
; tests the inline report for the attributor's internalization.

; CHECK-CL: DEAD STATIC FUNC: __clang_call_terminate
; CHECK-CL: DEAD STATIC FUNC: __clang_call_terminate.internalized
; CHECK-CL: DEAD STATIC FUNC: inner3
; CHECK-CL: DEAD STATIC FUNC: inner4
; CHECK-CL: DEAD STATIC FUNC: unused_arg
; CHECK-CL: DEAD STATIC FUNC: unused_arg.internalized
; CHECK-CL: COMPILE FUNC: outer1
; CHECK-CL: inner1
; CHECK-CL: inner2
; CHECK-CL: inner3.internalized
; CHECK-CL: inner4.internalized
; CHECK-CL: COMPILE FUNC: inner1
; CHECK-CL: COMPILE FUNC: inner2
; CHECK-CL: COMPILE FUNC: inner3.internalized
; CHECK-CL: COMPILE FUNC: inner4.internalized
; CHECK-CL: COMPILE FUNC: unused_arg_caller
; CHECK-CL: DELETE: unused_arg.internalized
; CHECK-CL: COMPILE FUNC: inner5

; CHECK-MD: COMPILE FUNC: inner1
; CHECK-MD: COMPILE FUNC: inner2
; CHECK-MD: DEAD STATIC FUNC: inner3
; CHECK-MD: DEAD STATIC FUNC: inner4
; CHECK-MD: COMPILE FUNC: inner5
; CHECK-MD: COMPILE FUNC: outer1
; CHECK-MD: inner1
; CHECK-MD: inner2
; CHECK-MD: inner3.internalized
; CHECK-MD: inner4.internalized
; CHECK-MD: DEAD STATIC FUNC: unused_arg
; CHECK-MD: COMPILE FUNC: unused_arg_caller
; CHECK-MD: DELETE: unused_arg.internalized
; CHECK-MD: DEAD STATIC FUNC: __clang_call_terminate
; CHECK-MD: COMPILE FUNC: inner3.internalized
; CHECK-MD: COMPILE FUNC: inner4.internalized
; CHECK-MD: DEAD STATIC FUNC: unused_arg.internalized
; CHECK-MD: DEAD STATIC FUNC: __clang_call_terminate.internalized

; Deep Wrapper enabled

; TEST 1: This function is of linkage `linkonce`, we cannot internalize this
;         function and use information derived from it
;
define linkonce i32 @inner1(i32 %a, i32 %b) {
entry:
  %c = add i32 %a, %b
  ret i32 %c
}

; TEST 2: This function is of linkage `weak`, we cannot internalize this function and
;         use information derived from it
;
define weak i32 @inner2(i32 %a, i32 %b) {
entry:
  %c = add i32 %a, %b
  ret i32 %c
}

; TEST 3: This function is of linkage `linkonce_odr`, which can be internalized using the
;         deep wrapper, and the IP information derived from this function can be used
;
define linkonce_odr i32 @inner3(i32 %a, i32 %b) {
entry:
  %c = add i32 %a, %b
  ret i32 %c
}

; TEST 4: This function is of linkage `weak_odr`, which can be internalized using the deep
;         wrapper
;
define weak_odr i32 @inner4(i32 %a, i32 %b) {
entry:
  %c = add i32 %a, %b
  ret i32 %c
}

; TEST 5: This function has linkage `linkonce_odr` but is never called (num of use = 0), so there
;         is no need to internalize this
;
define linkonce_odr i32 @inner5(i32 %a, i32 %b) {
entry:
  %c = add i32 %a, %b
  ret i32 %c
}

; Since the inner1 cannot be internalized, there should be no change to its callsite
; Since the inner2 cannot be internalized, there should be no change to its callsite
; Since the inner3 is internalized, the use of the original function should be replaced by the
;  copied one
;
define i32 @outer1() {
entry:
  %ret1 = call i32 @inner1(i32 1, i32 2)
  %ret2 = call i32 @inner2(i32 1, i32 2)
  %ret3 = call i32 @inner3(i32 %ret1, i32 %ret2)
  %ret4 = call i32 @inner4(i32 %ret3, i32 %ret3)
  ret i32 %ret4
}


define linkonce_odr void @unused_arg(i8) {
  unreachable
}

define void @unused_arg_caller() {
  call void @unused_arg(i8 0)
  ret void
}

; Don't crash on linkonce_odr hidden functions
define linkonce_odr hidden void @__clang_call_terminate() {
  call void @__clang_call_terminate()
  unreachable
}

; CGSCC_ENABLED: attributes #[[ATTR0:[0-9]+]] = { nofree nosync nounwind readnone willreturn }
; CGSCC_ENABLED: attributes #[[ATTR1:[0-9]+]] = { nofree noreturn nosync nounwind readnone willreturn }
; CGSCC_ENABLED: attributes #[[ATTR2]] = { nofree norecurse noreturn nosync nounwind readnone willreturn }
;.
; CHECK_ENABLED: attributes #[[ATTR0]] = { mustprogress nofree norecurse nosync nounwind willreturn memory(none) }
; CHECK_ENABLED: attributes #[[ATTR1:[0-9]+]] = { nounwind memory(none) }
;.
