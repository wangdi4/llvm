; Make sure points-to info of %arrayidx39 is invalidated in Andersens's
; dump after SimplifyCFG does sinking transformation.
; %arrayidx35 and %arrayidx39 are sunk in SimplifyCFG by creating new
; PHI node like below:
;  %k4.sink = phi ptr [ %k4, %if.else ], [ %kr, %if.then20 ]

; REQUIRES: asserts

; RUN: opt < %s  -passes='require<anders-aa>,simplifycfg,print-alias-sets' -sink-common-insts=1 -invalidate-anders-res=1 -print-anders-points-to-updates -disable-output 2>&1 | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Makes sure points-to info of %k4.sink is invalidated.
; CHECK: Invalidated PHI user:   %arrayidx39 = getelementptr inbounds [100 x i32], ptr %k4.sink, i64 0, i64 %30

; Makes sure both %k and %k4 are in same AliasSet.
; CHECK: Alias sets for function 'main':
; CHECK:   AliasSet{{.*}}may alias, Mod/Ref Pointers: {{.*}}%k{{.*}}%k4{{.*}}

@.str = private unnamed_addr constant [15 x i8] c"%u %u %u %u %u\00", align 1
@.str.1 = private unnamed_addr constant [10 x i8] c"res = %u\0A\00", align 1

; Function Attrs: nofree norecurse nosync nounwind memory(argmem: write) uwtable
declare dso_local void @init(ptr nocapture noundef writeonly %a, i32 noundef %n, i32 noundef %seed) local_unnamed_addr #0

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg %0, ptr nocapture %1) #1

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg %0, ptr nocapture %1) #1

; Function Attrs: nofree norecurse nosync nounwind memory(argmem: read) uwtable
declare dso_local i32 @checkSum(ptr nocapture noundef readonly %a, i32 noundef %n) local_unnamed_addr #2

; Function Attrs: nofree nounwind uwtable
define dso_local i32 @main() local_unnamed_addr #3 {
entry:
  %k = alloca i32, align 4
  %n4 = alloca i32, align 4
  %nn = alloca i32, align 4
  %n = alloca i32, align 4
  %a = alloca i32, align 4
  %w = alloca [100 x i32], align 16
  %n0 = alloca [100 x i32], align 16
  %zs = alloca [100 x i32], align 16
  %kr = alloca [100 x i32], align 16
  %zp9 = alloca [100 x i32], align 16
  %aj9 = alloca [100 x i32], align 16
  %k4 = alloca [100 x i32], align 16
  %h1 = alloca [100 x i32], align 16
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %k) #6
  store i32 75, ptr %k, align 4, !tbaa !3
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %n4) #6
  store i32 74, ptr %n4, align 4, !tbaa !3
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %nn) #6
  store i32 99, ptr %nn, align 4, !tbaa !3
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %n) #6
  store i32 52, ptr %n, align 4, !tbaa !3
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %a) #6
  store i32 69, ptr %a, align 4, !tbaa !3
  call void @llvm.lifetime.start.p0(i64 400, ptr nonnull %w) #6
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 16 dereferenceable(400) %w, i8 0, i64 400, i1 false)
  call void @llvm.lifetime.start.p0(i64 400, ptr nonnull %n0) #6
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 16 dereferenceable(400) %n0, i8 0, i64 400, i1 false)
  call void @llvm.lifetime.start.p0(i64 400, ptr nonnull %zs) #6
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 16 dereferenceable(400) %zs, i8 0, i64 400, i1 false)
  call void @llvm.lifetime.start.p0(i64 400, ptr nonnull %kr) #6
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 16 dereferenceable(400) %kr, i8 0, i64 400, i1 false)
  call void @llvm.lifetime.start.p0(i64 400, ptr nonnull %zp9) #6
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 16 dereferenceable(400) %zp9, i8 0, i64 400, i1 false)
  call void @llvm.lifetime.start.p0(i64 400, ptr nonnull %aj9) #6
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 16 dereferenceable(400) %aj9, i8 0, i64 400, i1 false)
  call void @llvm.lifetime.start.p0(i64 400, ptr nonnull %k4) #6
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 16 dereferenceable(400) %k4, i8 0, i64 400, i1 false)
  call void @llvm.lifetime.start.p0(i64 400, ptr nonnull %h1) #6
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 16 dereferenceable(400) %h1, i8 0, i64 400, i1 false)
  %arrayidx = getelementptr inbounds [100 x i32], ptr %zs, i64 0, i64 57, !intel-tbaa !11
  br label %for.body.i

for.body.i:                                       ; preds = %for.body.i, %entry
  %indvars.iv.i = phi i64 [ 0, %entry ], [ %indvars.iv.next.i, %for.body.i ]
  %rem12.i = and i64 %indvars.iv.i, 1
  %cmp1.i = icmp eq i64 %rem12.i, 0
  %0 = trunc i64 %indvars.iv.i to i32
  %1 = sub i32 0, %0
  %.p.i = select i1 %cmp1.i, i32 %0, i32 %1
  %2 = add i32 %.p.i, 9
  %rem2.i = urem i32 %2, 101
  %arrayidx.i = getelementptr inbounds [100 x i32], ptr %w, i64 0, i64 %indvars.iv.i
  store i32 %rem2.i, ptr %arrayidx.i, align 4, !tbaa !3
  %indvars.iv.next.i = add nuw nsw i64 %indvars.iv.i, 1
  %exitcond.not.i = icmp eq i64 %indvars.iv.next.i, 100
  br i1 %exitcond.not.i, label %for.body.i98.preheader, label %for.body.i, !llvm.loop !7

for.body.i98.preheader:                           ; preds = %for.body.i
  br label %for.body.i98

for.body.i98:                                     ; preds = %for.body.i98.preheader, %for.body.i98
  %indvars.iv.i99 = phi i64 [ %indvars.iv.next.i105, %for.body.i98 ], [ 0, %for.body.i98.preheader ]
  %rem12.i100 = and i64 %indvars.iv.i99, 1
  %cmp1.i101 = icmp eq i64 %rem12.i100, 0
  %3 = trunc i64 %indvars.iv.i99 to i32
  %4 = sub i32 0, %3
  %.p.i102 = select i1 %cmp1.i101, i32 %3, i32 %4
  %5 = add i32 %.p.i102, 84
  %rem2.i103 = urem i32 %5, 101
  %arrayidx.i104 = getelementptr inbounds [100 x i32], ptr %n0, i64 0, i64 %indvars.iv.i99
  store i32 %rem2.i103, ptr %arrayidx.i104, align 4, !tbaa !3
  %indvars.iv.next.i105 = add nuw nsw i64 %indvars.iv.i99, 1
  %exitcond.not.i106 = icmp eq i64 %indvars.iv.next.i105, 100
  br i1 %exitcond.not.i106, label %for.body.i108.preheader, label %for.body.i98, !llvm.loop !7

for.body.i108.preheader:                          ; preds = %for.body.i98
  br label %for.body.i108

for.body.i108:                                    ; preds = %for.body.i108.preheader, %for.body.i108
  %indvars.iv.i109 = phi i64 [ %indvars.iv.next.i115, %for.body.i108 ], [ 0, %for.body.i108.preheader ]
  %rem12.i110 = and i64 %indvars.iv.i109, 1
  %cmp1.i111 = icmp eq i64 %rem12.i110, 0
  %6 = trunc i64 %indvars.iv.i109 to i32
  %7 = sub i32 0, %6
  %.p.i112 = select i1 %cmp1.i111, i32 %6, i32 %7
  %8 = add i32 %.p.i112, 88
  %rem2.i113 = urem i32 %8, 101
  %arrayidx.i114 = getelementptr inbounds [100 x i32], ptr %zs, i64 0, i64 %indvars.iv.i109
  store i32 %rem2.i113, ptr %arrayidx.i114, align 4, !tbaa !3
  %indvars.iv.next.i115 = add nuw nsw i64 %indvars.iv.i109, 1
  %exitcond.not.i116 = icmp eq i64 %indvars.iv.next.i115, 100
  br i1 %exitcond.not.i116, label %for.body.i118.preheader, label %for.body.i108, !llvm.loop !7

for.body.i118.preheader:                          ; preds = %for.body.i108
  br label %for.body.i118

for.body.i118:                                    ; preds = %for.body.i118.preheader, %for.body.i118
  %indvars.iv.i119 = phi i64 [ %indvars.iv.next.i125, %for.body.i118 ], [ 0, %for.body.i118.preheader ]
  %rem12.i120 = and i64 %indvars.iv.i119, 1
  %cmp1.i121 = icmp eq i64 %rem12.i120, 0
  %9 = trunc i64 %indvars.iv.i119 to i32
  %10 = sub i32 0, %9
  %.p.i122 = select i1 %cmp1.i121, i32 %9, i32 %10
  %11 = add i32 %.p.i122, 18
  %rem2.i123 = urem i32 %11, 101
  %arrayidx.i124 = getelementptr inbounds [100 x i32], ptr %kr, i64 0, i64 %indvars.iv.i119
  store i32 %rem2.i123, ptr %arrayidx.i124, align 4, !tbaa !3
  %indvars.iv.next.i125 = add nuw nsw i64 %indvars.iv.i119, 1
  %exitcond.not.i126 = icmp eq i64 %indvars.iv.next.i125, 100
  br i1 %exitcond.not.i126, label %for.body.i128.preheader, label %for.body.i118, !llvm.loop !7

for.body.i128.preheader:                          ; preds = %for.body.i118
  br label %for.body.i128

for.body.i128:                                    ; preds = %for.body.i128.preheader, %for.body.i128
  %indvars.iv.i129 = phi i64 [ %indvars.iv.next.i135, %for.body.i128 ], [ 0, %for.body.i128.preheader ]
  %rem12.i130 = and i64 %indvars.iv.i129, 1
  %cmp1.i131 = icmp eq i64 %rem12.i130, 0
  %12 = trunc i64 %indvars.iv.i129 to i32
  %13 = sub i32 0, %12
  %.p.i132 = select i1 %cmp1.i131, i32 %12, i32 %13
  %14 = add i32 %.p.i132, 99
  %rem2.i133 = urem i32 %14, 101
  %arrayidx.i134 = getelementptr inbounds [100 x i32], ptr %zp9, i64 0, i64 %indvars.iv.i129
  store i32 %rem2.i133, ptr %arrayidx.i134, align 4, !tbaa !3
  %indvars.iv.next.i135 = add nuw nsw i64 %indvars.iv.i129, 1
  %exitcond.not.i136 = icmp eq i64 %indvars.iv.next.i135, 100
  br i1 %exitcond.not.i136, label %for.body.i138.preheader, label %for.body.i128, !llvm.loop !7

for.body.i138.preheader:                          ; preds = %for.body.i128
  br label %for.body.i138

for.body.i138:                                    ; preds = %for.body.i138.preheader, %for.body.i138
  %indvars.iv.i139 = phi i64 [ %indvars.iv.next.i145, %for.body.i138 ], [ 0, %for.body.i138.preheader ]
  %rem12.i140 = and i64 %indvars.iv.i139, 1
  %cmp1.i141 = icmp eq i64 %rem12.i140, 0
  %15 = trunc i64 %indvars.iv.i139 to i32
  %16 = sub i32 0, %15
  %.p.i142 = select i1 %cmp1.i141, i32 %15, i32 %16
  %17 = add i32 %.p.i142, 14
  %rem2.i143 = urem i32 %17, 101
  %arrayidx.i144 = getelementptr inbounds [100 x i32], ptr %aj9, i64 0, i64 %indvars.iv.i139
  store i32 %rem2.i143, ptr %arrayidx.i144, align 4, !tbaa !3
  %indvars.iv.next.i145 = add nuw nsw i64 %indvars.iv.i139, 1
  %exitcond.not.i146 = icmp eq i64 %indvars.iv.next.i145, 100
  br i1 %exitcond.not.i146, label %for.body.i148.preheader, label %for.body.i138, !llvm.loop !7

for.body.i148.preheader:                          ; preds = %for.body.i138
  br label %for.body.i148

for.body.i148:                                    ; preds = %for.body.i148.preheader, %for.body.i148
  %indvars.iv.i149 = phi i64 [ %indvars.iv.next.i155, %for.body.i148 ], [ 0, %for.body.i148.preheader ]
  %rem12.i150 = and i64 %indvars.iv.i149, 1
  %cmp1.i151 = icmp eq i64 %rem12.i150, 0
  %18 = trunc i64 %indvars.iv.i149 to i32
  %19 = sub i32 0, %18
  %.p.i152 = select i1 %cmp1.i151, i32 %18, i32 %19
  %20 = add i32 %.p.i152, 91
  %rem2.i153 = urem i32 %20, 101
  %arrayidx.i154 = getelementptr inbounds [100 x i32], ptr %k4, i64 0, i64 %indvars.iv.i149
  store i32 %rem2.i153, ptr %arrayidx.i154, align 4, !tbaa !3
  %indvars.iv.next.i155 = add nuw nsw i64 %indvars.iv.i149, 1
  %exitcond.not.i156 = icmp eq i64 %indvars.iv.next.i155, 100
  br i1 %exitcond.not.i156, label %for.body.i158.preheader, label %for.body.i148, !llvm.loop !7

for.body.i158.preheader:                          ; preds = %for.body.i148
  br label %for.body.i158

for.body.i158:                                    ; preds = %for.body.i158.preheader, %for.body.i158
  %indvars.iv.i159 = phi i64 [ %indvars.iv.next.i165, %for.body.i158 ], [ 0, %for.body.i158.preheader ]
  %rem12.i160 = and i64 %indvars.iv.i159, 1
  %cmp1.i161 = icmp eq i64 %rem12.i160, 0
  %21 = trunc i64 %indvars.iv.i159 to i32
  %22 = sub i32 0, %21
  %.p.i162 = select i1 %cmp1.i161, i32 %21, i32 %22
  %23 = add i32 %.p.i162, 49
  %rem2.i163 = urem i32 %23, 101
  %arrayidx.i164 = getelementptr inbounds [100 x i32], ptr %h1, i64 0, i64 %indvars.iv.i159
  store i32 %rem2.i163, ptr %arrayidx.i164, align 4, !tbaa !3
  %indvars.iv.next.i165 = add nuw nsw i64 %indvars.iv.i159, 1
  %exitcond.not.i166 = icmp eq i64 %indvars.iv.next.i165, 100
  br i1 %exitcond.not.i166, label %init.exit167, label %for.body.i158, !llvm.loop !7

init.exit167:                                     ; preds = %for.body.i158
  %arrayidx1 = getelementptr inbounds [100 x i32], ptr %k4, i64 0, i64 27, !intel-tbaa !11
  %call = call i32 (ptr, ...) @__isoc99_scanf(ptr noundef nonnull @.str, ptr noundef nonnull %k, ptr noundef nonnull %n4, ptr noundef nonnull %nn, ptr noundef nonnull %n, ptr noundef nonnull %a)
  %arrayidx.promoted = load i32, ptr %arrayidx, align 4, !tbaa !11
  %nn.promoted = load i32, ptr %nn, align 4, !tbaa !3
  %n4.promoted = load i32, ptr %n4, align 4, !tbaa !3
  %n.promoted = load i32, ptr %n, align 4, !tbaa !3
  br label %if.then

if.then:                                          ; preds = %init.exit167, %if.end48
  %indvars.iv = phi i64 [ 82, %init.exit167 ], [ %indvars.iv.next, %if.end48 ]
  %sub291300 = phi i32 [ %arrayidx.promoted, %init.exit167 ], [ %sub47, %if.end48 ]
  %inc36293299 = phi i32 [ %nn.promoted, %init.exit167 ], [ %inc36292, %if.end48 ]
  %inc295298 = phi i32 [ %n4.promoted, %init.exit167 ], [ %inc294, %if.end48 ]
  %dec296297 = phi i32 [ %n.promoted, %init.exit167 ], [ %dec, %if.end48 ]
  %arrayidx9 = getelementptr inbounds [100 x i32], ptr %w, i64 0, i64 %indvars.iv, !intel-tbaa !11
  %24 = load i32, ptr %arrayidx9, align 4, !tbaa !11
  %sub = sub i32 %sub291300, %24
  store i32 %sub, ptr %arrayidx, align 4, !tbaa !11
  %arrayidx16 = getelementptr inbounds [100 x i32], ptr %zs, i64 0, i64 %indvars.iv, !intel-tbaa !11
  %25 = load i32, ptr %arrayidx16, align 4, !tbaa !11
  %cmp19 = icmp eq i32 %25, -18
  br i1 %cmp19, label %if.then20, label %if.else

if.then20:                                        ; preds = %if.then
  %arrayidx24 = getelementptr inbounds [100 x i32], ptr %kr, i64 0, i64 %indvars.iv, !intel-tbaa !11
  %26 = load i32, ptr %arrayidx24, align 4, !tbaa !11
  %add25 = add i32 %26, %25
  store i32 %add25, ptr %arrayidx24, align 4, !tbaa !11
  %inc = add i32 %inc295298, 1
  store i32 %inc, ptr %n4, align 4, !tbaa !3
  %27 = add nuw nsw i64 %indvars.iv, 1
  %arrayidx28 = getelementptr inbounds [100 x i32], ptr %zp9, i64 0, i64 %27, !intel-tbaa !11
  %28 = load i32, ptr %arrayidx28, align 4, !tbaa !11
  %add29 = add i32 %28, %inc295298
  store i32 %add29, ptr %arrayidx28, align 4, !tbaa !11
  %arrayidx32 = getelementptr inbounds [100 x i32], ptr %aj9, i64 0, i64 %27, !intel-tbaa !11
  %29 = load i32, ptr %arrayidx32, align 4, !tbaa !11
  %30 = add nsw i64 %indvars.iv, -1
  %arrayidx35 = getelementptr inbounds [100 x i32], ptr %kr, i64 0, i64 %30, !intel-tbaa !11
  store i32 %29, ptr %arrayidx35, align 4, !tbaa !11
  br label %if.end48

if.else:                                          ; preds = %if.then
  %mul = mul i32 %sub, %sub
  store i32 %mul, ptr %arrayidx, align 4, !tbaa !11
  %inc36 = add i32 %inc36293299, 1
  store i32 %inc36, ptr %nn, align 4, !tbaa !3
  %31 = add nuw nsw i64 %indvars.iv, 1
  %arrayidx39 = getelementptr inbounds [100 x i32], ptr %k4, i64 0, i64 %31, !intel-tbaa !11
  store i32 %inc36, ptr %arrayidx39, align 4, !tbaa !11
  br label %if.end48

if.end48:                                         ; preds = %if.then20, %if.else
  %inc294 = phi i32 [ %inc, %if.then20 ], [ %inc295298, %if.else ]
  %inc36292 = phi i32 [ %inc36293299, %if.then20 ], [ %inc36, %if.else ]
  %32 = trunc i64 %indvars.iv to i32
  %33 = mul i32 %32, -66
  %sub43 = add i32 %24, %33
  store i32 %sub43, ptr %arrayidx9, align 4, !tbaa !11
  %arrayidx45 = getelementptr inbounds [100 x i32], ptr %h1, i64 0, i64 %indvars.iv, !intel-tbaa !11
  %34 = load i32, ptr %arrayidx45, align 4, !tbaa !11
  %sub46.neg = sub i32 %32, %34
  %35 = load i32, ptr %arrayidx1, align 4, !tbaa !11
  %sub47 = add i32 %sub46.neg, %35
  store i32 %sub47, ptr %arrayidx1, align 4, !tbaa !11
  %36 = add nuw nsw i64 %indvars.iv, 1
  %arrayidx51 = getelementptr inbounds [100 x i32], ptr %h1, i64 0, i64 %36, !intel-tbaa !11
  %37 = load i32, ptr %arrayidx51, align 4, !tbaa !11
  %arrayidx53 = getelementptr inbounds [100 x i32], ptr %aj9, i64 0, i64 %indvars.iv, !intel-tbaa !11
  store i32 %37, ptr %arrayidx53, align 4, !tbaa !11
  store i32 %sub47, ptr %arrayidx, align 4, !tbaa !11
  %dec = add i32 %dec296297, -1
  %sub56 = sub i32 %34, %dec296297
  store i32 %sub56, ptr %arrayidx45, align 4, !tbaa !11
  %indvars.iv.next = add nsw i64 %indvars.iv, -1
  %cmp = icmp ugt i64 %indvars.iv.next, 4
  br i1 %cmp, label %if.then, label %for.end, !llvm.loop !13

for.end:                                          ; preds = %if.end48
  %38 = add i32 %n.promoted, -78
  store i32 4, ptr %k, align 4, !tbaa !3
  store i32 %38, ptr %n, align 4, !tbaa !3
  %arrayidx58 = getelementptr inbounds [100 x i32], ptr %kr, i64 0, i64 80, !intel-tbaa !11
  %39 = load i32, ptr %arrayidx58, align 16, !tbaa !11
  store i32 %39, ptr %a, align 4, !tbaa !3
  %40 = load i32, ptr %n4, align 4, !tbaa !3
  %sub59.neg = sub i32 %40, %39
  %arrayidx60 = getelementptr inbounds [100 x i32], ptr %w, i64 0, i64 18, !intel-tbaa !11
  %41 = load i32, ptr %arrayidx60, align 8, !tbaa !11
  %sub61 = add i32 %sub59.neg, %41
  store i32 %sub61, ptr %arrayidx60, align 8, !tbaa !11
  %42 = load i32, ptr %nn, align 4, !tbaa !3
  %43 = load i32, ptr %n, align 4, !tbaa !3
  br label %for.body.i168

for.body.i168:                                    ; preds = %cond.end.i, %for.end
  %indvars.iv.i169 = phi i64 [ 0, %for.end ], [ %indvars.iv.next.i171, %cond.end.i ]
  %sum.012.i = phi i32 [ 0, %for.end ], [ %add.i, %cond.end.i ]
  %rem14.i = and i64 %indvars.iv.i169, 1
  %cmp1.i170 = icmp eq i64 %rem14.i, 0
  br i1 %cmp1.i170, label %cond.true.i, label %cond.false.i

cond.true.i:                                      ; preds = %for.body.i168
  %arrayidx.i173 = getelementptr inbounds [100 x i32], ptr %w, i64 0, i64 %indvars.iv.i169
  %44 = load i32, ptr %arrayidx.i173, align 4, !tbaa !3
  br label %cond.end.i

cond.false.i:                                     ; preds = %for.body.i168
  %arrayidx3.i = getelementptr inbounds [100 x i32], ptr %w, i64 0, i64 %indvars.iv.i169
  %45 = load i32, ptr %arrayidx3.i, align 4, !tbaa !3
  %sub.i = sub i32 0, %45
  br label %cond.end.i

cond.end.i:                                       ; preds = %cond.false.i, %cond.true.i
  %cond.i = phi i32 [ %44, %cond.true.i ], [ %sub.i, %cond.false.i ]
  %add.i = add i32 %cond.i, %sum.012.i
  %indvars.iv.next.i171 = add nuw nsw i64 %indvars.iv.i169, 1
  %exitcond.not.i172 = icmp eq i64 %indvars.iv.next.i171, 100
  br i1 %exitcond.not.i172, label %for.body.i174.preheader, label %for.body.i168, !llvm.loop !10

for.body.i174.preheader:                          ; preds = %cond.end.i
  br label %for.body.i174

for.body.i174:                                    ; preds = %for.body.i174.preheader, %cond.end.i182
  %indvars.iv.i175 = phi i64 [ %indvars.iv.next.i185, %cond.end.i182 ], [ 0, %for.body.i174.preheader ]
  %sum.012.i176 = phi i32 [ %add.i184, %cond.end.i182 ], [ 0, %for.body.i174.preheader ]
  %rem14.i177 = and i64 %indvars.iv.i175, 1
  %cmp1.i178 = icmp eq i64 %rem14.i177, 0
  br i1 %cmp1.i178, label %cond.true.i187, label %cond.false.i179

cond.true.i187:                                   ; preds = %for.body.i174
  %arrayidx.i188 = getelementptr inbounds [100 x i32], ptr %n0, i64 0, i64 %indvars.iv.i175
  %46 = load i32, ptr %arrayidx.i188, align 4, !tbaa !3
  br label %cond.end.i182

cond.false.i179:                                  ; preds = %for.body.i174
  %arrayidx3.i180 = getelementptr inbounds [100 x i32], ptr %n0, i64 0, i64 %indvars.iv.i175
  %47 = load i32, ptr %arrayidx3.i180, align 4, !tbaa !3
  %sub.i181 = sub i32 0, %47
  br label %cond.end.i182

cond.end.i182:                                    ; preds = %cond.false.i179, %cond.true.i187
  %cond.i183 = phi i32 [ %46, %cond.true.i187 ], [ %sub.i181, %cond.false.i179 ]
  %add.i184 = add i32 %cond.i183, %sum.012.i176
  %indvars.iv.next.i185 = add nuw nsw i64 %indvars.iv.i175, 1
  %exitcond.not.i186 = icmp eq i64 %indvars.iv.next.i185, 100
  br i1 %exitcond.not.i186, label %for.body.i190.preheader, label %for.body.i174, !llvm.loop !10

for.body.i190.preheader:                          ; preds = %cond.end.i182
  br label %for.body.i190

for.body.i190:                                    ; preds = %for.body.i190.preheader, %cond.end.i198
  %indvars.iv.i191 = phi i64 [ %indvars.iv.next.i201, %cond.end.i198 ], [ 0, %for.body.i190.preheader ]
  %sum.012.i192 = phi i32 [ %add.i200, %cond.end.i198 ], [ 0, %for.body.i190.preheader ]
  %rem14.i193 = and i64 %indvars.iv.i191, 1
  %cmp1.i194 = icmp eq i64 %rem14.i193, 0
  br i1 %cmp1.i194, label %cond.true.i203, label %cond.false.i195

cond.true.i203:                                   ; preds = %for.body.i190
  %arrayidx.i204 = getelementptr inbounds [100 x i32], ptr %zs, i64 0, i64 %indvars.iv.i191
  %48 = load i32, ptr %arrayidx.i204, align 4, !tbaa !3
  br label %cond.end.i198

cond.false.i195:                                  ; preds = %for.body.i190
  %arrayidx3.i196 = getelementptr inbounds [100 x i32], ptr %zs, i64 0, i64 %indvars.iv.i191
  %49 = load i32, ptr %arrayidx3.i196, align 4, !tbaa !3
  %sub.i197 = sub i32 0, %49
  br label %cond.end.i198

cond.end.i198:                                    ; preds = %cond.false.i195, %cond.true.i203
  %cond.i199 = phi i32 [ %48, %cond.true.i203 ], [ %sub.i197, %cond.false.i195 ]
  %add.i200 = add i32 %cond.i199, %sum.012.i192
  %indvars.iv.next.i201 = add nuw nsw i64 %indvars.iv.i191, 1
  %exitcond.not.i202 = icmp eq i64 %indvars.iv.next.i201, 100
  br i1 %exitcond.not.i202, label %for.body.i206.preheader, label %for.body.i190, !llvm.loop !10

for.body.i206.preheader:                          ; preds = %cond.end.i198
  br label %for.body.i206

for.body.i206:                                    ; preds = %for.body.i206.preheader, %cond.end.i214
  %indvars.iv.i207 = phi i64 [ %indvars.iv.next.i217, %cond.end.i214 ], [ 0, %for.body.i206.preheader ]
  %sum.012.i208 = phi i32 [ %add.i216, %cond.end.i214 ], [ 0, %for.body.i206.preheader ]
  %rem14.i209 = and i64 %indvars.iv.i207, 1
  %cmp1.i210 = icmp eq i64 %rem14.i209, 0
  br i1 %cmp1.i210, label %cond.true.i219, label %cond.false.i211

cond.true.i219:                                   ; preds = %for.body.i206
  %arrayidx.i220 = getelementptr inbounds [100 x i32], ptr %kr, i64 0, i64 %indvars.iv.i207
  %50 = load i32, ptr %arrayidx.i220, align 4, !tbaa !3
  br label %cond.end.i214

cond.false.i211:                                  ; preds = %for.body.i206
  %arrayidx3.i212 = getelementptr inbounds [100 x i32], ptr %kr, i64 0, i64 %indvars.iv.i207
  %51 = load i32, ptr %arrayidx3.i212, align 4, !tbaa !3
  %sub.i213 = sub i32 0, %51
  br label %cond.end.i214

cond.end.i214:                                    ; preds = %cond.false.i211, %cond.true.i219
  %cond.i215 = phi i32 [ %50, %cond.true.i219 ], [ %sub.i213, %cond.false.i211 ]
  %add.i216 = add i32 %cond.i215, %sum.012.i208
  %indvars.iv.next.i217 = add nuw nsw i64 %indvars.iv.i207, 1
  %exitcond.not.i218 = icmp eq i64 %indvars.iv.next.i217, 100
  br i1 %exitcond.not.i218, label %for.body.i222.preheader, label %for.body.i206, !llvm.loop !10

for.body.i222.preheader:                          ; preds = %cond.end.i214
  br label %for.body.i222

for.body.i222:                                    ; preds = %for.body.i222.preheader, %cond.end.i230
  %indvars.iv.i223 = phi i64 [ %indvars.iv.next.i233, %cond.end.i230 ], [ 0, %for.body.i222.preheader ]
  %sum.012.i224 = phi i32 [ %add.i232, %cond.end.i230 ], [ 0, %for.body.i222.preheader ]
  %rem14.i225 = and i64 %indvars.iv.i223, 1
  %cmp1.i226 = icmp eq i64 %rem14.i225, 0
  br i1 %cmp1.i226, label %cond.true.i235, label %cond.false.i227

cond.true.i235:                                   ; preds = %for.body.i222
  %arrayidx.i236 = getelementptr inbounds [100 x i32], ptr %zp9, i64 0, i64 %indvars.iv.i223
  %52 = load i32, ptr %arrayidx.i236, align 4, !tbaa !3
  br label %cond.end.i230

cond.false.i227:                                  ; preds = %for.body.i222
  %arrayidx3.i228 = getelementptr inbounds [100 x i32], ptr %zp9, i64 0, i64 %indvars.iv.i223
  %53 = load i32, ptr %arrayidx3.i228, align 4, !tbaa !3
  %sub.i229 = sub i32 0, %53
  br label %cond.end.i230

cond.end.i230:                                    ; preds = %cond.false.i227, %cond.true.i235
  %cond.i231 = phi i32 [ %52, %cond.true.i235 ], [ %sub.i229, %cond.false.i227 ]
  %add.i232 = add i32 %cond.i231, %sum.012.i224
  %indvars.iv.next.i233 = add nuw nsw i64 %indvars.iv.i223, 1
  %exitcond.not.i234 = icmp eq i64 %indvars.iv.next.i233, 100
  br i1 %exitcond.not.i234, label %for.body.i238.preheader, label %for.body.i222, !llvm.loop !10

for.body.i238.preheader:                          ; preds = %cond.end.i230
  br label %for.body.i238

for.body.i238:                                    ; preds = %for.body.i238.preheader, %cond.end.i246
  %indvars.iv.i239 = phi i64 [ %indvars.iv.next.i249, %cond.end.i246 ], [ 0, %for.body.i238.preheader ]
  %sum.012.i240 = phi i32 [ %add.i248, %cond.end.i246 ], [ 0, %for.body.i238.preheader ]
  %rem14.i241 = and i64 %indvars.iv.i239, 1
  %cmp1.i242 = icmp eq i64 %rem14.i241, 0
  br i1 %cmp1.i242, label %cond.true.i251, label %cond.false.i243

cond.true.i251:                                   ; preds = %for.body.i238
  %arrayidx.i252 = getelementptr inbounds [100 x i32], ptr %aj9, i64 0, i64 %indvars.iv.i239
  %54 = load i32, ptr %arrayidx.i252, align 4, !tbaa !3
  br label %cond.end.i246

cond.false.i243:                                  ; preds = %for.body.i238
  %arrayidx3.i244 = getelementptr inbounds [100 x i32], ptr %aj9, i64 0, i64 %indvars.iv.i239
  %55 = load i32, ptr %arrayidx3.i244, align 4, !tbaa !3
  %sub.i245 = sub i32 0, %55
  br label %cond.end.i246

cond.end.i246:                                    ; preds = %cond.false.i243, %cond.true.i251
  %cond.i247 = phi i32 [ %54, %cond.true.i251 ], [ %sub.i245, %cond.false.i243 ]
  %add.i248 = add i32 %cond.i247, %sum.012.i240
  %indvars.iv.next.i249 = add nuw nsw i64 %indvars.iv.i239, 1
  %exitcond.not.i250 = icmp eq i64 %indvars.iv.next.i249, 100
  br i1 %exitcond.not.i250, label %for.body.i254.preheader, label %for.body.i238, !llvm.loop !10

for.body.i254.preheader:                          ; preds = %cond.end.i246
  br label %for.body.i254

for.body.i254:                                    ; preds = %for.body.i254.preheader, %cond.end.i262
  %indvars.iv.i255 = phi i64 [ %indvars.iv.next.i265, %cond.end.i262 ], [ 0, %for.body.i254.preheader ]
  %sum.012.i256 = phi i32 [ %add.i264, %cond.end.i262 ], [ 0, %for.body.i254.preheader ]
  %rem14.i257 = and i64 %indvars.iv.i255, 1
  %cmp1.i258 = icmp eq i64 %rem14.i257, 0
  br i1 %cmp1.i258, label %cond.true.i267, label %cond.false.i259

cond.true.i267:                                   ; preds = %for.body.i254
  %arrayidx.i268 = getelementptr inbounds [100 x i32], ptr %k4, i64 0, i64 %indvars.iv.i255
  %56 = load i32, ptr %arrayidx.i268, align 4, !tbaa !3
  br label %cond.end.i262

cond.false.i259:                                  ; preds = %for.body.i254
  %arrayidx3.i260 = getelementptr inbounds [100 x i32], ptr %k4, i64 0, i64 %indvars.iv.i255
  %57 = load i32, ptr %arrayidx3.i260, align 4, !tbaa !3
  %sub.i261 = sub i32 0, %57
  br label %cond.end.i262

cond.end.i262:                                    ; preds = %cond.false.i259, %cond.true.i267
  %cond.i263 = phi i32 [ %56, %cond.true.i267 ], [ %sub.i261, %cond.false.i259 ]
  %add.i264 = add i32 %cond.i263, %sum.012.i256
  %indvars.iv.next.i265 = add nuw nsw i64 %indvars.iv.i255, 1
  %exitcond.not.i266 = icmp eq i64 %indvars.iv.next.i265, 100
  br i1 %exitcond.not.i266, label %for.body.i270.preheader, label %for.body.i254, !llvm.loop !10

for.body.i270.preheader:                          ; preds = %cond.end.i262
  br label %for.body.i270

for.body.i270:                                    ; preds = %for.body.i270.preheader, %cond.end.i278
  %indvars.iv.i271 = phi i64 [ %indvars.iv.next.i281, %cond.end.i278 ], [ 0, %for.body.i270.preheader ]
  %sum.012.i272 = phi i32 [ %add.i280, %cond.end.i278 ], [ 0, %for.body.i270.preheader ]
  %rem14.i273 = and i64 %indvars.iv.i271, 1
  %cmp1.i274 = icmp eq i64 %rem14.i273, 0
  br i1 %cmp1.i274, label %cond.true.i283, label %cond.false.i275

cond.true.i283:                                   ; preds = %for.body.i270
  %arrayidx.i284 = getelementptr inbounds [100 x i32], ptr %h1, i64 0, i64 %indvars.iv.i271
  %58 = load i32, ptr %arrayidx.i284, align 4, !tbaa !3
  br label %cond.end.i278

cond.false.i275:                                  ; preds = %for.body.i270
  %arrayidx3.i276 = getelementptr inbounds [100 x i32], ptr %h1, i64 0, i64 %indvars.iv.i271
  %59 = load i32, ptr %arrayidx3.i276, align 4, !tbaa !3
  %sub.i277 = sub i32 0, %59
  br label %cond.end.i278

cond.end.i278:                                    ; preds = %cond.false.i275, %cond.true.i283
  %cond.i279 = phi i32 [ %58, %cond.true.i283 ], [ %sub.i277, %cond.false.i275 ]
  %add.i280 = add i32 %cond.i279, %sum.012.i272
  %indvars.iv.next.i281 = add nuw nsw i64 %indvars.iv.i271, 1
  %exitcond.not.i282 = icmp eq i64 %indvars.iv.next.i281, 100
  br i1 %exitcond.not.i282, label %checkSum.exit285, label %for.body.i270, !llvm.loop !10

checkSum.exit285:                                 ; preds = %cond.end.i278
  %60 = add i32 %40, 4
  %61 = add i32 %39, %42
  %add70 = sub i32 %60, %61
  %62 = add i32 %add70, %43
  %63 = add i32 %62, %add.i
  %sub85 = add i32 %63, %add.i184
  %64 = add i32 %sub85, %add.i216
  %65 = add i32 %add.i200, %add.i232
  %66 = add i32 %64, %add.i248
  %67 = add i32 %65, %add.i264
  %sub65 = sub i32 %66, %67
  %add89 = add i32 %sub65, %add.i280
  %call90 = call i32 (ptr, ...) @printf(ptr noundef nonnull dereferenceable(1) @.str.1, i32 noundef %add89)
  call void @llvm.lifetime.end.p0(i64 400, ptr nonnull %h1) #6
  call void @llvm.lifetime.end.p0(i64 400, ptr nonnull %k4) #6
  call void @llvm.lifetime.end.p0(i64 400, ptr nonnull %aj9) #6
  call void @llvm.lifetime.end.p0(i64 400, ptr nonnull %zp9) #6
  call void @llvm.lifetime.end.p0(i64 400, ptr nonnull %kr) #6
  call void @llvm.lifetime.end.p0(i64 400, ptr nonnull %zs) #6
  call void @llvm.lifetime.end.p0(i64 400, ptr nonnull %n0) #6
  call void @llvm.lifetime.end.p0(i64 400, ptr nonnull %w) #6
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %a) #6
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %n) #6
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %nn) #6
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %n4) #6
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %k) #6
  ret i32 0
}

; Function Attrs: mustprogress nocallback nofree nounwind willreturn memory(argmem: write)
declare void @llvm.memset.p0.i64(ptr nocapture writeonly %0, i8 %1, i64 %2, i1 immarg %3) #4

; Function Attrs: nofree nounwind
declare dso_local noundef i32 @__isoc99_scanf(ptr nocapture noundef readonly %0, ...) local_unnamed_addr #5

; Function Attrs: nofree nounwind
declare dso_local noundef i32 @printf(ptr nocapture noundef readonly %0, ...) local_unnamed_addr #5

attributes #0 = { nofree norecurse nosync nounwind memory(argmem: write) uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #2 = { nofree norecurse nosync nounwind memory(argmem: read) uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #3 = { nofree nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #4 = { mustprogress nocallback nofree nounwind willreturn memory(argmem: write) }
attributes #5 = { nofree nounwind "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #6 = { nounwind }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2024.0.0 (2024.x.0.YYYYMMDD)"}
!3 = !{!4, !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = distinct !{!7, !8, !9}
!8 = !{!"llvm.loop.mustprogress"}
!9 = !{!"llvm.loop.unroll.disable"}
!10 = distinct !{!10, !8, !9}
!11 = !{!12, !4, i64 0}
!12 = !{!"array@_ZTSA100_j", !4, i64 0}
!13 = distinct !{!13, !8, !9}
