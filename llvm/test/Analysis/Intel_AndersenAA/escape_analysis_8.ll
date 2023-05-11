; RUN: opt < %s -passes='require<anders-aa>' -print-non-escape-candidates -disable-output 2>&1 | grep "foo." | count 2
; RUN: opt < %s -passes=convert-to-subscript -S | opt -passes='require<anders-aa>' -print-non-escape-candidates -disable-output 2>&1 | grep "foo." | count 2

; Non-Escape-Static-Vars_Begin
; foo.fooBuf<mem>
; foo.init<mem>
; Non-Escape-Static-Vars_End
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@foo.init = internal unnamed_addr global i1 false
@foo.fooBuf = internal unnamed_addr global ptr null, align 8
@foo.local_fooBuf = internal global [2048 x i32] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define i32 @foo(ptr nocapture %fooPtr, i32 %aconst, i32 %n) #0 {
entry:
  %.b = load i1, ptr @foo.init, align 1
  br i1 %.b, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  store ptr @foo.local_fooBuf, ptr @foo.fooBuf, align 8, !tbaa !1
  store i1 true, ptr @foo.init, align 1
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  %call = tail call ptr (...) @bar() #2
  %i = load i64, ptr @foo.fooBuf, align 8, !tbaa !1
  %i1 = bitcast ptr %call to ptr
  store i64 %i, ptr %i1, align 8, !tbaa !1
  %cmp3.16 = icmp sgt i32 %aconst, 0
  br i1 %cmp3.16, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %if.end
  %.cast = inttoptr i64 %i to ptr
  %idxprom1 = sext i32 %n to i64
  %div = sdiv i32 %aconst, 2
  %idxprom = sext i32 %div to i64
  %arrayidx2 = getelementptr inbounds [2 x [1024 x i32]], ptr %.cast, i64 0, i64 %idxprom1, i64 %idxprom
  %i2 = load i32, ptr %arrayidx2, align 4, !tbaa !5
  %i3 = add i32 %aconst, -1
  %i4 = zext i32 %i3 to i64
  %i5 = add nuw nsw i64 %i4, 1
  %min.iters.check = icmp ult i64 %i5, 8
  br i1 %min.iters.check, label %for.body.preheader32, label %min.iters.checked

min.iters.checked:                                ; preds = %for.body.preheader
  %n.vec = and i64 %i5, 8589934584
  %cmp.zero = icmp eq i64 %n.vec, 0
  br i1 %cmp.zero, label %for.body.preheader32, label %vector.ph

vector.ph:                                        ; preds = %min.iters.checked
  %broadcast.splatinsert25 = insertelement <4 x i32> undef, i32 %i2, i32 0
  %broadcast.splat26 = shufflevector <4 x i32> %broadcast.splatinsert25, <4 x i32> undef, <4 x i32> zeroinitializer
  %i6 = add i32 %aconst, -1
  %i7 = zext i32 %i6 to i64
  %i8 = add nuw nsw i64 %i7, 1
  %i9 = and i64 %i8, 8589934584
  %i10 = add nsw i64 %i9, -8
  %i11 = lshr exact i64 %i10, 3
  %i12 = and i64 %i11, 1
  %lcmp.mod = icmp eq i64 %i12, 0
  br i1 %lcmp.mod, label %vector.body.prol, label %vector.ph.split

vector.body.prol:                                 ; preds = %vector.ph
  %i13 = bitcast ptr %fooPtr to ptr
  store <4 x i32> <i32 0, i32 1, i32 2, i32 3>, ptr %i13, align 4, !tbaa !5
  %i14 = getelementptr i32, ptr %fooPtr, i64 4
  %i15 = bitcast ptr %i14 to ptr
  store <4 x i32> <i32 4, i32 5, i32 6, i32 7>, ptr %i15, align 4, !tbaa !5
  %i16 = add nsw <4 x i32> %broadcast.splat26, <i32 undef, i32 0, i32 0, i32 0>
  br label %vector.ph.split

vector.ph.split:                                  ; preds = %vector.body.prol, %vector.ph
  %.lcssa.unr = phi <4 x i32> [ undef, %vector.ph ], [ %i16, %vector.body.prol ]
  %index.unr = phi i64 [ 0, %vector.ph ], [ 8, %vector.body.prol ]
  %vec.phi.unr = phi <4 x i32> [ <i32 undef, i32 0, i32 0, i32 0>, %vector.ph ], [ %i16, %vector.body.prol ]
  %vec.phi20.unr = phi <4 x i32> [ zeroinitializer, %vector.ph ], [ %broadcast.splat26, %vector.body.prol ]
  %i17 = icmp eq i64 %i11, 0
  br i1 %i17, label %middle.block, label %vector.ph.split.split

vector.ph.split.split:                            ; preds = %vector.ph.split
  br label %vector.body

vector.body:                                      ; preds = %vector.body, %vector.ph.split.split
  %index = phi i64 [ %index.unr, %vector.ph.split.split ], [ %index.next.1, %vector.body ]
  %vec.phi = phi <4 x i32> [ %vec.phi.unr, %vector.ph.split.split ], [ %i30, %vector.body ]
  %vec.phi20 = phi <4 x i32> [ %vec.phi20.unr, %vector.ph.split.split ], [ %i31, %vector.body ]
  %i18 = getelementptr inbounds i32, ptr %fooPtr, i64 %index
  %i19 = trunc i64 %index to i32
  %broadcast.splatinsert21 = insertelement <4 x i32> undef, i32 %i19, i32 0
  %broadcast.splat22 = shufflevector <4 x i32> %broadcast.splatinsert21, <4 x i32> undef, <4 x i32> zeroinitializer
  %induction23 = add <4 x i32> %broadcast.splat22, <i32 0, i32 1, i32 2, i32 3>
  %induction24 = add <4 x i32> %broadcast.splat22, <i32 4, i32 5, i32 6, i32 7>
  %i20 = bitcast ptr %i18 to ptr
  store <4 x i32> %induction23, ptr %i20, align 4, !tbaa !5
  %i21 = getelementptr i32, ptr %i18, i64 4
  %i22 = bitcast ptr %i21 to ptr
  store <4 x i32> %induction24, ptr %i22, align 4, !tbaa !5
  %i23 = add nsw <4 x i32> %broadcast.splat26, %vec.phi
  %i24 = add nsw <4 x i32> %broadcast.splat26, %vec.phi20
  %index.next = add i64 %index, 8
  %i25 = getelementptr inbounds i32, ptr %fooPtr, i64 %index.next
  %i26 = trunc i64 %index.next to i32
  %broadcast.splatinsert21.1 = insertelement <4 x i32> undef, i32 %i26, i32 0
  %broadcast.splat22.1 = shufflevector <4 x i32> %broadcast.splatinsert21.1, <4 x i32> undef, <4 x i32> zeroinitializer
  %induction23.1 = add <4 x i32> %broadcast.splat22.1, <i32 0, i32 1, i32 2, i32 3>
  %induction24.1 = add <4 x i32> %broadcast.splat22.1, <i32 4, i32 5, i32 6, i32 7>
  %i27 = bitcast ptr %i25 to ptr
  store <4 x i32> %induction23.1, ptr %i27, align 4, !tbaa !5
  %i28 = getelementptr i32, ptr %i25, i64 4
  %i29 = bitcast ptr %i28 to ptr
  store <4 x i32> %induction24.1, ptr %i29, align 4, !tbaa !5
  %i30 = add nsw <4 x i32> %broadcast.splat26, %i23
  %i31 = add nsw <4 x i32> %broadcast.splat26, %i24
  %index.next.1 = add i64 %index, 16
  %i32 = icmp eq i64 %index.next.1, %n.vec
  br i1 %i32, label %middle.block.unr-lcssa, label %vector.body, !llvm.loop !7

middle.block.unr-lcssa:                           ; preds = %vector.body
  %.lcssa35 = phi <4 x i32> [ %i31, %vector.body ]
  %.lcssa34 = phi <4 x i32> [ %i30, %vector.body ]
  br label %middle.block

middle.block:                                     ; preds = %middle.block.unr-lcssa, %vector.ph.split
  %.lcssa33 = phi <4 x i32> [ %broadcast.splat26, %vector.ph.split ], [ %.lcssa35, %middle.block.unr-lcssa ]
  %.lcssa = phi <4 x i32> [ %.lcssa.unr, %vector.ph.split ], [ %.lcssa34, %middle.block.unr-lcssa ]
  %bin.rdx = add <4 x i32> %.lcssa33, %.lcssa
  %rdx.shuf = shufflevector <4 x i32> %bin.rdx, <4 x i32> undef, <4 x i32> <i32 2, i32 3, i32 undef, i32 undef>
  %bin.rdx29 = add <4 x i32> %bin.rdx, %rdx.shuf
  %rdx.shuf30 = shufflevector <4 x i32> %bin.rdx29, <4 x i32> undef, <4 x i32> <i32 1, i32 undef, i32 undef, i32 undef>
  %bin.rdx31 = add <4 x i32> %bin.rdx29, %rdx.shuf30
  %i33 = extractelement <4 x i32> %bin.rdx31, i32 0
  %cmp.n = icmp eq i64 %i5, %n.vec
  br i1 %cmp.n, label %for.end, label %for.body.preheader32

for.body.preheader32:                             ; preds = %middle.block, %min.iters.checked, %for.body.preheader
  %indvars.iv.ph = phi i64 [ 0, %min.iters.checked ], [ 0, %for.body.preheader ], [ %n.vec, %middle.block ]
  %sum.017.ph = phi i32 [ undef, %min.iters.checked ], [ undef, %for.body.preheader ], [ %i33, %middle.block ]
  br label %for.body

for.body:                                         ; preds = %for.body, %for.body.preheader32
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ %indvars.iv.ph, %for.body.preheader32 ]
  %sum.017 = phi i32 [ %add, %for.body ], [ %sum.017.ph, %for.body.preheader32 ]
  %arrayidx5 = getelementptr inbounds i32, ptr %fooPtr, i64 %indvars.iv
  %i34 = trunc i64 %indvars.iv to i32
  store i32 %i34, ptr %arrayidx5, align 4, !tbaa !5
  %add = add nsw i32 %i2, %sum.017
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %aconst
  br i1 %exitcond, label %for.end.loopexit, label %for.body, !llvm.loop !10

for.end.loopexit:                                 ; preds = %for.body
  %add.lcssa = phi i32 [ %add, %for.body ]
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %middle.block, %if.end
  %sum.0.lcssa = phi i32 [ undef, %if.end ], [ %i33, %middle.block ], [ %add.lcssa, %for.end.loopexit ]
  ret i32 %sum.0.lcssa
}

declare ptr @bar(...) #1

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 (trunk 1453)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"any pointer", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
!5 = !{!6, !6, i64 0}
!6 = !{!"int", !3, i64 0}
!7 = distinct !{!7, !8, !9}
!8 = !{!"llvm.loop.vectorize.width", i32 1}
!9 = !{!"llvm.loop.interleave.count", i32 1}
!10 = distinct !{!10, !11, !8, !9}
!11 = !{!"llvm.loop.unroll.runtime.disable"}
