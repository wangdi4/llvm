; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -enable-new-pm=0 -hir-locality-analysis -hir-temporal-locality | FileCheck %s
; RUN: opt < %s -passes=hir-ssa-deconstruction | opt -passes="print<hir-locality-analysis>" -hir-temporal-locality -disable-output 2>&1 | FileCheck %s

; Verify that there is no temporal lcality in this loop.
; We were incorrectly forming temporal reuse locality groups for refs such as (%pSrc1)[sext.i32.i64(%src1Step) * i1] and (%pSrc1)[sext.i32.i64(%src1Step) * i1 + 1].

; HIR-
; + DO i1 = 0, 15, 1   <DO_LOOP>
; |   %0 = (%pSrc1)[sext.i32.i64(%src1Step) * i1];
; |   %1 = (%pSrc2)[sext.i32.i64(%src2Step) * i1];
; |   (%pDst)[(sext.i32.i64(%dstStep) * i1)/u2] = sext.i8.i16(%0) + sext.i8.i16(%1);
; |   %2 = (%pSrc1)[sext.i32.i64(%src1Step) * i1 + 1];
; |   %3 = (%pSrc2)[sext.i32.i64(%src2Step) * i1 + 1];
; |   (%pDst)[(sext.i32.i64(%dstStep) * i1 + 2)/u2] = sext.i8.i16(%2) + sext.i8.i16(%3);
; |   %4 = (%pSrc1)[sext.i32.i64(%src1Step) * i1 + 2];
; |   %5 = (%pSrc2)[sext.i32.i64(%src2Step) * i1 + 2];
; |   (%pDst)[(sext.i32.i64(%dstStep) * i1 + 4)/u2] = sext.i8.i16(%4) + sext.i8.i16(%5);
; |   %6 = (%pSrc1)[sext.i32.i64(%src1Step) * i1 + 3];
; |   %7 = (%pSrc2)[sext.i32.i64(%src2Step) * i1 + 3];
; |   (%pDst)[(sext.i32.i64(%dstStep) * i1 + 6)/u2] = sext.i8.i16(%6) + sext.i8.i16(%7);
; |   %8 = (%pSrc1)[sext.i32.i64(%src1Step) * i1 + 4];
; |   %9 = (%pSrc2)[sext.i32.i64(%src2Step) * i1 + 4];
; |   (%pDst)[(sext.i32.i64(%dstStep) * i1 + 8)/u2] = sext.i8.i16(%8) + sext.i8.i16(%9);
; |   %10 = (%pSrc1)[sext.i32.i64(%src1Step) * i1 + 5];
; |   %11 = (%pSrc2)[sext.i32.i64(%src2Step) * i1 + 5];
; |   (%pDst)[(sext.i32.i64(%dstStep) * i1 + 10)/u2] = sext.i8.i16(%10) + sext.i8.i16(%11);
; |   %12 = (%pSrc1)[sext.i32.i64(%src1Step) * i1 + 6];
; |   %13 = (%pSrc2)[sext.i32.i64(%src2Step) * i1 + 6];
; |   (%pDst)[(sext.i32.i64(%dstStep) * i1 + 12)/u2] = sext.i8.i16(%12) + sext.i8.i16(%13);
; |   %14 = (%pSrc1)[sext.i32.i64(%src1Step) * i1 + 7];
; |   %15 = (%pSrc2)[sext.i32.i64(%src2Step) * i1 + 7];
; |   (%pDst)[(sext.i32.i64(%dstStep) * i1 + 14)/u2] = sext.i8.i16(%14) + sext.i8.i16(%15);
; |   %16 = (%pSrc1)[sext.i32.i64(%src1Step) * i1 + 8];
; |   %17 = (%pSrc2)[sext.i32.i64(%src2Step) * i1 + 8];
; |   (%pDst)[(sext.i32.i64(%dstStep) * i1 + 16)/u2] = sext.i8.i16(%16) + sext.i8.i16(%17);
; |   %18 = (%pSrc1)[sext.i32.i64(%src1Step) * i1 + 9];
; |   %19 = (%pSrc2)[sext.i32.i64(%src2Step) * i1 + 9];
; |   (%pDst)[(sext.i32.i64(%dstStep) * i1 + 18)/u2] = sext.i8.i16(%18) + sext.i8.i16(%19);
; |   %20 = (%pSrc1)[sext.i32.i64(%src1Step) * i1 + 10];
; |   %21 = (%pSrc2)[sext.i32.i64(%src2Step) * i1 + 10];
; |   (%pDst)[(sext.i32.i64(%dstStep) * i1 + 20)/u2] = sext.i8.i16(%20) + sext.i8.i16(%21);
; |   %22 = (%pSrc1)[sext.i32.i64(%src1Step) * i1 + 11];
; |   %23 = (%pSrc2)[sext.i32.i64(%src2Step) * i1 + 11];
; |   (%pDst)[(sext.i32.i64(%dstStep) * i1 + 22)/u2] = sext.i8.i16(%22) + sext.i8.i16(%23);
; |   %24 = (%pSrc1)[sext.i32.i64(%src1Step) * i1 + 12];
; |   %25 = (%pSrc2)[sext.i32.i64(%src2Step) * i1 + 12];
; |   (%pDst)[(sext.i32.i64(%dstStep) * i1 + 24)/u2] = sext.i8.i16(%24) + sext.i8.i16(%25);
; |   %26 = (%pSrc1)[sext.i32.i64(%src1Step) * i1 + 13];
; |   %27 = (%pSrc2)[sext.i32.i64(%src2Step) * i1 + 13];
; |   (%pDst)[(sext.i32.i64(%dstStep) * i1 + 26)/u2] = sext.i8.i16(%26) + sext.i8.i16(%27);
; |   %28 = (%pSrc1)[sext.i32.i64(%src1Step) * i1 + 14];
; |   %29 = (%pSrc2)[sext.i32.i64(%src2Step) * i1 + 14];
; |   (%pDst)[(sext.i32.i64(%dstStep) * i1 + 28)/u2] = sext.i8.i16(%28) + sext.i8.i16(%29);
; |   %30 = (%pSrc1)[sext.i32.i64(%src1Step) * i1 + 15];
; |   %31 = (%pSrc2)[sext.i32.i64(%src2Step) * i1 + 15];
; |   (%pDst)[(sext.i32.i64(%dstStep) * i1 + 30)/u2] = sext.i8.i16(%30) + sext.i8.i16(%31);
; + END LOOP

; CHECK: TempInv: 0
; CHECK: TempReuse: 0


;Module Before HIR; ModuleID = 'cq153724.c'
source_filename = "cq153724.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define void @cq153724(i8* noalias nocapture readonly %pSrc1, i32 %src1Step, i8* noalias nocapture readonly %pSrc2, i32 %src2Step, i16* noalias nocapture %pDst, i32 %dstStep) local_unnamed_addr #0 {
entry:
  %idx.ext = sext i32 %src1Step to i64
  %idx.ext10 = sext i32 %src2Step to i64
  %idx.ext12 = sext i32 %dstStep to i64
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond1.preheader, %entry
  %i.031 = phi i32 [ 0, %entry ], [ %inc15, %for.cond1.preheader ]
  %pSrc1.addr.030 = phi i8* [ %pSrc1, %entry ], [ %add.ptr, %for.cond1.preheader ]
  %pDst.addr.029 = phi i16* [ %pDst, %entry ], [ %33, %for.cond1.preheader ]
  %pSrc2.addr.028 = phi i8* [ %pSrc2, %entry ], [ %add.ptr11, %for.cond1.preheader ]
  %0 = load i8, i8* %pSrc1.addr.030, align 1, !tbaa !1
  %conv = sext i8 %0 to i16
  %1 = load i8, i8* %pSrc2.addr.028, align 1, !tbaa !1
  %conv6 = sext i8 %1 to i16
  %add = add nsw i16 %conv6, %conv
  store i16 %add, i16* %pDst.addr.029, align 2, !tbaa !4
  %arrayidx.1 = getelementptr inbounds i8, i8* %pSrc1.addr.030, i64 1
  %2 = load i8, i8* %arrayidx.1, align 1, !tbaa !1
  %conv.1 = sext i8 %2 to i16
  %arrayidx5.1 = getelementptr inbounds i8, i8* %pSrc2.addr.028, i64 1
  %3 = load i8, i8* %arrayidx5.1, align 1, !tbaa !1
  %conv6.1 = sext i8 %3 to i16
  %add.1 = add nsw i16 %conv6.1, %conv.1
  %arrayidx9.1 = getelementptr inbounds i16, i16* %pDst.addr.029, i64 1
  store i16 %add.1, i16* %arrayidx9.1, align 2, !tbaa !4
  %arrayidx.2 = getelementptr inbounds i8, i8* %pSrc1.addr.030, i64 2
  %4 = load i8, i8* %arrayidx.2, align 1, !tbaa !1
  %conv.2 = sext i8 %4 to i16
  %arrayidx5.2 = getelementptr inbounds i8, i8* %pSrc2.addr.028, i64 2
  %5 = load i8, i8* %arrayidx5.2, align 1, !tbaa !1
  %conv6.2 = sext i8 %5 to i16
  %add.2 = add nsw i16 %conv6.2, %conv.2
  %arrayidx9.2 = getelementptr inbounds i16, i16* %pDst.addr.029, i64 2
  store i16 %add.2, i16* %arrayidx9.2, align 2, !tbaa !4
  %arrayidx.3 = getelementptr inbounds i8, i8* %pSrc1.addr.030, i64 3
  %6 = load i8, i8* %arrayidx.3, align 1, !tbaa !1
  %conv.3 = sext i8 %6 to i16
  %arrayidx5.3 = getelementptr inbounds i8, i8* %pSrc2.addr.028, i64 3
  %7 = load i8, i8* %arrayidx5.3, align 1, !tbaa !1
  %conv6.3 = sext i8 %7 to i16
  %add.3 = add nsw i16 %conv6.3, %conv.3
  %arrayidx9.3 = getelementptr inbounds i16, i16* %pDst.addr.029, i64 3
  store i16 %add.3, i16* %arrayidx9.3, align 2, !tbaa !4
  %arrayidx.4 = getelementptr inbounds i8, i8* %pSrc1.addr.030, i64 4
  %8 = load i8, i8* %arrayidx.4, align 1, !tbaa !1
  %conv.4 = sext i8 %8 to i16
  %arrayidx5.4 = getelementptr inbounds i8, i8* %pSrc2.addr.028, i64 4
  %9 = load i8, i8* %arrayidx5.4, align 1, !tbaa !1
  %conv6.4 = sext i8 %9 to i16
  %add.4 = add nsw i16 %conv6.4, %conv.4
  %arrayidx9.4 = getelementptr inbounds i16, i16* %pDst.addr.029, i64 4
  store i16 %add.4, i16* %arrayidx9.4, align 2, !tbaa !4
  %arrayidx.5 = getelementptr inbounds i8, i8* %pSrc1.addr.030, i64 5
  %10 = load i8, i8* %arrayidx.5, align 1, !tbaa !1
  %conv.5 = sext i8 %10 to i16
  %arrayidx5.5 = getelementptr inbounds i8, i8* %pSrc2.addr.028, i64 5
  %11 = load i8, i8* %arrayidx5.5, align 1, !tbaa !1
  %conv6.5 = sext i8 %11 to i16
  %add.5 = add nsw i16 %conv6.5, %conv.5
  %arrayidx9.5 = getelementptr inbounds i16, i16* %pDst.addr.029, i64 5
  store i16 %add.5, i16* %arrayidx9.5, align 2, !tbaa !4
  %arrayidx.6 = getelementptr inbounds i8, i8* %pSrc1.addr.030, i64 6
  %12 = load i8, i8* %arrayidx.6, align 1, !tbaa !1
  %conv.6 = sext i8 %12 to i16
  %arrayidx5.6 = getelementptr inbounds i8, i8* %pSrc2.addr.028, i64 6
  %13 = load i8, i8* %arrayidx5.6, align 1, !tbaa !1
  %conv6.6 = sext i8 %13 to i16
  %add.6 = add nsw i16 %conv6.6, %conv.6
  %arrayidx9.6 = getelementptr inbounds i16, i16* %pDst.addr.029, i64 6
  store i16 %add.6, i16* %arrayidx9.6, align 2, !tbaa !4
  %arrayidx.7 = getelementptr inbounds i8, i8* %pSrc1.addr.030, i64 7
  %14 = load i8, i8* %arrayidx.7, align 1, !tbaa !1
  %conv.7 = sext i8 %14 to i16
  %arrayidx5.7 = getelementptr inbounds i8, i8* %pSrc2.addr.028, i64 7
  %15 = load i8, i8* %arrayidx5.7, align 1, !tbaa !1
  %conv6.7 = sext i8 %15 to i16
  %add.7 = add nsw i16 %conv6.7, %conv.7
  %arrayidx9.7 = getelementptr inbounds i16, i16* %pDst.addr.029, i64 7
  store i16 %add.7, i16* %arrayidx9.7, align 2, !tbaa !4
  %arrayidx.8 = getelementptr inbounds i8, i8* %pSrc1.addr.030, i64 8
  %16 = load i8, i8* %arrayidx.8, align 1, !tbaa !1
  %conv.8 = sext i8 %16 to i16
  %arrayidx5.8 = getelementptr inbounds i8, i8* %pSrc2.addr.028, i64 8
  %17 = load i8, i8* %arrayidx5.8, align 1, !tbaa !1
  %conv6.8 = sext i8 %17 to i16
  %add.8 = add nsw i16 %conv6.8, %conv.8
  %arrayidx9.8 = getelementptr inbounds i16, i16* %pDst.addr.029, i64 8
  store i16 %add.8, i16* %arrayidx9.8, align 2, !tbaa !4
  %arrayidx.9 = getelementptr inbounds i8, i8* %pSrc1.addr.030, i64 9
  %18 = load i8, i8* %arrayidx.9, align 1, !tbaa !1
  %conv.9 = sext i8 %18 to i16
  %arrayidx5.9 = getelementptr inbounds i8, i8* %pSrc2.addr.028, i64 9
  %19 = load i8, i8* %arrayidx5.9, align 1, !tbaa !1
  %conv6.9 = sext i8 %19 to i16
  %add.9 = add nsw i16 %conv6.9, %conv.9
  %arrayidx9.9 = getelementptr inbounds i16, i16* %pDst.addr.029, i64 9
  store i16 %add.9, i16* %arrayidx9.9, align 2, !tbaa !4
  %arrayidx.10 = getelementptr inbounds i8, i8* %pSrc1.addr.030, i64 10
  %20 = load i8, i8* %arrayidx.10, align 1, !tbaa !1
  %conv.10 = sext i8 %20 to i16
  %arrayidx5.10 = getelementptr inbounds i8, i8* %pSrc2.addr.028, i64 10
  %21 = load i8, i8* %arrayidx5.10, align 1, !tbaa !1
  %conv6.10 = sext i8 %21 to i16
  %add.10 = add nsw i16 %conv6.10, %conv.10
  %arrayidx9.10 = getelementptr inbounds i16, i16* %pDst.addr.029, i64 10
  store i16 %add.10, i16* %arrayidx9.10, align 2, !tbaa !4
  %arrayidx.11 = getelementptr inbounds i8, i8* %pSrc1.addr.030, i64 11
  %22 = load i8, i8* %arrayidx.11, align 1, !tbaa !1
  %conv.11 = sext i8 %22 to i16
  %arrayidx5.11 = getelementptr inbounds i8, i8* %pSrc2.addr.028, i64 11
  %23 = load i8, i8* %arrayidx5.11, align 1, !tbaa !1
  %conv6.11 = sext i8 %23 to i16
  %add.11 = add nsw i16 %conv6.11, %conv.11
  %arrayidx9.11 = getelementptr inbounds i16, i16* %pDst.addr.029, i64 11
  store i16 %add.11, i16* %arrayidx9.11, align 2, !tbaa !4
  %arrayidx.12 = getelementptr inbounds i8, i8* %pSrc1.addr.030, i64 12
  %24 = load i8, i8* %arrayidx.12, align 1, !tbaa !1
  %conv.12 = sext i8 %24 to i16
  %arrayidx5.12 = getelementptr inbounds i8, i8* %pSrc2.addr.028, i64 12
  %25 = load i8, i8* %arrayidx5.12, align 1, !tbaa !1
  %conv6.12 = sext i8 %25 to i16
  %add.12 = add nsw i16 %conv6.12, %conv.12
  %arrayidx9.12 = getelementptr inbounds i16, i16* %pDst.addr.029, i64 12
  store i16 %add.12, i16* %arrayidx9.12, align 2, !tbaa !4
  %arrayidx.13 = getelementptr inbounds i8, i8* %pSrc1.addr.030, i64 13
  %26 = load i8, i8* %arrayidx.13, align 1, !tbaa !1
  %conv.13 = sext i8 %26 to i16
  %arrayidx5.13 = getelementptr inbounds i8, i8* %pSrc2.addr.028, i64 13
  %27 = load i8, i8* %arrayidx5.13, align 1, !tbaa !1
  %conv6.13 = sext i8 %27 to i16
  %add.13 = add nsw i16 %conv6.13, %conv.13
  %arrayidx9.13 = getelementptr inbounds i16, i16* %pDst.addr.029, i64 13
  store i16 %add.13, i16* %arrayidx9.13, align 2, !tbaa !4
  %arrayidx.14 = getelementptr inbounds i8, i8* %pSrc1.addr.030, i64 14
  %28 = load i8, i8* %arrayidx.14, align 1, !tbaa !1
  %conv.14 = sext i8 %28 to i16
  %arrayidx5.14 = getelementptr inbounds i8, i8* %pSrc2.addr.028, i64 14
  %29 = load i8, i8* %arrayidx5.14, align 1, !tbaa !1
  %conv6.14 = sext i8 %29 to i16
  %add.14 = add nsw i16 %conv6.14, %conv.14
  %arrayidx9.14 = getelementptr inbounds i16, i16* %pDst.addr.029, i64 14
  store i16 %add.14, i16* %arrayidx9.14, align 2, !tbaa !4
  %arrayidx.15 = getelementptr inbounds i8, i8* %pSrc1.addr.030, i64 15
  %30 = load i8, i8* %arrayidx.15, align 1, !tbaa !1
  %conv.15 = sext i8 %30 to i16
  %arrayidx5.15 = getelementptr inbounds i8, i8* %pSrc2.addr.028, i64 15
  %31 = load i8, i8* %arrayidx5.15, align 1, !tbaa !1
  %conv6.15 = sext i8 %31 to i16
  %add.15 = add nsw i16 %conv6.15, %conv.15
  %arrayidx9.15 = getelementptr inbounds i16, i16* %pDst.addr.029, i64 15
  store i16 %add.15, i16* %arrayidx9.15, align 2, !tbaa !4
  %add.ptr = getelementptr inbounds i8, i8* %pSrc1.addr.030, i64 %idx.ext
  %add.ptr11 = getelementptr inbounds i8, i8* %pSrc2.addr.028, i64 %idx.ext10
  %32 = bitcast i16* %pDst.addr.029 to i8*
  %add.ptr13 = getelementptr inbounds i8, i8* %32, i64 %idx.ext12
  %33 = bitcast i8* %add.ptr13 to i16*
  %inc15 = add nuw nsw i32 %i.031, 1
  %exitcond = icmp eq i32 %inc15, 16
  br i1 %exitcond, label %for.end16, label %for.cond1.preheader

for.end16:                                        ; preds = %for.cond1.preheader
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (trunk 20746) (llvm/branches/loopopt 20773)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"omnipotent char", !3, i64 0}
!3 = !{!"Simple C/C++ TBAA"}
!4 = !{!5, !5, i64 0}
!5 = !{!"short", !2, i64 0}
