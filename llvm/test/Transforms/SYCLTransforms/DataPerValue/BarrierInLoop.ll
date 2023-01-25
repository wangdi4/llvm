; RUN: opt -disable-output 2>&1 -passes='print<sycl-kernel-data-per-value-analysis>' %s | FileCheck %s

define void @test() {
entry:
  %id = call i64 @_Z13get_global_idj(i32 0)
  br label %loop.body

loop.body:
  %iv = phi i64 [ %id,  %entry ], [ %iv.n, %SyncBB ]
  %iv.n = add i64 %iv, 1
  %id1 = call i64 @_Z13get_global_idj(i32 1)
  br label %middle

middle:
  %use = add i64 %id1, 10
  br label %SyncBB

SyncBB:
  call void @_Z18work_group_barrierj(i32 0)
  %c = icmp eq i64 %iv.n, 10
  br i1 %c, label %r, label %loop.body

r:
  ret void
}

declare i64 @_Z13get_global_idj(i32)
declare void @_Z18work_group_barrierj(i32)

; Checks that %id and %id1 are not in any of Group A, B.1 or B.2.
; CHECK:      Group-A Values
; CHECK-EMPTY:
; CHECK-NEXT: Group-B.1 Values
; CHECK-NEXT: +test
; CHECK-NEXT:   -iv.n   (0)
; CHECK-NEXT: *
; CHECK-EMPTY:
; CHECK-NEXT: Group-B.2 Values
; CHECK-NEXT: Function Equivalence Classes:
; CHECK-NEXT: [test]: test
; CHECK-NEXT: Buffer Total Size:
