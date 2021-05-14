; RUN: opt < %s -analyze -branch-prob -enable-new-pm=0 | FileCheck %s
; RUN: opt < %s -passes='print<branch-prob>' --disable-output 2>&1 | FileCheck %s

define void @test_and2cmp3_branch(i64 %spec.select303, i64 %llMinScore.fpriv.1,
                                  i64 %conv373, i64 %add374)
{
entry:
  br label %loop
loop:
  %spec.select303.i = phi i64 [ %spec.select303, %entry ], [ %i, %discardPoint ]
  %cmp433.not = icmp sge i64 %spec.select303.i, %llMinScore.fpriv.1
  %cmp436 = icmp sgt i64 %conv373, 0
  %or.cond304 = and i1 %cmp433.not, %cmp436
  %cmp443 = icmp eq i64 %spec.select303.i, %add374
  %or.cond305 = and i1 %or.cond304, %cmp443
  br i1 %or.cond305, label %land.lhs.true445, label %discardPoint
; CHECK:   edge loop -> land.lhs.true445 probability is 0x10a3d70a / 0x80000000 = 13.00%
; CHECK:   edge loop -> discardPoint probability is 0x6f5c28f6 / 0x80000000 = 87.00% [HOT edge]

land.lhs.true445:
  %cmp1 = icmp sgt i64 %conv373, %add374
  br i1 %cmp1, label %land.lhs.true445.exit, label %discardPoint
discardPoint:
  %i = add i64 %spec.select303.i, 1
  %cmp2 = icmp sgt i64 %i, %add374
  br i1 %cmp2, label %land.lhs.true445.exit, label %loop
land.lhs.true445.exit:
  ret void
}
