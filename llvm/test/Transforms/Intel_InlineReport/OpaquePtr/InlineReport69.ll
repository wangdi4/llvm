; RUN: opt -opaque-pointers -passes='cgscc(inline)' -inline-report=0x7 < %s -S 2>&1 | FileCheck --check-prefixes=CHECK-OLD,CHECK-CODE-BEFORE %s
; RUN: opt -opaque-pointers -passes='inlinereportsetup,cgscc(inline),inlinereportemitter' -inline-report=0xe886 -S < %s 2>&1 | FileCheck --check-prefixes=CHECK-NEW,CHECK-CODE-AFTER %s

; Check that inlining between callers and callees of different languages
; does not occur.

; CHECK-CODE-BEFORE: define dso_local i32 @foo() #0
; CHECK-CODE-BEFORE: %call = call i32 @bar()
; CHECK-CODE-BEFORE: define dso_local i32 @bar()

; CHECK-OLD: COMPILE FUNC: bar
; CHECK-OLD: COMPILE FUNC: foo
; CHECK-OLD: bar{{.*}}Caller and Callee have different source languages

; CHECK-NEW: COMPILE FUNC: foo
; CHECK-NEW: bar{{.*}}Caller and Callee have different source languages
; CHECK-NEW: COMPILE FUNC: bar

; CHECK-CODE-AFTER: define dso_local i32 @foo() #0
; CHECK-CODE-AFTER: %call = call i32 @bar()
; CHECK-CODE-AFTER: define dso_local i32 @bar()

define dso_local i32 @foo() #0 {
  %call = call i32 @bar()
  ret i32 %call
}

define dso_local i32 @bar() {
  ret i32 7
}

attributes #0 = { "intel-lang"="fortran" }

