; RUN: opt < %s -ip-cloning -inline -inline-report=7 -S 2>&1 | FileCheck --check-prefix=CHECK-OLD %s
; RUN: opt < %s -passes='module(ip-cloning),cgscc(inline)' -inline-report=7 -S 2>&1 | FileCheck --check-prefix=CHECK-NEW %s
@count = available_externally dso_local local_unnamed_addr global i32 0, align 8

; Test that all recursive progression clones are inlined.

; CHECK-OLD: DEAD STATIC FUNC: foo.8
; CHECK-OLD: DEAD STATIC FUNC: foo.7
; CHECK-OLD: DEAD STATIC FUNC: foo.6
; CHECK-OLD: DEAD STATIC FUNC: foo.5
; CHECK-OLD: DEAD STATIC FUNC: foo.4
; CHECK-OLD: DEAD STATIC FUNC: foo.3
; CHECK-OLD: DEAD STATIC FUNC: foo.2
; CHECK-OLD: DEAD STATIC FUNC: foo.1
; CHECK-OLD: COMPILE FUNC: MAIN__
; CHECK-OLD: INLINE: foo.1{{.*}}<<Callee is recursive progression clone>>
; CHECK-OLD: INLINE: foo.2{{.*}}<<Callee is recursive progression clone>>
; CHECK-OLD: INLINE: foo.3{{.*}}<<Callee is recursive progression clone>>
; CHECK-OLD: INLINE: foo.4{{.*}}<<Callee is recursive progression clone>>
; CHECK-OLD: INLINE: foo.5{{.*}}<<Callee is recursive progression clone>>
; CHECK-OLD: INLINE: foo.6{{.*}}<<Callee is recursive progression clone>>
; CHECK-OLD: INLINE: foo.7{{.*}}<<Callee is recursive progression clone>>
; CHECK-OLD: INLINE: foo.8{{.*}}<<Callee is recursive progression clone>>

; CHECK-LABEL: define{{.*}}MAIN__
; CHECK-NOT: define{{.*}}foo
; CHECK-NOT: call{{.*}}foo

; CHECK-NEW: DEAD STATIC FUNC: foo.8
; CHECK-NEW: DEAD STATIC FUNC: foo.7
; CHECK-NEW: DEAD STATIC FUNC: foo.6
; CHECK-NEW: DEAD STATIC FUNC: foo.5
; CHECK-NEW: DEAD STATIC FUNC: foo.4
; CHECK-NEW: DEAD STATIC FUNC: foo.3
; CHECK-NEW: DEAD STATIC FUNC: foo.2
; CHECK-NEW: DEAD STATIC FUNC: foo.1
; CHECK-NEW: COMPILE FUNC: MAIN__
; CHECK-NEW: INLINE: foo.1{{.*}}<<Callee is recursive progression clone>>
; CHECK-NEW: INLINE: foo.2{{.*}}<<Callee is recursive progression clone>>
; CHECK-NEW: INLINE: foo.3{{.*}}<<Callee is recursive progression clone>>
; CHECK-NEW: INLINE: foo.4{{.*}}<<Callee is recursive progression clone>>
; CHECK-NEW: INLINE: foo.5{{.*}}<<Callee is recursive progression clone>>
; CHECK-NEW: INLINE: foo.6{{.*}}<<Callee is recursive progression clone>>
; CHECK-NEW: INLINE: foo.7{{.*}}<<Callee is recursive progression clone>>
; CHECK-NEW: INLINE: foo.8{{.*}}<<Callee is recursive progression clone>>

define dso_local void @MAIN__() #0 {
  %1 = alloca i32, align 4
  store i32 1, i32* %1, align 4
  call void @foo(i32* nonnull %1)
  ret void
}

; Function Attrs: nounwind
define internal void @foo(i32* noalias nocapture readonly) {
  %2 = alloca i32, align 4
  %3 = load i32, i32* @count, align 8
  %4 = add nsw i32 %3, 1
  store i32 %4, i32* @count, align 8
  %5 = load i32, i32* %0, align 4
  %6 = icmp eq i32 %5, 8
  br i1 %6, label %7, label %9

; <label>:7:                                      ; preds = %1
  %8 = add nsw i32 %3, 2
  store i32 %8, i32* @count, align 8
  br label %13

; <label>:9:                                      ; preds = %1
  %10 = icmp slt i32 %4, 500000
  br i1 %10, label %11, label %13

; <label>:11:                                     ; preds = %9
  %12 = add nsw i32 %5, 1
  store i32 %12, i32* %2, align 4
  call void @foo(i32* nonnull %2)
  br label %13

; <label>:13:                                     ; preds = %11, %9, %7
  ret void
}

