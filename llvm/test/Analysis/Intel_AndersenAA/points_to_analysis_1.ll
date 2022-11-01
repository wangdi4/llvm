; RUN: opt < %s -passes='require<anders-aa>,function(aa-eval)' -aa-pipeline=anders-aa -print-all-alias-modref-info -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -passes='require<anders-aa>,function(aa-eval)' -aa-pipeline=anders-aa -evaluate-loopcarried-alias -print-all-alias-modref-info -disable-output 2>&1 | FileCheck %s
@p1 = internal unnamed_addr global i32* null, align 8
@p2 = internal global i32* null, align 8

; Function Attrs: noinline nounwind uwtable
define noalias i32* @foo() {
entry:
  %call = tail call noalias i8* @malloc(i64 40)
  %0 = bitcast i8* %call to i32*
  ret i32* %0
}

; Function Attrs: nounwind
declare noalias i8* @malloc(i64) 

; Function Attrs: noinline nounwind uwtable
define noalias i32* @bar()  {
entry:
  %call = tail call noalias i8* @malloc(i64 40) 
  %0 = bitcast i8* %call to i32*
  ret i32* %0
}

; Function Attrs: noinline nounwind uwtable
define internal void @foo1(i32** nocapture %p)  {
entry:
  %call = tail call noalias i8* @malloc(i64 40) 
  %0 = bitcast i32** %p to i8**
  store i8* %call, i8** %0, align 8
  ret void
}

; CHECK: Function: test
; CHECK:   NoAlias: i32* %c, i32* %d

; Function Attrs: nounwind uwtable
define i32 @test(i32 %i)  {
entry:
  %call = tail call i32* @foo()
  store i32* %call, i32** @p1, align 8
  %call1 = tail call i32* @bar()
  store i32* %call1, i32** @p2, align 8
  tail call void @foo1(i32** nonnull @p2)
  %d = load i32*, i32** @p1, align 8
  store i32 1, i32* %d, align 4
  %c = load i32*, i32** @p2, align 8
  store i32 2, i32* %c, align 4
  %b = load i32, i32* %d, align 4
  ret i32 %b
}
