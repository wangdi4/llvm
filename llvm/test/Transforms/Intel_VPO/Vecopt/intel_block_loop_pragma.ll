; RUN: opt -vplan-pragma-omp-ordered-simd-extract -S %s | FileCheck %s

;void foo() {
;  int s, i, j, k;
;  #pragma block_loop factor (16) level (1)
;  #pragma block_loop factor (32) level (2) private(s)
;  for (i=0; i<256; i++) {
;    for (j=0; j<256; j++) {
;      s = 0;
;      for (k=0; k<256; k++) {
;        s += B[i][k] * C[k][j];
;      }
;      A[i][j] += s;
;    }
;  }
;}

; Check that block_loop pragmas do not trigger errors
; CHECK-LABEL: define dso_local void @foo()
; CHECK: {{%.*}} = call token @llvm.directive.region.entry() [ "DIR.PRAGMA.BLOCK_LOOP"(), "QUAL.PRAGMA.LEVEL"(i32 1), "QUAL.PRAGMA.FACTOR"(i32 16) ]
; CHECK: @llvm.directive.region.exit(token %0) [ "DIR.PRAGMA.END.BLOCK_LOOP"() ]

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@C = dso_local local_unnamed_addr global [4096 x [4096 x i32]] zeroinitializer, align 16
@A = dso_local local_unnamed_addr global [4096 x [4096 x i32]] zeroinitializer, align 16
@B = dso_local local_unnamed_addr global [4096 x [4096 x i32]] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define dso_local void @foo() local_unnamed_addr #0 {
entry:
  br label %DIR.PRAGMA.BLOCK_LOOP.1

DIR.PRAGMA.BLOCK_LOOP.1:                          ; preds = %entry
  %0 = call token @llvm.directive.region.entry() [ "DIR.PRAGMA.BLOCK_LOOP"(), "QUAL.PRAGMA.LEVEL"(i32 1), "QUAL.PRAGMA.FACTOR"(i32 16) ]
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc25, %DIR.PRAGMA.BLOCK_LOOP.1
  %indvars.iv48 = phi i64 [ 0, %DIR.PRAGMA.BLOCK_LOOP.1 ], [ %indvars.iv.next49, %for.inc25 ]
  br label %for.cond4.preheader

for.cond4.preheader:                              ; preds = %for.inc22, %for.cond1.preheader
  %indvars.iv45 = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next46, %for.inc22 ]
  %arrayidx8 = getelementptr inbounds [4096 x [4096 x i32]], [4096 x [4096 x i32]]* @C, i64 0, i64 %indvars.iv48, i64 %indvars.iv45, !intel-tbaa !2
  br label %for.body6

for.body6:                                        ; preds = %for.body6, %for.cond4.preheader
  %indvars.iv = phi i64 [ 0, %for.cond4.preheader ], [ %indvars.iv.next, %for.body6 ]
  %1 = load i32, i32* %arrayidx8, align 4, !tbaa !2
  %arrayidx12 = getelementptr inbounds [4096 x [4096 x i32]], [4096 x [4096 x i32]]* @A, i64 0, i64 %indvars.iv48, i64 %indvars.iv, !intel-tbaa !2
  %2 = load i32, i32* %arrayidx12, align 4, !tbaa !2
  %arrayidx16 = getelementptr inbounds [4096 x [4096 x i32]], [4096 x [4096 x i32]]* @B, i64 0, i64 %indvars.iv, i64 %indvars.iv45, !intel-tbaa !2
  %3 = load i32, i32* %arrayidx16, align 4, !tbaa !2
  %mul = mul nsw i32 %3, %2
  %add = add nsw i32 %mul, %1
  %arrayidx20 = getelementptr inbounds [4096 x [4096 x i32]], [4096 x [4096 x i32]]* @C, i64 0, i64 %indvars.iv, i64 %indvars.iv45, !intel-tbaa !2
  %4 = load i32, i32* %arrayidx20, align 4, !tbaa !2
  %add21 = add nsw i32 %add, %4
  store i32 %add21, i32* %arrayidx20, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 4096
  br i1 %exitcond, label %for.inc22, label %for.body6

for.inc22:                                        ; preds = %for.body6
  %indvars.iv.next46 = add nuw nsw i64 %indvars.iv45, 1
  %exitcond47 = icmp eq i64 %indvars.iv.next46, 4096
  br i1 %exitcond47, label %for.inc25, label %for.cond4.preheader

for.inc25:                                        ; preds = %for.inc22
  %indvars.iv.next49 = add nuw nsw i64 %indvars.iv48, 1
  %exitcond50 = icmp eq i64 %indvars.iv.next49, 4096
  br i1 %exitcond50, label %for.end27, label %for.cond1.preheader

for.end27:                                        ; preds = %for.inc25
  call void @llvm.directive.region.exit(token %0) [ "DIR.PRAGMA.END.BLOCK_LOOP"() ]
  br label %DIR.PRAGMA.END.BLOCK_LOOP.2

DIR.PRAGMA.END.BLOCK_LOOP.2:                      ; preds = %for.end27
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!2 = !{!3, !5, i64 0}
!3 = !{!"array@_ZTSA4096_A4096_i", !4, i64 0}
!4 = !{!"array@_ZTSA4096_i", !5, i64 0}
!5 = !{!"int", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
