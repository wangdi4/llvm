; RUN: opt < %s -passes='require<anders-aa>,function(aa-eval)' -aa-pipeline=anders-aa -print-all-alias-modref-info -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -passes='require<anders-aa>,function(aa-eval)' -aa-pipeline=anders-aa -evaluate-loopcarried-alias -print-all-alias-modref-info -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -passes=convert-to-subscript -S | opt -passes='require<anders-aa>,function(aa-eval)' -aa-pipeline=anders-aa -print-all-alias-modref-info -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -passes=convert-to-subscript -S | opt -passes='require<anders-aa>,function(aa-eval)' -aa-pipeline=anders-aa -evaluate-loopcarried-alias -print-all-alias-modref-info -disable-output 2>&1 | FileCheck %s

@glob1 = common global [10 x i32] zeroinitializer, align 16
@p1 = internal global ptr null, align 8
@p2 = internal global ptr null, align 8
@glob2 = common global [10 x i32] zeroinitializer, align 16

; Function Attrs: noinline nounwind uwtable
define internal void @foo(ptr nocapture %p)  {
entry:
  store ptr getelementptr inbounds ([10 x i32], ptr @glob1, i64 0, i64 4), ptr %p, align 8
  ret void
}

; Function Attrs: noinline nounwind uwtable
define internal void @bar(ptr nocapture %p)  {
entry:
  %call = tail call noalias ptr @malloc(i64 40) 
  %0 = bitcast ptr %p to ptr
  store ptr %call, ptr %0, align 8
  ret void
}

; Function Attrs: nounwind
declare noalias ptr @malloc(i64) 

; CHECK: Function: test
; CHECK:   NoAlias: i32* %c, i32* %d

; Function Attrs: nounwind uwtable
define i32 @test(i32 %i)  {
entry:
  tail call void @foo(ptr nonnull @p1)
  tail call void @bar(ptr nonnull @p2)
  %d = load ptr, ptr @p1, align 8
  store i32 1, ptr %d, align 4
  %c = load ptr, ptr @p2, align 8
  store i32 2, ptr %c, align 4
  %b = load i32, ptr %d, align 4
  ret i32 %b
}
