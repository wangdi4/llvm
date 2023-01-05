; RUN: opt < %s -passes='require<anders-aa>'  -disable-output 2>/dev/null
; Test for a bug (CQ378470) in Andersens analysis that caused crash when
; Instruction::Select is passed as argument to Instruction::Call.

@.str = private unnamed_addr constant [9 x i8] c"string_1\00", align 1
@.str.1 = private unnamed_addr constant [9 x i8] c"string_2\00", align 1
@.str.2 = private unnamed_addr constant [9 x i8] c"string_3\00", align 1

; Function Attrs: nounwind uwtable
define i8* @foo(i8* readnone %topage, i8* readnone %frompage) {
entry:
  %call = tail call i8* @bar(i8* select (i1 icmp eq (i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str.1, i64 0, i64 0), i8* inttoptr (i64 1 to i8*)), i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str.2, i64 0, i64 0), i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str, i64 0, i64 0)))
  ret i8* %call
}

declare i8* @bar(i8*) 
