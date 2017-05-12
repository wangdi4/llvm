; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-cleanup | FileCheck %s

; The test was failing due to missing end iterator check when searching for lexical control flow successor.

; CHECK:    BEGIN REGION { }
; CHECK:         while.body:
; CHECK:         switch(0x0)
; CHECK:         {
; CHECK:         case 0x0:
; CHECK:           switch(0x0)
; CHECK:           {
; CHECK:           case 0x0:
; CHECK:              break;
; CHECK:           case 0x0:
; CHECK:              break;
; CHECK:           case 0x0:
; CHECK:              break;
; CHECK:           default:
; CHECK:              break;
; CHECK:           }
; CHECK:            break;
; CHECK:         case 0x0:
; CHECK:            break;
; CHECK:         case 0x0:
; CHECK:            break;
; CHECK:         default:
; CHECK:            break;
; CHECK:         }
; CHECK:        0x0 = 0x0  +  0x0;
; CHECK:        0x0 = 0x0 true 0x0;
; CHECK:        0x0 = 0x0;
; CHECK:        if (0x0 true 0x0)
; CHECK:        {
; CHECK:        }
; CHECK:        else
; CHECK:        {
; CHECK:           goto while.body;
; CHECK:        }
; CHECK:    END REGION


; ModuleID = 'red.ll'
source_filename = "bugpoint-output-fd4cddf.bc"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @main() local_unnamed_addr #0 {
entry:
  br label %while.body

while.body:                                       ; preds = %if.end45, %entry
  %dec152 = phi i32 [ %dec, %if.end45 ], [ undef, %entry ]
  switch i8 undef, label %if.end45 [
    i8 108, label %if.then
    i8 101, label %if.then34
    i8 99, label %if.then42
  ]

if.then:                                          ; preds = %while.body
  switch i8 undef, label %if.end45 [
    i8 49, label %if.then9
    i8 50, label %if.then16
    i8 51, label %if.then24
  ]

if.then9:                                         ; preds = %if.then
  br label %if.end45

if.then16:                                        ; preds = %if.then
  br label %if.end45

if.then24:                                        ; preds = %if.then
  br label %if.end45

if.then34:                                        ; preds = %while.body
  br label %if.end45

if.then42:                                        ; preds = %while.body
  br label %if.end45

if.end45:                                         ; preds = %if.then42, %if.then34, %if.then24, %if.then16, %if.then9, %if.then, %while.body
  %dec = add nsw i32 %dec152, -1
  %tobool = icmp eq i32 %dec, 0
  br i1 %tobool, label %while.end, label %while.body

while.end:                                        ; preds = %if.end45
  ret void
}

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (branches/xmain_web 20412)"}
