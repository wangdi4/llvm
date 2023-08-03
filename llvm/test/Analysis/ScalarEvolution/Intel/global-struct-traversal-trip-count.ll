; RUN: opt < %s -analyze -bugpoint-enable-legacy-pm -scalar-evolution | FileCheck %s
; RUN: opt < %s -disable-output "-passes=print<scalar-evolution>" 2>&1 | FileCheck %s

; Verify that we are able to simplify the backedge taken count of this loop iterating the global structure to 1.

; CHECK: Loop %loop: backedge-taken count is 1

%struct = type { i32, i32 }

@glob_const = internal constant [2 x %struct] [%struct { i32 4, i32 5 }, %struct { i32 8, i32 9 }], align 16

define void @foo() {
entry:
  br label %loop

loop:                                              ; preds = %loop, %entry
  %iv = phi ptr [ @glob_const, %entry ], [ %iv.inc, %loop ]
  %ld = load i32, ptr %iv, align 8
  %iv.inc = getelementptr inbounds %struct, ptr %iv, i64 1
  %cmp = icmp ult ptr %iv.inc, getelementptr inbounds ([2 x %struct], ptr @glob_const, i64 1, i64 0)
  br i1 %cmp, label %loop, label %exit

exit:
  ret void
}

