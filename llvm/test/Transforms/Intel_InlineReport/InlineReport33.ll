; RUN: opt < %s -ip-cloning -inline -inline-report=7 -S 2>&1 | FileCheck --check-prefixes=CHECK,CHECK-OLD %s
; RUN: opt < %s -passes='module(ip-cloning),cgscc(inline)' -inline-report=7 -S 2>&1 | FileCheck --check-prefixes=CHECK,CHECK-NEW %s

; Test that the first n-1 recursive progression clones are inlined, while
; the n-th is not.

; CHECK-OLD: DEAD STATIC FUNC: foo.1
; CHECK-OLD: DEAD STATIC FUNC: foo.2
; CHECK-OLD: DEAD STATIC FUNC: foo.3
; CHECK-OLD: COMPILE FUNC: foo.4
; CHECK-OLD: INLINE: foo.1{{.*}}Callee is recursive progression clone
; CHECK-OLD: INLINE: foo.2{{.*}}Callee is recursive progression clone
; CHECK-OLD: INLINE: foo.3{{.*}}Callee is recursive progression clone
; CHECK-OLD: foo.4{{.*}}Callee has recursion
; CHECK-OLD: COMPILE FUNC: main
; CHECK-OLD: INLINE: foo.1{{.*}}Callee is recursive progression clone
; CHECK-OLD: INLINE: foo.2{{.*}}Callee is recursive progression clone
; CHECK-OLD: INLINE: foo.3{{.*}}Callee is recursive progression clone
; CHECK-OLD: foo.4{{.*}}Callee has recursion

; CHECK-LABEL: define{{.*}}main
; CHECK: call{{.*}}foo.4
; CHECK-LABEL: define{{.*}}foo.4
; CHECK: call{{.*}}foo.4
;
; CHECK-NEW: DEAD STATIC FUNC: foo.2
; CHECK-NEW: DEAD STATIC FUNC: foo.3
; CHECK-NEW: DEAD STATIC FUNC: foo.1
; CHECK-NEW: COMPILE FUNC: foo.4
; CHECK-NEW: INLINE: foo.1{{.*}}Callee is recursive progression clone
; CHECK-NEW: INLINE: foo.2{{.*}}Callee is recursive progression clone
; CHECK-NEW: INLINE: foo.3{{.*}}Callee is recursive progression clone
; CHECK-NEW: foo.4{{.*}}Callee has recursion
; CHECK-NEW: COMPILE FUNC: main
; CHECK-NEW: INLINE: foo.1{{.*}}Callee is recursive progression clone
; CHECK-NEW: INLINE: foo.2{{.*}}Callee is recursive progression clone
; CHECK-NEW: INLINE: foo.3{{.*}}Callee is recursive progression clone
; CHECK-NEW: foo.4{{.*}}Callee has recursion

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

