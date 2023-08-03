; RUN: opt < %s -passes='require<anders-aa>'  -disable-output 2>/dev/null
; Test for a bug (CQ378470) in Andersens analysis that caused crash when
; Instruction::Select is passed as argument to Instruction::Call.
; CMPLRLLVM-45955: Moved out "select" instruction out of "call"
; since select is removed as ConstantExpr.

@.str = private unnamed_addr constant [9 x i8] c"string_1\00", align 1
@.str.1 = private unnamed_addr constant [9 x i8] c"string_2\00", align 1
@.str.2 = private unnamed_addr constant [9 x i8] c"string_3\00", align 1

; Function Attrs: nounwind uwtable
define ptr @foo(ptr readnone %topage, ptr readnone %frompage) {
entry:
  %s = select i1 icmp eq (ptr getelementptr inbounds ([9 x i8], ptr @.str.1, i64 0, i64 0), ptr inttoptr (i64 1 to ptr)), ptr getelementptr inbounds ([9 x i8], ptr @.str.2, i64 0, i64 0), ptr getelementptr inbounds ([9 x i8], ptr @.str, i64 0, i64 0)
  %call = tail call ptr @bar(ptr %s)
  ret ptr %call
}

declare ptr @bar(ptr) 
