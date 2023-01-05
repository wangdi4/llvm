; RUN: opt < %s -passes='require<anders-aa>'  -print-anders-constraints -disable-output  2>&1 | FileCheck %s
; Test for a bug (CQ377893) in Andersens analysis that caused runtime failure
; because some special direct calls are treated as indirect calls.

; CHECK: bar:call = <universal>


%struct.set_family = type opaque
@total_size = external global i32, align 4
@A = external global %struct.set_family*, align 8

; Function Attrs: nounwind uwtable
define void @bar() #0 {
entry:
  %0 = load i32, i32* @total_size, align 4
  %call = call %struct.set_family* (i32, i32, ...) bitcast (%struct.set_family* (...)* @sf_new to %struct.set_family* (i32, i32, ...)*)(i32 %0, i32 10)
  store %struct.set_family* %call, %struct.set_family** @A, align 8
  ret void
}

declare %struct.set_family* @sf_new(...) #1

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
