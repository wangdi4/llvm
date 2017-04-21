; REQUIRES: asserts
; RUN: opt -anders-aa  -hir-ssa-deconstruction  -hir-temp-cleanup -debug  -hir-loop-interchange  < %s 2>&1 | FileCheck %s
; CHECK:  Interchanged:

;Module Before HIR; ModuleID = 'fft_interchange.c'
source_filename = "fft_interchange.c"
target datalayout = "e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-unknown-linux-gnu"

; Function Attrs: nounwind
define void @FFT_transform_internal(i32 %N, double* noalias nocapture %data, i32 %direction, double* nocapture readonly %twp) local_unnamed_addr #0 {
entry:
  %div = sdiv i32 %N, 2
  %0 = and i32 %N, -2
  %1 = icmp eq i32 %0, 2
  br i1 %1, label %cleanup, label %if.end

if.end:                                           ; preds = %entry
  %call = tail call i32 @int_log2(i32 %div) #3
  %cmp1 = icmp ne i32 %N, 0
  %cmp4158 = icmp sgt i32 %call, 0
  %or.cond = and i1 %cmp1, %cmp4158
  br i1 %or.cond, label %for.cond5.preheader.lr.ph, label %cleanup

for.cond5.preheader.lr.ph:                        ; preds = %if.end
  %cmp6150 = icmp sgt i32 %div, 0
  br label %for.cond5.preheader

for.cond5.preheader:                              ; preds = %for.end68, %for.cond5.preheader.lr.ph
  %twp.addr.0162 = phi double* [ %twp, %for.cond5.preheader.lr.ph ], [ %twp.addr.1.lcssa, %for.end68 ]
  %bit.0161 = phi i32 [ 0, %for.cond5.preheader.lr.ph ], [ %inc70, %for.end68 ]
  %dual.0159 = phi i32 [ 1, %for.cond5.preheader.lr.ph ], [ %mul64, %for.end68 ]
  br i1 %cmp6150, label %for.body7.lr.ph, label %for.cond25.preheader

for.body7.lr.ph:                                  ; preds = %for.cond5.preheader
  %mul23 = shl i32 %dual.0159, 1
  br label %for.body7

for.cond25.preheader.loopexit:                    ; preds = %for.body7
  br label %for.cond25.preheader

for.cond25.preheader:                             ; preds = %for.cond25.preheader.loopexit, %for.cond5.preheader
  %cmp26154 = icmp sgt i32 %dual.0159, 1
  %mul64 = shl i32 %dual.0159, 1
  br i1 %cmp26154, label %for.body27.lr.ph, label %for.end68

for.body27.lr.ph:                                 ; preds = %for.cond25.preheader
  %scevgep = getelementptr double, double* %twp.addr.0162, i32 -2
  br label %for.body27

for.body7:                                        ; preds = %for.body7.lr.ph, %for.body7
  %b.0151 = phi i32 [ 0, %for.body7.lr.ph ], [ %add24, %for.body7 ]
  %mul = shl nsw i32 %b.0151, 1
  %add = add nsw i32 %b.0151, %dual.0159
  %mul8 = shl nsw i32 %add, 1
  %arrayidx = getelementptr inbounds double, double* %data, i32 %mul8
  %2 = load double, double* %arrayidx, align 4, !tbaa !1
  %add9 = or i32 %mul8, 1
  %arrayidx10 = getelementptr inbounds double, double* %data, i32 %add9
  %3 = load double, double* %arrayidx10, align 4, !tbaa !1
  %arrayidx11 = getelementptr inbounds double, double* %data, i32 %mul
  %4 = load double, double* %arrayidx11, align 4, !tbaa !1
  %sub = fsub double %4, %2
  store double %sub, double* %arrayidx, align 4, !tbaa !1
  %add13 = or i32 %mul, 1
  %arrayidx14 = getelementptr inbounds double, double* %data, i32 %add13
  %5 = load double, double* %arrayidx14, align 4, !tbaa !1
  %sub15 = fsub double %5, %3
  store double %sub15, double* %arrayidx10, align 4, !tbaa !1
  %6 = load double, double* %arrayidx11, align 4, !tbaa !1
  %add19 = fadd double %2, %6
  store double %add19, double* %arrayidx11, align 4, !tbaa !1
  %7 = load double, double* %arrayidx14, align 4, !tbaa !1
  %add22 = fadd double %3, %7
  store double %add22, double* %arrayidx14, align 4, !tbaa !1
  %add24 = add nsw i32 %b.0151, %mul23
  %cmp6 = icmp slt i32 %add24, %div
  br i1 %cmp6, label %for.body7, label %for.cond25.preheader.loopexit

for.body27:                                       ; preds = %for.inc67, %for.body27.lr.ph
  %twp.addr.1157 = phi double* [ %twp.addr.0162, %for.body27.lr.ph ], [ %incdec.ptr28, %for.inc67 ]
  %a.0155 = phi i32 [ 1, %for.body27.lr.ph ], [ %inc, %for.inc67 ]
  %incdec.ptr = getelementptr inbounds double, double* %twp.addr.1157, i32 1
  %8 = load double, double* %twp.addr.1157, align 4, !tbaa !1
  %incdec.ptr28 = getelementptr inbounds double, double* %twp.addr.1157, i32 2
  %9 = load double, double* %incdec.ptr, align 4, !tbaa !1
  br i1 %cmp6150, label %for.body31.preheader, label %for.inc67

for.body31.preheader:                             ; preds = %for.body27
  br label %for.body31

for.body31:                                       ; preds = %for.body31.preheader, %for.body31
  %b.1153 = phi i32 [ %add65, %for.body31 ], [ 0, %for.body31.preheader ]
  %add33 = add nsw i32 %b.1153, %a.0155
  %mul34 = shl nsw i32 %add33, 1
  %add37 = add nsw i32 %add33, %dual.0159
  %mul38 = shl nsw i32 %add37, 1
  %arrayidx39 = getelementptr inbounds double, double* %data, i32 %mul38
  %10 = load double, double* %arrayidx39, align 4, !tbaa !1
  %add40 = or i32 %mul38, 1
  %arrayidx41 = getelementptr inbounds double, double* %data, i32 %add40
  %11 = load double, double* %arrayidx41, align 4, !tbaa !1
  %mul43 = fmul double %8, %10
  %mul44 = fmul double %9, %11
  %sub45 = fsub double %mul43, %mul44
  %mul47 = fmul double %8, %11
  %mul48 = fmul double %9, %10
  %add49 = fadd double %mul48, %mul47
  %arrayidx50 = getelementptr inbounds double, double* %data, i32 %mul34
  %12 = load double, double* %arrayidx50, align 4, !tbaa !1
  %sub51 = fsub double %12, %sub45
  store double %sub51, double* %arrayidx39, align 4, !tbaa !1
  %add53 = or i32 %mul34, 1
  %arrayidx54 = getelementptr inbounds double, double* %data, i32 %add53
  %13 = load double, double* %arrayidx54, align 4, !tbaa !1
  %sub55 = fsub double %13, %add49
  store double %sub55, double* %arrayidx41, align 4, !tbaa !1
  %14 = load double, double* %arrayidx50, align 4, !tbaa !1
  %add59 = fadd double %sub45, %14
  store double %add59, double* %arrayidx50, align 4, !tbaa !1
  %15 = load double, double* %arrayidx54, align 4, !tbaa !1
  %add62 = fadd double %add49, %15
  store double %add62, double* %arrayidx54, align 4, !tbaa !1
  %add65 = add nsw i32 %b.1153, %mul64
  %cmp30 = icmp slt i32 %add65, %div
  br i1 %cmp30, label %for.body31, label %for.inc67.loopexit

for.inc67.loopexit:                               ; preds = %for.body31
  br label %for.inc67

for.inc67:                                        ; preds = %for.inc67.loopexit, %for.body27
  %inc = add nuw nsw i32 %a.0155, 1
  %exitcond = icmp eq i32 %inc, %dual.0159
  br i1 %exitcond, label %for.end68.loopexit, label %for.body27

for.end68.loopexit:                               ; preds = %for.inc67
  %scevgep163 = getelementptr double, double* %scevgep, i32 %mul64
  br label %for.end68

for.end68:                                        ; preds = %for.cond25.preheader, %for.end68.loopexit
  %twp.addr.1.lcssa = phi double* [ %scevgep163, %for.end68.loopexit ], [ %twp.addr.0162, %for.cond25.preheader ]
  %inc70 = add nuw nsw i32 %bit.0161, 1
  %exitcond164 = icmp eq i32 %inc70, %call
  br i1 %exitcond164, label %cleanup.loopexit, label %for.cond5.preheader

cleanup.loopexit:                                 ; preds = %for.end68
  br label %cleanup

cleanup:                                          ; preds = %cleanup.loopexit, %if.end, %entry
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

declare i32 @int_log2(i32) local_unnamed_addr #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #0 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="pentium4" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="pentium4" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (trunk 20662) (llvm/branches/loopopt 20673)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"double", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}

