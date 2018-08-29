; This test verifies that call-graph cloning is done correctly for
; DynClone transformation.

;  RUN: opt < %s -S -whole-program-assume -dtrans-dynclone 2>&1 | FileCheck %s
;  RUN: opt < %s -S -whole-program-assume -passes=dtrans-dynclone 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; 1 and 6 fields are qualified as final candidates.
%struct.test.01 = type { i32, i64, i32, i32, i16, i64*, i64 }

; Call to "init" routine is qualified as InitRoutine for DynClone.
; Calls to @proc1 and @proc3 are cloned.
define i32 @main() {
entry:
  %call1 = tail call noalias i8* @calloc(i64 10, i64 48)
  %j = bitcast i8* %call1 to %struct.test.01*
  call void @init(%struct.test.01* %j);

  %r1 = call i32 @proc1(%struct.test.01* %j);

; @proc1 routine is cloned as @proc1.* since %struct.test.01 is accessed
; in the routine. @proc1 call is specialized and fixed return values.

; CHECK-LABEL: call void @init(%struct.test.01* %j)
; CHECK:   [[LD1:%d.gld[0-9]*]] = load i8, i8* @__Shrink__Happened__
; CHECK-NEXT:  [[CMP1:%d.gc[0-9]*]] = icmp eq i8 [[LD1]], 0
; CHECK: br i1 [[CMP1]], label %d.t{{.*}}, label %d.f{{.*}}
; CHECK: d.t{{.*}}:
; CHECK: [[RET1:%[0-9]+]] = call i32 @proc1.{{.*}}(%struct.test.01* %j)
; CHECK:   br label %d.m{{.*}}
; CHECK: d.f{{.*}}:
; CHECK: [[RET2:%r1]] = call i32 @proc1(%struct.test.01* %j)
; CHECK:   br label %d.m{{.*}}
; CHECK: d.m{{.*}}:
; CHECK:  [[PHI1:%d.p[0-9]*]] = phi i32 [ [[RET1]], %d.t{{.*}} ], [ [[RET2]], %d.f{{.*}} ]

  call void @proc2();
; @proc2 routine is not cloned since %struct.test.01 is not accessed.
; CHECK:  call void @proc2()
; CHECK-NOT:  call void @proc2.{{.*}}()

  call void @proc3(%struct.test.01* %j);

; @proc3 routine is cloned as @proc3.* since %struct.test.01 is accessed
; in the routine. @proc3 call is specialized. No need to handle return
; value since it doesn't return.
; CHECK:  [[LD2:%d.gld[0-9]*]] = load i8, i8* @__Shrink__Happened__
; CHECK:  [[CMP2:%d.gc[0-9]*]] = icmp eq i8 [[LD2]], 0
; CHECK:  br i1 [[CMP2]], label %d.t{{.*}}, label %d.f{{.*}}

; CHECK: d.t{{.*}}:
; CHECK:  call void @proc3.{{.*}}(%struct.test.01* %j)
; CHECK:  br label %d.m{{.*}}

; CHECK: d.f{{.*}}:
; CHECK:  call void @proc3(%struct.test.01* %j)
; CHECK:  br label %d.m{{.*}}

  ret i32 %r1

; CHECK: d.m{{.*}}:
; CHECK:   ret i32 [[PHI1]]

}

; This routine just accesses candidate field.
define internal i32 @proc1(%struct.test.01* %p10) {
  %F6 = getelementptr %struct.test.01, %struct.test.01* %p10, i32 0, i32 6

; Calls in this routine are not cloned.
; CHECK-LABEL: define internal i32 @proc1(%struct.test.01* %p10)
; CHECK: call void @proc3(%struct.test.01* %p10)
; CHECK: call void @proc4(%struct.test.01* %p10)
  call void @proc3(%struct.test.01* %p10);
  call void bitcast (void (%struct.test.01*)* @proc4 to void (%struct.test.01*)*)(%struct.test.01* %p10)
  ret i32 20
}

define void @proc2() {
  ret void
}

define internal void @proc3(%struct.test.01* %p30) {
  %F6 = getelementptr %struct.test.01, %struct.test.01* %p30, i32 0, i32 6

; Calls in this routine are not cloned.
; CHECK-LABEL: define internal void @proc3(%struct.test.01* %p30)
; CHECK: call void @proc3(%struct.test.01* %p30)
  call void @proc3(%struct.test.01* %p30);
  ret void
}

define internal void @proc4(%struct.test.01* %p40) {
  %F6 = getelementptr %struct.test.01, %struct.test.01* %p40, i32 0, i32 6
  ret void
}

; All calls in cloned routine of @proc1 are cloned.
; CHECK-LABEL: define internal i32 @proc1.{{.*}}(%struct.test.01* %p10)
; CHECK:  call void @proc3.{{.*}}(%struct.test.01* %p10)
; CHECK:  call void @proc4.{{.*}}(%struct.test.01* %p10)

; @proc3 is recursive routine.  All calls in cloned routine of @proc3
; are cloned.
; CHECK-LABEL: define internal void @proc3.{{.*}}(%struct.test.01* %p30)
; CHECK:  call void @proc3.{{.*}}(%struct.test.01* %p30)

; This routine is selected as InitRoutine.
define internal void @init(%struct.test.01* %tp1) {
  %F1 = getelementptr %struct.test.01, %struct.test.01* %tp1, i32 0, i32 1
  %g1 = select i1 undef, i64 500, i64 1000
  store i64 %g1, i64* %F1, align 8
  %F6 = getelementptr %struct.test.01, %struct.test.01* %tp1, i32 0, i32 6
  %g2 = select i1 undef, i64 -5000, i64 20000
  store i64 %g2, i64* %F6, align 8
  ret void
}

; Function Attrs: nounwind
declare dso_local noalias i8* @calloc(i64, i64)
