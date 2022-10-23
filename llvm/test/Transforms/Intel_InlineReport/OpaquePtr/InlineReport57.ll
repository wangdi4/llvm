; RUN: opt -opaque-pointers -passes='cgscc(inline)' -inline-report=0xec07 < %s -S 2>&1 | FileCheck --check-prefixes=CHECK-NEW,CHECK-BEFORE %s
; RUN: opt -opaque-pointers -passes='inlinereportsetup' -inline-report=0xec86 < %s -S | opt -passes='cgscc(inline)' -inline-report=0xec86 -S | opt -passes='inlinereportemitter' -inline-report=0xec86 -S 2>&1 | FileCheck --check-prefixes=CHECK-META,CHECK-AFTER %s

; Check that when 0x400 bit is selected in the inlining report, that the
; language of the functions and callsites is printed as either 'F' (Fortran)
; or 'C' (C/C++). Ensure they are printed correctly, even when the functions
; are dead.

; CHECK-BEFORE: define dso_local i32 @foof
; CHECK-BEFORE-NOT: call i32 @barf()
; CHECK-BEFORE: define dso_local i32 @fooc
; CHECK-BEFORE-NOT: call i32 @barc()
; CHECK-BEFORE-NOT: define internal i32 @barf
; CHECK-BEFORE-NOT: define internal i32 @barc

; CHECK-OLD: DEAD STATIC FUNC: C barc
; CHECK-OLD: DEAD STATIC FUNC: F barf
; CHECK-OLD: COMPILE FUNC: F foof
; CHECK-OLD: INLINE: F barf
; CHECK-OLD: COMPILE FUNC: C fooc
; CHECK-OLD: INLINE: C barc

; CHECK-NEW: DEAD STATIC FUNC: C barc
; CHECK-NEW: DEAD STATIC FUNC: F barf
; CHECK-NEW: COMPILE FUNC: F foof
; CHECK-NEW: INLINE: F barf
; CHECK-NEW: COMPILE FUNC: C fooc
; CHECK-NEW: INLINE: C barc

; CHECK-META: COMPILE FUNC: F foof
; CHECK-META: INLINE: F barf
; CHECK-META: DEAD STATIC FUNC: F barf
; CHECK-META: COMPILE FUNC: C fooc
; CHECK-META: INLINE: C barc
; CHECK-META: DEAD STATIC FUNC: C barc

; CHECK-AFTER: define dso_local i32 @foof
; CHECK-AFTER-NOT: call i32 @barf()
; CHECK-AFTER: define dso_local i32 @fooc
; CHECK-AFTER-NOT: call i32 @barc()
; CHECK-AFTER-NOT: define internal i32 @barf
; CHECK-AFTER-NOT: define internal i32 @barc

define dso_local i32 @foof() #0 {
  %call = call i32 @barf()
  ret i32 %call
}

define internal i32 @barf() #0 {
  ret i32 7
}

define dso_local i32 @fooc() {
  %call = call i32 @barc()
  ret i32 %call
}

define internal i32 @barc() {
  ret i32 7
}

attributes #0 = { "intel-lang"="fortran" }

