; Check if this is compiling successfully since original vecclone alloca
; replacement transformation could miss some vecparam users due to the incorrect
; iterating over them

; RUN: opt -passes="vec-clone" -S < %s | FileCheck %s

; CHECK-LABEL: @_ZGVbN4v_foo

; ModuleID = 'test.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
; Function Attrs: norecurse nounwind uwtable
define dso_local i32 @foo(i32 %a) local_unnamed_addr #0 {
entry:
  %mul = mul nsw i32 %a, 20
  %add = add nsw i32 %a, %a
  %add1 = add nsw i32 %add, %mul
  ret i32 %add1
}

attributes #0 = { norecurse nounwind uwtable "vector-variants"="_ZGVbN4v_foo" "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false\
" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-featur\
es"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
