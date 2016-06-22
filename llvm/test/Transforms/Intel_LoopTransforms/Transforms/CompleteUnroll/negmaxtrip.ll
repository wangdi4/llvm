; This test will check that we ignore loops which have max trip count as negative.

; RUN: opt -hir-ssa-deconstruction -hir-complete-unroll -print-after=hir-complete-unroll 2>&1 < %s | FileCheck %s

; CHECK: BEGIN REGION { }
; CHECK: END REGION

; HIR
; BEGIN REGION { }
;   DO i1 = 0, 44, 1   <DO_LOOP>
;   %ucs.promoted37 = {al:4}(@ucs)[0];
;   %0 = %ucs.promoted37;
;   + DO i2 = 0, -1 * i1 + 42, 1   <DO_LOOP>
;     %0 = %0  +  -1 * i1 + -1 * i2 + 54;
;   + END LOOP
;   {al:4}(@ucs)[0] = %0;
;   %t.029 = i1;
;   %t.029.out = %t.029;
;   END LOOP
; END REGION




; ModuleID = 'cq138770.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@ucs = common global i32 0, align 4
@.str = private unnamed_addr constant [4 x i8] c"%s\0A\00", align 1
@.str.1 = private unnamed_addr constant [7 x i8] c"passed\00", align 1
@.str.2 = private unnamed_addr constant [7 x i8] c"FAILED\00", align 1

; Function Attrs: nounwind uwtable
define i32 @main() #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.end10, %entry
  %indvars.iv = phi i32 [ 54, %entry ], [ %indvars.iv.next, %for.end10 ]
  %i.030 = phi i32 [ 0, %entry ], [ %inc12, %for.end10 ]
  %t.029 = phi i32 [ 0, %entry ], [ %t.1.lcssa, %for.end10 ]
  %cmp227 = icmp ult i32 %i.030, 43
  br i1 %cmp227, label %for.end.preheader, label %for.end10

for.end.preheader:                                ; preds = %for.cond1.preheader
  %ucs.promoted37 = load i32, i32* @ucs, align 4, !tbaa !1
  br label %for.end

for.end:                                          ; preds = %for.end.preheader, %for.end
  %0 = phi i32 [ %1, %for.end ], [ %ucs.promoted37, %for.end.preheader ]
  %indvars.iv33 = phi i32 [ %indvars.iv.next34, %for.end ], [ %indvars.iv, %for.end.preheader ]
  %k.028 = phi i32 [ %inc9, %for.end ], [ %i.030, %for.end.preheader ]
  %1 = add i32 %0, %indvars.iv33
  %inc9 = add nuw nsw i32 %k.028, 1
  %indvars.iv.next34 = add nsw i32 %indvars.iv33, -1
  %exitcond35 = icmp eq i32 %inc9, 43
  br i1 %exitcond35, label %for.end10.loopexit, label %for.end

for.end10.loopexit:                               ; preds = %for.end
  store i32 %1, i32* @ucs, align 4, !tbaa !1
  br label %for.end10

for.end10:                                        ; preds = %for.end10.loopexit, %for.cond1.preheader
  %t.1.lcssa = phi i32 [ %t.029, %for.cond1.preheader ], [ %i.030, %for.end10.loopexit ]
  %inc12 = add nuw nsw i32 %i.030, 1
  %indvars.iv.next = add nsw i32 %indvars.iv, -1
  %exitcond36 = icmp eq i32 %inc12, 45
  br i1 %exitcond36, label %for.end13, label %for.cond1.preheader

for.end13:                                        ; preds = %for.end10
  %cmp14 = icmp eq i32 %t.1.lcssa, 42
  %cond = select i1 %cmp14, i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str.1, i64 0, i64 0), i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str.2, i64 0, i64 0)
  %puts = tail call i32 @puts(i8* %cond)
  ret i32 0
}

; Function Attrs: nounwind
declare i32 @puts(i8* nocapture) #1

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.9.0 (trunk 6818) (llvm/branches/loopopt 8638)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
