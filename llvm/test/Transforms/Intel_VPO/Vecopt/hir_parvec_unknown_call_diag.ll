; REQUIRES: asserts
; RUN: opt -enable-new-pm=0 < %s -hir-ssa-deconstruction -analyze -hir-parvec-analysis 2>&1 | FileCheck %s
; RUN: opt < %s -passes='hir-ssa-deconstruction,print<hir-parvec-analysis>' -disable-output  2>&1 | FileCheck %s

@arr.i32.1 = common local_unnamed_addr global [1024 x i32] zeroinitializer, align 16
@arr.i32.2 = common local_unnamed_addr global [1024 x i32] zeroinitializer, align 16
@arr.i32.3 = common local_unnamed_addr global [1024 x i32] zeroinitializer, align 16

declare {i32, i1} @llvm.uadd.with.overflow.i32(i32 %a, i32 %b)

define void @doit() local_unnamed_addr #0 {
; Ensure that correct diagnostic is emitted for a call that prevents vectorization.

; CHECK: was not vectorized: function call to %s cannot be vectorized
entry:
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %ld.idx = getelementptr inbounds [1024 x i32], [1024 x i32]* @arr.i32.1, i64 0, i64 %indvars.iv

  %ld = load i32, i32* %ld.idx

  %ld2.idx = getelementptr inbounds [1024 x i32], [1024 x i32]* @arr.i32.3, i64 0, i64 %indvars.iv
  %ld2 = load i32, i32* %ld2.idx

  %res = call {i32, i1} @llvm.uadd.with.overflow.i32(i32 %ld, i32 %ld2)
  %add = extractvalue {i32, i1} %res, 0

  %st.idx = getelementptr inbounds [1024 x i32], [1024 x i32]* @arr.i32.2, i64 0, i64 %indvars.iv
  store i32 %add, i32* %st.idx

  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}
