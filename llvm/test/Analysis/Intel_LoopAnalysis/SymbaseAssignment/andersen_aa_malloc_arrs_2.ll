; RUN: opt < %s -loop-simplify -hir-ssa-deconstruction | opt -analyze -anders-aa -symbase | FileCheck %s
; With an Andersen-AA impl, aa and bb array refs should be in diff symbase
; Find aa and it's symbase, make sure there is no bb ref before or after
; with same symbase

; CHECK-NOT: {{.*%bb.*\[.*}} {sb:[[BASE1]]}
; CHECK: {{.*%aa.*\[.*}} {sb:[[BASE1:[0-9]+]]}
; CHECK-NOT: {{.*%bb.*\[.*}} {sb:[[BASE1]]}

@A = internal global i32* null, align 8
@B = internal global i32* null, align 8

; Function Attrs: noinline nounwind uwtable
define void @foo(i32** nocapture %A)  {
entry:
  %call = tail call noalias i8* @malloc(i64 40) 
  %0 = bitcast i32** %A to i8**
  store i8* %call, i8** %0, align 8
  ret void
}

; Function Attrs: nounwind
declare noalias i8* @malloc(i64) 

; Function Attrs: noinline nounwind uwtable
define void @bar(i32** nocapture %B)  {
entry:
  %call = tail call noalias i8* @malloc(i64 40) 
  %0 = bitcast i32** %B to i8**
  store i8* %call, i8** %0, align 8
  ret void
}

; Function Attrs: noinline nounwind uwtable
define void @foo1(i32** nocapture readonly %p)  {
entry:
  %0 = load i32*, i32** %p, align 8
  store i32 1, i32* %0, align 4
  %arrayidx1 = getelementptr inbounds i32*, i32** %p, i64 1
  %1 = load i32*, i32** %arrayidx1, align 8
  store i32 2, i32* %1, align 4
  %arrayidx2 = getelementptr inbounds i32*, i32** %p, i64 2
  %2 = load i32*, i32** %arrayidx2, align 8
  store i32 3, i32* %2, align 4
  %arrayidx3 = getelementptr inbounds i32*, i32** %p, i64 3
  %3 = load i32*, i32** %arrayidx3, align 8
  store i32 4, i32* %3, align 4
  %arrayidx4 = getelementptr inbounds i32*, i32** %p, i64 4
  %4 = load i32*, i32** %arrayidx4, align 8
  store i32 5, i32* %4, align 4
  ret void
}

; Function Attrs: nounwind uwtable
define i32 @test()  {
entry:
  tail call void @foo(i32** nonnull @A)
  tail call void @bar(i32** nonnull @B)
  tail call void @foo1(i32** nonnull @B)
  %bb = load i32*, i32** @B, align 8
  %aa = load i32*, i32** @A, align 8
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %j.07 = phi i64 [ 0, %entry ], [ %inc, %for.body ]
  %arrayidx = getelementptr inbounds i32, i32* %bb, i64 %j.07
  %0 = load i32, i32* %arrayidx, align 4
  %arrayidx1 = getelementptr inbounds i32, i32* %aa, i64 %j.07
  store i32 %0, i32* %arrayidx1, align 4
  %inc = add nuw nsw i64 %j.07, 1
  %exitcond = icmp eq i64 %inc, 5
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  %1 = load i32*, i32** @A, align 8
  %arrayidx2 = getelementptr inbounds i32, i32* %1, i64 2
  %2 = load i32, i32* %arrayidx2, align 4
  ret i32 %2
}
