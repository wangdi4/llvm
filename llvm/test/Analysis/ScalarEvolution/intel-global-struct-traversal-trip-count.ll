; RUN: opt < %s -analyze -enable-new-pm=0 -scalar-evolution | FileCheck %s
; RUN: opt < %s -disable-output "-passes=print<scalar-evolution>" 2>&1 | FileCheck %s

; Verify that we are able to simplify the backedge taken count of this loop iterating the global structure to 1.

; NOTE: the backedge-taken count used to be 1 but it isn't simplified anymore due to community changes.

; CHECK: Loop %loop: backedge-taken count is ((-1 + (-1 * (ptrtoint [2 x %struct]* @glob_const to i64)) + ((8 + (ptrtoint [2 x %struct]* @glob_const to i64))<nuw><nsw> umax (16 + (ptrtoint [2 x %struct]* @glob_const to i64)))) /u 8)


%struct = type { i32, i32 }

@glob_const = internal constant [2 x %struct] [%struct { i32 4, i32 5 }, %struct { i32 8, i32 9 }], align 16

define void @foo() {
entry:
  br label %loop

loop:                                              ; preds = %loop, %entry
  %iv = phi %struct* [ getelementptr inbounds ([2 x %struct], [2 x %struct]* @glob_const, i64 0, i64 0), %entry ], [ %iv.inc, %loop ]
  %gep = getelementptr inbounds %struct, %struct* %iv, i64 0, i32 0
  %ld = load i32, i32* %gep, align 8
  %iv.inc = getelementptr inbounds %struct, %struct* %iv, i64 1
  %cmp = icmp ult %struct* %iv.inc, getelementptr inbounds ([2 x %struct], [2 x %struct]* @glob_const, i64 1, i64 0)
  br i1 %cmp, label %loop, label %exit

exit:
  ret void
}

