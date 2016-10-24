; l2reversal-ddtest-1d.ll
; (2-level loop, sanity testcase, for 2D array dependence tests)
; 
; [REASONS]
; - Applicalbe: NO, HAS no valid (0) negative memory-access addresses, and 0 positive memory-access address; 
; - Profitable: N/A
; - Legal:      N/A
;
;
; *** Source Code ***
;
;[BEFORE LOOP REVERSAL]
;
;//testing for data dependence for 2D array
;int foo(int A[100][100], int B[100][100]) {
;  int i = 0, j = 0;
;
;  //Loop0: (<,<): valid for reversal
;  //Status: 
;  for (i = 1; i <= 10; i++) {
;    for (j = 1; j <= 10; j++) {
;      A[i][j] = A[i - 1][j - 1] + 1;
;    }
;  }
;
;  //Loop1: (<,=): valid for reversal
;  //Status: 
;  for (i = 1; i <= 10; i++) {
;    for (j = 1; j <= 10; j++) {
;      A[i][j] = A[i - 1][j] + 1;
;    }
;  }
;
;  //Loop2: (<,>): valid for reversal
;  //Status: 
;  for (i = 1; i <= 10; i++) {
;    for (j = 1; j <= 10; j++) {
;      A[i][j] = A[i - 1][j + 1] + 1;
;    }
;  }
;
;  //Loop3: (=,<): invalid for reversal
;  //Status: 
;  for (i = 1; i <= 10; i++) {
;    for (j = 1; j <= 10; j++) {
;      A[i][j] = A[i][j - 1] + 1;
;    }
;  }
;
;  //Loop4: (=,=): valid for reversal
;  //Status: 
;  for (i = 1; i <= 10; i++) {
;    for (j = 1; j <= 10; j++) {
;      A[i][j] = A[i][j] + 1;
;    }
;  }
;
;  //Loop5: (=,>): invalid for reversal
;  //Status: 
;  for (i = 1; i <= 10; i++) {
;    for (j = 1; j <= 10; j++) {
;      A[i][j] = A[i][j + 1] + 1;
;    }
;  }
;
;  //Loop6: (>,<): valid for reversal
;  //Status: 
;  for (i = 1; i <= 10; i++) {
;    for (j = 1; j <= 10; j++) {
;      A[i][j] = A[i + 1][j - 1] + 1;
;    }
;  }
;
;  //Loop7: (>,=): valid for reversal
;  //Status: 
;  for (i = 1; i <= 10; i++) {
;    for (j = 1; j <= 10; j++) {
;      A[i][j] = A[i + 1][j] + 1;
;    }
;  }
;
;  //8. (>,>): valid for reversal
;  //Status: 
;  for (i = 1; i <= 10; i++) {
;    for (j = 1; j <= 10; j++) {
;      A[i][j] = A[i + 1][j + 1] + 1;
;    }
;  }
;
;  return A[1][1] + B[1][1] + 1;
;}
;
;[AFTER LOOP REVERSAL]{Expected!}
;
;//testing for data dependence for 2D array
;int foo(int A[100][100], int B[100][100]) {
;  int i = 0, j = 0;
;
;  //Loop0: (<,<): valid for reversal, not suitable
;  //Status:
;  for (i = 1; i <= 10; i++) {
;    for (j = 1; j <= 10; j++) {
;      A[i][j] = A[i - 1][j - 1] + 1;
;    }
;  }
;
;  //2. (<,=): valid for reversal, not suitble
;  for (i = 1; i <= 10; i++) {
;    for (j = 1; j <= 10; j++) {
;      A[i][j] = A[i - 1][j] + 1;
;    }
;  }
;
;  //3. (<,>): valid for reversal, not suitble
;  for (i = 1; i <= 10; i++) {
;    for (j = 1; j <= 10; j++) {
;      A[i][j] = A[i - 1][j + 1] + 1;
;    }
;  }
;
;  //4. (=,<): invalid for reversal
;  for (i = 1; i <= 10; i++) {
;    for (j = 1; j <= 10; j++) {
;      A[i][j] = A[i][j - 1] + 1;
;    }
;  }
;
;  //5. (=,=): valid for reversal, not suitble
;  for (i = 1; i <= 10; i++) {
;    for (j = 1; j <= 10; j++) {
;      A[i][j] = A[i][j] + 1;
;    }
;  }
;
;  //6. (=,>): invalid for reversal
;  for (i = 1; i <= 10; i++) {
;    for (j = 1; j <= 10; j++) {
;      A[i][j] = A[i][j + 1] + 1;
;    }
;  }
;
;  //7. (>,<): valid for reversal, not suitble
;  for (i = 1; i <= 10; i++) {
;    for (j = 1; j <= 10; j++) {
;      A[i][j] = A[i + 1][j - 1] + 1;
;    }
;  }
;
;  //8. (>,=): valid for reversal, not suitble
;  for (i = 1; i <= 10; i++) {
;    for (j = 1; j <= 10; j++) {
;      A[i][j] = A[i + 1][j] + 1;
;    }
;  }
;
;  //9. (>,>): valid for reversal, not suitble
;  for (i = 1; i <= 10; i++) {
;    for (j = 1; j <= 10; j++) {
;      A[i][j] = A[i + 1][j + 1] + 1;
;    }
;  }
;
;  return A[1][1] + B[1][1] + 1;
;}
; 
; ===-----------------------------------===
; *** Run0: BEFORE HIR Loop Reversal ***
; ===-----------------------------------===
; RUN: opt -hir-ssa-deconstruction -hir-loop-reversal -print-before=hir-loop-reversal -S 2>&1	\
; RUN: < %s  |	FileCheck %s -check-prefix=BEFORE 
;
;; ===-----------------------------------===
; *** Run1: AFTER HIR Loop Reversal ***
; ===-----------------------------------===
; RUN: opt -hir-ssa-deconstruction -hir-loop-reversal -print-after=hir-loop-reversal -S 2>&1	\
; RUN: < %s  |	FileCheck %s -check-prefix=AFTER
;
;
; === -------------------------------------- ===
; *** Tests0: W/O HIR Loop Reversal Output ***
; === -------------------------------------- ===
; Expected output before Loop Reversal
;
; Loop0: 
;          BEGIN REGION { }
;<117>        + DO i1 = 0, 9, 1   <DO_LOOP>
;<4>          |   %2 = (%A)[%indvars.iv359 + -1][i1];
;<7>          |   (%A)[%indvars.iv359][i1 + 1] = %2 + 1;
;<117>        + END LOOP
;          END REGION
;
; Loop1:
;          BEGIN REGION { }
;<118>        + DO i1 = 0, 9, 1   <DO_LOOP>
;<17>         |   %4 = (%A)[%indvars.iv351 + -1][i1 + 1];
;<20>         |   (%A)[%indvars.iv351][i1 + 1] = %4 + 1;
;<118>        + END LOOP
;          END REGION
;
; Loop2:
;          BEGIN REGION { }
;<119>        + DO i1 = 0, 9, 1   <DO_LOOP>
;<31>         |   %6 = (%A)[%indvars.iv344 + -1][i1 + 2];
;<34>         |   (%A)[%indvars.iv344][i1 + 1] = %6 + 1;
;<119>        + END LOOP
;          END REGION
;
; Loop3:
;          BEGIN REGION { }
;<120>        + DO i1 = 0, 9, 1   <DO_LOOP>
;<44>         |   (%A)[%indvars.iv338][i1 + 1] = i1 + %.pre + 1;
;<120>        + END LOOP
;          END REGION
;
; Loop4:
;          BEGIN REGION { }
;<121>        + DO i1 = 0, 9, 1   <DO_LOOP>
;<55>         |   %8 = (%A)[%indvars.iv331][i1 + 1];
;<57>         |   (%A)[%indvars.iv331][i1 + 1] = %8 + 1;
;<121>        + END LOOP
;          END REGION
;
; Loop5:
;          BEGIN REGION { }
;<122>        + DO i1 = 0, 9, 1   <DO_LOOP>
;<68>         |   %9 = (%A)[%indvars.iv325][i1 + 2];
;<71>         |   (%A)[%indvars.iv325][i1 + 1] = %9 + 1;
;<122>        + END LOOP
;          END REGION
;
; Loop6:
;          BEGIN REGION { }
;<123>        + DO i1 = 0, 9, 1   <DO_LOOP>
;<81>         |   %11 = (%A)[%indvars.iv319 + 1][i1];
;<84>         |   (%A)[%indvars.iv319][i1 + 1] = %11 + 1;
;<123>        + END LOOP
;          END REGION
;
; Loop7:
;          BEGIN REGION { }
;<124>        + DO i1 = 0, 9, 1   <DO_LOOP>
;<94>         |   %12 = (%A)[%indvars.iv312 + 1][i1 + 1];
;<97>         |   (%A)[%indvars.iv312][i1 + 1] = %12 + 1;
;<124>        + END LOOP
;          END REGION
;
; Loop8:
;          BEGIN REGION { }
;<125>        + DO i1 = 0, 9, 1   <DO_LOOP>
;<108>        |   %13 = (%A)[%indvars.iv306 + 1][i1 + 2];
;<111>        |   (%A)[%indvars.iv306][i1 + 1] = %13 + 1;
;<125>        + END LOOP
;          END REGION
;
;
;CHECK OUTPUT BeFORE DOING LOOP REVERSAL
;
; Loop0: 
; BEFORE:  BEGIN REGION { }
; BEFORE:     + DO i1 = 0, 9, 1   <DO_LOOP>
; BEFORE:     |   %2 = (%A)[%indvars.iv359 + -1][i1];
; BEFORE:     |   (%A)[%indvars.iv359][i1 + 1] = %2 + 1;
; BEFORE:     + END LOOP
; BEFORE:  END REGION
;
; Loop1:
; BEFORE:   BEGIN REGION { }
; BEFORE:     + DO i1 = 0, 9, 1   <DO_LOOP>
; BEFORE:     |   %4 = (%A)[%indvars.iv351 + -1][i1 + 1];
; BEFORE:     |   (%A)[%indvars.iv351][i1 + 1] = %4 + 1;
; BEFORE:     + END LOOP
; BEFORE:  END REGION
;
; Loop2:
; BEFORE:  BEGIN REGION { }
; BEFORE:     + DO i1 = 0, 9, 1   <DO_LOOP>
; BEFORE:     |   %6 = (%A)[%indvars.iv344 + -1][i1 + 2];
; BEFORE:     |   (%A)[%indvars.iv344][i1 + 1] = %6 + 1;
; BEFORE:     + END LOOP
; BEFORE:  END REGION
;
; Loop3:
; BEFORE:  BEGIN REGION { }
; BEFORE:     + DO i1 = 0, 9, 1   <DO_LOOP>
; BEFORE:     |   (%A)[%indvars.iv338][i1 + 1] = i1 + %.pre + 1;
; BEFORE:     + END LOOP
; BEFORE:  END REGION
;
; Loop4:
; BEFORE:  BEGIN REGION { }
; BEFORE:     + DO i1 = 0, 9, 1   <DO_LOOP>
; BEFORE:     |   %8 = (%A)[%indvars.iv331][i1 + 1];
; BEFORE:     |   (%A)[%indvars.iv331][i1 + 1] = %8 + 1;
; BEFORE:     + END LOOP
; BEFORE:  END REGION
;
; Loop5:
; BEFORE:  BEGIN REGION { }
; BEFORE:     + DO i1 = 0, 9, 1   <DO_LOOP>
; BEFORE:     |   %9 = (%A)[%indvars.iv325][i1 + 2];
; BEFORE:     |   (%A)[%indvars.iv325][i1 + 1] = %9 + 1;
; BEFORE:      + END LOOP
;          END REGION
;
; Loop6:
; BEFORE:  BEGIN REGION { }
; BEFORE:     + DO i1 = 0, 9, 1   <DO_LOOP>
; BEFORE:     |   %11 = (%A)[%indvars.iv319 + 1][i1];
; BEFORE:     |   (%A)[%indvars.iv319][i1 + 1] = %11 + 1;
; BEFORE:     + END LOOP
; BEFORE:  END REGION
;
; Loop7:
; BEFORE:  BEGIN REGION { }
; BEFORE:     + DO i1 = 0, 9, 1   <DO_LOOP>
; BEFORE:     |   %12 = (%A)[%indvars.iv312 + 1][i1 + 1];
; BEFORE:     |   (%A)[%indvars.iv312][i1 + 1] = %12 + 1;
; BEFORE:     + END LOOP
; BEFORE:  END REGION
;
; Loop8:
; BEFORE:  BEGIN REGION { }
; BEFORE:     + DO i1 = 0, 9, 1   <DO_LOOP>
; BEFORE:     |   %13 = (%A)[%indvars.iv306 + 1][i1 + 2];
; BEFORE:     |   (%A)[%indvars.iv306][i1 + 1] = %13 + 1;
; BEFORE:     + END LOOP
; BEFORE:  END REGION
;
; === -------------------------------------- ===
; *** Tests1: With HIR Loop Reversal Output ***
; === -------------------------------------- ===
;
; Expected HIR output after Loop-Reversal is enabled: Not reversal ever happened in this given input!
; 
; Loop0: 
;          BEGIN REGION { }
;<117>        + DO i1 = 0, 9, 1   <DO_LOOP>
;<4>          |   %2 = (%A)[%indvars.iv359 + -1][i1];
;<7>          |   (%A)[%indvars.iv359][i1 + 1] = %2 + 1;
;<117>        + END LOOP
;          END REGION
;
; Loop1:
;          BEGIN REGION { }
;<118>        + DO i1 = 0, 9, 1   <DO_LOOP>
;<17>         |   %4 = (%A)[%indvars.iv351 + -1][i1 + 1];
;<20>         |   (%A)[%indvars.iv351][i1 + 1] = %4 + 1;
;<118>        + END LOOP
;          END REGION
;
; Loop2:
;          BEGIN REGION { }
;<119>        + DO i1 = 0, 9, 1   <DO_LOOP>
;<31>         |   %6 = (%A)[%indvars.iv344 + -1][i1 + 2];
;<34>         |   (%A)[%indvars.iv344][i1 + 1] = %6 + 1;
;<119>        + END LOOP
;          END REGION
;
; Loop3:
;          BEGIN REGION { }
;<120>        + DO i1 = 0, 9, 1   <DO_LOOP>
;<44>         |   (%A)[%indvars.iv338][i1 + 1] = i1 + %.pre + 1;
;<120>        + END LOOP
;          END REGION
;
; Loop4:
;          BEGIN REGION { }
;<121>        + DO i1 = 0, 9, 1   <DO_LOOP>
;<55>         |   %8 = (%A)[%indvars.iv331][i1 + 1];
;<57>         |   (%A)[%indvars.iv331][i1 + 1] = %8 + 1;
;<121>        + END LOOP
;          END REGION
;
; Loop5:
;          BEGIN REGION { }
;<122>        + DO i1 = 0, 9, 1   <DO_LOOP>
;<68>         |   %9 = (%A)[%indvars.iv325][i1 + 2];
;<71>         |   (%A)[%indvars.iv325][i1 + 1] = %9 + 1;
;<122>        + END LOOP
;          END REGION
;
; Loop6:
;          BEGIN REGION { }
;<123>        + DO i1 = 0, 9, 1   <DO_LOOP>
;<81>         |   %11 = (%A)[%indvars.iv319 + 1][i1];
;<84>         |   (%A)[%indvars.iv319][i1 + 1] = %11 + 1;
;<123>        + END LOOP
;          END REGION
;
; Loop7:
;          BEGIN REGION { }
;<124>        + DO i1 = 0, 9, 1   <DO_LOOP>
;<94>         |   %12 = (%A)[%indvars.iv312 + 1][i1 + 1];
;<97>         |   (%A)[%indvars.iv312][i1 + 1] = %12 + 1;
;<124>        + END LOOP
;          END REGION
;
; Loop8:
;          BEGIN REGION { }
;<125>        + DO i1 = 0, 9, 1   <DO_LOOP>
;<108>        |   %13 = (%A)[%indvars.iv306 + 1][i1 + 2];
;<111>        |   (%A)[%indvars.iv306][i1 + 1] = %13 + 1;
;<125>        + END LOOP
;          END REGION
;
;
;CHECK OUTPUT After DOING LOOP REVERSAL
;
; Loop0: 
; AFTER:  BEGIN REGION { }
; AFTER:     + DO i1 = 0, 9, 1   <DO_LOOP>
; AFTER:     |   %2 = (%A)[%indvars.iv359 + -1][i1];
; AFTER:     |   (%A)[%indvars.iv359][i1 + 1] = %2 + 1;
; AFTER:     + END LOOP
; AFTER:  END REGION
;
; Loop1:
; AFTER:   BEGIN REGION { }
; AFTER:     + DO i1 = 0, 9, 1   <DO_LOOP>
; AFTER:     |   %4 = (%A)[%indvars.iv351 + -1][i1 + 1];
; AFTER:     |   (%A)[%indvars.iv351][i1 + 1] = %4 + 1;
; AFTER:     + END LOOP
; AFTER:  END REGION
;
; Loop2:
; AFTER:  BEGIN REGION { }
; AFTER:     + DO i1 = 0, 9, 1   <DO_LOOP>
; AFTER:     |   %6 = (%A)[%indvars.iv344 + -1][i1 + 2];
; AFTER:     |   (%A)[%indvars.iv344][i1 + 1] = %6 + 1;
; AFTER:     + END LOOP
; AFTER:  END REGION
;
; Loop3:
; AFTER:  BEGIN REGION { }
; AFTER:     + DO i1 = 0, 9, 1   <DO_LOOP>
; AFTER:     |   (%A)[%indvars.iv338][i1 + 1] = i1 + %.pre + 1;
; AFTER:     + END LOOP
; AFTER:  END REGION
;
; Loop4:
; AFTER:  BEGIN REGION { }
; AFTER:     + DO i1 = 0, 9, 1   <DO_LOOP>
; AFTER:     |   %8 = (%A)[%indvars.iv331][i1 + 1];
; AFTER:     |   (%A)[%indvars.iv331][i1 + 1] = %8 + 1;
; AFTER:     + END LOOP
; AFTER:  END REGION
;
; Loop5:
; AFTER:  BEGIN REGION { }
; AFTER:     + DO i1 = 0, 9, 1   <DO_LOOP>
; AFTER:     |   %9 = (%A)[%indvars.iv325][i1 + 2];
; AFTER:     |   (%A)[%indvars.iv325][i1 + 1] = %9 + 1;
; AFTER:      + END LOOP
;          END REGION
;
; Loop6:
; AFTER:  BEGIN REGION { }
; AFTER:     + DO i1 = 0, 9, 1   <DO_LOOP>
; AFTER:     |   %11 = (%A)[%indvars.iv319 + 1][i1];
; AFTER:     |   (%A)[%indvars.iv319][i1 + 1] = %11 + 1;
; AFTER:     + END LOOP
; AFTER:  END REGION
;
; Loop7:
; AFTER:  BEGIN REGION { }
; AFTER:     + DO i1 = 0, 9, 1   <DO_LOOP>
; AFTER:     |   %12 = (%A)[%indvars.iv312 + 1][i1 + 1];
; AFTER:     |   (%A)[%indvars.iv312][i1 + 1] = %12 + 1;
; AFTER:     + END LOOP
; AFTER:  END REGION
;
; Loop8:
; AFTER:  BEGIN REGION { }
; AFTER:     + DO i1 = 0, 9, 1   <DO_LOOP>
; AFTER:     |   %13 = (%A)[%indvars.iv306 + 1][i1 + 2];
; AFTER:     |   (%A)[%indvars.iv306][i1 + 1] = %13 + 1;
; AFTER:     + END LOOP
; AFTER:  END REGION
; 
;
; === ---------------------------------------------------------------- ===
; Following is the LLVM's input code!
; === ---------------------------------------------------------------- ===
;
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define i32 @foo([100 x i32]* nocapture %A, [100 x i32]* nocapture readonly %B) #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc11, %entry
  %indvars.iv359 = phi i64 [ 1, %entry ], [ %indvars.iv.next360, %for.inc11 ]
  %0 = add nsw i64 %indvars.iv359, -1
  br label %for.body3

for.body3:                                        ; preds = %for.body3, %for.cond1.preheader
  %indvars.iv355 = phi i64 [ 1, %for.cond1.preheader ], [ %indvars.iv.next356, %for.body3 ]
  %1 = add nsw i64 %indvars.iv355, -1
  %arrayidx6 = getelementptr inbounds [100 x i32], [100 x i32]* %A, i64 %0, i64 %1
  %2 = load i32, i32* %arrayidx6, align 4, !tbaa !1
  %add = add nsw i32 %2, 1
  %arrayidx10 = getelementptr inbounds [100 x i32], [100 x i32]* %A, i64 %indvars.iv359, i64 %indvars.iv355
  store i32 %add, i32* %arrayidx10, align 4, !tbaa !1
  %indvars.iv.next356 = add nuw nsw i64 %indvars.iv355, 1
  %exitcond358 = icmp eq i64 %indvars.iv.next356, 11
  br i1 %exitcond358, label %for.inc11, label %for.body3

for.inc11:                                        ; preds = %for.body3
  %indvars.iv.next360 = add nuw nsw i64 %indvars.iv359, 1
  %exitcond362 = icmp eq i64 %indvars.iv.next360, 11
  br i1 %exitcond362, label %for.cond17.preheader, label %for.cond1.preheader

for.cond17.preheader:                             ; preds = %for.inc11, %for.inc33
  %indvars.iv351 = phi i64 [ %indvars.iv.next352, %for.inc33 ], [ 1, %for.inc11 ]
  %3 = add nsw i64 %indvars.iv351, -1
  br label %for.body19

for.body19:                                       ; preds = %for.body19, %for.cond17.preheader
  %indvars.iv348 = phi i64 [ 1, %for.cond17.preheader ], [ %indvars.iv.next349, %for.body19 ]
  %arrayidx24 = getelementptr inbounds [100 x i32], [100 x i32]* %A, i64 %3, i64 %indvars.iv348
  %4 = load i32, i32* %arrayidx24, align 4, !tbaa !1
  %add25 = add nsw i32 %4, 1
  %arrayidx29 = getelementptr inbounds [100 x i32], [100 x i32]* %A, i64 %indvars.iv351, i64 %indvars.iv348
  store i32 %add25, i32* %arrayidx29, align 4, !tbaa !1
  %indvars.iv.next349 = add nuw nsw i64 %indvars.iv348, 1
  %exitcond350 = icmp eq i64 %indvars.iv.next349, 11
  br i1 %exitcond350, label %for.inc33, label %for.body19

for.inc33:                                        ; preds = %for.body19
  %indvars.iv.next352 = add nuw nsw i64 %indvars.iv351, 1
  %exitcond354 = icmp eq i64 %indvars.iv.next352, 11
  br i1 %exitcond354, label %for.cond39.preheader, label %for.cond17.preheader

for.cond39.preheader:                             ; preds = %for.inc33, %for.inc56
  %indvars.iv344 = phi i64 [ %indvars.iv.next345, %for.inc56 ], [ 1, %for.inc33 ]
  %5 = add nsw i64 %indvars.iv344, -1
  br label %for.body41

for.body41:                                       ; preds = %for.body41, %for.cond39.preheader
  %indvars.iv341 = phi i64 [ 1, %for.cond39.preheader ], [ %indvars.iv.next342, %for.body41 ]
  %indvars.iv.next342 = add nuw nsw i64 %indvars.iv341, 1
  %arrayidx47 = getelementptr inbounds [100 x i32], [100 x i32]* %A, i64 %5, i64 %indvars.iv.next342
  %6 = load i32, i32* %arrayidx47, align 4, !tbaa !1
  %add48 = add nsw i32 %6, 1
  %arrayidx52 = getelementptr inbounds [100 x i32], [100 x i32]* %A, i64 %indvars.iv344, i64 %indvars.iv341
  store i32 %add48, i32* %arrayidx52, align 4, !tbaa !1
  %exitcond343 = icmp eq i64 %indvars.iv.next342, 11
  br i1 %exitcond343, label %for.inc56, label %for.body41

for.inc56:                                        ; preds = %for.body41
  %indvars.iv.next345 = add nuw nsw i64 %indvars.iv344, 1
  %exitcond347 = icmp eq i64 %indvars.iv.next345, 11
  br i1 %exitcond347, label %for.cond62.preheader, label %for.cond39.preheader

for.cond62.preheader:                             ; preds = %for.inc56, %for.inc78
  %indvars.iv338 = phi i64 [ %indvars.iv.next339, %for.inc78 ], [ 1, %for.inc56 ]
  %arrayidx69.phi.trans.insert = getelementptr inbounds [100 x i32], [100 x i32]* %A, i64 %indvars.iv338, i64 0
  %.pre = load i32, i32* %arrayidx69.phi.trans.insert, align 4, !tbaa !1
  br label %for.body64

for.body64:                                       ; preds = %for.body64, %for.cond62.preheader
  %7 = phi i32 [ %.pre, %for.cond62.preheader ], [ %add70, %for.body64 ]
  %indvars.iv334 = phi i64 [ 1, %for.cond62.preheader ], [ %indvars.iv.next335, %for.body64 ]
  %add70 = add nsw i32 %7, 1
  %arrayidx74 = getelementptr inbounds [100 x i32], [100 x i32]* %A, i64 %indvars.iv338, i64 %indvars.iv334
  store i32 %add70, i32* %arrayidx74, align 4, !tbaa !1
  %indvars.iv.next335 = add nuw nsw i64 %indvars.iv334, 1
  %exitcond337 = icmp eq i64 %indvars.iv.next335, 11
  br i1 %exitcond337, label %for.inc78, label %for.body64

for.inc78:                                        ; preds = %for.body64
  %indvars.iv.next339 = add nuw nsw i64 %indvars.iv338, 1
  %exitcond340 = icmp eq i64 %indvars.iv.next339, 11
  br i1 %exitcond340, label %for.cond84.preheader, label %for.cond62.preheader

for.cond84.preheader:                             ; preds = %for.inc78, %for.inc99
  %indvars.iv331 = phi i64 [ %indvars.iv.next332, %for.inc99 ], [ 1, %for.inc78 ]
  br label %for.body86

for.body86:                                       ; preds = %for.body86, %for.cond84.preheader
  %indvars.iv328 = phi i64 [ 1, %for.cond84.preheader ], [ %indvars.iv.next329, %for.body86 ]
  %arrayidx90 = getelementptr inbounds [100 x i32], [100 x i32]* %A, i64 %indvars.iv331, i64 %indvars.iv328
  %8 = load i32, i32* %arrayidx90, align 4, !tbaa !1
  %add91 = add nsw i32 %8, 1
  store i32 %add91, i32* %arrayidx90, align 4, !tbaa !1
  %indvars.iv.next329 = add nuw nsw i64 %indvars.iv328, 1
  %exitcond330 = icmp eq i64 %indvars.iv.next329, 11
  br i1 %exitcond330, label %for.inc99, label %for.body86

for.inc99:                                        ; preds = %for.body86
  %indvars.iv.next332 = add nuw nsw i64 %indvars.iv331, 1
  %exitcond333 = icmp eq i64 %indvars.iv.next332, 11
  br i1 %exitcond333, label %for.cond105.preheader, label %for.cond84.preheader

for.cond105.preheader:                            ; preds = %for.inc99, %for.inc121
  %indvars.iv325 = phi i64 [ %indvars.iv.next326, %for.inc121 ], [ 1, %for.inc99 ]
  br label %for.body107

for.body107:                                      ; preds = %for.body107, %for.cond105.preheader
  %indvars.iv322 = phi i64 [ 1, %for.cond105.preheader ], [ %indvars.iv.next323, %for.body107 ]
  %indvars.iv.next323 = add nuw nsw i64 %indvars.iv322, 1
  %arrayidx112 = getelementptr inbounds [100 x i32], [100 x i32]* %A, i64 %indvars.iv325, i64 %indvars.iv.next323
  %9 = load i32, i32* %arrayidx112, align 4, !tbaa !1
  %add113 = add nsw i32 %9, 1
  %arrayidx117 = getelementptr inbounds [100 x i32], [100 x i32]* %A, i64 %indvars.iv325, i64 %indvars.iv322
  store i32 %add113, i32* %arrayidx117, align 4, !tbaa !1
  %exitcond324 = icmp eq i64 %indvars.iv.next323, 11
  br i1 %exitcond324, label %for.inc121, label %for.body107

for.inc121:                                       ; preds = %for.body107
  %indvars.iv.next326 = add nuw nsw i64 %indvars.iv325, 1
  %exitcond327 = icmp eq i64 %indvars.iv.next326, 11
  br i1 %exitcond327, label %for.cond127.preheader, label %for.cond105.preheader

for.cond127.preheader:                            ; preds = %for.inc121, %for.inc144
  %indvars.iv319 = phi i64 [ %indvars.iv.next320, %for.inc144 ], [ 1, %for.inc121 ]
  %indvars.iv.next320 = add nuw nsw i64 %indvars.iv319, 1
  br label %for.body129

for.body129:                                      ; preds = %for.body129, %for.cond127.preheader
  %indvars.iv315 = phi i64 [ 1, %for.cond127.preheader ], [ %indvars.iv.next316, %for.body129 ]
  %10 = add nsw i64 %indvars.iv315, -1
  %arrayidx135 = getelementptr inbounds [100 x i32], [100 x i32]* %A, i64 %indvars.iv.next320, i64 %10
  %11 = load i32, i32* %arrayidx135, align 4, !tbaa !1
  %add136 = add nsw i32 %11, 1
  %arrayidx140 = getelementptr inbounds [100 x i32], [100 x i32]* %A, i64 %indvars.iv319, i64 %indvars.iv315
  store i32 %add136, i32* %arrayidx140, align 4, !tbaa !1
  %indvars.iv.next316 = add nuw nsw i64 %indvars.iv315, 1
  %exitcond318 = icmp eq i64 %indvars.iv.next316, 11
  br i1 %exitcond318, label %for.inc144, label %for.body129

for.inc144:                                       ; preds = %for.body129
  %exitcond321 = icmp eq i64 %indvars.iv.next320, 11
  br i1 %exitcond321, label %for.cond150.preheader, label %for.cond127.preheader

for.cond150.preheader:                            ; preds = %for.inc144, %for.inc166
  %indvars.iv312 = phi i64 [ %indvars.iv.next313, %for.inc166 ], [ 1, %for.inc144 ]
  %indvars.iv.next313 = add nuw nsw i64 %indvars.iv312, 1
  br label %for.body152

for.body152:                                      ; preds = %for.body152, %for.cond150.preheader
  %indvars.iv309 = phi i64 [ 1, %for.cond150.preheader ], [ %indvars.iv.next310, %for.body152 ]
  %arrayidx157 = getelementptr inbounds [100 x i32], [100 x i32]* %A, i64 %indvars.iv.next313, i64 %indvars.iv309
  %12 = load i32, i32* %arrayidx157, align 4, !tbaa !1
  %add158 = add nsw i32 %12, 1
  %arrayidx162 = getelementptr inbounds [100 x i32], [100 x i32]* %A, i64 %indvars.iv312, i64 %indvars.iv309
  store i32 %add158, i32* %arrayidx162, align 4, !tbaa !1
  %indvars.iv.next310 = add nuw nsw i64 %indvars.iv309, 1
  %exitcond311 = icmp eq i64 %indvars.iv.next310, 11
  br i1 %exitcond311, label %for.inc166, label %for.body152

for.inc166:                                       ; preds = %for.body152
  %exitcond314 = icmp eq i64 %indvars.iv.next313, 11
  br i1 %exitcond314, label %for.cond172.preheader, label %for.cond150.preheader

for.cond172.preheader:                            ; preds = %for.inc166, %for.inc189
  %indvars.iv306 = phi i64 [ %indvars.iv.next307, %for.inc189 ], [ 1, %for.inc166 ]
  %indvars.iv.next307 = add nuw nsw i64 %indvars.iv306, 1
  br label %for.body174

for.body174:                                      ; preds = %for.body174, %for.cond172.preheader
  %indvars.iv = phi i64 [ 1, %for.cond172.preheader ], [ %indvars.iv.next, %for.body174 ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %arrayidx180 = getelementptr inbounds [100 x i32], [100 x i32]* %A, i64 %indvars.iv.next307, i64 %indvars.iv.next
  %13 = load i32, i32* %arrayidx180, align 4, !tbaa !1
  %add181 = add nsw i32 %13, 1
  %arrayidx185 = getelementptr inbounds [100 x i32], [100 x i32]* %A, i64 %indvars.iv306, i64 %indvars.iv
  store i32 %add181, i32* %arrayidx185, align 4, !tbaa !1
  %exitcond = icmp eq i64 %indvars.iv.next, 11
  br i1 %exitcond, label %for.inc189, label %for.body174

for.inc189:                                       ; preds = %for.body174
  %exitcond308 = icmp eq i64 %indvars.iv.next307, 11
  br i1 %exitcond308, label %for.end191, label %for.cond172.preheader

for.end191:                                       ; preds = %for.inc189
  %arrayidx193 = getelementptr inbounds [100 x i32], [100 x i32]* %A, i64 1, i64 1
  %14 = load i32, i32* %arrayidx193, align 4, !tbaa !1
  %arrayidx195 = getelementptr inbounds [100 x i32], [100 x i32]* %B, i64 1, i64 1
  %15 = load i32, i32* %arrayidx195, align 4, !tbaa !1
  %add196 = add i32 %14, 1
  %add197 = add i32 %add196, %15
  ret i32 %add197
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #0 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 (trunk 2121) (llvm/branches/loopopt 3727)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
