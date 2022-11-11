; RUN: opt < %s -passes=function-splitting -function-splitting-min-size=2 -function-splitting-only-hot=false -S | FileCheck %s

; This is a basic test for the Intel Function Splitting pass.
;
; In this test, the code under the 'if.else' tag is never executed. With the
; the parameters specified on the command line, this code should be split
; to a separate function. The new function should get marked as 'noinline'.
;
; This is a basic test to just verify the function splitting is occuring

; CHECK-LABEL: define i32 @test(i32 %x, i32* %y)
; CHECK-LABEL: codeRepl:
; CHECK: call void @test.if.else
; CHECK: Function Attrs: noinline nounwind uwtable
; CHECK-NEXT: define internal void @test.if.else

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [10 x i8] c"hot path\0A\00", align 1
@.str.1 = private unnamed_addr constant [11 x i8] c"cold path\0A\00", align 1
@str = private unnamed_addr constant [10 x i8] c"cold path\00"
@str.2 = private unnamed_addr constant [9 x i8] c"hot path\00"

; Function Attrs: nounwind uwtable
define i32 @test(i32 %x, i32* %y) local_unnamed_addr #0 !prof !29 {
entry:
  %cmp = icmp slt i32 %x, 100
  br i1 %cmp, label %if.then, label %if.else, !prof !30

if.then:                                          ; preds = %entry
  %puts13 = call i32 @puts(i8* getelementptr inbounds ([9 x i8], [9 x i8]* @str.2, i64 0, i64 0))
  %0 = load i32, i32* %y, align 4
  br label %if.end8

if.else:                                          ; preds = %entry
  %puts = call i32 @puts(i8* getelementptr inbounds ([10 x i8], [10 x i8]* @str, i64 0, i64 0))
  %1 = load i32, i32* %y, align 4
  %cmp3 = icmp sgt i32 %1, 0
  %sub = sub nsw i32 0, %1
  %2 = select i1 %cmp3, i32 %1, i32 %sub
  br label %if.end8

if.end8:                                          ; preds = %if.else, %if.then
  %res.0 = phi i32 [ %0, %if.then ], [ %2, %if.else ]
  ret i32 %res.0
}


; Function Attrs: nounwind
declare i32 @printf(i8* nocapture readonly, ...) local_unnamed_addr #1

; Function Attrs: nounwind
declare i32 @puts(i8* nocapture readonly) #1

attributes #0 = { nounwind uwtable }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!28}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, !"ProfileSummary", !2}
!2 = !{!3, !4, !5, !6, !7, !8, !9, !10}
!3 = !{!"ProfileFormat", !"InstrProf"}
!4 = !{!"TotalCount", i64 303}
!5 = !{!"MaxCount", i64 102}
!6 = !{!"MaxInternalCount", i64 100}
!7 = !{!"MaxFunctionCount", i64 102}
!8 = !{!"NumCounts", i64 6}
!9 = !{!"NumFunctions", i64 2}
!10 = !{!"DetailedSummary", !11}
!11 = !{!12, !13, !14, !15, !16, !17, !17, !18, !18, !19, !20, !21, !22, !23, !24, !25, !26, !27}
!12 = !{i32 10000, i64 102, i32 1}
!13 = !{i32 100000, i64 102, i32 1}
!14 = !{i32 200000, i64 102, i32 1}
!15 = !{i32 300000, i64 102, i32 1}
!16 = !{i32 400000, i64 100, i32 3}
!17 = !{i32 500000, i64 100, i32 3}
!18 = !{i32 600000, i64 100, i32 3}
!19 = !{i32 700000, i64 100, i32 3}
!20 = !{i32 800000, i64 100, i32 3}
!21 = !{i32 900000, i64 100, i32 3}
!22 = !{i32 950000, i64 100, i32 3}
!23 = !{i32 990000, i64 100, i32 3}
!24 = !{i32 999000, i64 100, i32 3}
!25 = !{i32 999900, i64 100, i32 3}
!26 = !{i32 999990, i64 100, i32 3}
!27 = !{i32 999999, i64 100, i32 3}
!28 = !{!"clang version 6.0.0"}
!29 = !{!"function_entry_count", i64 100}
!30 = !{!"branch_weights", i32 100, i32 0}
