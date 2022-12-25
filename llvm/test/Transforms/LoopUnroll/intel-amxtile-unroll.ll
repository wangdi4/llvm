; RUN: opt -passes="default<O2>" -S %s | FileCheck %s

; This is sample code from the AMX SDK.
; It has C++ code like this:
;
;      for (int n_acc = 0; n_acc < N_ACC; ++n_acc)
;        for (int m_acc = 0; m_acc < M_ACC; ++m_acc)
;          tilezero(tC[m_acc][n_acc]);
;
; The tilezero call must get a constant value from tC.
; To do this, we need to fully unroll the loop and then perform SROA on tC
; to convert it to registers/constants.

; This test makes sure that the optimization pipeline at O2 can do it.

; CHECK: call x86_amx @llvm.x86.tilezero.internal(i16 16, i16 64)
; CHECK-NEXT: call x86_amx @llvm.x86.tilezero.internal(i16 16, i16 64)
; CHECK-NEXT: call x86_amx @llvm.x86.tilezero.internal(i16 16, i16 64)
; CHECK-NEXT: call x86_amx @llvm.x86.tilezero.internal(i16 16, i16 64)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class.Tile = type { %struct.__tile1024i_str }
%struct.__tile1024i_str = type <{ i16, i16, [60 x i8], <256 x i32> }>

@.str = external hidden unnamed_addr constant [64 x i8], align 1
@.str.1 = external hidden unnamed_addr constant [9 x i8], align 1
@__PRETTY_FUNCTION__._Z13inner_productPiS_S_iii = external hidden unnamed_addr constant [55 x i8], align 1

; Function Attrs: noinline nounwind uwtable
define dso_local void @_Z13inner_productPiS_S_iii(i32* noundef %A_mem, i32* noundef %B_mem, i32* noundef %C_mem, i32 noundef %M, i32 noundef %N, i32 noundef %K) local_unnamed_addr #0 {
entry:
  %agg.tmp68163.sroa.5 = alloca [60 x i8], align 4
  %agg.tmp162.sroa.5 = alloca [60 x i8], align 4
  %tC = alloca [2 x [2 x %class.Tile]], align 64
  %tA = alloca [2 x %class.Tile], align 64
  %tB.sroa.7 = alloca [60 x i8], align 4
  %agg.tmp.sroa.0.sroa.3 = alloca [60 x i8], align 4
  %agg.tmp68.sroa.0.sroa.3 = alloca [60 x i8], align 4
  %0 = and i32 %M, 31
  %cmp = icmp eq i32 %0, 0
  %1 = and i32 %N, 31
  %cmp2 = icmp eq i32 %1, 0
  %or.cond = and i1 %cmp, %cmp2
  %2 = and i32 %K, 15
  %cmp4 = icmp eq i32 %2, 0
  %or.cond149 = and i1 %or.cond, %cmp4
  br i1 %or.cond149, label %for.cond, label %cond.false

cond.false:                                       ; preds = %entry
  call void @__assert_fail(i8* noundef getelementptr inbounds ([64 x i8], [64 x i8]* @.str, i64 0, i64 0), i8* noundef getelementptr inbounds ([9 x i8], [9 x i8]* @.str.1, i64 0, i64 0), i32 noundef 50, i8* noundef getelementptr inbounds ([55 x i8], [55 x i8]* @__PRETTY_FUNCTION__._Z13inner_productPiS_S_iii, i64 0, i64 0)) #6
  unreachable

for.cond:                                         ; preds = %for.cond.cleanup8, %entry
  %n.0 = phi i32 [ %add100, %for.cond.cleanup8 ], [ 0, %entry ]
  %cmp5 = icmp slt i32 %n.0, %N
  br i1 %cmp5, label %for.cond6, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  ret void

for.cond6:                                        ; preds = %for.cond.cleanup32, %for.cond
  %m.0 = phi i32 [ %add97, %for.cond.cleanup32 ], [ 0, %for.cond ]
  %cmp7 = icmp slt i32 %m.0, %M
  br i1 %cmp7, label %for.body9, label %for.cond.cleanup8

for.cond.cleanup8:                                ; preds = %for.cond6
  %add100 = add nuw nsw i32 %n.0, 32
  br label %for.cond, !llvm.loop !3

for.body9:                                        ; preds = %for.cond6
  %3 = bitcast [2 x [2 x %class.Tile]]* %tC to i8*
  call void @llvm.lifetime.start.p0i8(i64 4352, i8* nonnull %3) #4
  %array.begin = getelementptr inbounds [2 x [2 x %class.Tile]], [2 x [2 x %class.Tile]]* %tC, i64 0, i64 0, i64 0
  %arrayctor.end = getelementptr inbounds [2 x [2 x %class.Tile]], [2 x [2 x %class.Tile]]* %tC, i64 0, i64 0, i64 4
  br label %arrayctor.loop

arrayctor.loop:                                   ; preds = %arrayctor.loop, %for.body9
  %arrayctor.cur = phi %class.Tile* [ %array.begin, %for.body9 ], [ %arrayctor.next, %arrayctor.loop ]
  %row.i = getelementptr inbounds %class.Tile, %class.Tile* %arrayctor.cur, i64 0, i32 0, i32 0, !intel-tbaa !5
  store i16 16, i16* %row.i, align 64, !tbaa !5
  %col.i = getelementptr inbounds %class.Tile, %class.Tile* %arrayctor.cur, i64 0, i32 0, i32 1, !intel-tbaa !11
  store i16 64, i16* %col.i, align 2, !tbaa !11
  %tile2.i = getelementptr inbounds %class.Tile, %class.Tile* %arrayctor.cur, i64 0, i32 0, i32 3
  store <256 x i32> zeroinitializer, <256 x i32>* %tile2.i, align 64, !tbaa !12
  %arrayctor.next = getelementptr inbounds %class.Tile, %class.Tile* %arrayctor.cur, i64 1
  %arrayctor.done = icmp eq %class.Tile* %arrayctor.next, %arrayctor.end
  br i1 %arrayctor.done, label %arrayctor.cont, label %arrayctor.loop

arrayctor.cont:                                   ; preds = %arrayctor.loop
  %4 = bitcast [2 x %class.Tile]* %tA to i8*
  call void @llvm.lifetime.start.p0i8(i64 2176, i8* nonnull %4) #4
  %array.begin10 = getelementptr inbounds [2 x %class.Tile], [2 x %class.Tile]* %tA, i64 0, i64 0
  %arrayctor.end11 = getelementptr inbounds [2 x %class.Tile], [2 x %class.Tile]* %tA, i64 0, i64 2
  br label %arrayctor.loop12

arrayctor.loop12:                                 ; preds = %arrayctor.loop12, %arrayctor.cont
  %arrayctor.cur13 = phi %class.Tile* [ %array.begin10, %arrayctor.cont ], [ %arrayctor.next14, %arrayctor.loop12 ]
  %row.i150 = getelementptr inbounds %class.Tile, %class.Tile* %arrayctor.cur13, i64 0, i32 0, i32 0, !intel-tbaa !5
  store i16 16, i16* %row.i150, align 64, !tbaa !5
  %col.i151 = getelementptr inbounds %class.Tile, %class.Tile* %arrayctor.cur13, i64 0, i32 0, i32 1, !intel-tbaa !11
  store i16 64, i16* %col.i151, align 2, !tbaa !11
  %tile2.i152 = getelementptr inbounds %class.Tile, %class.Tile* %arrayctor.cur13, i64 0, i32 0, i32 3
  store <256 x i32> zeroinitializer, <256 x i32>* %tile2.i152, align 64, !tbaa !12
  %arrayctor.next14 = getelementptr inbounds %class.Tile, %class.Tile* %arrayctor.cur13, i64 1
  %arrayctor.done15 = icmp eq %class.Tile* %arrayctor.next14, %arrayctor.end11
  br i1 %arrayctor.done15, label %arrayctor.cont16, label %arrayctor.loop12

arrayctor.cont16:                                 ; preds = %arrayctor.loop12
  %tB.sroa.7.0..sroa_idx207 = getelementptr inbounds [60 x i8], [60 x i8]* %tB.sroa.7, i64 0, i64 0
  call void @llvm.lifetime.start.p0i8(i64 60, i8* %tB.sroa.7.0..sroa_idx207)
  br label %for.cond17

for.cond17:                                       ; preds = %for.cond.cleanup23, %arrayctor.cont16
  %n_acc.0 = phi i32 [ 0, %arrayctor.cont16 ], [ %inc28, %for.cond.cleanup23 ]
  %cmp18 = icmp ult i32 %n_acc.0, 2
  br i1 %cmp18, label %for.cond21, label %for.cond30

for.cond21:                                       ; preds = %for.body24, %for.cond17
  %m_acc.0 = phi i32 [ %inc, %for.body24 ], [ 0, %for.cond17 ]
  %cmp22 = icmp ult i32 %m_acc.0, 2
  br i1 %cmp22, label %for.body24, label %for.cond.cleanup23

for.cond.cleanup23:                               ; preds = %for.cond21
  %inc28 = add nuw nsw i32 %n_acc.0, 1
  br label %for.cond17, !llvm.loop !13

for.body24:                                       ; preds = %for.cond21
  %idxprom = zext i32 %m_acc.0 to i64
  %idxprom25 = zext i32 %n_acc.0 to i64
  %arrayidx26 = getelementptr inbounds [2 x [2 x %class.Tile]], [2 x [2 x %class.Tile]]* %tC, i64 0, i64 %idxprom, i64 %idxprom25, !intel-tbaa !14
  %row.i.i = getelementptr inbounds %class.Tile, %class.Tile* %arrayidx26, i64 0, i32 0, i32 0, !intel-tbaa !5
  %5 = load i16, i16* %row.i.i, align 64, !tbaa !17
  %col.i.i = getelementptr inbounds %class.Tile, %class.Tile* %arrayidx26, i64 0, i32 0, i32 1, !intel-tbaa !11
  %6 = load i16, i16* %col.i.i, align 2, !tbaa !18
  %7 = call x86_amx @llvm.x86.tilezero.internal(i16 %5, i16 %6) #4
  %8 = call <256 x i32> @llvm.x86.cast.tile.to.vector.v256i32(x86_amx %7) #4
  %tile.i.i = getelementptr inbounds %class.Tile, %class.Tile* %arrayidx26, i64 0, i32 0, i32 3
  store <256 x i32> %8, <256 x i32>* %tile.i.i, align 64, !tbaa !12
  %inc = add nuw nsw i32 %m_acc.0, 1
  br label %for.cond21, !llvm.loop !19

for.cond30:                                       ; preds = %for.cond.cleanup37, %for.cond17
  %k.0 = phi i32 [ %add94, %for.cond.cleanup37 ], [ 0, %for.cond17 ]
  %cmp31 = icmp slt i32 %k.0, %K
  br i1 %cmp31, label %for.cond35, label %for.cond.cleanup32

for.cond.cleanup32:                               ; preds = %for.cond30
  call void @llvm.lifetime.end.p0i8(i64 60, i8* %tB.sroa.7.0..sroa_idx207)
  call void @llvm.lifetime.end.p0i8(i64 2176, i8* nonnull %4) #4
  call void @llvm.lifetime.end.p0i8(i64 4352, i8* nonnull %3) #4
  %add97 = add nuw nsw i32 %m.0, 32
  br label %for.cond6, !llvm.loop !20

for.cond35:                                       ; preds = %for.cond.cleanup49, %for.cond30
  %n_acc34.0 = phi i32 [ %inc91, %for.cond.cleanup49 ], [ 0, %for.cond30 ]
  %cmp36 = icmp ult i32 %n_acc34.0, 2
  br i1 %cmp36, label %for.body38, label %for.cond.cleanup37

for.cond.cleanup37:                               ; preds = %for.cond35
  %add94 = add nuw nsw i32 %k.0, 16
  br label %for.cond30, !llvm.loop !21

for.body38:                                       ; preds = %for.cond35
  %mul = mul nsw i32 %k.0, %N
  %idx.ext = sext i32 %mul to i64
  %add.ptr = getelementptr inbounds i32, i32* %B_mem, i64 %idx.ext, !intel-tbaa !22
  %idx.ext39 = zext i32 %n.0 to i64
  %add.ptr40 = getelementptr inbounds i32, i32* %add.ptr, i64 %idx.ext39, !intel-tbaa !22
  %mul41 = shl nsw i32 %n_acc34.0, 4
  %9 = zext i32 %mul41 to i64
  %add.ptr43 = getelementptr inbounds i32, i32* %add.ptr40, i64 %9, !intel-tbaa !22
  %10 = bitcast i32* %add.ptr43 to i8*
  %conv = sext i32 %N to i64
  %mul44 = shl nsw i64 %conv, 2
  %11 = call x86_amx @llvm.x86.tileloadd64.internal(i16 16, i16 64, i8* %10, i64 %mul44) #4
  %12 = call <256 x i32> @llvm.x86.cast.tile.to.vector.v256i32(x86_amx %11) #4
  br label %for.cond47

for.cond47:                                       ; preds = %for.inc87, %for.body38
  %m_acc46.0 = phi i32 [ 0, %for.body38 ], [ %inc88, %for.inc87 ]
  %cmp48 = icmp ult i32 %m_acc46.0, 2
  br i1 %cmp48, label %for.body50, label %for.cond.cleanup49

for.cond.cleanup49:                               ; preds = %for.cond47
  %inc91 = add nuw nsw i32 %n_acc34.0, 1
  br label %for.cond35, !llvm.loop !24

for.body50:                                       ; preds = %for.cond47
  %cmp51 = icmp eq i32 %n_acc34.0, 0
  br i1 %cmp51, label %if.then, label %if.end

if.then:                                          ; preds = %for.body50
  %idxprom52 = zext i32 %m_acc46.0 to i64
  %arrayidx53 = getelementptr inbounds [2 x %class.Tile], [2 x %class.Tile]* %tA, i64 0, i64 %idxprom52, !intel-tbaa !25
  %mul54 = shl nsw i32 %m_acc46.0, 4
  %add = add nuw nsw i32 %m.0, %mul54
  %mul55 = mul nsw i32 %add, %K
  %idx.ext56 = sext i32 %mul55 to i64
  %add.ptr57 = getelementptr inbounds i32, i32* %A_mem, i64 %idx.ext56, !intel-tbaa !22
  %idx.ext58 = zext i32 %k.0 to i64
  %add.ptr59 = getelementptr inbounds i32, i32* %add.ptr57, i64 %idx.ext58, !intel-tbaa !22
  %13 = bitcast i32* %add.ptr59 to i8*
  %conv60 = sext i32 %K to i64
  %mul61 = shl nsw i64 %conv60, 2
  %row.i.i159 = getelementptr inbounds %class.Tile, %class.Tile* %arrayidx53, i64 0, i32 0, i32 0, !intel-tbaa !5
  %14 = load i16, i16* %row.i.i159, align 64, !tbaa !26
  %col.i.i160 = getelementptr inbounds %class.Tile, %class.Tile* %arrayidx53, i64 0, i32 0, i32 1, !intel-tbaa !11
  %15 = load i16, i16* %col.i.i160, align 2, !tbaa !27
  %16 = call x86_amx @llvm.x86.tileloadd64.internal(i16 %14, i16 %15, i8* %13, i64 %mul61) #4
  %17 = call <256 x i32> @llvm.x86.cast.tile.to.vector.v256i32(x86_amx %16) #4
  %tile.i.i161 = getelementptr inbounds %class.Tile, %class.Tile* %arrayidx53, i64 0, i32 0, i32 3
  store <256 x i32> %17, <256 x i32>* %tile.i.i161, align 64, !tbaa !12
  br label %if.end

if.end:                                           ; preds = %if.then, %for.body50
  %idxprom62 = zext i32 %m_acc46.0 to i64
  %idxprom64 = zext i32 %n_acc34.0 to i64
  %arrayidx65 = getelementptr inbounds [2 x [2 x %class.Tile]], [2 x [2 x %class.Tile]]* %tC, i64 0, i64 %idxprom62, i64 %idxprom64, !intel-tbaa !14
  %arrayidx67 = getelementptr inbounds [2 x %class.Tile], [2 x %class.Tile]* %tA, i64 0, i64 %idxprom62, !intel-tbaa !25
  %agg.tmp.sroa.0.sroa.0.0.agg.tmp.sroa.0.0..sroa_cast.sroa_idx = getelementptr inbounds %class.Tile, %class.Tile* %arrayidx67, i64 0, i32 0, i32 0
  %agg.tmp.sroa.0.sroa.0.0.copyload = load i16, i16* %agg.tmp.sroa.0.sroa.0.0.agg.tmp.sroa.0.0..sroa_cast.sroa_idx, align 64, !tbaa.struct !28
  %agg.tmp.sroa.0.sroa.2.0.agg.tmp.sroa.0.0..sroa_cast.sroa_idx198 = getelementptr inbounds %class.Tile, %class.Tile* %arrayidx67, i64 0, i32 0, i32 1
  %agg.tmp.sroa.0.sroa.2.0.copyload = load i16, i16* %agg.tmp.sroa.0.sroa.2.0.agg.tmp.sroa.0.0..sroa_cast.sroa_idx198, align 2, !tbaa.struct !30
  %agg.tmp.sroa.0.sroa.3.0.agg.tmp.sroa.0.0..sroa_cast.sroa_idx = getelementptr inbounds %class.Tile, %class.Tile* %arrayidx67, i64 0, i32 0, i32 2, i64 0
  %agg.tmp.sroa.0.sroa.3.0..sroa_idx = getelementptr inbounds [60 x i8], [60 x i8]* %agg.tmp.sroa.0.sroa.3, i64 0, i64 0
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 4 %agg.tmp.sroa.0.sroa.3.0..sroa_idx, i8* align 4 %agg.tmp.sroa.0.sroa.3.0.agg.tmp.sroa.0.0..sroa_cast.sroa_idx, i64 60, i1 false), !tbaa.struct !31
  %agg.tmp.sroa.0.sroa.4.0.agg.tmp.sroa.0.0..sroa_cast.sroa_idx199 = getelementptr inbounds %class.Tile, %class.Tile* %arrayidx67, i64 0, i32 0, i32 3
  %agg.tmp.sroa.0.sroa.4.0.copyload = load <256 x i32>, <256 x i32>* %agg.tmp.sroa.0.sroa.4.0.agg.tmp.sroa.0.0..sroa_cast.sroa_idx199, align 64, !tbaa.struct !32
  %agg.tmp68.sroa.0.sroa.3.4..sroa_idx = getelementptr inbounds [60 x i8], [60 x i8]* %agg.tmp68.sroa.0.sroa.3, i64 0, i64 0
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 4 %agg.tmp68.sroa.0.sroa.3.4..sroa_idx, i8* align 4 %tB.sroa.7.0..sroa_idx207, i64 60, i1 false), !tbaa.struct !31
  %agg.tmp162.sroa.5.0..sroa_idx209 = getelementptr inbounds [60 x i8], [60 x i8]* %agg.tmp162.sroa.5, i64 0, i64 0
  call void @llvm.lifetime.start.p0i8(i64 60, i8* %agg.tmp162.sroa.5.0..sroa_idx209)
  %agg.tmp68163.sroa.5.0..sroa_idx = getelementptr inbounds [60 x i8], [60 x i8]* %agg.tmp68163.sroa.5, i64 0, i64 0
  call void @llvm.lifetime.start.p0i8(i64 60, i8* %agg.tmp68163.sroa.5.0..sroa_idx)
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 4 %agg.tmp68163.sroa.5.0..sroa_idx, i8* align 4 %agg.tmp68.sroa.0.sroa.3.4..sroa_idx, i64 60, i1 false)
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 4 %agg.tmp162.sroa.5.0..sroa_idx209, i8* align 4 %agg.tmp.sroa.0.sroa.3.0..sroa_idx, i64 60, i1 false)
  %tile.i.i164 = getelementptr inbounds %class.Tile, %class.Tile* %arrayidx65, i64 0, i32 0, i32 3
  %18 = load <256 x i32>, <256 x i32>* %tile.i.i164, align 64, !tbaa !12
  %19 = call x86_amx @llvm.x86.cast.vector.to.tile.v256i32(<256 x i32> %18) #4
  %20 = call x86_amx @llvm.x86.cast.vector.to.tile.v256i32(<256 x i32> %agg.tmp.sroa.0.sroa.4.0.copyload) #4
  %21 = call x86_amx @llvm.x86.cast.vector.to.tile.v256i32(<256 x i32> %12) #4
  %22 = call x86_amx @llvm.x86.tdpbssd.internal(i16 %agg.tmp.sroa.0.sroa.0.0.copyload, i16 64, i16 %agg.tmp.sroa.0.sroa.2.0.copyload, x86_amx %19, x86_amx %20, x86_amx %21) #4
  %23 = call <256 x i32> @llvm.x86.cast.tile.to.vector.v256i32(x86_amx %22) #4
  store <256 x i32> %23, <256 x i32>* %tile.i.i164, align 64, !tbaa !12
  call void @llvm.lifetime.end.p0i8(i64 60, i8* %agg.tmp162.sroa.5.0..sroa_idx209)
  call void @llvm.lifetime.end.p0i8(i64 60, i8* %agg.tmp68163.sroa.5.0..sroa_idx)
  %sub = add nsw i32 %K, -16
  %cmp69 = icmp eq i32 %k.0, %sub
  br i1 %cmp69, label %if.then70, label %for.inc87

if.then70:                                        ; preds = %if.end
  %mul71 = shl nsw i32 %m_acc46.0, 4
  %add72 = add nuw nsw i32 %m.0, %mul71
  %add74 = add nuw nsw i32 %n.0, %mul41
  %mul79 = mul nsw i32 %add72, %N
  %idx.ext80 = sext i32 %mul79 to i64
  %add.ptr81 = getelementptr inbounds i32, i32* %C_mem, i64 %idx.ext80, !intel-tbaa !22
  %idx.ext82 = zext i32 %add74 to i64
  %add.ptr83 = getelementptr inbounds i32, i32* %add.ptr81, i64 %idx.ext82, !intel-tbaa !22
  %24 = bitcast i32* %add.ptr83 to i8*
  %agg.tmp.sroa.0.0..sroa_idx.i165 = getelementptr inbounds %class.Tile, %class.Tile* %arrayidx65, i64 0, i32 0, i32 0
  %agg.tmp.sroa.0.0.copyload.i166 = load i16, i16* %agg.tmp.sroa.0.0..sroa_idx.i165, align 64, !tbaa.struct !28
  %agg.tmp.sroa.2.0..sroa_idx14.i = getelementptr inbounds %class.Tile, %class.Tile* %arrayidx65, i64 0, i32 0, i32 1
  %agg.tmp.sroa.2.0.copyload.i167 = load i16, i16* %agg.tmp.sroa.2.0..sroa_idx14.i, align 2, !tbaa.struct !30
  %25 = call x86_amx @llvm.x86.cast.vector.to.tile.v256i32(<256 x i32> %23) #4
  call void @llvm.x86.tilestored64.internal(i16 %agg.tmp.sroa.0.0.copyload.i166, i16 %agg.tmp.sroa.2.0.copyload.i167, i8* %24, i64 %mul44, x86_amx %25) #4
  br label %for.inc87

for.inc87:                                        ; preds = %if.then70, %if.end
  %inc88 = add nuw nsw i32 %m_acc46.0, 1
  br label %for.cond47, !llvm.loop !33
}

; Function Attrs: nofree noreturn nounwind
declare dso_local void @__assert_fail(i8* noundef, i8* noundef, i32 noundef, i8* noundef) local_unnamed_addr #1

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #2

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #2

; Function Attrs: argmemonly nofree nounwind willreturn
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* noalias nocapture writeonly, i8* noalias nocapture readonly, i64, i1 immarg) #3

; Function Attrs: nounwind
declare x86_amx @llvm.x86.tilezero.internal(i16, i16) #4

; Function Attrs: nounwind readnone
declare <256 x i32> @llvm.x86.cast.tile.to.vector.v256i32(x86_amx) #5

; Function Attrs: nounwind
declare x86_amx @llvm.x86.tileloadd64.internal(i16, i16, i8*, i64) #4

; Function Attrs: nounwind
declare x86_amx @llvm.x86.tdpbssd.internal(i16, i16, i16, x86_amx, x86_amx, x86_amx) #4

; Function Attrs: nounwind readnone
declare x86_amx @llvm.x86.cast.vector.to.tile.v256i32(<256 x i32>) #5

; Function Attrs: nounwind
declare void @llvm.x86.tilestored64.internal(i16, i16, i8*, i64, x86_amx) #4

attributes #0 = { noinline nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "min-legal-vector-width"="8192" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="sapphirerapids" "target-features"="+adx,+aes,+amx-bf16,+amx-int8,+amx-tile,+avx,+avx2,+avx512bf16,+avx512bitalg,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512fp16,+avx512ifma,+avx512vbmi,+avx512vbmi2,+avx512vl,+avx512vnni,+avx512vp2intersect,+avx512vpopcntdq,+avxvnni,+bmi,+bmi2,+cldemote,+clflushopt,+clwb,+crc32,+cx16,+cx8,+enqcmd,+f16c,+fma,+fsgsbase,+fxsr,+gfni,+invpcid,+lzcnt,+mmx,+movbe,+movdir64b,+movdiri,+pclmul,+pconfig,+pku,+popcnt,+prfchw,+ptwrite,+rdpid,+rdrnd,+rdseed,+sahf,+serialize,+sgx,+sha,+shstk,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+tsxldtrk,+uintr,+vaes,+vpclmulqdq,+waitpkg,+wbnoinvd,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #1 = { nofree noreturn nounwind "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="sapphirerapids" "target-features"="+adx,+aes,+amx-bf16,+amx-int8,+amx-tile,+avx,+avx2,+avx512bf16,+avx512bitalg,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512fp16,+avx512ifma,+avx512vbmi,+avx512vbmi2,+avx512vl,+avx512vnni,+avx512vp2intersect,+avx512vpopcntdq,+avxvnni,+bmi,+bmi2,+cldemote,+clflushopt,+clwb,+crc32,+cx16,+cx8,+enqcmd,+f16c,+fma,+fsgsbase,+fxsr,+gfni,+invpcid,+lzcnt,+mmx,+movbe,+movdir64b,+movdiri,+pclmul,+pconfig,+pku,+popcnt,+prfchw,+ptwrite,+rdpid,+rdrnd,+rdseed,+sahf,+serialize,+sgx,+sha,+shstk,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+tsxldtrk,+uintr,+vaes,+vpclmulqdq,+waitpkg,+wbnoinvd,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #2 = { argmemonly nocallback nofree nosync nounwind willreturn }
attributes #3 = { argmemonly nofree nounwind willreturn }
attributes #4 = { nounwind }
attributes #5 = { nounwind readnone }
attributes #6 = { noreturn nounwind }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!3 = distinct !{!3, !4}
!4 = !{!"llvm.loop.mustprogress"}
!5 = !{!6, !8, i64 0}
!6 = !{!"struct@_ZTS4TileILs16ELs16EE", !7, i64 0}
!7 = !{!"struct@_ZTS15__tile1024i_str", !8, i64 0, !8, i64 2, !9, i64 64}
!8 = !{!"short", !9, i64 0}
!9 = !{!"omnipotent char", !10, i64 0}
!10 = !{!"Simple C++ TBAA"}
!11 = !{!6, !8, i64 2}
!12 = !{!9, !9, i64 0}
!13 = distinct !{!13, !4}
!14 = !{!15, !6, i64 0}
!15 = !{!"array@_ZTSA2_A2_4TileILs16ELs16EE", !16, i64 0}
!16 = !{!"array@_ZTSA2_4TileILs16ELs16EE", !6, i64 0}
!17 = !{!15, !8, i64 0}
!18 = !{!15, !8, i64 2}
!19 = distinct !{!19, !4}
!20 = distinct !{!20, !4}
!21 = distinct !{!21, !4}
!22 = !{!23, !23, i64 0}
!23 = !{!"int", !9, i64 0}
!24 = distinct !{!24, !4}
!25 = !{!16, !6, i64 0}
!26 = !{!16, !8, i64 0}
!27 = !{!16, !8, i64 2}
!28 = !{i64 0, i64 2, !29, i64 2, i64 2, !29, i64 64, i64 1024, !12}
!29 = !{!8, !8, i64 0}
!30 = !{i64 0, i64 2, !29, i64 62, i64 1024, !12}
!31 = !{i64 60, i64 1024, !12}
!32 = !{i64 0, i64 1024, !12}
!33 = distinct !{!33, !4}
