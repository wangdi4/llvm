; RUN: opt < %s  -passes="require<anders-aa>,function(require<aa>,nonltoglobalopt)" -aa-pipeline=anders-aa -S | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@foo.p = internal unnamed_addr global i32* null, align 8
@foo_r.p = internal unnamed_addr global i32* null, align 8

; The compiler expects that @bar.p still exists as a global after
; global opt due to the setjmp call in @bar.
; This is defined up here because the output prints all global variables first.
;
; CHECK: @bar.p
@bar.p = internal unnamed_addr global i32* null, align 8
@b0 = internal global [1 x %struct.__jmp_buf_tag] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define void @foo(i32 %n, i32* %a) #0 {
; The compiler should replace the global variable @p with the local
; variable %p thus the store to @p can be eliminated.
; CHECK-LABEL: @foo(
; CHECK: alloca
entry:
  store i32* %a, i32** @foo.p, align 8, !tbaa !1
  %cmp.4 = icmp sgt i32 %n, 0
  br i1 %cmp.4, label %for.body.lr.ph, label %for.end

for.body.lr.ph:                                   ; preds = %entry
  %0 = add i32 %n, -1
  %1 = zext i32 %0 to i64
  br label %for.body

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %incdec.ptr6 = phi i32* [ %a, %for.body.lr.ph ], [ %incdec.ptr, %for.body ]
  %i.05 = phi i32 [ 0, %for.body.lr.ph ], [ %inc, %for.body ]
  %2 = load i32, i32* %incdec.ptr6, align 4, !tbaa !5
  %add = add nsw i32 %2, %i.05
  store i32 %add, i32* %incdec.ptr6, align 4, !tbaa !5
  %incdec.ptr = getelementptr inbounds i32, i32* %incdec.ptr6, i64 1
  %inc = add nuw nsw i32 %i.05, 1
  %exitcond = icmp eq i32 %inc, %n
  br i1 %exitcond, label %for.cond.for.end_crit_edge, label %for.body

for.cond.for.end_crit_edge:                       ; preds = %for.body
  %3 = add nuw nsw i64 %1, 1
  %scevgep = getelementptr i32, i32* %a, i64 %3
  store i32* %scevgep, i32** @foo.p, align 8, !tbaa !1
  br label %for.end

for.end:                                          ; preds = %for.cond.for.end_crit_edge, %entry
  ret void
}

; The intel-GlobalOpt should abort if the ReturnsTwice functions such as setjmp are present.
; NOTE: This is a negative test that makes sure a transformation doesn't happen. It's easy
; to get the same effect by just not configuring the pass pipeline correctly on the command
; line. So it's in the same file as another test case that does get optimized
; to prove that the pass is working.
;
%struct.__jmp_buf_tag = type { [8 x i64], i32, %struct.__sigset_t }
%struct.__sigset_t = type { [16 x i64] }

; Function Attrs: nounwind uwtable
define void @bar(i32 %n, i32* %a) local_unnamed_addr #0 {
; CHECK-LABEL: @bar(
; CHECK-NOT: alloca
entry:
  store i32* %a, i32** @bar.p, align 8
  %call = call i32 @_setjmp(%struct.__jmp_buf_tag* getelementptr inbounds ([1 x %struct.__jmp_buf_tag], [1 x %struct.__jmp_buf_tag]* @b0, i64 0, i64 0)) #3
  %cmp4 = icmp sgt i32 %n, 0
  br i1 %cmp4, label %for.body.lr.ph, label %for.end

for.body.lr.ph:                                   ; preds = %entry
  %bar.p.promoted = load i32*, i32** @bar.p, align 8
  %0 = add i32 %n, -1
  %1 = zext i32 %0 to i64
  %2 = add nuw nsw i64 %1, 1
  br label %for.body

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %incdec.ptr6 = phi i32* [ %bar.p.promoted, %for.body.lr.ph ], [ %incdec.ptr, %for.body ]
  %i.05 = phi i32 [ 0, %for.body.lr.ph ], [ %inc, %for.body ]
  %3 = load i32, i32* %incdec.ptr6, align 4
  %add = add nsw i32 %3, %i.05
  store i32 %add, i32* %incdec.ptr6, align 4
  %incdec.ptr = getelementptr inbounds i32, i32* %incdec.ptr6, i64 1
  %inc = add nuw nsw i32 %i.05, 1
  %exitcond = icmp eq i32 %inc, %n
  br i1 %exitcond, label %for.cond.for.end_crit_edge, label %for.body

for.cond.for.end_crit_edge:                       ; preds = %for.body
  %scevgep = getelementptr i32, i32* %bar.p.promoted, i64 %2
  store i32* %scevgep, i32** @bar.p, align 8
  br label %for.end

for.end:                                          ; preds = %for.cond.for.end_crit_edge, %entry
  ret void
}

; This cases should be avoided because of possible recursion.
; Function Attrs: nounwind uwtable
define void @foo_recurse(i32 %n, i32* %a) #0 {
; CHECK-LABEL: @foo_recurse(
; CHECK-NOT: alloca
entry:
  store i32* %a, i32** @foo_r.p, align 8, !tbaa !1
  %cmp.4 = icmp sgt i32 %n, 0
  br i1 %cmp.4, label %for.body.lr.ph, label %for.end

for.body.lr.ph:                                   ; preds = %entry
  %0 = add i32 %n, -1
  %1 = zext i32 %0 to i64
  br label %for.body

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %incdec.ptr6 = phi i32* [ %a, %for.body.lr.ph ], [ %incdec.ptr, %for.body ]
  %i.05 = phi i32 [ 0, %for.body.lr.ph ], [ %inc, %for.body ]
  %2 = load i32, i32* %incdec.ptr6, align 4, !tbaa !5
  %add = add nsw i32 %2, %i.05
  call void @foo(i32 %2, i32* %incdec.ptr6) #1
  store i32 %add, i32* %incdec.ptr6, align 4, !tbaa !5
  %incdec.ptr = getelementptr inbounds i32, i32* %incdec.ptr6, i64 1
  %inc = add nuw nsw i32 %i.05, 1
  %exitcond = icmp eq i32 %inc, %n
  br i1 %exitcond, label %for.cond.for.end_crit_edge, label %for.body

for.cond.for.end_crit_edge:                       ; preds = %for.body
  %3 = add nuw nsw i64 %1, 1
  %scevgep = getelementptr i32, i32* %a, i64 %3
  store i32* %scevgep, i32** @foo_r.p, align 8, !tbaa !1
  br label %for.end

for.end:                                          ; preds = %for.cond.for.end_crit_edge, %entry
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: nounwind returns_twice
declare i32 @_setjmp(%struct.__jmp_buf_tag*) local_unnamed_addr #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind returns_twice }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 (trunk 1485)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"any pointer", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
!5 = !{!6, !6, i64 0}
!6 = !{!"int", !3, i64 0}
