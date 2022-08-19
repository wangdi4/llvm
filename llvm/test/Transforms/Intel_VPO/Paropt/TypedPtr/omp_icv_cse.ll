; RUN: opt -inferattrs -S %s 2>&1 | FileCheck %s
; CHECK-NOT: Function Attrs: nounwind readonly

; CMPLRLLVM-7246: -licm and -early-cse-memssa should not move function calls
; with non-deterministic behavior out of loops. It is sufficient to check if
; any calls have the "readonly" attribute, as this is how the passes
; decide whether they are invariant.

; Function Attrs: noinline uwtable
define dso_local double @_Z4waitd(double %seconds) #0 {
entry:
  %call = call double @omp_get_wtime()
  br label %while.cond

while.cond:                                       ; preds = %while.cond, %entry
  %add = fadd double %call, %seconds
  %call1 = call double @omp_get_wtime()
  %cmp = fcmp olt double %add, %call1
  br i1 %cmp, label %while.end, label %while.cond

while.end:                                        ; preds = %while.cond
  %call2 = call i64 @clock() #5
  %conv = sitofp i64 %call2 to double
  br label %while.cond3

while.cond3:                                      ; preds = %while.cond3, %while.end
  %mul = fmul double %seconds, 1.000000e+06
  %add5 = fadd double %conv, %mul
  %call6 = call i64 @clock() #5
  %conv7 = sitofp i64 %call6 to double
  %cmp8 = fcmp olt double %add5, %conv7
  br i1 %cmp8, label %while.end11, label %while.cond3

while.end11:                                      ; preds = %while.cond3
  ret double %conv
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

declare dso_local double @omp_get_wtime() #2

; Function Attrs: nounwind
declare dso_local i64 @clock() #3

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

; Function Attrs: norecurse uwtable
define dso_local i32 @main() #4 {
entry:
  %call = call double @_Z4waitd(double 3.000000e+00)
  %conv = fptosi double %call to i32
  ret i32 %conv
}

attributes #0 = { noinline uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { norecurse uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #5 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 8.0.0"}
