; Inline report
; RUN: opt -opaque-pointers -passes='cgscc(inline)' -inline-threshold=5 -inlinehint-threshold=6 -double-callsite-inlinehint-threshold=675 -inline-report=0xe807 < %s -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-CL
; Inline report via metadata
; RUN: opt -opaque-pointers -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='cgscc(inline)' -inline-threshold=5 -inlinehint-threshold=6 -double-callsite-inlinehint-threshold=675 -inline-report=0xe886 -S | opt -passes='inlinereportemitter' -inline-report=0xe886 -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-MD

; Test for -double-callsite-inlinehint-threshold. @mydoublecallee0 should be
; inlined because it is a double callsite linkonce_odr function with an
; inlinehint. @mydoublecallee1 should not be inlined because it does not
; have an inlinehint. @mytriplecallee should not be inlined because it is
; a triple, rather than double, callsite.

; FIXME: Corresponding change is currently disabled on Windows as it leads
;        to unexpected fails across testbase. See CMPLRLLVM-35571
; XFAIL: windows-msvc

; CHECK-MD: DEAD STATIC FUNC: mydoublecallee0
; CHECK-MD: COMPILE FUNC: mytriplecallee
; CHECK-MD: COMPILE FUNC: mydoublecallee1
; CHECK-MD: COMPILE FUNC: mycaller
; CHECK-MD: INLINE: mydoublecallee0{{.*}}Inlining is profitable
; CHECK-MD: INLINE: mydoublecallee0{{.*}}Callee has single callsite and local linkage
; CHECK-MD: mydoublecallee1{{.*}}Inlining is not profitable
; CHECK-MD: mydoublecallee1{{.*}}Inlining is not profitable
; CHECK-MD: mytriplecallee{{.*}}Inlining is not profitable
; CHECK-MD: mytriplecallee{{.*}}Inlining is not profitable
; CHECK-MD: mytriplecallee{{.*}}Inlining is not profitable

; CHECK: define linkonce_odr dso_local i32 @mytriplecallee
; CHECK: define linkonce_odr dso_local i32 @mydoublecallee1
; CHECK: define i32 @mycaller
; CHECK-NOT: tail call i32 @mydoublecallee0
; CHECK: tail call i32 @mydoublecallee1
; CHECK: tail call i32 @mydoublecallee1
; CHECK:  tail call i32 @mytriplecallee
; CHECK:  tail call i32 @mytriplecallee
; CHECK:  tail call i32 @mytriplecallee
; CHECK-NOT: define linkonce_odr dso_local i32 @mydoublecallee0

; CHECK-CL: DEAD STATIC FUNC: mydoublecallee0
; CHECK-CL: COMPILE FUNC: mytriplecallee
; CHECK-CL: COMPILE FUNC: mydoublecallee1
; CHECK-CL: COMPILE FUNC: mycaller
; CHECK-CL: INLINE: mydoublecallee0{{.*}}Inlining is profitable
; CHECK-CL: INLINE: mydoublecallee0{{.*}}Callee has single callsite and local linkage
; CHECK-CL: mydoublecallee1{{.*}}Inlining is not profitable
; CHECK-CL: mydoublecallee1{{.*}}Inlining is not profitable
; CHECK-CL: mytriplecallee{{.*}}Inlining is not profitable
; CHECK-CL: mytriplecallee{{.*}}Inlining is not profitable
; CHECK-CL: mytriplecallee{{.*}}Inlining is not profitable

define linkonce_odr dso_local i32 @mydoublecallee0(i32 %arg0, i32 %arg1) #0 {
  %test = icmp eq i32 %arg0, %arg1
  br i1 %test, label %label0, label %label1

label0: 
  %mysub0 = sub i32 %arg0, %arg1
  %mysub1 = sub i32 %arg0, %arg1
  %myres0 = sub i32 %mysub0, %mysub1
  %mysub2 = sub i32 %arg0, %arg1
  %mysub3 = sub i32 %arg0, %arg1
  %myres1 = sub i32 %mysub2, %mysub3
  %mysub = sub i32 %myres0, %myres1 
  br label %label2

label1:
  %myadd = add i32 %arg0, %arg1
  br label %label2

label2: 
  %myres = phi i32 [ %mysub, %label0 ], [ %myadd, %label1 ]
  ret i32 %myres
}

define linkonce_odr dso_local i32 @mytriplecallee(i32 %arg0, i32 %arg1) #0 {
  %test = icmp eq i32 %arg0, %arg1
  br i1 %test, label %label0, label %label1

label0: 
  %myadd0 = add i32 %arg0, %arg1
  %myadd1 = add i32 %arg0, %arg1
  %myres0 = add i32 %myadd0, %myadd1
  %myadd2 = add i32 %arg0, %arg1
  %myadd3 = add i32 %arg0, %arg1
  %myres1 = add i32 %myadd2, %myadd3
  %myadd4 = add i32 %myres0, %myres1 
  br label %label2

label1:
  %myadd5 = add i32 %arg0, %arg1
  br label %label2

label2: 
  %myres = phi i32 [ %myadd4, %label0 ], [ %myadd5, %label1 ]
  ret i32 %myres
}

define linkonce_odr dso_local i32 @mydoublecallee1(i32 %arg0, i32 %arg1) {
  %test = icmp eq i32 %arg0, %arg1
  br i1 %test, label %label0, label %label1

label0: 
  %mysub0 = sub i32 %arg0, %arg1
  %myshl1 = shl i32 %arg0, %arg1
  %myres0 = sub i32 %mysub0, %myshl1
  %myshl2 = shl i32 %arg0, %arg1
  %mysub3 = sub i32 %arg0, %arg1
  %myres1 = shl i32 %myshl2, %mysub3
  %myadd0 = add i32 %myres0, %myres1 
  br label %label2

label1:
  %myadd1 = add i32 %arg0, %arg1
  br label %label2

label2: 
  %myres = phi i32 [ %myadd0, %label0 ], [ %myadd1, %label1 ]
  ret i32 %myres
}

define i32 @mycaller(i32 %arg0, i32 %arg1) {
  %myres0 = tail call i32 @mydoublecallee0(i32 %arg0, i32 %arg1)
  %myres1 = tail call i32 @mydoublecallee0(i32 %arg0, i32 %arg1)
  %myadd0 = add i32 %myres0, %myres1 
  %myres6 = tail call i32 @mydoublecallee1(i32 %arg0, i32 %arg1)
  %myres7 = tail call i32 @mydoublecallee1(i32 %arg0, i32 %arg1)
  %myadd1 = add i32 %myres6, %myres7 
  %myadd2 = add i32 %myadd0, %myadd1
  %myres2 = tail call i32 @mytriplecallee(i32 %arg0, i32 %arg1)
  %myres3 = tail call i32 @mytriplecallee(i32 %arg0, i32 %arg1)
  %myres4 = add i32 %myres2, %myres3 
  %myres5 = tail call i32 @mytriplecallee(i32 %arg0, i32 %arg1)
  %myadd3 = add i32 %myres5, %myadd2
  ret i32 %myadd3
}

attributes #0 = { inlinehint }
