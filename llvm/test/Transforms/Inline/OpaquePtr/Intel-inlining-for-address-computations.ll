; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; INTEL CUSTOMIZATION:

; RUN: opt -opaque-pointers -passes='cgscc(inline)' -inlining-for-address-computations -inlining-for-ac-min-arg-refs=4 -inlining-for-ac-loop-depth=2 -inline-threshold=40 -inline-report=0xe807 -disable-output < %s -S 2>&1 | FileCheck %s

; Test checks that inlining happens for second bar() call site. The inlining would help optimize multiple address comptutations.

; CHECK: COMPILE FUNC: foo
; CHECK-NEXT: {{.*}}bar{{.*}}Inlining is not profitable
; CHECK-NEXT: INLINE{{.*}}bar{{.*}}Inlining for complicated address computations

%struct.S = type { [200 x ptr], [200 x ptr], [10 x ptr] }
%struct.SS = type { i64, i32, i32 }

@N = external dso_local local_unnamed_addr global i64, align 4

define dso_local i32 @foo(i64 %n, ptr %s) local_unnamed_addr {
entry:
  %arrayidx = getelementptr inbounds %struct.S, ptr %s, i64 0, i32 0, i64 0
  %op1 = load ptr, ptr %arrayidx, align 8
  %arrayidx1 = getelementptr inbounds %struct.S, ptr %s, i64 0, i32 1, i64 0
  %op2 = load ptr, ptr %arrayidx1, align 8
  %arrayidx2 = getelementptr inbounds %struct.S, ptr %s, i64 0, i32 2, i64 undef
  %op3 = load ptr, ptr %arrayidx2, align 8
  %call = call i32 @bar(ptr %op1, ptr %op2, ptr %op3)
  br label %for.cond

for.cond:                                         ; preds = %for.inc16, %entry
  %i.0 = phi i64 [ 0, %entry ], [ %inc17, %for.inc16 ]
  %sum.0 = phi i32 [ %call, %entry ], [ %sum.1, %for.inc16 ]
  %cmp = icmp slt i64 %i.0, %n
  br i1 %cmp, label %for.cond4, label %for.end18

for.cond4:                                        ; preds = %for.body8, %for.cond
  %j.0 = phi i64 [ %inc, %for.body8 ], [ 0, %for.cond ]
  %sum.1 = phi i32 [ %add, %for.body8 ], [ %sum.0, %for.cond ]
  %cmp6 = icmp slt i64 %j.0, %n
  br i1 %cmp6, label %for.body8, label %for.inc16

for.body8:                                        ; preds = %for.cond4
  %arrayidx10 = getelementptr inbounds %struct.S, ptr %s, i64 0, i32 0, i64 %i.0
  %op4 = load ptr, ptr %arrayidx10, align 8
  %arrayidx12 = getelementptr inbounds %struct.S, ptr %s, i64 0, i32 1, i64 %j.0
  %op5 = load ptr, ptr %arrayidx12, align 8
  %arrayidx14 = getelementptr inbounds %struct.S, ptr %s, i64 0, i32 2, i64 %i.0
  %op6 = load ptr, ptr %arrayidx14, align 8
  %call15 = call i32 @bar(ptr %op4, ptr %op5, ptr %op6)
  %add = add nsw i32 %sum.1, %call15
  %inc = add nuw nsw i64 %j.0, 1
  br label %for.cond4

for.inc16:                                        ; preds = %for.cond4
  %inc17 = add nuw nsw i64 %i.0, 1
  br label %for.cond

for.end18:                                        ; preds = %for.cond
  ret i32 %sum.0
}

define dso_local i32 @bar(ptr %ptr1, ptr %ptr2, ptr %ss) local_unnamed_addr {
entry:
  %a = getelementptr inbounds %struct.SS, ptr %ss, i64 0, i32 0
  %a.1 = load i64, ptr %a, align 8
  %N.1 = load i64, ptr @N, align 8
  %cmp = icmp slt i64 %a.1, %N.1
  br i1 %cmp, label %for.cond, label %if.end

for.cond:                                         ; preds = %for.body, %entry
  %ptr2.addr.0 = phi ptr [ %incdec.ptr, %for.body ], [ %ptr2, %entry ]
  %res.0 = phi i32 [ %add5, %for.body ], [ 0, %entry ]
  %i.0 = phi i64 [ %add, %for.body ], [ 0, %entry ]
  %N.2 = load i64, ptr @N, align 8
  %cmp1 = icmp slt i64 %i.0, %N.2
  br i1 %cmp1, label %for.body, label %if.end

for.body:                                         ; preds = %for.cond
  %arrayidx = getelementptr inbounds i32, ptr %ptr1, i64 %i.0
  %tmp1 = load i32, ptr %arrayidx, align 4
  %add = add nuw nsw i64 %i.0, 1
  %arrayidx2 = getelementptr inbounds i32, ptr %ptr1, i64 %add
  %tmp2 = load i32, ptr %arrayidx2, align 4
  %sub = sub nsw i32 %tmp1, %tmp2
  %b = getelementptr inbounds %struct.SS, ptr %ss, i64 0, i32 1
  %tmp3 = load i32, ptr %b, align 8
  %mul = mul nsw i32 %sub, %tmp3
  %tmp4 = load i32, ptr %ptr2.addr.0, align 4
  %add3 = add nsw i32 %mul, %tmp4
  %add.ptr = getelementptr inbounds i32, ptr %ptr2.addr.0, i64 1
  %tmp5 = load i32, ptr %add.ptr, align 4
  %sub4 = sub nsw i32 %add3, %tmp5
  %add5 = add nsw i32 %res.0, %sub4
  %incdec.ptr = getelementptr inbounds i32, ptr %ptr2.addr.0, i64 1
  br label %for.cond

if.end:                                           ; preds = %for.cond, %entry
  %res.1 = phi i32 [ %res.0, %for.cond ], [ 0, %entry ]
  ret i32 %res.1
}
; end INTEL_FEATURE_SW_ADVANCED
