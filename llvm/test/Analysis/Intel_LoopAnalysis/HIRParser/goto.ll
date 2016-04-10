; Check goto parsing
; RUN: opt < %s -mem2reg | opt -analyze -hir-parser | FileCheck %s
; CHECK-DAG: goto [[LABEL1:.*]];
; CHECK-DAG: goto [[LABEL2:.*]];
; CHECK-DAG: [[LABEL1]]:
; CHECK-DAG: [[LABEL2]]:

; Check goto CG
; RUN: opt < %s -mem2reg -hir-cg -force-hir-cg -S | FileCheck -check-prefix=CHECK-CG %s
;          BEGIN REGION { }
;<54>         + DO i1 = 0, 1, 1   <DO_LOOP>
;<4>          |   (@A)[0][i1] = i1;
;<6>          |   if (i1 == 0)
;<6>          |   {
;<6>          |   }
;<6>          |   else
;<6>          |   {
;<40>         |      if (i1 == 1)
;<40>         |      {
;<44>         |         goto L2;
;<40>         |      }
;<40>         |      else
;<40>         |      {
;<47>         |         if (i1 == 2)
;<47>         |         {
;<51>         |            goto L2.63;
;<47>         |         }
;<40>         |      }
;<6>          |   }
;<14>         |   %0 = (@A)[0][i1];
;<16>         |   (@A)[0][i1] = %0 + 1;
;<18>         |   L2:
;<21>         |   %1 = (@A)[0][i1];
;<23>         |   (@A)[0][i1] = %1 + 1;
;<25>         |   L2.63:
;<28>         |   %2 = (@A)[0][i1];
;<30>         |   (@A)[0][i1] = %2 + 1;
;<54>         + END LOOP
;          END REGION
;CHECK-CG: region:
;look in first else block
;CHECK-CG: else{{.*}}:
;CHECK-CG: load i32, i32* %i1.i32
;look for the i1 == 1 condition
;CHECK-CG-NEXT: %hir.cmp.[[IF_NUM:[0-9]+]] = icmp eq i32 {{.*}}, 1
;CHECK-CG-NEXT: br i1 %hir.cmp.[[IF_NUM]]

;then block contains only a jump to the hir version of L2
;CHECK-CG: then.[[IF_NUM]]:
;CHECK-CG-NEXT: br label %hir.L2

;which contains ld/st to a[0][i1]
;CHECK-CG: hir.L2:
;CHECK-CG: getelementptr inbounds [5 x i32], [5 x i32]* @A, i32 0,
;CHECK-CG: getelementptr inbounds [5 x i32], [5 x i32]* @A, i32 0,
; and a jump to hir version of L2.63
;CHECK-CG: br label %hir.L2.63


;look for the i1 == 2 condition
;CHECK-CG: %hir.cmp.[[IF_NUM2:[0-9]+]] = icmp eq i32 {{.*}}, 2
;CHECK-CG-NEXT: br i1 %hir.cmp.[[IF_NUM2]]

;then block contains only a jump to the hir version of L2.63
;CHECK-CG: then.[[IF_NUM2]]:
;CHECK-CG-NEXT: br label %hir.L2.63

;which also contains ld/st to a[0][i1]
;CHECK-CG: hir.L2.63:
;CHECK-CG: getelementptr inbounds [5 x i32], [5 x i32]* @A, i32 0,
;CHECK-CG: getelementptr inbounds [5 x i32], [5 x i32]* @A, i32 0,
; and ivupdate and loop end
;CHECK-CG: br i1 %condloop

; ModuleID = 'goto.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = internal global [5 x i32] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %i = alloca i32, align 4
  store i32 0, i32* %retval
  store i32 0, i32* %i, align 4
  br label %do.body

do.body:                                          ; preds = %do.cond, %entry
  %0 = load i32, i32* %i, align 4
  %1 = load i32, i32* %i, align 4
  %idxprom = sext i32 %1 to i64
  %arrayidx = getelementptr inbounds [5 x i32], [5 x i32]* @A, i32 0, i64 %idxprom
  store i32 %0, i32* %arrayidx, align 4
  %2 = load i32, i32* %i, align 4
  %cmp = icmp eq i32 %2, 0
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %do.body
  br label %L1

if.end:                                           ; preds = %do.body
  %3 = load i32, i32* %i, align 4
  %cmp1 = icmp eq i32 %3, 1
  br i1 %cmp1, label %if.then.2, label %if.end.3

if.then.2:                                        ; preds = %if.end
  br label %L2

if.end.3:                                         ; preds = %if.end
  %4 = load i32, i32* %i, align 4
  %cmp4 = icmp eq i32 %4, 2
  br i1 %cmp4, label %if.then.5, label %if.end.6

if.then.5:                                        ; preds = %if.end.3
  br label %L2.63

if.end.6:                                         ; preds = %if.end.3
  br label %L1

L1:                                               ; preds = %if.end.6, %if.then
  %5 = load i32, i32* %i, align 4
  %idxprom7 = sext i32 %5 to i64
  %arrayidx8 = getelementptr inbounds [5 x i32], [5 x i32]* @A, i32 0, i64 %idxprom7
  %6 = load i32, i32* %arrayidx8, align 4
  %inc = add nsw i32 %6, 1
  store i32 %inc, i32* %arrayidx8, align 4
  br label %L2

L2:                                               ; preds = %L1, %if.then.2
  %7 = load i32, i32* %i, align 4
  %idxprom9 = sext i32 %7 to i64
  %arrayidx10 = getelementptr inbounds [5 x i32], [5 x i32]* @A, i32 0, i64 %idxprom9
  %8 = load i32, i32* %arrayidx10, align 4
  %inc11 = add nsw i32 %8, 1
  store i32 %inc11, i32* %arrayidx10, align 4
  br label %L2.63

L2.63:                                               ; preds = %L2, %if.then.5
  %9 = load i32, i32* %i, align 4
  %idxprom12 = sext i32 %9 to i64
  %arrayidx13 = getelementptr inbounds [5 x i32], [5 x i32]* @A, i32 0, i64 %idxprom12
  %10 = load i32, i32* %arrayidx13, align 4
  %inc14 = add nsw i32 %10, 1
  store i32 %inc14, i32* %arrayidx13, align 4
  %11 = load i32, i32* %i, align 4
  %inc15 = add nsw i32 %11, 1
  store i32 %inc15, i32* %i, align 4
  br label %do.cond

do.cond:                                          ; preds = %L2.63
  %12 = load i32, i32* %i, align 4
  %cmp16 = icmp ne i32 %12, 2
  br i1 %cmp16, label %do.body, label %do.end

do.end:                                           ; preds = %do.cond
  %13 = load i32, i32* getelementptr inbounds ([5 x i32], [5 x i32]* @A, i32 0, i64 0), align 4
  ret i32 %13
}
