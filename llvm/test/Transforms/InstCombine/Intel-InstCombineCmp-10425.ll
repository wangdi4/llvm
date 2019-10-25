; RUN: opt < %s -S -instcombine | FileCheck %s

; CMPLRLLVM-10425
;
;  %call.ptr = inttoptr i64 %call to i8*
;  %call.neg = sub i64 0, %call
;  %add.ptr = getelementptr inbounds i8, i8* %call.ptr, i64 %call.neg
;  %cmp = icmp eq i8* %add.ptr, null
;  %conv = zext i1 %cmp to i32
;
; The GEP %add.ptr is always null, because %call.neg is -(%call.ptr).
; This violates the inbounds keyword assumption, which says that %add.ptr
; may only be null if %call.ptr is null.
; InstCombine correctly removes the GEP and replaces it with %call.ptr.
; But this breaks 502.gcc, boost, and others (obstack.h)

; This optimization is legal and we still want to allow it for other
; cases. Test function opt_this_one below will check one other case.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [22 x i8] c"This should be 1: %d\0A\00", align 1

declare dso_local i64 @ptrtoint(float* %p) local_unnamed_addr #0;

; CHECK: %add.ptr = getelementptr inbounds i8, i8* %call.ptr, i64 %call.neg
; CHECK-NEXT: %cmp = icmp eq i8* %add.ptr, null
define dso_local i32 @dont_opt() local_unnamed_addr #1 {
entry:
  %f = alloca float, align 4
  %0 = bitcast float* %f to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #4
  %call = call i64 @ptrtoint(float* %f)
  %call.ptr = inttoptr i64 %call to i8*
  %call.neg = sub i64 0, %call
  %add.ptr = getelementptr inbounds i8, i8* %call.ptr, i64 %call.neg
  %cmp = icmp eq i8* %add.ptr, null
  %conv = zext i1 %cmp to i32
  %call1 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([22 x i8], [22 x i8]* @.str, i64 0, i64 0), i32 %conv)
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %0) #4
  ret i32 0
}

; We want to optimize this case, even though the ptrtoint() call may return
; the same value.

; CHECK-NOT: %add.ptr = getelementptr inbounds i8
; Function Attrs: nounwind uwtable
define dso_local i32 @opt_this_one() local_unnamed_addr #1 {
entry:
  %f = alloca float, align 4
  %0 = bitcast float* %f to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #4
  %call = call i64 @ptrtoint(float* %f)
  %sub = sub i64 0, %call
  %call1 = call i64 @ptrtoint(float* %f)
  %1 = inttoptr i64 %call1 to float*
  %2 = bitcast float* %1 to i8*
  %add.ptr = getelementptr inbounds i8, i8* %2, i64 %sub
  %cmp = icmp eq i8* %add.ptr, null
  %conv = zext i1 %cmp to i32
  %call2 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([22 x i8], [22 x i8]* @.str, i64 0, i64 0), i32 %conv)
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %0) #4
  ret i32 0
}

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #2

; Function Attrs: nofree nounwind
declare dso_local i32 @printf(i8* nocapture readonly, ...) local_unnamed_addr #3

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #2

attributes #0 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { argmemonly nounwind willreturn }
attributes #3 = { nofree nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { nounwind }
