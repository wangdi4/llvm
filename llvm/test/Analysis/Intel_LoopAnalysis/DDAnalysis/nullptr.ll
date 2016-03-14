;
;  There was a compfail in bzip/blocksort in DD for nullptr 
;  
;  compillation is okay if it starts to dump DDG for last function mainGtU 
 
; RUN:  opt < %s  -loop-simplify  -hir-ssa-deconstruction | opt  -dda  -dda-verify=Region  -analyze  | FileCheck %s 
; CHECK: 'Data Dependence Analysis' for function 'mainGtU' 
; CHECK: DD graph for function: 


; ModuleID = 'blocksort.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct._IO_FILE = type { i32, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, %struct._IO_marker*, %struct._IO_FILE*, i32, i32, i64, i16, i8, [1 x i8], i8*, i64, i8*, i8*, i8*, i8*, i64, i32, [20 x i8] }
%struct._IO_marker = type { %struct._IO_marker*, %struct._IO_FILE*, i32 }
%struct.EState = type { %struct.bz_stream*, i32, i32, i32, i32*, i32*, i32*, i32, i32*, i8*, i16*, i8*, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, [256 x i8], [256 x i8], i32, i32, i32, i32, i32, i32, i32, i32, [258 x i32], [18002 x i8], [18002 x i8], [6 x [258 x i8]], [6 x [258 x i32]], [6 x [258 x i32]], [258 x [4 x i32]] }
%struct.bz_stream = type { i8*, i32, i32, i32, i8*, i32, i32, i32, i8*, i8* (i8*, i32, i32)*, void (i8*, i8*)*, i8* }

@stderr = external global %struct._IO_FILE*, align 8
@.str = private unnamed_addr constant [38 x i8] c"      %d work, %d block, ratio %5.2f\0A\00", align 1
@.str.1 = private unnamed_addr constant [54 x i8] c"    too repetitive; using fallback sorting algorithm\0A\00", align 1
@.str.2 = private unnamed_addr constant [28 x i8] c"        bucket sorting ...\0A\00", align 1
@.str.3 = private unnamed_addr constant [23 x i8] c"        depth %6d has \00", align 1
@.str.4 = private unnamed_addr constant [24 x i8] c"%6d unresolved strings\0A\00", align 1
@.str.5 = private unnamed_addr constant [34 x i8] c"        reconstructing block ...\0A\00", align 1
@.str.6 = private unnamed_addr constant [34 x i8] c"        main sort initialise ...\0A\00", align 1
@.str.7 = private unnamed_addr constant [48 x i8] c"        qsort [0x%x, 0x%x]   done %d   this %d\0A\00", align 1
@.str.8 = private unnamed_addr constant [44 x i8] c"        %d pointers, %d sorted, %d scanned\0A\00", align 1
@incs = internal unnamed_addr constant [14 x i32] [i32 1, i32 4, i32 13, i32 40, i32 121, i32 364, i32 1093, i32 3280, i32 9841, i32 29524, i32 88573, i32 265720, i32 797161, i32 2391484], align 16

; Function Attrs: nounwind uwtable
define void @BZ2_blockSort(%struct.EState* nocapture %s) #0 {
entry:
  %budget = alloca i32, align 4
  %ptr1 = getelementptr inbounds %struct.EState, %struct.EState* %s, i64 0, i32 8
  %0 = load i32*, i32** %ptr1, align 8, !tbaa !1
  %block2 = getelementptr inbounds %struct.EState, %struct.EState* %s, i64 0, i32 9
  %1 = load i8*, i8** %block2, align 8, !tbaa !7
  %ftab3 = getelementptr inbounds %struct.EState, %struct.EState* %s, i64 0, i32 6
  %2 = load i32*, i32** %ftab3, align 8, !tbaa !8
  %nblock4 = getelementptr inbounds %struct.EState, %struct.EState* %s, i64 0, i32 17
  %3 = load i32, i32* %nblock4, align 4, !tbaa !9
  %verbosity = getelementptr inbounds %struct.EState, %struct.EState* %s, i64 0, i32 28
  %4 = load i32, i32* %verbosity, align 8, !tbaa !10
  %workFactor = getelementptr inbounds %struct.EState, %struct.EState* %s, i64 0, i32 12
  %5 = load i32, i32* %workFactor, align 8, !tbaa !11
  %6 = bitcast i32* %budget to i8*
  call void @llvm.lifetime.start(i64 4, i8* %6) #7
  %cmp = icmp slt i32 %3, 10000
  br i1 %cmp, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  %arr1 = getelementptr inbounds %struct.EState, %struct.EState* %s, i64 0, i32 4
  %7 = load i32*, i32** %arr1, align 8, !tbaa !12
  %arr2 = getelementptr inbounds %struct.EState, %struct.EState* %s, i64 0, i32 5
  %8 = load i32*, i32** %arr2, align 8, !tbaa !13
  tail call fastcc void @fallbackSort(i32* %7, i32* %8, i32* %2, i32 %3, i32 %4)
  br label %if.end33

if.else:                                          ; preds = %entry
  %add = add nsw i32 %3, 34
  %and = and i32 %add, 1
  %tobool = icmp eq i32 %and, 0
  %inc = add nsw i32 %3, 35
  %add.inc = select i1 %tobool, i32 %add, i32 %inc
  %idxprom = sext i32 %add.inc to i64
  %arrayidx = getelementptr inbounds i8, i8* %1, i64 %idxprom
  %9 = bitcast i8* %arrayidx to i16*
  %cmp6 = icmp slt i32 %5, 1
  br i1 %cmp6, label %if.end11, label %if.end8

if.end8:                                          ; preds = %if.else
  %cmp9 = icmp sgt i32 %5, 100
  %.wfact.0 = select i1 %cmp9, i32 100, i32 %5
  %phitmp = add i32 %.wfact.0, -1
  %phitmp99 = sdiv i32 %phitmp, 3
  br label %if.end11

if.end11:                                         ; preds = %if.end8, %if.else
  %wfact.1 = phi i32 [ 0, %if.else ], [ %phitmp99, %if.end8 ]
  %mul = mul nsw i32 %wfact.1, %3
  store i32 %mul, i32* %budget, align 4, !tbaa !14
  call fastcc void @mainSort(i32* %0, i8* %1, i16* %9, i32* %2, i32 %3, i32 %4, i32* nonnull %budget)
  %cmp12 = icmp sgt i32 %4, 2
  br i1 %cmp12, label %if.then13, label %if.end21

if.then13:                                        ; preds = %if.end11
  %10 = load %struct._IO_FILE*, %struct._IO_FILE** @stderr, align 8, !tbaa !15
  %11 = load i32, i32* %budget, align 4, !tbaa !14
  %sub14 = sub nsw i32 %mul, %11
  %conv = sitofp i32 %sub14 to float
  %12 = sitofp i32 %3 to float
  %div19 = fdiv float %conv, %12
  %conv20 = fpext float %div19 to double
  %call = tail call i32 (%struct._IO_FILE*, i8*, ...) @fprintf(%struct._IO_FILE* %10, i8* nonnull getelementptr inbounds ([38 x i8], [38 x i8]* @.str, i64 0, i64 0), i32 %sub14, i32 %3, double %conv20) #8
  br label %if.end21

if.end21:                                         ; preds = %if.then13, %if.end11
  %13 = load i32, i32* %budget, align 4, !tbaa !14
  %cmp22 = icmp slt i32 %13, 0
  br i1 %cmp22, label %if.then24, label %if.end33

if.then24:                                        ; preds = %if.end21
  %cmp25 = icmp sgt i32 %4, 1
  br i1 %cmp25, label %if.then27, label %if.end29

if.then27:                                        ; preds = %if.then24
  %14 = load %struct._IO_FILE*, %struct._IO_FILE** @stderr, align 8, !tbaa !15
  %15 = tail call i64 @fwrite(i8* nonnull getelementptr inbounds ([54 x i8], [54 x i8]* @.str.1, i64 0, i64 0), i64 53, i64 1, %struct._IO_FILE* %14) #8
  br label %if.end29

if.end29:                                         ; preds = %if.then27, %if.then24
  %arr130 = getelementptr inbounds %struct.EState, %struct.EState* %s, i64 0, i32 4
  %16 = load i32*, i32** %arr130, align 8, !tbaa !12
  %arr231 = getelementptr inbounds %struct.EState, %struct.EState* %s, i64 0, i32 5
  %17 = load i32*, i32** %arr231, align 8, !tbaa !13
  tail call fastcc void @fallbackSort(i32* %16, i32* %17, i32* %2, i32 %3, i32 %4)
  br label %if.end33

if.end33:                                         ; preds = %if.end21, %if.end29, %if.then
  %origPtr = getelementptr inbounds %struct.EState, %struct.EState* %s, i64 0, i32 7
  store i32 -1, i32* %origPtr, align 8, !tbaa !16
  %18 = load i32, i32* %nblock4, align 4, !tbaa !9
  %cmp35101 = icmp sgt i32 %18, 0
  br i1 %cmp35101, label %for.body, label %for.endthread-pre-split

for.body:                                         ; preds = %if.end33, %for.inc
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.inc ], [ 0, %if.end33 ]
  %arrayidx38 = getelementptr inbounds i32, i32* %0, i64 %indvars.iv
  %19 = load i32, i32* %arrayidx38, align 4, !tbaa !14
  %cmp39 = icmp eq i32 %19, 0
  br i1 %cmp39, label %if.then41, label %for.inc

if.then41:                                        ; preds = %for.body
  %20 = trunc i64 %indvars.iv to i32
  store i32 %20, i32* %origPtr, align 8, !tbaa !16
  br label %for.end

for.inc:                                          ; preds = %for.body
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %21 = load i32, i32* %nblock4, align 4, !tbaa !9
  %22 = sext i32 %21 to i64
  %cmp35 = icmp slt i64 %indvars.iv.next, %22
  br i1 %cmp35, label %for.body, label %for.endthread-pre-split

for.endthread-pre-split:                          ; preds = %for.inc, %if.end33
  %.pr = load i32, i32* %origPtr, align 8, !tbaa !16
  br label %for.end

for.end:                                          ; preds = %for.endthread-pre-split, %if.then41
  %23 = phi i32 [ %.pr, %for.endthread-pre-split ], [ %20, %if.then41 ]
  %cmp46 = icmp eq i32 %23, -1
  br i1 %cmp46, label %if.then48, label %if.end49

if.then48:                                        ; preds = %for.end
  tail call void @BZ2_bz__AssertH__fail(i32 1003) #7
  br label %if.end49

if.end49:                                         ; preds = %for.end, %if.then48
  call void @llvm.lifetime.end(i64 4, i8* %6) #7
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: nounwind uwtable
define internal fastcc void @fallbackSort(i32* nocapture %fmap, i32* nocapture %eclass, i32* nocapture %bhtab, i32 %nblock, i32 %verb) unnamed_addr #0 {
entry:
  %bhtab495 = bitcast i32* %bhtab to i8*
  %ftab = alloca [257 x i32], align 16
  %0 = bitcast [257 x i32]* %ftab to i8*
  %ftabCopy = alloca [256 x i32], align 16
  %1 = bitcast [256 x i32]* %ftabCopy to i8*
  call void @llvm.lifetime.start(i64 1028, i8* %0) #7
  call void @llvm.lifetime.start(i64 1024, i8* %1) #7
  %2 = bitcast i32* %eclass to i8*
  %cmp = icmp sgt i32 %verb, 3
  br i1 %cmp, label %if.then, label %for.cond2.preheader

if.then:                                          ; preds = %entry
  %3 = load %struct._IO_FILE*, %struct._IO_FILE** @stderr, align 8, !tbaa !15
  %4 = tail call i64 @fwrite(i8* nonnull getelementptr inbounds ([28 x i8], [28 x i8]* @.str.2, i64 0, i64 0), i64 27, i64 1, %struct._IO_FILE* %3) #8
  br label %for.cond2.preheader

for.cond2.preheader:                              ; preds = %entry, %if.then
  call void @llvm.memset.p0i8.i64(i8* %0, i8 0, i64 1028, i32 16, i1 false)
  %cmp3459 = icmp sgt i32 %nblock, 0
  br i1 %cmp3459, label %for.body4, label %for.body25.preheader

for.body4:                                        ; preds = %for.cond2.preheader, %for.body4
  %indvars.iv488 = phi i64 [ %indvars.iv.next489, %for.body4 ], [ 0, %for.cond2.preheader ]
  %arrayidx6 = getelementptr inbounds i8, i8* %2, i64 %indvars.iv488
  %5 = load i8, i8* %arrayidx6, align 1, !tbaa !17
  %idxprom7 = zext i8 %5 to i64
  %arrayidx8 = getelementptr inbounds [257 x i32], [257 x i32]* %ftab, i64 0, i64 %idxprom7
  %6 = load i32, i32* %arrayidx8, align 4, !tbaa !14
  %inc9 = add nsw i32 %6, 1
  store i32 %inc9, i32* %arrayidx8, align 4, !tbaa !14
  %indvars.iv.next489 = add nuw nsw i64 %indvars.iv488, 1
  %lftr.wideiv490 = trunc i64 %indvars.iv.next489 to i32
  %exitcond491 = icmp eq i32 %lftr.wideiv490, %nblock
  br i1 %exitcond491, label %for.body25.preheader, label %for.body4

for.body25.preheader:                             ; preds = %for.body4, %for.cond2.preheader
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %1, i8* nonnull %0, i64 1024, i32 16, i1 false)
  br label %for.body25

for.cond33.preheader:                             ; preds = %for.body25
  %cmp34455 = icmp sgt i32 %nblock, 0
  br i1 %cmp34455, label %for.body35, label %for.end47

for.body25:                                       ; preds = %for.body25, %for.body25.preheader
  %indvars.iv481 = phi i64 [ 1, %for.body25.preheader ], [ %indvars.iv.next482, %for.body25 ]
  %7 = add nsw i64 %indvars.iv481, -1
  %arrayidx27 = getelementptr inbounds [257 x i32], [257 x i32]* %ftab, i64 0, i64 %7
  %8 = load i32, i32* %arrayidx27, align 4, !tbaa !14
  %arrayidx29 = getelementptr inbounds [257 x i32], [257 x i32]* %ftab, i64 0, i64 %indvars.iv481
  %9 = load i32, i32* %arrayidx29, align 4, !tbaa !14
  %add = add nsw i32 %9, %8
  store i32 %add, i32* %arrayidx29, align 4, !tbaa !14
  %indvars.iv.next482 = add nuw nsw i64 %indvars.iv481, 1
  %exitcond484 = icmp eq i64 %indvars.iv.next482, 257
  br i1 %exitcond484, label %for.cond33.preheader, label %for.body25

for.body35:                                       ; preds = %for.cond33.preheader, %for.body35
  %indvars.iv477 = phi i64 [ %indvars.iv.next478, %for.body35 ], [ 0, %for.cond33.preheader ]
  %arrayidx37 = getelementptr inbounds i8, i8* %2, i64 %indvars.iv477
  %10 = load i8, i8* %arrayidx37, align 1, !tbaa !17
  %idxprom38 = zext i8 %10 to i64
  %arrayidx39 = getelementptr inbounds [257 x i32], [257 x i32]* %ftab, i64 0, i64 %idxprom38
  %11 = load i32, i32* %arrayidx39, align 4, !tbaa !14
  %sub40 = add nsw i32 %11, -1
  store i32 %sub40, i32* %arrayidx39, align 4, !tbaa !14
  %idxprom43 = sext i32 %sub40 to i64
  %arrayidx44 = getelementptr inbounds i32, i32* %fmap, i64 %idxprom43
  %12 = trunc i64 %indvars.iv477 to i32
  store i32 %12, i32* %arrayidx44, align 4, !tbaa !14
  %indvars.iv.next478 = add nuw nsw i64 %indvars.iv477, 1
  %lftr.wideiv479 = trunc i64 %indvars.iv.next478 to i32
  %exitcond480 = icmp eq i32 %lftr.wideiv479, %nblock
  br i1 %exitcond480, label %for.end47, label %for.body35

for.end47:                                        ; preds = %for.body35, %for.cond33.preheader
  %cmp50453 = icmp sgt i32 %nblock, -64
  br i1 %cmp50453, label %for.body61.preheader.loopexit, label %for.body61

for.body61.preheader.loopexit:                    ; preds = %for.end47
  %div = sdiv i32 %nblock, 32
  %13 = add nsw i32 %div, 1
  %14 = icmp sgt i32 %13, 0
  %smax = select i1 %14, i32 %13, i32 0
  %15 = zext i32 %smax to i64
  %16 = shl nuw nsw i64 %15, 2
  %17 = add nuw nsw i64 %16, 4
  call void @llvm.memset.p0i8.i64(i8* %bhtab495, i8 0, i64 %17, i32 4, i1 false)
  br label %for.body61

for.body61:                                       ; preds = %for.end47, %for.body61.preheader.loopexit, %for.body61
  %indvars.iv472 = phi i64 [ %indvars.iv.next473, %for.body61 ], [ 0, %for.body61.preheader.loopexit ], [ 0, %for.end47 ]
  %arrayidx63 = getelementptr inbounds [257 x i32], [257 x i32]* %ftab, i64 0, i64 %indvars.iv472
  %18 = load i32, i32* %arrayidx63, align 4, !tbaa !14
  %and = and i32 %18, 31
  %shl = shl i32 1, %and
  %shr = ashr i32 %18, 5
  %idxprom66 = sext i32 %shr to i64
  %arrayidx67 = getelementptr inbounds i32, i32* %bhtab, i64 %idxprom66
  %19 = load i32, i32* %arrayidx67, align 4, !tbaa !14
  %or = or i32 %shl, %19
  store i32 %or, i32* %arrayidx67, align 4, !tbaa !14
  %indvars.iv.next473 = add nuw nsw i64 %indvars.iv472, 1
  %exitcond474 = icmp eq i64 %indvars.iv.next473, 256
  br i1 %exitcond474, label %for.body74, label %for.body61

while.body.preheader:                             ; preds = %for.body74
  %cmp105445 = icmp sgt i32 %nblock, 0
  br label %while.body

for.body74:                                       ; preds = %for.body61, %for.body74
  %i.7451 = phi i32 [ %inc97, %for.body74 ], [ 0, %for.body61 ]
  %mul = shl nsw i32 %i.7451, 1
  %add75 = add nsw i32 %mul, %nblock
  %and76 = and i32 %add75, 31
  %shl77 = shl i32 1, %and76
  %shr80 = ashr i32 %add75, 5
  %idxprom81 = sext i32 %shr80 to i64
  %arrayidx82 = getelementptr inbounds i32, i32* %bhtab, i64 %idxprom81
  %20 = load i32, i32* %arrayidx82, align 4, !tbaa !14
  %or83 = or i32 %20, %shl77
  store i32 %or83, i32* %arrayidx82, align 4, !tbaa !14
  %add86 = add nsw i32 %add75, 1
  %and87 = and i32 %add86, 31
  %shl88 = shl i32 1, %and87
  %neg = xor i32 %shl88, -1
  %shr92 = ashr i32 %add86, 5
  %idxprom93 = sext i32 %shr92 to i64
  %arrayidx94 = getelementptr inbounds i32, i32* %bhtab, i64 %idxprom93
  %21 = load i32, i32* %arrayidx94, align 4, !tbaa !14
  %and95 = and i32 %21, %neg
  store i32 %and95, i32* %arrayidx94, align 4, !tbaa !14
  %inc97 = add nuw nsw i32 %i.7451, 1
  %exitcond471 = icmp eq i32 %inc97, 32
  br i1 %exitcond471, label %while.body.preheader, label %for.body74

while.body:                                       ; preds = %while.body.preheader, %if.end260
  %H.0 = phi i32 [ %mul261, %if.end260 ], [ 1, %while.body.preheader ]
  br i1 %cmp, label %if.then101, label %for.cond104.preheader

if.then101:                                       ; preds = %while.body
  %22 = load %struct._IO_FILE*, %struct._IO_FILE** @stderr, align 8, !tbaa !15
  %call102 = tail call i32 (%struct._IO_FILE*, i8*, ...) @fprintf(%struct._IO_FILE* %22, i8* nonnull getelementptr inbounds ([23 x i8], [23 x i8]* @.str.3, i64 0, i64 0), i32 %H.0) #8
  br label %for.cond104.preheader

for.cond104.preheader:                            ; preds = %if.then101, %while.body
  br i1 %cmp105445, label %for.body107, label %while.body130.outer

for.body107:                                      ; preds = %for.cond104.preheader, %for.body107
  %indvars.iv465 = phi i64 [ %indvars.iv.next466, %for.body107 ], [ 0, %for.cond104.preheader ]
  %j.0446 = phi i32 [ %j.0.i.8, %for.body107 ], [ 0, %for.cond104.preheader ]
  %23 = trunc i64 %indvars.iv465 to i32
  %shr108 = ashr i32 %23, 5
  %idxprom109 = sext i32 %shr108 to i64
  %arrayidx110 = getelementptr inbounds i32, i32* %bhtab, i64 %idxprom109
  %24 = load i32, i32* %arrayidx110, align 4, !tbaa !14
  %25 = trunc i64 %indvars.iv465 to i32
  %and111 = and i32 %25, 31
  %shl112 = shl i32 1, %and111
  %and113 = and i32 %24, %shl112
  %tobool = icmp eq i32 %and113, 0
  %26 = trunc i64 %indvars.iv465 to i32
  %j.0.i.8 = select i1 %tobool, i32 %j.0446, i32 %26
  %arrayidx117 = getelementptr inbounds i32, i32* %fmap, i64 %indvars.iv465
  %27 = load i32, i32* %arrayidx117, align 4, !tbaa !14
  %sub118 = sub i32 %27, %H.0
  %cmp119 = icmp slt i32 %sub118, 0
  %add122 = select i1 %cmp119, i32 %nblock, i32 0
  %28 = add nsw i32 %add122, %sub118
  %idxprom124 = sext i32 %28 to i64
  %arrayidx125 = getelementptr inbounds i32, i32* %eclass, i64 %idxprom124
  store i32 %j.0.i.8, i32* %arrayidx125, align 4, !tbaa !14
  %indvars.iv.next466 = add nuw nsw i64 %indvars.iv465, 1
  %lftr.wideiv467 = trunc i64 %indvars.iv.next466 to i32
  %exitcond468 = icmp eq i32 %lftr.wideiv467, %nblock
  br i1 %exitcond468, label %while.body130.outer, label %for.body107

while.body130.outer:                              ; preds = %if.then229, %for.inc251, %for.cond104.preheader, %for.body107
  %r.0.ph = phi i32 [ -1, %for.body107 ], [ -1, %for.cond104.preheader ], [ %sub222, %for.inc251 ], [ %sub222, %if.then229 ]
  %nNotDone.0.ph = phi i32 [ 0, %for.body107 ], [ 0, %for.cond104.preheader ], [ %add232, %for.inc251 ], [ %add232, %if.then229 ]
  br label %while.cond132

while.cond132:                                    ; preds = %if.end226, %while.cond132, %while.body130.outer
  %k.1.in = phi i32 [ %r.0.ph, %while.body130.outer ], [ %k.1, %while.cond132 ], [ %sub222, %if.end226 ]
  %k.1 = add nsw i32 %k.1.in, 1
  %shr133 = ashr i32 %k.1, 5
  %idxprom134 = sext i32 %shr133 to i64
  %arrayidx135 = getelementptr inbounds i32, i32* %bhtab, i64 %idxprom134
  %29 = load i32, i32* %arrayidx135, align 4, !tbaa !14
  %and136 = and i32 %k.1, 31
  %shl137 = shl i32 1, %and136
  %and138 = and i32 %shl137, %29
  %tobool139 = icmp ne i32 %and138, 0
  %tobool141 = icmp ne i32 %and136, 0
  %or.cond439 = and i1 %tobool141, %tobool139
  br i1 %or.cond439, label %while.cond132, label %while.end

while.end:                                        ; preds = %while.cond132
  %tobool150 = icmp eq i32 %and138, 0
  br i1 %tobool150, label %if.end172, label %while.cond152

while.cond152:                                    ; preds = %while.end, %while.cond152
  %k.2 = phi i32 [ %add159, %while.cond152 ], [ %k.1, %while.end ]
  %shr153 = ashr i32 %k.2, 5
  %idxprom154 = sext i32 %shr153 to i64
  %arrayidx155 = getelementptr inbounds i32, i32* %bhtab, i64 %idxprom154
  %30 = load i32, i32* %arrayidx155, align 4, !tbaa !14
  %cmp156 = icmp eq i32 %30, -1
  %add159 = add nsw i32 %k.2, 32
  br i1 %cmp156, label %while.cond152, label %while.cond161

while.cond161:                                    ; preds = %while.cond152, %while.cond161
  %k.3 = phi i32 [ %inc170, %while.cond161 ], [ %k.2, %while.cond152 ]
  %shr162 = ashr i32 %k.3, 5
  %idxprom163 = sext i32 %shr162 to i64
  %arrayidx164 = getelementptr inbounds i32, i32* %bhtab, i64 %idxprom163
  %31 = load i32, i32* %arrayidx164, align 4, !tbaa !14
  %and165 = and i32 %k.3, 31
  %shl166 = shl i32 1, %and165
  %and167 = and i32 %shl166, %31
  %tobool168 = icmp eq i32 %and167, 0
  %inc170 = add nsw i32 %k.3, 1
  br i1 %tobool168, label %if.end172, label %while.cond161

if.end172:                                        ; preds = %while.cond161, %while.end
  %k.4 = phi i32 [ %k.1, %while.end ], [ %k.3, %while.cond161 ]
  %cmp174 = icmp sgt i32 %k.4, %nblock
  br i1 %cmp174, label %while.end255, label %while.cond178

while.cond178:                                    ; preds = %if.end172, %while.cond178
  %k.5 = phi i32 [ %inc191, %while.cond178 ], [ %k.4, %if.end172 ]
  %shr179 = ashr i32 %k.5, 5
  %idxprom180 = sext i32 %shr179 to i64
  %arrayidx181 = getelementptr inbounds i32, i32* %bhtab, i64 %idxprom180
  %32 = load i32, i32* %arrayidx181, align 4, !tbaa !14
  %and182 = and i32 %k.5, 31
  %shl183 = shl i32 1, %and182
  %and184 = and i32 %shl183, %32
  %tobool185 = icmp eq i32 %and184, 0
  %tobool188 = icmp ne i32 %and182, 0
  %or.cond440 = and i1 %tobool188, %tobool185
  %inc191 = add nsw i32 %k.5, 1
  br i1 %or.cond440, label %while.cond178, label %while.end192

while.end192:                                     ; preds = %while.cond178
  br i1 %tobool185, label %while.cond201, label %if.end221

while.cond201:                                    ; preds = %while.end192, %while.cond201
  %k.6 = phi i32 [ %add208, %while.cond201 ], [ %k.5, %while.end192 ]
  %shr202 = ashr i32 %k.6, 5
  %idxprom203 = sext i32 %shr202 to i64
  %arrayidx204 = getelementptr inbounds i32, i32* %bhtab, i64 %idxprom203
  %33 = load i32, i32* %arrayidx204, align 4, !tbaa !14
  %cmp205 = icmp eq i32 %33, 0
  %add208 = add nsw i32 %k.6, 32
  br i1 %cmp205, label %while.cond201, label %while.cond210

while.cond210:                                    ; preds = %while.cond201, %while.cond210
  %k.7 = phi i32 [ %inc219, %while.cond210 ], [ %k.6, %while.cond201 ]
  %shr211 = ashr i32 %k.7, 5
  %idxprom212 = sext i32 %shr211 to i64
  %arrayidx213 = getelementptr inbounds i32, i32* %bhtab, i64 %idxprom212
  %34 = load i32, i32* %arrayidx213, align 4, !tbaa !14
  %and214 = and i32 %k.7, 31
  %shl215 = shl i32 1, %and214
  %and216 = and i32 %shl215, %34
  %lnot = icmp eq i32 %and216, 0
  %inc219 = add nsw i32 %k.7, 1
  br i1 %lnot, label %while.cond210, label %if.end221

if.end221:                                        ; preds = %while.cond210, %while.end192
  %k.8 = phi i32 [ %k.5, %while.end192 ], [ %k.7, %while.cond210 ]
  %sub222 = add nsw i32 %k.8, -1
  %cmp223 = icmp sgt i32 %k.8, %nblock
  br i1 %cmp223, label %while.end255, label %if.end226

if.end226:                                        ; preds = %if.end221
  %cmp227 = icmp sgt i32 %k.8, %k.4
  br i1 %cmp227, label %if.then229, label %while.cond132

if.then229:                                       ; preds = %if.end226
  %sub173.le = add nsw i32 %k.4, -1
  %sub173.neg = sub i32 1, %k.4
  %sub230 = add i32 %nNotDone.0.ph, 1
  %add231 = add i32 %sub230, %sub173.neg
  %add232 = add i32 %add231, %sub222
  tail call fastcc void @fallbackQSort3(i32* %fmap, i32* %eclass, i32 %sub173.le, i32 %sub222)
  %cmp234448 = icmp sgt i32 %k.4, %k.8
  br i1 %cmp234448, label %while.body130.outer, label %for.body236.preheader

for.body236.preheader:                            ; preds = %if.then229
  %35 = add i32 %k.4, -1
  %36 = sext i32 %35 to i64
  br label %for.body236

for.body236:                                      ; preds = %for.body236.preheader, %for.inc251
  %indvars.iv469 = phi i64 [ %36, %for.body236.preheader ], [ %indvars.iv.next470, %for.inc251 ]
  %i.9450 = phi i32 [ %sub173.le, %for.body236.preheader ], [ %inc252, %for.inc251 ]
  %cc.0449 = phi i32 [ -1, %for.body236.preheader ], [ %cc.1, %for.inc251 ]
  %arrayidx238 = getelementptr inbounds i32, i32* %fmap, i64 %indvars.iv469
  %37 = load i32, i32* %arrayidx238, align 4, !tbaa !14
  %idxprom239 = zext i32 %37 to i64
  %arrayidx240 = getelementptr inbounds i32, i32* %eclass, i64 %idxprom239
  %38 = load i32, i32* %arrayidx240, align 4, !tbaa !14
  %cmp241 = icmp eq i32 %cc.0449, %38
  br i1 %cmp241, label %for.inc251, label %if.then243

if.then243:                                       ; preds = %for.body236
  %39 = trunc i64 %indvars.iv469 to i32
  %and244 = and i32 %39, 31
  %shl245 = shl i32 1, %and244
  %40 = trunc i64 %indvars.iv469 to i32
  %shr246 = ashr i32 %40, 5
  %idxprom247 = sext i32 %shr246 to i64
  %arrayidx248 = getelementptr inbounds i32, i32* %bhtab, i64 %idxprom247
  %41 = load i32, i32* %arrayidx248, align 4, !tbaa !14
  %or249 = or i32 %41, %shl245
  store i32 %or249, i32* %arrayidx248, align 4, !tbaa !14
  br label %for.inc251

for.inc251:                                       ; preds = %for.body236, %if.then243
  %cc.1 = phi i32 [ %38, %if.then243 ], [ %cc.0449, %for.body236 ]
  %inc252 = add nsw i32 %i.9450, 1
  %cmp234 = icmp slt i32 %inc252, %k.8
  %indvars.iv.next470 = add nsw i64 %indvars.iv469, 1
  br i1 %cmp234, label %for.body236, label %while.body130.outer

while.end255:                                     ; preds = %if.end221, %if.end172
  br i1 %cmp, label %if.then258, label %if.end260

if.then258:                                       ; preds = %while.end255
  %42 = load %struct._IO_FILE*, %struct._IO_FILE** @stderr, align 8, !tbaa !15
  %call259 = tail call i32 (%struct._IO_FILE*, i8*, ...) @fprintf(%struct._IO_FILE* %42, i8* nonnull getelementptr inbounds ([24 x i8], [24 x i8]* @.str.4, i64 0, i64 0), i32 %nNotDone.0.ph) #8
  br label %if.end260

if.end260:                                        ; preds = %if.then258, %while.end255
  %mul261 = shl nsw i32 %H.0, 1
  %cmp262 = icmp sgt i32 %mul261, %nblock
  %cmp264 = icmp eq i32 %nNotDone.0.ph, 0
  %or.cond = or i1 %cmp262, %cmp264
  br i1 %or.cond, label %while.end268, label %while.body

while.end268:                                     ; preds = %if.end260
  br i1 %cmp, label %if.then271, label %for.cond274.preheader

if.then271:                                       ; preds = %while.end268
  %43 = load %struct._IO_FILE*, %struct._IO_FILE** @stderr, align 8, !tbaa !15
  %44 = tail call i64 @fwrite(i8* nonnull getelementptr inbounds ([34 x i8], [34 x i8]* @.str.5, i64 0, i64 0), i64 33, i64 1, %struct._IO_FILE* %43) #8
  br label %for.cond274.preheader

for.cond274.preheader:                            ; preds = %if.then271, %while.end268
  %cmp275442 = icmp sgt i32 %nblock, 0
  br i1 %cmp275442, label %while.cond278.preheader, label %if.end299

while.cond278.preheader:                          ; preds = %for.cond274.preheader, %while.end285
  %indvars.iv463 = phi i64 [ %indvars.iv.next464, %while.end285 ], [ 0, %for.cond274.preheader ]
  %j.2443 = phi i64 [ %indvars.iv, %while.end285 ], [ 0, %for.cond274.preheader ]
  %sext = shl i64 %j.2443, 32
  %45 = ashr exact i64 %sext, 32
  br label %while.cond278

while.cond278:                                    ; preds = %while.cond278, %while.cond278.preheader
  %indvars.iv = phi i64 [ %indvars.iv.next, %while.cond278 ], [ %45, %while.cond278.preheader ]
  %arrayidx280 = getelementptr inbounds [256 x i32], [256 x i32]* %ftabCopy, i64 0, i64 %indvars.iv
  %46 = load i32, i32* %arrayidx280, align 4, !tbaa !14
  %cmp281 = icmp eq i32 %46, 0
  %indvars.iv.next = add i64 %indvars.iv, 1
  br i1 %cmp281, label %while.cond278, label %while.end285

while.end285:                                     ; preds = %while.cond278
  %dec = add nsw i32 %46, -1
  store i32 %dec, i32* %arrayidx280, align 4, !tbaa !14
  %conv288 = trunc i64 %indvars.iv to i8
  %arrayidx290 = getelementptr inbounds i32, i32* %fmap, i64 %indvars.iv463
  %47 = load i32, i32* %arrayidx290, align 4, !tbaa !14
  %idxprom291 = zext i32 %47 to i64
  %arrayidx292 = getelementptr inbounds i8, i8* %2, i64 %idxprom291
  store i8 %conv288, i8* %arrayidx292, align 1, !tbaa !17
  %indvars.iv.next464 = add nuw nsw i64 %indvars.iv463, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next464 to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %nblock
  br i1 %exitcond, label %for.end295, label %while.cond278.preheader

for.end295:                                       ; preds = %while.end285
  %48 = trunc i64 %indvars.iv to i32
  %phitmp = icmp slt i32 %48, 256
  br i1 %phitmp, label %if.end299, label %if.then298

if.then298:                                       ; preds = %for.end295
  tail call void @BZ2_bz__AssertH__fail(i32 1005) #7
  br label %if.end299

if.end299:                                        ; preds = %for.cond274.preheader, %if.then298, %for.end295
  call void @llvm.lifetime.end(i64 1024, i8* %1) #7
  call void @llvm.lifetime.end(i64 1028, i8* %0) #7
  ret void
}

; Function Attrs: nounwind uwtable
define internal fastcc void @mainSort(i32* nocapture %ptr, i8* %block, i16* nocapture %quadrant, i32* %ftab, i32 %nblock, i32 %verb, i32* nocapture %budget) unnamed_addr #0 {
entry:
  %ftab891 = bitcast i32* %ftab to i8*
  %runningOrder = alloca [256 x i32], align 16
  %bigDone = alloca [256 x i8], align 16
  %bigDone888 = getelementptr inbounds [256 x i8], [256 x i8]* %bigDone, i64 0, i64 0
  %copyStart = alloca [256 x i32], align 16
  %copyEnd = alloca [256 x i32], align 16
  %0 = bitcast [256 x i32]* %runningOrder to i8*
  call void @llvm.lifetime.start(i64 1024, i8* %0) #7
  %1 = getelementptr inbounds [256 x i8], [256 x i8]* %bigDone, i64 0, i64 0
  call void @llvm.lifetime.start(i64 256, i8* %1) #7
  %2 = bitcast [256 x i32]* %copyStart to i8*
  call void @llvm.lifetime.start(i64 1024, i8* %2) #7
  %3 = bitcast [256 x i32]* %copyEnd to i8*
  call void @llvm.lifetime.start(i64 1024, i8* %3) #7
  %cmp = icmp sgt i32 %verb, 3
  br i1 %cmp, label %if.then, label %for.end

if.then:                                          ; preds = %entry
  %4 = load %struct._IO_FILE*, %struct._IO_FILE** @stderr, align 8, !tbaa !15
  %5 = tail call i64 @fwrite(i8* nonnull getelementptr inbounds ([34 x i8], [34 x i8]* @.str.6, i64 0, i64 0), i64 33, i64 1, %struct._IO_FILE* %4) #8
  br label %for.end

for.end:                                          ; preds = %entry, %if.then
  call void @llvm.memset.p0i8.i64(i8* %ftab891, i8 0, i64 262148, i32 4, i1 false)
  %6 = load i8, i8* %block, align 1, !tbaa !17
  %conv = zext i8 %6 to i32
  %shl = shl nuw nsw i32 %conv, 8
  %sub = add nsw i32 %nblock, -1
  %cmp4840 = icmp sgt i32 %sub, 2
  br i1 %cmp4840, label %for.body6.preheader, label %for.cond61.preheader

for.body6.preheader:                              ; preds = %for.end
  %7 = add i32 %nblock, -1
  %8 = sext i32 %7 to i64
  %9 = add i32 %nblock, -4
  %10 = and i32 %9, -4
  br label %for.body6

for.cond61.preheader.loopexit:                    ; preds = %for.body6
  %11 = add i32 %nblock, -5
  %12 = sub i32 %11, %10
  br label %for.cond61.preheader

for.cond61.preheader:                             ; preds = %for.cond61.preheader.loopexit, %for.end
  %i.1.lcssa = phi i32 [ %sub, %for.end ], [ %12, %for.cond61.preheader.loopexit ]
  %j.0.lcssa = phi i32 [ %shl, %for.end ], [ %or54, %for.cond61.preheader.loopexit ]
  %cmp62837 = icmp sgt i32 %i.1.lcssa, -1
  br i1 %cmp62837, label %for.body64.preheader, label %for.body83.preheader

for.body64.preheader:                             ; preds = %for.cond61.preheader
  %13 = sext i32 %i.1.lcssa to i64
  br label %for.body64

for.body6:                                        ; preds = %for.body6.preheader, %for.body6
  %indvars.iv884 = phi i64 [ %8, %for.body6.preheader ], [ %indvars.iv.next885, %for.body6 ]
  %i.1842 = phi i32 [ %sub, %for.body6.preheader ], [ %sub59, %for.body6 ]
  %j.0841 = phi i32 [ %shl, %for.body6.preheader ], [ %or54, %for.body6 ]
  %arrayidx8 = getelementptr inbounds i16, i16* %quadrant, i64 %indvars.iv884
  store i16 0, i16* %arrayidx8, align 2, !tbaa !18
  %shr = ashr i32 %j.0841, 8
  %arrayidx10 = getelementptr inbounds i8, i8* %block, i64 %indvars.iv884
  %14 = load i8, i8* %arrayidx10, align 1, !tbaa !17
  %conv12 = zext i8 %14 to i32
  %shl13 = shl nuw nsw i32 %conv12, 8
  %or = or i32 %shl13, %shr
  %idxprom14 = sext i32 %or to i64
  %arrayidx15 = getelementptr inbounds i32, i32* %ftab, i64 %idxprom14
  %15 = load i32, i32* %arrayidx15, align 4, !tbaa !14
  %inc = add i32 %15, 1
  store i32 %inc, i32* %arrayidx15, align 4, !tbaa !14
  %sub16 = add nsw i32 %i.1842, -1
  %idxprom17 = sext i32 %sub16 to i64
  %arrayidx18 = getelementptr inbounds i16, i16* %quadrant, i64 %idxprom17
  store i16 0, i16* %arrayidx18, align 2, !tbaa !18
  %shr19 = ashr i32 %or, 8
  %arrayidx22 = getelementptr inbounds i8, i8* %block, i64 %idxprom17
  %16 = load i8, i8* %arrayidx22, align 1, !tbaa !17
  %conv24 = zext i8 %16 to i32
  %shl25 = shl nuw nsw i32 %conv24, 8
  %or26 = or i32 %shl25, %shr19
  %idxprom27 = sext i32 %or26 to i64
  %arrayidx28 = getelementptr inbounds i32, i32* %ftab, i64 %idxprom27
  %17 = load i32, i32* %arrayidx28, align 4, !tbaa !14
  %inc29 = add i32 %17, 1
  store i32 %inc29, i32* %arrayidx28, align 4, !tbaa !14
  %sub30 = add nsw i32 %i.1842, -2
  %idxprom31 = sext i32 %sub30 to i64
  %arrayidx32 = getelementptr inbounds i16, i16* %quadrant, i64 %idxprom31
  store i16 0, i16* %arrayidx32, align 2, !tbaa !18
  %shr33 = ashr i32 %or26, 8
  %arrayidx36 = getelementptr inbounds i8, i8* %block, i64 %idxprom31
  %18 = load i8, i8* %arrayidx36, align 1, !tbaa !17
  %conv38 = zext i8 %18 to i32
  %shl39 = shl nuw nsw i32 %conv38, 8
  %or40 = or i32 %shl39, %shr33
  %idxprom41 = sext i32 %or40 to i64
  %arrayidx42 = getelementptr inbounds i32, i32* %ftab, i64 %idxprom41
  %19 = load i32, i32* %arrayidx42, align 4, !tbaa !14
  %inc43 = add i32 %19, 1
  store i32 %inc43, i32* %arrayidx42, align 4, !tbaa !14
  %sub44 = add nsw i32 %i.1842, -3
  %idxprom45 = sext i32 %sub44 to i64
  %arrayidx46 = getelementptr inbounds i16, i16* %quadrant, i64 %idxprom45
  store i16 0, i16* %arrayidx46, align 2, !tbaa !18
  %shr47 = ashr i32 %or40, 8
  %arrayidx50 = getelementptr inbounds i8, i8* %block, i64 %idxprom45
  %20 = load i8, i8* %arrayidx50, align 1, !tbaa !17
  %conv52 = zext i8 %20 to i32
  %shl53 = shl nuw nsw i32 %conv52, 8
  %or54 = or i32 %shl53, %shr47
  %idxprom55 = sext i32 %or54 to i64
  %arrayidx56 = getelementptr inbounds i32, i32* %ftab, i64 %idxprom55
  %21 = load i32, i32* %arrayidx56, align 4, !tbaa !14
  %inc57 = add i32 %21, 1
  store i32 %inc57, i32* %arrayidx56, align 4, !tbaa !14
  %sub59 = add nsw i32 %i.1842, -4
  %cmp4 = icmp sgt i32 %sub59, 2
  %indvars.iv.next885 = add nsw i64 %indvars.iv884, -4
  br i1 %cmp4, label %for.body6, label %for.cond61.preheader.loopexit

for.body64:                                       ; preds = %for.body64.preheader, %for.body64
  %indvars.iv882 = phi i64 [ %13, %for.body64.preheader ], [ %indvars.iv.next883, %for.body64 ]
  %j.1838 = phi i32 [ %j.0.lcssa, %for.body64.preheader ], [ %or73, %for.body64 ]
  %arrayidx66 = getelementptr inbounds i16, i16* %quadrant, i64 %indvars.iv882
  store i16 0, i16* %arrayidx66, align 2, !tbaa !18
  %shr67 = ashr i32 %j.1838, 8
  %arrayidx69 = getelementptr inbounds i8, i8* %block, i64 %indvars.iv882
  %22 = load i8, i8* %arrayidx69, align 1, !tbaa !17
  %conv71 = zext i8 %22 to i32
  %shl72 = shl nuw nsw i32 %conv71, 8
  %or73 = or i32 %shl72, %shr67
  %idxprom74 = sext i32 %or73 to i64
  %arrayidx75 = getelementptr inbounds i32, i32* %ftab, i64 %idxprom74
  %23 = load i32, i32* %arrayidx75, align 4, !tbaa !14
  %inc76 = add i32 %23, 1
  store i32 %inc76, i32* %arrayidx75, align 4, !tbaa !14
  %indvars.iv.next883 = add nsw i64 %indvars.iv882, -1
  %cmp62 = icmp sgt i64 %indvars.iv882, 0
  br i1 %cmp62, label %for.body64, label %for.body83.preheader

for.body83.preheader:                             ; preds = %for.body64, %for.cond61.preheader
  %24 = sext i32 %nblock to i64
  br label %for.body83

for.body83:                                       ; preds = %for.body83, %for.body83.preheader
  %indvars.iv878 = phi i64 [ 0, %for.body83.preheader ], [ %indvars.iv.next879, %for.body83 ]
  %arrayidx85 = getelementptr inbounds i8, i8* %block, i64 %indvars.iv878
  %25 = load i8, i8* %arrayidx85, align 1, !tbaa !17
  %26 = add nsw i64 %indvars.iv878, %24
  %arrayidx87 = getelementptr inbounds i8, i8* %block, i64 %26
  store i8 %25, i8* %arrayidx87, align 1, !tbaa !17
  %arrayidx90 = getelementptr inbounds i16, i16* %quadrant, i64 %26
  store i16 0, i16* %arrayidx90, align 2, !tbaa !18
  %indvars.iv.next879 = add nuw nsw i64 %indvars.iv878, 1
  %exitcond881 = icmp eq i64 %indvars.iv.next879, 34
  br i1 %exitcond881, label %for.end93, label %for.body83

for.end93:                                        ; preds = %for.body83
  br i1 %cmp, label %if.then96, label %for.body102

if.then96:                                        ; preds = %for.end93
  %27 = load %struct._IO_FILE*, %struct._IO_FILE** @stderr, align 8, !tbaa !15
  %28 = tail call i64 @fwrite(i8* nonnull getelementptr inbounds ([28 x i8], [28 x i8]* @.str.2, i64 0, i64 0), i64 27, i64 1, %struct._IO_FILE* %27) #8
  br label %for.body102

for.body102:                                      ; preds = %for.end93, %if.then96, %for.body102
  %indvars.iv874 = phi i64 [ %indvars.iv.next875, %for.body102 ], [ 1, %if.then96 ], [ 1, %for.end93 ]
  %29 = add nsw i64 %indvars.iv874, -1
  %arrayidx105 = getelementptr inbounds i32, i32* %ftab, i64 %29
  %30 = load i32, i32* %arrayidx105, align 4, !tbaa !14
  %arrayidx107 = getelementptr inbounds i32, i32* %ftab, i64 %indvars.iv874
  %31 = load i32, i32* %arrayidx107, align 4, !tbaa !14
  %add108 = add i32 %31, %30
  store i32 %add108, i32* %arrayidx107, align 4, !tbaa !14
  %indvars.iv.next875 = add nuw nsw i64 %indvars.iv874, 1
  %exitcond877 = icmp eq i64 %indvars.iv.next875, 65537
  br i1 %exitcond877, label %for.end111, label %for.body102

for.end111:                                       ; preds = %for.body102
  %32 = load i8, i8* %block, align 1, !tbaa !17
  %conv113 = zext i8 %32 to i16
  %shl114 = shl nuw i16 %conv113, 8
  %cmp118830 = icmp sgt i32 %sub, 2
  br i1 %cmp118830, label %for.body120.preheader, label %for.cond190.preheader

for.body120.preheader:                            ; preds = %for.end111
  %33 = add i32 %nblock, -1
  %34 = sext i32 %33 to i64
  %35 = add i32 %nblock, -4
  %36 = and i32 %35, -4
  br label %for.body120

for.cond190.preheader.loopexit:                   ; preds = %for.body120
  %37 = add i32 %nblock, -5
  %38 = sub i32 %37, %36
  br label %for.cond190.preheader

for.cond190.preheader:                            ; preds = %for.cond190.preheader.loopexit, %for.end111
  %i.5.lcssa = phi i32 [ %sub, %for.end111 ], [ %38, %for.cond190.preheader.loopexit ]
  %s.0.lcssa = phi i16 [ %shl114, %for.end111 ], [ %or177, %for.cond190.preheader.loopexit ]
  %cmp191827 = icmp sgt i32 %i.5.lcssa, -1
  br i1 %cmp191827, label %for.body193.preheader, label %for.body215.preheader

for.body193.preheader:                            ; preds = %for.cond190.preheader
  %39 = sext i32 %i.5.lcssa to i64
  br label %for.body193

for.body120:                                      ; preds = %for.body120.preheader, %for.body120
  %indvars.iv872 = phi i64 [ %34, %for.body120.preheader ], [ %indvars.iv.next873, %for.body120 ]
  %i.5832 = phi i32 [ %sub, %for.body120.preheader ], [ %sub188, %for.body120 ]
  %s.0831 = phi i16 [ %shl114, %for.body120.preheader ], [ %or177, %for.body120 ]
  %shr122802 = lshr i16 %s.0831, 8
  %arrayidx124 = getelementptr inbounds i8, i8* %block, i64 %indvars.iv872
  %40 = load i8, i8* %arrayidx124, align 1, !tbaa !17
  %conv125 = zext i8 %40 to i16
  %shl126 = shl nuw i16 %conv125, 8
  %or127 = or i16 %shl126, %shr122802
  %idxprom129 = zext i16 %or127 to i64
  %arrayidx130 = getelementptr inbounds i32, i32* %ftab, i64 %idxprom129
  %41 = load i32, i32* %arrayidx130, align 4, !tbaa !14
  %sub131 = add i32 %41, -1
  store i32 %sub131, i32* %arrayidx130, align 4, !tbaa !14
  %idxprom134 = sext i32 %sub131 to i64
  %arrayidx135 = getelementptr inbounds i32, i32* %ptr, i64 %idxprom134
  %42 = trunc i64 %indvars.iv872 to i32
  store i32 %42, i32* %arrayidx135, align 4, !tbaa !14
  %sub138 = add nsw i32 %i.5832, -1
  %idxprom139 = sext i32 %sub138 to i64
  %arrayidx140 = getelementptr inbounds i8, i8* %block, i64 %idxprom139
  %43 = load i8, i8* %arrayidx140, align 1, !tbaa !17
  %conv141 = zext i8 %43 to i16
  %shl142 = shl nuw i16 %conv141, 8
  %or143 = or i16 %shl142, %conv125
  %idxprom145 = zext i16 %or143 to i64
  %arrayidx146 = getelementptr inbounds i32, i32* %ftab, i64 %idxprom145
  %44 = load i32, i32* %arrayidx146, align 4, !tbaa !14
  %sub147 = add i32 %44, -1
  store i32 %sub147, i32* %arrayidx146, align 4, !tbaa !14
  %idxprom151 = sext i32 %sub147 to i64
  %arrayidx152 = getelementptr inbounds i32, i32* %ptr, i64 %idxprom151
  store i32 %sub138, i32* %arrayidx152, align 4, !tbaa !14
  %sub155 = add nsw i32 %i.5832, -2
  %idxprom156 = sext i32 %sub155 to i64
  %arrayidx157 = getelementptr inbounds i8, i8* %block, i64 %idxprom156
  %45 = load i8, i8* %arrayidx157, align 1, !tbaa !17
  %conv158 = zext i8 %45 to i16
  %shl159 = shl nuw i16 %conv158, 8
  %or160 = or i16 %shl159, %conv141
  %idxprom162 = zext i16 %or160 to i64
  %arrayidx163 = getelementptr inbounds i32, i32* %ftab, i64 %idxprom162
  %46 = load i32, i32* %arrayidx163, align 4, !tbaa !14
  %sub164 = add i32 %46, -1
  store i32 %sub164, i32* %arrayidx163, align 4, !tbaa !14
  %idxprom168 = sext i32 %sub164 to i64
  %arrayidx169 = getelementptr inbounds i32, i32* %ptr, i64 %idxprom168
  store i32 %sub155, i32* %arrayidx169, align 4, !tbaa !14
  %sub172 = add nsw i32 %i.5832, -3
  %idxprom173 = sext i32 %sub172 to i64
  %arrayidx174 = getelementptr inbounds i8, i8* %block, i64 %idxprom173
  %47 = load i8, i8* %arrayidx174, align 1, !tbaa !17
  %conv175 = zext i8 %47 to i16
  %shl176 = shl nuw i16 %conv175, 8
  %or177 = or i16 %shl176, %conv158
  %idxprom179 = zext i16 %or177 to i64
  %arrayidx180 = getelementptr inbounds i32, i32* %ftab, i64 %idxprom179
  %48 = load i32, i32* %arrayidx180, align 4, !tbaa !14
  %sub181 = add i32 %48, -1
  store i32 %sub181, i32* %arrayidx180, align 4, !tbaa !14
  %idxprom185 = sext i32 %sub181 to i64
  %arrayidx186 = getelementptr inbounds i32, i32* %ptr, i64 %idxprom185
  store i32 %sub172, i32* %arrayidx186, align 4, !tbaa !14
  %sub188 = add nsw i32 %i.5832, -4
  %cmp118 = icmp sgt i32 %sub188, 2
  %indvars.iv.next873 = add nsw i64 %indvars.iv872, -4
  br i1 %cmp118, label %for.body120, label %for.cond190.preheader.loopexit

for.body193:                                      ; preds = %for.body193.preheader, %for.body193
  %indvars.iv870 = phi i64 [ %39, %for.body193.preheader ], [ %indvars.iv.next871, %for.body193 ]
  %s.1828 = phi i16 [ %s.0.lcssa, %for.body193.preheader ], [ %or200, %for.body193 ]
  %shr195801 = lshr i16 %s.1828, 8
  %arrayidx197 = getelementptr inbounds i8, i8* %block, i64 %indvars.iv870
  %49 = load i8, i8* %arrayidx197, align 1, !tbaa !17
  %conv198 = zext i8 %49 to i16
  %shl199 = shl nuw i16 %conv198, 8
  %or200 = or i16 %shl199, %shr195801
  %idxprom202 = zext i16 %or200 to i64
  %arrayidx203 = getelementptr inbounds i32, i32* %ftab, i64 %idxprom202
  %50 = load i32, i32* %arrayidx203, align 4, !tbaa !14
  %sub204 = add i32 %50, -1
  store i32 %sub204, i32* %arrayidx203, align 4, !tbaa !14
  %idxprom207 = sext i32 %sub204 to i64
  %arrayidx208 = getelementptr inbounds i32, i32* %ptr, i64 %idxprom207
  %51 = trunc i64 %indvars.iv870 to i32
  store i32 %51, i32* %arrayidx208, align 4, !tbaa !14
  %indvars.iv.next871 = add nsw i64 %indvars.iv870, -1
  %cmp191 = icmp sgt i64 %indvars.iv870, 0
  br i1 %cmp191, label %for.body193, label %for.body215.preheader

for.body215.preheader:                            ; preds = %for.body193, %for.cond190.preheader
  call void @llvm.memset.p0i8.i64(i8* %bigDone888, i8 0, i64 256, i32 16, i1 false)
  br label %for.body215

for.body215:                                      ; preds = %for.body215, %for.body215.preheader
  %indvars.iv867 = phi i64 [ 0, %for.body215.preheader ], [ %indvars.iv.next868, %for.body215 ]
  %arrayidx219 = getelementptr inbounds [256 x i32], [256 x i32]* %runningOrder, i64 0, i64 %indvars.iv867
  %52 = trunc i64 %indvars.iv867 to i32
  store i32 %52, i32* %arrayidx219, align 4, !tbaa !14
  %indvars.iv.next868 = add nuw nsw i64 %indvars.iv867, 1
  %exitcond869 = icmp eq i64 %indvars.iv.next868, 256
  br i1 %exitcond869, label %do.body226, label %for.body215

do.body226:                                       ; preds = %for.body215, %do.cond273
  %h.1 = phi i32 [ %div, %do.cond273 ], [ 364, %for.body215 ]
  %div = sdiv i32 %h.1, 3
  %cmp228824 = icmp slt i32 %h.1, 768
  br i1 %cmp228824, label %for.body230.preheader, label %do.cond273

for.body230.preheader:                            ; preds = %do.body226
  %53 = sext i32 %div to i64
  br label %for.body230

for.body230:                                      ; preds = %for.body230.preheader, %zero
  %indvars.iv865 = phi i64 [ %53, %for.body230.preheader ], [ %indvars.iv.next866, %zero ]
  %arrayidx232 = getelementptr inbounds [256 x i32], [256 x i32]* %runningOrder, i64 0, i64 %indvars.iv865
  %54 = load i32, i32* %arrayidx232, align 4, !tbaa !14
  %add247 = shl i32 %54, 8
  %shl248 = add i32 %add247, 256
  %idxprom249 = sext i32 %shl248 to i64
  %arrayidx250 = getelementptr inbounds i32, i32* %ftab, i64 %idxprom249
  %55 = load i32, i32* %arrayidx250, align 4
  %idxprom252 = sext i32 %add247 to i64
  %arrayidx253 = getelementptr inbounds i32, i32* %ftab, i64 %idxprom252
  %56 = load i32, i32* %arrayidx253, align 4
  %sub254 = sub i32 %55, %56
  %57 = trunc i64 %indvars.iv865 to i32
  br label %while.cond

while.cond:                                       ; preds = %while.body, %for.body230
  %j.2 = phi i32 [ %57, %for.body230 ], [ %sub233, %while.body ]
  %sub233 = sub nsw i32 %j.2, %div
  %idxprom234 = sext i32 %sub233 to i64
  %arrayidx235 = getelementptr inbounds [256 x i32], [256 x i32]* %runningOrder, i64 0, i64 %idxprom234
  %58 = load i32, i32* %arrayidx235, align 4, !tbaa !14
  %add236 = shl i32 %58, 8
  %shl237 = add i32 %add236, 256
  %idxprom238 = sext i32 %shl237 to i64
  %arrayidx239 = getelementptr inbounds i32, i32* %ftab, i64 %idxprom238
  %59 = load i32, i32* %arrayidx239, align 4, !tbaa !14
  %idxprom244 = sext i32 %add236 to i64
  %arrayidx245 = getelementptr inbounds i32, i32* %ftab, i64 %idxprom244
  %60 = load i32, i32* %arrayidx245, align 4, !tbaa !14
  %sub246 = sub i32 %59, %60
  %cmp255 = icmp ugt i32 %sub246, %sub254
  br i1 %cmp255, label %while.body, label %zero

while.body:                                       ; preds = %while.cond
  %idxprom260 = sext i32 %j.2 to i64
  %arrayidx261 = getelementptr inbounds [256 x i32], [256 x i32]* %runningOrder, i64 0, i64 %idxprom260
  store i32 %58, i32* %arrayidx261, align 4, !tbaa !14
  %cmp264 = icmp slt i32 %sub233, %div
  br i1 %cmp264, label %zero, label %while.cond

zero:                                             ; preds = %while.cond, %while.body
  %j.3 = phi i32 [ %sub233, %while.body ], [ %j.2, %while.cond ]
  %idxprom268 = sext i32 %j.3 to i64
  %arrayidx269 = getelementptr inbounds [256 x i32], [256 x i32]* %runningOrder, i64 0, i64 %idxprom268
  store i32 %54, i32* %arrayidx269, align 4, !tbaa !14
  %indvars.iv.next866 = add nsw i64 %indvars.iv865, 1
  %cmp228 = icmp slt i64 %indvars.iv.next866, 256
  br i1 %cmp228, label %for.body230, label %do.cond273

do.cond273:                                       ; preds = %zero, %do.body226
  %h.1.off = add nsw i32 %h.1, -3
  %61 = icmp ugt i32 %h.1.off, 2
  br i1 %61, label %do.body226, label %for.body280

for.body280:                                      ; preds = %do.cond273, %for.inc507
  %indvars.iv863 = phi i64 [ %indvars.iv.next864, %for.inc507 ], [ 0, %do.cond273 ]
  %numQSorted.0822 = phi i32 [ %numQSorted.5, %for.inc507 ], [ 0, %do.cond273 ]
  %arrayidx282 = getelementptr inbounds [256 x i32], [256 x i32]* %runningOrder, i64 0, i64 %indvars.iv863
  %62 = load i32, i32* %arrayidx282, align 4, !tbaa !14
  %shl290 = shl i32 %62, 8
  %63 = sext i32 %shl290 to i64
  %64 = zext i32 %62 to i64
  br label %for.body286

for.body286:                                      ; preds = %for.body280, %for.inc327
  %indvars.iv = phi i64 [ 0, %for.body280 ], [ %indvars.iv.next, %for.inc327 ]
  %numQSorted.1810 = phi i32 [ %numQSorted.0822, %for.body280 ], [ %numQSorted.5, %for.inc327 ]
  %cmp287 = icmp eq i64 %indvars.iv, %64
  br i1 %cmp287, label %for.inc327, label %if.then289

if.then289:                                       ; preds = %for.body286
  %65 = add nuw nsw i64 %indvars.iv, %63
  %arrayidx293 = getelementptr inbounds i32, i32* %ftab, i64 %65
  %66 = load i32, i32* %arrayidx293, align 4, !tbaa !14
  %and = and i32 %66, 2097152
  %tobool = icmp eq i32 %and, 0
  br i1 %tobool, label %if.then294, label %if.end322

if.then294:                                       ; preds = %if.then289
  %and297 = and i32 %66, -2097153
  %67 = add nsw i64 %65, 1
  %arrayidx300 = getelementptr inbounds i32, i32* %ftab, i64 %67
  %68 = load i32, i32* %arrayidx300, align 4, !tbaa !14
  %and301 = and i32 %68, -2097153
  %sub302 = add i32 %and301, -1
  %cmp303 = icmp sgt i32 %sub302, %and297
  br i1 %cmp303, label %if.then305, label %if.end322

if.then305:                                       ; preds = %if.then294
  br i1 %cmp, label %if.then308, label %if.end312

if.then308:                                       ; preds = %if.then305
  %69 = load %struct._IO_FILE*, %struct._IO_FILE** @stderr, align 8, !tbaa !15
  %sub309 = sub i32 1, %and297
  %add310 = add i32 %sub309, %sub302
  %70 = trunc i64 %indvars.iv to i32
  %call311 = tail call i32 (%struct._IO_FILE*, i8*, ...) @fprintf(%struct._IO_FILE* %69, i8* nonnull getelementptr inbounds ([48 x i8], [48 x i8]* @.str.7, i64 0, i64 0), i32 %62, i32 %70, i32 %numQSorted.1810, i32 %add310) #8
  br label %if.end312

if.end312:                                        ; preds = %if.then308, %if.then305
  tail call fastcc void @mainQSort3(i32* %ptr, i8* %block, i16* %quadrant, i32 %nblock, i32 %and297, i32 %sub302, i32* %budget)
  %sub313 = add i32 %numQSorted.1810, 1
  %add314 = sub i32 %sub313, %and297
  %add315 = add i32 %add314, %sub302
  %71 = load i32, i32* %budget, align 4, !tbaa !14
  %cmp316 = icmp slt i32 %71, 0
  br i1 %cmp316, label %cleanup516, label %if.end322

if.end322:                                        ; preds = %if.end312, %if.then294, %if.then289
  %numQSorted.4 = phi i32 [ %numQSorted.1810, %if.then289 ], [ %add315, %if.end312 ], [ %numQSorted.1810, %if.then294 ]
  %72 = load i32, i32* %arrayidx293, align 4, !tbaa !14
  %or325 = or i32 %72, 2097152
  store i32 %or325, i32* %arrayidx293, align 4, !tbaa !14
  br label %for.inc327

for.inc327:                                       ; preds = %for.body286, %if.end322
  %numQSorted.5 = phi i32 [ %numQSorted.4, %if.end322 ], [ %numQSorted.1810, %for.body286 ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %cmp284 = icmp slt i64 %indvars.iv.next, 256
  br i1 %cmp284, label %for.body286, label %for.end329

for.end329:                                       ; preds = %for.inc327
  %idxprom330 = sext i32 %62 to i64
  %arrayidx331 = getelementptr inbounds [256 x i8], [256 x i8]* %bigDone, i64 0, i64 %idxprom330
  %73 = load i8, i8* %arrayidx331, align 1, !tbaa !17
  %tobool332 = icmp eq i8 %73, 0
  br i1 %tobool332, label %for.body338.preheader, label %if.then333

if.then333:                                       ; preds = %for.end329
  tail call void @BZ2_bz__AssertH__fail(i32 1006) #7
  br label %for.body338.preheader

for.body338.preheader:                            ; preds = %for.end329, %if.then333
  %74 = sext i32 %62 to i64
  br label %for.body338

for.body338:                                      ; preds = %for.body338, %for.body338.preheader
  %indvars.iv849 = phi i64 [ 0, %for.body338.preheader ], [ %indvars.iv.next850, %for.body338 ]
  %75 = shl i64 %indvars.iv849, 8
  %76 = add nsw i64 %75, %74
  %arrayidx342 = getelementptr inbounds i32, i32* %ftab, i64 %76
  %77 = load i32, i32* %arrayidx342, align 4, !tbaa !14
  %and343 = and i32 %77, -2097153
  %arrayidx345 = getelementptr inbounds [256 x i32], [256 x i32]* %copyStart, i64 0, i64 %indvars.iv849
  store i32 %and343, i32* %arrayidx345, align 4, !tbaa !14
  %78 = add nsw i64 %76, 1
  %arrayidx350 = getelementptr inbounds i32, i32* %ftab, i64 %78
  %79 = load i32, i32* %arrayidx350, align 4, !tbaa !14
  %and351 = and i32 %79, -2097153
  %sub352 = add i32 %and351, -1
  %arrayidx354 = getelementptr inbounds [256 x i32], [256 x i32]* %copyEnd, i64 0, i64 %indvars.iv849
  store i32 %sub352, i32* %arrayidx354, align 4, !tbaa !14
  %indvars.iv.next850 = add nuw nsw i64 %indvars.iv849, 1
  %exitcond = icmp eq i64 %indvars.iv.next850, 256
  br i1 %exitcond, label %for.end357, label %for.body338

for.end357:                                       ; preds = %for.body338
  %shl358 = shl i32 %62, 8
  %idxprom359 = sext i32 %shl358 to i64
  %arrayidx360 = getelementptr inbounds i32, i32* %ftab, i64 %idxprom359
  %80 = load i32, i32* %arrayidx360, align 4, !tbaa !14
  %and361 = and i32 %80, -2097153
  %arrayidx364 = getelementptr inbounds [256 x i32], [256 x i32]* %copyStart, i64 0, i64 %idxprom330
  %81 = load i32, i32* %arrayidx364, align 4, !tbaa !14
  %cmp365813 = icmp slt i32 %and361, %81
  br i1 %cmp365813, label %for.body367.preheader, label %for.end390

for.body367.preheader:                            ; preds = %for.end357
  %82 = sext i32 %and361 to i64
  br label %for.body367

for.body367:                                      ; preds = %for.body367.preheader, %for.inc388
  %indvars.iv854 = phi i64 [ %82, %for.body367.preheader ], [ %indvars.iv.next855, %for.inc388 ]
  %arrayidx369 = getelementptr inbounds i32, i32* %ptr, i64 %indvars.iv854
  %83 = load i32, i32* %arrayidx369, align 4, !tbaa !14
  %sub370 = add i32 %83, -1
  %cmp371 = icmp slt i32 %sub370, 0
  %add374 = select i1 %cmp371, i32 %nblock, i32 0
  %add374.sub370 = add nsw i32 %add374, %sub370
  %idxprom376 = sext i32 %add374.sub370 to i64
  %arrayidx377 = getelementptr inbounds i8, i8* %block, i64 %idxprom376
  %84 = load i8, i8* %arrayidx377, align 1, !tbaa !17
  %idxprom378 = zext i8 %84 to i64
  %arrayidx379 = getelementptr inbounds [256 x i8], [256 x i8]* %bigDone, i64 0, i64 %idxprom378
  %85 = load i8, i8* %arrayidx379, align 1, !tbaa !17
  %tobool380 = icmp eq i8 %85, 0
  br i1 %tobool380, label %if.then381, label %for.inc388

if.then381:                                       ; preds = %for.body367
  %arrayidx383 = getelementptr inbounds [256 x i32], [256 x i32]* %copyStart, i64 0, i64 %idxprom378
  %86 = load i32, i32* %arrayidx383, align 4, !tbaa !14
  %inc384 = add nsw i32 %86, 1
  store i32 %inc384, i32* %arrayidx383, align 4, !tbaa !14
  %idxprom385 = sext i32 %86 to i64
  %arrayidx386 = getelementptr inbounds i32, i32* %ptr, i64 %idxprom385
  store i32 %add374.sub370, i32* %arrayidx386, align 4, !tbaa !14
  br label %for.inc388

for.inc388:                                       ; preds = %for.body367, %if.then381
  %indvars.iv.next855 = add i64 %indvars.iv854, 1
  %87 = load i32, i32* %arrayidx364, align 4, !tbaa !14
  %88 = sext i32 %87 to i64
  %cmp365 = icmp slt i64 %indvars.iv.next855, %88
  br i1 %cmp365, label %for.body367, label %for.end390

for.end390:                                       ; preds = %for.inc388, %for.end357
  %shl392 = add i32 %shl358, 256
  %idxprom393 = sext i32 %shl392 to i64
  %arrayidx394 = getelementptr inbounds i32, i32* %ftab, i64 %idxprom393
  %89 = load i32, i32* %arrayidx394, align 4, !tbaa !14
  %and395 = and i32 %89, -2097153
  %j.7815 = add i32 %and395, -1
  %arrayidx399 = getelementptr inbounds [256 x i32], [256 x i32]* %copyEnd, i64 0, i64 %idxprom330
  %90 = load i32, i32* %arrayidx399, align 4, !tbaa !14
  %cmp400816 = icmp sgt i32 %j.7815, %90
  br i1 %cmp400816, label %for.body402, label %for.end425

for.body402:                                      ; preds = %for.end390, %for.cond397.backedge
  %j.7817 = phi i32 [ %j.7, %for.cond397.backedge ], [ %j.7815, %for.end390 ]
  %idxprom403 = sext i32 %j.7817 to i64
  %arrayidx404 = getelementptr inbounds i32, i32* %ptr, i64 %idxprom403
  %91 = load i32, i32* %arrayidx404, align 4, !tbaa !14
  %sub405 = add i32 %91, -1
  %cmp406 = icmp slt i32 %sub405, 0
  %add409 = select i1 %cmp406, i32 %nblock, i32 0
  %add409.sub405 = add nsw i32 %add409, %sub405
  %idxprom411 = sext i32 %add409.sub405 to i64
  %arrayidx412 = getelementptr inbounds i8, i8* %block, i64 %idxprom411
  %92 = load i8, i8* %arrayidx412, align 1, !tbaa !17
  %idxprom413 = zext i8 %92 to i64
  %arrayidx414 = getelementptr inbounds [256 x i8], [256 x i8]* %bigDone, i64 0, i64 %idxprom413
  %93 = load i8, i8* %arrayidx414, align 1, !tbaa !17
  %tobool415 = icmp eq i8 %93, 0
  br i1 %tobool415, label %if.then416, label %for.cond397.backedge

for.cond397.backedge:                             ; preds = %for.body402, %if.then416
  %j.7 = add i32 %j.7817, -1
  %94 = load i32, i32* %arrayidx399, align 4, !tbaa !14
  %cmp400 = icmp sgt i32 %j.7, %94
  br i1 %cmp400, label %for.body402, label %for.end425

if.then416:                                       ; preds = %for.body402
  %arrayidx418 = getelementptr inbounds [256 x i32], [256 x i32]* %copyEnd, i64 0, i64 %idxprom413
  %95 = load i32, i32* %arrayidx418, align 4, !tbaa !14
  %dec419 = add nsw i32 %95, -1
  store i32 %dec419, i32* %arrayidx418, align 4, !tbaa !14
  %idxprom420 = sext i32 %95 to i64
  %arrayidx421 = getelementptr inbounds i32, i32* %ptr, i64 %idxprom420
  store i32 %add409.sub405, i32* %arrayidx421, align 4, !tbaa !14
  br label %for.cond397.backedge

for.end425:                                       ; preds = %for.cond397.backedge, %for.end390
  %.lcssa = phi i32 [ %90, %for.end390 ], [ %94, %for.cond397.backedge ]
  %96 = load i32, i32* %arrayidx364, align 4, !tbaa !14
  %sub428 = add nsw i32 %96, -1
  %cmp431 = icmp eq i32 %sub428, %.lcssa
  br i1 %cmp431, label %for.body447.preheader, label %lor.lhs.false

lor.lhs.false:                                    ; preds = %for.end425
  %cmp435 = icmp eq i32 %96, 0
  %cmp440 = icmp eq i32 %.lcssa, %sub
  %or.cond = and i1 %cmp440, %cmp435
  br i1 %or.cond, label %for.body447.preheader, label %if.then442

if.then442:                                       ; preds = %lor.lhs.false
  tail call void @BZ2_bz__AssertH__fail(i32 1007) #7
  br label %for.body447.preheader

for.body447.preheader:                            ; preds = %lor.lhs.false, %if.then442, %for.end425
  %97 = sext i32 %62 to i64
  br label %for.body447

for.body447:                                      ; preds = %for.body447, %for.body447.preheader
  %indvars.iv856 = phi i64 [ 0, %for.body447.preheader ], [ %indvars.iv.next857, %for.body447 ]
  %98 = shl i64 %indvars.iv856, 8
  %99 = add nsw i64 %98, %97
  %arrayidx451 = getelementptr inbounds i32, i32* %ftab, i64 %99
  %100 = load i32, i32* %arrayidx451, align 4, !tbaa !14
  %or452 = or i32 %100, 2097152
  store i32 %or452, i32* %arrayidx451, align 4, !tbaa !14
  %indvars.iv.next857 = add nuw nsw i64 %indvars.iv856, 1
  %exitcond860 = icmp eq i64 %indvars.iv.next857, 256
  br i1 %exitcond860, label %for.end455, label %for.body447

for.end455:                                       ; preds = %for.body447
  store i8 1, i8* %arrayidx331, align 1, !tbaa !17
  %cmp458 = icmp slt i64 %indvars.iv863, 255
  br i1 %cmp458, label %if.then460, label %for.inc507

if.then460:                                       ; preds = %for.end455
  %101 = load i32, i32* %arrayidx360, align 4, !tbaa !14
  %and464 = and i32 %101, -2097153
  %102 = load i32, i32* %arrayidx394, align 4, !tbaa !14
  %and469 = and i32 %102, -2097153
  %sub470 = sub i32 %and469, %and464
  br label %while.cond471

while.cond471:                                    ; preds = %while.cond471, %if.then460
  %shifts.0 = phi i32 [ 0, %if.then460 ], [ %inc476, %while.cond471 ]
  %shr472 = ashr i32 %sub470, %shifts.0
  %cmp473 = icmp sgt i32 %shr472, 65534
  %inc476 = add nuw nsw i32 %shifts.0, 1
  br i1 %cmp473, label %while.cond471, label %while.end477

while.end477:                                     ; preds = %while.cond471
  %sub478 = add nsw i32 %sub470, -1
  %cmp480820 = icmp sgt i32 %sub470, 0
  br i1 %cmp480820, label %for.body482.preheader, label %for.end499

for.body482.preheader:                            ; preds = %while.end477
  %103 = sub i32 %and469, %and464
  %104 = sext i32 %103 to i64
  br label %for.body482

for.body482:                                      ; preds = %for.body482.preheader, %if.end496
  %indvars.iv861.in = phi i64 [ %104, %for.body482.preheader ], [ %indvars.iv861, %if.end496 ]
  %j.9821 = phi i32 [ %sub478, %for.body482.preheader ], [ %dec498, %if.end496 ]
  %indvars.iv861 = add i64 %indvars.iv861.in, -1
  %add483 = add nsw i32 %j.9821, %and464
  %idxprom484 = sext i32 %add483 to i64
  %arrayidx485 = getelementptr inbounds i32, i32* %ptr, i64 %idxprom484
  %105 = load i32, i32* %arrayidx485, align 4, !tbaa !14
  %106 = trunc i64 %indvars.iv861 to i32
  %shr486 = ashr i32 %106, %shifts.0
  %conv487 = trunc i32 %shr486 to i16
  %idxprom488 = sext i32 %105 to i64
  %arrayidx489 = getelementptr inbounds i16, i16* %quadrant, i64 %idxprom488
  store i16 %conv487, i16* %arrayidx489, align 2, !tbaa !18
  %cmp490 = icmp slt i32 %105, 34
  br i1 %cmp490, label %if.then492, label %if.end496

if.then492:                                       ; preds = %for.body482
  %add493 = add nsw i32 %105, %nblock
  %idxprom494 = sext i32 %add493 to i64
  %arrayidx495 = getelementptr inbounds i16, i16* %quadrant, i64 %idxprom494
  store i16 %conv487, i16* %arrayidx495, align 2, !tbaa !18
  br label %if.end496

if.end496:                                        ; preds = %if.then492, %for.body482
  %dec498 = add nsw i32 %j.9821, -1
  %cmp480 = icmp sgt i64 %indvars.iv861, 0
  br i1 %cmp480, label %for.body482, label %for.end499

for.end499:                                       ; preds = %if.end496, %while.end477
  %shr501 = ashr i32 %sub478, %shifts.0
  %cmp502 = icmp slt i32 %shr501, 65536
  br i1 %cmp502, label %for.inc507, label %if.then504

if.then504:                                       ; preds = %for.end499
  tail call void @BZ2_bz__AssertH__fail(i32 1002) #7
  br label %for.inc507

for.inc507:                                       ; preds = %for.end499, %if.then504, %for.end455
  %indvars.iv.next864 = add nuw nsw i64 %indvars.iv863, 1
  %cmp278 = icmp slt i64 %indvars.iv.next864, 256
  br i1 %cmp278, label %for.body280, label %for.end509

for.end509:                                       ; preds = %for.inc507
  br i1 %cmp, label %if.then512, label %cleanup516

if.then512:                                       ; preds = %for.end509
  %107 = load %struct._IO_FILE*, %struct._IO_FILE** @stderr, align 8, !tbaa !15
  %sub513 = sub nsw i32 %nblock, %numQSorted.5
  %call514 = tail call i32 (%struct._IO_FILE*, i8*, ...) @fprintf(%struct._IO_FILE* %107, i8* nonnull getelementptr inbounds ([44 x i8], [44 x i8]* @.str.8, i64 0, i64 0), i32 %nblock, i32 %numQSorted.5, i32 %sub513) #8
  br label %cleanup516

cleanup516:                                       ; preds = %if.end312, %for.end509, %if.then512
  call void @llvm.lifetime.end(i64 1024, i8* %3) #7
  call void @llvm.lifetime.end(i64 1024, i8* %2) #7
  call void @llvm.lifetime.end(i64 256, i8* %1) #7
  call void @llvm.lifetime.end(i64 1024, i8* %0) #7
  ret void
}

; Function Attrs: nounwind
declare i32 @fprintf(%struct._IO_FILE* nocapture, i8* nocapture readonly, ...) #2

declare void @BZ2_bz__AssertH__fail(i32) #3

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

; Function Attrs: nounwind uwtable
define internal fastcc void @fallbackQSort3(i32* nocapture %fmap, i32* nocapture readonly %eclass, i32 %loSt, i32 %hiSt) unnamed_addr #0 {
entry:
  %stackLo = alloca [100 x i32], align 16
  %stackHi = alloca [100 x i32], align 16
  %0 = bitcast [100 x i32]* %stackLo to i8*
  call void @llvm.lifetime.start(i64 400, i8* %0) #7
  %1 = bitcast [100 x i32]* %stackHi to i8*
  call void @llvm.lifetime.start(i64 400, i8* %1) #7
  %arrayidx = getelementptr inbounds [100 x i32], [100 x i32]* %stackLo, i64 0, i64 0
  store i32 %loSt, i32* %arrayidx, align 16, !tbaa !14
  %arrayidx2 = getelementptr inbounds [100 x i32], [100 x i32]* %stackHi, i64 0, i64 0
  store i32 %hiSt, i32* %arrayidx2, align 16, !tbaa !14
  br label %while.body.lr.ph

while.body.lr.ph:                                 ; preds = %entry, %while.cond.outer.backedge
  %r.0.ph416 = phi i32 [ 0, %entry ], [ %rem, %while.cond.outer.backedge ]
  %sp.0.ph415 = phi i32 [ 1, %entry ], [ %sp.0.ph.be, %while.cond.outer.backedge ]
  %2 = sext i32 %sp.0.ph415 to i64
  br label %while.body

while.body:                                       ; preds = %while.body.lr.ph, %if.then9
  %indvars.iv = phi i64 [ %2, %while.body.lr.ph ], [ %indvars.iv.next, %if.then9 ]
  %sp.0372 = phi i32 [ %sp.0.ph415, %while.body.lr.ph ], [ %dec, %if.then9 ]
  %cmp3 = icmp slt i64 %indvars.iv, 100
  br i1 %cmp3, label %if.end, label %if.then

if.then:                                          ; preds = %while.body
  tail call void @BZ2_bz__AssertH__fail(i32 1004) #7
  br label %if.end

if.end:                                           ; preds = %if.then, %while.body
  %dec = add nsw i32 %sp.0372, -1
  %idxprom4 = sext i32 %dec to i64
  %arrayidx5 = getelementptr inbounds [100 x i32], [100 x i32]* %stackLo, i64 0, i64 %idxprom4
  %3 = load i32, i32* %arrayidx5, align 4, !tbaa !14
  %arrayidx7 = getelementptr inbounds [100 x i32], [100 x i32]* %stackHi, i64 0, i64 %idxprom4
  %4 = load i32, i32* %arrayidx7, align 4, !tbaa !14
  %sub = sub nsw i32 %4, %3
  %cmp8 = icmp slt i32 %sub, 10
  br i1 %cmp8, label %if.then9, label %if.end10

if.then9:                                         ; preds = %if.end
  tail call fastcc void @fallbackSimpleSort(i32* %fmap, i32* %eclass, i32 %3, i32 %4)
  %cmp = icmp sgt i64 %indvars.iv, 1
  %indvars.iv.next = add nsw i64 %indvars.iv, -1
  br i1 %cmp, label %while.body, label %while.end192

if.end10:                                         ; preds = %if.end
  %5 = trunc i64 %indvars.iv to i32
  %mul = mul nuw nsw i32 %r.0.ph416, 7621
  %add = add nuw nsw i32 %mul, 1
  %rem = and i32 %add, 32767
  %rem11 = urem i32 %rem, 3
  switch i32 %rem11, label %if.else25 [
    i32 0, label %if.then13
    i32 1, label %if.then19
  ]

if.then13:                                        ; preds = %if.end10
  %idxprom14 = sext i32 %3 to i64
  %arrayidx15 = getelementptr inbounds i32, i32* %fmap, i64 %idxprom14
  %6 = load i32, i32* %arrayidx15, align 4, !tbaa !14
  %idxprom16 = zext i32 %6 to i64
  %arrayidx17 = getelementptr inbounds i32, i32* %eclass, i64 %idxprom16
  br label %if.end31

if.then19:                                        ; preds = %if.end10
  %add20 = add nsw i32 %4, %3
  %shr = ashr i32 %add20, 1
  %idxprom21 = sext i32 %shr to i64
  %arrayidx22 = getelementptr inbounds i32, i32* %fmap, i64 %idxprom21
  %7 = load i32, i32* %arrayidx22, align 4, !tbaa !14
  %idxprom23 = zext i32 %7 to i64
  %arrayidx24 = getelementptr inbounds i32, i32* %eclass, i64 %idxprom23
  br label %if.end31

if.else25:                                        ; preds = %if.end10
  %idxprom26 = sext i32 %4 to i64
  %arrayidx27 = getelementptr inbounds i32, i32* %fmap, i64 %idxprom26
  %8 = load i32, i32* %arrayidx27, align 4, !tbaa !14
  %idxprom28 = zext i32 %8 to i64
  %arrayidx29 = getelementptr inbounds i32, i32* %eclass, i64 %idxprom28
  br label %if.end31

if.end31:                                         ; preds = %if.then19, %if.else25, %if.then13
  %med.0.in = phi i32* [ %arrayidx17, %if.then13 ], [ %arrayidx24, %if.then19 ], [ %arrayidx29, %if.else25 ]
  %med.0 = load i32, i32* %med.0.in, align 4, !tbaa !14
  br label %while.body35.outer

while.body35.outer:                               ; preds = %if.end92, %if.end31
  %gtHi.0.ph = phi i32 [ %25, %if.end92 ], [ %4, %if.end31 ]
  %ltLo.0.ph = phi i32 [ %ltLo.0.ph358.lcssa, %if.end92 ], [ %3, %if.end31 ]
  %unHi.0.ph = phi i32 [ %dec103, %if.end92 ], [ %4, %if.end31 ]
  %unLo.0.ph = phi i32 [ %inc102, %if.end92 ], [ %3, %if.end31 ]
  %cmp36373380 = icmp sgt i32 %unLo.0.ph, %unHi.0.ph
  br i1 %cmp36373380, label %while.body62.preheader, label %if.end38.lr.ph.preheader

if.end38.lr.ph.preheader:                         ; preds = %while.body35.outer
  %9 = sext i32 %unHi.0.ph to i64
  %10 = sext i32 %ltLo.0.ph to i64
  br label %if.end38.lr.ph

if.end38.lr.ph:                                   ; preds = %if.end38.lr.ph.preheader, %if.then45
  %indvars.iv454 = phi i64 [ %10, %if.end38.lr.ph.preheader ], [ %indvars.iv.next455, %if.then45 ]
  %unLo.0.ph360382 = phi i32 [ %unLo.0.ph, %if.end38.lr.ph.preheader ], [ %inc55, %if.then45 ]
  %ltLo.0.ph358381 = phi i32 [ %ltLo.0.ph, %if.end38.lr.ph.preheader ], [ %inc54, %if.then45 ]
  %11 = sext i32 %unLo.0.ph360382 to i64
  br label %if.end38

if.end38:                                         ; preds = %if.end59, %if.end38.lr.ph
  %indvars.iv452 = phi i64 [ %indvars.iv.next453, %if.end59 ], [ %11, %if.end38.lr.ph ]
  %unLo.0374 = phi i32 [ %inc60, %if.end59 ], [ %unLo.0.ph360382, %if.end38.lr.ph ]
  %arrayidx40 = getelementptr inbounds i32, i32* %fmap, i64 %indvars.iv452
  %12 = load i32, i32* %arrayidx40, align 4, !tbaa !14
  %idxprom41 = zext i32 %12 to i64
  %arrayidx42 = getelementptr inbounds i32, i32* %eclass, i64 %idxprom41
  %13 = load i32, i32* %arrayidx42, align 4, !tbaa !14
  %cmp44 = icmp eq i32 %13, %med.0
  br i1 %cmp44, label %if.then45, label %if.end56

if.then45:                                        ; preds = %if.end38
  %14 = trunc i64 %indvars.iv452 to i32
  %arrayidx49 = getelementptr inbounds i32, i32* %fmap, i64 %indvars.iv454
  %15 = load i32, i32* %arrayidx49, align 4, !tbaa !14
  store i32 %15, i32* %arrayidx40, align 4, !tbaa !14
  store i32 %12, i32* %arrayidx49, align 4, !tbaa !14
  %inc54 = add nsw i32 %ltLo.0.ph358381, 1
  %inc55 = add nsw i32 %14, 1
  %cmp36373 = icmp slt i32 %14, %unHi.0.ph
  %indvars.iv.next455 = add i64 %indvars.iv454, 1
  br i1 %cmp36373, label %if.end38.lr.ph, label %while.body62.preheader

if.end56:                                         ; preds = %if.end38
  %cmp57 = icmp sgt i32 %13, %med.0
  %16 = trunc i64 %indvars.iv452 to i32
  br i1 %cmp57, label %while.body62.preheader.loopexit, label %if.end59

while.body62.preheader.loopexit:                  ; preds = %if.end56, %if.end59
  %unLo.0.lcssa.ph = phi i32 [ %inc60, %if.end59 ], [ %16, %if.end56 ]
  %17 = trunc i64 %indvars.iv454 to i32
  br label %while.body62.preheader

while.body62.preheader:                           ; preds = %if.then45, %while.body62.preheader.loopexit, %while.body35.outer
  %ltLo.0.ph358.lcssa = phi i32 [ %ltLo.0.ph, %while.body35.outer ], [ %17, %while.body62.preheader.loopexit ], [ %inc54, %if.then45 ]
  %unLo.0.lcssa = phi i32 [ %unLo.0.ph, %while.body35.outer ], [ %unLo.0.lcssa.ph, %while.body62.preheader.loopexit ], [ %inc55, %if.then45 ]
  %cmp63385395 = icmp sgt i32 %unLo.0.lcssa, %unHi.0.ph
  br i1 %cmp63385395, label %while.end104, label %if.end65.lr.ph.preheader

if.end65.lr.ph.preheader:                         ; preds = %while.body62.preheader
  %18 = sext i32 %unLo.0.lcssa to i64
  %19 = sext i32 %gtHi.0.ph to i64
  br label %if.end65.lr.ph

if.end59:                                         ; preds = %if.end56
  %inc60 = add nsw i32 %unLo.0374, 1
  %cmp36 = icmp slt i64 %indvars.iv452, %9
  %indvars.iv.next453 = add nsw i64 %indvars.iv452, 1
  br i1 %cmp36, label %if.end38, label %while.body62.preheader.loopexit

if.end65:                                         ; preds = %if.end87, %if.end65.lr.ph
  %indvars.iv456 = phi i64 [ %indvars.iv.next457, %if.end87 ], [ %24, %if.end65.lr.ph ]
  %unHi.1386 = phi i32 [ %dec88, %if.end87 ], [ %unHi.1.ph397, %if.end65.lr.ph ]
  %arrayidx67 = getelementptr inbounds i32, i32* %fmap, i64 %indvars.iv456
  %20 = load i32, i32* %arrayidx67, align 4, !tbaa !14
  %idxprom68 = zext i32 %20 to i64
  %arrayidx69 = getelementptr inbounds i32, i32* %eclass, i64 %idxprom68
  %21 = load i32, i32* %arrayidx69, align 4, !tbaa !14
  %cmp71 = icmp eq i32 %21, %med.0
  br i1 %cmp71, label %if.then72, label %if.end84

if.then72:                                        ; preds = %if.end65
  %22 = trunc i64 %indvars.iv456 to i32
  %arrayidx77 = getelementptr inbounds i32, i32* %fmap, i64 %indvars.iv458
  %23 = load i32, i32* %arrayidx77, align 4, !tbaa !14
  store i32 %23, i32* %arrayidx67, align 4, !tbaa !14
  store i32 %20, i32* %arrayidx77, align 4, !tbaa !14
  %dec82 = add nsw i32 %gtHi.1.ph396, -1
  %dec83 = add nsw i32 %22, -1
  %cmp63385 = icmp slt i32 %unLo.0.lcssa, %22
  %indvars.iv.next459 = add i64 %indvars.iv458, -1
  br i1 %cmp63385, label %if.end65.lr.ph, label %while.end104

if.end65.lr.ph:                                   ; preds = %if.end65.lr.ph.preheader, %if.then72
  %indvars.iv458 = phi i64 [ %19, %if.end65.lr.ph.preheader ], [ %indvars.iv.next459, %if.then72 ]
  %unHi.1.ph397 = phi i32 [ %unHi.0.ph, %if.end65.lr.ph.preheader ], [ %dec83, %if.then72 ]
  %gtHi.1.ph396 = phi i32 [ %gtHi.0.ph, %if.end65.lr.ph.preheader ], [ %dec82, %if.then72 ]
  %24 = sext i32 %unHi.1.ph397 to i64
  br label %if.end65

if.end84:                                         ; preds = %if.end65
  %cmp85 = icmp slt i32 %21, %med.0
  br i1 %cmp85, label %if.end92, label %if.end87

if.end87:                                         ; preds = %if.end84
  %dec88 = add nsw i32 %unHi.1386, -1
  %cmp63 = icmp slt i64 %18, %indvars.iv456
  %indvars.iv.next457 = add nsw i64 %indvars.iv456, -1
  br i1 %cmp63, label %if.end65, label %while.end104.loopexit

if.end92:                                         ; preds = %if.end84
  %25 = trunc i64 %indvars.iv458 to i32
  %26 = trunc i64 %indvars.iv456 to i32
  %idxprom94 = sext i32 %unLo.0.lcssa to i64
  %arrayidx95 = getelementptr inbounds i32, i32* %fmap, i64 %idxprom94
  %27 = load i32, i32* %arrayidx95, align 4, !tbaa !14
  store i32 %20, i32* %arrayidx95, align 4, !tbaa !14
  store i32 %27, i32* %arrayidx67, align 4, !tbaa !14
  %inc102 = add nsw i32 %unLo.0.lcssa, 1
  %dec103 = add nsw i32 %26, -1
  br label %while.body35.outer

while.end104.loopexit:                            ; preds = %if.end87
  %28 = trunc i64 %indvars.iv458 to i32
  br label %while.end104

while.end104:                                     ; preds = %while.body62.preheader, %if.then72, %while.end104.loopexit
  %gtHi.1.ph.lcssa370 = phi i32 [ %28, %while.end104.loopexit ], [ %dec82, %if.then72 ], [ %gtHi.0.ph, %while.body62.preheader ]
  %unHi.1.lcssa = phi i32 [ %dec88, %while.end104.loopexit ], [ %dec83, %if.then72 ], [ %unHi.0.ph, %while.body62.preheader ]
  %cmp105 = icmp slt i32 %gtHi.1.ph.lcssa370, %ltLo.0.ph358.lcssa
  br i1 %cmp105, label %while.cond.outer.backedge, label %if.end107

while.cond.outer.backedge:                        ; preds = %while.end104, %if.then169, %if.else180
  %sp.0.ph.be = phi i32 [ %inc190, %if.else180 ], [ %inc179, %if.then169 ], [ %dec, %while.end104 ]
  %cmp371 = icmp sgt i32 %sp.0.ph.be, 0
  br i1 %cmp371, label %while.body.lr.ph, label %while.end192

if.end107:                                        ; preds = %while.end104
  %sub108 = sub nsw i32 %ltLo.0.ph358.lcssa, %3
  %sub109 = sub nsw i32 %unLo.0.lcssa, %ltLo.0.ph358.lcssa
  %cmp110 = icmp slt i32 %sub108, %sub109
  %sub108.sub109 = select i1 %cmp110, i32 %sub108, i32 %sub109
  %cmp115406 = icmp sgt i32 %sub108.sub109, 0
  br i1 %cmp115406, label %while.body116.preheader, label %while.end129

while.body116.preheader:                          ; preds = %if.end107
  %29 = add i32 %unLo.0.lcssa, 1
  %30 = add i32 %ltLo.0.ph358.lcssa, -1
  %31 = sub i32 %30, %unLo.0.lcssa
  %32 = add i32 %3, -1
  %33 = sub i32 %32, %ltLo.0.ph358.lcssa
  %34 = icmp sgt i32 %31, %33
  %smax = select i1 %34, i32 %31, i32 %33
  %35 = add i32 %29, %smax
  %36 = sext i32 %35 to i64
  %37 = sext i32 %3 to i64
  br label %while.body116

while.body116:                                    ; preds = %while.body116.preheader, %while.body116
  %indvars.iv462 = phi i64 [ %37, %while.body116.preheader ], [ %indvars.iv.next463, %while.body116 ]
  %indvars.iv460 = phi i64 [ %36, %while.body116.preheader ], [ %indvars.iv.next461, %while.body116 ]
  %yyn.0409 = phi i32 [ %sub108.sub109, %while.body116.preheader ], [ %dec128, %while.body116 ]
  %arrayidx119 = getelementptr inbounds i32, i32* %fmap, i64 %indvars.iv462
  %38 = load i32, i32* %arrayidx119, align 4, !tbaa !14
  %arrayidx121 = getelementptr inbounds i32, i32* %fmap, i64 %indvars.iv460
  %39 = load i32, i32* %arrayidx121, align 4, !tbaa !14
  store i32 %39, i32* %arrayidx119, align 4, !tbaa !14
  store i32 %38, i32* %arrayidx121, align 4, !tbaa !14
  %dec128 = add nsw i32 %yyn.0409, -1
  %cmp115 = icmp sgt i32 %yyn.0409, 1
  %indvars.iv.next461 = add nsw i64 %indvars.iv460, 1
  %indvars.iv.next463 = add nsw i64 %indvars.iv462, 1
  br i1 %cmp115, label %while.body116, label %while.end129

while.end129:                                     ; preds = %while.body116, %if.end107
  %sub130 = sub nsw i32 %4, %gtHi.1.ph.lcssa370
  %sub131 = sub nsw i32 %gtHi.1.ph.lcssa370, %unHi.1.lcssa
  %cmp132 = icmp slt i32 %sub130, %sub131
  %sub130.sub131 = select i1 %cmp132, i32 %sub130, i32 %sub131
  %cmp145411 = icmp sgt i32 %sub130.sub131, 0
  br i1 %cmp145411, label %while.body146.preheader, label %while.end159

while.body146.preheader:                          ; preds = %while.end129
  %sub141 = sub nsw i32 %4, %sub130.sub131
  %40 = sext i32 %unLo.0.lcssa to i64
  br label %while.body146

while.body146:                                    ; preds = %while.body146.preheader, %while.body146
  %indvars.iv467 = phi i64 [ %40, %while.body146.preheader ], [ %indvars.iv.next468, %while.body146 ]
  %yyp2140.0414.in = phi i32 [ %sub141, %while.body146.preheader ], [ %yyp2140.0414, %while.body146 ]
  %yyn143.0413 = phi i32 [ %sub130.sub131, %while.body146.preheader ], [ %dec158, %while.body146 ]
  %yyp2140.0414 = add nsw i32 %yyp2140.0414.in, 1
  %arrayidx149 = getelementptr inbounds i32, i32* %fmap, i64 %indvars.iv467
  %41 = load i32, i32* %arrayidx149, align 4, !tbaa !14
  %idxprom150 = sext i32 %yyp2140.0414 to i64
  %arrayidx151 = getelementptr inbounds i32, i32* %fmap, i64 %idxprom150
  %42 = load i32, i32* %arrayidx151, align 4, !tbaa !14
  store i32 %42, i32* %arrayidx149, align 4, !tbaa !14
  store i32 %41, i32* %arrayidx151, align 4, !tbaa !14
  %dec158 = add nsw i32 %yyn143.0413, -1
  %cmp145 = icmp sgt i32 %yyn143.0413, 1
  %indvars.iv.next468 = add nsw i64 %indvars.iv467, 1
  br i1 %cmp145, label %while.body146, label %while.end159

while.end159:                                     ; preds = %while.body146, %while.end129
  %add160 = add i32 %3, -1
  %sub161 = sub i32 %add160, %ltLo.0.ph358.lcssa
  %sub162 = add i32 %sub161, %unLo.0.lcssa
  %sub164 = sub nsw i32 %4, %sub131
  %add165 = add nsw i32 %sub164, 1
  %sub166 = sub nsw i32 %sub162, %3
  %sub167 = sub nsw i32 %4, %add165
  %cmp168 = icmp sgt i32 %sub166, %sub167
  br i1 %cmp168, label %if.then169, label %if.else180

if.then169:                                       ; preds = %while.end159
  store i32 %sub162, i32* %arrayidx7, align 4, !tbaa !14
  %sext475 = shl i64 %indvars.iv, 32
  %idxprom175 = ashr exact i64 %sext475, 32
  %arrayidx176 = getelementptr inbounds [100 x i32], [100 x i32]* %stackLo, i64 0, i64 %idxprom175
  store i32 %add165, i32* %arrayidx176, align 4, !tbaa !14
  %arrayidx178 = getelementptr inbounds [100 x i32], [100 x i32]* %stackHi, i64 0, i64 %idxprom175
  store i32 %4, i32* %arrayidx178, align 4, !tbaa !14
  %inc179 = add nsw i32 %5, 1
  br label %while.cond.outer.backedge

if.else180:                                       ; preds = %while.end159
  store i32 %add165, i32* %arrayidx5, align 4, !tbaa !14
  %sext = shl i64 %indvars.iv, 32
  %idxprom186 = ashr exact i64 %sext, 32
  %arrayidx187 = getelementptr inbounds [100 x i32], [100 x i32]* %stackLo, i64 0, i64 %idxprom186
  store i32 %3, i32* %arrayidx187, align 4, !tbaa !14
  %arrayidx189 = getelementptr inbounds [100 x i32], [100 x i32]* %stackHi, i64 0, i64 %idxprom186
  store i32 %sub162, i32* %arrayidx189, align 4, !tbaa !14
  %inc190 = add nsw i32 %5, 1
  br label %while.cond.outer.backedge

while.end192:                                     ; preds = %while.cond.outer.backedge, %if.then9
  call void @llvm.lifetime.end(i64 400, i8* %1) #7
  call void @llvm.lifetime.end(i64 400, i8* %0) #7
  ret void
}

; Function Attrs: inlinehint norecurse nounwind uwtable
define internal fastcc void @fallbackSimpleSort(i32* nocapture %fmap, i32* nocapture readonly %eclass, i32 %lo, i32 %hi) unnamed_addr #4 {
entry:
  %cmp = icmp eq i32 %hi, %lo
  br i1 %cmp, label %cleanup, label %if.end

if.end:                                           ; preds = %entry
  %sub = sub nsw i32 %hi, %lo
  %cmp2 = icmp slt i32 %sub, 4
  %sub4 = add nsw i32 %hi, -4
  %cmp5122 = icmp slt i32 %sub4, %lo
  %or.cond = or i1 %cmp2, %cmp5122
  br i1 %or.cond, label %for.cond29.preheader, label %for.body.preheader

for.cond29.preheader:                             ; preds = %for.end, %if.end
  %cmp30113 = icmp sgt i32 %hi, %lo
  br i1 %cmp30113, label %for.body31.preheader, label %cleanup

for.body31.preheader:                             ; preds = %for.cond29.preheader
  %0 = sext i32 %hi to i64
  %1 = sext i32 %hi to i64
  %2 = sext i32 %lo to i64
  br label %for.body31

for.body.preheader:                               ; preds = %if.end
  %3 = add i32 %hi, -4
  %4 = sext i32 %3 to i64
  %5 = sext i32 %hi to i64
  %6 = sext i32 %lo to i64
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.end
  %indvars.iv131 = phi i64 [ %4, %for.body.preheader ], [ %indvars.iv.next132, %for.end ]
  %indvars.iv = phi i32 [ %hi, %for.body.preheader ], [ %indvars.iv.next, %for.end ]
  %arrayidx = getelementptr inbounds i32, i32* %fmap, i64 %indvars.iv131
  %7 = load i32, i32* %arrayidx, align 4, !tbaa !14
  %idxprom6 = sext i32 %7 to i64
  %arrayidx7 = getelementptr inbounds i32, i32* %eclass, i64 %idxprom6
  %8 = load i32, i32* %arrayidx7, align 4, !tbaa !14
  %9 = add nsw i64 %indvars.iv131, 4
  %cmp9117 = icmp sgt i64 %9, %5
  br i1 %cmp9117, label %for.end, label %land.rhs.preheader

land.rhs.preheader:                               ; preds = %for.body
  %10 = sext i32 %indvars.iv to i64
  %11 = trunc i64 %9 to i32
  br label %land.rhs

land.rhs:                                         ; preds = %land.rhs.preheader, %for.body15
  %indvars.iv129 = phi i64 [ %10, %land.rhs.preheader ], [ %indvars.iv.next130, %for.body15 ]
  %j.0119 = phi i32 [ %11, %land.rhs.preheader ], [ %j.0, %for.body15 ]
  %j.0.in118.in = phi i64 [ %indvars.iv131, %land.rhs.preheader ], [ %indvars.iv129, %for.body15 ]
  %arrayidx11 = getelementptr inbounds i32, i32* %fmap, i64 %indvars.iv129
  %12 = load i32, i32* %arrayidx11, align 4, !tbaa !14
  %idxprom12 = zext i32 %12 to i64
  %arrayidx13 = getelementptr inbounds i32, i32* %eclass, i64 %idxprom12
  %13 = load i32, i32* %arrayidx13, align 4, !tbaa !14
  %cmp14 = icmp ugt i32 %8, %13
  br i1 %cmp14, label %for.body15, label %for.end

for.body15:                                       ; preds = %land.rhs
  %sext134 = shl i64 %j.0.in118.in, 32
  %idxprom19 = ashr exact i64 %sext134, 32
  %arrayidx20 = getelementptr inbounds i32, i32* %fmap, i64 %idxprom19
  store i32 %12, i32* %arrayidx20, align 4, !tbaa !14
  %j.0 = add nsw i32 %j.0119, 4
  %cmp9 = icmp sgt i32 %j.0, %hi
  %indvars.iv.next130 = add i64 %indvars.iv129, 4
  br i1 %cmp9, label %for.end, label %land.rhs

for.end:                                          ; preds = %land.rhs, %for.body15, %for.body
  %j.0.in.lcssa = phi i64 [ %indvars.iv131, %for.body ], [ %j.0.in118.in, %land.rhs ], [ %indvars.iv129, %for.body15 ]
  %sext = shl i64 %j.0.in.lcssa, 32
  %idxprom23 = ashr exact i64 %sext, 32
  %arrayidx24 = getelementptr inbounds i32, i32* %fmap, i64 %idxprom23
  store i32 %7, i32* %arrayidx24, align 4, !tbaa !14
  %cmp5 = icmp sgt i64 %indvars.iv131, %6
  %indvars.iv.next = add i32 %indvars.iv, -1
  %indvars.iv.next132 = add nsw i64 %indvars.iv131, -1
  br i1 %cmp5, label %for.body, label %for.cond29.preheader

for.body31:                                       ; preds = %for.body31.preheader, %for.end53
  %indvars.iv127 = phi i64 [ %1, %for.body31.preheader ], [ %indvars.iv.next128, %for.end53 ]
  %i.1115.in = phi i32 [ %hi, %for.body31.preheader ], [ %i.1115, %for.end53 ]
  %indvars.iv.next128 = add nsw i64 %indvars.iv127, -1
  %i.1115 = add nsw i32 %i.1115.in, -1
  %arrayidx33 = getelementptr inbounds i32, i32* %fmap, i64 %indvars.iv.next128
  %14 = load i32, i32* %arrayidx33, align 4, !tbaa !14
  %idxprom34 = sext i32 %14 to i64
  %arrayidx35 = getelementptr inbounds i32, i32* %eclass, i64 %idxprom34
  %15 = load i32, i32* %arrayidx35, align 4, !tbaa !14
  %cmp38109 = icmp sgt i32 %i.1115.in, %hi
  br i1 %cmp38109, label %for.end53, label %land.rhs39

land.rhs39:                                       ; preds = %for.body31, %for.body46
  %indvars.iv124 = phi i64 [ %indvars.iv.next125, %for.body46 ], [ %indvars.iv127, %for.body31 ]
  %j.1110 = phi i32 [ %inc, %for.body46 ], [ %i.1115.in, %for.body31 ]
  %arrayidx41 = getelementptr inbounds i32, i32* %fmap, i64 %indvars.iv124
  %16 = load i32, i32* %arrayidx41, align 4, !tbaa !14
  %idxprom42 = zext i32 %16 to i64
  %arrayidx43 = getelementptr inbounds i32, i32* %eclass, i64 %idxprom42
  %17 = load i32, i32* %arrayidx43, align 4, !tbaa !14
  %cmp44 = icmp ugt i32 %15, %17
  %18 = trunc i64 %indvars.iv124 to i32
  br i1 %cmp44, label %for.body46, label %for.end53

for.body46:                                       ; preds = %land.rhs39
  %19 = add nsw i64 %indvars.iv124, -1
  %arrayidx51 = getelementptr inbounds i32, i32* %fmap, i64 %19
  store i32 %16, i32* %arrayidx51, align 4, !tbaa !14
  %inc = add nsw i32 %j.1110, 1
  %cmp38 = icmp slt i64 %indvars.iv124, %0
  %indvars.iv.next125 = add nsw i64 %indvars.iv124, 1
  br i1 %cmp38, label %land.rhs39, label %for.end53

for.end53:                                        ; preds = %for.body46, %land.rhs39, %for.body31
  %j.1.lcssa = phi i32 [ %i.1115.in, %for.body31 ], [ %18, %land.rhs39 ], [ %inc, %for.body46 ]
  %sub54 = add nsw i32 %j.1.lcssa, -1
  %idxprom55 = sext i32 %sub54 to i64
  %arrayidx56 = getelementptr inbounds i32, i32* %fmap, i64 %idxprom55
  store i32 %14, i32* %arrayidx56, align 4, !tbaa !14
  %cmp30 = icmp sgt i64 %indvars.iv.next128, %2
  br i1 %cmp30, label %for.body31, label %cleanup

cleanup:                                          ; preds = %for.end53, %for.cond29.preheader, %entry
  ret void
}

; Function Attrs: nounwind uwtable
define internal fastcc void @mainQSort3(i32* nocapture %ptr, i8* readonly %block, i16* nocapture readonly %quadrant, i32 %nblock, i32 %loSt, i32 %hiSt, i32* nocapture %budget) unnamed_addr #0 {
entry:
  %stackLo = alloca [100 x i32], align 16
  %stackHi = alloca [100 x i32], align 16
  %stackD = alloca [100 x i32], align 16
  %0 = bitcast [100 x i32]* %stackLo to i8*
  call void @llvm.lifetime.start(i64 400, i8* %0) #7
  %1 = bitcast [100 x i32]* %stackHi to i8*
  call void @llvm.lifetime.start(i64 400, i8* %1) #7
  %2 = bitcast [100 x i32]* %stackD to i8*
  call void @llvm.lifetime.start(i64 400, i8* %2) #7
  %arrayidx = getelementptr inbounds [100 x i32], [100 x i32]* %stackLo, i64 0, i64 0
  store i32 %loSt, i32* %arrayidx, align 16, !tbaa !14
  %arrayidx2 = getelementptr inbounds [100 x i32], [100 x i32]* %stackHi, i64 0, i64 0
  store i32 %hiSt, i32* %arrayidx2, align 16, !tbaa !14
  %arrayidx4 = getelementptr inbounds [100 x i32], [100 x i32]* %stackD, i64 0, i64 0
  store i32 2, i32* %arrayidx4, align 16, !tbaa !14
  br label %while.body.lr.ph

while.body.lr.ph:                                 ; preds = %entry, %while.cond.outer.backedge
  %sp.0.ph66 = phi i32 [ 1, %entry ], [ %sp.0.ph.be, %while.cond.outer.backedge ]
  %cmp5 = icmp slt i32 %sp.0.ph66, 100
  %dec = add nsw i32 %sp.0.ph66, -1
  %idxprom6 = sext i32 %dec to i64
  %arrayidx7 = getelementptr inbounds [100 x i32], [100 x i32]* %stackLo, i64 0, i64 %idxprom6
  %arrayidx9 = getelementptr inbounds [100 x i32], [100 x i32]* %stackHi, i64 0, i64 %idxprom6
  %arrayidx11 = getelementptr inbounds [100 x i32], [100 x i32]* %stackD, i64 0, i64 %idxprom6
  %3 = load i32, i32* %arrayidx7, align 4
  %4 = load i32, i32* %arrayidx9, align 4
  %sub = sub nsw i32 %4, %3
  %cmp12 = icmp slt i32 %sub, 20
  %idxprom19 = sext i32 %3 to i64
  %arrayidx20 = getelementptr inbounds i32, i32* %ptr, i64 %idxprom19
  %idxprom23 = sext i32 %4 to i64
  %arrayidx24 = getelementptr inbounds i32, i32* %ptr, i64 %idxprom23
  %add28 = add nsw i32 %4, %3
  %shr = ashr i32 %add28, 1
  %idxprom29 = sext i32 %shr to i64
  %arrayidx30 = getelementptr inbounds i32, i32* %ptr, i64 %idxprom29
  br label %while.body

while.body:                                       ; preds = %if.then120, %while.body.lr.ph
  br i1 %cmp5, label %if.end, label %if.then

if.then:                                          ; preds = %while.body
  tail call void @BZ2_bz__AssertH__fail(i32 1001) #7
  br label %if.end

if.end:                                           ; preds = %if.then, %while.body
  %5 = load i32, i32* %arrayidx11, align 4, !tbaa !14
  %cmp13 = icmp sgt i32 %5, 14
  %or.cond = or i1 %cmp12, %cmp13
  br i1 %or.cond, label %if.then14, label %if.end18

if.then14:                                        ; preds = %if.end
  tail call fastcc void @mainSimpleSort(i32* %ptr, i8* %block, i16* %quadrant, i32 %nblock, i32 %3, i32 %4, i32 %5, i32* %budget)
  %6 = load i32, i32* %budget, align 4, !tbaa !14
  %cmp15 = icmp slt i32 %6, 0
  br i1 %cmp15, label %cleanup, label %while.cond.outer.backedge

while.cond.outer.backedge:                        ; preds = %if.then14, %while.end185
  %sp.0.ph.be = phi i32 [ %inc301, %while.end185 ], [ %dec, %if.then14 ]
  %cmp = icmp sgt i32 %sp.0.ph.be, 0
  br i1 %cmp, label %while.body.lr.ph, label %cleanup

if.end18:                                         ; preds = %if.end
  %7 = load i32, i32* %arrayidx20, align 4, !tbaa !14
  %add = add i32 %7, %5
  %idxprom21 = zext i32 %add to i64
  %arrayidx22 = getelementptr inbounds i8, i8* %block, i64 %idxprom21
  %8 = load i8, i8* %arrayidx22, align 1, !tbaa !17
  %9 = load i32, i32* %arrayidx24, align 4, !tbaa !14
  %add25 = add i32 %9, %5
  %idxprom26 = zext i32 %add25 to i64
  %arrayidx27 = getelementptr inbounds i8, i8* %block, i64 %idxprom26
  %10 = load i8, i8* %arrayidx27, align 1, !tbaa !17
  %11 = load i32, i32* %arrayidx30, align 4, !tbaa !14
  %add31 = add i32 %11, %5
  %idxprom32 = zext i32 %add31 to i64
  %arrayidx33 = getelementptr inbounds i8, i8* %block, i64 %idxprom32
  %12 = load i8, i8* %arrayidx33, align 1, !tbaa !17
  %call = tail call fastcc zeroext i8 @mmed3(i8 zeroext %8, i8 zeroext %10, i8 zeroext %12)
  br label %while.body37.outer

while.body37.outer:                               ; preds = %if.end105, %if.end18
  %gtHi.0.ph = phi i32 [ %29, %if.end105 ], [ %4, %if.end18 ]
  %ltLo.0.ph = phi i32 [ %ltLo.0.ph3.lcssa, %if.end105 ], [ %3, %if.end18 ]
  %unHi.0.ph = phi i32 [ %dec116, %if.end105 ], [ %4, %if.end18 ]
  %unLo.0.ph = phi i32 [ %inc115, %if.end105 ], [ %3, %if.end18 ]
  %cmp382431 = icmp sgt i32 %unLo.0.ph, %unHi.0.ph
  br i1 %cmp382431, label %while.body69.preheader, label %if.end41.lr.ph.preheader

if.end41.lr.ph.preheader:                         ; preds = %while.body37.outer
  %13 = sext i32 %unHi.0.ph to i64
  %14 = sext i32 %ltLo.0.ph to i64
  br label %if.end41.lr.ph

if.end41.lr.ph:                                   ; preds = %if.end41.lr.ph.preheader, %if.then51
  %indvars.iv108 = phi i64 [ %14, %if.end41.lr.ph.preheader ], [ %indvars.iv.next109, %if.then51 ]
  %unLo.0.ph533 = phi i32 [ %unLo.0.ph, %if.end41.lr.ph.preheader ], [ %inc61, %if.then51 ]
  %ltLo.0.ph332 = phi i32 [ %ltLo.0.ph, %if.end41.lr.ph.preheader ], [ %inc60, %if.then51 ]
  %15 = sext i32 %unLo.0.ph533 to i64
  br label %if.end41

if.end41:                                         ; preds = %if.end66, %if.end41.lr.ph
  %indvars.iv = phi i64 [ %indvars.iv.next, %if.end66 ], [ %15, %if.end41.lr.ph ]
  %unLo.025 = phi i32 [ %inc67, %if.end66 ], [ %unLo.0.ph533, %if.end41.lr.ph ]
  %arrayidx43 = getelementptr inbounds i32, i32* %ptr, i64 %indvars.iv
  %16 = load i32, i32* %arrayidx43, align 4, !tbaa !14
  %add44 = add i32 %16, %5
  %idxprom45 = zext i32 %add44 to i64
  %arrayidx46 = getelementptr inbounds i8, i8* %block, i64 %idxprom45
  %17 = load i8, i8* %arrayidx46, align 1, !tbaa !17
  %cmp49 = icmp eq i8 %17, %call
  br i1 %cmp49, label %if.then51, label %if.end62

if.then51:                                        ; preds = %if.end41
  %18 = trunc i64 %indvars.iv to i32
  %arrayidx55 = getelementptr inbounds i32, i32* %ptr, i64 %indvars.iv108
  %19 = load i32, i32* %arrayidx55, align 4, !tbaa !14
  store i32 %19, i32* %arrayidx43, align 4, !tbaa !14
  store i32 %16, i32* %arrayidx55, align 4, !tbaa !14
  %inc60 = add nsw i32 %ltLo.0.ph332, 1
  %inc61 = add nsw i32 %18, 1
  %cmp3824 = icmp slt i32 %18, %unHi.0.ph
  %indvars.iv.next109 = add i64 %indvars.iv108, 1
  br i1 %cmp3824, label %if.end41.lr.ph, label %while.body69.preheader

if.end62:                                         ; preds = %if.end41
  %cmp63 = icmp ugt i8 %17, %call
  %20 = trunc i64 %indvars.iv to i32
  br i1 %cmp63, label %while.body69.preheader.loopexit, label %if.end66

while.body69.preheader.loopexit:                  ; preds = %if.end62, %if.end66
  %unLo.0.lcssa.ph = phi i32 [ %inc67, %if.end66 ], [ %20, %if.end62 ]
  %21 = trunc i64 %indvars.iv108 to i32
  br label %while.body69.preheader

while.body69.preheader:                           ; preds = %if.then51, %while.body69.preheader.loopexit, %while.body37.outer
  %ltLo.0.ph3.lcssa = phi i32 [ %ltLo.0.ph, %while.body37.outer ], [ %21, %while.body69.preheader.loopexit ], [ %inc60, %if.then51 ]
  %unLo.0.lcssa = phi i32 [ %unLo.0.ph, %while.body37.outer ], [ %unLo.0.lcssa.ph, %while.body69.preheader.loopexit ], [ %inc61, %if.then51 ]
  %cmp703646 = icmp sgt i32 %unLo.0.lcssa, %unHi.0.ph
  br i1 %cmp703646, label %while.end117, label %if.end73.lr.ph.preheader

if.end73.lr.ph.preheader:                         ; preds = %while.body69.preheader
  %22 = sext i32 %unLo.0.lcssa to i64
  %23 = sext i32 %gtHi.0.ph to i64
  br label %if.end73.lr.ph

if.end66:                                         ; preds = %if.end62
  %inc67 = add nsw i32 %unLo.025, 1
  %cmp38 = icmp slt i64 %indvars.iv, %13
  %indvars.iv.next = add nsw i64 %indvars.iv, 1
  br i1 %cmp38, label %if.end41, label %while.body69.preheader.loopexit

if.end73:                                         ; preds = %if.end99, %if.end73.lr.ph
  %indvars.iv110 = phi i64 [ %indvars.iv.next111, %if.end99 ], [ %28, %if.end73.lr.ph ]
  %unHi.137 = phi i32 [ %dec100, %if.end99 ], [ %unHi.1.ph48, %if.end73.lr.ph ]
  %arrayidx75 = getelementptr inbounds i32, i32* %ptr, i64 %indvars.iv110
  %24 = load i32, i32* %arrayidx75, align 4, !tbaa !14
  %add76 = add i32 %24, %5
  %idxprom77 = zext i32 %add76 to i64
  %arrayidx78 = getelementptr inbounds i8, i8* %block, i64 %idxprom77
  %25 = load i8, i8* %arrayidx78, align 1, !tbaa !17
  %cmp81 = icmp eq i8 %25, %call
  br i1 %cmp81, label %if.then83, label %if.end95

if.then83:                                        ; preds = %if.end73
  %26 = trunc i64 %indvars.iv110 to i32
  %arrayidx88 = getelementptr inbounds i32, i32* %ptr, i64 %indvars.iv112
  %27 = load i32, i32* %arrayidx88, align 4, !tbaa !14
  store i32 %27, i32* %arrayidx75, align 4, !tbaa !14
  store i32 %24, i32* %arrayidx88, align 4, !tbaa !14
  %dec93 = add nsw i32 %gtHi.1.ph47, -1
  %dec94 = add nsw i32 %26, -1
  %cmp7036 = icmp slt i32 %unLo.0.lcssa, %26
  %indvars.iv.next113 = add i64 %indvars.iv112, -1
  br i1 %cmp7036, label %if.end73.lr.ph, label %while.end117

if.end73.lr.ph:                                   ; preds = %if.end73.lr.ph.preheader, %if.then83
  %indvars.iv112 = phi i64 [ %23, %if.end73.lr.ph.preheader ], [ %indvars.iv.next113, %if.then83 ]
  %unHi.1.ph48 = phi i32 [ %unHi.0.ph, %if.end73.lr.ph.preheader ], [ %dec94, %if.then83 ]
  %gtHi.1.ph47 = phi i32 [ %gtHi.0.ph, %if.end73.lr.ph.preheader ], [ %dec93, %if.then83 ]
  %28 = sext i32 %unHi.1.ph48 to i64
  br label %if.end73

if.end95:                                         ; preds = %if.end73
  %cmp96 = icmp ult i8 %25, %call
  br i1 %cmp96, label %if.end105, label %if.end99

if.end99:                                         ; preds = %if.end95
  %dec100 = add nsw i32 %unHi.137, -1
  %cmp70 = icmp slt i64 %22, %indvars.iv110
  %indvars.iv.next111 = add nsw i64 %indvars.iv110, -1
  br i1 %cmp70, label %if.end73, label %while.end117.loopexit

if.end105:                                        ; preds = %if.end95
  %29 = trunc i64 %indvars.iv112 to i32
  %30 = trunc i64 %indvars.iv110 to i32
  %idxprom107 = sext i32 %unLo.0.lcssa to i64
  %arrayidx108 = getelementptr inbounds i32, i32* %ptr, i64 %idxprom107
  %31 = load i32, i32* %arrayidx108, align 4, !tbaa !14
  store i32 %24, i32* %arrayidx108, align 4, !tbaa !14
  store i32 %31, i32* %arrayidx75, align 4, !tbaa !14
  %inc115 = add nsw i32 %unLo.0.lcssa, 1
  %dec116 = add nsw i32 %30, -1
  br label %while.body37.outer

while.end117.loopexit:                            ; preds = %if.end99
  %32 = trunc i64 %indvars.iv112 to i32
  br label %while.end117

while.end117:                                     ; preds = %while.body69.preheader, %if.then83, %while.end117.loopexit
  %gtHi.1.ph.lcssa13 = phi i32 [ %32, %while.end117.loopexit ], [ %dec93, %if.then83 ], [ %gtHi.0.ph, %while.body69.preheader ]
  %unHi.1.lcssa = phi i32 [ %dec100, %while.end117.loopexit ], [ %dec94, %if.then83 ], [ %unHi.0.ph, %while.body69.preheader ]
  %cmp118 = icmp slt i32 %gtHi.1.ph.lcssa13, %ltLo.0.ph3.lcssa
  br i1 %cmp118, label %if.then120, label %if.end129

if.then120:                                       ; preds = %while.end117
  %add125 = add nsw i32 %5, 1
  store i32 %add125, i32* %arrayidx11, align 4, !tbaa !14
  br label %while.body

if.end129:                                        ; preds = %while.end117
  %sub130 = sub nsw i32 %ltLo.0.ph3.lcssa, %3
  %sub131 = sub nsw i32 %unLo.0.lcssa, %ltLo.0.ph3.lcssa
  %cmp132 = icmp slt i32 %sub130, %sub131
  %sub130.sub131 = select i1 %cmp132, i32 %sub130, i32 %sub131
  %cmp13857 = icmp sgt i32 %sub130.sub131, 0
  br i1 %cmp13857, label %while.body140.preheader, label %while.end153

while.body140.preheader:                          ; preds = %if.end129
  %33 = add i32 %unLo.0.lcssa, 1
  %34 = add i32 %3, -1
  %35 = sub i32 %34, %ltLo.0.ph3.lcssa
  %36 = add i32 %ltLo.0.ph3.lcssa, -1
  %37 = sub i32 %36, %unLo.0.lcssa
  %38 = icmp sgt i32 %35, %37
  %smax = select i1 %38, i32 %35, i32 %37
  %39 = add i32 %33, %smax
  %40 = sext i32 %39 to i64
  %41 = sext i32 %3 to i64
  br label %while.body140

while.body140:                                    ; preds = %while.body140.preheader, %while.body140
  %indvars.iv116 = phi i64 [ %41, %while.body140.preheader ], [ %indvars.iv.next117, %while.body140 ]
  %indvars.iv114 = phi i64 [ %40, %while.body140.preheader ], [ %indvars.iv.next115, %while.body140 ]
  %yyn.060 = phi i32 [ %sub130.sub131, %while.body140.preheader ], [ %dec152, %while.body140 ]
  %arrayidx143 = getelementptr inbounds i32, i32* %ptr, i64 %indvars.iv116
  %42 = load i32, i32* %arrayidx143, align 4, !tbaa !14
  %arrayidx145 = getelementptr inbounds i32, i32* %ptr, i64 %indvars.iv114
  %43 = load i32, i32* %arrayidx145, align 4, !tbaa !14
  store i32 %43, i32* %arrayidx143, align 4, !tbaa !14
  store i32 %42, i32* %arrayidx145, align 4, !tbaa !14
  %dec152 = add nsw i32 %yyn.060, -1
  %cmp138 = icmp sgt i32 %yyn.060, 1
  %indvars.iv.next115 = add nsw i64 %indvars.iv114, 1
  %indvars.iv.next117 = add nsw i64 %indvars.iv116, 1
  br i1 %cmp138, label %while.body140, label %while.end153

while.end153:                                     ; preds = %while.body140, %if.end129
  %sub154 = sub nsw i32 %4, %gtHi.1.ph.lcssa13
  %sub155 = sub nsw i32 %gtHi.1.ph.lcssa13, %unHi.1.lcssa
  %cmp156 = icmp slt i32 %sub154, %sub155
  %sub154.sub155 = select i1 %cmp156, i32 %sub154, i32 %sub155
  %cmp17062 = icmp sgt i32 %sub154.sub155, 0
  br i1 %cmp17062, label %while.body172.preheader, label %while.end185

while.body172.preheader:                          ; preds = %while.end153
  %sub166 = sub nsw i32 %4, %sub154.sub155
  %44 = sext i32 %unLo.0.lcssa to i64
  br label %while.body172

while.body172:                                    ; preds = %while.body172.preheader, %while.body172
  %indvars.iv121 = phi i64 [ %44, %while.body172.preheader ], [ %indvars.iv.next122, %while.body172 ]
  %yyp2165.065.in = phi i32 [ %sub166, %while.body172.preheader ], [ %yyp2165.065, %while.body172 ]
  %yyn168.064 = phi i32 [ %sub154.sub155, %while.body172.preheader ], [ %dec184, %while.body172 ]
  %yyp2165.065 = add nsw i32 %yyp2165.065.in, 1
  %arrayidx175 = getelementptr inbounds i32, i32* %ptr, i64 %indvars.iv121
  %45 = load i32, i32* %arrayidx175, align 4, !tbaa !14
  %idxprom176 = sext i32 %yyp2165.065 to i64
  %arrayidx177 = getelementptr inbounds i32, i32* %ptr, i64 %idxprom176
  %46 = load i32, i32* %arrayidx177, align 4, !tbaa !14
  store i32 %46, i32* %arrayidx175, align 4, !tbaa !14
  store i32 %45, i32* %arrayidx177, align 4, !tbaa !14
  %dec184 = add nsw i32 %yyn168.064, -1
  %cmp170 = icmp sgt i32 %yyn168.064, 1
  %indvars.iv.next122 = add nsw i64 %indvars.iv121, 1
  br i1 %cmp170, label %while.body172, label %while.end185

while.end185:                                     ; preds = %while.body172, %while.end153
  %add186 = sub i32 %3, %ltLo.0.ph3.lcssa
  %sub187 = add i32 %add186, %unLo.0.lcssa
  %sub188 = add nsw i32 %sub187, -1
  %sub190 = sub nsw i32 %4, %sub155
  %add191 = add nsw i32 %sub190, 1
  %add202 = add nsw i32 %5, 1
  %sub206 = sub nsw i32 %sub188, %3
  %sub209 = sub nsw i32 %4, %add191
  %cmp210 = icmp slt i32 %sub206, %sub209
  %.add191 = select i1 %cmp210, i32 %3, i32 %add191
  %add191. = select i1 %cmp210, i32 %add191, i32 %3
  %sub188. = select i1 %cmp210, i32 %sub188, i32 %4
  %.sub188 = select i1 %cmp210, i32 %4, i32 %sub188
  %sub228 = sub nsw i32 %sub188., %.add191
  %sub231 = sub nsw i32 %sub190, %sub187
  %cmp232 = icmp slt i32 %sub228, %sub231
  %sub187..add191 = select i1 %cmp232, i32 %sub187, i32 %.add191
  %sub188..sub190 = select i1 %cmp232, i32 %sub188., i32 %sub190
  %sub190.sub188. = select i1 %cmp232, i32 %sub190, i32 %sub188.
  %.add202 = select i1 %cmp232, i32 %5, i32 %add202
  %add202. = select i1 %cmp232, i32 %add202, i32 %5
  %.add191.sub187 = select i1 %cmp232, i32 %.add191, i32 %sub187
  %sub251 = sub nsw i32 %.sub188, %add191.
  %sub254 = sub nsw i32 %sub190.sub188., %sub187..add191
  %cmp255 = icmp slt i32 %sub251, %sub254
  %add191..nextLo.sroa.10.1 = select i1 %cmp255, i32 %add191., i32 %sub187..add191
  %nextLo.sroa.10.1.add191. = select i1 %cmp255, i32 %sub187..add191, i32 %add191.
  %.sub188.nextHi.sroa.10.1 = select i1 %cmp255, i32 %.sub188, i32 %sub190.sub188.
  %nextHi.sroa.10.1..sub188 = select i1 %cmp255, i32 %sub190.sub188., i32 %.sub188
  %.nextD.sroa.8.1 = select i1 %cmp255, i32 %5, i32 %add202.
  %nextD.sroa.8.1. = select i1 %cmp255, i32 %add202., i32 %5
  store i32 %nextLo.sroa.10.1.add191., i32* %arrayidx7, align 4, !tbaa !14
  store i32 %nextHi.sroa.10.1..sub188, i32* %arrayidx9, align 4, !tbaa !14
  store i32 %nextD.sroa.8.1., i32* %arrayidx11, align 4, !tbaa !14
  %idxprom283 = sext i32 %sp.0.ph66 to i64
  %arrayidx284 = getelementptr inbounds [100 x i32], [100 x i32]* %stackLo, i64 0, i64 %idxprom283
  store i32 %add191..nextLo.sroa.10.1, i32* %arrayidx284, align 4, !tbaa !14
  %arrayidx287 = getelementptr inbounds [100 x i32], [100 x i32]* %stackHi, i64 0, i64 %idxprom283
  store i32 %.sub188.nextHi.sroa.10.1, i32* %arrayidx287, align 4, !tbaa !14
  %arrayidx290 = getelementptr inbounds [100 x i32], [100 x i32]* %stackD, i64 0, i64 %idxprom283
  store i32 %.nextD.sroa.8.1, i32* %arrayidx290, align 4, !tbaa !14
  %inc291 = add nsw i32 %sp.0.ph66, 1
  %idxprom293 = sext i32 %inc291 to i64
  %arrayidx294 = getelementptr inbounds [100 x i32], [100 x i32]* %stackLo, i64 0, i64 %idxprom293
  store i32 %.add191.sub187, i32* %arrayidx294, align 4, !tbaa !14
  %arrayidx297 = getelementptr inbounds [100 x i32], [100 x i32]* %stackHi, i64 0, i64 %idxprom293
  store i32 %sub188..sub190, i32* %arrayidx297, align 4, !tbaa !14
  %arrayidx300 = getelementptr inbounds [100 x i32], [100 x i32]* %stackD, i64 0, i64 %idxprom293
  store i32 %.add202, i32* %arrayidx300, align 4, !tbaa !14
  %inc301 = add nsw i32 %sp.0.ph66, 2
  br label %while.cond.outer.backedge

cleanup:                                          ; preds = %if.then14, %while.cond.outer.backedge
  call void @llvm.lifetime.end(i64 400, i8* %2) #7
  call void @llvm.lifetime.end(i64 400, i8* %1) #7
  call void @llvm.lifetime.end(i64 400, i8* %0) #7
  ret void
}

; Function Attrs: norecurse nounwind uwtable
define internal fastcc void @mainSimpleSort(i32* nocapture %ptr, i8* readonly %block, i16* nocapture readonly %quadrant, i32 %nblock, i32 %lo, i32 %hi, i32 %d, i32* nocapture %budget) unnamed_addr #5 {
entry:
  %sub = sub nsw i32 %hi, %lo
  %add = add nsw i32 %sub, 1
  %cmp = icmp slt i32 %add, 2
  br i1 %cmp, label %cleanup, label %while.cond

while.cond:                                       ; preds = %entry, %while.cond
  %indvars.iv203 = phi i64 [ %indvars.iv.next204, %while.cond ], [ 0, %entry ]
  %arrayidx = getelementptr inbounds [14 x i32], [14 x i32]* @incs, i64 0, i64 %indvars.iv203
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !14
  %cmp1 = icmp sgt i32 %0, %sub
  %indvars.iv.next204 = add nuw nsw i64 %indvars.iv203, 1
  br i1 %cmp1, label %for.cond.preheader, label %while.cond

for.cond.preheader:                               ; preds = %while.cond
  %1 = trunc i64 %indvars.iv203 to i32
  %cmp2196 = icmp sgt i32 %1, 0
  br i1 %cmp2196, label %for.body.preheader, label %cleanup

for.body.preheader:                               ; preds = %for.cond.preheader
  %2 = sext i32 %hi to i64
  %3 = sext i32 %hi to i64
  %4 = sext i32 %hi to i64
  %sext = shl i64 %indvars.iv203, 32
  %5 = ashr exact i64 %sext, 32
  br label %for.body

for.cond.loopexit:                                ; preds = %while.end31, %while.body7, %while.end60
  %cmp2 = icmp sgt i64 %indvars.iv201, 1
  br i1 %cmp2, label %for.body, label %cleanup

for.body:                                         ; preds = %for.body.preheader, %for.cond.loopexit
  %indvars.iv201 = phi i64 [ %5, %for.body.preheader ], [ %indvars.iv.next202, %for.cond.loopexit ]
  %indvars.iv.next202 = add nsw i64 %indvars.iv201, -1
  %arrayidx4 = getelementptr inbounds [14 x i32], [14 x i32]* @incs, i64 0, i64 %indvars.iv.next202
  %6 = load i32, i32* %arrayidx4, align 4, !tbaa !14
  %add5 = add nsw i32 %6, %lo
  %7 = add i32 %6, %lo
  %8 = sext i32 %7 to i64
  br label %while.body7

while.body7:                                      ; preds = %while.end89, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %while.end89 ], [ %8, %for.body ]
  %cmp8 = icmp sgt i64 %indvars.iv, %2
  br i1 %cmp8, label %for.cond.loopexit, label %if.end10

if.end10:                                         ; preds = %while.body7
  %arrayidx12 = getelementptr inbounds i32, i32* %ptr, i64 %indvars.iv
  %9 = load i32, i32* %arrayidx12, align 4, !tbaa !14
  %add18 = add i32 %9, %d
  %10 = trunc i64 %indvars.iv to i32
  br label %while.cond13

while.cond13:                                     ; preds = %while.body19, %if.end10
  %j.0 = phi i32 [ %10, %if.end10 ], [ %sub14, %while.body19 ]
  %sub14 = sub nsw i32 %j.0, %6
  %idxprom15 = sext i32 %sub14 to i64
  %arrayidx16 = getelementptr inbounds i32, i32* %ptr, i64 %idxprom15
  %11 = load i32, i32* %arrayidx16, align 4, !tbaa !14
  %add17 = add i32 %11, %d
  %call = tail call fastcc zeroext i8 @mainGtU(i32 %add17, i32 %add18, i8* %block, i16* %quadrant, i32 %nblock, i32* %budget)
  %tobool = icmp eq i8 %call, 0
  br i1 %tobool, label %while.end31, label %while.body19

while.body19:                                     ; preds = %while.cond13
  %12 = load i32, i32* %arrayidx16, align 4, !tbaa !14
  %idxprom23 = sext i32 %j.0 to i64
  %arrayidx24 = getelementptr inbounds i32, i32* %ptr, i64 %idxprom23
  store i32 %12, i32* %arrayidx24, align 4, !tbaa !14
  %cmp28 = icmp slt i32 %sub14, %add5
  br i1 %cmp28, label %while.end31, label %while.cond13

while.end31:                                      ; preds = %while.cond13, %while.body19
  %j.1 = phi i32 [ %sub14, %while.body19 ], [ %j.0, %while.cond13 ]
  %idxprom32 = sext i32 %j.1 to i64
  %arrayidx33 = getelementptr inbounds i32, i32* %ptr, i64 %idxprom32
  store i32 %9, i32* %arrayidx33, align 4, !tbaa !14
  %13 = add nsw i64 %indvars.iv, 1
  %cmp35 = icmp slt i64 %indvars.iv, %3
  br i1 %cmp35, label %if.end37, label %for.cond.loopexit

if.end37:                                         ; preds = %while.end31
  %arrayidx39 = getelementptr inbounds i32, i32* %ptr, i64 %13
  %14 = load i32, i32* %arrayidx39, align 4, !tbaa !14
  %add45 = add i32 %14, %d
  %15 = trunc i64 %13 to i32
  br label %while.cond40

while.cond40:                                     ; preds = %while.body48, %if.end37
  %j.2 = phi i32 [ %15, %if.end37 ], [ %sub41, %while.body48 ]
  %sub41 = sub nsw i32 %j.2, %6
  %idxprom42 = sext i32 %sub41 to i64
  %arrayidx43 = getelementptr inbounds i32, i32* %ptr, i64 %idxprom42
  %16 = load i32, i32* %arrayidx43, align 4, !tbaa !14
  %add44 = add i32 %16, %d
  %call46 = tail call fastcc zeroext i8 @mainGtU(i32 %add44, i32 %add45, i8* %block, i16* %quadrant, i32 %nblock, i32* %budget)
  %tobool47 = icmp eq i8 %call46, 0
  br i1 %tobool47, label %while.end60, label %while.body48

while.body48:                                     ; preds = %while.cond40
  %17 = load i32, i32* %arrayidx43, align 4, !tbaa !14
  %idxprom52 = sext i32 %j.2 to i64
  %arrayidx53 = getelementptr inbounds i32, i32* %ptr, i64 %idxprom52
  store i32 %17, i32* %arrayidx53, align 4, !tbaa !14
  %cmp57 = icmp slt i32 %sub41, %add5
  br i1 %cmp57, label %while.end60, label %while.cond40

while.end60:                                      ; preds = %while.cond40, %while.body48
  %j.3 = phi i32 [ %sub41, %while.body48 ], [ %j.2, %while.cond40 ]
  %idxprom61 = sext i32 %j.3 to i64
  %arrayidx62 = getelementptr inbounds i32, i32* %ptr, i64 %idxprom61
  store i32 %14, i32* %arrayidx62, align 4, !tbaa !14
  %18 = add nsw i64 %indvars.iv, 2
  %cmp64 = icmp sgt i64 %18, %4
  br i1 %cmp64, label %for.cond.loopexit, label %if.end66

if.end66:                                         ; preds = %while.end60
  %arrayidx68 = getelementptr inbounds i32, i32* %ptr, i64 %18
  %19 = load i32, i32* %arrayidx68, align 4, !tbaa !14
  %add74 = add i32 %19, %d
  %20 = trunc i64 %18 to i32
  br label %while.cond69

while.cond69:                                     ; preds = %while.body77, %if.end66
  %j.4 = phi i32 [ %20, %if.end66 ], [ %sub70, %while.body77 ]
  %sub70 = sub nsw i32 %j.4, %6
  %idxprom71 = sext i32 %sub70 to i64
  %arrayidx72 = getelementptr inbounds i32, i32* %ptr, i64 %idxprom71
  %21 = load i32, i32* %arrayidx72, align 4, !tbaa !14
  %add73 = add i32 %21, %d
  %call75 = tail call fastcc zeroext i8 @mainGtU(i32 %add73, i32 %add74, i8* %block, i16* %quadrant, i32 %nblock, i32* %budget)
  %tobool76 = icmp eq i8 %call75, 0
  br i1 %tobool76, label %while.end89, label %while.body77

while.body77:                                     ; preds = %while.cond69
  %22 = load i32, i32* %arrayidx72, align 4, !tbaa !14
  %idxprom81 = sext i32 %j.4 to i64
  %arrayidx82 = getelementptr inbounds i32, i32* %ptr, i64 %idxprom81
  store i32 %22, i32* %arrayidx82, align 4, !tbaa !14
  %cmp86 = icmp slt i32 %sub70, %add5
  br i1 %cmp86, label %while.end89, label %while.cond69

while.end89:                                      ; preds = %while.cond69, %while.body77
  %j.5 = phi i32 [ %sub70, %while.body77 ], [ %j.4, %while.cond69 ]
  %idxprom90 = sext i32 %j.5 to i64
  %arrayidx91 = getelementptr inbounds i32, i32* %ptr, i64 %idxprom90
  store i32 %19, i32* %arrayidx91, align 4, !tbaa !14
  %23 = load i32, i32* %budget, align 4, !tbaa !14
  %cmp93 = icmp slt i32 %23, 0
  %indvars.iv.next = add i64 %indvars.iv, 3
  br i1 %cmp93, label %cleanup, label %while.body7

cleanup:                                          ; preds = %for.cond.loopexit, %while.end89, %for.cond.preheader, %entry
  ret void
}

; Function Attrs: inlinehint norecurse nounwind readnone uwtable
define internal fastcc zeroext i8 @mmed3(i8 zeroext %a, i8 zeroext %b, i8 zeroext %c) unnamed_addr #6 {
entry:
  %cmp = icmp ugt i8 %a, %b
  %a.b = select i1 %cmp, i8 %a, i8 %b
  %b.a = select i1 %cmp, i8 %b, i8 %a
  %cmp5 = icmp ugt i8 %a.b, %c
  br i1 %cmp5, label %if.then7, label %if.end14

if.then7:                                         ; preds = %entry
  %cmp10 = icmp ugt i8 %b.a, %c
  %b.a.c = select i1 %cmp10, i8 %b.a, i8 %c
  ret i8 %b.a.c

if.end14:                                         ; preds = %entry
  ret i8 %a.b
}

; Function Attrs: inlinehint norecurse nounwind uwtable
define internal fastcc zeroext i8 @mainGtU(i32 %i1, i32 %i2, i8* readonly %block, i16* nocapture readonly %quadrant, i32 %nblock, i32* nocapture %budget) unnamed_addr #4 {
entry:
  %idxprom = zext i32 %i1 to i64
  %arrayidx = getelementptr inbounds i8, i8* %block, i64 %idxprom
  %0 = load i8, i8* %arrayidx, align 1, !tbaa !17
  %idxprom1 = zext i32 %i2 to i64
  %arrayidx2 = getelementptr inbounds i8, i8* %block, i64 %idxprom1
  %1 = load i8, i8* %arrayidx2, align 1, !tbaa !17
  %cmp = icmp eq i8 %0, %1
  br i1 %cmp, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  %cmp7 = icmp ugt i8 %0, %1
  br label %cleanup

if.end:                                           ; preds = %entry
  %inc = add i32 %i1, 1
  %inc10 = add i32 %i2, 1
  %idxprom11 = zext i32 %inc to i64
  %arrayidx12 = getelementptr inbounds i8, i8* %block, i64 %idxprom11
  %2 = load i8, i8* %arrayidx12, align 1, !tbaa !17
  %idxprom13 = zext i32 %inc10 to i64
  %arrayidx14 = getelementptr inbounds i8, i8* %block, i64 %idxprom13
  %3 = load i8, i8* %arrayidx14, align 1, !tbaa !17
  %cmp17 = icmp eq i8 %2, %3
  br i1 %cmp17, label %if.end25, label %if.then19

if.then19:                                        ; preds = %if.end
  %cmp22 = icmp ugt i8 %2, %3
  br label %cleanup

if.end25:                                         ; preds = %if.end
  %inc26 = add i32 %i1, 2
  %inc27 = add i32 %i2, 2
  %idxprom28 = zext i32 %inc26 to i64
  %arrayidx29 = getelementptr inbounds i8, i8* %block, i64 %idxprom28
  %4 = load i8, i8* %arrayidx29, align 1, !tbaa !17
  %idxprom30 = zext i32 %inc27 to i64
  %arrayidx31 = getelementptr inbounds i8, i8* %block, i64 %idxprom30
  %5 = load i8, i8* %arrayidx31, align 1, !tbaa !17
  %cmp34 = icmp eq i8 %4, %5
  br i1 %cmp34, label %if.end42, label %if.then36

if.then36:                                        ; preds = %if.end25
  %cmp39 = icmp ugt i8 %4, %5
  br label %cleanup

if.end42:                                         ; preds = %if.end25
  %inc43 = add i32 %i1, 3
  %inc44 = add i32 %i2, 3
  %idxprom45 = zext i32 %inc43 to i64
  %arrayidx46 = getelementptr inbounds i8, i8* %block, i64 %idxprom45
  %6 = load i8, i8* %arrayidx46, align 1, !tbaa !17
  %idxprom47 = zext i32 %inc44 to i64
  %arrayidx48 = getelementptr inbounds i8, i8* %block, i64 %idxprom47
  %7 = load i8, i8* %arrayidx48, align 1, !tbaa !17
  %cmp51 = icmp eq i8 %6, %7
  br i1 %cmp51, label %if.end59, label %if.then53

if.then53:                                        ; preds = %if.end42
  %cmp56 = icmp ugt i8 %6, %7
  br label %cleanup

if.end59:                                         ; preds = %if.end42
  %inc60 = add i32 %i1, 4
  %inc61 = add i32 %i2, 4
  %idxprom62 = zext i32 %inc60 to i64
  %arrayidx63 = getelementptr inbounds i8, i8* %block, i64 %idxprom62
  %8 = load i8, i8* %arrayidx63, align 1, !tbaa !17
  %idxprom64 = zext i32 %inc61 to i64
  %arrayidx65 = getelementptr inbounds i8, i8* %block, i64 %idxprom64
  %9 = load i8, i8* %arrayidx65, align 1, !tbaa !17
  %cmp68 = icmp eq i8 %8, %9
  br i1 %cmp68, label %if.end76, label %if.then70

if.then70:                                        ; preds = %if.end59
  %cmp73 = icmp ugt i8 %8, %9
  br label %cleanup

if.end76:                                         ; preds = %if.end59
  %inc77 = add i32 %i1, 5
  %inc78 = add i32 %i2, 5
  %idxprom79 = zext i32 %inc77 to i64
  %arrayidx80 = getelementptr inbounds i8, i8* %block, i64 %idxprom79
  %10 = load i8, i8* %arrayidx80, align 1, !tbaa !17
  %idxprom81 = zext i32 %inc78 to i64
  %arrayidx82 = getelementptr inbounds i8, i8* %block, i64 %idxprom81
  %11 = load i8, i8* %arrayidx82, align 1, !tbaa !17
  %cmp85 = icmp eq i8 %10, %11
  br i1 %cmp85, label %if.end93, label %if.then87

if.then87:                                        ; preds = %if.end76
  %cmp90 = icmp ugt i8 %10, %11
  br label %cleanup

if.end93:                                         ; preds = %if.end76
  %inc94 = add i32 %i1, 6
  %inc95 = add i32 %i2, 6
  %idxprom96 = zext i32 %inc94 to i64
  %arrayidx97 = getelementptr inbounds i8, i8* %block, i64 %idxprom96
  %12 = load i8, i8* %arrayidx97, align 1, !tbaa !17
  %idxprom98 = zext i32 %inc95 to i64
  %arrayidx99 = getelementptr inbounds i8, i8* %block, i64 %idxprom98
  %13 = load i8, i8* %arrayidx99, align 1, !tbaa !17
  %cmp102 = icmp eq i8 %12, %13
  br i1 %cmp102, label %if.end110, label %if.then104

if.then104:                                       ; preds = %if.end93
  %cmp107 = icmp ugt i8 %12, %13
  br label %cleanup

if.end110:                                        ; preds = %if.end93
  %inc111 = add i32 %i1, 7
  %inc112 = add i32 %i2, 7
  %idxprom113 = zext i32 %inc111 to i64
  %arrayidx114 = getelementptr inbounds i8, i8* %block, i64 %idxprom113
  %14 = load i8, i8* %arrayidx114, align 1, !tbaa !17
  %idxprom115 = zext i32 %inc112 to i64
  %arrayidx116 = getelementptr inbounds i8, i8* %block, i64 %idxprom115
  %15 = load i8, i8* %arrayidx116, align 1, !tbaa !17
  %cmp119 = icmp eq i8 %14, %15
  br i1 %cmp119, label %if.end127, label %if.then121

if.then121:                                       ; preds = %if.end110
  %cmp124 = icmp ugt i8 %14, %15
  br label %cleanup

if.end127:                                        ; preds = %if.end110
  %inc128 = add i32 %i1, 8
  %inc129 = add i32 %i2, 8
  %idxprom130 = zext i32 %inc128 to i64
  %arrayidx131 = getelementptr inbounds i8, i8* %block, i64 %idxprom130
  %16 = load i8, i8* %arrayidx131, align 1, !tbaa !17
  %idxprom132 = zext i32 %inc129 to i64
  %arrayidx133 = getelementptr inbounds i8, i8* %block, i64 %idxprom132
  %17 = load i8, i8* %arrayidx133, align 1, !tbaa !17
  %cmp136 = icmp eq i8 %16, %17
  br i1 %cmp136, label %if.end144, label %if.then138

if.then138:                                       ; preds = %if.end127
  %cmp141 = icmp ugt i8 %16, %17
  br label %cleanup

if.end144:                                        ; preds = %if.end127
  %inc145 = add i32 %i1, 9
  %inc146 = add i32 %i2, 9
  %idxprom147 = zext i32 %inc145 to i64
  %arrayidx148 = getelementptr inbounds i8, i8* %block, i64 %idxprom147
  %18 = load i8, i8* %arrayidx148, align 1, !tbaa !17
  %idxprom149 = zext i32 %inc146 to i64
  %arrayidx150 = getelementptr inbounds i8, i8* %block, i64 %idxprom149
  %19 = load i8, i8* %arrayidx150, align 1, !tbaa !17
  %cmp153 = icmp eq i8 %18, %19
  br i1 %cmp153, label %if.end161, label %if.then155

if.then155:                                       ; preds = %if.end144
  %cmp158 = icmp ugt i8 %18, %19
  br label %cleanup

if.end161:                                        ; preds = %if.end144
  %inc162 = add i32 %i1, 10
  %inc163 = add i32 %i2, 10
  %idxprom164 = zext i32 %inc162 to i64
  %arrayidx165 = getelementptr inbounds i8, i8* %block, i64 %idxprom164
  %20 = load i8, i8* %arrayidx165, align 1, !tbaa !17
  %idxprom166 = zext i32 %inc163 to i64
  %arrayidx167 = getelementptr inbounds i8, i8* %block, i64 %idxprom166
  %21 = load i8, i8* %arrayidx167, align 1, !tbaa !17
  %cmp170 = icmp eq i8 %20, %21
  br i1 %cmp170, label %if.end178, label %if.then172

if.then172:                                       ; preds = %if.end161
  %cmp175 = icmp ugt i8 %20, %21
  br label %cleanup

if.end178:                                        ; preds = %if.end161
  %inc179 = add i32 %i1, 11
  %inc180 = add i32 %i2, 11
  %idxprom181 = zext i32 %inc179 to i64
  %arrayidx182 = getelementptr inbounds i8, i8* %block, i64 %idxprom181
  %22 = load i8, i8* %arrayidx182, align 1, !tbaa !17
  %idxprom183 = zext i32 %inc180 to i64
  %arrayidx184 = getelementptr inbounds i8, i8* %block, i64 %idxprom183
  %23 = load i8, i8* %arrayidx184, align 1, !tbaa !17
  %cmp187 = icmp eq i8 %22, %23
  br i1 %cmp187, label %if.end195, label %if.then189

if.then189:                                       ; preds = %if.end178
  %cmp192 = icmp ugt i8 %22, %23
  br label %cleanup

if.end195:                                        ; preds = %if.end178
  %inc196 = add i32 %i1, 12
  %inc197 = add i32 %i2, 12
  %add = add i32 %nblock, 8
  br label %do.body

do.body:                                          ; preds = %if.end451, %if.end195
  %i2.addr.0 = phi i32 [ %inc197, %if.end195 ], [ %56, %if.end451 ]
  %i1.addr.0 = phi i32 [ %inc196, %if.end195 ], [ %inc452.sub, %if.end451 ]
  %k.0 = phi i32 [ %add, %if.end195 ], [ %sub463, %if.end451 ]
  %idxprom198 = zext i32 %i1.addr.0 to i64
  %arrayidx199 = getelementptr inbounds i8, i8* %block, i64 %idxprom198
  %24 = load i8, i8* %arrayidx199, align 1, !tbaa !17
  %idxprom200 = zext i32 %i2.addr.0 to i64
  %arrayidx201 = getelementptr inbounds i8, i8* %block, i64 %idxprom200
  %25 = load i8, i8* %arrayidx201, align 1, !tbaa !17
  %cmp204 = icmp eq i8 %24, %25
  br i1 %cmp204, label %if.end212, label %if.then206

if.then206:                                       ; preds = %do.body
  %cmp209 = icmp ugt i8 %24, %25
  br label %cleanup

if.end212:                                        ; preds = %do.body
  %arrayidx214 = getelementptr inbounds i16, i16* %quadrant, i64 %idxprom198
  %26 = load i16, i16* %arrayidx214, align 2, !tbaa !18
  %arrayidx216 = getelementptr inbounds i16, i16* %quadrant, i64 %idxprom200
  %27 = load i16, i16* %arrayidx216, align 2, !tbaa !18
  %cmp219 = icmp eq i16 %26, %27
  br i1 %cmp219, label %if.end227, label %if.then221

if.then221:                                       ; preds = %if.end212
  %cmp224 = icmp ugt i16 %26, %27
  br label %cleanup

if.end227:                                        ; preds = %if.end212
  %inc228 = add i32 %i1.addr.0, 1
  %inc229 = add i32 %i2.addr.0, 1
  %idxprom230 = zext i32 %inc228 to i64
  %arrayidx231 = getelementptr inbounds i8, i8* %block, i64 %idxprom230
  %28 = load i8, i8* %arrayidx231, align 1, !tbaa !17
  %idxprom232 = zext i32 %inc229 to i64
  %arrayidx233 = getelementptr inbounds i8, i8* %block, i64 %idxprom232
  %29 = load i8, i8* %arrayidx233, align 1, !tbaa !17
  %cmp236 = icmp eq i8 %28, %29
  br i1 %cmp236, label %if.end244, label %if.then238

if.then238:                                       ; preds = %if.end227
  %cmp241 = icmp ugt i8 %28, %29
  br label %cleanup

if.end244:                                        ; preds = %if.end227
  %arrayidx246 = getelementptr inbounds i16, i16* %quadrant, i64 %idxprom230
  %30 = load i16, i16* %arrayidx246, align 2, !tbaa !18
  %arrayidx248 = getelementptr inbounds i16, i16* %quadrant, i64 %idxprom232
  %31 = load i16, i16* %arrayidx248, align 2, !tbaa !18
  %cmp251 = icmp eq i16 %30, %31
  br i1 %cmp251, label %if.end259, label %if.then253

if.then253:                                       ; preds = %if.end244
  %cmp256 = icmp ugt i16 %30, %31
  br label %cleanup

if.end259:                                        ; preds = %if.end244
  %inc260 = add i32 %i1.addr.0, 2
  %inc261 = add i32 %i2.addr.0, 2
  %idxprom262 = zext i32 %inc260 to i64
  %arrayidx263 = getelementptr inbounds i8, i8* %block, i64 %idxprom262
  %32 = load i8, i8* %arrayidx263, align 1, !tbaa !17
  %idxprom264 = zext i32 %inc261 to i64
  %arrayidx265 = getelementptr inbounds i8, i8* %block, i64 %idxprom264
  %33 = load i8, i8* %arrayidx265, align 1, !tbaa !17
  %cmp268 = icmp eq i8 %32, %33
  br i1 %cmp268, label %if.end276, label %if.then270

if.then270:                                       ; preds = %if.end259
  %cmp273 = icmp ugt i8 %32, %33
  br label %cleanup

if.end276:                                        ; preds = %if.end259
  %arrayidx278 = getelementptr inbounds i16, i16* %quadrant, i64 %idxprom262
  %34 = load i16, i16* %arrayidx278, align 2, !tbaa !18
  %arrayidx280 = getelementptr inbounds i16, i16* %quadrant, i64 %idxprom264
  %35 = load i16, i16* %arrayidx280, align 2, !tbaa !18
  %cmp283 = icmp eq i16 %34, %35
  br i1 %cmp283, label %if.end291, label %if.then285

if.then285:                                       ; preds = %if.end276
  %cmp288 = icmp ugt i16 %34, %35
  br label %cleanup

if.end291:                                        ; preds = %if.end276
  %inc292 = add i32 %i1.addr.0, 3
  %inc293 = add i32 %i2.addr.0, 3
  %idxprom294 = zext i32 %inc292 to i64
  %arrayidx295 = getelementptr inbounds i8, i8* %block, i64 %idxprom294
  %36 = load i8, i8* %arrayidx295, align 1, !tbaa !17
  %idxprom296 = zext i32 %inc293 to i64
  %arrayidx297 = getelementptr inbounds i8, i8* %block, i64 %idxprom296
  %37 = load i8, i8* %arrayidx297, align 1, !tbaa !17
  %cmp300 = icmp eq i8 %36, %37
  br i1 %cmp300, label %if.end308, label %if.then302

if.then302:                                       ; preds = %if.end291
  %cmp305 = icmp ugt i8 %36, %37
  br label %cleanup

if.end308:                                        ; preds = %if.end291
  %arrayidx310 = getelementptr inbounds i16, i16* %quadrant, i64 %idxprom294
  %38 = load i16, i16* %arrayidx310, align 2, !tbaa !18
  %arrayidx312 = getelementptr inbounds i16, i16* %quadrant, i64 %idxprom296
  %39 = load i16, i16* %arrayidx312, align 2, !tbaa !18
  %cmp315 = icmp eq i16 %38, %39
  br i1 %cmp315, label %if.end323, label %if.then317

if.then317:                                       ; preds = %if.end308
  %cmp320 = icmp ugt i16 %38, %39
  br label %cleanup

if.end323:                                        ; preds = %if.end308
  %inc324 = add i32 %i1.addr.0, 4
  %inc325 = add i32 %i2.addr.0, 4
  %idxprom326 = zext i32 %inc324 to i64
  %arrayidx327 = getelementptr inbounds i8, i8* %block, i64 %idxprom326
  %40 = load i8, i8* %arrayidx327, align 1, !tbaa !17
  %idxprom328 = zext i32 %inc325 to i64
  %arrayidx329 = getelementptr inbounds i8, i8* %block, i64 %idxprom328
  %41 = load i8, i8* %arrayidx329, align 1, !tbaa !17
  %cmp332 = icmp eq i8 %40, %41
  br i1 %cmp332, label %if.end340, label %if.then334

if.then334:                                       ; preds = %if.end323
  %cmp337 = icmp ugt i8 %40, %41
  br label %cleanup

if.end340:                                        ; preds = %if.end323
  %arrayidx342 = getelementptr inbounds i16, i16* %quadrant, i64 %idxprom326
  %42 = load i16, i16* %arrayidx342, align 2, !tbaa !18
  %arrayidx344 = getelementptr inbounds i16, i16* %quadrant, i64 %idxprom328
  %43 = load i16, i16* %arrayidx344, align 2, !tbaa !18
  %cmp347 = icmp eq i16 %42, %43
  br i1 %cmp347, label %if.end355, label %if.then349

if.then349:                                       ; preds = %if.end340
  %cmp352 = icmp ugt i16 %42, %43
  br label %cleanup

if.end355:                                        ; preds = %if.end340
  %inc356 = add i32 %i1.addr.0, 5
  %inc357 = add i32 %i2.addr.0, 5
  %idxprom358 = zext i32 %inc356 to i64
  %arrayidx359 = getelementptr inbounds i8, i8* %block, i64 %idxprom358
  %44 = load i8, i8* %arrayidx359, align 1, !tbaa !17
  %idxprom360 = zext i32 %inc357 to i64
  %arrayidx361 = getelementptr inbounds i8, i8* %block, i64 %idxprom360
  %45 = load i8, i8* %arrayidx361, align 1, !tbaa !17
  %cmp364 = icmp eq i8 %44, %45
  br i1 %cmp364, label %if.end372, label %if.then366

if.then366:                                       ; preds = %if.end355
  %cmp369 = icmp ugt i8 %44, %45
  br label %cleanup

if.end372:                                        ; preds = %if.end355
  %arrayidx374 = getelementptr inbounds i16, i16* %quadrant, i64 %idxprom358
  %46 = load i16, i16* %arrayidx374, align 2, !tbaa !18
  %arrayidx376 = getelementptr inbounds i16, i16* %quadrant, i64 %idxprom360
  %47 = load i16, i16* %arrayidx376, align 2, !tbaa !18
  %cmp379 = icmp eq i16 %46, %47
  br i1 %cmp379, label %if.end387, label %if.then381

if.then381:                                       ; preds = %if.end372
  %cmp384 = icmp ugt i16 %46, %47
  br label %cleanup

if.end387:                                        ; preds = %if.end372
  %inc388 = add i32 %i1.addr.0, 6
  %inc389 = add i32 %i2.addr.0, 6
  %idxprom390 = zext i32 %inc388 to i64
  %arrayidx391 = getelementptr inbounds i8, i8* %block, i64 %idxprom390
  %48 = load i8, i8* %arrayidx391, align 1, !tbaa !17
  %idxprom392 = zext i32 %inc389 to i64
  %arrayidx393 = getelementptr inbounds i8, i8* %block, i64 %idxprom392
  %49 = load i8, i8* %arrayidx393, align 1, !tbaa !17
  %cmp396 = icmp eq i8 %48, %49
  br i1 %cmp396, label %if.end404, label %if.then398

if.then398:                                       ; preds = %if.end387
  %cmp401 = icmp ugt i8 %48, %49
  br label %cleanup

if.end404:                                        ; preds = %if.end387
  %arrayidx406 = getelementptr inbounds i16, i16* %quadrant, i64 %idxprom390
  %50 = load i16, i16* %arrayidx406, align 2, !tbaa !18
  %arrayidx408 = getelementptr inbounds i16, i16* %quadrant, i64 %idxprom392
  %51 = load i16, i16* %arrayidx408, align 2, !tbaa !18
  %cmp411 = icmp eq i16 %50, %51
  br i1 %cmp411, label %if.end419, label %if.then413

if.then413:                                       ; preds = %if.end404
  %cmp416 = icmp ugt i16 %50, %51
  br label %cleanup

if.end419:                                        ; preds = %if.end404
  %inc420 = add i32 %i1.addr.0, 7
  %inc421 = add i32 %i2.addr.0, 7
  %idxprom422 = zext i32 %inc420 to i64
  %arrayidx423 = getelementptr inbounds i8, i8* %block, i64 %idxprom422
  %52 = load i8, i8* %arrayidx423, align 1, !tbaa !17
  %idxprom424 = zext i32 %inc421 to i64
  %arrayidx425 = getelementptr inbounds i8, i8* %block, i64 %idxprom424
  %53 = load i8, i8* %arrayidx425, align 1, !tbaa !17
  %cmp428 = icmp eq i8 %52, %53
  br i1 %cmp428, label %if.end436, label %if.then430

if.then430:                                       ; preds = %if.end419
  %cmp433 = icmp ugt i8 %52, %53
  br label %cleanup

if.end436:                                        ; preds = %if.end419
  %arrayidx438 = getelementptr inbounds i16, i16* %quadrant, i64 %idxprom422
  %54 = load i16, i16* %arrayidx438, align 2, !tbaa !18
  %arrayidx440 = getelementptr inbounds i16, i16* %quadrant, i64 %idxprom424
  %55 = load i16, i16* %arrayidx440, align 2, !tbaa !18
  %cmp443 = icmp eq i16 %54, %55
  br i1 %cmp443, label %if.end451, label %if.then445

if.then445:                                       ; preds = %if.end436
  %cmp448 = icmp ugt i16 %54, %55
  br label %cleanup

if.end451:                                        ; preds = %if.end436
  %inc452 = add i32 %i1.addr.0, 8
  %inc453 = add i32 %i2.addr.0, 8
  %cmp454 = icmp ult i32 %inc452, %nblock
  %sub = select i1 %cmp454, i32 0, i32 %nblock
  %inc452.sub = sub i32 %inc452, %sub
  %cmp458 = icmp ult i32 %inc453, %nblock
  %sub461 = select i1 %cmp458, i32 0, i32 %nblock
  %56 = sub i32 %inc453, %sub461
  %sub463 = add nsw i32 %k.0, -8
  %57 = load i32, i32* %budget, align 4, !tbaa !14
  %dec = add nsw i32 %57, -1
  store i32 %dec, i32* %budget, align 4, !tbaa !14
  %cmp464 = icmp sgt i32 %sub463, -1
  br i1 %cmp464, label %do.body, label %cleanup

cleanup:                                          ; preds = %if.end451, %if.then445, %if.then430, %if.then413, %if.then398, %if.then381, %if.then366, %if.then349, %if.then334, %if.then317, %if.then302, %if.then285, %if.then270, %if.then253, %if.then238, %if.then221, %if.then206, %if.then189, %if.then172, %if.then155, %if.then138, %if.then121, %if.then104, %if.then87, %if.then70, %if.then53, %if.then36, %if.then19, %if.then
  %retval.0.shrunk = phi i1 [ %cmp7, %if.then ], [ %cmp22, %if.then19 ], [ %cmp39, %if.then36 ], [ %cmp56, %if.then53 ], [ %cmp73, %if.then70 ], [ %cmp90, %if.then87 ], [ %cmp107, %if.then104 ], [ %cmp124, %if.then121 ], [ %cmp141, %if.then138 ], [ %cmp158, %if.then155 ], [ %cmp175, %if.then172 ], [ %cmp192, %if.then189 ], [ %cmp209, %if.then206 ], [ %cmp224, %if.then221 ], [ %cmp241, %if.then238 ], [ %cmp256, %if.then253 ], [ %cmp273, %if.then270 ], [ %cmp288, %if.then285 ], [ %cmp305, %if.then302 ], [ %cmp320, %if.then317 ], [ %cmp337, %if.then334 ], [ %cmp352, %if.then349 ], [ %cmp369, %if.then366 ], [ %cmp384, %if.then381 ], [ %cmp401, %if.then398 ], [ %cmp416, %if.then413 ], [ %cmp433, %if.then430 ], [ %cmp448, %if.then445 ], [ false, %if.end451 ]
  %retval.0 = zext i1 %retval.0.shrunk to i8
  ret i8 %retval.0
}

; Function Attrs: nounwind
declare i64 @fwrite(i8* nocapture, i64, i64, %struct._IO_FILE* nocapture) #7

; Function Attrs: argmemonly nounwind
declare void @llvm.memset.p0i8.i64(i8* nocapture, i8, i64, i32, i1) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* nocapture, i8* nocapture readonly, i64, i32, i1) #1

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { inlinehint norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #5 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #6 = { inlinehint norecurse nounwind readnone uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #7 = { nounwind }
attributes #8 = { cold }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 (trunk 2039) (llvm/branches/loopopt 2061)"}
!1 = !{!2, !3, i64 56}
!2 = !{!"", !3, i64 0, !6, i64 8, !6, i64 12, !6, i64 16, !3, i64 24, !3, i64 32, !3, i64 40, !6, i64 48, !3, i64 56, !3, i64 64, !3, i64 72, !3, i64 80, !6, i64 88, !6, i64 92, !6, i64 96, !6, i64 100, !6, i64 104, !6, i64 108, !6, i64 112, !6, i64 116, !6, i64 120, !6, i64 124, !4, i64 128, !4, i64 384, !6, i64 640, !6, i64 644, !6, i64 648, !6, i64 652, !6, i64 656, !6, i64 660, !6, i64 664, !6, i64 668, !4, i64 672, !4, i64 1704, !4, i64 19706, !4, i64 37708, !4, i64 39256, !4, i64 45448, !4, i64 51640}
!3 = !{!"any pointer", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!"int", !4, i64 0}
!7 = !{!2, !3, i64 64}
!8 = !{!2, !3, i64 40}
!9 = !{!2, !6, i64 108}
!10 = !{!2, !6, i64 656}
!11 = !{!2, !6, i64 88}
!12 = !{!2, !3, i64 24}
!13 = !{!2, !3, i64 32}
!14 = !{!6, !6, i64 0}
!15 = !{!3, !3, i64 0}
!16 = !{!2, !6, i64 48}
!17 = !{!4, !4, i64 0}
!18 = !{!19, !19, i64 0}
!19 = !{!"short", !4, i64 0}
