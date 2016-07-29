; RUN: opt < %s  -anders-aa -domtree -loops -loop-simplify -lcssa -basicaa -aa -nonltoglobalopt  -S | FileCheck %s

; The intel-GlobalOpt should abort if the ReturnsTwice functions such as setjmp are present.
;
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.__jmp_buf_tag = type { [8 x i64], i32, %struct.__sigset_t }
%struct.__sigset_t = type { [16 x i64] }

; The compiler expects that @foo.p still exists as a global after
; global opt due to the setjmp call in @foo.
;
; CHECK: @foo.p
@foo.p = internal unnamed_addr global i32* null, align 8
@b0 = internal global [1 x %struct.__jmp_buf_tag] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define void @foo(i32 %n, i32* %a) local_unnamed_addr #0 {
entry:
  store i32* %a, i32** @foo.p, align 8
  %call = call i32 @_setjmp(%struct.__jmp_buf_tag* getelementptr inbounds ([1 x %struct.__jmp_buf_tag], [1 x %struct.__jmp_buf_tag]* @b0, i64 0, i64 0)) #3
  %cmp4 = icmp sgt i32 %n, 0
  br i1 %cmp4, label %for.body.lr.ph, label %for.end

for.body.lr.ph:                                   ; preds = %entry
  %foo.p.promoted = load i32*, i32** @foo.p, align 8
  %0 = add i32 %n, -1
  %1 = zext i32 %0 to i64
  %2 = add nuw nsw i64 %1, 1
  br label %for.body

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %incdec.ptr6 = phi i32* [ %foo.p.promoted, %for.body.lr.ph ], [ %incdec.ptr, %for.body ]
  %i.05 = phi i32 [ 0, %for.body.lr.ph ], [ %inc, %for.body ]
  %3 = load i32, i32* %incdec.ptr6, align 4
  %add = add nsw i32 %3, %i.05
  store i32 %add, i32* %incdec.ptr6, align 4
  %incdec.ptr = getelementptr inbounds i32, i32* %incdec.ptr6, i64 1
  %inc = add nuw nsw i32 %i.05, 1
  %exitcond = icmp eq i32 %inc, %n
  br i1 %exitcond, label %for.cond.for.end_crit_edge, label %for.body

for.cond.for.end_crit_edge:                       ; preds = %for.body
  %scevgep = getelementptr i32, i32* %foo.p.promoted, i64 %2
  store i32* %scevgep, i32** @foo.p, align 8
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

attributes #3 = { nounwind returns_twice }

