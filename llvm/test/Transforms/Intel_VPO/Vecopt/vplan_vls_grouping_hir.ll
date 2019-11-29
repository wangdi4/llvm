; This test checks for VLS-grouping from the VPO vectorizer
;
; Generated the IR using the following command:
;icx -O2 vectvls.cpp -S -emit-llvm -mllvm -print-module-before-loopopt -mllvm -disable-hir-loop-reroll=true -xSSE3
;
; Original C/C++ source
;
; int x[300];
; void foo(int N) {
; int i,a,b,c;
; for (i=0;i<300;i+=3){
;   a=x[i];
;   b=x[i+1];
;   c=x[i+2];
;   x[i]=a+1;
;   x[i+1]=b+1;
;   x[i+2]=c+1;
;  }
;}

; REQUIRES: asserts
; RUN: opt < %s -O2 -hir-ssa-deconstruction -hir-vec-dir-insert -VPlanDriverHIR -debug-only=ovls -disable-hir-complete-unroll -enable-vplan-vls-cg=false -vplan-force-vf=8 2>&1 | FileCheck %s

; CHECK:       Printing Groups- Total Groups 2
; CHECK-NEXT:  Group#1
; CHECK-NEXT:    Vector Length(in bytes): 64
; CHECK-NEXT:    AccType: SLoad, Stride (in bytes): 12
; CHECK-NEXT:    AccessMask(per byte, R to L): 111111111111
; CHECK-NEXT:   #1 <8 x 32> SLoad: i32 %{{.*}} = load i32* %{{.*}} | (@x)[0][3 * i1]
; CHECK-NEXT:   #2 <8 x 32> SLoad: i32 %{{.*}} = load i32* %{{.*}} | (@x)[0][3 * i1 + 1]
; CHECK-NEXT:   #3 <8 x 32> SLoad: i32 %{{.*}} = load i32* %{{.*}} | (@x)[0][3 * i1 + 2]
; CHECK-NEXT:  Group#2
; CHECK-NEXT:    Vector Length(in bytes): 64
; CHECK-NEXT:    AccType: SStore, Stride (in bytes): 12
; CHECK-NEXT:    AccessMask(per byte, R to L): 111111111111
; CHECK-NEXT:   #4 <8 x 32> SStore: store i32 %{{.*}} i32* %{{.*}} | (@x)[0][3 * i1]
; CHECK-NEXT:   #5 <8 x 32> SStore: store i32 %{{.*}} i32* %{{.*}} | (@x)[0][3 * i1 + 1]
; CHECK-NEXT:   #6 <8 x 32> SStore: store i32 %{{.*}} i32* %{{.*}} | (@x)[0][3 * i1 + 2]

@x = dso_local local_unnamed_addr global [300 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define dso_local void @_Z3fooi(i32 %N) local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds [300 x i32], [300 x i32]* @x, i64 0, i64 %indvars.iv, !intel-tbaa !2
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !2
  %1 = add nuw nsw i64 %indvars.iv, 1
  %arrayidx2 = getelementptr inbounds [300 x i32], [300 x i32]* @x, i64 0, i64 %1, !intel-tbaa !2
  %2 = load i32, i32* %arrayidx2, align 4, !tbaa !2
  %3 = add nuw nsw i64 %indvars.iv, 2
  %arrayidx5 = getelementptr inbounds [300 x i32], [300 x i32]* @x, i64 0, i64 %3, !intel-tbaa !2
  %4 = load i32, i32* %arrayidx5, align 4, !tbaa !2
  %add6 = add nsw i32 %0, 1
  store i32 %add6, i32* %arrayidx, align 4, !tbaa !2
  %add9 = add nsw i32 %2, 1
  store i32 %add9, i32* %arrayidx2, align 4, !tbaa !2
  %add13 = add nsw i32 %4, 1
  store i32 %add13, i32* %arrayidx5, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 3
  %cmp = icmp ult i64 %indvars.iv.next, 300
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.body
  ret void
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="nocona" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sse,+sse2,+sse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"icx (ICX) dev.8.x.0"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA300_i", !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C++ TBAA"}
