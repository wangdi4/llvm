; RUN: opt -intel-libirc-allowed -hir-create-function-level-region -passes="hir-ssa-deconstruction,require<hir-loop-statistics>,hir-cross-loop-array-contraction,print<hir>,hir-cg" -force-hir-cg -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

;        BEGIN REGION { }
;              + DO i1 = 0, 99, 1   <DO_LOOP>
;              |   + DO i2 = 0, 99, 1   <DO_LOOP>
;              |   |   + DO i3 = 0, 99, 1   <DO_LOOP>
;              |   |   |   + DO i4 = 0, 9, 1   <DO_LOOP>
;              |   |   |   |   + DO i5 = 0, 9, 1   <DO_LOOP>
;              |   |   |   |   |   (%A)[0][i1][i2][i3][i4][i5] = 0;
;              |   |   |   |   + END LOOP
;              |   |   |   + END LOOP
;              |   |   + END LOOP
;              |   + END LOOP
;              + END LOOP
;
;              + UNKNOWN LOOP i1
;              |   <i1 = 0>
;              |   do.body:
;              |
;              |   + DO i2 = 0, 99, 1   <DO_LOOP>
;              |   |   + DO i3 = 0, 99, 1   <DO_LOOP>
;              |   |   |   + DO i4 = 0, 99, 1   <DO_LOOP>
;              |   |   |   |   + DO i5 = 0, 9, 1   <DO_LOOP>
;              |   |   |   |   |   + DO i6 = 0, 9, 1   <DO_LOOP>
;              |   |   |   |   |   |   (%A)[0][i2][i3][i4][i5][i6] = i5 + i6;
;              |   |   |   |   |   + END LOOP
;              |   |   |   |   + END LOOP
;              |   |   |   + END LOOP
;              |   |   + END LOOP
;              |   + END LOOP
;              |
;              |
;              |   + DO i2 = 0, 99, 1   <DO_LOOP>
;              |   |   + DO i3 = 0, 99, 1   <DO_LOOP>
;              |   |   |   + DO i4 = 0, 99, 1   <DO_LOOP>
;              |   |   |   |   + DO i5 = 0, 9, 1   <DO_LOOP>
;              |   |   |   |   |   + DO i6 = 0, 9, 1   <DO_LOOP>
;              |   |   |   |   |   |   %0 = (%A)[0][i2][i3][i4][i6][i5];
;              |   |   |   |   |   |   (@B)[0][i2][i3][i4][i5][i6] = %0 + 1;
;              |   |   |   |   |   + END LOOP
;              |   |   |   |   + END LOOP
;              |   |   |   + END LOOP
;              |   |   + END LOOP
;              |   + END LOOP
;              |
;              |   %1 = (%b)[0];
;              |   if (%1 != 0)
;              |   {
;              |      <i1 = i1 + 1>
;              |      goto do.body;
;              |   }
;              + END LOOP
;
;              ret ;
;        END REGION

; *** Region after HIR-CrossLoop Array Contraction Transformation ***

;            BEGIN REGION { modified }
;                  + UNKNOWN LOOP i1
;                  |   <i1 = 0>
;                  |   do.body:
;                  |
;                  |   + DO i2 = 0, 99, 1   <DO_LOOP>
;                  |   |   + DO i3 = 0, 99, 1   <DO_LOOP>
;                  |   |   |   + DO i4 = 0, 99, 1   <DO_LOOP>
;                  |   |   |   |   + DO i5 = 0, 9, 1   <DO_LOOP>
;                  |   |   |   |   |   + DO i6 = 0, 9, 1   <DO_LOOP>
;                  |   |   |   |   |   |   (%A)[0][i2][i3][i4][i5][i6] = i5 + i6;
;                  |   |   |   |   |   + END LOOP
;                  |   |   |   |   + END LOOP
;                  |   |   |   |
;                  |   |   |   |
;                  |   |   |   |   + DO i5 = 0, 9, 1   <DO_LOOP>
;                  |   |   |   |   |   + DO i6 = 0, 9, 1   <DO_LOOP>
;                  |   |   |   |   |   |   %0 = (%A)[0][i2][i3][i4][i6][i5];
;                  |   |   |   |   |   |   (@B)[0][i2][i3][i4][i5][i6] = %0 + 1;
;                  |   |   |   |   |   + END LOOP
;                  |   |   |   |   + END LOOP
;                  |   |   |   + END LOOP
;                  |   |   + END LOOP
;                  |   + END LOOP
;                  |
;                  |   %1 = (%b)[0];
;                  |   if (%1 != 0)
;                  |   {
;                  |      <i1 = i1 + 1>
;                  |      goto do.body;
;                  |   }
;                  + END LOOP
;
;                  ret ;
;            END REGION


; *** Region after HIR-CrossLoop Transformation with Array Contraction ***

; CHECK:     BEGIN REGION { modified }
;                  + UNKNOWN LOOP i1
;                  |   <i1 = 0>
;                  |   do.body:
;                  |
; CHECK:           |   + DO i2 = 0, 99, 1   <DO_LOOP>
; CHECK:           |   |   + DO i3 = 0, 99, 1   <DO_LOOP>
; CHECK:           |   |   |   + DO i4 = 0, 99, 1   <DO_LOOP>
; CHECK:           |   |   |   |   + DO i5 = 0, 9, 1   <DO_LOOP>
; CHECK:           |   |   |   |   |   + DO i6 = 0, 9, 1   <DO_LOOP>
; CHECK:           |   |   |   |   |   |   (%ContractedArray)[0][i5][i6] = i5 + i6;
; CHECK:           |   |   |   |   |   + END LOOP
; CHECK:           |   |   |   |   + END LOOP
;                  |   |   |   |
;                  |   |   |   |
; CHECK:           |   |   |   |   + DO i5 = 0, 9, 1   <DO_LOOP>
; CHECK:           |   |   |   |   |   + DO i6 = 0, 9, 1   <DO_LOOP>
; CHECK:           |   |   |   |   |   |   %0 = (%ContractedArray)[0][i6][i5];
; CHECK:           |   |   |   |   |   |   (@B)[0][i2][i3][i4][i5][i6] = %0 + 1;
; CHECK:           |   |   |   |   |   + END LOOP
; CHECK:           |   |   |   |   + END LOOP
; CHECK:           |   |   |   + END LOOP
; CHECK:           |   |   + END LOOP
; CHECK:           |   + END LOOP
;                  |
;                  |   %1 = (%b)[0];
;                  |   if (%1 != 0)
;                  |   {
;                  |      <i1 = i1 + 1>
;                  |      goto do.body;
;                  |   }
;                  + END LOOP
;
;                  ret ;
; CHECK:     END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = dso_local global [100 x [100 x [100 x [10 x [10 x i32]]]]] zeroinitializer, align 16

; Function Attrs: noinline nounwind uwtable
define dso_local void @shell(ptr %b) #0 {
entry:
  %A = alloca [100 x [100 x [100 x [10 x [10 x i32]]]]], align 16
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc30
  %m.052 = phi i32 [ 0, %entry ], [ %inc31, %for.inc30 ]
  br label %for.body3

for.body3:                                        ; preds = %for.body, %for.inc27
  %l.051 = phi i32 [ 0, %for.body ], [ %inc28, %for.inc27 ]
  br label %for.body6

for.body6:                                        ; preds = %for.body3, %for.inc24
  %k.050 = phi i32 [ 0, %for.body3 ], [ %inc25, %for.inc24 ]
  br label %for.body9

for.body9:                                        ; preds = %for.body6, %for.inc21
  %i.049 = phi i32 [ 0, %for.body6 ], [ %inc22, %for.inc21 ]
  br label %for.body12

for.body12:                                       ; preds = %for.body9, %for.inc
  %j.048 = phi i32 [ 0, %for.body9 ], [ %inc, %for.inc ]
  %idxprom = sext i32 %m.052 to i64
  %arrayidx = getelementptr inbounds [100 x [100 x [100 x [10 x [10 x i32]]]]], ptr %A, i64 0, i64 %idxprom
  %idxprom13 = sext i32 %l.051 to i64
  %arrayidx14 = getelementptr inbounds [100 x [100 x [10 x [10 x i32]]]], ptr %arrayidx, i64 0, i64 %idxprom13
  %idxprom15 = sext i32 %k.050 to i64
  %arrayidx16 = getelementptr inbounds [100 x [10 x [10 x i32]]], ptr %arrayidx14, i64 0, i64 %idxprom15
  %idxprom17 = sext i32 %i.049 to i64
  %arrayidx18 = getelementptr inbounds [10 x [10 x i32]], ptr %arrayidx16, i64 0, i64 %idxprom17
  %idxprom19 = sext i32 %j.048 to i64
  %arrayidx20 = getelementptr inbounds [10 x i32], ptr %arrayidx18, i64 0, i64 %idxprom19
  store i32 0, ptr %arrayidx20, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body12
  %inc = add nsw i32 %j.048, 1
  %cmp11 = icmp slt i32 %inc, 10
  br i1 %cmp11, label %for.body12, label %for.end

for.end:                                          ; preds = %for.inc
  br label %for.inc21

for.inc21:                                        ; preds = %for.end
  %inc22 = add nsw i32 %i.049, 1
  %cmp8 = icmp slt i32 %inc22, 10
  br i1 %cmp8, label %for.body9, label %for.end23

for.end23:                                        ; preds = %for.inc21
  br label %for.inc24

for.inc24:                                        ; preds = %for.end23
  %inc25 = add nsw i32 %k.050, 1
  %cmp5 = icmp slt i32 %inc25, 100
  br i1 %cmp5, label %for.body6, label %for.end26

for.end26:                                        ; preds = %for.inc24
  br label %for.inc27

for.inc27:                                        ; preds = %for.end26
  %inc28 = add nsw i32 %l.051, 1
  %cmp2 = icmp slt i32 %inc28, 100
  br i1 %cmp2, label %for.body3, label %for.end29

for.end29:                                        ; preds = %for.inc27
  br label %for.inc30

for.inc30:                                        ; preds = %for.end29
  %inc31 = add nsw i32 %m.052, 1
  %cmp = icmp slt i32 %inc31, 100
  br i1 %cmp, label %for.body, label %for.end32

for.end32:                                        ; preds = %for.inc30
  br label %do.body

do.body:                                          ; preds = %do.cond, %for.end32
  br label %for.body36

for.body36:                                       ; preds = %do.body, %for.inc75
  %m33.042 = phi i32 [ 0, %do.body ], [ %inc76, %for.inc75 ]
  br label %for.body40

for.body40:                                       ; preds = %for.body36, %for.inc72
  %l37.041 = phi i32 [ 0, %for.body36 ], [ %inc73, %for.inc72 ]
  br label %for.body44

for.body44:                                       ; preds = %for.body40, %for.inc69
  %k41.040 = phi i32 [ 0, %for.body40 ], [ %inc70, %for.inc69 ]
  br label %for.body48

for.body48:                                       ; preds = %for.body44, %for.inc66
  %i45.039 = phi i32 [ 0, %for.body44 ], [ %inc67, %for.inc66 ]
  br label %for.body52

for.body52:                                       ; preds = %for.body48, %for.inc63
  %j49.038 = phi i32 [ 0, %for.body48 ], [ %inc64, %for.inc63 ]
  %add = add nsw i32 %j49.038, %i45.039
  %idxprom53 = sext i32 %m33.042 to i64
  %arrayidx54 = getelementptr inbounds [100 x [100 x [100 x [10 x [10 x i32]]]]], ptr %A, i64 0, i64 %idxprom53
  %idxprom55 = sext i32 %l37.041 to i64
  %arrayidx56 = getelementptr inbounds [100 x [100 x [10 x [10 x i32]]]], ptr %arrayidx54, i64 0, i64 %idxprom55
  %idxprom57 = sext i32 %k41.040 to i64
  %arrayidx58 = getelementptr inbounds [100 x [10 x [10 x i32]]], ptr %arrayidx56, i64 0, i64 %idxprom57
  %idxprom59 = sext i32 %i45.039 to i64
  %arrayidx60 = getelementptr inbounds [10 x [10 x i32]], ptr %arrayidx58, i64 0, i64 %idxprom59
  %idxprom61 = sext i32 %j49.038 to i64
  %arrayidx62 = getelementptr inbounds [10 x i32], ptr %arrayidx60, i64 0, i64 %idxprom61
  store i32 %add, ptr %arrayidx62, align 4
  br label %for.inc63

for.inc63:                                        ; preds = %for.body52
  %inc64 = add nsw i32 %j49.038, 1
  %cmp51 = icmp slt i32 %inc64, 10
  br i1 %cmp51, label %for.body52, label %for.end65

for.end65:                                        ; preds = %for.inc63
  br label %for.inc66

for.inc66:                                        ; preds = %for.end65
  %inc67 = add nsw i32 %i45.039, 1
  %cmp47 = icmp slt i32 %inc67, 10
  br i1 %cmp47, label %for.body48, label %for.end68

for.end68:                                        ; preds = %for.inc66
  br label %for.inc69

for.inc69:                                        ; preds = %for.end68
  %inc70 = add nsw i32 %k41.040, 1
  %cmp43 = icmp slt i32 %inc70, 100
  br i1 %cmp43, label %for.body44, label %for.end71

for.end71:                                        ; preds = %for.inc69
  br label %for.inc72

for.inc72:                                        ; preds = %for.end71
  %inc73 = add nsw i32 %l37.041, 1
  %cmp39 = icmp slt i32 %inc73, 100
  br i1 %cmp39, label %for.body40, label %for.end74

for.end74:                                        ; preds = %for.inc72
  br label %for.inc75

for.inc75:                                        ; preds = %for.end74
  %inc76 = add nsw i32 %m33.042, 1
  %cmp35 = icmp slt i32 %inc76, 100
  br i1 %cmp35, label %for.body36, label %for.end77

for.end77:                                        ; preds = %for.inc75
  br label %for.body81

for.body81:                                       ; preds = %for.end77, %for.inc131
  %m78.047 = phi i32 [ 0, %for.end77 ], [ %inc132, %for.inc131 ]
  br label %for.body85

for.body85:                                       ; preds = %for.body81, %for.inc128
  %l82.046 = phi i32 [ 0, %for.body81 ], [ %inc129, %for.inc128 ]
  br label %for.body89

for.body89:                                       ; preds = %for.body85, %for.inc125
  %k86.045 = phi i32 [ 0, %for.body85 ], [ %inc126, %for.inc125 ]
  br label %for.body93

for.body93:                                       ; preds = %for.body89, %for.inc122
  %i90.044 = phi i32 [ 0, %for.body89 ], [ %inc123, %for.inc122 ]
  br label %for.body97

for.body97:                                       ; preds = %for.body93, %for.inc119
  %j94.043 = phi i32 [ 0, %for.body93 ], [ %inc120, %for.inc119 ]
  %idxprom98 = sext i32 %m78.047 to i64
  %arrayidx99 = getelementptr inbounds [100 x [100 x [100 x [10 x [10 x i32]]]]], ptr %A, i64 0, i64 %idxprom98
  %idxprom100 = sext i32 %l82.046 to i64
  %arrayidx101 = getelementptr inbounds [100 x [100 x [10 x [10 x i32]]]], ptr %arrayidx99, i64 0, i64 %idxprom100
  %idxprom102 = sext i32 %k86.045 to i64
  %arrayidx103 = getelementptr inbounds [100 x [10 x [10 x i32]]], ptr %arrayidx101, i64 0, i64 %idxprom102
  %idxprom104 = sext i32 %j94.043 to i64
  %arrayidx105 = getelementptr inbounds [10 x [10 x i32]], ptr %arrayidx103, i64 0, i64 %idxprom104
  %idxprom106 = sext i32 %i90.044 to i64
  %arrayidx107 = getelementptr inbounds [10 x i32], ptr %arrayidx105, i64 0, i64 %idxprom106
  %0 = load i32, ptr %arrayidx107, align 4
  %add108 = add nsw i32 %0, 1
  %idxprom109 = sext i32 %m78.047 to i64
  %arrayidx110 = getelementptr inbounds [100 x [100 x [100 x [10 x [10 x i32]]]]], ptr @B, i64 0, i64 %idxprom109
  %idxprom111 = sext i32 %l82.046 to i64
  %arrayidx112 = getelementptr inbounds [100 x [100 x [10 x [10 x i32]]]], ptr %arrayidx110, i64 0, i64 %idxprom111
  %idxprom113 = sext i32 %k86.045 to i64
  %arrayidx114 = getelementptr inbounds [100 x [10 x [10 x i32]]], ptr %arrayidx112, i64 0, i64 %idxprom113
  %idxprom115 = sext i32 %i90.044 to i64
  %arrayidx116 = getelementptr inbounds [10 x [10 x i32]], ptr %arrayidx114, i64 0, i64 %idxprom115
  %idxprom117 = sext i32 %j94.043 to i64
  %arrayidx118 = getelementptr inbounds [10 x i32], ptr %arrayidx116, i64 0, i64 %idxprom117
  store i32 %add108, ptr %arrayidx118, align 4
  br label %for.inc119

for.inc119:                                       ; preds = %for.body97
  %inc120 = add nsw i32 %j94.043, 1
  %cmp96 = icmp slt i32 %inc120, 10
  br i1 %cmp96, label %for.body97, label %for.end121

for.end121:                                       ; preds = %for.inc119
  br label %for.inc122

for.inc122:                                       ; preds = %for.end121
  %inc123 = add nsw i32 %i90.044, 1
  %cmp92 = icmp slt i32 %inc123, 10
  br i1 %cmp92, label %for.body93, label %for.end124

for.end124:                                       ; preds = %for.inc122
  br label %for.inc125

for.inc125:                                       ; preds = %for.end124
  %inc126 = add nsw i32 %k86.045, 1
  %cmp88 = icmp slt i32 %inc126, 100
  br i1 %cmp88, label %for.body89, label %for.end127

for.end127:                                       ; preds = %for.inc125
  br label %for.inc128

for.inc128:                                       ; preds = %for.end127
  %inc129 = add nsw i32 %l82.046, 1
  %cmp84 = icmp slt i32 %inc129, 100
  br i1 %cmp84, label %for.body85, label %for.end130

for.end130:                                       ; preds = %for.inc128
  br label %for.inc131

for.inc131:                                       ; preds = %for.end130
  %inc132 = add nsw i32 %m78.047, 1
  %cmp80 = icmp slt i32 %inc132, 100
  br i1 %cmp80, label %for.body81, label %for.end133

for.end133:                                       ; preds = %for.inc131
  br label %do.cond

do.cond:                                          ; preds = %for.end133
  %1 = load i32, ptr %b, align 4
  %tobool = icmp ne i32 %1, 0
  br i1 %tobool, label %do.body, label %do.end

do.end:                                           ; preds = %do.cond
  ret void
}

attributes #0 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="corei7-avx" "target-features"="+avx,+cx16,+cx8,+fxsr,+mmx,+pclmul,+popcnt,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++ Compiler Pro 2021.1 (YYYY.x.0.MMDD)"}
