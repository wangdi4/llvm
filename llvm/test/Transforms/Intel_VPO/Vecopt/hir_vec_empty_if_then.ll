; LLVM IR generated from testcase below using icx -O1 -S -emit-llvm
; void foo(int *Mask, int n, int element, int scale) {
;   int Lo = scale * element;
;   int Hi = scale * (element + 1);
;   for (int i = 0; i < n; ++i)
;     if (i < Lo || i >= Hi)
;       Mask[i] = -1;
; }
;
; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -vplan-force-vf=4 -S -print-after=hir-vplan-vec  < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>" -vplan-force-vf=4 -S < %s 2>&1 | FileCheck %s

; CHECK: DO i1 = 0, {{.*}}, 4 
; CHECK:   [[Cmp1:%.*]] = i1 + <i64 0, i64 1, i64 2, i64 3> >= (%element * %scale);
; CHECK:   [[Cmp2:%.*]] = i1 + <i64 0, i64 1, i64 2, i64 3> < ((1 + %element) * %scale);
; CHECK:   [[And:%.*]] = [[Cmp1]] &  [[Cmp2]]
; CHECK:   [[FPred:%.*]] = [[And]]  ^  -1;
; CHECK:   (<4 x i32>*)(%Mask)[i1] = -1, Mask = @{[[FPred]]};
; CHECK: END LOOP
define void @foo(i32* %Mask, i32 %n, i32 %element, i32 %scale) {
entry:
  %cmp13 = icmp sgt i32 %n, 0
  br i1 %cmp13, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  %add = add nsw i32 %element, 1
  %mul1 = mul nsw i32 %add, %scale
  %mul = mul nsw i32 %scale, %element
  %0 = sext i32 %mul1 to i64
  %1 = sext i32 %mul to i64
  %wide.trip.count = sext i32 %n to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.inc
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.inc, %for.body.preheader
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.inc ]
  %cmp2 = icmp sge i64 %indvars.iv, %1
  %cmp3 = icmp slt i64 %indvars.iv, %0
  %or.cond = and i1 %cmp2, %cmp3
  br i1 %or.cond, label %for.inc, label %if.then

if.then:                                          ; preds = %for.body
  %arrayidx = getelementptr inbounds i32, i32* %Mask, i64 %indvars.iv
  store i32 -1, i32* %arrayidx, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  %indvars.iv.in = bitcast i64 %indvars.iv.next to i64
  br i1 %exitcond, label %for.cond.cleanup.loopexit, label %for.body
}
