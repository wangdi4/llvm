; Sanity Test(s) on HIR Loop Reversal: testcase reported by CQ DPD200413783
; 
; [REASONS]
; - Applicable: YES
; - Profitable: YES
; - Legal:      NO
; - Decision:   NOT Reverse the loop
;
; Explain:
; %add1233826:22 (%t)[0][-1 * i1 + 42] --> (%t)[0][-1 * i1 + 44] ANTI (<) fails reversal's legal test.

; Issue with the CQ DPD200417762: same as CQDPD200415561
; 
; *** Source Code ***
; from ATG Tests, reported by Youcef/John.
;
;
; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-loop-reversal -print-before=hir-loop-reversal -print-after=hir-loop-reversal < %s 2>&1 | FileCheck %s
;
; CHECK: IR Dump Before HIR Loop Reversal
;
; CHECK:  + DO i1 = 0, 41, 1   <DO_LOOP>
; CHECK:  |   (%t)[0][-1 * i1 + 44] = -1 * i1 + %nv.promoted + -1;
; CHECK:  |   %add12338 = %add12338  +  (%t)[0][-1 * i1 + 42];
; CHECK:  + END LOOP
;
;
; CHECK: IR Dump After HIR Loop Reversal
;
; CHECK:  + DO i1 = 0, 41, 1   <DO_LOOP>
; CHECK:  |   (%t)[0][-1 * i1 + 44] = -1 * i1 + %nv.promoted + -1;
; CHECK:  |   %add12338 = %add12338  +  (%t)[0][-1 * i1 + 42];
; CHECK:  + END LOOP
;
; === ---------------------------------------------------------------- ===
; Following is the LLVM's input code!
; === ---------------------------------------------------------------- ===
;
source_filename = "bug5.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct._IO_FILE = type { i32, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, %struct._IO_marker*, %struct._IO_FILE*, i32, i32, i64, i16, i8, [1 x i8], i8*, i64, i8*, i8*, i8*, i8*, i64, i32, [20 x i8] }
%struct._IO_marker = type { %struct._IO_marker*, %struct._IO_FILE*, i32 }

@.str = private unnamed_addr constant [10 x i8] c"input.txt\00", align 1
@.str.1 = private unnamed_addr constant [2 x i8] c"r\00", align 1
@.str.2 = private unnamed_addr constant [27 x i8] c"%u %u %u %u %u %u %u %u %u\00", align 1
@.str.3 = private unnamed_addr constant [10 x i8] c"res = %u\0A\00", align 1

; Function Attrs: norecurse nounwind uwtable
define void @init(i32* nocapture %a, i32 %n, i32 %seed) local_unnamed_addr #0 {
entry:
  %cmp8 = icmp eq i32 %n, 0
  br i1 %cmp8, label %for.end, label %for.body.preheader

for.body.preheader:                               ; preds = %entry
  %wide.trip.count = zext i32 %n to i64
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %for.body.preheader ]
  %0 = trunc i64 %indvars.iv to i32
  %rem = and i32 %0, 1
  %cmp1 = icmp eq i32 %rem, 0
  %1 = sub i32 0, %0
  %.p = select i1 %cmp1, i32 %0, i32 %1
  %2 = add i32 %.p, %seed
  %rem2 = urem i32 %2, 101
  %arrayidx = getelementptr inbounds i32, i32* %a, i64 %indvars.iv
  store i32 %rem2, i32* %arrayidx, align 4, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body, %entry
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

; Function Attrs: norecurse nounwind readonly uwtable
define i32 @checkSum(i32* nocapture readonly %a, i32 %n) local_unnamed_addr #2 {
entry:
  %cmp10 = icmp eq i32 %n, 0
  br i1 %cmp10, label %for.end, label %for.body.preheader

for.body.preheader:                               ; preds = %entry
  %wide.trip.count = zext i32 %n to i64
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %for.body.preheader ]
  %sum.012 = phi i32 [ %add, %for.body ], [ 0, %for.body.preheader ]
  %rem = and i64 %indvars.iv, 1
  %cmp1 = icmp eq i64 %rem, 0
  %arrayidx = getelementptr inbounds i32, i32* %a, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !1
  %sub = sub i32 0, %0
  %1 = select i1 %cmp1, i32 %0, i32 %sub
  %add = add i32 %1, %sum.012
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body, %entry
  %sum.0.lcssa = phi i32 [ 0, %entry ], [ %add, %for.body ]
  ret i32 %sum.0.lcssa
}

; Function Attrs: nounwind uwtable
define i32 @main() local_unnamed_addr #3 {
entry:
  %k = alloca i32, align 4
  %nv = alloca i32, align 4
  %s5 = alloca i32, align 4
  %j = alloca i32, align 4
  %g = alloca i32, align 4
  %jh = alloca i32, align 4
  %n = alloca i32, align 4
  %jx = alloca i32, align 4
  %n2 = alloca i32, align 4
  %t = alloca [100 x i32], align 16
  %ml = alloca [100 x i32], align 16
  %pz = alloca [100 x i32], align 16
  %u = alloca [100 x [100 x i32]], align 16
  %gq = alloca [100 x [100 x i32]], align 16
  %b = alloca [100 x i32], align 16
  %j5 = alloca [100 x [100 x [100 x i32]]], align 16
  %c = alloca [100 x [100 x i32]], align 16
  %call = tail call %struct._IO_FILE* @fopen(i8* getelementptr inbounds ([10 x i8], [10 x i8]* @.str, i64 0, i64 0), i8* getelementptr inbounds ([2 x i8], [2 x i8]* @.str.1, i64 0, i64 0))
  %0 = bitcast i32* %k to i8*
  call void @llvm.lifetime.start(i64 4, i8* %0) #6
  store i32 18, i32* %k, align 4, !tbaa !1
  %1 = bitcast i32* %nv to i8*
  call void @llvm.lifetime.start(i64 4, i8* %1) #6
  store i32 81, i32* %nv, align 4, !tbaa !1
  %2 = bitcast i32* %s5 to i8*
  call void @llvm.lifetime.start(i64 4, i8* %2) #6
  store i32 13, i32* %s5, align 4, !tbaa !1
  %3 = bitcast i32* %j to i8*
  call void @llvm.lifetime.start(i64 4, i8* %3) #6
  store i32 28, i32* %j, align 4, !tbaa !1
  %4 = bitcast i32* %g to i8*
  call void @llvm.lifetime.start(i64 4, i8* %4) #6
  store i32 76, i32* %g, align 4, !tbaa !1
  %5 = bitcast i32* %jh to i8*
  call void @llvm.lifetime.start(i64 4, i8* %5) #6
  store i32 97, i32* %jh, align 4, !tbaa !1
  %6 = bitcast i32* %n to i8*
  call void @llvm.lifetime.start(i64 4, i8* %6) #6
  store i32 37, i32* %n, align 4, !tbaa !1
  %7 = bitcast i32* %jx to i8*
  call void @llvm.lifetime.start(i64 4, i8* %7) #6
  store i32 63, i32* %jx, align 4, !tbaa !1
  %8 = bitcast i32* %n2 to i8*
  call void @llvm.lifetime.start(i64 4, i8* %8) #6
  store i32 77, i32* %n2, align 4, !tbaa !1
  %9 = bitcast [100 x i32]* %t to i8*
  call void @llvm.lifetime.start(i64 400, i8* %9) #6
  %10 = bitcast [100 x i32]* %ml to i8*
  call void @llvm.lifetime.start(i64 400, i8* %10) #6
  %11 = bitcast [100 x i32]* %pz to i8*
  call void @llvm.lifetime.start(i64 400, i8* %11) #6
  %12 = bitcast [100 x [100 x i32]]* %u to i8*
  call void @llvm.lifetime.start(i64 40000, i8* %12) #6
  %13 = bitcast [100 x [100 x i32]]* %gq to i8*
  call void @llvm.lifetime.start(i64 40000, i8* %13) #6
  %14 = bitcast [100 x i32]* %b to i8*
  call void @llvm.lifetime.start(i64 400, i8* %14) #6
  %15 = bitcast [100 x [100 x [100 x i32]]]* %j5 to i8*
  call void @llvm.lifetime.start(i64 4000000, i8* %15) #6
  %16 = bitcast [100 x [100 x i32]]* %c to i8*
  call void @llvm.lifetime.start(i64 40000, i8* %16) #6
  br label %for.body.i

for.body.i:                                       ; preds = %for.body.i, %entry
  %indvars.iv401 = phi i64 [ 0, %entry ], [ %indvars.iv.next402, %for.body.i ]
  %17 = trunc i64 %indvars.iv401 to i32
  %rem.i = and i32 %17, 1
  %cmp1.i = icmp eq i32 %rem.i, 0
  %18 = sub nsw i32 0, %17
  %.p.i = select i1 %cmp1.i, i32 %17, i32 %18
  %19 = add i32 %.p.i, 19
  %rem2.i = urem i32 %19, 101
  %arrayidx.i = getelementptr inbounds [100 x i32], [100 x i32]* %t, i64 0, i64 %indvars.iv401
  store i32 %rem2.i, i32* %arrayidx.i, align 4, !tbaa !1
  %indvars.iv.next402 = add nuw nsw i64 %indvars.iv401, 1
  %exitcond403 = icmp eq i64 %indvars.iv.next402, 100
  br i1 %exitcond403, label %for.body.i311.preheader, label %for.body.i

for.body.i311.preheader:                          ; preds = %for.body.i
  %arrayidx = getelementptr inbounds [100 x i32], [100 x i32]* %ml, i64 0, i64 37
  br label %for.body.i311

for.body.i311:                                    ; preds = %for.body.i311, %for.body.i311.preheader
  %indvars.iv398 = phi i64 [ 0, %for.body.i311.preheader ], [ %indvars.iv.next399, %for.body.i311 ]
  %20 = trunc i64 %indvars.iv398 to i32
  %rem.i304 = and i32 %20, 1
  %cmp1.i305 = icmp eq i32 %rem.i304, 0
  %21 = sub nsw i32 0, %20
  %.p.i306 = select i1 %cmp1.i305, i32 %20, i32 %21
  %22 = add i32 %.p.i306, 42
  %rem2.i307 = urem i32 %22, 101
  %arrayidx.i309 = getelementptr inbounds [100 x i32], [100 x i32]* %ml, i64 0, i64 %indvars.iv398
  store i32 %rem2.i307, i32* %arrayidx.i309, align 4, !tbaa !1
  %indvars.iv.next399 = add nuw nsw i64 %indvars.iv398, 1
  %exitcond400 = icmp eq i64 %indvars.iv.next399, 100
  br i1 %exitcond400, label %for.body.i299, label %for.body.i311

for.body.i299:                                    ; preds = %for.body.i311, %for.body.i299
  %indvars.iv395 = phi i64 [ %indvars.iv.next396, %for.body.i299 ], [ 0, %for.body.i311 ]
  %23 = trunc i64 %indvars.iv395 to i32
  %rem.i292 = and i32 %23, 1
  %cmp1.i293 = icmp eq i32 %rem.i292, 0
  %24 = sub nsw i32 0, %23
  %.p.i294 = select i1 %cmp1.i293, i32 %23, i32 %24
  %25 = add i32 %.p.i294, 63
  %rem2.i295 = urem i32 %25, 101
  %arrayidx.i297 = getelementptr inbounds [100 x i32], [100 x i32]* %pz, i64 0, i64 %indvars.iv395
  store i32 %rem2.i295, i32* %arrayidx.i297, align 4, !tbaa !1
  %indvars.iv.next396 = add nuw nsw i64 %indvars.iv395, 1
  %exitcond397 = icmp eq i64 %indvars.iv.next396, 100
  br i1 %exitcond397, label %for.body.i287, label %for.body.i299

for.body.i287:                                    ; preds = %for.body.i299, %for.body.i287
  %indvars.iv392 = phi i64 [ %indvars.iv.next393, %for.body.i287 ], [ 0, %for.body.i299 ]
  %26 = trunc i64 %indvars.iv392 to i32
  %rem.i280 = and i32 %26, 1
  %cmp1.i281 = icmp eq i32 %rem.i280, 0
  %27 = sub nsw i32 0, %26
  %.p.i282 = select i1 %cmp1.i281, i32 %26, i32 %27
  %28 = add i32 %.p.i282, 67
  %rem2.i283 = urem i32 %28, 101
  %arrayidx.i285 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* %u, i64 0, i64 0, i64 %indvars.iv392
  store i32 %rem2.i283, i32* %arrayidx.i285, align 4, !tbaa !1
  %indvars.iv.next393 = add nuw nsw i64 %indvars.iv392, 1
  %exitcond394 = icmp eq i64 %indvars.iv.next393, 10000
  br i1 %exitcond394, label %for.body.i275, label %for.body.i287

for.body.i275:                                    ; preds = %for.body.i287, %for.body.i275
  %indvars.iv389 = phi i64 [ %indvars.iv.next390, %for.body.i275 ], [ 0, %for.body.i287 ]
  %29 = trunc i64 %indvars.iv389 to i32
  %rem.i268 = and i32 %29, 1
  %cmp1.i269 = icmp eq i32 %rem.i268, 0
  %30 = sub nsw i32 0, %29
  %.p.i270 = select i1 %cmp1.i269, i32 %29, i32 %30
  %31 = add i32 %.p.i270, 89
  %rem2.i271 = urem i32 %31, 101
  %arrayidx.i273 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* %gq, i64 0, i64 0, i64 %indvars.iv389
  store i32 %rem2.i271, i32* %arrayidx.i273, align 4, !tbaa !1
  %indvars.iv.next390 = add nuw nsw i64 %indvars.iv389, 1
  %exitcond391 = icmp eq i64 %indvars.iv.next390, 10000
  br i1 %exitcond391, label %for.body.i263, label %for.body.i275

for.body.i263:                                    ; preds = %for.body.i275, %for.body.i263
  %indvars.iv386 = phi i64 [ %indvars.iv.next387, %for.body.i263 ], [ 0, %for.body.i275 ]
  %32 = trunc i64 %indvars.iv386 to i32
  %rem.i256 = and i32 %32, 1
  %cmp1.i257 = icmp eq i32 %rem.i256, 0
  %33 = sub nsw i32 0, %32
  %.p.i258 = select i1 %cmp1.i257, i32 %32, i32 %33
  %34 = add i32 %.p.i258, 52
  %rem2.i259 = urem i32 %34, 101
  %arrayidx.i261 = getelementptr inbounds [100 x i32], [100 x i32]* %b, i64 0, i64 %indvars.iv386
  store i32 %rem2.i259, i32* %arrayidx.i261, align 4, !tbaa !1
  %indvars.iv.next387 = add nuw nsw i64 %indvars.iv386, 1
  %exitcond388 = icmp eq i64 %indvars.iv.next387, 100
  br i1 %exitcond388, label %for.body.i251, label %for.body.i263

for.body.i251:                                    ; preds = %for.body.i263, %for.body.i251
  %indvars.iv383 = phi i64 [ %indvars.iv.next384, %for.body.i251 ], [ 0, %for.body.i263 ]
  %35 = trunc i64 %indvars.iv383 to i32
  %rem.i244 = and i32 %35, 1
  %cmp1.i245 = icmp eq i32 %rem.i244, 0
  %36 = sub nsw i32 0, %35
  %.p.i246 = select i1 %cmp1.i245, i32 %35, i32 %36
  %37 = add i32 %.p.i246, 54
  %rem2.i247 = urem i32 %37, 101
  %arrayidx.i249 = getelementptr inbounds [100 x [100 x [100 x i32]]], [100 x [100 x [100 x i32]]]* %j5, i64 0, i64 0, i64 0, i64 %indvars.iv383
  store i32 %rem2.i247, i32* %arrayidx.i249, align 4, !tbaa !1
  %indvars.iv.next384 = add nuw nsw i64 %indvars.iv383, 1
  %exitcond385 = icmp eq i64 %indvars.iv.next384, 1000000
  br i1 %exitcond385, label %for.body.i239, label %for.body.i251

for.body.i239:                                    ; preds = %for.body.i251, %for.body.i239
  %indvars.iv380 = phi i64 [ %indvars.iv.next381, %for.body.i239 ], [ 0, %for.body.i251 ]
  %38 = trunc i64 %indvars.iv380 to i32
  %rem.i232 = and i32 %38, 1
  %cmp1.i233 = icmp eq i32 %rem.i232, 0
  %39 = sub nsw i32 0, %38
  %.p.i234 = select i1 %cmp1.i233, i32 %38, i32 %39
  %40 = add i32 %.p.i234, 27
  %rem2.i235 = urem i32 %40, 101
  %arrayidx.i237 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* %c, i64 0, i64 0, i64 %indvars.iv380
  store i32 %rem2.i235, i32* %arrayidx.i237, align 4, !tbaa !1
  %indvars.iv.next381 = add nuw nsw i64 %indvars.iv380, 1
  %exitcond382 = icmp eq i64 %indvars.iv.next381, 10000
  br i1 %exitcond382, label %init.exit240, label %for.body.i239

init.exit240:                                     ; preds = %for.body.i239
  %call8 = call i32 (%struct._IO_FILE*, i8*, ...) @__isoc99_fscanf(%struct._IO_FILE* %call, i8* getelementptr inbounds ([27 x i8], [27 x i8]* @.str.2, i64 0, i64 0), i32* nonnull %k, i32* nonnull %nv, i32* nonnull %s5, i32* nonnull %j, i32* nonnull %g, i32* nonnull %jh, i32* nonnull %n, i32* nonnull %jx, i32* nonnull %n2) #6
  store i32 43, i32* %k, align 4, !tbaa !1
  %nv.promoted = load i32, i32* %nv, align 4, !tbaa !1
  %s5.promoted = load i32, i32* %s5, align 4, !tbaa !1
  br label %for.body

for.cond20.preheader:                             ; preds = %for.body
  %41 = add i32 %nv.promoted, -42
  store i32 %41, i32* %nv, align 4, !tbaa !1
  store i32 %add12, i32* %s5, align 4, !tbaa !1
  store i32 1, i32* %k, align 4, !tbaa !1
  store i32 43, i32* %j, align 4, !tbaa !1
  %42 = load i32, i32* %g, align 4, !tbaa !1
  %scevgep = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* %gq, i64 0, i64 22, i64 4
  %n2.promoted.pre.pre = load i32, i32* %n2, align 4, !tbaa !1
  %arrayidx42.phi.trans.insert = getelementptr inbounds [100 x i32], [100 x i32]* %t, i64 0, i64 0
  br label %for.body22

for.body:                                         ; preds = %init.exit240, %for.body
  %indvars.iv377 = phi i64 [ 43, %init.exit240 ], [ %indvars.iv.next378, %for.body ]
  %add12338 = phi i32 [ %s5.promoted, %init.exit240 ], [ %add12, %for.body ]
  %dec337 = phi i32 [ %nv.promoted, %init.exit240 ], [ %dec, %for.body ]
  %dec = add i32 %dec337, -1
  %43 = add nuw nsw i64 %indvars.iv377, 1
  %arrayidx9 = getelementptr inbounds [100 x i32], [100 x i32]* %t, i64 0, i64 %43
  store i32 %dec, i32* %arrayidx9, align 4, !tbaa !1
  %indvars.iv.next378 = add nsw i64 %indvars.iv377, -1
  %arrayidx11 = getelementptr inbounds [100 x i32], [100 x i32]* %t, i64 0, i64 %indvars.iv.next378
  %44 = load i32, i32* %arrayidx11, align 4, !tbaa !1
  %add12 = add i32 %add12338, %44
  %cmp = icmp ugt i64 %indvars.iv.next378, 1
  br i1 %cmp, label %for.body, label %for.cond20.preheader

for.body22:                                       ; preds = %for.cond20.preheader, %for.inc89
  %n2.promoted.pre = phi i32 [ %n2.promoted.pre.pre, %for.cond20.preheader ], [ %47, %for.inc89 ]
  %45 = phi i32 [ 43, %for.cond20.preheader ], [ %dec90, %for.inc89 ]
  %46 = phi i32 [ %42, %for.cond20.preheader ], [ %sub24, %for.inc89 ]
  %ph8.0335 = phi i32* [ %arrayidx, %for.cond20.preheader ], [ %scevgep, %for.inc89 ]
  %sub24 = add i32 %46, -75
  store i32 %sub24, i32* %g, align 4, !tbaa !1
  store i32 1, i32* %jh, align 4, !tbaa !1
  %idxprom28 = zext i32 %45 to i64
  br label %for.body27

for.cond25.loopexit:                              ; preds = %for.body39
  %47 = add i32 %n2.promoted, 5
  store i32 %47, i32* %n2, align 4, !tbaa !1
  store i32 6, i32* %jx, align 4, !tbaa !1
  %arrayidx52.le = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* %gq, i64 0, i64 %indvars.iv.next374, i64 4
  %48 = trunc i64 %indvars.iv.next374 to i32
  store i32 %48, i32* %jh, align 4, !tbaa !1
  %exitcond376 = icmp eq i64 %indvars.iv.next374, 22
  br i1 %exitcond376, label %for.inc89, label %for.body27

for.body27:                                       ; preds = %for.cond25.loopexit, %for.body22
  %n2.promoted = phi i32 [ %n2.promoted.pre, %for.body22 ], [ %47, %for.cond25.loopexit ]
  %indvars.iv373 = phi i64 [ 1, %for.body22 ], [ %indvars.iv.next374, %for.cond25.loopexit ]
  %ph8.1334 = phi i32* [ %ph8.0335, %for.body22 ], [ %arrayidx52.le, %for.cond25.loopexit ]
  %indvars.iv.next374 = add nuw nsw i64 %indvars.iv373, 1
  %arrayidx32 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* %u, i64 0, i64 %indvars.iv.next374, i64 %idxprom28
  %49 = load i32, i32* %arrayidx32, align 4, !tbaa !1
  %50 = add nsw i64 %indvars.iv373, -1
  %arrayidx35 = getelementptr inbounds [100 x i32], [100 x i32]* %pz, i64 0, i64 %50
  %51 = load i32, i32* %arrayidx35, align 4, !tbaa !1
  %add36 = add i32 %51, %49
  store i32 %add36, i32* %arrayidx35, align 4, !tbaa !1
  %52 = load i32, i32* %ph8.1334, align 4, !tbaa !1
  store i32 %52, i32* %n, align 4, !tbaa !1
  store i32 1, i32* %jx, align 4, !tbaa !1
  %.pre = load i32, i32* %arrayidx42.phi.trans.insert, align 16, !tbaa !1
  br label %for.body39

for.body39:                                       ; preds = %for.body39, %for.body27
  %53 = phi i32 [ %.pre, %for.body27 ], [ %sub58, %for.body39 ]
  %indvars.iv369 = phi i64 [ 1, %for.body27 ], [ %indvars.iv.next370, %for.body39 ]
  %inc332 = phi i32 [ %n2.promoted, %for.body27 ], [ %inc, %for.body39 ]
  %54 = add nsw i64 %indvars.iv369, -1
  %indvars.iv.next370 = add nuw nsw i64 %indvars.iv369, 1
  %arrayidx45 = getelementptr inbounds [100 x i32], [100 x i32]* %pz, i64 0, i64 %indvars.iv.next370
  %55 = load i32, i32* %arrayidx45, align 4, !tbaa !1
  %sub46 = sub i32 %55, %53
  store i32 %sub46, i32* %arrayidx45, align 4, !tbaa !1
  %arrayidx55 = getelementptr inbounds [100 x i32], [100 x i32]* %b, i64 0, i64 %54
  %56 = load i32, i32* %arrayidx55, align 4, !tbaa !1
  %arrayidx57 = getelementptr inbounds [100 x i32], [100 x i32]* %t, i64 0, i64 %indvars.iv369
  %57 = load i32, i32* %arrayidx57, align 4, !tbaa !1
  %sub58 = sub i32 %57, %56
  store i32 %sub58, i32* %arrayidx57, align 4, !tbaa !1
  %inc = add i32 %inc332, 1
  %arrayidx61 = getelementptr inbounds [100 x i32], [100 x i32]* %pz, i64 0, i64 %54
  %58 = load i32, i32* %arrayidx61, align 4, !tbaa !1
  %sub62 = sub i32 %58, %inc332
  store i32 %sub62, i32* %arrayidx61, align 4, !tbaa !1
  %arrayidx73 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* %c, i64 0, i64 %50, i64 %indvars.iv369
  %59 = load i32, i32* %arrayidx73, align 4, !tbaa !1
  %arrayidx81 = getelementptr inbounds [100 x [100 x [100 x i32]]], [100 x [100 x [100 x i32]]]* %j5, i64 0, i64 %54, i64 %indvars.iv369, i64 %50
  %60 = load i32, i32* %arrayidx81, align 4, !tbaa !1
  %add82 = add i32 %60, %59
  store i32 %add82, i32* %arrayidx81, align 4, !tbaa !1
  %exitcond372 = icmp eq i64 %indvars.iv.next370, 6
  br i1 %exitcond372, label %for.cond25.loopexit, label %for.body39

for.inc89:                                        ; preds = %for.cond25.loopexit
  %dec90 = add i32 %45, -1
  store i32 %dec90, i32* %j, align 4, !tbaa !1
  %cmp21 = icmp ugt i32 %dec90, 1
  br i1 %cmp21, label %for.body22, label %for.body.i227.preheader

for.body.i227.preheader:                          ; preds = %for.inc89
  br label %for.body.i227

for.body.i227:                                    ; preds = %for.body.i227.preheader, %for.body.i227
  %indvars.iv366 = phi i64 [ %indvars.iv.next367, %for.body.i227 ], [ 0, %for.body.i227.preheader ]
  %sum.0.i217330 = phi i32 [ %add.i225, %for.body.i227 ], [ 0, %for.body.i227.preheader ]
  %rem.i220 = and i64 %indvars.iv366, 1
  %cmp1.i221 = icmp eq i64 %rem.i220, 0
  %arrayidx.i223 = getelementptr inbounds [100 x i32], [100 x i32]* %t, i64 0, i64 %indvars.iv366
  %61 = load i32, i32* %arrayidx.i223, align 4, !tbaa !1
  %sub.i224 = sub i32 0, %61
  %62 = select i1 %cmp1.i221, i32 %61, i32 %sub.i224
  %add.i225 = add i32 %62, %sum.0.i217330
  %indvars.iv.next367 = add nuw nsw i64 %indvars.iv366, 1
  %exitcond368 = icmp eq i64 %indvars.iv.next367, 100
  br i1 %exitcond368, label %for.body.i214.preheader, label %for.body.i227

for.body.i214.preheader:                          ; preds = %for.body.i227
  br label %for.body.i214

for.body.i214:                                    ; preds = %for.body.i214.preheader, %for.body.i214
  %indvars.iv363 = phi i64 [ %indvars.iv.next364, %for.body.i214 ], [ 0, %for.body.i214.preheader ]
  %sum.0.i204328 = phi i32 [ %add.i212, %for.body.i214 ], [ 0, %for.body.i214.preheader ]
  %rem.i207 = and i64 %indvars.iv363, 1
  %cmp1.i208 = icmp eq i64 %rem.i207, 0
  %arrayidx.i210 = getelementptr inbounds [100 x i32], [100 x i32]* %ml, i64 0, i64 %indvars.iv363
  %63 = load i32, i32* %arrayidx.i210, align 4, !tbaa !1
  %sub.i211 = sub i32 0, %63
  %64 = select i1 %cmp1.i208, i32 %63, i32 %sub.i211
  %add.i212 = add i32 %64, %sum.0.i204328
  %indvars.iv.next364 = add nuw nsw i64 %indvars.iv363, 1
  %exitcond365 = icmp eq i64 %indvars.iv.next364, 100
  br i1 %exitcond365, label %for.body.i201.preheader, label %for.body.i214

for.body.i201.preheader:                          ; preds = %for.body.i214
  br label %for.body.i201

for.body.i201:                                    ; preds = %for.body.i201.preheader, %for.body.i201
  %indvars.iv360 = phi i64 [ %indvars.iv.next361, %for.body.i201 ], [ 0, %for.body.i201.preheader ]
  %sum.0.i191326 = phi i32 [ %add.i199, %for.body.i201 ], [ 0, %for.body.i201.preheader ]
  %rem.i194 = and i64 %indvars.iv360, 1
  %cmp1.i195 = icmp eq i64 %rem.i194, 0
  %arrayidx.i197 = getelementptr inbounds [100 x i32], [100 x i32]* %pz, i64 0, i64 %indvars.iv360
  %65 = load i32, i32* %arrayidx.i197, align 4, !tbaa !1
  %sub.i198 = sub i32 0, %65
  %66 = select i1 %cmp1.i195, i32 %65, i32 %sub.i198
  %add.i199 = add i32 %66, %sum.0.i191326
  %indvars.iv.next361 = add nuw nsw i64 %indvars.iv360, 1
  %exitcond362 = icmp eq i64 %indvars.iv.next361, 100
  br i1 %exitcond362, label %for.body.i188.preheader, label %for.body.i201

for.body.i188.preheader:                          ; preds = %for.body.i201
  br label %for.body.i188

for.body.i188:                                    ; preds = %for.body.i188.preheader, %for.body.i188
  %indvars.iv357 = phi i64 [ %indvars.iv.next358, %for.body.i188 ], [ 0, %for.body.i188.preheader ]
  %sum.0.i178324 = phi i32 [ %add.i186, %for.body.i188 ], [ 0, %for.body.i188.preheader ]
  %rem.i181 = and i64 %indvars.iv357, 1
  %cmp1.i182 = icmp eq i64 %rem.i181, 0
  %arrayidx.i184 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* %u, i64 0, i64 0, i64 %indvars.iv357
  %67 = load i32, i32* %arrayidx.i184, align 4, !tbaa !1
  %sub.i185 = sub i32 0, %67
  %68 = select i1 %cmp1.i182, i32 %67, i32 %sub.i185
  %add.i186 = add i32 %68, %sum.0.i178324
  %indvars.iv.next358 = add nuw nsw i64 %indvars.iv357, 1
  %exitcond359 = icmp eq i64 %indvars.iv.next358, 10000
  br i1 %exitcond359, label %for.body.i175.preheader, label %for.body.i188

for.body.i175.preheader:                          ; preds = %for.body.i188
  br label %for.body.i175

for.body.i175:                                    ; preds = %for.body.i175.preheader, %for.body.i175
  %indvars.iv354 = phi i64 [ %indvars.iv.next355, %for.body.i175 ], [ 0, %for.body.i175.preheader ]
  %sum.0.i165322 = phi i32 [ %add.i173, %for.body.i175 ], [ 0, %for.body.i175.preheader ]
  %rem.i168 = and i64 %indvars.iv354, 1
  %cmp1.i169 = icmp eq i64 %rem.i168, 0
  %arrayidx.i171 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* %gq, i64 0, i64 0, i64 %indvars.iv354
  %69 = load i32, i32* %arrayidx.i171, align 4, !tbaa !1
  %sub.i172 = sub i32 0, %69
  %70 = select i1 %cmp1.i169, i32 %69, i32 %sub.i172
  %add.i173 = add i32 %70, %sum.0.i165322
  %indvars.iv.next355 = add nuw nsw i64 %indvars.iv354, 1
  %exitcond356 = icmp eq i64 %indvars.iv.next355, 10000
  br i1 %exitcond356, label %for.body.i162.preheader, label %for.body.i175

for.body.i162.preheader:                          ; preds = %for.body.i175
  br label %for.body.i162

for.body.i162:                                    ; preds = %for.body.i162.preheader, %for.body.i162
  %indvars.iv351 = phi i64 [ %indvars.iv.next352, %for.body.i162 ], [ 0, %for.body.i162.preheader ]
  %sum.0.i152320 = phi i32 [ %add.i160, %for.body.i162 ], [ 0, %for.body.i162.preheader ]
  %rem.i155 = and i64 %indvars.iv351, 1
  %cmp1.i156 = icmp eq i64 %rem.i155, 0
  %arrayidx.i158 = getelementptr inbounds [100 x i32], [100 x i32]* %b, i64 0, i64 %indvars.iv351
  %71 = load i32, i32* %arrayidx.i158, align 4, !tbaa !1
  %sub.i159 = sub i32 0, %71
  %72 = select i1 %cmp1.i156, i32 %71, i32 %sub.i159
  %add.i160 = add i32 %72, %sum.0.i152320
  %indvars.iv.next352 = add nuw nsw i64 %indvars.iv351, 1
  %exitcond353 = icmp eq i64 %indvars.iv.next352, 100
  br i1 %exitcond353, label %for.body.i149.preheader, label %for.body.i162

for.body.i149.preheader:                          ; preds = %for.body.i162
  br label %for.body.i149

for.body.i149:                                    ; preds = %for.body.i149.preheader, %for.body.i149
  %indvars.iv348 = phi i64 [ %indvars.iv.next349, %for.body.i149 ], [ 0, %for.body.i149.preheader ]
  %sum.0.i139318 = phi i32 [ %add.i147, %for.body.i149 ], [ 0, %for.body.i149.preheader ]
  %rem.i142 = and i64 %indvars.iv348, 1
  %cmp1.i143 = icmp eq i64 %rem.i142, 0
  %arrayidx.i145 = getelementptr inbounds [100 x [100 x [100 x i32]]], [100 x [100 x [100 x i32]]]* %j5, i64 0, i64 0, i64 0, i64 %indvars.iv348
  %73 = load i32, i32* %arrayidx.i145, align 4, !tbaa !1
  %sub.i146 = sub i32 0, %73
  %74 = select i1 %cmp1.i143, i32 %73, i32 %sub.i146
  %add.i147 = add i32 %74, %sum.0.i139318
  %indvars.iv.next349 = add nuw nsw i64 %indvars.iv348, 1
  %exitcond350 = icmp eq i64 %indvars.iv.next349, 1000000
  br i1 %exitcond350, label %for.body.i137.preheader, label %for.body.i149

for.body.i137.preheader:                          ; preds = %for.body.i149
  br label %for.body.i137

for.body.i137:                                    ; preds = %for.body.i137.preheader, %for.body.i137
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body.i137 ], [ 0, %for.body.i137.preheader ]
  %sum.0.i316 = phi i32 [ %add.i, %for.body.i137 ], [ 0, %for.body.i137.preheader ]
  %rem.i132 = and i64 %indvars.iv, 1
  %cmp1.i133 = icmp eq i64 %rem.i132, 0
  %arrayidx.i135 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* %c, i64 0, i64 0, i64 %indvars.iv
  %75 = load i32, i32* %arrayidx.i135, align 4, !tbaa !1
  %sub.i = sub i32 0, %75
  %76 = select i1 %cmp1.i133, i32 %75, i32 %sub.i
  %add.i = add i32 %76, %sum.0.i316
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 10000
  br i1 %exitcond, label %checkSum.exit, label %for.body.i137

checkSum.exit:                                    ; preds = %for.body.i137
  %add100 = sub i32 %41, %sub24
  %sub103 = add i32 %add100, %add12
  %add106 = add i32 %sub103, %52
  %sub109 = sub i32 %add106, %47
  %add112 = add i32 %sub109, %add.i225
  %sub115 = add i32 %add112, %add.i212
  %add118 = sub i32 %sub115, %add.i199
  %add92 = add i32 %add118, %add.i186
  %sub93 = sub i32 %add92, %add.i173
  %add94 = add i32 %sub93, %add.i160
  %sub95 = sub i32 %add94, %add.i147
  %add119 = add i32 %sub95, %add.i
  %call120 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([10 x i8], [10 x i8]* @.str.3, i64 0, i64 0), i32 %add119)
  call void @llvm.lifetime.end(i64 40000, i8* nonnull %16) #6
  call void @llvm.lifetime.end(i64 4000000, i8* %15) #6
  call void @llvm.lifetime.end(i64 400, i8* %14) #6
  call void @llvm.lifetime.end(i64 40000, i8* %13) #6
  call void @llvm.lifetime.end(i64 40000, i8* %12) #6
  call void @llvm.lifetime.end(i64 400, i8* %11) #6
  call void @llvm.lifetime.end(i64 400, i8* %10) #6
  call void @llvm.lifetime.end(i64 400, i8* %9) #6
  call void @llvm.lifetime.end(i64 4, i8* %8) #6
  call void @llvm.lifetime.end(i64 4, i8* %7) #6
  call void @llvm.lifetime.end(i64 4, i8* %6) #6
  call void @llvm.lifetime.end(i64 4, i8* %5) #6
  call void @llvm.lifetime.end(i64 4, i8* %4) #6
  call void @llvm.lifetime.end(i64 4, i8* %3) #6
  call void @llvm.lifetime.end(i64 4, i8* %2) #6
  call void @llvm.lifetime.end(i64 4, i8* %1) #6
  call void @llvm.lifetime.end(i64 4, i8* %0) #6
  ret i32 0
}

; Function Attrs: nounwind
declare noalias %struct._IO_FILE* @fopen(i8* nocapture readonly, i8* nocapture readonly) local_unnamed_addr #4

declare i32 @__isoc99_fscanf(%struct._IO_FILE*, i8*, ...) local_unnamed_addr #5

; Function Attrs: nounwind
declare i32 @printf(i8* nocapture readonly, ...) local_unnamed_addr #4

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { norecurse nounwind readonly uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #5 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #6 = { nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (trunk 20746) (llvm/branches/loopopt 20781)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
