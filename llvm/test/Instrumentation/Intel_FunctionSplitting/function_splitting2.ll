; RUN: opt < %s -function-splitting -function-splitting-min-size=2 -function-splitting-only-hot=false -function-splitting-cold-threshold-percentage=5 -S | FileCheck %s
; RUN: opt < %s -passes=function-splitting -function-splitting-min-size=2 -function-splitting-only-hot=false -function-splitting-cold-threshold-percentage=5 -S | FileCheck %s

; This is a basic test for the Intel Function Splitting pass to verify
; branch frequency count gets used for the threshold, and propagates to
; the cold routine.
;
; In this test, the code under the 'if.else' tag is executed less than 5% of
; the time. With the parameters specified on the command line, this code
; should be split to a separate function.

; CHECK-LABEL: define i32 @test(i32 %x, i32* %y)
; CHECK-LABEL: codeRepl:
; CHECK: call void @test_if.else

; Verify that the function entry for the split routine is given a non-zero
; execution count based on the frequency/probability information of the original.
; We don't check for the exact value here because it's being derived by
; the code extractor functionality that is outside of the control of the
; function splitter.

; CHECK: define internal void @test_if.else(i32* %y, i32* %.out)
; CHECK-SAME: !prof ![[META_FEC:[0-9]+]]
; CHECK: ![[META_FEC]] =
; CHECK-NOT: i64 0
; CHECK-SAME: !"function_entry_count"

@str = private unnamed_addr constant [10 x i8] c"cold path\00"
@str.2 = private unnamed_addr constant [9 x i8] c"hot path\00"

define i32 @test(i32 %x, i32* %y) !prof !1 {
entry:
  %cmp = icmp slt i32 %x, 100
  br i1 %cmp, label %if.then, label %if.else, !prof !2

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

declare i32 @puts(i8*)


; These provide some counts, such that a non-zero count can be derived
; for the splinter routine.
!1 = !{!"function_entry_count", i64 10000}
!2 = !{!"branch_weights", i32 99, i32 1}
