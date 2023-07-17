; RUN: opt -passes=inline -disable-output -inline-report=0xf847 < %s 2>&1 | FileCheck --check-prefixes=CHECK-CL,CHECK-CL-MAX %s
; RUN: opt -passes='inlinereportsetup,inline,inlinereportemitter' -disable-output -inline-report=0xf8c6 < %s 2>&1 | FileCheck --check-prefixes=CHECK-CL,CHECK-CL-MAX %s
; RUN: opt -passes=inline -disable-output -inline-report=0x2f847 < %s 2>&1 | FileCheck --check-prefixes=CHECK-CL,CHECK-CL-CPT %s
; RUN: opt -passes='inlinereportsetup,inline,inlinereportemitter' -disable-output -inline-report=0x2f8c6 < %s 2>&1 | FileCheck --check-prefixes=CHECK-CL,CHECK-CL-CPT %s
; RUN: opt -passes=inline -disable-output -inline-report=0xf847 -inline-report-compact-threshold=0 < %s 2>&1 | FileCheck --check-prefixes=CHECK-CL,CHECK-CL-CPT %s
; RUN: opt -passes='inlinereportsetup,inline,inlinereportemitter' -disable-output -inline-report=0xf8c6 -inline-report-compact-threshold=0 < %s 2>&1 | FileCheck --check-prefixes=CHECK-CL,CHECK-CL-CPT %s
; RUN: opt -passes=inline -disable-output -inline-report=0xf847 -inline-report-compact-threshold=1 < %s 2>&1 | FileCheck --check-prefixes=CHECK-CL,CHECK-CL-CPT %s
; RUN: opt -passes='inlinereportsetup,inline,inlinereportemitter' -disable-output -inline-report=0xf8c6 -inline-report-compact-threshold=2 < %s 2>&1 | FileCheck --check-prefixes=CHECK-CL,CHECK-CL-TH2 %s
; RUN: opt -passes=inline -disable-output -inline-report=0xf847 -inline-report-compact-threshold=1 < %s 2>&1 | FileCheck --check-prefixes=CHECK-CL,CHECK-CL-CPT %s
; RUN: opt -passes='inlinereportsetup,inline,inlinereportemitter' -disable-output -inline-report=0xf8c6 -inline-report-compact-threshold=1 < %s 2>&1 | FileCheck --check-prefixes=CHECK-CL,CHECK-CL-CPT %s
; RUN: opt -passes=inline -disable-output -inline-report=0xf847 -inline-report-compact-threshold=2 < %s 2>&1 | FileCheck --check-prefixes=CHECK-CL,CHECK-CL-TH2 %s
; RUN: opt -passes='inlinereportsetup,inline,inlinereportemitter' -disable-output -inline-report=0xf8c6 -inline-report-compact-threshold=2 < %s 2>&1 | FileCheck --check-prefixes=CHECK-CL,CHECK-CL-TH2 %s
; RUN: opt -passes=inline -disable-output -inline-report=0xf847 -inline-report-compact-threshold=3 < %s 2>&1 | FileCheck --check-prefixes=CHECK-CL,CHECK-CL-MAX %s
; RUN: opt -passes='inlinereportsetup,inline,inlinereportemitter' -disable-output -inline-report=0xf8c6 -inline-report-compact-threshold=3 < %s 2>&1 | FileCheck --check-prefixes=CHECK-CL,CHECK-CL-MAX %s

; Test compact form of inlining report.

; CHECK-CL: COMPILE FUNC: mynoinline
; CHECK-CL: COMPILE FUNC: foo
; CHECK-CL: mynoinline {{.*}}Callee has noinline attribute
; CHECK-CL: COMPILE FUNC: boo
; CHECK-CL: COMPILE FUNC: goo
; CHECK-CL: INLINE: foo {{.*}}Callee is single basic block
; CHECK-CL: mynoinline {{.*}}Callee has noinline attribute
; CHECK-CL: INLINE: foo {{.*}}Callee is single basic block
; CHECK-CL: mynoinline {{.*}}Callee has noinline attribute
; CHECK-CL: INLINE: foo {{.*}}Callee is single basic block
; CHECK-CL: mynoinline {{.*}}Callee has noinline attribute
; CHECK-CL: COMPILE FUNC: hoo
; CHECK-CL: INLINE: boo {{.*}}Callee is single basic block
; CHECK-CL: INLINE: boo {{.*}}Callee is single basic block

; CHECK-CL: COMPILE FUNC: main
; CHECK-CL: mynoinline {{.*}}Callee has noinline attribute
; CHECK-CL-MAX: INLINE: goo {{.*}}Callee is single basic block
; CHECK-CL-CPT: <C> INLINE: goo {{.*}}Callee is single basic block
; CHECK-CL-MAX: INLINE: foo {{.*}}Callee is single basic block
; CHECK-CL: mynoinline {{.*}}Callee has noinline attribute
; CHECK-CL-MAX: INLINE: foo {{.*}}Callee is single basic block
; CHECK-CL: mynoinline {{.*}}Callee has noinline attribute
; CHECK-CL-MAX: INLINE: foo {{.*}}Callee is single basic block
; CHECK-CL: mynoinline {{.*}}Callee has noinline attribute
; CHECK-CL-MAX: INLINE: goo {{.*}}Callee is single basic block
; CHECK-CL-CPT: <C> INLINE: goo {{.*}}Callee is single basic block
; CHECK-CL-MAX: INLINE: foo {{.*}}Callee is single basic block
; CHECK-CL: mynoinline {{.*}}Callee has noinline attribute
; CHECK-CL-MAX: INLINE: foo {{.*}}Callee is single basic block
; CHECK-CL: mynoinline {{.*}}Callee has noinline attribute
; CHECK-CL-MAX: INLINE: foo {{.*}}Callee is single basic block
; CHECK-CL: mynoinline {{.*}}Callee has noinline attribute
; CHECK-CL-MAX: INLINE: hoo {{.*}}Callee is single basic block
; CHECK-CL-TH2: INLINE: hoo {{.*}}Callee is single basic block
; CHECK-CL-CPT: <C> INLINE: hoo {{.*}}Callee is single basic block
; CHECK-CL-MAX: INLINE: boo {{.*}}Callee is single basic block
; CHECK-CL-TH2: INLINE: boo {{.*}}Callee is single basic block
; CHECK-CL-MAX: INLINE: boo {{.*}}Callee is single basic block
; CHECK-CL-TH2: INLINE: boo {{.*}}Callee is single basic block
; CHECK-CL: INLINE: foo {{.*}}Callee is single basic block
; CHECK-CL: mynoinline {{.*}}Callee has noinline attribute
; CHECK-CPT: SUMMARIZED INLINED CALL SITE COUNTS
; CHECK-TH2: SUMMARIZED INLINED CALL SITE COUNTS
; CHECK-CPT: 6 foo
; CHECK-TH2: 6 foo
; CHECK-CPT: 2 boo

define void @mynoinline() #0 {
entry:
  ret void
}


define i32 @foo() {
entry:
  call void @mynoinline()
  ret i32 5
}

define i32 @boo() {
entry:
  ret i32 5
}

define i32 @goo() {
entry:
  %call0 = call i32 @foo()
  %call1 = call i32 @foo()
  %call2 = sub i32 %call0, %call1
  %call3 = call i32 @foo()
  %call4 = add i32 %call2, %call3
  ret i32 %call4
}

define i32 @hoo() {
entry:
  %call0 = call i32 @boo()
  %call1 = call i32 @boo()
  %call2 = sub i32 %call0, %call1
  ret i32 %call2
}


define i32 @main() {
entry:
  call void @mynoinline()
  %call0 = call i32 @goo()
  %call1 = call i32 @goo()
  %call2 = add i32 %call0, %call1
  %call3 = call i32 @hoo()
  %call4 = add i32 %call2, %call3
  %call5 = call i32 @foo()
  %call6 = add i32 %call4, %call5
  ret i32 %call6
}

attributes #0 = { noinline }

