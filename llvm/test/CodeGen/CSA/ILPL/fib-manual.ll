; RUN: llc -mtriple=csa -csa-ilpl-selection=manual < %s | FileCheck %s --check-prefix=CHECK
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

; Function Attrs: nounwind
define void @csa_fib(i64* noalias nocapture %results, i64* noalias nocapture readonly %inputs, i64 %size) local_unnamed_addr #0 {
; Ensure there's a pickany, a token being initialized with multiple values, and
; at least one pick64 driven by the pickany result. Here, we also ensure that
; the number of token values matches exactly the depth specified by the
; programmer. (7.)
; CHECK-DAG:  pickany0 %ign, [[CTRL:%.+]], [[OUTER:%.+]], [[INNER:%.+]]
; CHECK-DAG:  .curr [[TOKEN:%.+]]; .value 0; .avail 0
; CHECK:      .curr [[TOKEN]]; .value 0; .avail
; CHECK-NEXT: .curr [[TOKEN]]; .value 0; .avail
; CHECK-NEXT: .curr [[TOKEN]]; .value 0; .avail
; CHECK-NEXT: .curr [[TOKEN]]; .value 0; .avail
; CHECK-NEXT: .curr [[TOKEN]]; .value 0; .avail
; CHECK-NEXT: .curr [[TOKEN]]; .value 0; .avail
; CHECK-NOT:  .curr [[TOKEN]]; .value 0; .avail
; CHECK-DAG:  pick64 [[INNERVAL:%.+]], [[CTRL]], [[V1:%.+]], [[V2:%.+]]
entry:
  %clie_pre = tail call i32 @llvm.csa.parallel.region.entry(i32 1023)
  %cmp12 = icmp sgt i64 %size, 0
  br i1 %cmp12, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.cond.cleanup3, %entry
  tail call void @llvm.csa.parallel.region.exit(i32 %clie_pre)
  ret void

for.body:                                         ; preds = %for.body.preheader, %for.cond.cleanup3
  %size.addr.015 = phi i64 [ %dec, %for.cond.cleanup3 ], [ %size, %for.body.preheader ]
  %src.014 = phi i64* [ %incdec.ptr, %for.cond.cleanup3 ], [ %inputs, %for.body.preheader ]
  %dest.013 = phi i64* [ %incdec.ptr6, %for.cond.cleanup3 ], [ %results, %for.body.preheader ]
  %clie_pse = tail call i32 @llvm.csa.parallel.section.entry(i32 %clie_pre)
  %0 = load i64, i64* %src.014, align 8, !tbaa !2
  %ilpl_entry = tail call i32 @llvm.csa.pipeline.loop.entry(i32 7)
  %cmp28 = icmp sgt i64 %0, 0
  br i1 %cmp28, label %for.body4.preheader, label %for.cond.cleanup3

for.body4.preheader:                              ; preds = %for.body
  br label %for.body4

for.cond.cleanup3:                                ; preds = %for.body4, %for.body
  %curr.0.lcssa = phi i64 [ 0, %for.body ], [ %add, %for.body4 ]
  tail call void @llvm.csa.pipeline.loop.exit(i32 %ilpl_entry)
  store i64 %curr.0.lcssa, i64* %dest.013, align 8, !tbaa !2
  tail call void @llvm.csa.parallel.section.exit(i32 %clie_pse)
  %dec = add nsw i64 %size.addr.015, -1
  %incdec.ptr = getelementptr inbounds i64, i64* %src.014, i64 1
  %incdec.ptr6 = getelementptr inbounds i64, i64* %dest.013, i64 1
  %cmp = icmp sgt i64 %size.addr.015, 1
  br i1 %cmp, label %for.body, label %for.cond.cleanup

for.body4:                                        ; preds = %for.body4.preheader, %for.body4
  %i.011 = phi i64 [ %inc, %for.body4 ], [ 0, %for.body4.preheader ]
  %curr.010 = phi i64 [ %add, %for.body4 ], [ 0, %for.body4.preheader ]
  %prev.09 = phi i64 [ %curr.010, %for.body4 ], [ 1, %for.body4.preheader ]
  %add = add nsw i64 %curr.010, %prev.09
  %inc = add nuw nsw i64 %i.011, 1
  %exitcond = icmp eq i64 %inc, %0
  br i1 %exitcond, label %for.cond.cleanup3, label %for.body4
}

; Function Attrs: inaccessiblemem_or_argmemonly nounwind
declare i32 @llvm.csa.pipeline.loop.entry(i32) #1

; Function Attrs: inaccessiblemem_or_argmemonly nounwind
declare void @llvm.csa.pipeline.loop.exit(i32) #1

; Function Attrs: inaccessiblemem_or_argmemonly nounwind
declare i32 @llvm.csa.parallel.region.entry(i32) #1

; Function Attrs: inaccessiblemem_or_argmemonly nounwind
declare void @llvm.csa.parallel.region.exit(i32) #1

; Function Attrs: inaccessiblemem_or_argmemonly nounwind
declare i32 @llvm.csa.parallel.section.entry(i32) #1

; Function Attrs: inaccessiblemem_or_argmemonly nounwind
declare void @llvm.csa.parallel.section.exit(i32) #1

attributes #0 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { inaccessiblemem_or_argmemonly nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 6.0.0 (http://llvm.org/git/clang.git ca0ff8f6a2597cf765cad1835931ca2fe0d1c79e) (http://llvm.org/git/llvm.git 89187e1ea09ade848e2d6e60e96aaf387b1a154d)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"long long", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
