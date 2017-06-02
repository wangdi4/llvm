; Initially, this test was written to check if LICM is using AndersensAA
; results at O2. Now, LICM is not running after Andersens-Analysis since
; Andersens-Analysis is moved closer to Intel specific passes like HIR,
; Vec etc. So, this test is fixed to check LICM is using AndersensAA if
; LICM runs after Andersens-Analysis.

; It checks LICM to hoist invariant load out of loop using Andersens
; points-to info. 
; RUN: opt < %s -S -anders-aa -licm -disable-output -stats 2>&1 | grep "1 licm"
; REQUIRES: asserts


@A = common global [100 x i32] zeroinitializer, align 16
@p = internal unnamed_addr global i32* null, align 8
@B = common global [100 x i32] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define i32 @test() #0 {
entry:
  tail call void @foo1()
  %0 = load i32*, i32** @p, align 8
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds [100 x i32], [100 x i32]* @A, i64 0, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx, align 4
  %2 = load i32, i32* %0, align 4       ; hoist invariant load
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

; Function Attrs: noinline nounwind uwtable
define void @foo1() #1 {
entry:
  %call = tail call noalias i8* @malloc(i64 40) #3
  store i8* %call, i8** bitcast (i32** @p to i8**), align 8
  ret void
}

; Function Attrs: nounwind
declare noalias i8* @malloc(i64) #2

; Function Attrs: noinline nounwind uwtable
define void @foo() #1 {
entry:
  %call = tail call noalias i8* @malloc(i64 40) #3
  store i8* %call, i8** bitcast (i32** @p to i8**), align 8
  ret void
}

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { noinline nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }
