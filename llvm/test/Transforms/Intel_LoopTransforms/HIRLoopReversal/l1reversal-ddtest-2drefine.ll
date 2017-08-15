; l2reversal-ddtest-1d.ll
; (2-level loop, sanity testcase, for 2D array dependence tests with DV refinements)
;
; All loops fail to reverse; 
; 
; [REASONS]
; - Applicalbe: NO, HAS NO valid  negative memory-access addresses, and 0 positive memory-access address; 
; - Profitable: NO
;   has no valid negative memory-address strides;
; - Legal:      Confusing
;   The Dependence Analyzer can't properly figure out many cases. Thus it takes the conservative approach and
;   marks all (*) though the case could be more refined.
;

; *** Source Code ***
;
;[BEFORE LOOP REVERSAL]
;
;//testing for refine DV over data dependence in 2D array
;int foo(int **restrict A, int **restrict B) {
;  int i = 0, j = 0;
;  //Loop0: (*,*):
;  for (i = 1; i <= 10; i++) {
;    for (j = 1; j <= 10; j++) {
;      A[i][j] = A[i + R(i)][j + (j % 3 - 1)];
;    }
;  }
;
;  //Loop1: (<=,*):
;  for (i = 1; i <= 10; i++) {
;    for (j = 1; j <= 10; j++) {
;      A[i][j] = A[i - i % 2][j + (j % 3 - 1)];
;    }
;  }
;
;  //Loop2: (>=,*):
;  for (i = 1; i <= 10; i++) {
;    for (j = 1; j <= 10; j++) {
;      A[i][j] = A[i + i % 2][j + (j % 3 - 1)];
;    }
;  }
;
;  //Loop3: (<>,*):
;  const int ConstT = -1;
;  int T = 1;
;  for (i = 1; i <= 10; i++) {
;    for (j = 1; j <= 10; j++) {
;      A[i][j] = A[i + T * ConstT][j + (j % 3 - 1)];
;    }
;  }
;
;  return A[1][1] + B[1][1] + 1;
;}
;
;[AFTER LOOP REVERSAL]{Expected!}
;
; 
; ===-----------------------------------===
; *** Run0: BEFORE HIR Loop Reversal ***
; ===-----------------------------------===
; RUN: opt -hir-ssa-deconstruction -hir-loop-reversal -print-before=hir-loop-reversal -S 2>&1 \
; RUN: < %s  |	FileCheck %s -check-prefix=BEFORE 
;
; ===-----------------------------------===
; *** Run1: AFTER HIR Loop Reversal ***
; ===-----------------------------------===
; RUN: opt -hir-ssa-deconstruction -hir-loop-reversal -print-after=hir-loop-reversal -S 2>&1 \ 
; RUN: < %s  |	FileCheck %s -check-prefix=AFTER
;
;
; === -------------------------------------- ===
; *** Tests0: W/O HIR Loop Reversal Output ***
; === -------------------------------------- ===
; Expected output before Loop Reversal
;
; Loop0: (*,*)
;          BEGIN REGION { }
;<88>         + DO i1 = 0, 9, 1   <DO_LOOP>
;<3>          |   %rem = i1 + 1  %  3;
;<10>         |   %3 = (%A)[trunc.i64.i32(%indvars.iv175) + %rem + -1];
;<12>         |   %4 = (%3)[i1 + %rem];
;<14>         |   (%0)[i1 + 1] = %4;
;<88>         + END LOOP
;          END REGION
;
; Loop1: (<=, *)
;          BEGIN REGION { }
;<89>         + DO i1 = 0, 9, 1   <DO_LOOP>
;<24>         |   %rem22 = i1 + 1  %  3;
;<30>         |   %11 = (%6)[i1 + %rem22];
;<32>         |   (%7)[i1 + 1] = %11;
;<89>         + END LOOP
;          END REGION
;
; Loop2: (>=, *)
;          BEGIN REGION { }
;<90>         + DO i1 = 0, 9, 1   <DO_LOOP>
;<42>         |   %rem47 = i1 + 1  %  3;
;<48>         |   %18 = (%13)[i1 + %rem47];
;<50>         |   (%14)[i1 + 1] = %18;
;<90>         + END LOOP
;          END REGION
;
; Loop3: (<>, *)
;          BEGIN REGION { }
;<91>         + DO i1 = 0, 9, 1   <DO_LOOP>
;<60>         |   %20 = (%A)[i1 + 1];
;<92>         |   + DO i2 = 0, 9, 1   <DO_LOOP>
;<65>         |   |   %rem72 = i2 + 1  %  3;
;<71>         |   |   %24 = (%19)[i2 + %rem72];
;<73>         |   |   (%20)[i2 + 1] = %24;
;<92>         |   + END LOOP
;<83>         |   %19 = &((%20)[0]);
;<91>         + END LOOP
;          END REGION
;
;
;***
;***
;***
;
; Loop0: (*,*)
;          BEGIN REGION { }
; BEFORE:         + DO i1 = 0, 9, 1   <DO_LOOP>
; BEFORE:      |   %rem = i1 + 1  %  3;
; BEFORE:     |   %3 = (%A)[trunc.i64.i32(%indvars.iv175) + %rem + -1];
; BEFORE:     |   %4 = (%3)[i1 + %rem];
; BEFORE:     |   (%0)[i1 + 1] = %4;
; BEFORE:     + END LOOP
; BEFORE:  END REGION
;
; Loop1: (<=, *)
; BEFORE:  BEGIN REGION { }
; BEFORE:     + DO i1 = 0, 9, 1   <DO_LOOP>
; BEFORE:     |   %rem22 = i1 + 1  %  3;
; BEFORE:     |   %11 = (%6)[i1 + %rem22];
; BEFORE:     |   (%7)[i1 + 1] = %11;
; BEFORE:     + END LOOP
; BEFORE:  END REGION
;
; Loop2: (>=, *)
; BEFORE:  BEGIN REGION { }
; BEFORE:     + DO i1 = 0, 9, 1   <DO_LOOP>
; BEFORE:     |   %rem47 = i1 + 1  %  3;
; BEFORE:     |   %18 = (%13)[i1 + %rem47];
; BEFORE:     |   (%14)[i1 + 1] = %18;
; BEFORE:     + END LOOP
; BEFORE:  END REGION
;
; Loop3: (<>, *)
; BEFORE:  BEGIN REGION { }
; BEFORE:     + DO i1 = 0, 9, 1   <DO_LOOP>
; BEFORE:     |   %20 = (%A)[i1 + 1];
; BEFORE:     |   + DO i2 = 0, 9, 1   <DO_LOOP>
; BEFORE:     |   |   %rem72 = i2 + 1  %  3;
; BEFORE:     |   |   %24 = (%19)[i2 + %rem72];
; BEFORE:     |   |   (%20)[i2 + 1] = %24;
; BEFORE:     |   + END LOOP
; BEFORE:     |   %19 = &((%20)[0]);
; BEFORE:     + END LOOP
; BEFORE:  END REGION
;
;
; === -------------------------------------- ===
; *** Tests1: With HIR Loop Reversal Output ***
; === -------------------------------------- ===
;
; Expected HIR output after Loop-Reversal is enabled: Not reversal ever happened in this given input!
; 
;
;          BEGIN REGION { }
;<88>         + DO i1 = 0, 9, 1   <DO_LOOP>
;<3>          |   %rem = i1 + 1  %  3;
;<10>         |   %3 = (%A)[trunc.i64.i32(%indvars.iv175) + %rem + -1];
;<12>         |   %4 = (%3)[i1 + %rem];
;<14>         |   (%0)[i1 + 1] = %4;
;<88>         + END LOOP
;          END REGION
;
;          BEGIN REGION { }
;<89>         + DO i1 = 0, 9, 1   <DO_LOOP>
;<24>         |   %rem22 = i1 + 1  %  3;
;<30>         |   %11 = (%6)[i1 + %rem22];
;<32>         |   (%7)[i1 + 1] = %11;
;<89>         + END LOOP
;          END REGION
;
;          BEGIN REGION { }
;<90>         + DO i1 = 0, 9, 1   <DO_LOOP>
;<42>         |   %rem47 = i1 + 1  %  3;
;<48>         |   %18 = (%13)[i1 + %rem47];
;<50>         |   (%14)[i1 + 1] = %18;
;<90>         + END LOOP
;          END REGION
;
;          BEGIN REGION { }
;<91>         + DO i1 = 0, 9, 1   <DO_LOOP>
;<60>         |   %20 = (%A)[i1 + 1];
;<92>         |   + DO i2 = 0, 9, 1   <DO_LOOP>
;<65>         |   |   %rem72 = i2 + 1  %  3;
;<71>         |   |   %24 = (%19)[i2 + %rem72];
;<73>         |   |   (%20)[i2 + 1] = %24;
;<92>         |   + END LOOP
;<83>         |   %19 = &((%20)[0]);
;<91>         + END LOOP
;          END REGION
;
;
;***
;***
;***
;
; Loop0: (*,*)
;          BEGIN REGION { }
; AFTER:         + DO i1 = 0, 9, 1   <DO_LOOP>
; AFTER:      |   %rem = i1 + 1  %  3;
; AFTER:     |   %3 = (%A)[trunc.i64.i32(%indvars.iv175) + %rem + -1];
; AFTER:     |   %4 = (%3)[i1 + %rem];
; AFTER:     |   (%0)[i1 + 1] = %4;
; AFTER:     + END LOOP
; AFTER:  END REGION
;
; Loop1: (<=, *)
; AFTER:  BEGIN REGION { }
; AFTER:     + DO i1 = 0, 9, 1   <DO_LOOP>
; AFTER:     |   %rem22 = i1 + 1  %  3;
; AFTER:     |   %11 = (%6)[i1 + %rem22];
; AFTER:     |   (%7)[i1 + 1] = %11;
; AFTER:     + END LOOP
; AFTER:  END REGION
;
; Loop2: (>=, *)
; AFTER:  BEGIN REGION { }
; AFTER:     + DO i1 = 0, 9, 1   <DO_LOOP>
; AFTER:     |   %rem47 = i1 + 1  %  3;
; AFTER:     |   %18 = (%13)[i1 + %rem47];
; AFTER:     |   (%14)[i1 + 1] = %18;
; AFTER:     + END LOOP
; AFTER:  END REGION
;
; Loop3: (<>, *)
; AFTER:  BEGIN REGION { }
; AFTER:     + DO i1 = 0, 9, 1   <DO_LOOP>
; AFTER:     |   %20 = (%A)[i1 + 1];
; AFTER:     |   + DO i2 = 0, 9, 1   <DO_LOOP>
; AFTER:     |   |   %rem72 = i2 + 1  %  3;
; AFTER:     |   |   %24 = (%19)[i2 + %rem72];
; AFTER:     |   |   (%20)[i2 + 1] = %24;
; AFTER:     |   + END LOOP
; AFTER:     |   %19 = &((%20)[0]);
; AFTER:     + END LOOP
; AFTER:  END REGION
; 
;
; === ---------------------------------------------------------------- ===
; Following is the LLVM's input code!
; === ---------------------------------------------------------------- ===
;
source_filename = "t.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define i32 @foo(i32** noalias nocapture readonly %A, i32** noalias nocapture readonly %B) #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc13, %entry
  %indvars.iv175 = phi i64 [ 1, %entry ], [ %indvars.iv.next176, %for.inc13 ]
  %arrayidx11 = getelementptr inbounds i32*, i32** %A, i64 %indvars.iv175
  %0 = load i32*, i32** %arrayidx11, align 8, !tbaa !1
  %1 = trunc i64 %indvars.iv175 to i32
  br label %for.body3

for.body3:                                        ; preds = %for.body3, %for.cond1.preheader
  %indvars.iv172 = phi i64 [ 1, %for.cond1.preheader ], [ %indvars.iv.next173, %for.body3 ]
  %2 = trunc i64 %indvars.iv172 to i32
  %rem = srem i32 %2, 3
  %sub = add nsw i32 %rem, -1
  %add = add nsw i32 %sub, %2
  %idxprom = sext i32 %add to i64
  %add6 = add nsw i32 %sub, %1
  %idxprom7 = sext i32 %add6 to i64
  %arrayidx = getelementptr inbounds i32*, i32** %A, i64 %idxprom7
  %3 = load i32*, i32** %arrayidx, align 8, !tbaa !1
  %arrayidx8 = getelementptr inbounds i32, i32* %3, i64 %idxprom
  %4 = load i32, i32* %arrayidx8, align 4, !tbaa !5
  %arrayidx12 = getelementptr inbounds i32, i32* %0, i64 %indvars.iv172
  store i32 %4, i32* %arrayidx12, align 4, !tbaa !5
  %indvars.iv.next173 = add nuw nsw i64 %indvars.iv172, 1
  %exitcond174 = icmp eq i64 %indvars.iv.next173, 11
  br i1 %exitcond174, label %for.inc13, label %for.body3

for.inc13:                                        ; preds = %for.body3
  %indvars.iv.next176 = add nuw nsw i64 %indvars.iv175, 1
  %exitcond177 = icmp eq i64 %indvars.iv.next176, 11
  br i1 %exitcond177, label %for.cond19.preheader, label %for.cond1.preheader

for.cond19.preheader:                             ; preds = %for.inc13, %for.inc38
  %indvars.iv169 = phi i64 [ %indvars.iv.next170, %for.inc38 ], [ 1, %for.inc13 ]
  %5 = trunc i64 %indvars.iv169 to i32
  %rem26 = srem i32 %5, 2
  %sub27 = sub nsw i32 %5, %rem26
  %idxprom28 = sext i32 %sub27 to i64
  %arrayidx29 = getelementptr inbounds i32*, i32** %A, i64 %idxprom28
  %6 = load i32*, i32** %arrayidx29, align 8, !tbaa !1
  %arrayidx33 = getelementptr inbounds i32*, i32** %A, i64 %indvars.iv169
  %7 = load i32*, i32** %arrayidx33, align 8, !tbaa !1
  br label %for.body21

for.body21:                                       ; preds = %for.body21, %for.cond19.preheader
  %indvars.iv165 = phi i64 [ 1, %for.cond19.preheader ], [ %indvars.iv.next166, %for.body21 ]
  %8 = trunc i64 %indvars.iv165 to i32
  %rem22 = srem i32 %8, 3
  %9 = add i64 %indvars.iv165, 4294967295
  %10 = trunc i64 %9 to i32
  %add24 = add i32 %10, %rem22
  %idxprom25 = sext i32 %add24 to i64
  %arrayidx30 = getelementptr inbounds i32, i32* %6, i64 %idxprom25
  %11 = load i32, i32* %arrayidx30, align 4, !tbaa !5
  %arrayidx34 = getelementptr inbounds i32, i32* %7, i64 %indvars.iv165
  store i32 %11, i32* %arrayidx34, align 4, !tbaa !5
  %indvars.iv.next166 = add nuw nsw i64 %indvars.iv165, 1
  %exitcond168 = icmp eq i64 %indvars.iv.next166, 11
  br i1 %exitcond168, label %for.inc38, label %for.body21

for.inc38:                                        ; preds = %for.body21
  %indvars.iv.next170 = add nuw nsw i64 %indvars.iv169, 1
  %exitcond171 = icmp eq i64 %indvars.iv.next170, 11
  br i1 %exitcond171, label %for.cond44.preheader, label %for.cond19.preheader

for.cond44.preheader:                             ; preds = %for.inc38, %for.inc63
  %indvars.iv162 = phi i64 [ %indvars.iv.next163, %for.inc63 ], [ 1, %for.inc38 ]
  %12 = trunc i64 %indvars.iv162 to i32
  %rem51 = srem i32 %12, 2
  %add52 = add nsw i32 %rem51, %12
  %idxprom53 = sext i32 %add52 to i64
  %arrayidx54 = getelementptr inbounds i32*, i32** %A, i64 %idxprom53
  %13 = load i32*, i32** %arrayidx54, align 8, !tbaa !1
  %arrayidx58 = getelementptr inbounds i32*, i32** %A, i64 %indvars.iv162
  %14 = load i32*, i32** %arrayidx58, align 8, !tbaa !1
  br label %for.body46

for.body46:                                       ; preds = %for.body46, %for.cond44.preheader
  %indvars.iv158 = phi i64 [ 1, %for.cond44.preheader ], [ %indvars.iv.next159, %for.body46 ]
  %15 = trunc i64 %indvars.iv158 to i32
  %rem47 = srem i32 %15, 3
  %16 = add i64 %indvars.iv158, 4294967295
  %17 = trunc i64 %16 to i32
  %add49 = add i32 %17, %rem47
  %idxprom50 = sext i32 %add49 to i64
  %arrayidx55 = getelementptr inbounds i32, i32* %13, i64 %idxprom50
  %18 = load i32, i32* %arrayidx55, align 4, !tbaa !5
  %arrayidx59 = getelementptr inbounds i32, i32* %14, i64 %indvars.iv158
  store i32 %18, i32* %arrayidx59, align 4, !tbaa !5
  %indvars.iv.next159 = add nuw nsw i64 %indvars.iv158, 1
  %exitcond161 = icmp eq i64 %indvars.iv.next159, 11
  br i1 %exitcond161, label %for.inc63, label %for.body46

for.inc63:                                        ; preds = %for.body46
  %indvars.iv.next163 = add nuw nsw i64 %indvars.iv162, 1
  %exitcond164 = icmp eq i64 %indvars.iv.next163, 11
  br i1 %exitcond164, label %for.cond69.preheader.preheader, label %for.cond44.preheader

for.cond69.preheader.preheader:                   ; preds = %for.inc63
  %.pre = load i32*, i32** %A, align 8, !tbaa !1
  br label %for.cond69.preheader

for.cond69.preheader:                             ; preds = %for.inc87, %for.cond69.preheader.preheader
  %19 = phi i32* [ %.pre, %for.cond69.preheader.preheader ], [ %20, %for.inc87 ]
  %indvars.iv154 = phi i64 [ 1, %for.cond69.preheader.preheader ], [ %indvars.iv.next155, %for.inc87 ]
  %arrayidx82 = getelementptr inbounds i32*, i32** %A, i64 %indvars.iv154
  %20 = load i32*, i32** %arrayidx82, align 8, !tbaa !1
  br label %for.body71

for.body71:                                       ; preds = %for.body71, %for.cond69.preheader
  %indvars.iv = phi i64 [ 1, %for.cond69.preheader ], [ %indvars.iv.next, %for.body71 ]
  %21 = trunc i64 %indvars.iv to i32
  %rem72 = srem i32 %21, 3
  %22 = add i64 %indvars.iv, 4294967295
  %23 = trunc i64 %22 to i32
  %add74 = add i32 %23, %rem72
  %idxprom75 = sext i32 %add74 to i64
  %arrayidx79 = getelementptr inbounds i32, i32* %19, i64 %idxprom75
  %24 = load i32, i32* %arrayidx79, align 4, !tbaa !5
  %arrayidx83 = getelementptr inbounds i32, i32* %20, i64 %indvars.iv
  store i32 %24, i32* %arrayidx83, align 4, !tbaa !5
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 11
  br i1 %exitcond, label %for.inc87, label %for.body71

for.inc87:                                        ; preds = %for.body71
  %indvars.iv.next155 = add nuw nsw i64 %indvars.iv154, 1
  %exitcond157 = icmp eq i64 %indvars.iv.next155, 11
  br i1 %exitcond157, label %for.end89, label %for.cond69.preheader

for.end89:                                        ; preds = %for.inc87
  %arrayidx90 = getelementptr inbounds i32*, i32** %A, i64 1
  %25 = load i32*, i32** %arrayidx90, align 8, !tbaa !1
  %arrayidx91 = getelementptr inbounds i32, i32* %25, i64 1
  %26 = load i32, i32* %arrayidx91, align 4, !tbaa !5
  %arrayidx92 = getelementptr inbounds i32*, i32** %B, i64 1
  %27 = load i32*, i32** %arrayidx92, align 8, !tbaa !1
  %arrayidx93 = getelementptr inbounds i32, i32* %27, i64 1
  %28 = load i32, i32* %arrayidx93, align 4, !tbaa !5
  %add94 = add i32 %26, 1
  %add95 = add i32 %add94, %28
  ret i32 %add95
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #0 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.9.0 (trunk 9725) (llvm/branches/loopopt 9729)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"any pointer", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
!5 = !{!6, !6, i64 0}
!6 = !{!"int", !3, i64 0}
