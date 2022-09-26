; RUN: opt %s -mattr=+avx512f,+avx512vl -hir-ssa-deconstruction -hir-temp-cleanup -hir-vec-dir-insert -disable-output -debug-only=parvec-analysis -enable-compress-expand-idiom 2>&1 | FileCheck %s

; Reduced spec2006test450Cpp/450 (false idiom recognition).

; BEGIN REGION { }
;       + DO i1 = 0, zext.i32.i64((-1 + %call)), 1   <DO_MULTI_EXIT_LOOP>  <MAX_TC_EST = 4294967295>  <LEGAL_MAX_TC = 4294967295>
;       |   if ((%1)[-1 * i1 + sext.i32.i64(%call)] != 0)
;       |   {
;       |      if ((%1)[0] != 0)
;       |      {
;       |         goto cleanup.loopexit;
;       |      }
;       |      %primals.026 = %primals.026  +  1;
;       |   }
;       + END LOOP
; END REGION

; CHECK:      [Compress/Expand Idiom] Increment {sb:3}+1 detected: <15>         %primals.026 = %primals.026  +  1;
; CHECK-NEXT: [Compress/Expand Idiom] Disabled for multi-exit loops.
; CHECK-NEXT: Idiom List
; CHECK-NEXT:   No idioms detected.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class.f = type { %"class.f::e" }
%"class.f::e" = type { %class.b }
%class.b = type { i32* }

; Function Attrs: mustprogress uwtable
define dso_local noundef i32 @_ZNK1f1lEi(%class.f* noundef nonnull align 8 dereferenceable(8) %this, i32 noundef %0) local_unnamed_addr #0 align 2 {
entry:
  %m = getelementptr inbounds %class.f, %class.f* %this, i64 0, i32 0, !intel-tbaa !3
  %call = tail call noundef i32 @_ZNK1f1e1kEv(%"class.f::e"* noundef nonnull align 8 dereferenceable(8) %m)
  %tobool.not25 = icmp ne i32 %call, 0
  call void @llvm.assume(i1 %tobool.not25)
  %c.i.i = getelementptr inbounds %class.f, %class.f* %this, i64 0, i32 0, i32 0, i32 0, !intel-tbaa !10
  %1 = load i32*, i32** %c.i.i, align 8, !tbaa !10
  %2 = sext i32 %call to i64
  br label %for.body

for.cond10.preheader:                             ; preds = %for.inc
  %primals.1.lcssa = phi i32 [ %primals.1, %for.inc ]
  %tobool11.not = icmp ne i32 %primals.1.lcssa, 0
  call void @llvm.assume(i1 %tobool11.not)
  br label %cleanup

for.body:                                         ; preds = %entry, %for.inc
  %indvars.iv = phi i64 [ %2, %entry ], [ %indvars.iv.next, %for.inc ]
  %primals.026 = phi i32 [ undef, %entry ], [ %primals.1, %for.inc ]
  %arrayidx.i.i = getelementptr inbounds i32, i32* %1, i64 %indvars.iv
  %3 = load i32, i32* %arrayidx.i.i, align 4, !tbaa !11
  %tobool4.not = icmp eq i32 %3, 0
  br i1 %tobool4.not, label %for.inc, label %if.then

if.then:                                          ; preds = %for.body
  %4 = load i32, i32* %1, align 4, !tbaa !11
  %tobool7.not = icmp eq i32 %4, 0
  br i1 %tobool7.not, label %if.else, label %cleanup.loopexit

if.else:                                          ; preds = %if.then
  %inc = add nsw i32 %primals.026, 1
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.else
  %primals.1 = phi i32 [ %inc, %if.else ], [ %primals.026, %for.body ]
  %indvars.iv.next = add nsw i64 %indvars.iv, -1
  %5 = trunc i64 %indvars.iv.next to i32
  %tobool.not = icmp eq i32 %5, 0
  br i1 %tobool.not, label %for.cond10.preheader, label %for.body, !llvm.loop !13

cleanup.loopexit:                                 ; preds = %if.then
  br label %cleanup

cleanup:                                          ; preds = %cleanup.loopexit, %for.cond10.preheader
  ret i32 undef
}

declare dso_local noundef i32 @_ZNK1f1e1kEv(%"class.f::e"* noundef nonnull align 8 dereferenceable(8)) local_unnamed_addr #1

; Function Attrs: inaccessiblememonly nocallback nofree nosync nounwind willreturn
declare void @llvm.assume(i1 noundef) #2

attributes #0 = { mustprogress uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #2 = { inaccessiblememonly nocallback nofree nosync nounwind willreturn }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2023.0.0 (2023.x.0.YYYYMMDD)"}
!3 = !{!4, !5, i64 0}
!4 = !{!"struct@_ZTS1f", !5, i64 0}
!5 = !{!"struct@_ZTSN1f1eE", !6, i64 0}
!6 = !{!"struct@_ZTS1bIN1f1e1hEE", !7, i64 0}
!7 = !{!"pointer@_ZTSPN1f1e1hE", !8, i64 0}
!8 = !{!"omnipotent char", !9, i64 0}
!9 = !{!"Simple C++ TBAA"}
!10 = !{!4, !7, i64 0}
!11 = !{!12, !12, i64 0}
!12 = !{!"_ZTSN1f1e1hE", !8, i64 0}
!13 = distinct !{!13, !14}
!14 = !{!"llvm.loop.mustprogress"}
