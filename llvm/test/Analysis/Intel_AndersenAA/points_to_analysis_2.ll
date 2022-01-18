; RUN: opt < %s -anders-aa -aa-eval -print-all-alias-modref-info -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -passes='require<anders-aa>,function(aa-eval)' -aa-pipeline=anders-aa  -print-all-alias-modref-info -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -anders-aa -aa-eval -evaluate-loopcarried-alias -print-all-alias-modref-info -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -passes='require<anders-aa>,function(aa-eval)' -aa-pipeline=anders-aa -evaluate-loopcarried-alias -print-all-alias-modref-info -disable-output 2>&1 | FileCheck %s

@p1 = internal global i32* null, align 8
@p2 = internal global i32* null, align 8
@glob1 = common global [10 x i32] zeroinitializer, align 16
@glob2 = common global [10 x i32] zeroinitializer, align 16

; Function Attrs: noinline nounwind uwtable
define void @foo(i32** nocapture %p)  {
entry:
  %call = tail call noalias i8* @malloc(i64 40) 
  %0 = bitcast i32** %p to i8**
  store i8* %call, i8** %0, align 8
  ret void
}

; Function Attrs: nounwind
declare noalias i8* @malloc(i64) 

; Function Attrs: noinline nounwind uwtable
define void @bar(i32** nocapture %p)  {
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
  tail call void @foo(i32** nonnull @p1)
  tail call void @bar(i32** nonnull @p2)
  %d = load i32*, i32** @p1, align 8
  store i32 1, i32* %d, align 4
  %c = load i32*, i32** @p2, align 8
  store i32 2, i32* %c, align 4
  %b = load i32, i32* %d, align 4
  ret i32 %b
}
