; ModuleID = 'ts.c'
; C source
; #define N 200
; unsigned int e2[N], h[N], ek[N][N], f[N], d[N];
;
; int foo(int jo, int no)
; {
;   int jc;
;
;   for (jc = 100; jc > 0; --jc) {
;     no -= e2[jc];
;
;     if (h[jc] == ek[jc+1][jo+1]) {
;       d[jc] = f[jc-1];
;     }
;   }
;
;   return no;
; }
; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-vec-dir-insert -hir-vplan-vec -print-after=hir-vplan-vec -S  -vplan-force-vf=4 < %s 2>&1 | FileCheck %s

; CHECK:      BEGIN REGION { modified }
; CHECK-NEXT:  %red.init = 0;
; CHECK-NEXT:  %red.init.insert = insertelement %red.init,  %no.addr.022,  0;
; CHECK-NEXT:  %phi.temp = %red.init.insert;
; CHECK:       DO i1 = 0, 99, 4   <DO_LOOP> <auto-vectorized> <novectorize>
; CHECK-NEXT:   %.vec7 = undef;
; CHECK-NEXT:   %.vec = (<4 x i32>*)(@e2)[0][-1 * i1 + 97];
; CHECK-NEXT:   %reverse = shufflevector %.vec,  undef,  <i32 3, i32 2, i32 1, i32 0>;
; CHECK-NEXT:   %.vec1 = %phi.temp  -  %reverse;
; CHECK-NEXT:   %.vec2 = (<4 x i32>*)(@h)[0][-1 * i1 + 97];
; CHECK-NEXT:   %reverse3 = shufflevector %.vec2,  undef,  <i32 3, i32 2, i32 1, i32 0>;
; CHECK-NEXT:   %.vec4 = (<4 x i32>*)(@ek)[0][-1 * i1 + -1 * <i64 0, i64 1, i64 2, i64 3> + 101][%jo + 1];
; CHECK-NEXT:   %.vec5 = %reverse3 == %.vec4;
; CHECK-NEXT:   %reverse6 = shufflevector %.vec5,  undef,  <i32 3, i32 2, i32 1, i32 0>;
; CHECK-NEXT:   %.vec7 = (<4 x i32>*)(@f)[0][-1 * i1 + 96]; Mask = @{%reverse6}
; CHECK-NEXT:   %reverse8 = shufflevector %.vec7,  undef,  <i32 3, i32 2, i32 1, i32 0>;
; CHECK-NEXT:   %reverse9 = shufflevector %.vec5,  undef,  <i32 3, i32 2, i32 1, i32 0>;
; CHECK-NEXT:   %reverse10 = shufflevector %reverse8,  undef,  <i32 3, i32 2, i32 1, i32 0>;
; CHECK-NEXT:   (<4 x i32>*)(@d)[0][-1 * i1 + 97] = %reverse10; Mask = @{%reverse9}
; CHECK-NEXT:   %phi.temp = %.vec1;
; CHECK-NEXT:  END LOOP
; CHECK:       %no.addr.022 = @llvm.vector.reduce.add.v4i32(%.vec1);
; CHECK-NEXT: END REGION
source_filename = "ts.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@e2 = common local_unnamed_addr global [200 x i32] zeroinitializer, align 16
@h = common local_unnamed_addr global [200 x i32] zeroinitializer, align 16
@ek = common local_unnamed_addr global [200 x [200 x i32]] zeroinitializer, align 16
@f = common local_unnamed_addr global [200 x i32] zeroinitializer, align 16
@d = common local_unnamed_addr global [200 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define i32 @foo(i32 %jo, i32 %no) local_unnamed_addr #0 {
entry:
  %add = add nsw i32 %jo, 1
  %idxprom3 = sext i32 %add to i64
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc
  %indvars.iv = phi i64 [ 100, %entry ], [ %indvars.iv.next, %for.inc ]
  %no.addr.022 = phi i32 [ %no, %entry ], [ %sub, %for.inc ]
  %arrayidx = getelementptr inbounds [200 x i32], [200 x i32]* @e2, i64 0, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !1
  %sub = sub i32 %no.addr.022, %0
  %arrayidx2 = getelementptr inbounds [200 x i32], [200 x i32]* @h, i64 0, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx2, align 4, !tbaa !1
  %2 = add nuw nsw i64 %indvars.iv, 1
  %arrayidx7 = getelementptr inbounds [200 x [200 x i32]], [200 x [200 x i32]]* @ek, i64 0, i64 %2, i64 %idxprom3
  %3 = load i32, i32* %arrayidx7, align 4, !tbaa !6
  %cmp8 = icmp eq i32 %1, %3
  br i1 %cmp8, label %if.then, label %for.inc

if.then:                                          ; preds = %for.body
  %4 = add nsw i64 %indvars.iv, -1
  %arrayidx11 = getelementptr inbounds [200 x i32], [200 x i32]* @f, i64 0, i64 %4
  %5 = load i32, i32* %arrayidx11, align 4, !tbaa !1
  %arrayidx13 = getelementptr inbounds [200 x i32], [200 x i32]* @d, i64 0, i64 %indvars.iv
  store i32 %5, i32* %arrayidx13, align 4, !tbaa !1
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then
  %indvars.iv.next = add nsw i64 %indvars.iv, -1
  %cmp = icmp sgt i64 %indvars.iv, 1
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.inc
  ret i32 %sub
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!1 = !{!2, !3, i64 0}
!2 = !{!"array@_ZTSA200_j", !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!7, !3, i64 0}
!7 = !{!"array@_ZTSA200_A200_j", !2, i64 0}
