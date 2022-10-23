; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; RUN: opt < %s -passes='module(ip-cloning),cgscc(inline)' -ip-gen-cloning-force-enable-dtrans -inline-report=0xe807 -S 2>&1 | FileCheck %s

; Test that the first n-1 recursive progression clones are inlined, while
; the n-th is not.

; CHECK: call{{.*}}foo.4
; CHECK-LABEL: define{{.*}}foo.4
; CHECK: call{{.*}}foo.4
;
; CHECK-DAG: DEAD STATIC FUNC: foo.2
; CHECK-DAG: DEAD STATIC FUNC: foo.3
; CHECK-DAG: DEAD STATIC FUNC: foo.1
; CHECK-DAG: COMPILE FUNC: foo.4
; CHECK-DAG: INLINE: foo.1{{.*}}Callee is recursive progression clone
; CHECK-DAG: INLINE: foo.2{{.*}}Callee is recursive progression clone
; CHECK-DAG: INLINE: foo.3{{.*}}Callee is recursive progression clone
; CHECK-DAG: foo.4{{.*}}Callee has recursion
; CHECK-DAG: COMPILE FUNC: main
; CHECK-DAG: INLINE: foo.1{{.*}}Callee is recursive progression clone
; CHECK-DAG: INLINE: foo.2{{.*}}Callee is recursive progression clone
; CHECK-DAG: INLINE: foo.3{{.*}}Callee is recursive progression clone
; CHECK-DAG: foo.4{{.*}}Callee has recursion

define internal i32 @foo(i32, i32) {
  %3 = srem i32 %0, 4
  %4 = icmp sgt i32 %3, 2
  br i1 %4, label %5, label %8

; <label>:5:                                      ; preds = %2
  %6 = shl i32 %1, 1
  %7 = add nsw i32 %6, 3
  br label %8

; <label>:8:                                      ; preds = %2
  %9 = add nsw i32 %3, 1
  %10 = call i32 @foo(i32 %9, i32 %1)
  %11 = shl nsw i32 %10, 1
  br label %12

; <label>:12:                                     ; preds = %8
  ret i32 %11
}

define dso_local i32 @main() {
  br label %1

; <label>:1:                                      ; preds = %1, %0
  %2 = phi i32 [ 0, %0 ], [ %6, %1 ]
  %3 = phi i32 [ 0, %0 ], [ %5, %1 ]
  %4 = tail call i32 @foo(i32 0, i32 %2)
  %5 = add nsw i32 %4, %3
  %6 = add nuw nsw i32 %2, 1
  %7 = icmp eq i32 %6, 10
  br i1 %7, label %8, label %1

; <label>:8:                                      ; preds = %1
  ret i32 %5
}

; end INTEL_FEATURE_SW_ADVANCED
