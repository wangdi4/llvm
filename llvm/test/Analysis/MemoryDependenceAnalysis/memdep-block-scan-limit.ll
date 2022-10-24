<<<<<<< HEAD
; RUN: opt -S -memdep -gvn -basic-aa < %s | FileCheck %s
; RUN: opt -S -memdep -memdep-block-scan-limit=1 -gvn -basic-aa < %s | FileCheck %s --check-prefix=WITH-LIMIT
=======
; RUN: opt -S -passes=gvn < %s | FileCheck %s
; RUN: opt -S -memdep-block-scan-limit=1 -passes=gvn < %s | FileCheck %s --check-prefix=WITH-LIMIT
>>>>>>> ec9ccb1668f60ae29e2f6c9627142f5ebfe15080
; CHECK-LABEL: @test(
; CHECK: load
; CHECK-NOT: load
; WITH-LIMIT-LABEL: @test(
; WITH-LIMIT-CHECK: load
; WITH-LIMIT-CHECK: load
define i32 @test(i32* %p) {
 %1 = load i32, i32* %p
 %2 = add i32 %1, 3
 %3 = load i32, i32* %p
 %4 = add i32 %2, %3
 ret i32 %4
}
