; RUN: opt < %s -disable-output "-passes=print<scalar-evolution>" < %s 2>&1 | FileCheck %s
; RUN: opt -disable-output -scalar-evolution-print-scoped-mode "-passes=print<scalar-evolution>" < %s 2>&1 | FileCheck %s --check-prefix=SCOPED-MODE

; Verify that in scoped mode we can propagate nsw flag to SCEV of %add by
; assuming that IR will not change during analysis. Although we ignore
; %add and all its users during the nowrap analysis tracking, we were 
; reaching its use in %sub due to alternatiave path of %sextn while
; looking at users of %n thereby giving up.

; Adding nsw flag on %add also simplifies %sub to 3.

; CHECK:      %add = add nsw i32 %n, 3
; CHECK-NEXT:   -->  (3 + %n)
; CHECK-NOT:    nsw
; CHECK:      %sub = sub nsw i64 %sexta, %sextn
; CHECK-NEXT:   -->  ((sext i32 (3 + %n) to i64) + (-1 * (sext i32 %n to i64))<nsw>)<nsw>

; SCOPED-MODE:      %add = add nsw i32 %n, 3
; SCOPED-MODE-NEXT:   -->  (3 + %n)<nsw> 
; SCOPED-MODE:      %sub = sub nsw i64 %sexta, %sextn
; SCOPED-MODE-NEXT:   -->  3


define i64 @foo(i32 %n) {
entry:
  %add = add nsw i32 %n, 3
  %sexta = sext i32 %add to i64
  %sextn = sext i32 %n to i64
  %sub = sub nsw i64 %sexta, %sextn
  ret i64 %sub
}
