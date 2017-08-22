; It checks anders-aa helps LICM to hoist invariant load out of loop
; RUN: opt < %s  -anders-aa -licm  -disable-output  -stats -S 2>&1 | grep "1 licm"
; REQUIRES: asserts

@p = internal unnamed_addr global i32* null, align 8
@A = common global [100 x i32] zeroinitializer, align 16
@B = common global [100 x i32] zeroinitializer, align 16

; Function Attrs: noinline nounwind uwtable
define void @foo1()  {
entry:
  %call = tail call noalias i8* @malloc(i64 40) 
  store i8* %call, i8** bitcast (i32** @p to i8**), align 8
  ret void
}

; Function Attrs: nounwind
declare noalias i8* @malloc(i64) 

; Function Attrs: noinline nounwind uwtable
define void @foo()  {
entry:
  %call = tail call noalias i8* @malloc(i64 40) 
  store i8* %call, i8** bitcast (i32** @p to i8**), align 8
  ret void
}

; Function Attrs: nounwind uwtable
define i32 @test()  {
entry:
  tail call void @foo1()
  %0 = load i32*, i32** @p, align 8
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds [100 x i32], [100 x i32]* @A, i64 0, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx, align 4
  %2 = load i32, i32* %0, align 4      ; hoist invariant load
  %mul = mul nsw i32 %2, %1
  %arrayidx2 = getelementptr inbounds [100 x i32], [100 x i32]* @B, i64 0, i64 %indvars.iv
  store i32 %mul, i32* %arrayidx2, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  %3 = load i32, i32* getelementptr inbounds ([100 x i32], [100 x i32]* @B, i64 0, i64 2), align 8
  ret i32 %3
}

