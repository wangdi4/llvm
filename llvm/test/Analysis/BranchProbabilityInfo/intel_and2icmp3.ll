; RUN: opt < %s -analyze -branch-prob -bugpoint-enable-legacy-pm | FileCheck %s
; RUN: opt < %s -passes='print<branch-prob>' --disable-output 2>&1 | FileCheck %s

define void @test_and2cmp3_branch(i64 %spec.select303, i64 %llMinScore.fpriv.1,
                                  i64 %conv373, i64 %add374, i64 %cmp2, i64* %addr_base)
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
; CHECK:   edge loop -> land.lhs.true445 probability is 0x0147ae14 / 0x80000000 = 1.00%
; CHECK:   edge loop -> discardPoint probability is 0x7eb851ec / 0x80000000 = 99.00% [HOT edge]

land.lhs.true445:
  %cmp1 = icmp sgt i64 %conv373, %add374
  br i1 %cmp1, label %loop2, label %discardPoint

loop2:
  %cmp2.j = phi i64 [%cmp2, %land.lhs.true445], [ %j, %loop2.j ]
  br label %loop3

loop2.j:
  %j = add i64 %cmp2.j, 1
  %cmp2.slt = icmp slt i64 %j, 5
  br i1 %cmp2.slt, label %discardPoint, label %loop2

loop3:
  %add.k = phi i64 [ %spec.select303.i, %loop2 ], [ %k, %loop3 ]
  %addr = getelementptr inbounds i64, i64* %addr_base, i64 %add.k
  %data = load i64, i64* %addr, align 4
  %k = add i64 %add.k, 1
  %cmp.data = icmp slt i64 %data, 5
  br i1 %cmp.data, label %loop2.j, label %loop3

discardPoint:
  %i = add i64 %spec.select303.i, 1
  %cmp3 = icmp sgt i64 %i, %add374
  br i1 %cmp3, label %land.lhs.true445.exit, label %loop
land.lhs.true445.exit:
  ret void
}

define void @test_and2cmp3_branch_const(i64 %spec.select303, i64 %conv373, i64 %add374)
{
entry:
  br label %loop
loop:
  %spec.select303.i = phi i64 [ %spec.select303, %entry ], [ %i, %discardPoint ]
  %cmp433.not = icmp sge i64 %spec.select303.i, -2
  %cmp436 = icmp sgt i64 %conv373, 0
  %or.cond304 = and i1 %cmp433.not, %cmp436
  %cmp443 = icmp eq i64 %spec.select303.i, 1
  %or.cond305 = and i1 %or.cond304, %cmp443
  br i1 %or.cond305, label %land.lhs.true445, label %discardPoint
; CHECK:   edge loop -> land.lhs.true445 probability is 0x40000000 / 0x80000000 = 50.00%
; CHECK:   edge loop -> discardPoint probability is 0x40000000 / 0x80000000 = 50.00%

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


define void @test_and2cmp3_branch_bb(i64 %spec.select303, i64 %llMinScore.fpriv.1,
                                  i64 %conv373, i64 %add374)
{
entry:
  %cmp436 = icmp sgt i64 %conv373, 0
  br label %loop
loop:
  %spec.select303.i = phi i64 [ %spec.select303, %entry ], [ %i, %discardPoint ]
  %cmp433.not = icmp sge i64 %spec.select303.i, %llMinScore.fpriv.1
  %or.cond304 = and i1 %cmp433.not, %cmp436
  %cmp443 = icmp eq i64 %spec.select303.i, %add374
  %or.cond305 = and i1 %or.cond304, %cmp443
  br i1 %or.cond305, label %land.lhs.true445, label %discardPoint
; CHECK:   edge loop -> land.lhs.true445 probability is 0x40000000 / 0x80000000 = 50.00%
; CHECK:   edge loop -> discardPoint probability is 0x40000000 / 0x80000000 = 50.00%

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
