; This test verifies that call-graph cloning is done correctly for
; DynClone transformation.

;  RUN: opt < %s -opaque-pointers -S -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -intel-libirc-allowed -passes=dtrans-dyncloneop 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; 1 and 6 fields are qualified as final candidates.
%struct.test.01 = type { i32, i64, i32, i32, i16, ptr, i64 }

; Call to "init" routine is qualified as InitRoutine for DynClone.
; Calls to @proc1 and @proc3 are cloned.
define i32 @main() {
entry:
  %j = call ptr @init();

  %r1 = call i32 @proc1(ptr %j);

; @proc1 routine is cloned as ptr since %struct.test.01 is accessed
; in the routine. @proc1 call is specialized and fixed return values.

; CHECK-LABEL: call ptr @init()
; CHECK:   [[LD1:%d.gld[0-9]*]] = load i8, ptr @__Shrink__Happened__
; CHECK-NEXT:  [[CMP1:%d.gc[0-9]*]] = icmp eq i8 [[LD1]], 0
; CHECK: br i1 [[CMP1]], label %d.t{{.*}}, label %d.f{{.*}}
; CHECK: d.t{{.*}}:
; CHECK: [[RET1:%[0-9]+]] = call i32 @proc1.{{.*}}(ptr %j)
; CHECK:   br label %d.m{{.*}}
; CHECK: d.f{{.*}}:
; CHECK: [[RET2:%r1]] = call i32 @proc1(ptr %j)
; CHECK:   br label %d.m{{.*}}
; CHECK: d.m{{.*}}:
; CHECK:  [[PHI1:%d.p[0-9]*]] = phi i32 [ [[RET1]], %d.t{{.*}} ], [ [[RET2]], %d.f{{.*}} ]

  call void @proc2();
; @proc2 routine is not cloned since %struct.test.01 is not accessed.
; CHECK:  call void @proc2()
; CHECK-NOT:  call void @proc2.{{.*}}()

  call void @proc3(ptr %j);

; @proc3 routine is cloned as ptr since %struct.test.01 is accessed
; in the routine. @proc3 call is specialized. No need to handle return
; value since it doesn't return.
; CHECK:  [[LD2:%d.gld[0-9]*]] = load i8, ptr @__Shrink__Happened__
; CHECK:  [[CMP2:%d.gc[0-9]*]] = icmp eq i8 [[LD2]], 0
; CHECK:  br i1 [[CMP2]], label %d.t{{.*}}, label %d.f{{.*}}

; CHECK: d.t{{.*}}:
; CHECK:  call void @proc3.{{.*}}(ptr %j)
; CHECK:  br label %d.m{{.*}}

; CHECK: d.f{{.*}}:
; CHECK:  call void @proc3(ptr %j)
; CHECK:  br label %d.m{{.*}}

  ret i32 %r1

; CHECK: d.m{{.*}}:
; CHECK:   ret i32 [[PHI1]]

}

; This routine just accesses candidate field.
define internal i32 @proc1(ptr "intel_dtrans_func_index"="1" %p10) !intel.dtrans.func.type !6 {
  %F6 = getelementptr %struct.test.01, ptr %p10, i32 0, i32 6

; Calls in this routine are not cloned.
; CHECK-LABEL: define internal i32 @proc1(ptr {{.*}}%p10)
; CHECK: call void @proc3(ptr %p10)
; CHECK: call void @proc4(ptr %p10)
  call void @proc3(ptr %p10);
  call void @proc4(ptr %p10)
  ret i32 20
}

define void @proc2() {
  ret void
}

define internal void @proc3(ptr "intel_dtrans_func_index"="1" %p30) !intel.dtrans.func.type !7 {
  %F6 = getelementptr %struct.test.01, ptr %p30, i32 0, i32 6

; Calls in this routine are not cloned.
; CHECK-LABEL: define internal void @proc3(ptr {{.*}}%p30)
; CHECK: call void @proc3(ptr %p30)
  call void @proc3(ptr %p30);
  ret void
}

define internal void @proc4(ptr "intel_dtrans_func_index"="1" %p40) !intel.dtrans.func.type !8 {
  %F6 = getelementptr %struct.test.01, ptr %p40, i32 0, i32 6
  ret void
}

; All calls in cloned routine of @proc1 are cloned.
; CHECK-LABEL: define internal i32 @proc1.{{.*}}(ptr{{.*}}%p10)
; CHECK:  call void @proc3.{{.*}}(ptr %p10)
; CHECK:  call void @proc4.{{.*}}(ptr %p10)

; @proc3 is recursive routine.  All calls in cloned routine of @proc3
; are cloned.
; CHECK-LABEL: define internal void @proc3.{{.*}}(ptr {{.*}}%p30)
; CHECK:  call void @proc3.{{.*}}(ptr %p30)

; This routine is selected as InitRoutine.
define internal "intel_dtrans_func_index"="1" ptr @init() !intel.dtrans.func.type !9 {
  %tp1 = tail call ptr @calloc(i64 10, i64 48)
  %F1 = getelementptr %struct.test.01, ptr %tp1, i32 0, i32 1
  %g1 = select i1 undef, i64 500, i64 1000
  store i64 %g1, ptr %F1, align 8
  %F6 = getelementptr %struct.test.01, ptr %tp1, i32 0, i32 6
  %g2 = select i1 undef, i64 -5000, i64 20000
  store i64 %g2, ptr %F6, align 8
  ret ptr null
}

; Function Attrs: nounwind
declare !intel.dtrans.func.type !11 "intel_dtrans_func_index"="1" ptr @calloc(i64, i64) #0

attributes #0 = { allockind("alloc,zeroed") allocsize(0,1) "alloc-family"="malloc" }

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{i16 0, i32 0}  ; i16
!4 = !{i64 0, i32 1}  ; i64*
!5 = !{%struct.test.01 zeroinitializer, i32 1}  ; %struct.test.01*
!6 = distinct !{!5}
!7 = distinct !{!5}
!8 = distinct !{!5}
!9 = distinct !{!5}
!10 = !{i8 0, i32 1}  ; i8*
!11 = distinct !{!10}
!12 = !{!"S", %struct.test.01 zeroinitializer, i32 7, !1, !2, !1, !1, !3, !4, !2} ; { i32, i64, i32, i32, i16, i64*, i64 }

!intel.dtrans.types = !{!12}

