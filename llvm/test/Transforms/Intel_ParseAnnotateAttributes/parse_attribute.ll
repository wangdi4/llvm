;; Check correct parsing from annotation attribute to a function attribute

; There is an implementation of this pass for the new pass manager, but it
; is not in the pass registry, so this run line fails. It's not clear that
; this pass is even used. If it is, the pass registry should be updated. 
; RUN: opt -enable-new-pm=0 -parse-annotate -S < %s | FileCheck %s

@.str = private unnamed_addr constant [12 x i8] c"sycl_kernel\00", section "llvm.metadata"
@.str.1 = private unnamed_addr constant [11 x i8] c"simple.cpp\00", section "llvm.metadata"
@llvm.global.annotations = appending global [1 x { i8*, i8*, i8*, i32 }] [{ i8*, i8*, i8*, i32 } { i8* bitcast (i32 (i32)* @_Z3fooi to i8*), i8* getelementptr inbounds ([12 x i8], [12 x i8]* @.str, i32 0, i32 0), i8* getelementptr inbounds ([11 x i8], [11 x i8]* @.str.1, i32 0, i32 0), i32 3 }], section "llvm.metadata"

define dso_local i32 @_Z3fooi(i32 returned %i) {
entry:
  ret i32 %i
}

; CHECK-NOT: @llvm.global.annotations
; CHECK: attributes #0 = { "sycl_kernel" }

