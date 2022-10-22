; Checks that the Multi-Versioning (MV) transformation does NOT trigger on a function, even though it may have passed
; the collection, but fails analysis.
;

; RUN: opt < %s -opaque-pointers -passes='module(call-tree-clone)' -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -call-tree-clone-mv-bypass-coll-for-littest=0 -S | FileCheck %s

; This is the same test case as call_tree_cloning_12.ll, but it checks for
; opaque pointers.

; ModuleID = 'call_tree_cloning_12-opaque-ptr.ll'
source_filename = "test.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@I0 = dso_local global i64 0, align 8
@I1 = dso_local global i64 1, align 8
@I2 = dso_local global i64 2, align 8
@I3 = dso_local global i64 3, align 8
@I4 = dso_local global i64 4, align 8
@I5 = dso_local global i64 5, align 8
@I6 = dso_local local_unnamed_addr global i64 6, align 8
@F0 = common dso_local global float 0.000000e+00, align 4
@F1 = common dso_local global float 0.000000e+00, align 4

; Function Attrs: noinline norecurse nounwind uwtable
define dso_local void @fftwf_cpy2d(ptr %I, ptr %O, i64 %n0, i64 %is0, i64 %os0, i64 %n1, i64 %is1, i64 %os1, i64 %vl) local_unnamed_addr #0 {
entry:
  switch i64 %vl, label %for.cond78.preheader [
    i64 1, label %for.cond.preheader
    i64 2, label %sw.bb12
  ]

for.cond.preheader:                               ; preds = %entry
  %cmp204 = icmp eq i64 %n1, 0
  br i1 %cmp204, label %sw.epilog, label %for.cond1.preheader.lr.ph

for.cond1.preheader.lr.ph:                        ; preds = %for.cond.preheader
  %cmp2202 = icmp eq i64 %n0, 0
  %xtraiter249 = and i64 %n0, 1
  %i = icmp eq i64 %n0, 1
  %unroll_iter251 = sub i64 %n0, %xtraiter249
  %lcmp.mod250 = icmp eq i64 %xtraiter249, 0
  br label %for.cond1.preheader

for.cond78.preheader:                             ; preds = %entry
  %cmp79199 = icmp eq i64 %n1, 0
  br i1 %cmp79199, label %sw.epilog, label %for.cond81.preheader.lr.ph

for.cond81.preheader.lr.ph:                       ; preds = %for.cond78.preheader
  %cmp82196 = icmp eq i64 %n0, 0
  %cmp85194 = icmp eq i64 %vl, 0
  %i1 = add i64 %vl, -8
  %i2 = lshr i64 %i1, 3
  %i3 = add nuw nsw i64 %i2, 1
  %min.iters.check = icmp ult i64 %vl, 8
  %n.vec = and i64 %vl, -8
  %xtraiter = and i64 %i3, 1
  %i4 = icmp eq i64 %i2, 0
  %unroll_iter = sub nuw nsw i64 %i3, %xtraiter
  %lcmp.mod = icmp eq i64 %xtraiter, 0
  %cmp.n = icmp eq i64 %n.vec, %vl
  %xtraiter247 = and i64 %vl, 3
  %lcmp.mod248 = icmp eq i64 %xtraiter247, 0
  br label %for.cond81.preheader

for.cond1.preheader:                              ; preds = %for.inc9, %for.cond1.preheader.lr.ph
  %i1.0205 = phi i64 [ 0, %for.cond1.preheader.lr.ph ], [ %inc10, %for.inc9 ]
  br i1 %cmp2202, label %for.inc9, label %for.body3.lr.ph

for.body3.lr.ph:                                  ; preds = %for.cond1.preheader
  %mul4 = mul i64 %i1.0205, %is1
  %mul6 = mul i64 %i1.0205, %os1
  br i1 %i, label %for.inc9.loopexit.unr-lcssa, label %for.body3

for.body3:                                        ; preds = %for.body3, %for.body3.lr.ph
  %i0.0203 = phi i64 [ %inc.1, %for.body3 ], [ 0, %for.body3.lr.ph ]
  %niter252 = phi i64 [ %niter252.nsub.1, %for.body3 ], [ %unroll_iter251, %for.body3.lr.ph ]
  %mul = mul i64 %i0.0203, %is0
  %add = add i64 %mul, %mul4
  %arrayidx = getelementptr inbounds float, ptr %I, i64 %add
  %i6 = load i32, ptr %arrayidx, align 4, !tbaa !2
  %mul5 = mul i64 %i0.0203, %os0
  %add7 = add i64 %mul5, %mul6
  %arrayidx8 = getelementptr inbounds float, ptr %O, i64 %add7
  store i32 %i6, ptr %arrayidx8, align 4, !tbaa !2
  %inc = or i64 %i0.0203, 1
  %mul.1 = mul i64 %inc, %is0
  %add.1 = add i64 %mul.1, %mul4
  %arrayidx.1 = getelementptr inbounds float, ptr %I, i64 %add.1
  %i9 = load i32, ptr %arrayidx.1, align 4, !tbaa !2
  %mul5.1 = mul i64 %inc, %os0
  %add7.1 = add i64 %mul5.1, %mul6
  %arrayidx8.1 = getelementptr inbounds float, ptr %O, i64 %add7.1
  store i32 %i9, ptr %arrayidx8.1, align 4, !tbaa !2
  %inc.1 = add i64 %i0.0203, 2
  %niter252.nsub.1 = add i64 %niter252, -2
  %niter252.ncmp.1 = icmp eq i64 %niter252.nsub.1, 0
  br i1 %niter252.ncmp.1, label %for.inc9.loopexit.unr-lcssa, label %for.body3

for.inc9.loopexit.unr-lcssa:                      ; preds = %for.body3, %for.body3.lr.ph
  %i0.0203.unr = phi i64 [ 0, %for.body3.lr.ph ], [ %inc.1, %for.body3 ]
  br i1 %lcmp.mod250, label %for.inc9, label %for.body3.epil

for.body3.epil:                                   ; preds = %for.inc9.loopexit.unr-lcssa
  %mul.epil = mul i64 %i0.0203.unr, %is0
  %add.epil = add i64 %mul.epil, %mul4
  %arrayidx.epil = getelementptr inbounds float, ptr %I, i64 %add.epil
  %i12 = load i32, ptr %arrayidx.epil, align 4, !tbaa !2
  %mul5.epil = mul i64 %i0.0203.unr, %os0
  %add7.epil = add i64 %mul5.epil, %mul6
  %arrayidx8.epil = getelementptr inbounds float, ptr %O, i64 %add7.epil
  store i32 %i12, ptr %arrayidx8.epil, align 4, !tbaa !2
  br label %for.inc9

for.inc9:                                         ; preds = %for.body3.epil, %for.inc9.loopexit.unr-lcssa, %for.cond1.preheader
  %inc10 = add nuw i64 %i1.0205, 1
  %exitcond224 = icmp eq i64 %inc10, %n1
  br i1 %exitcond224, label %sw.epilog, label %for.cond1.preheader

sw.bb12:                                          ; preds = %entry
  %i14 = ptrtoint ptr %I to i64
  %rem = and i64 %i14, 7
  %cmp13 = icmp eq i64 %rem, 0
  br i1 %cmp13, label %land.lhs.true, label %if.else

land.lhs.true:                                    ; preds = %sw.bb12
  %i15 = ptrtoint ptr %O to i64
  %rem14 = and i64 %i15, 7
  %and = and i64 %is0, 1
  %i16 = or i64 %and, %rem14
  %and19 = and i64 %is1, 1
  %and22 = and i64 %os0, 1
  %and25 = and i64 %os1, 1
  %i17 = or i64 %i16, %and22
  %i18 = or i64 %i17, %and19
  %i19 = or i64 %i18, %and25
  %i20 = icmp eq i64 %i19, 0
  br i1 %i20, label %for.cond27.preheader, label %if.else

for.cond27.preheader:                             ; preds = %land.lhs.true
  %cmp28209 = icmp eq i64 %n1, 0
  br i1 %cmp28209, label %sw.epilog, label %for.cond30.preheader.lr.ph

for.cond30.preheader.lr.ph:                       ; preds = %for.cond27.preheader
  %cmp31207 = icmp eq i64 %n0, 0
  %xtraiter253 = and i64 %n0, 1
  %i21 = icmp eq i64 %n0, 1
  %unroll_iter255 = sub i64 %n0, %xtraiter253
  %lcmp.mod254 = icmp eq i64 %xtraiter253, 0
  br label %for.cond30.preheader

for.cond30.preheader:                             ; preds = %for.inc44, %for.cond30.preheader.lr.ph
  %i1.1210 = phi i64 [ 0, %for.cond30.preheader.lr.ph ], [ %inc45, %for.inc44 ]
  br i1 %cmp31207, label %for.inc44, label %for.body32.lr.ph

for.body32.lr.ph:                                 ; preds = %for.cond30.preheader
  %mul34 = mul i64 %i1.1210, %is1
  %mul38 = mul i64 %i1.1210, %os1
  br i1 %i21, label %for.inc44.loopexit.unr-lcssa, label %for.body32

for.body32:                                       ; preds = %for.body32, %for.body32.lr.ph
  %i0.1208 = phi i64 [ %inc42.1, %for.body32 ], [ 0, %for.body32.lr.ph ]
  %niter256 = phi i64 [ %niter256.nsub.1, %for.body32 ], [ %unroll_iter255, %for.body32.lr.ph ]
  %mul33 = mul i64 %i0.1208, %is0
  %add35 = add i64 %mul33, %mul34
  %arrayidx36 = getelementptr inbounds float, ptr %I, i64 %add35
  %i23 = load i64, ptr %arrayidx36, align 8, !tbaa !6
  %mul37 = mul i64 %i0.1208, %os0
  %add39 = add i64 %mul37, %mul38
  %arrayidx40 = getelementptr inbounds float, ptr %O, i64 %add39
  store i64 %i23, ptr %arrayidx40, align 8, !tbaa !6
  %inc42 = or i64 %i0.1208, 1
  %mul33.1 = mul i64 %inc42, %is0
  %add35.1 = add i64 %mul33.1, %mul34
  %arrayidx36.1 = getelementptr inbounds float, ptr %I, i64 %add35.1
  %i26 = load i64, ptr %arrayidx36.1, align 8, !tbaa !6
  %mul37.1 = mul i64 %inc42, %os0
  %add39.1 = add i64 %mul37.1, %mul38
  %arrayidx40.1 = getelementptr inbounds float, ptr %O, i64 %add39.1
  store i64 %i26, ptr %arrayidx40.1, align 8, !tbaa !6
  %inc42.1 = add i64 %i0.1208, 2
  %niter256.nsub.1 = add i64 %niter256, -2
  %niter256.ncmp.1 = icmp eq i64 %niter256.nsub.1, 0
  br i1 %niter256.ncmp.1, label %for.inc44.loopexit.unr-lcssa, label %for.body32

for.inc44.loopexit.unr-lcssa:                     ; preds = %for.body32, %for.body32.lr.ph
  %i0.1208.unr = phi i64 [ 0, %for.body32.lr.ph ], [ %inc42.1, %for.body32 ]
  br i1 %lcmp.mod254, label %for.inc44, label %for.body32.epil

for.body32.epil:                                  ; preds = %for.inc44.loopexit.unr-lcssa
  %mul33.epil = mul i64 %i0.1208.unr, %is0
  %add35.epil = add i64 %mul33.epil, %mul34
  %arrayidx36.epil = getelementptr inbounds float, ptr %I, i64 %add35.epil
  %i29 = load i64, ptr %arrayidx36.epil, align 8, !tbaa !6
  %mul37.epil = mul i64 %i0.1208.unr, %os0
  %add39.epil = add i64 %mul37.epil, %mul38
  %arrayidx40.epil = getelementptr inbounds float, ptr %O, i64 %add39.epil
  store i64 %i29, ptr %arrayidx40.epil, align 8, !tbaa !6
  br label %for.inc44

for.inc44:                                        ; preds = %for.body32.epil, %for.inc44.loopexit.unr-lcssa, %for.cond30.preheader
  %inc45 = add nuw i64 %i1.1210, 1
  %exitcond226 = icmp eq i64 %inc45, %n1
  br i1 %exitcond226, label %sw.epilog, label %for.cond30.preheader

if.else:                                          ; preds = %land.lhs.true, %sw.bb12
  %cmp48214 = icmp eq i64 %n1, 0
  br i1 %cmp48214, label %sw.epilog, label %for.cond50.preheader.lr.ph

for.cond50.preheader.lr.ph:                       ; preds = %if.else
  %cmp51212 = icmp eq i64 %n0, 0
  %xtraiter257 = and i64 %n0, 1
  %i31 = icmp eq i64 %n0, 1
  %unroll_iter259 = sub i64 %n0, %xtraiter257
  %lcmp.mod258 = icmp eq i64 %xtraiter257, 0
  br label %for.cond50.preheader

for.cond50.preheader:                             ; preds = %for.inc75, %for.cond50.preheader.lr.ph
  %i1.2215 = phi i64 [ 0, %for.cond50.preheader.lr.ph ], [ %inc76, %for.inc75 ]
  br i1 %cmp51212, label %for.inc75, label %for.body52.lr.ph

for.body52.lr.ph:                                 ; preds = %for.cond50.preheader
  %mul55 = mul i64 %i1.2215, %is1
  %mul64 = mul i64 %i1.2215, %os1
  br i1 %i31, label %for.inc75.loopexit.unr-lcssa, label %for.body52

for.body52:                                       ; preds = %for.body52, %for.body52.lr.ph
  %i0.2213 = phi i64 [ %inc73.1, %for.body52 ], [ 0, %for.body52.lr.ph ]
  %niter260 = phi i64 [ %niter260.nsub.1, %for.body52 ], [ %unroll_iter259, %for.body52.lr.ph ]
  %mul54 = mul i64 %i0.2213, %is0
  %add56 = add i64 %mul54, %mul55
  %arrayidx57 = getelementptr inbounds float, ptr %I, i64 %add56
  %i33 = load i32, ptr %arrayidx57, align 4, !tbaa !2
  %add61 = add i64 %add56, 1
  %arrayidx62 = getelementptr inbounds float, ptr %I, i64 %add61
  %i35 = load i32, ptr %arrayidx62, align 4, !tbaa !2
  %mul63 = mul i64 %i0.2213, %os0
  %add65 = add i64 %mul63, %mul64
  %arrayidx66 = getelementptr inbounds float, ptr %O, i64 %add65
  store i32 %i33, ptr %arrayidx66, align 4, !tbaa !2
  %add70 = add i64 %add65, 1
  %arrayidx71 = getelementptr inbounds float, ptr %O, i64 %add70
  store i32 %i35, ptr %arrayidx71, align 4, !tbaa !2
  %inc73 = or i64 %i0.2213, 1
  %mul54.1 = mul i64 %inc73, %is0
  %add56.1 = add i64 %mul54.1, %mul55
  %arrayidx57.1 = getelementptr inbounds float, ptr %I, i64 %add56.1
  %i39 = load i32, ptr %arrayidx57.1, align 4, !tbaa !2
  %add61.1 = add i64 %add56.1, 1
  %arrayidx62.1 = getelementptr inbounds float, ptr %I, i64 %add61.1
  %i41 = load i32, ptr %arrayidx62.1, align 4, !tbaa !2
  %mul63.1 = mul i64 %inc73, %os0
  %add65.1 = add i64 %mul63.1, %mul64
  %arrayidx66.1 = getelementptr inbounds float, ptr %O, i64 %add65.1
  store i32 %i39, ptr %arrayidx66.1, align 4, !tbaa !2
  %add70.1 = add i64 %add65.1, 1
  %arrayidx71.1 = getelementptr inbounds float, ptr %O, i64 %add70.1
  store i32 %i41, ptr %arrayidx71.1, align 4, !tbaa !2
  %inc73.1 = add i64 %i0.2213, 2
  %niter260.nsub.1 = add i64 %niter260, -2
  %niter260.ncmp.1 = icmp eq i64 %niter260.nsub.1, 0
  br i1 %niter260.ncmp.1, label %for.inc75.loopexit.unr-lcssa, label %for.body52

for.inc75.loopexit.unr-lcssa:                     ; preds = %for.body52, %for.body52.lr.ph
  %i0.2213.unr = phi i64 [ 0, %for.body52.lr.ph ], [ %inc73.1, %for.body52 ]
  br i1 %lcmp.mod258, label %for.inc75, label %for.body52.epil

for.body52.epil:                                  ; preds = %for.inc75.loopexit.unr-lcssa
  %mul54.epil = mul i64 %i0.2213.unr, %is0
  %add56.epil = add i64 %mul54.epil, %mul55
  %arrayidx57.epil = getelementptr inbounds float, ptr %I, i64 %add56.epil
  %i45 = load i32, ptr %arrayidx57.epil, align 4, !tbaa !2
  %add61.epil = add i64 %add56.epil, 1
  %arrayidx62.epil = getelementptr inbounds float, ptr %I, i64 %add61.epil
  %i47 = load i32, ptr %arrayidx62.epil, align 4, !tbaa !2
  %mul63.epil = mul i64 %i0.2213.unr, %os0
  %add65.epil = add i64 %mul63.epil, %mul64
  %arrayidx66.epil = getelementptr inbounds float, ptr %O, i64 %add65.epil
  store i32 %i45, ptr %arrayidx66.epil, align 4, !tbaa !2
  %add70.epil = add i64 %add65.epil, 1
  %arrayidx71.epil = getelementptr inbounds float, ptr %O, i64 %add70.epil
  store i32 %i47, ptr %arrayidx71.epil, align 4, !tbaa !2
  br label %for.inc75

for.inc75:                                        ; preds = %for.body52.epil, %for.inc75.loopexit.unr-lcssa, %for.cond50.preheader
  %inc76 = add nuw i64 %i1.2215, 1
  %exitcond228 = icmp eq i64 %inc76, %n1
  br i1 %exitcond228, label %sw.epilog, label %for.cond50.preheader

for.cond81.preheader:                             ; preds = %for.inc104, %for.cond81.preheader.lr.ph
  %i1.3200 = phi i64 [ 0, %for.cond81.preheader.lr.ph ], [ %inc105, %for.inc104 ]
  %i50 = mul i64 %i1.3200, %os1
  %i51 = add i64 %i50, %vl
  %i52 = mul i64 %i1.3200, %is1
  %i53 = add i64 %i52, %vl
  br i1 %cmp82196, label %for.inc104, label %for.cond84.preheader.lr.ph

for.cond84.preheader.lr.ph:                       ; preds = %for.cond81.preheader
  %mul89 = mul i64 %i1.3200, %is1
  %mul94 = mul i64 %i1.3200, %os1
  br label %for.cond84.preheader

for.cond84.preheader:                             ; preds = %for.inc101, %for.cond84.preheader.lr.ph
  %i0.3197 = phi i64 [ 0, %for.cond84.preheader.lr.ph ], [ %inc102, %for.inc101 ]
  %i54 = mul i64 %i0.3197, %os0
  %i55 = add i64 %i50, %i54
  %scevgep = getelementptr float, ptr %O, i64 %i55
  %i56 = add i64 %i51, %i54
  %scevgep236 = getelementptr float, ptr %O, i64 %i56
  %i57 = mul i64 %i0.3197, %is0
  %i58 = add i64 %i52, %i57
  %scevgep238 = getelementptr float, ptr %I, i64 %i58
  %i59 = add i64 %i53, %i57
  %scevgep240 = getelementptr float, ptr %I, i64 %i59
  br i1 %cmp85194, label %for.inc101, label %for.body86.lr.ph

for.body86.lr.ph:                                 ; preds = %for.cond84.preheader
  %mul88 = mul i64 %i0.3197, %is0
  %add90 = add i64 %mul88, %mul89
  %mul93 = mul i64 %i0.3197, %os0
  %add95 = add i64 %mul93, %mul94
  br i1 %min.iters.check, label %for.body86.preheader, label %vector.memcheck

vector.memcheck:                                  ; preds = %for.body86.lr.ph
  %bound0 = icmp ult ptr %scevgep, %scevgep240
  %bound1 = icmp ult ptr %scevgep238, %scevgep236
  %found.conflict = and i1 %bound0, %bound1
  br i1 %found.conflict, label %for.body86.preheader, label %vector.ph

vector.ph:                                        ; preds = %vector.memcheck
  br i1 %i4, label %middle.block.unr-lcssa, label %vector.body

vector.body:                                      ; preds = %vector.body, %vector.ph
  %index = phi i64 [ %index.next.1, %vector.body ], [ 0, %vector.ph ]
  %niter = phi i64 [ %niter.nsub.1, %vector.body ], [ %unroll_iter, %vector.ph ]
  %i60 = add i64 %add90, %index
  %i61 = getelementptr inbounds float, ptr %I, i64 %i60
  %wide.load = load <4 x i32>, ptr %i61, align 4, !tbaa !2, !alias.scope !8
  %i63 = getelementptr inbounds float, ptr %i61, i64 4
  %wide.load243 = load <4 x i32>, ptr %i63, align 4, !tbaa !2, !alias.scope !8
  %i65 = add i64 %add95, %index
  %i66 = getelementptr inbounds float, ptr %O, i64 %i65
  store <4 x i32> %wide.load, ptr %i66, align 4, !tbaa !2, !alias.scope !11, !noalias !8
  %i68 = getelementptr inbounds float, ptr %i66, i64 4
  store <4 x i32> %wide.load243, ptr %i68, align 4, !tbaa !2, !alias.scope !11, !noalias !8
  %index.next = or i64 %index, 8
  %i70 = add i64 %add90, %index.next
  %i71 = getelementptr inbounds float, ptr %I, i64 %i70
  %wide.load.1 = load <4 x i32>, ptr %i71, align 4, !tbaa !2, !alias.scope !8
  %i73 = getelementptr inbounds float, ptr %i71, i64 4
  %wide.load243.1 = load <4 x i32>, ptr %i73, align 4, !tbaa !2, !alias.scope !8
  %i75 = add i64 %add95, %index.next
  %i76 = getelementptr inbounds float, ptr %O, i64 %i75
  store <4 x i32> %wide.load.1, ptr %i76, align 4, !tbaa !2, !alias.scope !11, !noalias !8
  %i78 = getelementptr inbounds float, ptr %i76, i64 4
  store <4 x i32> %wide.load243.1, ptr %i78, align 4, !tbaa !2, !alias.scope !11, !noalias !8
  %index.next.1 = add i64 %index, 16
  %niter.nsub.1 = add i64 %niter, -2
  %niter.ncmp.1 = icmp eq i64 %niter.nsub.1, 0
  br i1 %niter.ncmp.1, label %middle.block.unr-lcssa, label %vector.body, !llvm.loop !13

middle.block.unr-lcssa:                           ; preds = %vector.body, %vector.ph
  %index.unr = phi i64 [ 0, %vector.ph ], [ %index.next.1, %vector.body ]
  br i1 %lcmp.mod, label %middle.block, label %vector.body.epil

vector.body.epil:                                 ; preds = %middle.block.unr-lcssa
  %i80 = add i64 %add90, %index.unr
  %i81 = getelementptr inbounds float, ptr %I, i64 %i80
  %wide.load.epil = load <4 x i32>, ptr %i81, align 4, !tbaa !2, !alias.scope !8
  %i83 = getelementptr inbounds float, ptr %i81, i64 4
  %wide.load243.epil = load <4 x i32>, ptr %i83, align 4, !tbaa !2, !alias.scope !8
  %i85 = add i64 %add95, %index.unr
  %i86 = getelementptr inbounds float, ptr %O, i64 %i85
  store <4 x i32> %wide.load.epil, ptr %i86, align 4, !tbaa !2, !alias.scope !11, !noalias !8
  %i88 = getelementptr inbounds float, ptr %i86, i64 4
  store <4 x i32> %wide.load243.epil, ptr %i88, align 4, !tbaa !2, !alias.scope !11, !noalias !8
  br label %middle.block

middle.block:                                     ; preds = %vector.body.epil, %middle.block.unr-lcssa
  br i1 %cmp.n, label %for.inc101, label %for.body86.preheader

for.body86.preheader:                             ; preds = %middle.block, %vector.memcheck, %for.body86.lr.ph
  %v.0195.ph = phi i64 [ 0, %vector.memcheck ], [ 0, %for.body86.lr.ph ], [ %n.vec, %middle.block ]
  %i90 = xor i64 %v.0195.ph, -1
  %i91 = add i64 %i90, %vl
  br i1 %lcmp.mod248, label %for.body86.prol.loopexit, label %for.body86.prol

for.body86.prol:                                  ; preds = %for.body86.prol, %for.body86.preheader
  %v.0195.prol = phi i64 [ %inc99.prol, %for.body86.prol ], [ %v.0195.ph, %for.body86.preheader ]
  %prol.iter = phi i64 [ %prol.iter.sub, %for.body86.prol ], [ %xtraiter247, %for.body86.preheader ]
  %add91.prol = add i64 %add90, %v.0195.prol
  %arrayidx92.prol = getelementptr inbounds float, ptr %I, i64 %add91.prol
  %i93 = load i32, ptr %arrayidx92.prol, align 4, !tbaa !2
  %add96.prol = add i64 %add95, %v.0195.prol
  %arrayidx97.prol = getelementptr inbounds float, ptr %O, i64 %add96.prol
  store i32 %i93, ptr %arrayidx97.prol, align 4, !tbaa !2
  %inc99.prol = add nuw i64 %v.0195.prol, 1
  %prol.iter.sub = add i64 %prol.iter, -1
  %prol.iter.cmp = icmp eq i64 %prol.iter.sub, 0
  br i1 %prol.iter.cmp, label %for.body86.prol.loopexit, label %for.body86.prol, !llvm.loop !15

for.body86.prol.loopexit:                         ; preds = %for.body86.prol, %for.body86.preheader
  %v.0195.unr = phi i64 [ %v.0195.ph, %for.body86.preheader ], [ %inc99.prol, %for.body86.prol ]
  %i95 = icmp ult i64 %i91, 3
  br i1 %i95, label %for.inc101, label %for.body86

for.body86:                                       ; preds = %for.body86, %for.body86.prol.loopexit
  %v.0195 = phi i64 [ %inc99.3, %for.body86 ], [ %v.0195.unr, %for.body86.prol.loopexit ]
  %add91 = add i64 %add90, %v.0195
  %arrayidx92 = getelementptr inbounds float, ptr %I, i64 %add91
  %i97 = load i32, ptr %arrayidx92, align 4, !tbaa !2
  %add96 = add i64 %add95, %v.0195
  %arrayidx97 = getelementptr inbounds float, ptr %O, i64 %add96
  store i32 %i97, ptr %arrayidx97, align 4, !tbaa !2
  %inc99 = add nuw i64 %v.0195, 1
  %add91.1 = add i64 %add90, %inc99
  %arrayidx92.1 = getelementptr inbounds float, ptr %I, i64 %add91.1
  %i100 = load i32, ptr %arrayidx92.1, align 4, !tbaa !2
  %add96.1 = add i64 %add95, %inc99
  %arrayidx97.1 = getelementptr inbounds float, ptr %O, i64 %add96.1
  store i32 %i100, ptr %arrayidx97.1, align 4, !tbaa !2
  %inc99.1 = add i64 %v.0195, 2
  %add91.2 = add i64 %add90, %inc99.1
  %arrayidx92.2 = getelementptr inbounds float, ptr %I, i64 %add91.2
  %i103 = load i32, ptr %arrayidx92.2, align 4, !tbaa !2
  %add96.2 = add i64 %add95, %inc99.1
  %arrayidx97.2 = getelementptr inbounds float, ptr %O, i64 %add96.2
  store i32 %i103, ptr %arrayidx97.2, align 4, !tbaa !2
  %inc99.2 = add i64 %v.0195, 3
  %add91.3 = add i64 %add90, %inc99.2
  %arrayidx92.3 = getelementptr inbounds float, ptr %I, i64 %add91.3
  %i106 = load i32, ptr %arrayidx92.3, align 4, !tbaa !2
  %add96.3 = add i64 %add95, %inc99.2
  %arrayidx97.3 = getelementptr inbounds float, ptr %O, i64 %add96.3
  store i32 %i106, ptr %arrayidx97.3, align 4, !tbaa !2
  %inc99.3 = add i64 %v.0195, 4
  %exitcond.3 = icmp eq i64 %inc99.3, %vl
  br i1 %exitcond.3, label %for.inc101, label %for.body86, !llvm.loop !17

for.inc101:                                       ; preds = %for.body86, %for.body86.prol.loopexit, %middle.block, %for.cond84.preheader
  %inc102 = add nuw i64 %i0.3197, 1
  %exitcond221 = icmp eq i64 %inc102, %n0
  br i1 %exitcond221, label %for.inc104, label %for.cond84.preheader

for.inc104:                                       ; preds = %for.inc101, %for.cond81.preheader
  %inc105 = add nuw i64 %i1.3200, 1
  %exitcond222 = icmp eq i64 %inc105, %n1
  br i1 %exitcond222, label %sw.epilog, label %for.cond81.preheader

sw.epilog:                                        ; preds = %for.inc104, %for.inc75, %if.else, %for.inc44, %for.cond27.preheader, %for.inc9, %for.cond78.preheader, %for.cond.preheader
  ret void
}

; Function Attrs: noinline norecurse nounwind readnone uwtable
define dso_local i32 @toCTC(i32 %N) local_unnamed_addr #1 {
entry:
  %cmp8 = icmp sgt i32 %N, 0
  br i1 %cmp8, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  %i = add i32 %N, -1
  %xtraiter = and i32 %N, 7
  %i1 = icmp ult i32 %i, 7
  br i1 %i1, label %for.cond.cleanup.loopexit.unr-lcssa, label %for.body.preheader.new

for.body.preheader.new:                           ; preds = %for.body.preheader
  %unroll_iter = sub i32 %N, %xtraiter
  br label %for.body

for.cond.cleanup.loopexit.unr-lcssa:              ; preds = %for.body, %for.body.preheader
  %reass.mul.lcssa.ph = phi i32 [ undef, %for.body.preheader ], [ %reass.mul.7, %for.body ]
  %i.010.unr = phi i32 [ 0, %for.body.preheader ], [ %inc.7, %for.body ]
  %result.09.unr = phi i32 [ 0, %for.body.preheader ], [ %reass.mul.7, %for.body ]
  %lcmp.mod = icmp eq i32 %xtraiter, 0
  br i1 %lcmp.mod, label %for.cond.cleanup, label %for.body.epil

for.body.epil:                                    ; preds = %for.body.epil, %for.cond.cleanup.loopexit.unr-lcssa
  %i.010.epil = phi i32 [ %inc.epil, %for.body.epil ], [ %i.010.unr, %for.cond.cleanup.loopexit.unr-lcssa ]
  %result.09.epil = phi i32 [ %reass.mul.epil, %for.body.epil ], [ %result.09.unr, %for.cond.cleanup.loopexit.unr-lcssa ]
  %epil.iter = phi i32 [ %epil.iter.sub, %for.body.epil ], [ %xtraiter, %for.cond.cleanup.loopexit.unr-lcssa ]
  %reass.add.epil = add i32 %i.010.epil, %result.09.epil
  %reass.mul.epil = shl i32 %reass.add.epil, 1
  %inc.epil = add nuw nsw i32 %i.010.epil, 1
  %epil.iter.sub = add i32 %epil.iter, -1
  %epil.iter.cmp = icmp eq i32 %epil.iter.sub, 0
  br i1 %epil.iter.cmp, label %for.cond.cleanup, label %for.body.epil, !llvm.loop !18

for.cond.cleanup:                                 ; preds = %for.body.epil, %for.cond.cleanup.loopexit.unr-lcssa, %entry
  %result.0.lcssa = phi i32 [ 0, %entry ], [ %reass.mul.lcssa.ph, %for.cond.cleanup.loopexit.unr-lcssa ], [ %reass.mul.epil, %for.body.epil ]
  ret i32 %result.0.lcssa

for.body:                                         ; preds = %for.body, %for.body.preheader.new
  %i.010 = phi i32 [ 0, %for.body.preheader.new ], [ %inc.7, %for.body ]
  %result.09 = phi i32 [ 0, %for.body.preheader.new ], [ %reass.mul.7, %for.body ]
  %niter = phi i32 [ %unroll_iter, %for.body.preheader.new ], [ %niter.nsub.7, %for.body ]
  %reass.add = add i32 %i.010, %result.09
  %reass.mul = shl i32 %reass.add, 1
  %inc = or i32 %i.010, 1
  %reass.add.1 = add i32 %inc, %reass.mul
  %reass.mul.1 = shl i32 %reass.add.1, 1
  %inc.1 = or i32 %i.010, 2
  %reass.add.2 = add i32 %inc.1, %reass.mul.1
  %reass.mul.2 = shl i32 %reass.add.2, 1
  %inc.2 = or i32 %i.010, 3
  %reass.add.3 = add i32 %inc.2, %reass.mul.2
  %reass.mul.3 = shl i32 %reass.add.3, 1
  %inc.3 = or i32 %i.010, 4
  %reass.add.4 = add i32 %inc.3, %reass.mul.3
  %reass.mul.4 = shl i32 %reass.add.4, 1
  %inc.4 = or i32 %i.010, 5
  %reass.add.5 = add i32 %inc.4, %reass.mul.4
  %reass.mul.5 = shl i32 %reass.add.5, 1
  %inc.5 = or i32 %i.010, 6
  %reass.add.6 = add i32 %inc.5, %reass.mul.5
  %reass.mul.6 = shl i32 %reass.add.6, 1
  %inc.6 = or i32 %i.010, 7
  %reass.add.7 = add i32 %inc.6, %reass.mul.6
  %reass.mul.7 = shl i32 %reass.add.7, 1
  %inc.7 = add nuw nsw i32 %i.010, 8
  %niter.nsub.7 = add i32 %niter, -8
  %niter.ncmp.7 = icmp eq i32 %niter.nsub.7, 0
  br i1 %niter.ncmp.7, label %for.cond.cleanup.loopexit.unr-lcssa, label %for.body
}

; Function Attrs: nounwind uwtable
define dso_local i32 @main() local_unnamed_addr #2 {
entry:
  %N = alloca i32, align 4
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %N)
  store volatile i32 10, ptr %N, align 4, !tbaa !19
  %N.0.N.0.N.0. = load volatile i32, ptr %N, align 4, !tbaa !19
  %call = tail call i32 @toCTC(i32 %N.0.N.0.N.0.)
  %conv = sext i32 %call to i64
  store volatile i64 %conv, ptr @I5, align 8, !tbaa !21
  %i = load volatile i64, ptr @I0, align 8, !tbaa !21
  %i1 = load volatile i64, ptr @I1, align 8, !tbaa !21
  %i2 = load volatile i64, ptr @I2, align 8, !tbaa !21
  %i3 = load volatile i64, ptr @I3, align 8, !tbaa !21
  %i4 = load volatile i64, ptr @I4, align 8, !tbaa !21
  %i5 = load volatile i64, ptr @I5, align 8, !tbaa !21
  tail call void @fftwf_cpy2d(ptr nonnull @F0, ptr nonnull @F1, i64 %i, i64 %i1, i64 %i2, i64 %i3, i64 %i4, i64 %i5, i64 10)
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %N)
  ret i32 0
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #3

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #3

;
; CHECK-LABEL: define dso_local i32 @main() local_unnamed_addr #2 {
; CHECK:       %call = tail call i32 @toCTC(i32 %N.0.N.0.N.0.)
; CHECK:       %i5 = load volatile i64, ptr @I5, align 8, !tbaa !21
; CHECK:       call void @"fftwf_cpy2d|_._._._._._._._.10"(ptr nonnull @F0, ptr nonnull @F1, i64 %i, i64 %i1, i64 %i2, i64 %i3, i64 %i4, i64 %i5)

attributes #0 = { noinline norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { noinline norecurse nounwind readnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { argmemonly nofree nosync nounwind willreturn }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"icx (ICX) dev.8.x.0"}
!2 = !{!3, !3, i64 0}
!3 = !{!"float", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!7, !7, i64 0}
!7 = !{!"double", !4, i64 0}
!8 = !{!9}
!9 = distinct !{!9, !10}
!10 = distinct !{!10, !"LVerDomain"}
!11 = !{!12}
!12 = distinct !{!12, !10}
!13 = distinct !{!13, !14}
!14 = !{!"llvm.loop.isvectorized", i32 1}
!15 = distinct !{!15, !16}
!16 = !{!"llvm.loop.unroll.disable"}
!17 = distinct !{!17, !14}
!18 = distinct !{!18, !16}
!19 = !{!20, !20, i64 0}
!20 = !{!"int", !4, i64 0}
!21 = !{!22, !22, i64 0}
!22 = !{!"long", !4, i64 0}

; Original source test.c is below.
; Compile: icx -c -O0 -S -emit-llvm test.c
;
; The original failed function is called fftwf_cpy2d(), obtained from NAMD.2.13 package, under
; binaries/NAMD_2.13/9dbf1986511e68a6c3ee4af541687299/bld/NAMD_2.13/src/fftw-3.3.8/kernel/cpy2d.c.
; Test.c contains a modified failure function.
;
;#include <stdint.h>
;#include <stdio.h>
;
;typedef float R;
;typedef uint64_t INT;
;
;#define WIDE_TYPE double
;
;void fftwf_cpy2d(R *I, R *O, INT n0, INT is0, INT os0, INT n1, INT is1, INT os1,
;              INT vl) {
;  INT i0, i1, v;
;
;  switch (vl) {
;  case 1:
;    for (i1 = 0; i1 < n1; ++i1)
;      for (i0 = 0; i0 < n0; ++i0) {
;        R x0 = I[i0 * is0 + i1 * is1];
;        O[i0 * os0 + i1 * os1] = x0;
;      }
;    break;
;  case 2:
;    if (1 && (2 * sizeof(R) == sizeof(WIDE_TYPE)) &&
;        (sizeof(WIDE_TYPE) > sizeof(double)) &&
;        (((size_t)I) % sizeof(WIDE_TYPE) == 0) &&
;        (((size_t)O) % sizeof(WIDE_TYPE) == 0) && ((is0 & 1) == 0) &&
;        ((is1 & 1) == 0) && ((os0 & 1) == 0) && ((os1 & 1) == 0)) {
;      /* copy R[2] as WIDE_TYPE if WIDE_TYPE is large
;         enough to hold R[2], and if the input is
;         properly aligned.  This is a win when R==double
;         and WIDE_TYPE is 128 bits. */
;      for (i1 = 0; i1 < n1; ++i1)
;        for (i0 = 0; i0 < n0; ++i0) {
;          *(WIDE_TYPE *)&O[i0 * os0 + i1 * os1] =
;              *(WIDE_TYPE *)&I[i0 * is0 + i1 * is1];
;        }
;    } else if (1 && (2 * sizeof(R) == sizeof(double)) &&
;               (((size_t)I) % sizeof(double) == 0) &&
;               (((size_t)O) % sizeof(double) == 0) && ((is0 & 1) == 0) &&
;               ((is1 & 1) == 0) && ((os0 & 1) == 0) && ((os1 & 1) == 0)) {
;      /* copy R[2] as double if double is large enough to
;         hold R[2], and if the input is properly aligned.
;         This case applies when R==float */
;      for (i1 = 0; i1 < n1; ++i1)
;        for (i0 = 0; i0 < n0; ++i0) {
;          *(double *)&O[i0 * os0 + i1 * os1] =
;              *(double *)&I[i0 * is0 + i1 * is1];
;        }
;    } else {
;      for (i1 = 0; i1 < n1; ++i1)
;        for (i0 = 0; i0 < n0; ++i0) {
;          R x0 = I[i0 * is0 + i1 * is1];
;          R x1 = I[i0 * is0 + i1 * is1 + 1];
;          O[i0 * os0 + i1 * os1] = x0;
;          O[i0 * os0 + i1 * os1 + 1] = x1;
;        }
;    }
;    break;
;  default:
;    for (i1 = 0; i1 < n1; ++i1)
;      for (i0 = 0; i0 < n0; ++i0)
;        for (v = 0; v < vl; ++v) {
;          R x0 = I[i0 * is0 + i1 * is1 + v];
;          O[i0 * os0 + i1 * os1 + v] = x0;
;        }
;    break;
;  }
;}
;
;float F0;
;float F1;
;volatile uint64_t I0 = 0;
;volatile uint64_t I1 = 1;
;volatile uint64_t I2 = 2;
;volatile uint64_t I3 = 3;
;volatile uint64_t I4 = 4;
;volatile uint64_t I5 = 5;
;volatile uint64_t I6 = 6;
;
;int __attribute__((noinline)) toCTC(int N) {
;  int result = 0;
;  for (int i = 0; i < N; ++i) {
;    result += 2 * i + result;
;  }
;  return result;
;}
;
;int main(void) {
;  volatile int N = 10;
;  I5 = toCTC(N);
;  fftwf_cpy2d(&F0, &F1, I0, I1, I2, I3, I4, I5, 10);
;  return 0;
;}
;
