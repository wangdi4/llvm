; REQUIRES: asserts
; RUN: opt < %s -passes="require<anders-aa>,loop-simplify,hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=symbase-assignment -debug-only=hir-framework 2>&1 | FileCheck %s
; With an Andersen-AA impl, aa and bb array refs should be in diff symbase
; Find aa and it's symbase, make sure there is no bb ref before or after
; with same symbase

; CHECK-NOT: {{.*%bb.*\[.*}} {sb:[[BASE1]]}
; CHECK: {{.*%aa.*\[.*}} {sb:[[BASE1:[0-9]+]]}
; CHECK-NOT: {{.*%bb.*\[.*}} {sb:[[BASE1]]}

@A = internal unnamed_addr global ptr null, align 8
@B = internal global ptr null, align 8

; Function Attrs: noinline nounwind uwtable
define noalias ptr @foo()  {
entry:
  %call = tail call noalias ptr @malloc(i64 40)
  ret ptr %call
}

; Function Attrs: nounwind
declare noalias ptr @malloc(i64)

; Function Attrs: noinline nounwind uwtable
define noalias ptr @bar()  {
entry:
  %call = tail call noalias ptr @malloc(i64 40)
  ret ptr %call
}

; Function Attrs: noinline nounwind uwtable
define internal void @foo1(ptr nocapture readonly %p)  {
entry:
  %0 = load ptr, ptr %p, align 8
  store i32 1, ptr %0, align 4
  %arrayidx1 = getelementptr inbounds ptr, ptr %p, i64 1
  %1 = load ptr, ptr %arrayidx1, align 8
  store i32 2, ptr %1, align 4
  %arrayidx2 = getelementptr inbounds ptr, ptr %p, i64 2
  %2 = load ptr, ptr %arrayidx2, align 8
  store i32 3, ptr %2, align 4
  %arrayidx3 = getelementptr inbounds ptr, ptr %p, i64 3
  %3 = load ptr, ptr %arrayidx3, align 8
  store i32 4, ptr %3, align 4
  %arrayidx4 = getelementptr inbounds ptr, ptr %p, i64 4
  %4 = load ptr, ptr %arrayidx4, align 8
  store i32 5, ptr %4, align 4
  ret void
}

; Function Attrs: nounwind uwtable
define i32 @test()  {
entry:
  %call = tail call ptr @foo()
  store ptr %call, ptr @A, align 8
  %call1 = tail call ptr @bar()
  store ptr %call1, ptr @B, align 8
  tail call void @foo1(ptr nonnull @B)
  %bb = load ptr, ptr @B, align 8
  %aa = load ptr, ptr @A, align 8
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %j.08 = phi i64 [ 0, %entry ], [ %inc, %for.body ]
  %arrayidx = getelementptr inbounds i32, ptr %bb, i64 %j.08
  %0 = load i32, ptr %arrayidx, align 4
  %arrayidx2 = getelementptr inbounds i32, ptr %aa, i64 %j.08
  store i32 %0, ptr %arrayidx2, align 4
  %inc = add nuw nsw i64 %j.08, 1
  %exitcond = icmp eq i64 %inc, 5
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  %1 = load ptr, ptr @A, align 8
  %arrayidx3 = getelementptr inbounds i32, ptr %1, i64 2
  %2 = load i32, ptr %arrayidx3, align 4
  ret i32 %2
}
