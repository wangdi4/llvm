
; Verify that we get the same IR output for the same input. This test case was resulting in IR differences across runs because we were traversing a SmallPtrSet of bblocks in SSA deconstruction.

; RUN: opt %s -passes="hir-ssa-deconstruction" > %t1.newpm
; RUN: opt %s -passes="hir-ssa-deconstruction" > %t2.newpm
; RUN: diff %t1.newpm %t2.newpm

;Module Before HIR; ModuleID = 'mpeg/mpeg2enc/predict.c'
source_filename = "mpeg/mpeg2enc/predict.c"
target datalayout = "e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-unknown-linux-gnu"

%struct.mbinfo = type { i32, i32, i32, i32, i32, i32, [2 x [2 x [2 x i32]]], [2 x [2 x i32]], [2 x i32], double, i32 }

@chroma_format = external local_unnamed_addr global i32, align 4

; Function Attrs: norecurse nounwind
define internal fastcc void @pred(ptr nocapture readonly %src, i32 %sfield, ptr nocapture readonly %dst, i32 %dfield, i32 %lx, i32 %h, i32 %x, i32 %y, i32 %dx, i32 %dy, i32 %addflag) unnamed_addr #2 {
entry:
  %tobool = icmp eq i32 %sfield, 0
  %tobool15 = icmp eq i32 %dfield, 0
  %tobool9.i = icmp eq i32 %addflag, 0
  %tobool177.i = icmp ne i32 %addflag, 0
  br label %for.body

for.body:                                         ; preds = %pred_comp.exit, %entry
  %cc.022 = phi i32 [ 0, %entry ], [ %inc, %pred_comp.exit ]
  %lx.addr.021 = phi i32 [ %lx, %entry ], [ %lx.addr.1, %pred_comp.exit ]
  %dy.addr.020 = phi i32 [ %dy, %entry ], [ %dy.addr.2, %pred_comp.exit ]
  %dx.addr.019 = phi i32 [ %dx, %entry ], [ %dx.addr.1, %pred_comp.exit ]
  %y.addr.018 = phi i32 [ %y, %entry ], [ %y.addr.2, %pred_comp.exit ]
  %x.addr.017 = phi i32 [ %x, %entry ], [ %x.addr.1, %pred_comp.exit ]
  %h.addr.016 = phi i32 [ %h, %entry ], [ %h.addr.2, %pred_comp.exit ]
  %w.addr.015 = phi i32 [ 16, %entry ], [ %w.addr.1, %pred_comp.exit ]
  %cmp1 = icmp eq i32 %cc.022, 1
  br i1 %cmp1, label %if.then, label %if.end12

if.then:                                          ; preds = %for.body
  %0 = load i32, ptr @chroma_format, align 4, !tbaa !1
  switch i32 %0, label %if.then6 [
    i32 1, label %if.end.thread
    i32 3, label %if.end12
  ]

if.end.thread:                                    ; preds = %if.then
  %shr = ashr i32 %h.addr.016, 1
  %shr4 = ashr i32 %y.addr.018, 1
  %div = sdiv i32 %dy.addr.020, 2
  br label %if.then6

if.then6:                                         ; preds = %if.then, %if.end.thread
  %dy.addr.17 = phi i32 [ %div, %if.end.thread ], [ %dy.addr.020, %if.then ]
  %y.addr.16 = phi i32 [ %shr4, %if.end.thread ], [ %y.addr.018, %if.then ]
  %h.addr.15 = phi i32 [ %shr, %if.end.thread ], [ %h.addr.016, %if.then ]
  %shr7 = ashr i32 %w.addr.015, 1
  %shr8 = ashr i32 %x.addr.017, 1
  %div9 = sdiv i32 %dx.addr.019, 2
  %shr10 = ashr i32 %lx.addr.021, 1
  br label %if.end12

if.end12:                                         ; preds = %if.then, %if.then6, %for.body
  %w.addr.1 = phi i32 [ %shr7, %if.then6 ], [ %w.addr.015, %for.body ], [ %w.addr.015, %if.then ]
  %h.addr.2 = phi i32 [ %h.addr.15, %if.then6 ], [ %h.addr.016, %for.body ], [ %h.addr.016, %if.then ]
  %x.addr.1 = phi i32 [ %shr8, %if.then6 ], [ %x.addr.017, %for.body ], [ %x.addr.017, %if.then ]
  %y.addr.2 = phi i32 [ %y.addr.16, %if.then6 ], [ %y.addr.018, %for.body ], [ %y.addr.018, %if.then ]
  %dx.addr.1 = phi i32 [ %div9, %if.then6 ], [ %dx.addr.019, %for.body ], [ %dx.addr.019, %if.then ]
  %dy.addr.2 = phi i32 [ %dy.addr.17, %if.then6 ], [ %dy.addr.020, %for.body ], [ %dy.addr.020, %if.then ]
  %lx.addr.1 = phi i32 [ %shr10, %if.then6 ], [ %lx.addr.021, %for.body ], [ %lx.addr.021, %if.then ]
  %arrayidx = getelementptr inbounds ptr, ptr %src, i32 %cc.022
  %1 = load ptr, ptr %arrayidx, align 4, !tbaa !12
  %shr13 = ashr i32 %lx.addr.1, 1
  %.shr13 = select i1 %tobool, i32 0, i32 %shr13
  %add.ptr = getelementptr inbounds i8, ptr %1, i32 %.shr13
  %arrayidx14 = getelementptr inbounds ptr, ptr %dst, i32 %cc.022
  %2 = load ptr, ptr %arrayidx14, align 4, !tbaa !12
  %3 = select i1 %tobool15, i32 0, i32 %shr13
  %add.ptr21 = getelementptr inbounds i8, ptr %2, i32 %3
  %shr.i = ashr i32 %dx.addr.1, 1
  %shr1.i = ashr i32 %dy.addr.2, 1
  %and2.i = and i32 %dy.addr.2, 1
  %add.i = add nsw i32 %shr1.i, %y.addr.2
  %mul.i = mul nsw i32 %add.i, %lx.addr.1
  %add.ptr.i = getelementptr inbounds i8, ptr %add.ptr, i32 %mul.i
  %add3.i = add nsw i32 %shr.i, %x.addr.1
  %add.ptr4.i = getelementptr inbounds i8, ptr %add.ptr.i, i32 %add3.i
  %mul5.i = mul nsw i32 %lx.addr.1, %y.addr.2
  %add.ptr6.i = getelementptr inbounds i8, ptr %add.ptr21, i32 %mul5.i
  %add.ptr7.i = getelementptr inbounds i8, ptr %add.ptr6.i, i32 %x.addr.1
  %tobool8.i = icmp ne i32 %and2.i, 0
  %4 = or i32 %dy.addr.2, %dx.addr.1
  %5 = and i32 %4, 1
  %6 = icmp eq i32 %5, 0
  br i1 %6, label %if.then.i, label %if.else44.i

if.then.i:                                        ; preds = %if.end12
  %cmp27429.i = icmp sgt i32 %h.addr.2, 0
  br i1 %tobool9.i, label %for.cond26.preheader.i, label %for.cond.preheader.i

for.cond.preheader.i:                             ; preds = %if.then.i
  br i1 %cmp27429.i, label %for.cond11.preheader.lr.ph.i, label %pred_comp.exit

for.cond11.preheader.lr.ph.i:                     ; preds = %for.cond.preheader.i
  %cmp12433.i = icmp sgt i32 %w.addr.1, 0
  br label %for.cond11.preheader.i

for.cond26.preheader.i:                           ; preds = %if.then.i
  br i1 %cmp27429.i, label %for.cond30.preheader.lr.ph.i, label %pred_comp.exit

for.cond30.preheader.lr.ph.i:                     ; preds = %for.cond26.preheader.i
  %cmp31427.i = icmp sgt i32 %w.addr.1, 0
  br label %for.cond30.preheader.i

for.cond11.preheader.i:                           ; preds = %for.end.i, %for.cond11.preheader.lr.ph.i
  %d.0438.i = phi ptr [ %add.ptr7.i, %for.cond11.preheader.lr.ph.i ], [ %add.ptr22.i, %for.end.i ]
  %s.0437.i = phi ptr [ %add.ptr4.i, %for.cond11.preheader.lr.ph.i ], [ %add.ptr21.i, %for.end.i ]
  %j.0436.i = phi i32 [ 0, %for.cond11.preheader.lr.ph.i ], [ %inc24.i, %for.end.i ]
  br i1 %cmp12433.i, label %for.body13.i.preheader, label %for.end.i

for.body13.i.preheader:                           ; preds = %for.cond11.preheader.i
  br label %for.body13.i

for.body13.i:                                     ; preds = %for.body13.i.preheader, %for.body13.i
  %i.0434.i = phi i32 [ %inc.i, %for.body13.i ], [ 0, %for.body13.i.preheader ]
  %arrayidx.i = getelementptr inbounds i8, ptr %d.0438.i, i32 %i.0434.i
  %7 = load i8, ptr %arrayidx.i, align 1, !tbaa !16
  %conv.i = zext i8 %7 to i32
  %arrayidx14.i = getelementptr inbounds i8, ptr %s.0437.i, i32 %i.0434.i
  %8 = load i8, ptr %arrayidx14.i, align 1, !tbaa !16
  %conv15.i = zext i8 %8 to i32
  %add16.i = add nuw nsw i32 %conv.i, 1
  %add17.i = add nuw nsw i32 %add16.i, %conv15.i
  %shr18.i = lshr i32 %add17.i, 1
  %conv19.i = trunc i32 %shr18.i to i8
  store i8 %conv19.i, ptr %arrayidx.i, align 1, !tbaa !16
  %inc.i = add nuw nsw i32 %i.0434.i, 1
  %exitcond483.i = icmp eq i32 %inc.i, %w.addr.1
  br i1 %exitcond483.i, label %for.end.i.loopexit, label %for.body13.i

for.end.i.loopexit:                               ; preds = %for.body13.i
  br label %for.end.i

for.end.i:                                        ; preds = %for.end.i.loopexit, %for.cond11.preheader.i
  %add.ptr21.i = getelementptr inbounds i8, ptr %s.0437.i, i32 %lx.addr.1
  %add.ptr22.i = getelementptr inbounds i8, ptr %d.0438.i, i32 %lx.addr.1
  %inc24.i = add nuw nsw i32 %j.0436.i, 1
  %exitcond484.i = icmp eq i32 %inc24.i, %h.addr.2
  br i1 %exitcond484.i, label %pred_comp.exit.loopexit37, label %for.cond11.preheader.i

for.cond30.preheader.i:                           ; preds = %for.end38.i, %for.cond30.preheader.lr.ph.i
  %d.1432.i = phi ptr [ %add.ptr7.i, %for.cond30.preheader.lr.ph.i ], [ %add.ptr40.i, %for.end38.i ]
  %s.1431.i = phi ptr [ %add.ptr4.i, %for.cond30.preheader.lr.ph.i ], [ %add.ptr39.i, %for.end38.i ]
  %j.1430.i = phi i32 [ 0, %for.cond30.preheader.lr.ph.i ], [ %inc42.i, %for.end38.i ]
  br i1 %cmp31427.i, label %for.body33.i.preheader, label %for.end38.i

for.body33.i.preheader:                           ; preds = %for.cond30.preheader.i
  br label %for.body33.i

for.body33.i:                                     ; preds = %for.body33.i.preheader, %for.body33.i
  %i.1428.i = phi i32 [ %inc37.i, %for.body33.i ], [ 0, %for.body33.i.preheader ]
  %arrayidx34.i = getelementptr inbounds i8, ptr %s.1431.i, i32 %i.1428.i
  %9 = load i8, ptr %arrayidx34.i, align 1, !tbaa !16
  %arrayidx35.i = getelementptr inbounds i8, ptr %d.1432.i, i32 %i.1428.i
  store i8 %9, ptr %arrayidx35.i, align 1, !tbaa !16
  %inc37.i = add nuw nsw i32 %i.1428.i, 1
  %exitcond.i = icmp eq i32 %inc37.i, %w.addr.1
  br i1 %exitcond.i, label %for.end38.i.loopexit, label %for.body33.i

for.end38.i.loopexit:                             ; preds = %for.body33.i
  br label %for.end38.i

for.end38.i:                                      ; preds = %for.end38.i.loopexit, %for.cond30.preheader.i
  %add.ptr39.i = getelementptr inbounds i8, ptr %s.1431.i, i32 %lx.addr.1
  %add.ptr40.i = getelementptr inbounds i8, ptr %d.1432.i, i32 %lx.addr.1
  %inc42.i = add nuw nsw i32 %j.1430.i, 1
  %exitcond482.i = icmp eq i32 %inc42.i, %h.addr.2
  br i1 %exitcond482.i, label %pred_comp.exit.loopexit, label %for.cond30.preheader.i

if.else44.i:                                      ; preds = %if.end12
  %and.i = and i32 %dx.addr.1, 1
  %tobool45.i = icmp eq i32 %and.i, 0
  %or.cond259.i = and i1 %tobool45.i, %tobool8.i
  br i1 %or.cond259.i, label %if.then48.i, label %if.else110.i

if.then48.i:                                      ; preds = %if.else44.i
  %cmp84441.i = icmp sgt i32 %h.addr.2, 0
  br i1 %tobool9.i, label %for.cond83.preheader.i, label %for.cond51.preheader.i

for.cond51.preheader.i:                           ; preds = %if.then48.i
  br i1 %cmp84441.i, label %for.cond55.preheader.lr.ph.i, label %pred_comp.exit

for.cond55.preheader.lr.ph.i:                     ; preds = %for.cond51.preheader.i
  %cmp56445.i = icmp sgt i32 %w.addr.1, 0
  br label %for.cond55.preheader.i

for.cond83.preheader.i:                           ; preds = %if.then48.i
  br i1 %cmp84441.i, label %for.cond87.preheader.lr.ph.i, label %pred_comp.exit

for.cond87.preheader.lr.ph.i:                     ; preds = %for.cond83.preheader.i
  %cmp88439.i = icmp sgt i32 %w.addr.1, 0
  br label %for.cond87.preheader.i

for.cond55.preheader.i:                           ; preds = %for.end76.i, %for.cond55.preheader.lr.ph.i
  %d.2450.i = phi ptr [ %add.ptr7.i, %for.cond55.preheader.lr.ph.i ], [ %add.ptr78.i, %for.end76.i ]
  %s.2449.i = phi ptr [ %add.ptr4.i, %for.cond55.preheader.lr.ph.i ], [ %add.ptr77.i, %for.end76.i ]
  %j.2448.i = phi i32 [ 0, %for.cond55.preheader.lr.ph.i ], [ %inc80.i, %for.end76.i ]
  br i1 %cmp56445.i, label %for.body58.i.preheader, label %for.end76.i

for.body58.i.preheader:                           ; preds = %for.cond55.preheader.i
  br label %for.body58.i

for.body58.i:                                     ; preds = %for.body58.i.preheader, %for.body58.i
  %i.2446.i = phi i32 [ %inc75.i, %for.body58.i ], [ 0, %for.body58.i.preheader ]
  %arrayidx59.i = getelementptr inbounds i8, ptr %d.2450.i, i32 %i.2446.i
  %10 = load i8, ptr %arrayidx59.i, align 1, !tbaa !16
  %conv60.i = zext i8 %10 to i32
  %arrayidx61.i = getelementptr inbounds i8, ptr %s.2449.i, i32 %i.2446.i
  %11 = load i8, ptr %arrayidx61.i, align 1, !tbaa !16
  %conv62.i = zext i8 %11 to i32
  %add63.i = add nsw i32 %i.2446.i, %lx.addr.1
  %arrayidx64.i = getelementptr inbounds i8, ptr %s.2449.i, i32 %add63.i
  %12 = load i8, ptr %arrayidx64.i, align 1, !tbaa !16
  %conv65.i = zext i8 %12 to i32
  %add66.i = add nuw nsw i32 %conv62.i, 1
  %add67.i = add nuw nsw i32 %add66.i, %conv65.i
  %shr68.i = lshr i32 %add67.i, 1
  %add69.i = add nuw nsw i32 %conv60.i, 1
  %add70.i = add nuw nsw i32 %add69.i, %shr68.i
  %shr71.i = lshr i32 %add70.i, 1
  %conv72.i = trunc i32 %shr71.i to i8
  store i8 %conv72.i, ptr %arrayidx59.i, align 1, !tbaa !16
  %inc75.i = add nuw nsw i32 %i.2446.i, 1
  %exitcond487.i = icmp eq i32 %inc75.i, %w.addr.1
  br i1 %exitcond487.i, label %for.end76.i.loopexit, label %for.body58.i

for.end76.i.loopexit:                             ; preds = %for.body58.i
  br label %for.end76.i

for.end76.i:                                      ; preds = %for.end76.i.loopexit, %for.cond55.preheader.i
  %add.ptr77.i = getelementptr inbounds i8, ptr %s.2449.i, i32 %lx.addr.1
  %add.ptr78.i = getelementptr inbounds i8, ptr %d.2450.i, i32 %lx.addr.1
  %inc80.i = add nuw nsw i32 %j.2448.i, 1
  %exitcond488.i = icmp eq i32 %inc80.i, %h.addr.2
  br i1 %exitcond488.i, label %pred_comp.exit.loopexit39, label %for.cond55.preheader.i

for.cond87.preheader.i:                           ; preds = %for.end103.i, %for.cond87.preheader.lr.ph.i
  %d.3444.i = phi ptr [ %add.ptr7.i, %for.cond87.preheader.lr.ph.i ], [ %add.ptr105.i, %for.end103.i ]
  %s.3443.i = phi ptr [ %add.ptr4.i, %for.cond87.preheader.lr.ph.i ], [ %add.ptr104.i, %for.end103.i ]
  %j.3442.i = phi i32 [ 0, %for.cond87.preheader.lr.ph.i ], [ %inc107.i, %for.end103.i ]
  br i1 %cmp88439.i, label %for.body90.i.preheader, label %for.end103.i

for.body90.i.preheader:                           ; preds = %for.cond87.preheader.i
  br label %for.body90.i

for.body90.i:                                     ; preds = %for.body90.i.preheader, %for.body90.i
  %i.3440.i = phi i32 [ %inc102.i, %for.body90.i ], [ 0, %for.body90.i.preheader ]
  %arrayidx91.i = getelementptr inbounds i8, ptr %s.3443.i, i32 %i.3440.i
  %13 = load i8, ptr %arrayidx91.i, align 1, !tbaa !16
  %conv92.i = zext i8 %13 to i32
  %add93.i = add nsw i32 %i.3440.i, %lx.addr.1
  %arrayidx94.i = getelementptr inbounds i8, ptr %s.3443.i, i32 %add93.i
  %14 = load i8, ptr %arrayidx94.i, align 1, !tbaa !16
  %conv95.i = zext i8 %14 to i32
  %add96.i = add nuw nsw i32 %conv92.i, 1
  %add97.i = add nuw nsw i32 %add96.i, %conv95.i
  %shr98.i = lshr i32 %add97.i, 1
  %conv99.i = trunc i32 %shr98.i to i8
  %arrayidx100.i = getelementptr inbounds i8, ptr %d.3444.i, i32 %i.3440.i
  store i8 %conv99.i, ptr %arrayidx100.i, align 1, !tbaa !16
  %inc102.i = add nuw nsw i32 %i.3440.i, 1
  %exitcond485.i = icmp eq i32 %inc102.i, %w.addr.1
  br i1 %exitcond485.i, label %for.end103.i.loopexit, label %for.body90.i

for.end103.i.loopexit:                            ; preds = %for.body90.i
  br label %for.end103.i

for.end103.i:                                     ; preds = %for.end103.i.loopexit, %for.cond87.preheader.i
  %add.ptr104.i = getelementptr inbounds i8, ptr %s.3443.i, i32 %lx.addr.1
  %add.ptr105.i = getelementptr inbounds i8, ptr %d.3444.i, i32 %lx.addr.1
  %inc107.i = add nuw nsw i32 %j.3442.i, 1
  %exitcond486.i = icmp eq i32 %inc107.i, %h.addr.2
  br i1 %exitcond486.i, label %pred_comp.exit.loopexit38, label %for.cond87.preheader.i

if.else110.i:                                     ; preds = %if.else44.i
  %or.cond260.i = or i1 %tobool45.i, %tobool8.i
  %cmp180453.i = icmp sgt i32 %h.addr.2, 0
  br i1 %or.cond260.i, label %if.else176.i, label %if.then114.i

if.then114.i:                                     ; preds = %if.else110.i
  br i1 %tobool177.i, label %for.cond117.preheader.i, label %for.cond149.preheader.i

for.cond149.preheader.i:                          ; preds = %if.then114.i
  br i1 %cmp180453.i, label %for.cond153.preheader.lr.ph.i, label %pred_comp.exit

for.cond153.preheader.lr.ph.i:                    ; preds = %for.cond149.preheader.i
  %cmp154469.i = icmp sgt i32 %w.addr.1, 0
  br label %for.cond153.preheader.i

for.cond117.preheader.i:                          ; preds = %if.then114.i
  br i1 %cmp180453.i, label %for.cond121.preheader.lr.ph.i, label %pred_comp.exit

for.cond121.preheader.lr.ph.i:                    ; preds = %for.cond117.preheader.i
  %cmp122463.i = icmp sgt i32 %w.addr.1, 0
  br label %for.cond121.preheader.i

for.cond121.preheader.i:                          ; preds = %for.end142.i, %for.cond121.preheader.lr.ph.i
  %d.4468.i = phi ptr [ %add.ptr7.i, %for.cond121.preheader.lr.ph.i ], [ %add.ptr144.i, %for.end142.i ]
  %s.4467.i = phi ptr [ %add.ptr4.i, %for.cond121.preheader.lr.ph.i ], [ %add.ptr143.i, %for.end142.i ]
  %j.4466.i = phi i32 [ 0, %for.cond121.preheader.lr.ph.i ], [ %inc146.i, %for.end142.i ]
  br i1 %cmp122463.i, label %for.body124.i.preheader, label %for.end142.i

for.body124.i.preheader:                          ; preds = %for.cond121.preheader.i
  br label %for.body124.i

for.body124.i:                                    ; preds = %for.body124.i.preheader, %for.body124.i
  %i.4464.i = phi i32 [ %add129.i, %for.body124.i ], [ 0, %for.body124.i.preheader ]
  %arrayidx125.i = getelementptr inbounds i8, ptr %d.4468.i, i32 %i.4464.i
  %15 = load i8, ptr %arrayidx125.i, align 1, !tbaa !16
  %conv126.i = zext i8 %15 to i32
  %arrayidx127.i = getelementptr inbounds i8, ptr %s.4467.i, i32 %i.4464.i
  %16 = load i8, ptr %arrayidx127.i, align 1, !tbaa !16
  %conv128.i = zext i8 %16 to i32
  %add129.i = add nuw nsw i32 %i.4464.i, 1
  %arrayidx130.i = getelementptr inbounds i8, ptr %s.4467.i, i32 %add129.i
  %17 = load i8, ptr %arrayidx130.i, align 1, !tbaa !16
  %conv131.i = zext i8 %17 to i32
  %add132.i = add nuw nsw i32 %conv128.i, 1
  %add133.i = add nuw nsw i32 %add132.i, %conv131.i
  %shr134.i = lshr i32 %add133.i, 1
  %add135.i = add nuw nsw i32 %conv126.i, 1
  %add136.i = add nuw nsw i32 %add135.i, %shr134.i
  %shr137.i = lshr i32 %add136.i, 1
  %conv138.i = trunc i32 %shr137.i to i8
  store i8 %conv138.i, ptr %arrayidx125.i, align 1, !tbaa !16
  %exitcond493.i = icmp eq i32 %add129.i, %w.addr.1
  br i1 %exitcond493.i, label %for.end142.i.loopexit, label %for.body124.i

for.end142.i.loopexit:                            ; preds = %for.body124.i
  br label %for.end142.i

for.end142.i:                                     ; preds = %for.end142.i.loopexit, %for.cond121.preheader.i
  %add.ptr143.i = getelementptr inbounds i8, ptr %s.4467.i, i32 %lx.addr.1
  %add.ptr144.i = getelementptr inbounds i8, ptr %d.4468.i, i32 %lx.addr.1
  %inc146.i = add nuw nsw i32 %j.4466.i, 1
  %exitcond494.i = icmp eq i32 %inc146.i, %h.addr.2
  br i1 %exitcond494.i, label %pred_comp.exit.loopexit42, label %for.cond121.preheader.i

for.cond153.preheader.i:                          ; preds = %for.end169.i, %for.cond153.preheader.lr.ph.i
  %d.5474.i = phi ptr [ %add.ptr7.i, %for.cond153.preheader.lr.ph.i ], [ %add.ptr171.i, %for.end169.i ]
  %s.5473.i = phi ptr [ %add.ptr4.i, %for.cond153.preheader.lr.ph.i ], [ %add.ptr170.i, %for.end169.i ]
  %j.5472.i = phi i32 [ 0, %for.cond153.preheader.lr.ph.i ], [ %inc173.i, %for.end169.i ]
  br i1 %cmp154469.i, label %for.body156.i.preheader, label %for.end169.i

for.body156.i.preheader:                          ; preds = %for.cond153.preheader.i
  br label %for.body156.i

for.body156.i:                                    ; preds = %for.body156.i.preheader, %for.body156.i
  %i.5470.i = phi i32 [ %add159.i, %for.body156.i ], [ 0, %for.body156.i.preheader ]
  %arrayidx157.i = getelementptr inbounds i8, ptr %s.5473.i, i32 %i.5470.i
  %18 = load i8, ptr %arrayidx157.i, align 1, !tbaa !16
  %conv158.i = zext i8 %18 to i32
  %add159.i = add nuw nsw i32 %i.5470.i, 1
  %arrayidx160.i = getelementptr inbounds i8, ptr %s.5473.i, i32 %add159.i
  %19 = load i8, ptr %arrayidx160.i, align 1, !tbaa !16
  %conv161.i = zext i8 %19 to i32
  %add162.i = add nuw nsw i32 %conv158.i, 1
  %add163.i = add nuw nsw i32 %add162.i, %conv161.i
  %shr164.i = lshr i32 %add163.i, 1
  %conv165.i = trunc i32 %shr164.i to i8
  %arrayidx166.i = getelementptr inbounds i8, ptr %d.5474.i, i32 %i.5470.i
  store i8 %conv165.i, ptr %arrayidx166.i, align 1, !tbaa !16
  %exitcond495.i = icmp eq i32 %add159.i, %w.addr.1
  br i1 %exitcond495.i, label %for.end169.i.loopexit, label %for.body156.i

for.end169.i.loopexit:                            ; preds = %for.body156.i
  br label %for.end169.i

for.end169.i:                                     ; preds = %for.end169.i.loopexit, %for.cond153.preheader.i
  %add.ptr170.i = getelementptr inbounds i8, ptr %s.5473.i, i32 %lx.addr.1
  %add.ptr171.i = getelementptr inbounds i8, ptr %d.5474.i, i32 %lx.addr.1
  %inc173.i = add nuw nsw i32 %j.5472.i, 1
  %exitcond496.i = icmp eq i32 %inc173.i, %h.addr.2
  br i1 %exitcond496.i, label %pred_comp.exit.loopexit43, label %for.cond153.preheader.i

if.else176.i:                                     ; preds = %if.else110.i
  br i1 %tobool177.i, label %for.cond179.preheader.i, label %for.cond220.preheader.i

for.cond220.preheader.i:                          ; preds = %if.else176.i
  br i1 %cmp180453.i, label %for.cond224.preheader.lr.ph.i, label %pred_comp.exit

for.cond224.preheader.lr.ph.i:                    ; preds = %for.cond220.preheader.i
  %cmp225457.i = icmp sgt i32 %w.addr.1, 0
  br label %for.cond224.preheader.i

for.cond179.preheader.i:                          ; preds = %if.else176.i
  br i1 %cmp180453.i, label %for.cond183.preheader.lr.ph.i, label %pred_comp.exit

for.cond183.preheader.lr.ph.i:                    ; preds = %for.cond179.preheader.i
  %cmp184451.i = icmp sgt i32 %w.addr.1, 0
  br label %for.cond183.preheader.i

for.cond183.preheader.i:                          ; preds = %for.end213.i, %for.cond183.preheader.lr.ph.i
  %d.6456.i = phi ptr [ %add.ptr7.i, %for.cond183.preheader.lr.ph.i ], [ %add.ptr215.i, %for.end213.i ]
  %s.6455.i = phi ptr [ %add.ptr4.i, %for.cond183.preheader.lr.ph.i ], [ %add.ptr214.i, %for.end213.i ]
  %j.6454.i = phi i32 [ 0, %for.cond183.preheader.lr.ph.i ], [ %inc217.i, %for.end213.i ]
  br i1 %cmp184451.i, label %for.body186.i.preheader, label %for.end213.i

for.body186.i.preheader:                          ; preds = %for.cond183.preheader.i
  br label %for.body186.i

for.body186.i:                                    ; preds = %for.body186.i.preheader, %for.body186.i
  %i.6452.i = phi i32 [ %add191.i, %for.body186.i ], [ 0, %for.body186.i.preheader ]
  %arrayidx187.i = getelementptr inbounds i8, ptr %d.6456.i, i32 %i.6452.i
  %20 = load i8, ptr %arrayidx187.i, align 1, !tbaa !16
  %conv188.i = zext i8 %20 to i32
  %arrayidx189.i = getelementptr inbounds i8, ptr %s.6455.i, i32 %i.6452.i
  %21 = load i8, ptr %arrayidx189.i, align 1, !tbaa !16
  %conv190.i = zext i8 %21 to i32
  %add191.i = add nuw nsw i32 %i.6452.i, 1
  %arrayidx192.i = getelementptr inbounds i8, ptr %s.6455.i, i32 %add191.i
  %22 = load i8, ptr %arrayidx192.i, align 1, !tbaa !16
  %conv193.i = zext i8 %22 to i32
  %add195.i = add nsw i32 %i.6452.i, %lx.addr.1
  %arrayidx196.i = getelementptr inbounds i8, ptr %s.6455.i, i32 %add195.i
  %23 = load i8, ptr %arrayidx196.i, align 1, !tbaa !16
  %conv197.i = zext i8 %23 to i32
  %add200.i = add nsw i32 %add195.i, 1
  %arrayidx201.i = getelementptr inbounds i8, ptr %s.6455.i, i32 %add200.i
  %24 = load i8, ptr %arrayidx201.i, align 1, !tbaa !16
  %conv202.i = zext i8 %24 to i32
  %add194.i = add nuw nsw i32 %conv190.i, 2
  %add198.i = add nuw nsw i32 %add194.i, %conv193.i
  %add203.i = add nuw nsw i32 %add198.i, %conv197.i
  %add204.i = add nuw nsw i32 %add203.i, %conv202.i
  %shr205.i = lshr i32 %add204.i, 2
  %add206.i = add nuw nsw i32 %conv188.i, 1
  %add207.i = add nuw nsw i32 %add206.i, %shr205.i
  %shr208.i = lshr i32 %add207.i, 1
  %conv209.i = trunc i32 %shr208.i to i8
  store i8 %conv209.i, ptr %arrayidx187.i, align 1, !tbaa !16
  %exitcond489.i = icmp eq i32 %add191.i, %w.addr.1
  br i1 %exitcond489.i, label %for.end213.i.loopexit, label %for.body186.i

for.end213.i.loopexit:                            ; preds = %for.body186.i
  br label %for.end213.i

for.end213.i:                                     ; preds = %for.end213.i.loopexit, %for.cond183.preheader.i
  %add.ptr214.i = getelementptr inbounds i8, ptr %s.6455.i, i32 %lx.addr.1
  %add.ptr215.i = getelementptr inbounds i8, ptr %d.6456.i, i32 %lx.addr.1
  %inc217.i = add nuw nsw i32 %j.6454.i, 1
  %exitcond490.i = icmp eq i32 %inc217.i, %h.addr.2
  br i1 %exitcond490.i, label %pred_comp.exit.loopexit40, label %for.cond183.preheader.i

for.cond224.preheader.i:                          ; preds = %for.end249.i, %for.cond224.preheader.lr.ph.i
  %d.7462.i = phi ptr [ %add.ptr7.i, %for.cond224.preheader.lr.ph.i ], [ %add.ptr251.i, %for.end249.i ]
  %s.7461.i = phi ptr [ %add.ptr4.i, %for.cond224.preheader.lr.ph.i ], [ %add.ptr250.i, %for.end249.i ]
  %j.7460.i = phi i32 [ 0, %for.cond224.preheader.lr.ph.i ], [ %inc253.i, %for.end249.i ]
  br i1 %cmp225457.i, label %for.body227.i.preheader, label %for.end249.i

for.body227.i.preheader:                          ; preds = %for.cond224.preheader.i
  br label %for.body227.i

for.body227.i:                                    ; preds = %for.body227.i.preheader, %for.body227.i
  %i.7458.i = phi i32 [ %add230.i, %for.body227.i ], [ 0, %for.body227.i.preheader ]
  %arrayidx228.i = getelementptr inbounds i8, ptr %s.7461.i, i32 %i.7458.i
  %25 = load i8, ptr %arrayidx228.i, align 1, !tbaa !16
  %conv229.i = zext i8 %25 to i32
  %add230.i = add nuw nsw i32 %i.7458.i, 1
  %arrayidx231.i = getelementptr inbounds i8, ptr %s.7461.i, i32 %add230.i
  %26 = load i8, ptr %arrayidx231.i, align 1, !tbaa !16
  %conv232.i = zext i8 %26 to i32
  %add234.i = add nsw i32 %i.7458.i, %lx.addr.1
  %arrayidx235.i = getelementptr inbounds i8, ptr %s.7461.i, i32 %add234.i
  %27 = load i8, ptr %arrayidx235.i, align 1, !tbaa !16
  %conv236.i = zext i8 %27 to i32
  %add239.i = add nsw i32 %add234.i, 1
  %arrayidx240.i = getelementptr inbounds i8, ptr %s.7461.i, i32 %add239.i
  %28 = load i8, ptr %arrayidx240.i, align 1, !tbaa !16
  %conv241.i = zext i8 %28 to i32
  %add233.i = add nuw nsw i32 %conv229.i, 2
  %add237.i = add nuw nsw i32 %add233.i, %conv232.i
  %add242.i = add nuw nsw i32 %add237.i, %conv236.i
  %add243.i = add nuw nsw i32 %add242.i, %conv241.i
  %shr244.i = lshr i32 %add243.i, 2
  %conv245.i = trunc i32 %shr244.i to i8
  %arrayidx246.i = getelementptr inbounds i8, ptr %d.7462.i, i32 %i.7458.i
  store i8 %conv245.i, ptr %arrayidx246.i, align 1, !tbaa !16
  %exitcond491.i = icmp eq i32 %add230.i, %w.addr.1
  br i1 %exitcond491.i, label %for.end249.i.loopexit, label %for.body227.i

for.end249.i.loopexit:                            ; preds = %for.body227.i
  br label %for.end249.i

for.end249.i:                                     ; preds = %for.end249.i.loopexit, %for.cond224.preheader.i
  %add.ptr250.i = getelementptr inbounds i8, ptr %s.7461.i, i32 %lx.addr.1
  %add.ptr251.i = getelementptr inbounds i8, ptr %d.7462.i, i32 %lx.addr.1
  %inc253.i = add nuw nsw i32 %j.7460.i, 1
  %exitcond492.i = icmp eq i32 %inc253.i, %h.addr.2
  br i1 %exitcond492.i, label %pred_comp.exit.loopexit41, label %for.cond224.preheader.i

pred_comp.exit.loopexit:                          ; preds = %for.end38.i
  br label %pred_comp.exit

pred_comp.exit.loopexit37:                        ; preds = %for.end.i
  br label %pred_comp.exit

pred_comp.exit.loopexit38:                        ; preds = %for.end103.i
  br label %pred_comp.exit

pred_comp.exit.loopexit39:                        ; preds = %for.end76.i
  br label %pred_comp.exit

pred_comp.exit.loopexit40:                        ; preds = %for.end213.i
  br label %pred_comp.exit

pred_comp.exit.loopexit41:                        ; preds = %for.end249.i
  br label %pred_comp.exit

pred_comp.exit.loopexit42:                        ; preds = %for.end142.i
  br label %pred_comp.exit

pred_comp.exit.loopexit43:                        ; preds = %for.end169.i
  br label %pred_comp.exit

pred_comp.exit:                                   ; preds = %pred_comp.exit.loopexit43, %pred_comp.exit.loopexit42, %pred_comp.exit.loopexit41, %pred_comp.exit.loopexit40, %pred_comp.exit.loopexit39, %pred_comp.exit.loopexit38, %pred_comp.exit.loopexit37, %pred_comp.exit.loopexit, %for.cond.preheader.i, %for.cond26.preheader.i, %for.cond51.preheader.i, %for.cond83.preheader.i, %for.cond149.preheader.i, %for.cond117.preheader.i, %for.cond220.preheader.i, %for.cond179.preheader.i
  %inc = add nuw nsw i32 %cc.022, 1
  %exitcond = icmp eq i32 %inc, 3
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %pred_comp.exit
  ret void
}

attributes #0 = { nounwind "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="pentium4" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { norecurse nounwind "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="pentium4" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="pentium4" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (trunk 17978) (llvm/branches/loopopt 20388)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
!5 = !{!6, !2, i64 0}
!6 = !{!"struct@mbinfo", !2, i64 0, !2, i64 4, !2, i64 8, !2, i64 12, !2, i64 16, !2, i64 20, !7, i64 24, !8, i64 56, !9, i64 72, !10, i64 80, !2, i64 88}
!7 = !{!"array@_ZTSA2_A2_A2_i", !8, i64 0}
!8 = !{!"array@_ZTSA2_A2_i", !9, i64 0}
!9 = !{!"array@_ZTSA2_i", !2, i64 0}
!10 = !{!"double", !3, i64 0}
!11 = !{!6, !2, i64 4}
!12 = !{!13, !13, i64 0}
!13 = !{!"pointer@_ZTSPh", !3, i64 0}
!14 = !{!8, !2, i64 0}
!15 = !{!9, !2, i64 0}
!16 = !{!3, !3, i64 0}
