; This test checks that the graph building for the outermost level produces the same edges as building for inner loops first and then outer loop.

; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-dd-analysis -hir-dd-analysis-verify=Innermost,L1|sort > %t1.out
; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-dd-analysis -hir-dd-analysis-verify=L1|sort > %t2.out
; RUN: diff %t1.out %t2.out

; Most interesting part of the test HIR
; BEGIN REGION { }
;  + DO i1 = 0, 19, 1   <DO_LOOP>
;   |   %hir.de.ssa.copy0.out = -1 * i1 + 21;
;   |   %23 = (%mc)[0][-1 * i1 + 20];
;   |   %24 = (%mc)[0][-1 * i1 + 21];
;   |   (%mc)[0][-1 * i1 + 21] = ((%23 + %22) * %24);
;   |   + DO i2 = 0, 16, 1   <DO_LOOP>
;   |   |   %25 = (%x)[0][76][54][13];
;   |   |   %26 = (%a)[0][i2 + 1];
;   |   |   (%a)[0][i2 + 1] = %25 + %26;
;   |   |   %27 = (%x)[0][0][2][i2 + 2];
;   |   |   (%x)[0][82][57][59] = %27 + -35;
;   |   |   %29 = (%d)[0][i2];
;   |   |   (%d)[0][i2] = ((-35 + %27) * %29);
;   |   + END LOOP
;   |   + DO i2 = 0, 16, 1   <DO_LOOP>
;   |   |   %40 = (%x)[0][76][54][13];
;   |   |   %41 = (%a)[0][i2 + 1];
;   |   |   (%a)[0][i2 + 1] = %40 + %41;
;   |   |   %42 = (%x)[0][1][3][i2 + 2];
;   |   |   (%x)[0][82][57][59] = %42 + -36;
;   |   |   %44 = (%d)[0][i2];
;   |   |   (%d)[0][i2] = ((-36 + %42) * %44);
;   |   + END LOOP
;   |   + DO i2 = 0, 16, 1   <DO_LOOP>
;   |   |   %45 = (%x)[0][76][54][13];
;   |   |   %46 = (%a)[0][i2 + 1];
;   |   |   (%a)[0][i2 + 1] = %45 + %46;
;   |   |   %47 = (%x)[0][2][4][i2 + 2];
;   |   |   (%x)[0][82][57][59] = %47 + -37;
;   |   |   %49 = (%d)[0][i2];
;   |   |   (%d)[0][i2] = ((-37 + %47) * %49);
;   |   + END LOOP
;   |   + DO i2 = 0, 16, 1   <DO_LOOP>
;   |   |   %50 = (%x)[0][76][54][13];
;   |   |   %51 = (%a)[0][i2 + 1];
;   |   |   (%a)[0][i2 + 1] = %50 + %51;
;   |   |   %52 = (%x)[0][3][5][i2 + 2];
;   |   |   (%x)[0][82][57][59] = %52 + -38;
;   |   |   %54 = (%d)[0][i2];
;   |   |   (%d)[0][i2] = ((-38 + %52) * %54);
;   |   + END LOOP
;   |   + DO i2 = 0, 16, 1   <DO_LOOP>
;   |   |   %55 = (%x)[0][76][54][13];
;   |   |   %56 = (%a)[0][i2 + 1];
;   |   |   (%a)[0][i2 + 1] = %55 + %56;
;   |   |   %57 = (%x)[0][4][6][i2 + 2];
;   |   |   (%x)[0][82][57][59] = %57 + -39;
;   |   |   %59 = (%d)[0][i2];
;   |   |   (%d)[0][i2] = ((-39 + %57) * %59);
;   |   + END LOOP
;   |   %22 = -1 * i1 + 20;
;   + END LOOP
; END REGION

; ModuleID = 'module.ll'
source_filename = "t167.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct._IO_FILE = type { i32, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, %struct._IO_marker*, %struct._IO_FILE*, i32, i32, i64, i16, i8, [1 x i8], i8*, i64, i8*, i8*, i8*, i8*, i64, i32, [20 x i8] }
%struct._IO_marker = type { %struct._IO_marker*, %struct._IO_FILE*, i32 }

@.str = external hidden unnamed_addr constant [10 x i8], align 1
@.str.1 = external hidden unnamed_addr constant [2 x i8], align 1
@.str.2 = external hidden unnamed_addr constant [18 x i8], align 1
@.str.3 = external hidden unnamed_addr constant [10 x i8], align 1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #0

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #0

; Function Attrs: nounwind uwtable
define i32 @main() local_unnamed_addr #1 {
entry:
  %ku = alloca i32, align 4
  %jy0 = alloca i32, align 4
  %jg4 = alloca i32, align 4
  %k = alloca i32, align 4
  %ji = alloca i32, align 4
  %ux = alloca i32, align 4
  %x = alloca [100 x [100 x [100 x i32]]], align 16
  %mc = alloca [100 x i32], align 16
  %a = alloca [100 x i32], align 16
  %d = alloca [100 x i32], align 16
  %call = tail call %struct._IO_FILE* @fopen(i8* getelementptr inbounds ([10 x i8], [10 x i8]* @.str, i64 0, i64 0), i8* getelementptr inbounds ([2 x i8], [2 x i8]* @.str.1, i64 0, i64 0))
  %0 = bitcast i32* %ku to i8*
  call void @llvm.lifetime.start(i64 4, i8* %0) #4
  store i32 87, i32* %ku, align 4, !tbaa !1
  %1 = bitcast i32* %jy0 to i8*
  call void @llvm.lifetime.start(i64 4, i8* %1) #4
  store i32 57, i32* %jy0, align 4, !tbaa !1
  %2 = bitcast i32* %jg4 to i8*
  call void @llvm.lifetime.start(i64 4, i8* %2) #4
  store i32 97, i32* %jg4, align 4, !tbaa !1
  %3 = bitcast i32* %k to i8*
  call void @llvm.lifetime.start(i64 4, i8* %3) #4
  store i32 95, i32* %k, align 4, !tbaa !1
  %4 = bitcast i32* %ji to i8*
  call void @llvm.lifetime.start(i64 4, i8* %4) #4
  store i32 1, i32* %ji, align 4, !tbaa !1
  %5 = bitcast i32* %ux to i8*
  call void @llvm.lifetime.start(i64 4, i8* %5) #4
  store i32 30, i32* %ux, align 4, !tbaa !1
  %6 = bitcast [100 x [100 x [100 x i32]]]* %x to i8*
  call void @llvm.lifetime.start(i64 4000000, i8* %6) #4
  %7 = bitcast [100 x i32]* %mc to i8*
  call void @llvm.lifetime.start(i64 400, i8* %7) #4
  %8 = bitcast [100 x i32]* %a to i8*
  call void @llvm.lifetime.start(i64 400, i8* %8) #4
  %9 = bitcast [100 x i32]* %d to i8*
  call void @llvm.lifetime.start(i64 400, i8* %9) #4
  %arrayidx2 = getelementptr inbounds [100 x [100 x [100 x i32]]], [100 x [100 x [100 x i32]]]* %x, i64 0, i64 76, i64 54, i64 13
  br label %for.body.i

for.body.i:                                       ; preds = %for.body.i, %entry
  %indvars.iv.i = phi i64 [ %indvars.iv.next.i, %for.body.i ], [ 0, %entry ]
  %10 = trunc i64 %indvars.iv.i to i32
  %rem.i = and i32 %10, 1
  %cmp1.i = icmp eq i32 %rem.i, 0
  %11 = sub i32 0, %10
  %.p.i = select i1 %cmp1.i, i32 %10, i32 %11
  %12 = add i32 %.p.i, 39
  %rem2.i = urem i32 %12, 101
  %arrayidx.i = getelementptr inbounds [100 x [100 x [100 x i32]]], [100 x [100 x [100 x i32]]]* %x, i64 0, i64 0, i64 0, i64 %indvars.iv.i
  store i32 %rem2.i, i32* %arrayidx.i, align 4, !tbaa !1
  %indvars.iv.next.i = add nuw nsw i64 %indvars.iv.i, 1
  %exitcond216 = icmp eq i64 %indvars.iv.next.i, 1000000
  br i1 %exitcond216, label %for.body.i173.preheader, label %for.body.i

for.body.i173.preheader:                          ; preds = %for.body.i
  %arrayidx5 = getelementptr inbounds [100 x [100 x [100 x i32]]], [100 x [100 x [100 x i32]]]* %x, i64 0, i64 82, i64 57, i64 59
  br label %for.body.i173

for.body.i173:                                    ; preds = %for.body.i173, %for.body.i173.preheader
  %indvars.iv.i164 = phi i64 [ %indvars.iv.next.i170, %for.body.i173 ], [ 0, %for.body.i173.preheader ]
  %13 = trunc i64 %indvars.iv.i164 to i32
  %rem.i165 = and i32 %13, 1
  %cmp1.i166 = icmp eq i32 %rem.i165, 0
  %14 = sub i32 0, %13
  %.p.i167 = select i1 %cmp1.i166, i32 %13, i32 %14
  %15 = add i32 %.p.i167, 89
  %rem2.i168 = urem i32 %15, 101
  %arrayidx.i169 = getelementptr inbounds [100 x i32], [100 x i32]* %mc, i64 0, i64 %indvars.iv.i164
  store i32 %rem2.i168, i32* %arrayidx.i169, align 4, !tbaa !1
  %indvars.iv.next.i170 = add nuw nsw i64 %indvars.iv.i164, 1
  %exitcond215 = icmp eq i64 %indvars.iv.next.i170, 100
  br i1 %exitcond215, label %for.body.i162.preheader, label %for.body.i173

for.body.i162.preheader:                          ; preds = %for.body.i173
  br label %for.body.i162

for.body.i162:                                    ; preds = %for.body.i162, %for.body.i162.preheader
  %indvars.iv.i153 = phi i64 [ %indvars.iv.next.i159, %for.body.i162 ], [ 0, %for.body.i162.preheader ]
  %16 = trunc i64 %indvars.iv.i153 to i32
  %rem.i154 = and i32 %16, 1
  %cmp1.i155 = icmp eq i32 %rem.i154, 0
  %17 = sub i32 0, %16
  %.p.i156 = select i1 %cmp1.i155, i32 %16, i32 %17
  %18 = add i32 %.p.i156, 78
  %rem2.i157 = urem i32 %18, 101
  %arrayidx.i158 = getelementptr inbounds [100 x i32], [100 x i32]* %a, i64 0, i64 %indvars.iv.i153
  store i32 %rem2.i157, i32* %arrayidx.i158, align 4, !tbaa !1
  %indvars.iv.next.i159 = add nuw nsw i64 %indvars.iv.i153, 1
  %exitcond214 = icmp eq i64 %indvars.iv.next.i159, 100
  br i1 %exitcond214, label %for.body.i151.preheader, label %for.body.i162

for.body.i151.preheader:                          ; preds = %for.body.i162
  br label %for.body.i151

for.body.i151:                                    ; preds = %for.body.i151, %for.body.i151.preheader
  %indvars.iv.i142 = phi i64 [ %indvars.iv.next.i148, %for.body.i151 ], [ 0, %for.body.i151.preheader ]
  %19 = trunc i64 %indvars.iv.i142 to i32
  %rem.i143 = and i32 %19, 1
  %cmp1.i144 = icmp eq i32 %rem.i143, 0
  %20 = sub i32 0, %19
  %.p.i145 = select i1 %cmp1.i144, i32 %19, i32 %20
  %21 = add i32 %.p.i145, 29
  %rem2.i146 = urem i32 %21, 101
  %arrayidx.i147 = getelementptr inbounds [100 x i32], [100 x i32]* %d, i64 0, i64 %indvars.iv.i142
  store i32 %rem2.i146, i32* %arrayidx.i147, align 4, !tbaa !1
  %indvars.iv.next.i148 = add nuw nsw i64 %indvars.iv.i142, 1
  %exitcond213 = icmp eq i64 %indvars.iv.next.i148, 100
  br i1 %exitcond213, label %init.exit152, label %for.body.i151

init.exit152:                                     ; preds = %for.body.i151
  %call9 = call i32 (%struct._IO_FILE*, i8*, ...) @__isoc99_fscanf(%struct._IO_FILE* %call, i8* getelementptr inbounds ([18 x i8], [18 x i8]* @.str.2, i64 0, i64 0), i32* nonnull %ku, i32* nonnull %jy0, i32* nonnull %jg4, i32* nonnull %k, i32* nonnull %ji, i32* nonnull %ux) #4
  store i32 21, i32* %ku, align 4, !tbaa !1
  br label %for.end23

for.end23:                                        ; preds = %for.inc68.4, %init.exit152
  %indvars.iv209 = phi i64 [ 21, %init.exit152 ], [ %.pre, %for.inc68.4 ]
  %indvars.iv199 = phi i32 [ -22, %init.exit152 ], [ %indvars.iv.next200, %for.inc68.4 ]
  %22 = phi i32 [ 21, %init.exit152 ], [ %dec72, %for.inc68.4 ]
  %.pre = add nsw i64 %indvars.iv209, -1
  %arrayidx26 = getelementptr inbounds [100 x i32], [100 x i32]* %mc, i64 0, i64 %.pre
  %23 = load i32, i32* %arrayidx26, align 4, !tbaa !5
  %add27 = add i32 %23, %22
  %arrayidx29 = getelementptr inbounds [100 x i32], [100 x i32]* %mc, i64 0, i64 %indvars.iv209
  %24 = load i32, i32* %arrayidx29, align 4, !tbaa !5
  %mul = mul i32 %24, %add27
  store i32 %mul, i32* %arrayidx29, align 4, !tbaa !5
  br label %for.body35

for.body35:                                       ; preds = %for.body35, %for.end23
  %indvars.iv201 = phi i64 [ 1, %for.end23 ], [ %indvars.iv.next202, %for.body35 ]
  %25 = load i32, i32* %arrayidx2, align 4, !tbaa !1
  %arrayidx37 = getelementptr inbounds [100 x i32], [100 x i32]* %a, i64 0, i64 %indvars.iv201
  %26 = load i32, i32* %arrayidx37, align 4, !tbaa !5
  %add38 = add i32 %26, %25
  store i32 %add38, i32* %arrayidx37, align 4, !tbaa !5
  %indvars.iv.next202 = add nuw nsw i64 %indvars.iv201, 1
  %arrayidx47 = getelementptr inbounds [100 x [100 x [100 x i32]]], [100 x [100 x [100 x i32]]]* %x, i64 0, i64 0, i64 2, i64 %indvars.iv.next202
  %27 = load i32, i32* %arrayidx47, align 4, !tbaa !7
  %sub52 = add i32 %27, -35
  store i32 %sub52, i32* %arrayidx5, align 4, !tbaa !1
  %28 = add nsw i64 %indvars.iv201, -1
  %arrayidx60 = getelementptr inbounds [100 x i32], [100 x i32]* %d, i64 0, i64 %28
  %29 = load i32, i32* %arrayidx60, align 4, !tbaa !5
  %mul61 = mul i32 %29, %sub52
  store i32 %mul61, i32* %arrayidx60, align 4, !tbaa !5
  %exitcond204 = icmp eq i64 %indvars.iv.next202, 18
  br i1 %exitcond204, label %for.body35.1.preheader, label %for.body35

for.body35.1.preheader:                           ; preds = %for.body35
  br label %for.body35.1

for.end73:                                        ; preds = %for.inc68.4
  %.lcssa = phi i32 [ %22, %for.inc68.4 ]
  %30 = load i32, i32* %jg4, align 4, !tbaa !1
  %add53 = add i32 %30, 102
  %arrayidx63 = getelementptr inbounds [100 x i32], [100 x i32]* %a, i64 0, i64 17
  %31 = load i32, i32* %arrayidx63, align 4, !tbaa !5
  %sub64 = sub i32 %add53, %31
  store i32 1, i32* %ku, align 4, !tbaa !1
  store i32 %.lcssa, i32* %jy0, align 4, !tbaa !1
  store i32 18, i32* %ji, align 4, !tbaa !1
  store i32 6, i32* %k, align 4, !tbaa !1
  store i32 %sub64, i32* %ux, align 4, !tbaa !1
  br label %for.body.i140

for.body.i140:                                    ; preds = %for.body.i140, %for.end73
  %indvars.iv.i130 = phi i64 [ %indvars.iv.next.i137, %for.body.i140 ], [ 0, %for.end73 ]
  %sum.012.i131 = phi i32 [ %add.i136, %for.body.i140 ], [ 0, %for.end73 ]
  %rem.i132 = and i64 %indvars.iv.i130, 1
  %cmp1.i133 = icmp eq i64 %rem.i132, 0
  %arrayidx.i134 = getelementptr inbounds [100 x [100 x [100 x i32]]], [100 x [100 x [100 x i32]]]* %x, i64 0, i64 0, i64 0, i64 %indvars.iv.i130
  %32 = load i32, i32* %arrayidx.i134, align 4, !tbaa !1
  %sub.i135 = sub i32 0, %32
  %33 = select i1 %cmp1.i133, i32 %32, i32 %sub.i135
  %add.i136 = add i32 %33, %sum.012.i131
  %indvars.iv.next.i137 = add nuw nsw i64 %indvars.iv.i130, 1
  %exitcond191 = icmp eq i64 %indvars.iv.next.i137, 1000000
  br i1 %exitcond191, label %for.body.i128.preheader, label %for.body.i140

for.body.i128.preheader:                          ; preds = %for.body.i140
  %add.i136.lcssa = phi i32 [ %add.i136, %for.body.i140 ]
  br label %for.body.i128

for.body.i128:                                    ; preds = %for.body.i128, %for.body.i128.preheader
  %indvars.iv.i118 = phi i64 [ %indvars.iv.next.i125, %for.body.i128 ], [ 0, %for.body.i128.preheader ]
  %sum.012.i119 = phi i32 [ %add.i124, %for.body.i128 ], [ 0, %for.body.i128.preheader ]
  %rem.i120 = and i64 %indvars.iv.i118, 1
  %cmp1.i121 = icmp eq i64 %rem.i120, 0
  %arrayidx.i122 = getelementptr inbounds [100 x i32], [100 x i32]* %mc, i64 0, i64 %indvars.iv.i118
  %34 = load i32, i32* %arrayidx.i122, align 4, !tbaa !1
  %sub.i123 = sub i32 0, %34
  %35 = select i1 %cmp1.i121, i32 %34, i32 %sub.i123
  %add.i124 = add i32 %35, %sum.012.i119
  %indvars.iv.next.i125 = add nuw nsw i64 %indvars.iv.i118, 1
  %exitcond190 = icmp eq i64 %indvars.iv.next.i125, 100
  br i1 %exitcond190, label %for.body.i116.preheader, label %for.body.i128

for.body.i116.preheader:                          ; preds = %for.body.i128
  %add.i124.lcssa = phi i32 [ %add.i124, %for.body.i128 ]
  br label %for.body.i116

for.body.i116:                                    ; preds = %for.body.i116, %for.body.i116.preheader
  %indvars.iv.i106 = phi i64 [ %indvars.iv.next.i113, %for.body.i116 ], [ 0, %for.body.i116.preheader ]
  %sum.012.i107 = phi i32 [ %add.i112, %for.body.i116 ], [ 0, %for.body.i116.preheader ]
  %rem.i108 = and i64 %indvars.iv.i106, 1
  %cmp1.i109 = icmp eq i64 %rem.i108, 0
  %arrayidx.i110 = getelementptr inbounds [100 x i32], [100 x i32]* %a, i64 0, i64 %indvars.iv.i106
  %36 = load i32, i32* %arrayidx.i110, align 4, !tbaa !1
  %sub.i111 = sub i32 0, %36
  %37 = select i1 %cmp1.i109, i32 %36, i32 %sub.i111
  %add.i112 = add i32 %37, %sum.012.i107
  %indvars.iv.next.i113 = add nuw nsw i64 %indvars.iv.i106, 1
  %exitcond189 = icmp eq i64 %indvars.iv.next.i113, 100
  br i1 %exitcond189, label %for.body.i105.preheader, label %for.body.i116

for.body.i105.preheader:                          ; preds = %for.body.i116
  %add.i112.lcssa = phi i32 [ %add.i112, %for.body.i116 ]
  br label %for.body.i105

for.body.i105:                                    ; preds = %for.body.i105, %for.body.i105.preheader
  %indvars.iv.i98 = phi i64 [ %indvars.iv.next.i102, %for.body.i105 ], [ 0, %for.body.i105.preheader ]
  %sum.012.i = phi i32 [ %add.i, %for.body.i105 ], [ 0, %for.body.i105.preheader ]
  %rem.i99 = and i64 %indvars.iv.i98, 1
  %cmp1.i100 = icmp eq i64 %rem.i99, 0
  %arrayidx.i101 = getelementptr inbounds [100 x i32], [100 x i32]* %d, i64 0, i64 %indvars.iv.i98
  %38 = load i32, i32* %arrayidx.i101, align 4, !tbaa !1
  %sub.i = sub i32 0, %38
  %39 = select i1 %cmp1.i100, i32 %38, i32 %sub.i
  %add.i = add i32 %39, %sum.012.i
  %indvars.iv.next.i102 = add nuw nsw i64 %indvars.iv.i98, 1
  %exitcond = icmp eq i64 %indvars.iv.next.i102, 100
  br i1 %exitcond, label %checkSum.exit, label %for.body.i105

checkSum.exit:                                    ; preds = %for.body.i105
  %add.i.lcssa = phi i32 [ %add.i, %for.body.i105 ]
  %add78 = add i32 %add.i136.lcssa, %sub64
  %sub81 = add i32 %add78, %add.i124.lcssa
  %add84 = sub i32 %sub81, %add.i112.lcssa
  %add85 = add i32 %add84, %add.i.lcssa
  %call86 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([10 x i8], [10 x i8]* @.str.3, i64 0, i64 0), i32 %add85)
  call void @llvm.lifetime.end(i64 400, i8* nonnull %9) #4
  call void @llvm.lifetime.end(i64 400, i8* %8) #4
  call void @llvm.lifetime.end(i64 400, i8* %7) #4
  call void @llvm.lifetime.end(i64 4000000, i8* %6) #4
  call void @llvm.lifetime.end(i64 4, i8* %5) #4
  call void @llvm.lifetime.end(i64 4, i8* %4) #4
  call void @llvm.lifetime.end(i64 4, i8* %3) #4
  call void @llvm.lifetime.end(i64 4, i8* %2) #4
  call void @llvm.lifetime.end(i64 4, i8* %1) #4
  call void @llvm.lifetime.end(i64 4, i8* %0) #4
  ret i32 0

for.body35.1:                                     ; preds = %for.body35.1, %for.body35.1.preheader
  %indvars.iv201.1 = phi i64 [ %indvars.iv.next202.1, %for.body35.1 ], [ 1, %for.body35.1.preheader ]
  %40 = load i32, i32* %arrayidx2, align 4, !tbaa !1
  %arrayidx37.1 = getelementptr inbounds [100 x i32], [100 x i32]* %a, i64 0, i64 %indvars.iv201.1
  %41 = load i32, i32* %arrayidx37.1, align 4, !tbaa !5
  %add38.1 = add i32 %41, %40
  store i32 %add38.1, i32* %arrayidx37.1, align 4, !tbaa !5
  %indvars.iv.next202.1 = add nuw nsw i64 %indvars.iv201.1, 1
  %arrayidx47.1 = getelementptr inbounds [100 x [100 x [100 x i32]]], [100 x [100 x [100 x i32]]]* %x, i64 0, i64 1, i64 3, i64 %indvars.iv.next202.1
  %42 = load i32, i32* %arrayidx47.1, align 4, !tbaa !7
  %sub52.1 = add i32 %42, -36
  store i32 %sub52.1, i32* %arrayidx5, align 4, !tbaa !1
  %43 = add nsw i64 %indvars.iv201.1, -1
  %arrayidx60.1 = getelementptr inbounds [100 x i32], [100 x i32]* %d, i64 0, i64 %43
  %44 = load i32, i32* %arrayidx60.1, align 4, !tbaa !5
  %mul61.1 = mul i32 %44, %sub52.1
  store i32 %mul61.1, i32* %arrayidx60.1, align 4, !tbaa !5
  %exitcond204.1 = icmp eq i64 %indvars.iv.next202.1, 18
  br i1 %exitcond204.1, label %for.body35.2.preheader, label %for.body35.1

for.body35.2.preheader:                           ; preds = %for.body35.1
  br label %for.body35.2

for.body35.2:                                     ; preds = %for.body35.2, %for.body35.2.preheader
  %indvars.iv201.2 = phi i64 [ %indvars.iv.next202.2, %for.body35.2 ], [ 1, %for.body35.2.preheader ]
  %45 = load i32, i32* %arrayidx2, align 4, !tbaa !1
  %arrayidx37.2 = getelementptr inbounds [100 x i32], [100 x i32]* %a, i64 0, i64 %indvars.iv201.2
  %46 = load i32, i32* %arrayidx37.2, align 4, !tbaa !5
  %add38.2 = add i32 %46, %45
  store i32 %add38.2, i32* %arrayidx37.2, align 4, !tbaa !5
  %indvars.iv.next202.2 = add nuw nsw i64 %indvars.iv201.2, 1
  %arrayidx47.2 = getelementptr inbounds [100 x [100 x [100 x i32]]], [100 x [100 x [100 x i32]]]* %x, i64 0, i64 2, i64 4, i64 %indvars.iv.next202.2
  %47 = load i32, i32* %arrayidx47.2, align 4, !tbaa !7
  %sub52.2 = add i32 %47, -37
  store i32 %sub52.2, i32* %arrayidx5, align 4, !tbaa !1
  %48 = add nsw i64 %indvars.iv201.2, -1
  %arrayidx60.2 = getelementptr inbounds [100 x i32], [100 x i32]* %d, i64 0, i64 %48
  %49 = load i32, i32* %arrayidx60.2, align 4, !tbaa !5
  %mul61.2 = mul i32 %49, %sub52.2
  store i32 %mul61.2, i32* %arrayidx60.2, align 4, !tbaa !5
  %exitcond204.2 = icmp eq i64 %indvars.iv.next202.2, 18
  br i1 %exitcond204.2, label %for.body35.3.preheader, label %for.body35.2

for.body35.3.preheader:                           ; preds = %for.body35.2
  br label %for.body35.3

for.body35.3:                                     ; preds = %for.body35.3, %for.body35.3.preheader
  %indvars.iv201.3 = phi i64 [ %indvars.iv.next202.3, %for.body35.3 ], [ 1, %for.body35.3.preheader ]
  %50 = load i32, i32* %arrayidx2, align 4, !tbaa !1
  %arrayidx37.3 = getelementptr inbounds [100 x i32], [100 x i32]* %a, i64 0, i64 %indvars.iv201.3
  %51 = load i32, i32* %arrayidx37.3, align 4, !tbaa !5
  %add38.3 = add i32 %51, %50
  store i32 %add38.3, i32* %arrayidx37.3, align 4, !tbaa !5
  %indvars.iv.next202.3 = add nuw nsw i64 %indvars.iv201.3, 1
  %arrayidx47.3 = getelementptr inbounds [100 x [100 x [100 x i32]]], [100 x [100 x [100 x i32]]]* %x, i64 0, i64 3, i64 5, i64 %indvars.iv.next202.3
  %52 = load i32, i32* %arrayidx47.3, align 4, !tbaa !7
  %sub52.3 = add i32 %52, -38
  store i32 %sub52.3, i32* %arrayidx5, align 4, !tbaa !1
  %53 = add nsw i64 %indvars.iv201.3, -1
  %arrayidx60.3 = getelementptr inbounds [100 x i32], [100 x i32]* %d, i64 0, i64 %53
  %54 = load i32, i32* %arrayidx60.3, align 4, !tbaa !5
  %mul61.3 = mul i32 %54, %sub52.3
  store i32 %mul61.3, i32* %arrayidx60.3, align 4, !tbaa !5
  %exitcond204.3 = icmp eq i64 %indvars.iv.next202.3, 18
  br i1 %exitcond204.3, label %for.body35.4.preheader, label %for.body35.3

for.body35.4.preheader:                           ; preds = %for.body35.3
  br label %for.body35.4

for.body35.4:                                     ; preds = %for.body35.4, %for.body35.4.preheader
  %indvars.iv201.4 = phi i64 [ %indvars.iv.next202.4, %for.body35.4 ], [ 1, %for.body35.4.preheader ]
  %55 = load i32, i32* %arrayidx2, align 4, !tbaa !1
  %arrayidx37.4 = getelementptr inbounds [100 x i32], [100 x i32]* %a, i64 0, i64 %indvars.iv201.4
  %56 = load i32, i32* %arrayidx37.4, align 4, !tbaa !5
  %add38.4 = add i32 %56, %55
  store i32 %add38.4, i32* %arrayidx37.4, align 4, !tbaa !5
  %indvars.iv.next202.4 = add nuw nsw i64 %indvars.iv201.4, 1
  %arrayidx47.4 = getelementptr inbounds [100 x [100 x [100 x i32]]], [100 x [100 x [100 x i32]]]* %x, i64 0, i64 4, i64 6, i64 %indvars.iv.next202.4
  %57 = load i32, i32* %arrayidx47.4, align 4, !tbaa !7
  %sub52.4 = add i32 %57, -39
  store i32 %sub52.4, i32* %arrayidx5, align 4, !tbaa !1
  %58 = add nsw i64 %indvars.iv201.4, -1
  %arrayidx60.4 = getelementptr inbounds [100 x i32], [100 x i32]* %d, i64 0, i64 %58
  %59 = load i32, i32* %arrayidx60.4, align 4, !tbaa !5
  %mul61.4 = mul i32 %59, %sub52.4
  store i32 %mul61.4, i32* %arrayidx60.4, align 4, !tbaa !5
  %exitcond204.4 = icmp eq i64 %indvars.iv.next202.4, 18
  br i1 %exitcond204.4, label %for.inc68.4, label %for.body35.4

for.inc68.4:                                      ; preds = %for.body35.4
  %dec72 = add nsw i32 %22, -1
  %indvars.iv.next200 = add nuw nsw i32 %indvars.iv199, 1
  %exitcond212 = icmp eq i32 %indvars.iv.next200, -2
  br i1 %exitcond212, label %for.end73, label %for.end23
}

; Function Attrs: nounwind
declare noalias %struct._IO_FILE* @fopen(i8* nocapture readonly, i8* nocapture readonly) local_unnamed_addr #2

declare i32 @__isoc99_fscanf(%struct._IO_FILE*, i8*, ...) local_unnamed_addr #3

; Function Attrs: nounwind
declare i32 @printf(i8* nocapture readonly, ...) local_unnamed_addr #2

attributes #0 = { argmemonly nounwind }
attributes #1 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.9.0 (trunk 15884) (llvm/branches/loopopt 16227)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
!5 = !{!6, !2, i64 0}
!6 = !{!"array@_ZTSA100_j", !2, i64 0}
!7 = !{!8, !2, i64 0}
!8 = !{!"array@_ZTSA100_A100_A100_j", !9, i64 0}
!9 = !{!"array@_ZTSA100_A100_j", !6, i64 0}
