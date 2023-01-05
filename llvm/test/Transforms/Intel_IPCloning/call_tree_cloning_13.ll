; REQUIRES: asserts

; [DEBUG+TRACE]
; Show that inexperienced developers can tell whether call-tree cloning, post-processing, and multi-versioning
; transformations happen by checking the debug output stream.
;
; RUN: opt < %s -passes='module(call-tree-clone)' -debug-only=call-tree-clone \
; RUN:       -call-tree-clone-detail-log=1 -post-processing-detail-log=1 \
; RUN:       -multiversioning-detail-log=1 -enable-intel-advanced-opts=1 \
; RUN:       -mtriple=i686-- -mattr=+avx2 \
; RUN:       -call-tree-clone-mv-bypass-coll-for-littest=1 -disable-output 2>&1 | FileCheck %s

; ** Call-Tree Cloning related trace ***
; CHECK:    Call-Tree Cloning preliminary analysis:  good
; CHECK:    Call-Tree Cloning collection:  good
; CHECK:    Call-Tree Cloning analysis:  good
; CHECK:    Call-Tree Cloning transformation:  good
; CHECK:    Call-Tree Cloning Triggered

; ** Post-Processing related trace ***
; CHECK:    Post Processing  preliminary analysis:  good
; CHECK:    Post Processing  collection: NOT good
; CHECK:    Post Processing  analysis: NOT good
; CHECK:    Post Processing  transformation: NOT good
; CHECK:    Post Processing  NOT Triggered

; ** MultiVersioning related trace ***
; CHECK:    MultiVersioning preliminary analysis:  good
; CHECK:    MultiVersioning collection:  good
; CHECK:    MultiVersioning analysis:  good
; CHECK:    MultiVersioning transformation:  good
; CHECK:    MultiVersioning Triggered

source_filename = "test.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.x264_weight_type = type { [8 x i16], [8 x i16], i32, i32, i32, i32* }

@dst_stride = dso_local global i32 1, align 4
@i_dst_stride = dso_local local_unnamed_addr global i32* @dst_stride, align 8
@mvx = dso_local local_unnamed_addr global i32 1, align 4
@mvy = dso_local local_unnamed_addr global i32 2, align 4
@dst = common dso_local local_unnamed_addr global [1000 x i8] zeroinitializer, align 16
@src4 = common dso_local local_unnamed_addr global [4 x i8] zeroinitializer, align 1
@weight_t = common dso_local local_unnamed_addr global %struct.x264_weight_type zeroinitializer, align 8
@src = common dso_local local_unnamed_addr global [1000 x i8] zeroinitializer, align 16
@hpel_ref0 = internal unnamed_addr constant [16 x i8] c"\00\01\01\01\00\01\01\01\02\03\03\03\00\01\01\01", align 16
@hpel_ref1 = internal unnamed_addr constant [16 x i8] c"\00\00\00\00\02\02\03\02\02\02\03\02\02\02\03\02", align 16

; Function Attrs: norecurse nounwind uwtable
define dso_local i32 @main() local_unnamed_addr #0 {
entry:
  %0 = load i32*, i32** @i_dst_stride, align 8, !tbaa !2
  %1 = load i32, i32* @mvx, align 4, !tbaa !6
  %2 = load i32, i32* @mvy, align 4, !tbaa !6
  tail call fastcc void @get_ref(i32* %0, i32 %1, i32 %2, i32 16, i32 16)
  %3 = load i32*, i32** @i_dst_stride, align 8, !tbaa !2
  %4 = load i32, i32* @mvx, align 4, !tbaa !6
  %5 = load i32, i32* @mvy, align 4, !tbaa !6
  tail call fastcc void @get_ref(i32* %3, i32 %4, i32 %5, i32 8, i32 8)
  %6 = load i32*, i32** @i_dst_stride, align 8, !tbaa !2
  %7 = load i32, i32* @mvx, align 4, !tbaa !6
  %8 = load i32, i32* @mvy, align 4, !tbaa !6
  tail call fastcc void @get_ref(i32* %6, i32 %7, i32 %8, i32 8, i32 16)
  %9 = load i32*, i32** @i_dst_stride, align 8, !tbaa !2
  %10 = load i32, i32* @mvx, align 4, !tbaa !6
  %11 = load i32, i32* @mvy, align 4, !tbaa !6
  tail call fastcc void @get_ref(i32* %9, i32 %10, i32 %11, i32 16, i32 8)
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %i.010 = phi i32 [ 0, %entry ], [ %inc, %for.body ]
  %12 = load i32*, i32** @i_dst_stride, align 8, !tbaa !2
  %13 = load i32, i32* @mvx, align 4, !tbaa !6
  %14 = load i32, i32* @mvy, align 4, !tbaa !6
  %sub = sub nuw nsw i32 100, %i.010
  tail call fastcc void @get_ref(i32* %12, i32 %13, i32 %14, i32 %i.010, i32 %sub)
  %inc = add nuw nsw i32 %i.010, 1
  %exitcond = icmp eq i32 %inc, 100
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret i32 0
}


; Function Attrs: noinline norecurse nounwind uwtable
define internal fastcc void @get_ref(i32* nocapture %i_dst_stride, i32 %mvx, i32 %mvy, i32 %i_width, i32 %i_height) unnamed_addr #1 {
entry:
  %and = and i32 %mvy, 3
  %shl = shl nuw nsw i32 %and, 2
  %and1 = and i32 %mvx, 3
  %add = or i32 %shl, %and1
  %shr = ashr i32 %mvy, 2
  %mul = shl nsw i32 %shr, 1
  %shr2 = ashr i32 %mvx, 2
  %add3 = add nsw i32 %mul, %shr2
  %0 = zext i32 %add to i64
  %arrayidx = getelementptr inbounds [16 x i8], [16 x i8]* @hpel_ref0, i64 0, i64 %0, !intel-tbaa !8
  %1 = load i8, i8* %arrayidx, align 1, !tbaa !8
  %idxprom4 = zext i8 %1 to i64
  %arrayidx5 = getelementptr inbounds i8*, i8** bitcast ([4 x i8]* @src4 to i8**), i64 %idxprom4
  %2 = load i8*, i8** %arrayidx5, align 8, !tbaa !10
  %idx.ext = sext i32 %add3 to i64
  %add.ptr = getelementptr inbounds i8, i8* %2, i64 %idx.ext, !intel-tbaa !12
  %cmp = icmp eq i32 %and, 3
  %conv = zext i1 %cmp to i64
  %mul7 = shl nuw nsw i64 %conv, 1
  %add.ptr9 = getelementptr inbounds i8, i8* %add.ptr, i64 %mul7, !intel-tbaa !12
  %and10 = and i32 %add, 5
  %tobool = icmp eq i32 %and10, 0
  br i1 %tobool, label %if.else, label %if.then

if.then:                                          ; preds = %entry
  %cmp3.i = icmp sgt i32 %i_height, 0
  br i1 %cmp3.i, label %for.cond1.preheader.lr.ph.i, label %pixel_avg.exit

for.cond1.preheader.lr.ph.i:                      ; preds = %if.then
  %3 = load i32, i32* %i_dst_stride, align 4, !tbaa !6
  %arrayidx12 = getelementptr inbounds [16 x i8], [16 x i8]* @hpel_ref1, i64 0, i64 %0, !intel-tbaa !8
  %4 = load i8, i8* %arrayidx12, align 1, !tbaa !8
  %idxprom13 = zext i8 %4 to i64
  %arrayidx14 = getelementptr inbounds i8*, i8** bitcast ([4 x i8]* @src4 to i8**), i64 %idxprom13
  %5 = load i8*, i8** %arrayidx14, align 8, !tbaa !10
  %add.ptr16 = getelementptr inbounds i8, i8* %5, i64 %idx.ext, !intel-tbaa !12
  %cmp18 = icmp eq i32 %and1, 3
  %idx.ext20 = zext i1 %cmp18 to i64
  %add.ptr21 = getelementptr inbounds i8, i8* %add.ptr16, i64 %idx.ext20, !intel-tbaa !12
  %cmp21.i = icmp sgt i32 %i_width, 0
  %idx.ext.i = sext i32 %3 to i64
  %wide.trip.count.i = sext i32 %i_width to i64
  %6 = add nsw i64 %mul7, %idx.ext
  %7 = add nsw i64 %mul7, %wide.trip.count.i
  %8 = add nsw i64 %7, %idx.ext
  %9 = add nsw i64 %idx.ext, %idx.ext20
  %10 = add nsw i64 %wide.trip.count.i, %idx.ext
  %11 = add nsw i64 %10, %idx.ext20
  %12 = add nsw i64 %wide.trip.count.i, -16
  %13 = lshr i64 %12, 4
  %14 = add nuw nsw i64 %13, 1
  %min.iters.check = icmp ult i32 %i_width, 16
  %n.vec = and i64 %wide.trip.count.i, -16
  %xtraiter209 = and i64 %14, 1
  %15 = icmp eq i64 %13, 0
  %unroll_iter = sub nuw nsw i64 %14, %xtraiter209
  %lcmp.mod210 = icmp eq i64 %xtraiter209, 0
  %cmp.n = icmp eq i64 %n.vec, %wide.trip.count.i
  %xtraiter211 = and i64 %wide.trip.count.i, 1
  %lcmp.mod212 = icmp eq i64 %xtraiter211, 0
  %16 = sub nsw i64 0, %wide.trip.count.i
  br label %for.cond1.preheader.i

for.cond1.preheader.i:                            ; preds = %for.cond.cleanup3.i, %for.cond1.preheader.lr.ph.i
  %indvar = phi i64 [ %indvar.next, %for.cond.cleanup3.i ], [ 0, %for.cond1.preheader.lr.ph.i ]
  %y.07.i = phi i32 [ %inc17.i, %for.cond.cleanup3.i ], [ 0, %for.cond1.preheader.lr.ph.i ]
  %dst.addr.06.i = phi i8* [ %add.ptr.i, %for.cond.cleanup3.i ], [ getelementptr inbounds ([1000 x i8], [1000 x i8]* @dst, i64 0, i64 0), %for.cond1.preheader.lr.ph.i ]
  %src1.addr.05.i = phi i8* [ %add.ptr13.i, %for.cond.cleanup3.i ], [ %add.ptr9, %for.cond1.preheader.lr.ph.i ]
  %src2.addr.04.i = phi i8* [ %add.ptr15.i, %for.cond.cleanup3.i ], [ %add.ptr21, %for.cond1.preheader.lr.ph.i ]
  %17 = mul i64 %indvar, %idx.ext.i
  %scevgep = getelementptr [1000 x i8], [1000 x i8]* @dst, i64 0, i64 %17
  %18 = add i64 %17, %wide.trip.count.i
  %scevgep78 = getelementptr [1000 x i8], [1000 x i8]* @dst, i64 0, i64 %18
  %19 = shl i64 %indvar, 1
  %20 = add i64 %6, %19
  %scevgep79 = getelementptr i8, i8* %2, i64 %20
  %21 = add i64 %8, %19
  %scevgep80 = getelementptr i8, i8* %2, i64 %21
  %22 = add i64 %9, %19
  %scevgep81 = getelementptr i8, i8* %5, i64 %22
  %23 = add i64 %11, %19
  %scevgep82 = getelementptr i8, i8* %5, i64 %23
  br i1 %cmp21.i, label %for.body4.i.preheader, label %for.cond.cleanup3.i

for.body4.i.preheader:                            ; preds = %for.cond1.preheader.i
  br i1 %min.iters.check, label %for.body4.i.preheader206, label %vector.memcheck

for.body4.i.preheader206:                         ; preds = %middle.block, %vector.memcheck, %for.body4.i.preheader
  %indvars.iv.i.ph = phi i64 [ 0, %vector.memcheck ], [ 0, %for.body4.i.preheader ], [ %n.vec, %middle.block ]
  %24 = xor i64 %indvars.iv.i.ph, -1
  br i1 %lcmp.mod212, label %for.body4.i.prol.loopexit, label %for.body4.i.prol

for.body4.i.prol:                                 ; preds = %for.body4.i.preheader206
  %arrayidx.i.prol = getelementptr inbounds i8, i8* %src1.addr.05.i, i64 %indvars.iv.i.ph
  %25 = load i8, i8* %arrayidx.i.prol, align 1, !tbaa !12
  %conv.i.prol = zext i8 %25 to i32
  %arrayidx6.i.prol = getelementptr inbounds i8, i8* %src2.addr.04.i, i64 %indvars.iv.i.ph
  %26 = load i8, i8* %arrayidx6.i.prol, align 1, !tbaa !12
  %conv7.i.prol = zext i8 %26 to i32
  %add.i.prol = add nuw nsw i32 %conv.i.prol, 1
  %add8.i.prol = add nuw nsw i32 %add.i.prol, %conv7.i.prol
  %27 = lshr i32 %add8.i.prol, 1
  %conv9.i.prol = trunc i32 %27 to i8
  %arrayidx11.i.prol = getelementptr inbounds i8, i8* %dst.addr.06.i, i64 %indvars.iv.i.ph
  store i8 %conv9.i.prol, i8* %arrayidx11.i.prol, align 1, !tbaa !12
  %indvars.iv.next.i.prol = or i64 %indvars.iv.i.ph, 1
  br label %for.body4.i.prol.loopexit

for.body4.i.prol.loopexit:                        ; preds = %for.body4.i.prol, %for.body4.i.preheader206
  %indvars.iv.i.unr.ph = phi i64 [ %indvars.iv.next.i.prol, %for.body4.i.prol ], [ %indvars.iv.i.ph, %for.body4.i.preheader206 ]
  %28 = icmp eq i64 %24, %16
  br i1 %28, label %for.cond.cleanup3.i, label %for.body4.i

vector.memcheck:                                  ; preds = %for.body4.i.preheader
  %bound0 = icmp ult i8* %scevgep, %scevgep80
  %bound1 = icmp ult i8* %scevgep79, %scevgep78
  %found.conflict = and i1 %bound0, %bound1
  %bound083 = icmp ult i8* %scevgep, %scevgep82
  %bound184 = icmp ult i8* %scevgep81, %scevgep78
  %found.conflict85 = and i1 %bound083, %bound184
  %conflict.rdx = or i1 %found.conflict, %found.conflict85
  br i1 %conflict.rdx, label %for.body4.i.preheader206, label %vector.ph

vector.ph:                                        ; preds = %vector.memcheck
  br i1 %15, label %middle.block.unr-lcssa, label %vector.body

vector.body:                                      ; preds = %vector.ph, %vector.body
  %index = phi i64 [ %index.next.1, %vector.body ], [ 0, %vector.ph ]
  %niter = phi i64 [ %niter.nsub.1, %vector.body ], [ %unroll_iter, %vector.ph ]
  %29 = getelementptr inbounds i8, i8* %src1.addr.05.i, i64 %index
  %30 = bitcast i8* %29 to <16 x i8>*
  %wide.load = load <16 x i8>, <16 x i8>* %30, align 1, !tbaa !12, !alias.scope !13
  %31 = zext <16 x i8> %wide.load to <16 x i32>
  %32 = getelementptr inbounds i8, i8* %src2.addr.04.i, i64 %index
  %33 = bitcast i8* %32 to <16 x i8>*
  %wide.load86 = load <16 x i8>, <16 x i8>* %33, align 1, !tbaa !12, !alias.scope !16
  %34 = zext <16 x i8> %wide.load86 to <16 x i32>
  %35 = add nuw nsw <16 x i32> %31, <i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1>
  %36 = add nuw nsw <16 x i32> %35, %34
  %37 = lshr <16 x i32> %36, <i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1>
  %38 = trunc <16 x i32> %37 to <16 x i8>
  %39 = getelementptr inbounds i8, i8* %dst.addr.06.i, i64 %index
  %40 = bitcast i8* %39 to <16 x i8>*
  store <16 x i8> %38, <16 x i8>* %40, align 1, !tbaa !12, !alias.scope !18, !noalias !20
  %index.next = or i64 %index, 16
  %41 = getelementptr inbounds i8, i8* %src1.addr.05.i, i64 %index.next
  %42 = bitcast i8* %41 to <16 x i8>*
  %wide.load.1 = load <16 x i8>, <16 x i8>* %42, align 1, !tbaa !12, !alias.scope !13
  %43 = zext <16 x i8> %wide.load.1 to <16 x i32>
  %44 = getelementptr inbounds i8, i8* %src2.addr.04.i, i64 %index.next
  %45 = bitcast i8* %44 to <16 x i8>*
  %wide.load86.1 = load <16 x i8>, <16 x i8>* %45, align 1, !tbaa !12, !alias.scope !16
  %46 = zext <16 x i8> %wide.load86.1 to <16 x i32>
  %47 = add nuw nsw <16 x i32> %43, <i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1>
  %48 = add nuw nsw <16 x i32> %47, %46
  %49 = lshr <16 x i32> %48, <i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1>
  %50 = trunc <16 x i32> %49 to <16 x i8>
  %51 = getelementptr inbounds i8, i8* %dst.addr.06.i, i64 %index.next
  %52 = bitcast i8* %51 to <16 x i8>*
  store <16 x i8> %50, <16 x i8>* %52, align 1, !tbaa !12, !alias.scope !18, !noalias !20
  %index.next.1 = add i64 %index, 32
  %niter.nsub.1 = add i64 %niter, -2
  %niter.ncmp.1 = icmp eq i64 %niter.nsub.1, 0
  br i1 %niter.ncmp.1, label %middle.block.unr-lcssa, label %vector.body, !llvm.loop !21

middle.block.unr-lcssa:                           ; preds = %vector.body, %vector.ph
  %index.unr = phi i64 [ 0, %vector.ph ], [ %index.next.1, %vector.body ]
  br i1 %lcmp.mod210, label %middle.block, label %vector.body.epil

vector.body.epil:                                 ; preds = %middle.block.unr-lcssa
  %53 = getelementptr inbounds i8, i8* %src1.addr.05.i, i64 %index.unr
  %54 = bitcast i8* %53 to <16 x i8>*
  %wide.load.epil = load <16 x i8>, <16 x i8>* %54, align 1, !tbaa !12, !alias.scope !13
  %55 = zext <16 x i8> %wide.load.epil to <16 x i32>
  %56 = getelementptr inbounds i8, i8* %src2.addr.04.i, i64 %index.unr
  %57 = bitcast i8* %56 to <16 x i8>*
  %wide.load86.epil = load <16 x i8>, <16 x i8>* %57, align 1, !tbaa !12, !alias.scope !16
  %58 = zext <16 x i8> %wide.load86.epil to <16 x i32>
  %59 = add nuw nsw <16 x i32> %55, <i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1>
  %60 = add nuw nsw <16 x i32> %59, %58
  %61 = lshr <16 x i32> %60, <i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1>
  %62 = trunc <16 x i32> %61 to <16 x i8>
  %63 = getelementptr inbounds i8, i8* %dst.addr.06.i, i64 %index.unr
  %64 = bitcast i8* %63 to <16 x i8>*
  store <16 x i8> %62, <16 x i8>* %64, align 1, !tbaa !12, !alias.scope !18, !noalias !20
  br label %middle.block

middle.block:                                     ; preds = %middle.block.unr-lcssa, %vector.body.epil
  br i1 %cmp.n, label %for.cond.cleanup3.i, label %for.body4.i.preheader206

for.cond.cleanup3.i:                              ; preds = %for.body4.i.prol.loopexit, %for.body4.i, %middle.block, %for.cond1.preheader.i
  %add.ptr.i = getelementptr inbounds i8, i8* %dst.addr.06.i, i64 %idx.ext.i, !intel-tbaa !12
  %add.ptr13.i = getelementptr inbounds i8, i8* %src1.addr.05.i, i64 2, !intel-tbaa !12
  %add.ptr15.i = getelementptr inbounds i8, i8* %src2.addr.04.i, i64 2, !intel-tbaa !12
  %inc17.i = add nuw nsw i32 %y.07.i, 1
  %exitcond8.i = icmp eq i32 %inc17.i, %i_height
  %indvar.next = add i64 %indvar, 1
  br i1 %exitcond8.i, label %pixel_avg.exit, label %for.cond1.preheader.i

for.body4.i:                                      ; preds = %for.body4.i.prol.loopexit, %for.body4.i
  %indvars.iv.i = phi i64 [ %indvars.iv.next.i.1, %for.body4.i ], [ %indvars.iv.i.unr.ph, %for.body4.i.prol.loopexit ]
  %arrayidx.i = getelementptr inbounds i8, i8* %src1.addr.05.i, i64 %indvars.iv.i
  %65 = load i8, i8* %arrayidx.i, align 1, !tbaa !12
  %conv.i = zext i8 %65 to i32
  %arrayidx6.i = getelementptr inbounds i8, i8* %src2.addr.04.i, i64 %indvars.iv.i
  %66 = load i8, i8* %arrayidx6.i, align 1, !tbaa !12
  %conv7.i = zext i8 %66 to i32
  %add.i = add nuw nsw i32 %conv.i, 1
  %add8.i = add nuw nsw i32 %add.i, %conv7.i
  %67 = lshr i32 %add8.i, 1
  %conv9.i = trunc i32 %67 to i8
  %arrayidx11.i = getelementptr inbounds i8, i8* %dst.addr.06.i, i64 %indvars.iv.i
  store i8 %conv9.i, i8* %arrayidx11.i, align 1, !tbaa !12
  %indvars.iv.next.i = add nuw nsw i64 %indvars.iv.i, 1
  %arrayidx.i.1 = getelementptr inbounds i8, i8* %src1.addr.05.i, i64 %indvars.iv.next.i
  %68 = load i8, i8* %arrayidx.i.1, align 1, !tbaa !12
  %conv.i.1 = zext i8 %68 to i32
  %arrayidx6.i.1 = getelementptr inbounds i8, i8* %src2.addr.04.i, i64 %indvars.iv.next.i
  %69 = load i8, i8* %arrayidx6.i.1, align 1, !tbaa !12
  %conv7.i.1 = zext i8 %69 to i32
  %add.i.1 = add nuw nsw i32 %conv.i.1, 1
  %add8.i.1 = add nuw nsw i32 %add.i.1, %conv7.i.1
  %70 = lshr i32 %add8.i.1, 1
  %conv9.i.1 = trunc i32 %70 to i8
  %arrayidx11.i.1 = getelementptr inbounds i8, i8* %dst.addr.06.i, i64 %indvars.iv.next.i
  store i8 %conv9.i.1, i8* %arrayidx11.i.1, align 1, !tbaa !12
  %indvars.iv.next.i.1 = add nsw i64 %indvars.iv.i, 2
  %exitcond.i.1 = icmp eq i64 %indvars.iv.next.i.1, %wide.trip.count.i
  br i1 %exitcond.i.1, label %for.cond.cleanup3.i, label %for.body4.i, !llvm.loop !23

pixel_avg.exit:                                   ; preds = %for.cond.cleanup3.i, %if.then
  %71 = load i32*, i32** getelementptr inbounds (%struct.x264_weight_type, %struct.x264_weight_type* @weight_t, i64 0, i32 5), align 8, !tbaa !24
  %tobool22 = icmp eq i32* %71, null
  br i1 %tobool22, label %cleanup, label %if.then23

if.then23:                                        ; preds = %pixel_avg.exit
  %72 = load i32, i32* %i_dst_stride, align 4, !tbaa !6
  %73 = load i32, i32* getelementptr inbounds (%struct.x264_weight_type, %struct.x264_weight_type* @weight_t, i64 0, i32 2), align 8, !tbaa !28
  %cmp.i11 = icmp sgt i32 %73, 0
  br i1 %cmp.i11, label %for.cond.preheader.i18, label %for.cond17.preheader.i13

for.cond17.preheader.i13:                         ; preds = %if.then23
  br i1 %cmp3.i, label %for.cond23.preheader.lr.ph.i17, label %cleanup

for.cond23.preheader.lr.ph.i17:                   ; preds = %for.cond17.preheader.i13
  %cmp249.i14 = icmp sgt i32 %i_width, 0
  %idx.ext43.i15 = sext i32 %72 to i64
  %wide.trip.count19.i16 = sext i32 %i_width to i64
  %min.iters.check90 = icmp ult i32 %i_width, 4
  %n.vec103 = and i64 %wide.trip.count19.i16, -4
  %cmp.n107 = icmp eq i64 %n.vec103, %wide.trip.count19.i16
  %xtraiter207 = and i64 %wide.trip.count19.i16, 1
  %lcmp.mod208 = icmp eq i64 %xtraiter207, 0
  %74 = sub nsw i64 0, %wide.trip.count19.i16
  br label %for.cond23.preheader.i49

for.cond.preheader.i18:                           ; preds = %if.then23
  br i1 %cmp3.i, label %for.cond2.preheader.lr.ph.i22, label %cleanup

for.cond2.preheader.lr.ph.i22:                    ; preds = %for.cond.preheader.i18
  %cmp33.i19 = icmp sgt i32 %i_width, 0
  %idx.ext.i20 = sext i32 %72 to i64
  %wide.trip.count.i21 = sext i32 %i_width to i64
  %min.iters.check115 = icmp ult i32 %i_width, 4
  %n.vec128 = and i64 %wide.trip.count.i21, -4
  %cmp.n132 = icmp eq i64 %n.vec128, %wide.trip.count.i21
  br label %for.cond2.preheader.i26

for.cond2.preheader.i26:                          ; preds = %for.cond.cleanup4.i31, %for.cond2.preheader.lr.ph.i22
  %indvar117 = phi i64 [ %indvar.next118, %for.cond.cleanup4.i31 ], [ 0, %for.cond2.preheader.lr.ph.i22 ]
  %y.08.i23 = phi i32 [ %inc12.i27, %for.cond.cleanup4.i31 ], [ 0, %for.cond2.preheader.lr.ph.i22 ]
  %dst.addr.07.i24 = phi i8* [ %add.ptr.i28, %for.cond.cleanup4.i31 ], [ getelementptr inbounds ([1000 x i8], [1000 x i8]* @dst, i64 0, i64 0), %for.cond2.preheader.lr.ph.i22 ]
  %75 = mul i64 %indvar117, %idx.ext.i20
  %scevgep119 = getelementptr [1000 x i8], [1000 x i8]* @dst, i64 0, i64 %75
  %76 = add i64 %75, %wide.trip.count.i21
  %scevgep120 = getelementptr [1000 x i8], [1000 x i8]* @dst, i64 0, i64 %76
  br i1 %cmp33.i19, label %for.body5.i45.preheader, label %for.cond.cleanup4.i31

for.body5.i45.preheader:                          ; preds = %for.cond2.preheader.i26
  br i1 %min.iters.check115, label %for.body5.i45.preheader202, label %vector.memcheck125

for.body5.i45.preheader202:                       ; preds = %middle.block113, %vector.memcheck125, %for.body5.i45.preheader
  %indvars.iv.i32.ph = phi i64 [ 0, %vector.memcheck125 ], [ 0, %for.body5.i45.preheader ], [ %n.vec128, %middle.block113 ]
  br label %for.body5.i45

vector.memcheck125:                               ; preds = %for.body5.i45.preheader
  %bound0121 = icmp ult i8* %scevgep119, getelementptr (i8, i8* bitcast (i32* getelementptr inbounds (%struct.x264_weight_type, %struct.x264_weight_type* @weight_t, i64 0, i32 4) to i8*), i64 1)
  %bound1122 = icmp ugt i8* %scevgep120, bitcast (i32* getelementptr inbounds (%struct.x264_weight_type, %struct.x264_weight_type* @weight_t, i64 0, i32 4) to i8*)
  %found.conflict123 = and i1 %bound0121, %bound1122
  br i1 %found.conflict123, label %for.body5.i45.preheader202, label %vector.ph126

vector.ph126:                                     ; preds = %vector.memcheck125
  %77 = load i32, i32* getelementptr inbounds (%struct.x264_weight_type, %struct.x264_weight_type* @weight_t, i64 0, i32 3), align 4, !tbaa !29, !alias.scope !30
  %78 = insertelement <4 x i32> undef, i32 %77, i32 0
  %79 = shufflevector <4 x i32> %78, <4 x i32> undef, <4 x i32> zeroinitializer
  %80 = load i32, i32* getelementptr inbounds (%struct.x264_weight_type, %struct.x264_weight_type* @weight_t, i64 0, i32 2), align 8, !tbaa !28, !alias.scope !30
  %81 = insertelement <4 x i32> undef, i32 %80, i32 0
  %82 = shufflevector <4 x i32> %81, <4 x i32> undef, <4 x i32> zeroinitializer
  %83 = add nsw <4 x i32> %82, <i32 -1, i32 -1, i32 -1, i32 -1>
  %84 = shl <4 x i32> <i32 1, i32 1, i32 1, i32 1>, %83
  %85 = load i32, i32* getelementptr inbounds (%struct.x264_weight_type, %struct.x264_weight_type* @weight_t, i64 0, i32 4), align 8, !tbaa !33, !alias.scope !30
  %86 = insertelement <4 x i32> undef, i32 %85, i32 0
  %87 = shufflevector <4 x i32> %86, <4 x i32> undef, <4 x i32> zeroinitializer
  br label %vector.body112

vector.body112:                                   ; preds = %vector.body112, %vector.ph126
  %index129 = phi i64 [ 0, %vector.ph126 ], [ %index.next130, %vector.body112 ]
  %88 = getelementptr inbounds i8, i8* %dst.addr.07.i24, i64 %index129
  %89 = bitcast i8* %88 to <4 x i8>*
  %wide.load136 = load <4 x i8>, <4 x i8>* %89, align 1, !tbaa !12, !alias.scope !34, !noalias !30
  %90 = zext <4 x i8> %wide.load136 to <4 x i32>
  %91 = mul nsw <4 x i32> %79, %90
  %92 = add nsw <4 x i32> %84, %91
  %93 = ashr <4 x i32> %92, %82
  %94 = add nsw <4 x i32> %93, %87
  %95 = icmp sgt <4 x i32> %94, zeroinitializer
  %96 = select <4 x i1> %95, <4 x i32> %94, <4 x i32> zeroinitializer
  %97 = icmp slt <4 x i32> %96, <i32 255, i32 255, i32 255, i32 255>
  %98 = select <4 x i1> %97, <4 x i32> %96, <4 x i32> <i32 255, i32 255, i32 255, i32 255>
  %99 = trunc <4 x i32> %98 to <4 x i8>
  %100 = bitcast i8* %88 to <4 x i8>*
  store <4 x i8> %99, <4 x i8>* %100, align 1, !tbaa !12, !alias.scope !34, !noalias !30
  %index.next130 = add i64 %index129, 4
  %101 = icmp eq i64 %index.next130, %n.vec128
  br i1 %101, label %middle.block113, label %vector.body112, !llvm.loop !36

middle.block113:                                  ; preds = %vector.body112
  br i1 %cmp.n132, label %for.cond.cleanup4.i31, label %for.body5.i45.preheader202

for.cond.cleanup4.i31:                            ; preds = %for.body5.i45, %middle.block113, %for.cond2.preheader.i26
  %inc12.i27 = add nuw nsw i32 %y.08.i23, 1
  %add.ptr.i28 = getelementptr inbounds i8, i8* %dst.addr.07.i24, i64 %idx.ext.i20, !intel-tbaa !12
  %exitcond16.i30 = icmp eq i32 %inc12.i27, %i_height
  %indvar.next118 = add i64 %indvar117, 1
  br i1 %exitcond16.i30, label %cleanup, label %for.cond2.preheader.i26

for.body5.i45:                                    ; preds = %for.body5.i45.preheader202, %for.body5.i45
  %indvars.iv.i32 = phi i64 [ %indvars.iv.next.i43, %for.body5.i45 ], [ %indvars.iv.i32.ph, %for.body5.i45.preheader202 ]
  %arrayidx.i33 = getelementptr inbounds i8, i8* %dst.addr.07.i24, i64 %indvars.iv.i32
  %102 = load i8, i8* %arrayidx.i33, align 1, !tbaa !12
  %conv.i34 = zext i8 %102 to i32
  %103 = load i32, i32* getelementptr inbounds (%struct.x264_weight_type, %struct.x264_weight_type* @weight_t, i64 0, i32 3), align 4, !tbaa !29
  %mul.i35 = mul nsw i32 %103, %conv.i34
  %104 = load i32, i32* getelementptr inbounds (%struct.x264_weight_type, %struct.x264_weight_type* @weight_t, i64 0, i32 2), align 8, !tbaa !28
  %sub.i36 = add nsw i32 %104, -1
  %shl.i37 = shl i32 1, %sub.i36
  %add.i38 = add nsw i32 %shl.i37, %mul.i35
  %shr.i39 = ashr i32 %add.i38, %104
  %105 = load i32, i32* getelementptr inbounds (%struct.x264_weight_type, %struct.x264_weight_type* @weight_t, i64 0, i32 4), align 8, !tbaa !33
  %add8.i40 = add nsw i32 %shr.i39, %105
  %106 = icmp sgt i32 %add8.i40, 0
  %107 = select i1 %106, i32 %add8.i40, i32 0
  %108 = icmp slt i32 %107, 255
  %109 = select i1 %108, i32 %107, i32 255
  %conv.i.i41 = trunc i32 %109 to i8
  store i8 %conv.i.i41, i8* %arrayidx.i33, align 1, !tbaa !12
  %indvars.iv.next.i43 = add nuw nsw i64 %indvars.iv.i32, 1
  %exitcond.i44 = icmp eq i64 %indvars.iv.next.i43, %wide.trip.count.i21
  br i1 %exitcond.i44, label %for.cond.cleanup4.i31, label %for.body5.i45, !llvm.loop !37

for.cond23.preheader.i49:                         ; preds = %for.cond.cleanup26.i54, %for.cond23.preheader.lr.ph.i17
  %indvar92 = phi i64 [ %indvar.next93, %for.cond.cleanup26.i54 ], [ 0, %for.cond23.preheader.lr.ph.i17 ]
  %y16.014.i46 = phi i32 [ %inc42.i50, %for.cond.cleanup26.i54 ], [ 0, %for.cond23.preheader.lr.ph.i17 ]
  %dst.addr.113.i47 = phi i8* [ %add.ptr44.i51, %for.cond.cleanup26.i54 ], [ getelementptr inbounds ([1000 x i8], [1000 x i8]* @dst, i64 0, i64 0), %for.cond23.preheader.lr.ph.i17 ]
  %110 = mul i64 %indvar92, %idx.ext43.i15
  %scevgep94 = getelementptr [1000 x i8], [1000 x i8]* @dst, i64 0, i64 %110
  %111 = add i64 %110, %wide.trip.count19.i16
  %scevgep95 = getelementptr [1000 x i8], [1000 x i8]* @dst, i64 0, i64 %111
  br i1 %cmp249.i14, label %for.body27.i64.preheader, label %for.cond.cleanup26.i54

for.body27.i64.preheader:                         ; preds = %for.cond23.preheader.i49
  br i1 %min.iters.check90, label %for.body27.i64.preheader204, label %vector.memcheck100

for.body27.i64.preheader204:                      ; preds = %middle.block88, %vector.memcheck100, %for.body27.i64.preheader
  %indvars.iv17.i55.ph = phi i64 [ 0, %vector.memcheck100 ], [ 0, %for.body27.i64.preheader ], [ %n.vec103, %middle.block88 ]
  %112 = xor i64 %indvars.iv17.i55.ph, -1
  br i1 %lcmp.mod208, label %for.body27.i64.prol.loopexit, label %for.body27.i64.prol

for.body27.i64.prol:                              ; preds = %for.body27.i64.preheader204
  %arrayidx29.i56.prol = getelementptr inbounds i8, i8* %dst.addr.113.i47, i64 %indvars.iv17.i55.ph
  %113 = load i8, i8* %arrayidx29.i56.prol, align 1, !tbaa !12
  %conv30.i57.prol = zext i8 %113 to i32
  %114 = load i32, i32* getelementptr inbounds (%struct.x264_weight_type, %struct.x264_weight_type* @weight_t, i64 0, i32 3), align 4, !tbaa !29
  %mul32.i58.prol = mul nsw i32 %114, %conv30.i57.prol
  %115 = load i32, i32* getelementptr inbounds (%struct.x264_weight_type, %struct.x264_weight_type* @weight_t, i64 0, i32 4), align 8, !tbaa !33
  %add34.i59.prol = add nsw i32 %mul32.i58.prol, %115
  %116 = icmp sgt i32 %add34.i59.prol, 0
  %117 = select i1 %116, i32 %add34.i59.prol, i32 0
  %118 = icmp slt i32 %117, 255
  %119 = select i1 %118, i32 %117, i32 255
  %conv.i1.i60.prol = trunc i32 %119 to i8
  store i8 %conv.i1.i60.prol, i8* %arrayidx29.i56.prol, align 1, !tbaa !12
  %indvars.iv.next18.i62.prol = or i64 %indvars.iv17.i55.ph, 1
  br label %for.body27.i64.prol.loopexit

for.body27.i64.prol.loopexit:                     ; preds = %for.body27.i64.prol, %for.body27.i64.preheader204
  %indvars.iv17.i55.unr.ph = phi i64 [ %indvars.iv.next18.i62.prol, %for.body27.i64.prol ], [ %indvars.iv17.i55.ph, %for.body27.i64.preheader204 ]
  %120 = icmp eq i64 %112, %74
  br i1 %120, label %for.cond.cleanup26.i54, label %for.body27.i64

vector.memcheck100:                               ; preds = %for.body27.i64.preheader
  %bound096 = icmp ult i8* %scevgep94, getelementptr (i8, i8* bitcast (i32* getelementptr inbounds (%struct.x264_weight_type, %struct.x264_weight_type* @weight_t, i64 0, i32 4) to i8*), i64 1)
  %bound197 = icmp ugt i8* %scevgep95, bitcast (i32* getelementptr inbounds (%struct.x264_weight_type, %struct.x264_weight_type* @weight_t, i64 0, i32 4) to i8*)
  %found.conflict98 = and i1 %bound096, %bound197
  br i1 %found.conflict98, label %for.body27.i64.preheader204, label %vector.ph101

vector.ph101:                                     ; preds = %vector.memcheck100
  %121 = load i32, i32* getelementptr inbounds (%struct.x264_weight_type, %struct.x264_weight_type* @weight_t, i64 0, i32 3), align 4, !tbaa !29, !alias.scope !38
  %122 = insertelement <4 x i32> undef, i32 %121, i32 0
  %123 = shufflevector <4 x i32> %122, <4 x i32> undef, <4 x i32> zeroinitializer
  %124 = load i32, i32* getelementptr inbounds (%struct.x264_weight_type, %struct.x264_weight_type* @weight_t, i64 0, i32 4), align 8, !tbaa !33, !alias.scope !38
  %125 = insertelement <4 x i32> undef, i32 %124, i32 0
  %126 = shufflevector <4 x i32> %125, <4 x i32> undef, <4 x i32> zeroinitializer
  br label %vector.body87

vector.body87:                                    ; preds = %vector.body87, %vector.ph101
  %index104 = phi i64 [ 0, %vector.ph101 ], [ %index.next105, %vector.body87 ]
  %127 = getelementptr inbounds i8, i8* %dst.addr.113.i47, i64 %index104
  %128 = bitcast i8* %127 to <4 x i8>*
  %wide.load111 = load <4 x i8>, <4 x i8>* %128, align 1, !tbaa !12, !alias.scope !41, !noalias !38
  %129 = zext <4 x i8> %wide.load111 to <4 x i32>
  %130 = mul nsw <4 x i32> %123, %129
  %131 = add nsw <4 x i32> %130, %126
  %132 = icmp sgt <4 x i32> %131, zeroinitializer
  %133 = select <4 x i1> %132, <4 x i32> %131, <4 x i32> zeroinitializer
  %134 = icmp slt <4 x i32> %133, <i32 255, i32 255, i32 255, i32 255>
  %135 = select <4 x i1> %134, <4 x i32> %133, <4 x i32> <i32 255, i32 255, i32 255, i32 255>
  %136 = trunc <4 x i32> %135 to <4 x i8>
  %137 = bitcast i8* %127 to <4 x i8>*
  store <4 x i8> %136, <4 x i8>* %137, align 1, !tbaa !12, !alias.scope !41, !noalias !38
  %index.next105 = add i64 %index104, 4
  %138 = icmp eq i64 %index.next105, %n.vec103
  br i1 %138, label %middle.block88, label %vector.body87, !llvm.loop !43

middle.block88:                                   ; preds = %vector.body87
  br i1 %cmp.n107, label %for.cond.cleanup26.i54, label %for.body27.i64.preheader204

for.cond.cleanup26.i54:                           ; preds = %for.body27.i64.prol.loopexit, %for.body27.i64, %middle.block88, %for.cond23.preheader.i49
  %inc42.i50 = add nuw nsw i32 %y16.014.i46, 1
  %add.ptr44.i51 = getelementptr inbounds i8, i8* %dst.addr.113.i47, i64 %idx.ext43.i15, !intel-tbaa !12
  %exitcond21.i53 = icmp eq i32 %inc42.i50, %i_height
  %indvar.next93 = add i64 %indvar92, 1
  br i1 %exitcond21.i53, label %cleanup, label %for.cond23.preheader.i49

for.body27.i64:                                   ; preds = %for.body27.i64.prol.loopexit, %for.body27.i64
  %indvars.iv17.i55 = phi i64 [ %indvars.iv.next18.i62.1, %for.body27.i64 ], [ %indvars.iv17.i55.unr.ph, %for.body27.i64.prol.loopexit ]
  %arrayidx29.i56 = getelementptr inbounds i8, i8* %dst.addr.113.i47, i64 %indvars.iv17.i55
  %139 = load i8, i8* %arrayidx29.i56, align 1, !tbaa !12
  %conv30.i57 = zext i8 %139 to i32
  %140 = load i32, i32* getelementptr inbounds (%struct.x264_weight_type, %struct.x264_weight_type* @weight_t, i64 0, i32 3), align 4, !tbaa !29
  %mul32.i58 = mul nsw i32 %140, %conv30.i57
  %141 = load i32, i32* getelementptr inbounds (%struct.x264_weight_type, %struct.x264_weight_type* @weight_t, i64 0, i32 4), align 8, !tbaa !33
  %add34.i59 = add nsw i32 %mul32.i58, %141
  %142 = icmp sgt i32 %add34.i59, 0
  %143 = select i1 %142, i32 %add34.i59, i32 0
  %144 = icmp slt i32 %143, 255
  %145 = select i1 %144, i32 %143, i32 255
  %conv.i1.i60 = trunc i32 %145 to i8
  store i8 %conv.i1.i60, i8* %arrayidx29.i56, align 1, !tbaa !12
  %indvars.iv.next18.i62 = add nuw nsw i64 %indvars.iv17.i55, 1
  %arrayidx29.i56.1 = getelementptr inbounds i8, i8* %dst.addr.113.i47, i64 %indvars.iv.next18.i62
  %146 = load i8, i8* %arrayidx29.i56.1, align 1, !tbaa !12
  %conv30.i57.1 = zext i8 %146 to i32
  %147 = load i32, i32* getelementptr inbounds (%struct.x264_weight_type, %struct.x264_weight_type* @weight_t, i64 0, i32 3), align 4, !tbaa !29
  %mul32.i58.1 = mul nsw i32 %147, %conv30.i57.1
  %148 = load i32, i32* getelementptr inbounds (%struct.x264_weight_type, %struct.x264_weight_type* @weight_t, i64 0, i32 4), align 8, !tbaa !33
  %add34.i59.1 = add nsw i32 %mul32.i58.1, %148
  %149 = icmp sgt i32 %add34.i59.1, 0
  %150 = select i1 %149, i32 %add34.i59.1, i32 0
  %151 = icmp slt i32 %150, 255
  %152 = select i1 %151, i32 %150, i32 255
  %conv.i1.i60.1 = trunc i32 %152 to i8
  store i8 %conv.i1.i60.1, i8* %arrayidx29.i56.1, align 1, !tbaa !12
  %indvars.iv.next18.i62.1 = add nsw i64 %indvars.iv17.i55, 2
  %exitcond20.i63.1 = icmp eq i64 %indvars.iv.next18.i62.1, %wide.trip.count19.i16
  br i1 %exitcond20.i63.1, label %for.cond.cleanup26.i54, label %for.body27.i64, !llvm.loop !44

if.else:                                          ; preds = %entry
  %153 = load i32*, i32** getelementptr inbounds (%struct.x264_weight_type, %struct.x264_weight_type* @weight_t, i64 0, i32 5), align 8, !tbaa !24
  %tobool25 = icmp eq i32* %153, null
  br i1 %tobool25, label %if.else27, label %if.then26

if.then26:                                        ; preds = %if.else
  %154 = load i32, i32* %i_dst_stride, align 4, !tbaa !6
  %155 = load i32, i32* getelementptr inbounds (%struct.x264_weight_type, %struct.x264_weight_type* @weight_t, i64 0, i32 2), align 8, !tbaa !28
  %cmp.i = icmp sgt i32 %155, 0
  %cmp15.i = icmp sgt i32 %i_height, 0
  br i1 %cmp.i, label %for.cond.preheader.i, label %for.cond17.preheader.i

for.cond17.preheader.i:                           ; preds = %if.then26
  br i1 %cmp15.i, label %for.cond23.preheader.lr.ph.i, label %cleanup

for.cond23.preheader.lr.ph.i:                     ; preds = %for.cond17.preheader.i
  %cmp249.i = icmp sgt i32 %i_width, 0
  %idx.ext43.i = sext i32 %154 to i64
  %wide.trip.count19.i = sext i32 %i_width to i64
  %156 = add nsw i64 %mul7, %idx.ext
  %157 = add nsw i64 %mul7, %wide.trip.count19.i
  %158 = add nsw i64 %157, %idx.ext
  %min.iters.check140 = icmp ult i32 %i_width, 4
  %n.vec159 = and i64 %wide.trip.count19.i, -4
  %cmp.n163 = icmp eq i64 %n.vec159, %wide.trip.count19.i
  %xtraiter = and i64 %wide.trip.count19.i, 1
  %lcmp.mod = icmp eq i64 %xtraiter, 0
  %159 = sub nsw i64 0, %wide.trip.count19.i
  br label %for.cond23.preheader.i

for.cond.preheader.i:                             ; preds = %if.then26
  br i1 %cmp15.i, label %for.cond2.preheader.lr.ph.i, label %cleanup

for.cond2.preheader.lr.ph.i:                      ; preds = %for.cond.preheader.i
  %cmp33.i = icmp sgt i32 %i_width, 0
  %idx.ext.i1 = sext i32 %154 to i64
  %wide.trip.count.i2 = sext i32 %i_width to i64
  %160 = add nsw i64 %mul7, %idx.ext
  %161 = add nsw i64 %mul7, %wide.trip.count.i2
  %162 = add nsw i64 %161, %idx.ext
  %min.iters.check171 = icmp ult i32 %i_width, 4
  %n.vec190 = and i64 %wide.trip.count.i2, -4
  %cmp.n194 = icmp eq i64 %n.vec190, %wide.trip.count.i2
  br label %for.cond2.preheader.i

for.cond2.preheader.i:                            ; preds = %for.cond.cleanup4.i, %for.cond2.preheader.lr.ph.i
  %indvar173 = phi i64 [ %indvar.next174, %for.cond.cleanup4.i ], [ 0, %for.cond2.preheader.lr.ph.i ]
  %y.08.i = phi i32 [ %inc12.i, %for.cond.cleanup4.i ], [ 0, %for.cond2.preheader.lr.ph.i ]
  %dst.addr.07.i = phi i8* [ %add.ptr.i3, %for.cond.cleanup4.i ], [ getelementptr inbounds ([1000 x i8], [1000 x i8]* @dst, i64 0, i64 0), %for.cond2.preheader.lr.ph.i ]
  %src.addr.06.i = phi i8* [ %add.ptr14.i, %for.cond.cleanup4.i ], [ %add.ptr9, %for.cond2.preheader.lr.ph.i ]
  %163 = mul i64 %indvar173, %idx.ext.i1
  %scevgep175 = getelementptr [1000 x i8], [1000 x i8]* @dst, i64 0, i64 %163
  %164 = add i64 %163, %wide.trip.count.i2
  %scevgep176 = getelementptr [1000 x i8], [1000 x i8]* @dst, i64 0, i64 %164
  %165 = shl i64 %indvar173, 1
  %166 = add i64 %160, %165
  %scevgep177 = getelementptr i8, i8* %2, i64 %166
  %167 = add i64 %162, %165
  %scevgep178 = getelementptr i8, i8* %2, i64 %167
  br i1 %cmp33.i, label %for.body5.i.preheader, label %for.cond.cleanup4.i

for.body5.i.preheader:                            ; preds = %for.cond2.preheader.i
  br i1 %min.iters.check171, label %for.body5.i.preheader199, label %vector.memcheck187

for.body5.i.preheader199:                         ; preds = %middle.block169, %vector.memcheck187, %for.body5.i.preheader
  %indvars.iv.i4.ph = phi i64 [ 0, %vector.memcheck187 ], [ 0, %for.body5.i.preheader ], [ %n.vec190, %middle.block169 ]
  br label %for.body5.i

vector.memcheck187:                               ; preds = %for.body5.i.preheader
  %bound0179 = icmp ult i8* %scevgep175, %scevgep178
  %bound1180 = icmp ult i8* %scevgep177, %scevgep176
  %found.conflict181 = and i1 %bound0179, %bound1180
  %bound0182 = icmp ult i8* %scevgep175, getelementptr (i8, i8* bitcast (i32* getelementptr inbounds (%struct.x264_weight_type, %struct.x264_weight_type* @weight_t, i64 0, i32 4) to i8*), i64 1)
  %bound1183 = icmp ugt i8* %scevgep176, bitcast (i32* getelementptr inbounds (%struct.x264_weight_type, %struct.x264_weight_type* @weight_t, i64 0, i32 4) to i8*)
  %found.conflict184 = and i1 %bound0182, %bound1183
  %conflict.rdx185 = or i1 %found.conflict181, %found.conflict184
  br i1 %conflict.rdx185, label %for.body5.i.preheader199, label %vector.ph188

vector.ph188:                                     ; preds = %vector.memcheck187
  %168 = load i32, i32* getelementptr inbounds (%struct.x264_weight_type, %struct.x264_weight_type* @weight_t, i64 0, i32 3), align 4, !tbaa !29, !alias.scope !45
  %169 = insertelement <4 x i32> undef, i32 %168, i32 0
  %170 = shufflevector <4 x i32> %169, <4 x i32> undef, <4 x i32> zeroinitializer
  %171 = load i32, i32* getelementptr inbounds (%struct.x264_weight_type, %struct.x264_weight_type* @weight_t, i64 0, i32 2), align 8, !tbaa !28, !alias.scope !45
  %172 = insertelement <4 x i32> undef, i32 %171, i32 0
  %173 = shufflevector <4 x i32> %172, <4 x i32> undef, <4 x i32> zeroinitializer
  %174 = add nsw <4 x i32> %173, <i32 -1, i32 -1, i32 -1, i32 -1>
  %175 = shl <4 x i32> <i32 1, i32 1, i32 1, i32 1>, %174
  %176 = load i32, i32* getelementptr inbounds (%struct.x264_weight_type, %struct.x264_weight_type* @weight_t, i64 0, i32 4), align 8, !tbaa !33, !alias.scope !45
  %177 = insertelement <4 x i32> undef, i32 %176, i32 0
  %178 = shufflevector <4 x i32> %177, <4 x i32> undef, <4 x i32> zeroinitializer
  br label %vector.body168

vector.body168:                                   ; preds = %vector.body168, %vector.ph188
  %index191 = phi i64 [ 0, %vector.ph188 ], [ %index.next192, %vector.body168 ]
  %179 = getelementptr inbounds i8, i8* %src.addr.06.i, i64 %index191
  %180 = bitcast i8* %179 to <4 x i8>*
  %wide.load198 = load <4 x i8>, <4 x i8>* %180, align 1, !tbaa !12, !alias.scope !48
  %181 = zext <4 x i8> %wide.load198 to <4 x i32>
  %182 = mul nsw <4 x i32> %170, %181
  %183 = add nsw <4 x i32> %175, %182
  %184 = ashr <4 x i32> %183, %173
  %185 = add nsw <4 x i32> %184, %178
  %186 = icmp sgt <4 x i32> %185, zeroinitializer
  %187 = select <4 x i1> %186, <4 x i32> %185, <4 x i32> zeroinitializer
  %188 = icmp slt <4 x i32> %187, <i32 255, i32 255, i32 255, i32 255>
  %189 = select <4 x i1> %188, <4 x i32> %187, <4 x i32> <i32 255, i32 255, i32 255, i32 255>
  %190 = trunc <4 x i32> %189 to <4 x i8>
  %191 = getelementptr inbounds i8, i8* %dst.addr.07.i, i64 %index191
  %192 = bitcast i8* %191 to <4 x i8>*
  store <4 x i8> %190, <4 x i8>* %192, align 1, !tbaa !12, !alias.scope !50, !noalias !52
  %index.next192 = add i64 %index191, 4
  %193 = icmp eq i64 %index.next192, %n.vec190
  br i1 %193, label %middle.block169, label %vector.body168, !llvm.loop !53

middle.block169:                                  ; preds = %vector.body168
  br i1 %cmp.n194, label %for.cond.cleanup4.i, label %for.body5.i.preheader199

for.cond.cleanup4.i:                              ; preds = %for.body5.i, %middle.block169, %for.cond2.preheader.i
  %inc12.i = add nuw nsw i32 %y.08.i, 1
  %add.ptr.i3 = getelementptr inbounds i8, i8* %dst.addr.07.i, i64 %idx.ext.i1, !intel-tbaa !12
  %add.ptr14.i = getelementptr inbounds i8, i8* %src.addr.06.i, i64 2, !intel-tbaa !12
  %exitcond16.i = icmp eq i32 %inc12.i, %i_height
  %indvar.next174 = add i64 %indvar173, 1
  br i1 %exitcond16.i, label %cleanup, label %for.cond2.preheader.i

for.body5.i:                                      ; preds = %for.body5.i.preheader199, %for.body5.i
  %indvars.iv.i4 = phi i64 [ %indvars.iv.next.i9, %for.body5.i ], [ %indvars.iv.i4.ph, %for.body5.i.preheader199 ]
  %arrayidx.i5 = getelementptr inbounds i8, i8* %src.addr.06.i, i64 %indvars.iv.i4
  %194 = load i8, i8* %arrayidx.i5, align 1, !tbaa !12
  %conv.i6 = zext i8 %194 to i32
  %195 = load i32, i32* getelementptr inbounds (%struct.x264_weight_type, %struct.x264_weight_type* @weight_t, i64 0, i32 3), align 4, !tbaa !29
  %mul.i = mul nsw i32 %195, %conv.i6
  %196 = load i32, i32* getelementptr inbounds (%struct.x264_weight_type, %struct.x264_weight_type* @weight_t, i64 0, i32 2), align 8, !tbaa !28
  %sub.i = add nsw i32 %196, -1
  %shl.i = shl i32 1, %sub.i
  %add.i7 = add nsw i32 %shl.i, %mul.i
  %shr.i = ashr i32 %add.i7, %196
  %197 = load i32, i32* getelementptr inbounds (%struct.x264_weight_type, %struct.x264_weight_type* @weight_t, i64 0, i32 4), align 8, !tbaa !33
  %add8.i8 = add nsw i32 %shr.i, %197
  %198 = icmp sgt i32 %add8.i8, 0
  %199 = select i1 %198, i32 %add8.i8, i32 0
  %200 = icmp slt i32 %199, 255
  %201 = select i1 %200, i32 %199, i32 255
  %conv.i.i = trunc i32 %201 to i8
  %arrayidx10.i = getelementptr inbounds i8, i8* %dst.addr.07.i, i64 %indvars.iv.i4
  store i8 %conv.i.i, i8* %arrayidx10.i, align 1, !tbaa !12
  %indvars.iv.next.i9 = add nuw nsw i64 %indvars.iv.i4, 1
  %exitcond.i10 = icmp eq i64 %indvars.iv.next.i9, %wide.trip.count.i2
  br i1 %exitcond.i10, label %for.cond.cleanup4.i, label %for.body5.i, !llvm.loop !54

for.cond23.preheader.i:                           ; preds = %for.cond.cleanup26.i, %for.cond23.preheader.lr.ph.i
  %indvar142 = phi i64 [ %indvar.next143, %for.cond.cleanup26.i ], [ 0, %for.cond23.preheader.lr.ph.i ]
  %y16.014.i = phi i32 [ %inc42.i, %for.cond.cleanup26.i ], [ 0, %for.cond23.preheader.lr.ph.i ]
  %dst.addr.113.i = phi i8* [ %add.ptr44.i, %for.cond.cleanup26.i ], [ getelementptr inbounds ([1000 x i8], [1000 x i8]* @dst, i64 0, i64 0), %for.cond23.preheader.lr.ph.i ]
  %src.addr.112.i = phi i8* [ %add.ptr46.i, %for.cond.cleanup26.i ], [ %add.ptr9, %for.cond23.preheader.lr.ph.i ]
  %202 = mul i64 %indvar142, %idx.ext43.i
  %scevgep144 = getelementptr [1000 x i8], [1000 x i8]* @dst, i64 0, i64 %202
  %203 = add i64 %202, %wide.trip.count19.i
  %scevgep145 = getelementptr [1000 x i8], [1000 x i8]* @dst, i64 0, i64 %203
  %204 = shl i64 %indvar142, 1
  %205 = add i64 %156, %204
  %scevgep146 = getelementptr i8, i8* %2, i64 %205
  %206 = add i64 %158, %204
  %scevgep147 = getelementptr i8, i8* %2, i64 %206
  br i1 %cmp249.i, label %for.body27.i.preheader, label %for.cond.cleanup26.i

for.body27.i.preheader:                           ; preds = %for.cond23.preheader.i
  br i1 %min.iters.check140, label %for.body27.i.preheader200, label %vector.memcheck156

for.body27.i.preheader200:                        ; preds = %middle.block138, %vector.memcheck156, %for.body27.i.preheader
  %indvars.iv17.i.ph = phi i64 [ 0, %vector.memcheck156 ], [ 0, %for.body27.i.preheader ], [ %n.vec159, %middle.block138 ]
  %207 = xor i64 %indvars.iv17.i.ph, -1
  br i1 %lcmp.mod, label %for.body27.i.prol.loopexit, label %for.body27.i.prol

for.body27.i.prol:                                ; preds = %for.body27.i.preheader200
  %arrayidx29.i.prol = getelementptr inbounds i8, i8* %src.addr.112.i, i64 %indvars.iv17.i.ph
  %208 = load i8, i8* %arrayidx29.i.prol, align 1, !tbaa !12
  %conv30.i.prol = zext i8 %208 to i32
  %209 = load i32, i32* getelementptr inbounds (%struct.x264_weight_type, %struct.x264_weight_type* @weight_t, i64 0, i32 3), align 4, !tbaa !29
  %mul32.i.prol = mul nsw i32 %209, %conv30.i.prol
  %210 = load i32, i32* getelementptr inbounds (%struct.x264_weight_type, %struct.x264_weight_type* @weight_t, i64 0, i32 4), align 8, !tbaa !33
  %add34.i.prol = add nsw i32 %mul32.i.prol, %210
  %211 = icmp sgt i32 %add34.i.prol, 0
  %212 = select i1 %211, i32 %add34.i.prol, i32 0
  %213 = icmp slt i32 %212, 255
  %214 = select i1 %213, i32 %212, i32 255
  %conv.i1.i.prol = trunc i32 %214 to i8
  %arrayidx37.i.prol = getelementptr inbounds i8, i8* %dst.addr.113.i, i64 %indvars.iv17.i.ph
  store i8 %conv.i1.i.prol, i8* %arrayidx37.i.prol, align 1, !tbaa !12
  %indvars.iv.next18.i.prol = or i64 %indvars.iv17.i.ph, 1
  br label %for.body27.i.prol.loopexit

for.body27.i.prol.loopexit:                       ; preds = %for.body27.i.prol, %for.body27.i.preheader200
  %indvars.iv17.i.unr.ph = phi i64 [ %indvars.iv.next18.i.prol, %for.body27.i.prol ], [ %indvars.iv17.i.ph, %for.body27.i.preheader200 ]
  %215 = icmp eq i64 %207, %159
  br i1 %215, label %for.cond.cleanup26.i, label %for.body27.i

vector.memcheck156:                               ; preds = %for.body27.i.preheader
  %bound0148 = icmp ult i8* %scevgep144, %scevgep147
  %bound1149 = icmp ult i8* %scevgep146, %scevgep145
  %found.conflict150 = and i1 %bound0148, %bound1149
  %bound0151 = icmp ult i8* %scevgep144, getelementptr (i8, i8* bitcast (i32* getelementptr inbounds (%struct.x264_weight_type, %struct.x264_weight_type* @weight_t, i64 0, i32 4) to i8*), i64 1)
  %bound1152 = icmp ugt i8* %scevgep145, bitcast (i32* getelementptr inbounds (%struct.x264_weight_type, %struct.x264_weight_type* @weight_t, i64 0, i32 4) to i8*)
  %found.conflict153 = and i1 %bound0151, %bound1152
  %conflict.rdx154 = or i1 %found.conflict150, %found.conflict153
  br i1 %conflict.rdx154, label %for.body27.i.preheader200, label %vector.ph157

vector.ph157:                                     ; preds = %vector.memcheck156
  %216 = load i32, i32* getelementptr inbounds (%struct.x264_weight_type, %struct.x264_weight_type* @weight_t, i64 0, i32 3), align 4, !tbaa !29, !alias.scope !55
  %217 = insertelement <4 x i32> undef, i32 %216, i32 0
  %218 = shufflevector <4 x i32> %217, <4 x i32> undef, <4 x i32> zeroinitializer
  %219 = load i32, i32* getelementptr inbounds (%struct.x264_weight_type, %struct.x264_weight_type* @weight_t, i64 0, i32 4), align 8, !tbaa !33, !alias.scope !55
  %220 = insertelement <4 x i32> undef, i32 %219, i32 0
  %221 = shufflevector <4 x i32> %220, <4 x i32> undef, <4 x i32> zeroinitializer
  br label %vector.body137

vector.body137:                                   ; preds = %vector.body137, %vector.ph157
  %index160 = phi i64 [ 0, %vector.ph157 ], [ %index.next161, %vector.body137 ]
  %222 = getelementptr inbounds i8, i8* %src.addr.112.i, i64 %index160
  %223 = bitcast i8* %222 to <4 x i8>*
  %wide.load167 = load <4 x i8>, <4 x i8>* %223, align 1, !tbaa !12, !alias.scope !58
  %224 = zext <4 x i8> %wide.load167 to <4 x i32>
  %225 = mul nsw <4 x i32> %218, %224
  %226 = add nsw <4 x i32> %225, %221
  %227 = icmp sgt <4 x i32> %226, zeroinitializer
  %228 = select <4 x i1> %227, <4 x i32> %226, <4 x i32> zeroinitializer
  %229 = icmp slt <4 x i32> %228, <i32 255, i32 255, i32 255, i32 255>
  %230 = select <4 x i1> %229, <4 x i32> %228, <4 x i32> <i32 255, i32 255, i32 255, i32 255>
  %231 = trunc <4 x i32> %230 to <4 x i8>
  %232 = getelementptr inbounds i8, i8* %dst.addr.113.i, i64 %index160
  %233 = bitcast i8* %232 to <4 x i8>*
  store <4 x i8> %231, <4 x i8>* %233, align 1, !tbaa !12, !alias.scope !60, !noalias !62
  %index.next161 = add i64 %index160, 4
  %234 = icmp eq i64 %index.next161, %n.vec159
  br i1 %234, label %middle.block138, label %vector.body137, !llvm.loop !63

middle.block138:                                  ; preds = %vector.body137
  br i1 %cmp.n163, label %for.cond.cleanup26.i, label %for.body27.i.preheader200

for.cond.cleanup26.i:                             ; preds = %for.body27.i.prol.loopexit, %for.body27.i, %middle.block138, %for.cond23.preheader.i
  %inc42.i = add nuw nsw i32 %y16.014.i, 1
  %add.ptr44.i = getelementptr inbounds i8, i8* %dst.addr.113.i, i64 %idx.ext43.i, !intel-tbaa !12
  %add.ptr46.i = getelementptr inbounds i8, i8* %src.addr.112.i, i64 2, !intel-tbaa !12
  %exitcond21.i = icmp eq i32 %inc42.i, %i_height
  %indvar.next143 = add i64 %indvar142, 1
  br i1 %exitcond21.i, label %cleanup, label %for.cond23.preheader.i

for.body27.i:                                     ; preds = %for.body27.i.prol.loopexit, %for.body27.i
  %indvars.iv17.i = phi i64 [ %indvars.iv.next18.i.1, %for.body27.i ], [ %indvars.iv17.i.unr.ph, %for.body27.i.prol.loopexit ]
  %arrayidx29.i = getelementptr inbounds i8, i8* %src.addr.112.i, i64 %indvars.iv17.i
  %235 = load i8, i8* %arrayidx29.i, align 1, !tbaa !12
  %conv30.i = zext i8 %235 to i32
  %236 = load i32, i32* getelementptr inbounds (%struct.x264_weight_type, %struct.x264_weight_type* @weight_t, i64 0, i32 3), align 4, !tbaa !29
  %mul32.i = mul nsw i32 %236, %conv30.i
  %237 = load i32, i32* getelementptr inbounds (%struct.x264_weight_type, %struct.x264_weight_type* @weight_t, i64 0, i32 4), align 8, !tbaa !33
  %add34.i = add nsw i32 %mul32.i, %237
  %238 = icmp sgt i32 %add34.i, 0
  %239 = select i1 %238, i32 %add34.i, i32 0
  %240 = icmp slt i32 %239, 255
  %241 = select i1 %240, i32 %239, i32 255
  %conv.i1.i = trunc i32 %241 to i8
  %arrayidx37.i = getelementptr inbounds i8, i8* %dst.addr.113.i, i64 %indvars.iv17.i
  store i8 %conv.i1.i, i8* %arrayidx37.i, align 1, !tbaa !12
  %indvars.iv.next18.i = add nuw nsw i64 %indvars.iv17.i, 1
  %arrayidx29.i.1 = getelementptr inbounds i8, i8* %src.addr.112.i, i64 %indvars.iv.next18.i
  %242 = load i8, i8* %arrayidx29.i.1, align 1, !tbaa !12
  %conv30.i.1 = zext i8 %242 to i32
  %243 = load i32, i32* getelementptr inbounds (%struct.x264_weight_type, %struct.x264_weight_type* @weight_t, i64 0, i32 3), align 4, !tbaa !29
  %mul32.i.1 = mul nsw i32 %243, %conv30.i.1
  %244 = load i32, i32* getelementptr inbounds (%struct.x264_weight_type, %struct.x264_weight_type* @weight_t, i64 0, i32 4), align 8, !tbaa !33
  %add34.i.1 = add nsw i32 %mul32.i.1, %244
  %245 = icmp sgt i32 %add34.i.1, 0
  %246 = select i1 %245, i32 %add34.i.1, i32 0
  %247 = icmp slt i32 %246, 255
  %248 = select i1 %247, i32 %246, i32 255
  %conv.i1.i.1 = trunc i32 %248 to i8
  %arrayidx37.i.1 = getelementptr inbounds i8, i8* %dst.addr.113.i, i64 %indvars.iv.next18.i
  store i8 %conv.i1.i.1, i8* %arrayidx37.i.1, align 1, !tbaa !12
  %indvars.iv.next18.i.1 = add nsw i64 %indvars.iv17.i, 2
  %exitcond20.i.1 = icmp eq i64 %indvars.iv.next18.i.1, %wide.trip.count19.i
  br i1 %exitcond20.i.1, label %for.cond.cleanup26.i, label %for.body27.i, !llvm.loop !64

if.else27:                                        ; preds = %if.else
  store i32 2, i32* %i_dst_stride, align 4, !tbaa !6
  br label %cleanup

cleanup:                                          ; preds = %for.cond.cleanup26.i54, %for.cond.cleanup4.i31, %for.cond.cleanup26.i, %for.cond.cleanup4.i, %for.cond.preheader.i, %for.cond17.preheader.i, %for.cond.preheader.i18, %for.cond17.preheader.i13, %pixel_avg.exit, %if.else27
  ret void
}


attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { noinline norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"icx (ICX) dev.8.x.0"}
!2 = !{!3, !3, i64 0}
!3 = !{!"pointer@_ZTSPi", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!7, !7, i64 0}
!7 = !{!"int", !4, i64 0}
!8 = !{!9, !4, i64 0}
!9 = !{!"array@_ZTSA16_h", !4, i64 0}
!10 = !{!11, !11, i64 0}
!11 = !{!"pointer@_ZTSPh", !4, i64 0}
!12 = !{!4, !4, i64 0}
!13 = !{!14}
!14 = distinct !{!14, !15}
!15 = distinct !{!15, !"LVerDomain"}
!16 = !{!17}
!17 = distinct !{!17, !15}
!18 = !{!19}
!19 = distinct !{!19, !15}
!20 = !{!14, !17}
!21 = distinct !{!21, !22}
!22 = !{!"llvm.loop.isvectorized", i32 1}
!23 = distinct !{!23, !22}
!24 = !{!25, !3, i64 48}
!25 = !{!"struct@x264_weight_type", !26, i64 0, !26, i64 16, !7, i64 32, !7, i64 36, !7, i64 40, !3, i64 48}
!26 = !{!"array@_ZTSA8_s", !27, i64 0}
!27 = !{!"short", !4, i64 0}
!28 = !{!25, !7, i64 32}
!29 = !{!25, !7, i64 36}
!30 = !{!31}
!31 = distinct !{!31, !32}
!32 = distinct !{!32, !"LVerDomain"}
!33 = !{!25, !7, i64 40}
!34 = !{!35}
!35 = distinct !{!35, !32}
!36 = distinct !{!36, !22}
!37 = distinct !{!37, !22}
!38 = !{!39}
!39 = distinct !{!39, !40}
!40 = distinct !{!40, !"LVerDomain"}
!41 = !{!42}
!42 = distinct !{!42, !40}
!43 = distinct !{!43, !22}
!44 = distinct !{!44, !22}
!45 = !{!46}
!46 = distinct !{!46, !47}
!47 = distinct !{!47, !"LVerDomain"}
!48 = !{!49}
!49 = distinct !{!49, !47}
!50 = !{!51}
!51 = distinct !{!51, !47}
!52 = !{!49, !46}
!53 = distinct !{!53, !22}
!54 = distinct !{!54, !22}
!55 = !{!56}
!56 = distinct !{!56, !57}
!57 = distinct !{!57, !"LVerDomain"}
!58 = !{!59}
!59 = distinct !{!59, !57}
!60 = !{!61}
!61 = distinct !{!61, !57}
!62 = !{!59, !56}
!63 = distinct !{!63, !22}
!64 = distinct !{!64, !22}

; Original source test.c is below.
; Compile: icx -flto -c -O2 -S -emit-llvm test.c
;
;#include <stdint.h>
;#include <stdio.h>
;#include <string.h>
;
;typedef struct x264_weight_type {
;  /* aligning the first member is a gcc hack to force the struct to be
;     * 16 byte aligned, as well as force sizeof(struct) to be a multiple of 16 */
;  int16_t cachea[8];
;  int16_t cacheb[8];
;  int32_t i_denom;
;  int32_t i_scale;
;  int32_t i_offset;
;  //weight_fn_t *weightfn;
;  int *weightfn;
;} x264_weight_t;
;
;static const uint8_t hpel_ref0[16] = {0, 1, 1, 1, 0, 1, 1, 1, 2, 3, 3, 3, 0, 1, 1, 1};
;static const uint8_t hpel_ref1[16] = {0, 0, 0, 0, 2, 2, 3, 2, 2, 2, 3, 2, 2, 2, 3, 2};
;
;static __attribute__((always_inline)) uint8_t x264_clip_uint8(int x) {
;  return x & (~255) ? (-x) >> 31 : x;
;}
;
;//pixel_avg() is a leaf function by itself:
;// has only 2 static callers:
;// - 1 in mc_luma();
;// - 1 in get_ref();
;static __attribute__((always_inline)) void pixel_avg(uint8_t *dst, int i_dst_stride,
;                                                     uint8_t *src1, int i_src1_stride,
;                                                     uint8_t *src2, int i_src2_stride,
;                                                     int i_width, int i_height) {
;  for (int y = 0; y < i_height; y++) {
;    for (int x = 0; x < i_width; x++)
;      dst[x] = (src1[x] + src2[x] + 1) >> 1;
;    dst += i_dst_stride;
;    src1 += i_src1_stride;
;    src2 += i_src2_stride;
;  }
;}
;
;//mc_weight() is a leaf function once opscale() and opscale_noden() are properly expanded:
;// mc_weight() has 4 callers:
;// - 2 inside mc_chroma();
;// - 2 inside
;#define opscale(x) dst[x] = x264_clip_uint8(((src[x] * weight->i_scale + (1 << (weight->i_denom - 1))) >> weight->i_denom) + weight->i_offset)
;#define opscale_noden(x) dst[x] = x264_clip_uint8(src[x] * weight->i_scale + weight->i_offset)
;
;static __attribute__((always_inline)) void mc_weight(uint8_t *dst,
;                                                     int i_dst_stride, uint8_t *src, int i_src_stride, const x264_weight_t *weight, int i_width, int i_height) {
;
;  if (weight->i_denom >= 1) {
;    for (int y = 0; y < i_height; y++, dst += i_dst_stride, src += i_src_stride)
;      for (int x = 0; x < i_width; x++)
;        opscale(x);
;  } else {
;    for (int y = 0; y < i_height; y++, dst += i_dst_stride, src += i_src_stride)
;      for (int x = 0; x < i_width; x++)
;        opscale_noden(x);
;  }
;}
;
;//get_ref() only calls to 2 user-defined functions:
;// - pixel_avg();
;// - mc_weight();
;// once they are both inlined, get_ref() will become a leaf function.
;//
;static __attribute__((noinline)) uint8_t * get_ref(uint8_t *dst, int *i_dst_stride,
;                                                   uint8_t *src[4], int i_src_stride,
;                                                   int mvx, int mvy,
;                                                   int i_width, int i_height, const x264_weight_t *weight) {
;  int qpel_idx = ((mvy & 3) << 2) + (mvx & 3);
;  int offset = (mvy >> 2) * i_src_stride + (mvx >> 2);
;  uint8_t *src1 = src[hpel_ref0[qpel_idx]] + offset + ((mvy & 3) == 3) * i_src_stride;
;
;  if (qpel_idx & 5) /* qpel interpolation needed */
;  {
;    uint8_t *src2 = src[hpel_ref1[qpel_idx]] + offset + ((mvx & 3) == 3);
;    pixel_avg(dst, *i_dst_stride, src1, i_src_stride,
;              src2, i_src_stride, i_width, i_height);
;    if (weight->weightfn)
;      mc_weight(dst, *i_dst_stride, dst, *i_dst_stride, weight, i_width, i_height);
;    return dst;
;  } else if (weight->weightfn) {
;    mc_weight(dst, *i_dst_stride, src1, i_src_stride, weight, i_width, i_height);
;    return dst;
;  } else {
;    *i_dst_stride = i_src_stride;
;    return src1;
;  }
;}
;
;uint8_t dst[1000];
;uint8_t src[1000];
;uint8_t src4[4];
;int dst_stride = 1;
;int *i_dst_stride = &dst_stride;
;x264_weight_t weight_t;
;int mvx = 1, mvy = 2;
;
;int main(void) {
;  uint8_t *RV = 0;
;
;  RV = get_ref(dst, i_dst_stride,
;               &src4, 2, //int i_src_stride,
;               mvx, mvy,
;               16, //int i_width,
;               16, //int i_height,
;               &weight_t);
;
;  RV = get_ref(dst, i_dst_stride,
;               &src4, 2, //int i_src_stride,
;               mvx, mvy,
;               8, //int i_width,
;               8, //int i_height,
;               &weight_t);
;
;  RV = get_ref(dst, i_dst_stride,
;               &src4, 2, //int i_src_stride,
;               mvx, mvy,
;               8,  //int i_width,
;               16, //int i_height,
;               &weight_t);
;
;  RV = get_ref(dst, i_dst_stride,
;               &src4, 2, //int i_src_stride,
;               mvx, mvy,
;               16, //int i_width,
;               8,  //int i_height,
;               &weight_t);
;
;  int i;
;  for (i = 0; i < 100; ++i) {
;    RV = get_ref(dst, i_dst_stride,
;                 &src4, 2, //int i_src_stride,
;                 mvx, mvy,
;                 i,       //int i_width,
;                 100 - i, //int i_height,
;                 &weight_t);
;  }
;
;  return 0;
;}
;
;
