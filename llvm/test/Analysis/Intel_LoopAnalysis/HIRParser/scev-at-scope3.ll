; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser | FileCheck %s

; Check that %add23 outside i2 loop is successfully parsed as a blob after 'at scope' parsing fails. 

; CHECK:      + DO i1 = 0, 47, 1   <DO_LOOP>
; CHECK-NEXT: |   %add23.lcssa94 = 1;
; CHECK-NEXT: |   if (undef #UNDEF# undef)
; CHECK-NEXT: |   {
; CHECK-NEXT: |      + DO i2 = 0, (umax(4, %0) + -2)/u3, 1   <DO_LOOP>
; CHECK-NEXT: |      |   %add23 = 3 * i2 + 1  +  3;
; CHECK-NEXT: |      + END LOOP
; CHECK-NEXT: |      %add23.lcssa94 = %add23;
; CHECK-NEXT: |   }
; CHECK-NEXT: |   %0 = i1 + 2;
; CHECK-NEXT: + END LOOP


; ModuleID = 'bugpoint-reduced-simplified.bc'
target datalayout = "e-m:x-p:32:32-i64:64-f80:32-n8:16:32-a:0:32-S32"
target triple = "i386-pc-windows-msvc"

; Function Attrs: nounwind
define void @main() #0 {
entry:
  br label %for.body.i

for.body.i:                                       ; preds = %for.body.i, %entry
  br i1 undef, label %init.exit, label %for.body.i

init.exit:                                        ; preds = %for.body.i
  br label %for.body.i63

for.body.i63:                                     ; preds = %for.body.i63, %init.exit
  br i1 undef, label %init.exit64, label %for.body.i63

init.exit64:                                      ; preds = %for.body.i63
  br i1 undef, label %if.then, label %if.else

if.then:                                          ; preds = %init.exit64
  unreachable

if.else:                                          ; preds = %init.exit64
  br label %for.body

for.body:                                         ; preds = %for.inc25, %if.else
  %indvars.iv105 = phi i32 [ -1, %if.else ], [ %indvars.iv.next106, %for.inc25 ]
  %0 = phi i32 [ 1, %if.else ], [ %inc, %for.inc25 ]
  br i1 undef, label %for.body6.lr.ph, label %for.inc25

for.body6.lr.ph:                                  ; preds = %for.body
  br label %for.inc22

for.inc22:                                        ; preds = %for.inc22, %for.body6.lr.ph
  %add2392 = phi i32 [ 1, %for.body6.lr.ph ], [ %add23, %for.inc22 ]
  %add23 = add nuw nsw i32 %add2392, 3
  %cmp5 = icmp ult i32 %add23, %0
  br i1 %cmp5, label %for.inc22, label %for.cond4.for.inc25_crit_edge

for.cond4.for.inc25_crit_edge:                    ; preds = %for.inc22
  br label %for.inc25

for.inc25:                                        ; preds = %for.cond4.for.inc25_crit_edge, %for.body
  %add23.lcssa94 = phi i32 [ %add23, %for.cond4.for.inc25_crit_edge ], [ 1, %for.body ]
  %inc = add nuw nsw i32 %0, 1
  %indvars.iv.next106 = add nsw i32 %indvars.iv105, 1
  %exitcond = icmp eq i32 %indvars.iv.next106, 47
  br i1 %exitcond, label %for.end26, label %for.body

for.end26:                                        ; preds = %for.inc25
  store i32 %add23.lcssa94, i32* undef, align 4
  br label %for.body.i68

for.body.i68:                                     ; preds = %for.body.i68, %for.end26
  br i1 undef, label %checkSum.exit, label %for.body.i68

checkSum.exit:                                    ; preds = %for.body.i68
  br label %for.body.i78

for.body.i78:                                     ; preds = %for.body.i78, %checkSum.exit
  br i1 undef, label %checkSum.exit79, label %for.body.i78

checkSum.exit79:                                  ; preds = %for.body.i78
  br i1 undef, label %if.end39, label %if.then36

if.then36:                                        ; preds = %checkSum.exit79
  ret void

if.end39:                                         ; preds = %checkSum.exit79
  unreachable
}
