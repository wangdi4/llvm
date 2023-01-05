; RUN: opt < %s -passes=function-splitting -function-splitting-min-size=5 -function-splitting-only-hot=false -S | FileCheck %s

; This is a basic test for the Intel Function Splitting pass.
;
; In this test, the code under the 'if.else' tag is never executed. With the
; the parameters specified on the command line, this code should be split
; to a separate function.
;
; This is a test to verify the function splitting is occuring when there
; are several blocks that exit the region to a common successor.

; CHECK-LABEL: define void @test(i32 %x, i32* %y)
; CHECK-LABEL: codeRepl:
; CHECK: call void @test.if.else
; CHECK: define internal void @test.if.else

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [10 x i8] c"hot path\0A\00", align 1
@gValue1 = common local_unnamed_addr global i32 0, align 4
@.str.1 = private unnamed_addr constant [11 x i8] c"cold path\0A\00", align 1
@gValue2 = common local_unnamed_addr global i32 0, align 4
@.str.2 = private unnamed_addr constant [27 x i8] c"only reached via hot path\0A\00", align 1
@gValue3 = common local_unnamed_addr global i32 0, align 4
@str = private unnamed_addr constant [10 x i8] c"cold path\00"
@str.3 = private unnamed_addr constant [26 x i8] c"only reached via hot path\00"
@str.4 = private unnamed_addr constant [9 x i8] c"hot path\00"

; Function Attrs: inlinehint nounwind uwtable
define void @test(i32 %x, i32* %y) local_unnamed_addr #0 !prof !29 {
entry:
  %cmp = icmp slt i32 %x, 100
  br i1 %cmp, label %if.then, label %if.else, !prof !30

if.then:                                          ; preds = %entry
  %puts27 = call i32 @puts(i8* getelementptr inbounds ([9 x i8], [9 x i8]* @str.4, i64 0, i64 0))
  %0 = load i32, i32* %y, align 4
  store i32 %0, i32* @gValue1, align 4
  br label %if.end15

if.else:                                          ; preds = %entry
  %puts = call i32 @puts(i8* getelementptr inbounds ([10 x i8], [10 x i8]* @str, i64 0, i64 0))
  %1 = load i32, i32* %y, align 4
  %cmp3 = icmp sgt i32 %1, 0
  br i1 %cmp3, label %land.lhs.true, label %if.end15

land.lhs.true:                                    ; preds = %if.else
  %arrayidx4 = getelementptr inbounds i32, i32* %y, i64 1
  %2 = load i32, i32* %arrayidx4, align 4
  %cmp5 = icmp sgt i32 %2, 0
  br i1 %cmp5, label %land.lhs.true6, label %if.end15

land.lhs.true6:                                   ; preds = %land.lhs.true
  %arrayidx7 = getelementptr inbounds i32, i32* %y, i64 2
  %3 = load i32, i32* %arrayidx7, align 4
  %cmp8 = icmp sgt i32 %3, 0
  br i1 %cmp8, label %if.then9, label %if.end15

if.then9:                                         ; preds = %land.lhs.true6
  %add = add nsw i32 %1, %x
  %add12 = add nsw i32 %add, %2
  %add14 = add nsw i32 %add12, %3
  store i32 %add14, i32* @gValue2, align 4
  br label %if.end15

if.end15:                                         ; preds = %if.else, %land.lhs.true, %land.lhs.true6, %if.then9, %if.then
  %puts26 = call i32 @puts(i8* getelementptr inbounds ([26 x i8], [26 x i8]* @str.3, i64 0, i64 0))
  store i32 0, i32* @gValue3, align 4
  ret void
}

; Function Attrs: nounwind
declare i32 @printf(i8* nocapture readonly, ...) local_unnamed_addr #1

; Function Attrs: nounwind
declare i32 @puts(i8* nocapture readonly) #2

attributes #0 = { inlinehint nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }

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
!8 = !{!"NumCounts", i64 8}
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
