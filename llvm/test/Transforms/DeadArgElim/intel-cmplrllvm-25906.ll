; REQUIRES: asserts
; RUN: opt < %s -deadargelim -debug-only=deadargelim -vec-clone -S 2>&1 | FileCheck %s
; RUN: opt < %s -passes='deadargelim,vec-clone' -debug-only=deadargelim -S 2>&1 | FileCheck %s

; Check that dead argument elimination does not happen for @foo because it
; has vector variants. The arguments of vector variants cannot be eliminated
; because the number of arguments is tied to the function signature of the
; vector variant. In addition, do not turn dead args to poison because args not
; explicitly used in the function may be used by VecClone to calculate stride.

; CHECK: DeadArgumentEliminationPass - foo has vector variants
; CHECK: define dso_local i32 @main()
; CHECK: call i32 @foo(i32 %0, i32 %1)
; CHECK: define internal i32 @foo(i32 %i, i32 %x)

@glob1 = dso_local global i32 5, align 4
@glob2 = dso_local global i32 6, align 4

define dso_local i32 @main() {
entry:
  %0 = load i32, i32* @glob1, align 4
  %1 = load i32, i32* @glob2, align 4
  %call = call i32 @foo(i32 %0, i32 %1)
  ret i32 %call
}


define internal i32 @foo(i32 %i, i32 %x) #0 {
entry:
  %i.addr = alloca i32, align 4
  store i32 %i, i32* %i.addr, align 4
  %t1 = load i32, i32* %i.addr, align 4
  %add = add nsw i32 0, %t1
  ret i32 %add
}

attributes #0 = { nounwind uwtable "vector-variants"="_ZGVbM4lu_foo,_ZGVbN4lu_foo" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
