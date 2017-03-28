; RUN: opt < %s -hir-ssa-deconstruction -hir-temp-cleanup -print-after=hir-temp-cleanup -hir-cg -force-hir-cg -S 2>&1 | FileCheck %s

; Verify that CG can correctly handle opaque types. %3 is a pointer to an opaque type. Make sure &((%3)[0]) is lowered correctly to a pointer without a GEP.

; CHECK: + DO i1 = 0, 1, 1   <DO_LOOP>
; CHECK: |   %n.0.i13.out1 = %n.0.i13;
; CHECK: |   %2 = (%call)[0].2;
; CHECK: |   %3 = (%2)[i1];
; CHECK: |   if (&((%3)[0]) != null)
; CHECK: |   {
; CHECK: |      %n.0.i13 = %n.0.i13  +  1;
; CHECK: |      (%2)[%n.0.i13.out1] = &((%3)[0]);
; CHECK: |   }
; CHECK: + END LOOP

; CHECK: region.0:

; Check the first load to opaque type which is used to store to %3.
; CHECK: [[OPAQ_TY_LD1:%.*]] = load %struct.stickline_t*,
; CHECK-NEXT: store %struct.stickline_t* [[OPAQ_TY_LD1]]

; CHECK: [[OPAQ_TY_LD2:%.*]] = load %struct.stickline_t*,
; CHECK: icmp ne %struct.stickline_t* [[OPAQ_TY_LD2]]


target datalayout = "e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-unknown-linux-gnu"

%struct.sticklineset_t = type { i32, i32, %struct.stickline_t** }
%struct.stickline_t = type opaque

define %struct.sticklineset_t* @sticklineset_test(%struct.sticklineset_t* %call) local_unnamed_addr #3 {
entry:
  %tobool = icmp eq %struct.sticklineset_t* %call, null
  br i1 %tobool, label %cleanup, label %for.body.i.lr.ph

for.body.i.lr.ph:                                 ; preds = %entry
  %stickline = getelementptr inbounds %struct.sticklineset_t, %struct.sticklineset_t* %call, i32 0, i32 2
  %0 = load %struct.stickline_t**, %struct.stickline_t*** %stickline, align 4
  store %struct.stickline_t* null, %struct.stickline_t** %0, align 4
  %1 = load %struct.stickline_t**, %struct.stickline_t*** %stickline, align 4
  %arrayidx2 = getelementptr inbounds %struct.stickline_t*, %struct.stickline_t** %1, i32 1
  store %struct.stickline_t* inttoptr (i32 77 to %struct.stickline_t*), %struct.stickline_t** %arrayidx2, align 4
  %n = getelementptr inbounds %struct.sticklineset_t, %struct.sticklineset_t* %call, i32 0, i32 0
  store i32 2, i32* %n, align 4
  br label %for.body.i

for.body.i:                                       ; preds = %for.body.i.lr.ph, %if.end.i
  %k.0.i14 = phi i32 [ 0, %for.body.i.lr.ph ], [ %inc7.i, %if.end.i ]
  %n.0.i13 = phi i32 [ 0, %for.body.i.lr.ph ], [ %n.1.i, %if.end.i ]
  %2 = load %struct.stickline_t**, %struct.stickline_t*** %stickline, align 4
  %arrayidx.i = getelementptr inbounds %struct.stickline_t*, %struct.stickline_t** %2, i32 %k.0.i14
  %3 = load %struct.stickline_t*, %struct.stickline_t** %arrayidx.i, align 4
  %tobool3.i = icmp eq %struct.stickline_t* %3, null
  br i1 %tobool3.i, label %if.end.i, label %if.then4.i

if.then4.i:                                       ; preds = %for.body.i
  %inc.i = add nsw i32 %n.0.i13, 1
  %arrayidx6.i = getelementptr inbounds %struct.stickline_t*, %struct.stickline_t** %2, i32 %n.0.i13
  store %struct.stickline_t* %3, %struct.stickline_t** %arrayidx6.i, align 4
  br label %if.end.i

if.end.i:                                         ; preds = %if.then4.i, %for.body.i
  %n.1.i = phi i32 [ %inc.i, %if.then4.i ], [ %n.0.i13, %for.body.i ]
  %inc7.i = add nuw nsw i32 %k.0.i14, 1
  %cmp.i = icmp slt i32 %inc7.i, 2
  br i1 %cmp.i, label %for.body.i, label %sticklineset_compress.exit

sticklineset_compress.exit:                       ; preds = %if.end.i
  %n.1.i.lcssa = phi i32 [ %n.1.i, %if.end.i ]
  store i32 %n.1.i.lcssa, i32* %n, align 4
  br label %cleanup

cleanup:                                          ; preds = %entry, %sticklineset_compress.exit
  %retval.0 = phi %struct.sticklineset_t* [ %call, %sticklineset_compress.exit ], [ null, %entry ]
  ret %struct.sticklineset_t* %retval.0
}

