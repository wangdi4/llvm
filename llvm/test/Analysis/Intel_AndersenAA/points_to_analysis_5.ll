; RUN: opt < %s -passes='require<anders-aa>'  -print-anders-constraints -disable-output  2>&1 | FileCheck %s
; Test for a bug (CQ377893) in Andersens analysis that caused runtime failure
; because some special direct calls are treated as indirect calls.

; CHECK: bar:call = <universal>

@total_size = external global i32, align 4
@A = external global ptr, align 8

; Function Attrs: nounwind uwtable
define void @bar() #0 {
entry:
  %i = load i32, ptr @total_size, align 4
  %call = call ptr (i32, i32, ...) @sf_new(i32 %i, i32 10)
  store ptr %call, ptr @A, align 8
  ret void
}

declare ptr @sf_new(...) #1

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
