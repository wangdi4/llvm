; RUN: opt < %s -aa-pipeline=anders-aa -passes="default<O2>" -disable-output 2>/dev/null
; Test for a bug (CQ377860) in Andersens analysis that caused crash when 
; Instruction::Select is used as operand in Instruction::Store
@fp = external global ptr, align 8
; CMPLRLLVM-45955: Moved out "select" instruction out of "store" instruction
; since select is removed as ConstantExpr.

; Function Attrs: nounwind uwtable
define i32 @foo1() #0 {
entry:
  %retval = alloca i32, align 4
  %s = select i1 icmp eq (ptr inttoptr (i64 3 to ptr), ptr @foo), ptr @foo, ptr @bar
  store ptr %s, ptr @fp, align 8
  %i = load i32, ptr %retval, align 4
  ret i32 %i
}

declare i32 @foo() #1

declare i32 @bar() #1

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
