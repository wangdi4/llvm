; ModuleID = 'Program'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S32"
target triple = "i686-pc-win32"

%struct._image2d_t.0 = type opaque

define void @wlSimpleBoxBlur_GPU(<4 x float> addrspace(1)* nocapture %input, <4 x float> addrspace(1)* nocapture %output, i32 %width, i32 %height, i32 %buffer_size) nounwind {
entry:
  %call1 = tail call i32 @get_global_id(i32 0) nounwind readnone
  %call2 = tail call i32 @get_global_id(i32 1) nounwind readnone
  %call5 = tail call i32 @get_global_size(i32 0) nounwind readnone
  %call6 = tail call i32 @get_global_size(i32 1) nounwind readnone
  %mul = mul i32 %call2, %width
  %add = add i32 %mul, %call1
  %sub = add i32 %call2, -1
  %mul11 = mul i32 %sub, %width
  %add12 = add i32 %mul11, %call1
  %add13 = add i32 %call2, 1
  %mul14 = mul i32 %add13, %width
  %add15 = add i32 %mul14, %call1
  %arrayidx = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %add
  %0 = load <4 x float> addrspace(1)* %arrayidx, align 16
  %cmp = icmp eq i32 %call1, 0
  br i1 %cmp, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  %sub16 = add i32 %add, -1
  %arrayidx17 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %sub16
  %1 = load <4 x float> addrspace(1)* %arrayidx17, align 16
  %add18 = fadd <4 x float> %0, %1
  br label %if.end

if.end:                                           ; preds = %entry, %if.then
  %colorAccumulator.0 = phi <4 x float> [ %add18, %if.then ], [ %0, %entry ]
  %sub19 = add i32 %call5, -1
  %cmp20 = icmp ult i32 %call1, %sub19
  br i1 %cmp20, label %if.then21, label %if.end25

if.then21:                                        ; preds = %if.end
  %add22 = add i32 %add, 1
  %arrayidx23 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %add22
  %2 = load <4 x float> addrspace(1)* %arrayidx23, align 16
  %add24 = fadd <4 x float> %colorAccumulator.0, %2
  br label %if.end25

if.end25:                                         ; preds = %if.then21, %if.end
  %colorAccumulator.1 = phi <4 x float> [ %add24, %if.then21 ], [ %colorAccumulator.0, %if.end ]
  %cmp26 = icmp eq i32 %call2, 0
  br i1 %cmp26, label %if.end43, label %if.then27

if.then27:                                        ; preds = %if.end25
  %arrayidx28 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %add12
  %3 = load <4 x float> addrspace(1)* %arrayidx28, align 16
  %add29 = fadd <4 x float> %colorAccumulator.1, %3
  br i1 %cmp, label %if.end35, label %if.then31

if.then31:                                        ; preds = %if.then27
  %sub32 = add i32 %add12, -1
  %arrayidx33 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %sub32
  %4 = load <4 x float> addrspace(1)* %arrayidx33, align 16
  %add34 = fadd <4 x float> %add29, %4
  br label %if.end35

if.end35:                                         ; preds = %if.then27, %if.then31
  %colorAccumulator.2 = phi <4 x float> [ %add34, %if.then31 ], [ %add29, %if.then27 ]
  br i1 %cmp20, label %if.then38, label %if.end43

if.then38:                                        ; preds = %if.end35
  %add39 = add i32 %add12, 1
  %arrayidx40 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %add39
  %5 = load <4 x float> addrspace(1)* %arrayidx40, align 16
  %add41 = fadd <4 x float> %colorAccumulator.2, %5
  br label %if.end43

if.end43:                                         ; preds = %if.end25, %if.end35, %if.then38
  %colorAccumulator.3 = phi <4 x float> [ %add41, %if.then38 ], [ %colorAccumulator.2, %if.end35 ], [ %colorAccumulator.1, %if.end25 ]
  %sub44 = add i32 %call6, -1
  %cmp45 = icmp ult i32 %call2, %sub44
  br i1 %cmp45, label %if.then46, label %if.end62

if.then46:                                        ; preds = %if.end43
  %arrayidx47 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %add15
  %6 = load <4 x float> addrspace(1)* %arrayidx47, align 16
  %add48 = fadd <4 x float> %colorAccumulator.3, %6
  br i1 %cmp, label %if.end54, label %if.then50

if.then50:                                        ; preds = %if.then46
  %sub51 = add i32 %add15, -1
  %arrayidx52 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %sub51
  %7 = load <4 x float> addrspace(1)* %arrayidx52, align 16
  %add53 = fadd <4 x float> %add48, %7
  br label %if.end54

if.end54:                                         ; preds = %if.then46, %if.then50
  %colorAccumulator.4 = phi <4 x float> [ %add53, %if.then50 ], [ %add48, %if.then46 ]
  br i1 %cmp20, label %if.then57, label %if.end62

if.then57:                                        ; preds = %if.end54
  %add58 = add i32 %add15, 1
  %arrayidx59 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %add58
  %8 = load <4 x float> addrspace(1)* %arrayidx59, align 16
  %add60 = fadd <4 x float> %colorAccumulator.4, %8
  br label %if.end62

if.end62:                                         ; preds = %if.end54, %if.then57, %if.end43
  %colorAccumulator.5 = phi <4 x float> [ %add60, %if.then57 ], [ %colorAccumulator.4, %if.end54 ], [ %colorAccumulator.3, %if.end43 ]
  %div = fdiv <4 x float> %colorAccumulator.5, <float 9.000000e+00, float 9.000000e+00, float 9.000000e+00, float 9.000000e+00>
  %arrayidx63 = getelementptr inbounds <4 x float> addrspace(1)* %output, i32 %add
  store <4 x float> %div, <4 x float> addrspace(1)* %arrayidx63, align 16
  ret void
}

declare i32 @get_global_id(i32) nounwind readnone

declare i32 @get_global_size(i32) nounwind readnone

define void @wlSimpleBoxBlur_CPU(<4 x float> addrspace(1)* nocapture %input, <4 x float> addrspace(1)* nocapture %output, i32 %width, i32 %height, i32 %buffer_size) nounwind {
entry:
  %call1 = tail call i32 @get_global_id(i32 0) nounwind readnone
  %call5 = tail call i32 @get_global_size(i32 0) nounwind readnone
  %call6 = tail call i32 @get_global_size(i32 1) nounwind readnone
  %div = udiv i32 %width, %call5
  %div11 = udiv i32 %height, %call6
  %mul = mul i32 %call1, %width
  %div12 = udiv i32 %mul, %call5
  %call2 = tail call i32 @get_global_id(i32 1) nounwind readnone
  %mul13 = mul i32 %call2, %height
  %div14 = udiv i32 %mul13, %call6
  %cmp171 = icmp eq i32 %div, 0
  %sub29 = add i32 %width, -1
  %sub54 = add i32 %height, -1
  br label %for.cond16.preheader

for.cond16.preheader:                             ; preds = %entry, %for.inc76
  %i.06 = phi i32 [ 0, %entry ], [ %inc77, %for.inc76 ]
  %index_y.05 = phi i32 [ %div14, %entry ], [ %inc78.pre-phi, %for.inc76 ]
  br i1 %cmp171, label %for.cond16.preheader.for.inc76_crit_edge, label %for.body18.lr.ph

for.cond16.preheader.for.inc76_crit_edge:         ; preds = %for.cond16.preheader
  %inc78.pre = add i32 %index_y.05, 1
  br label %for.inc76

for.body18.lr.ph:                                 ; preds = %for.cond16.preheader
  %mul19 = mul i32 %index_y.05, %width
  %sub = add i32 %index_y.05, -1
  %mul20 = mul i32 %sub, %width
  %add22 = add i32 %index_y.05, 1
  %mul23 = mul i32 %add22, %width
  %cmp36 = icmp eq i32 %index_y.05, 0
  %cmp55 = icmp ult i32 %index_y.05, %sub54
  br label %for.body18

for.body18:                                       ; preds = %for.body18.lr.ph, %if.end72
  %index_x15.03 = phi i32 [ %div12, %for.body18.lr.ph ], [ %inc75, %if.end72 ]
  %j.02 = phi i32 [ 0, %for.body18.lr.ph ], [ %inc, %if.end72 ]
  %add = add i32 %index_x15.03, %mul19
  %add21 = add i32 %index_x15.03, %mul20
  %add24 = add i32 %index_x15.03, %mul23
  %arrayidx = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %add
  %0 = load <4 x float> addrspace(1)* %arrayidx, align 16
  %cmp25 = icmp eq i32 %index_x15.03, 0
  br i1 %cmp25, label %if.end, label %if.then

if.then:                                          ; preds = %for.body18
  %sub26 = add i32 %add, -1
  %arrayidx27 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %sub26
  %1 = load <4 x float> addrspace(1)* %arrayidx27, align 16
  %add28 = fadd <4 x float> %0, %1
  br label %if.end

if.end:                                           ; preds = %for.body18, %if.then
  %colorAccumulator.0 = phi <4 x float> [ %add28, %if.then ], [ %0, %for.body18 ]
  %cmp30 = icmp ult i32 %index_x15.03, %sub29
  br i1 %cmp30, label %if.then31, label %if.end35

if.then31:                                        ; preds = %if.end
  %add32 = add i32 %add, 1
  %arrayidx33 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %add32
  %2 = load <4 x float> addrspace(1)* %arrayidx33, align 16
  %add34 = fadd <4 x float> %colorAccumulator.0, %2
  br label %if.end35

if.end35:                                         ; preds = %if.then31, %if.end
  %colorAccumulator.1 = phi <4 x float> [ %add34, %if.then31 ], [ %colorAccumulator.0, %if.end ]
  br i1 %cmp36, label %if.end53, label %if.then37

if.then37:                                        ; preds = %if.end35
  %arrayidx38 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %add21
  %3 = load <4 x float> addrspace(1)* %arrayidx38, align 16
  %add39 = fadd <4 x float> %colorAccumulator.1, %3
  br i1 %cmp25, label %if.end45, label %if.then41

if.then41:                                        ; preds = %if.then37
  %sub42 = add i32 %add21, -1
  %arrayidx43 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %sub42
  %4 = load <4 x float> addrspace(1)* %arrayidx43, align 16
  %add44 = fadd <4 x float> %add39, %4
  br label %if.end45

if.end45:                                         ; preds = %if.then37, %if.then41
  %colorAccumulator.2 = phi <4 x float> [ %add44, %if.then41 ], [ %add39, %if.then37 ]
  br i1 %cmp30, label %if.then48, label %if.end53

if.then48:                                        ; preds = %if.end45
  %add49 = add i32 %add21, 1
  %arrayidx50 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %add49
  %5 = load <4 x float> addrspace(1)* %arrayidx50, align 16
  %add51 = fadd <4 x float> %colorAccumulator.2, %5
  br label %if.end53

if.end53:                                         ; preds = %if.end35, %if.end45, %if.then48
  %colorAccumulator.3 = phi <4 x float> [ %add51, %if.then48 ], [ %colorAccumulator.2, %if.end45 ], [ %colorAccumulator.1, %if.end35 ]
  br i1 %cmp55, label %if.then56, label %if.end72

if.then56:                                        ; preds = %if.end53
  %arrayidx57 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %add24
  %6 = load <4 x float> addrspace(1)* %arrayidx57, align 16
  %add58 = fadd <4 x float> %colorAccumulator.3, %6
  br i1 %cmp25, label %if.end64, label %if.then60

if.then60:                                        ; preds = %if.then56
  %sub61 = add i32 %add24, -1
  %arrayidx62 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %sub61
  %7 = load <4 x float> addrspace(1)* %arrayidx62, align 16
  %add63 = fadd <4 x float> %add58, %7
  br label %if.end64

if.end64:                                         ; preds = %if.then56, %if.then60
  %colorAccumulator.4 = phi <4 x float> [ %add63, %if.then60 ], [ %add58, %if.then56 ]
  br i1 %cmp30, label %if.then67, label %if.end72

if.then67:                                        ; preds = %if.end64
  %add68 = add i32 %add24, 1
  %arrayidx69 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %add68
  %8 = load <4 x float> addrspace(1)* %arrayidx69, align 16
  %add70 = fadd <4 x float> %colorAccumulator.4, %8
  br label %if.end72

if.end72:                                         ; preds = %if.end64, %if.then67, %if.end53
  %colorAccumulator.5 = phi <4 x float> [ %add70, %if.then67 ], [ %colorAccumulator.4, %if.end64 ], [ %colorAccumulator.3, %if.end53 ]
  %div73 = fdiv <4 x float> %colorAccumulator.5, <float 9.000000e+00, float 9.000000e+00, float 9.000000e+00, float 9.000000e+00>
  %arrayidx74 = getelementptr inbounds <4 x float> addrspace(1)* %output, i32 %add
  store <4 x float> %div73, <4 x float> addrspace(1)* %arrayidx74, align 16
  %inc = add i32 %j.02, 1
  %inc75 = add i32 %index_x15.03, 1
  %cmp17 = icmp ult i32 %inc, %div
  br i1 %cmp17, label %for.body18, label %for.inc76

for.inc76:                                        ; preds = %if.end72, %for.cond16.preheader.for.inc76_crit_edge
  %inc78.pre-phi = phi i32 [ %inc78.pre, %for.cond16.preheader.for.inc76_crit_edge ], [ %add22, %if.end72 ]
  %inc77 = add i32 %i.06, 1
  %cmp = icmp ult i32 %inc77, %div11
  br i1 %cmp, label %for.cond16.preheader, label %for.end79

for.end79:                                        ; preds = %for.inc76
  ret void
}

define <4 x float> @evaluatePixel(%struct._image2d_t.0* %inputImage, <2 x i32> %outCrd) nounwind readnone {
entry:
  %call = tail call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t.0* %inputImage, i32 1, <2 x i32> %outCrd) nounwind readnone
  ret <4 x float> %call
}

declare <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t.0*, i32, <2 x i32>) nounwind readnone

define void @wlSimpleBoxBlur_image2d(%struct._image2d_t.0* %inputImage, <4 x float> addrspace(1)* nocapture %output, i32 %rowCountPerGlobalID) nounwind {
entry:
  %call = tail call i32 @get_global_id(i32 0) nounwind readnone
  %mul = mul i32 %call, %rowCountPerGlobalID
  %call1 = tail call <2 x i32> @_Z13get_image_dimP10_image2d_t(%struct._image2d_t.0* %inputImage) nounwind readnone
  %add = add nsw i32 %mul, %rowCountPerGlobalID
  %0 = extractelement <2 x i32> %call1, i32 1
  %call2 = tail call i32 @_Z3minii(i32 %add, i32 %0) nounwind readnone
  %1 = extractelement <2 x i32> %call1, i32 0
  %cmp22 = icmp slt i32 %mul, %call2
  br i1 %cmp22, label %for.body.lr.ph, label %for.end38

for.body.lr.ph:                                   ; preds = %entry
  %mul3 = mul nsw i32 %1, %mul
  %cmp101 = icmp sgt i32 %1, 0
  br label %for.body

for.cond9.for.cond.loopexit_crit_edge:            ; preds = %for.body11
  %2 = add i32 %1, %index.027
  br label %for.cond.loopexit

for.cond.loopexit:                                ; preds = %for.cond9.for.cond.loopexit_crit_edge, %for.body
  %lowRightCrd.1.lcssa = phi <2 x i32> [ %20, %for.cond9.for.cond.loopexit_crit_edge ], [ %11, %for.body ]
  %lowLeftCrd.1.lcssa = phi <2 x i32> [ %19, %for.cond9.for.cond.loopexit_crit_edge ], [ %10, %for.body ]
  %lowCrd.1.lcssa = phi <2 x i32> [ %18, %for.cond9.for.cond.loopexit_crit_edge ], [ %9, %for.body ]
  %upRightCrd.1.lcssa = phi <2 x i32> [ %17, %for.cond9.for.cond.loopexit_crit_edge ], [ %8, %for.body ]
  %upLeftCrd.1.lcssa = phi <2 x i32> [ %16, %for.cond9.for.cond.loopexit_crit_edge ], [ %7, %for.body ]
  %index.1.lcssa = phi i32 [ %2, %for.cond9.for.cond.loopexit_crit_edge ], [ %index.027, %for.body ]
  %upCrd.1.lcssa = phi <2 x i32> [ %15, %for.cond9.for.cond.loopexit_crit_edge ], [ %6, %for.body ]
  %curCrd.1.lcssa = phi <2 x i32> [ %12, %for.cond9.for.cond.loopexit_crit_edge ], [ %3, %for.body ]
  %curLeftCrd.1.lcssa = phi <2 x i32> [ %13, %for.cond9.for.cond.loopexit_crit_edge ], [ %4, %for.body ]
  %curRightCrd.1.lcssa = phi <2 x i32> [ %14, %for.cond9.for.cond.loopexit_crit_edge ], [ %5, %for.body ]
  %exitcond42 = icmp eq i32 %add6, %call2
  br i1 %exitcond42, label %for.end38, label %for.body

for.body:                                         ; preds = %for.cond.loopexit, %for.body.lr.ph
  %lowRightCrd.033 = phi <2 x i32> [ undef, %for.body.lr.ph ], [ %lowRightCrd.1.lcssa, %for.cond.loopexit ]
  %lowLeftCrd.032 = phi <2 x i32> [ undef, %for.body.lr.ph ], [ %lowLeftCrd.1.lcssa, %for.cond.loopexit ]
  %lowCrd.031 = phi <2 x i32> [ undef, %for.body.lr.ph ], [ %lowCrd.1.lcssa, %for.cond.loopexit ]
  %row.030 = phi i32 [ %mul, %for.body.lr.ph ], [ %add6, %for.cond.loopexit ]
  %upRightCrd.029 = phi <2 x i32> [ undef, %for.body.lr.ph ], [ %upRightCrd.1.lcssa, %for.cond.loopexit ]
  %upLeftCrd.028 = phi <2 x i32> [ undef, %for.body.lr.ph ], [ %upLeftCrd.1.lcssa, %for.cond.loopexit ]
  %index.027 = phi i32 [ %mul3, %for.body.lr.ph ], [ %index.1.lcssa, %for.cond.loopexit ]
  %upCrd.026 = phi <2 x i32> [ undef, %for.body.lr.ph ], [ %upCrd.1.lcssa, %for.cond.loopexit ]
  %curCrd.025 = phi <2 x i32> [ undef, %for.body.lr.ph ], [ %curCrd.1.lcssa, %for.cond.loopexit ]
  %curLeftCrd.024 = phi <2 x i32> [ undef, %for.body.lr.ph ], [ %curLeftCrd.1.lcssa, %for.cond.loopexit ]
  %curRightCrd.023 = phi <2 x i32> [ undef, %for.body.lr.ph ], [ %curRightCrd.1.lcssa, %for.cond.loopexit ]
  %3 = insertelement <2 x i32> %curCrd.025, i32 %row.030, i32 1
  %4 = insertelement <2 x i32> %curLeftCrd.024, i32 %row.030, i32 1
  %5 = insertelement <2 x i32> %curRightCrd.023, i32 %row.030, i32 1
  %sub = add nsw i32 %row.030, -1
  %6 = insertelement <2 x i32> %upCrd.026, i32 %sub, i32 1
  %7 = insertelement <2 x i32> %upLeftCrd.028, i32 %sub, i32 1
  %8 = insertelement <2 x i32> %upRightCrd.029, i32 %sub, i32 1
  %add6 = add nsw i32 %row.030, 1
  %9 = insertelement <2 x i32> %lowCrd.031, i32 %add6, i32 1
  %10 = insertelement <2 x i32> %lowLeftCrd.032, i32 %add6, i32 1
  %11 = insertelement <2 x i32> %lowRightCrd.033, i32 %add6, i32 1
  br i1 %cmp101, label %for.body11, label %for.cond.loopexit

for.body11:                                       ; preds = %for.body, %for.body11
  %col.012 = phi i32 [ %add13, %for.body11 ], [ 0, %for.body ]
  %lowRightCrd.111 = phi <2 x i32> [ %20, %for.body11 ], [ %11, %for.body ]
  %lowLeftCrd.110 = phi <2 x i32> [ %19, %for.body11 ], [ %10, %for.body ]
  %lowCrd.19 = phi <2 x i32> [ %18, %for.body11 ], [ %9, %for.body ]
  %upRightCrd.18 = phi <2 x i32> [ %17, %for.body11 ], [ %8, %for.body ]
  %upLeftCrd.17 = phi <2 x i32> [ %16, %for.body11 ], [ %7, %for.body ]
  %index.16 = phi i32 [ %inc, %for.body11 ], [ %index.027, %for.body ]
  %upCrd.15 = phi <2 x i32> [ %15, %for.body11 ], [ %6, %for.body ]
  %curCrd.14 = phi <2 x i32> [ %12, %for.body11 ], [ %3, %for.body ]
  %curLeftCrd.13 = phi <2 x i32> [ %13, %for.body11 ], [ %4, %for.body ]
  %curRightCrd.12 = phi <2 x i32> [ %14, %for.body11 ], [ %5, %for.body ]
  %12 = insertelement <2 x i32> %curCrd.14, i32 %col.012, i32 0
  %sub12 = add nsw i32 %col.012, -1
  %13 = insertelement <2 x i32> %curLeftCrd.13, i32 %sub12, i32 0
  %add13 = add nsw i32 %col.012, 1
  %14 = insertelement <2 x i32> %curRightCrd.12, i32 %add13, i32 0
  %15 = insertelement <2 x i32> %upCrd.15, i32 %col.012, i32 0
  %16 = insertelement <2 x i32> %upLeftCrd.17, i32 %sub12, i32 0
  %17 = insertelement <2 x i32> %upRightCrd.18, i32 %add13, i32 0
  %18 = insertelement <2 x i32> %lowCrd.19, i32 %col.012, i32 0
  %19 = insertelement <2 x i32> %lowLeftCrd.110, i32 %sub12, i32 0
  %20 = insertelement <2 x i32> %lowRightCrd.111, i32 %add13, i32 0
  %call18 = tail call <4 x float> @evaluatePixel(%struct._image2d_t.0* %inputImage, <2 x i32> %12)
  %call19 = tail call <4 x float> @evaluatePixel(%struct._image2d_t.0* %inputImage, <2 x i32> %13)
  %add20 = fadd <4 x float> %call18, %call19
  %call21 = tail call <4 x float> @evaluatePixel(%struct._image2d_t.0* %inputImage, <2 x i32> %14)
  %add22 = fadd <4 x float> %add20, %call21
  %call23 = tail call <4 x float> @evaluatePixel(%struct._image2d_t.0* %inputImage, <2 x i32> %15)
  %add24 = fadd <4 x float> %add22, %call23
  %call25 = tail call <4 x float> @evaluatePixel(%struct._image2d_t.0* %inputImage, <2 x i32> %16)
  %add26 = fadd <4 x float> %add24, %call25
  %call27 = tail call <4 x float> @evaluatePixel(%struct._image2d_t.0* %inputImage, <2 x i32> %17)
  %add28 = fadd <4 x float> %add26, %call27
  %call29 = tail call <4 x float> @evaluatePixel(%struct._image2d_t.0* %inputImage, <2 x i32> %18)
  %add30 = fadd <4 x float> %add28, %call29
  %call31 = tail call <4 x float> @evaluatePixel(%struct._image2d_t.0* %inputImage, <2 x i32> %19)
  %add32 = fadd <4 x float> %add30, %call31
  %call33 = tail call <4 x float> @evaluatePixel(%struct._image2d_t.0* %inputImage, <2 x i32> %20)
  %add34 = fadd <4 x float> %add32, %call33
  %div = fdiv <4 x float> %add34, <float 9.000000e+00, float 9.000000e+00, float 9.000000e+00, float 9.000000e+00>
  %inc = add nsw i32 %index.16, 1
  %arrayidx = getelementptr inbounds <4 x float> addrspace(1)* %output, i32 %index.16
  store <4 x float> %div, <4 x float> addrspace(1)* %arrayidx, align 16
  %exitcond = icmp eq i32 %add13, %1
  br i1 %exitcond, label %for.cond9.for.cond.loopexit_crit_edge, label %for.body11

for.end38:                                        ; preds = %for.cond.loopexit, %entry
  ret void
}

declare <2 x i32> @_Z13get_image_dimP10_image2d_t(%struct._image2d_t.0*) nounwind readnone

declare i32 @_Z3minii(i32, i32) nounwind readnone

define void @wlSimpleBoxBlur_Optimized_CPU(<4 x float> addrspace(1)* %input, <4 x float> addrspace(1)* nocapture %output, i32 %width, i32 %height, i32 %buffer_size) nounwind {
entry:
  %call1 = tail call i32 @get_global_id(i32 0) nounwind readnone
  %call2 = tail call i32 @get_global_id(i32 1) nounwind readnone
  %call3 = tail call i32 @get_global_size(i32 0) nounwind readnone
  %call4 = tail call i32 @get_global_size(i32 1) nounwind readnone
  %div = udiv i32 %height, %call4
  %div5 = udiv i32 %width, %call3
  %mul = mul i32 %call1, %width
  %div6 = udiv i32 %mul, %call3
  %mul7 = mul i32 %call2, %height
  %div8 = udiv i32 %mul7, %call4
  %add = add i32 %div, 1
  %add9 = add i32 %add, %div8
  %cmp = icmp ult i32 %add9, %height
  br i1 %cmp, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  %sub = add i32 %height, -1
  %sub10 = sub i32 %sub, %div8
  br label %if.end

if.end:                                           ; preds = %entry, %if.then
  %count_y.0 = phi i32 [ %sub10, %if.then ], [ %div, %entry ]
  %bottomEdge.0 = phi i1 [ true, %if.then ], [ false, %entry ]
  %add11 = add i32 %div5, 1
  %add12 = add i32 %add11, %div6
  %cmp13 = icmp ult i32 %add12, %width
  br i1 %cmp13, label %if.end17, label %if.then14

if.then14:                                        ; preds = %if.end
  %sub15 = add i32 %width, -1
  %sub16 = sub i32 %sub15, %div6
  br label %if.end17

if.end17:                                         ; preds = %if.end, %if.then14
  %count_x.0 = phi i32 [ %sub16, %if.then14 ], [ %div5, %if.end ]
  %rightEdge.0 = phi i1 [ true, %if.then14 ], [ false, %if.end ]
  %cmp18 = icmp eq i32 %div8, 0
  %index_y.0 = select i1 %cmp18, i32 1, i32 %div8
  %cmp22 = icmp eq i32 %div6, 0
  %index_x.0 = select i1 %cmp22, i32 1, i32 %div6
  %sub26 = add i32 %index_y.0, -1
  %mul27 = mul i32 %sub26, %width
  %add28 = add i32 %index_x.0, -1
  %sub29 = add i32 %add28, %mul27
  %sub24 = sext i1 %cmp22 to i32
  %sub20 = sext i1 %cmp18 to i32
  %arrayidx = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %sub29
  %0 = load <4 x float> addrspace(1)* %arrayidx, align 16
  %add31 = fadd <4 x float> %0, zeroinitializer
  %add32 = add i32 %index_x.0, %mul27
  %arrayidx33 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %add32
  %1 = load <4 x float> addrspace(1)* %arrayidx33, align 16
  %add34 = fadd <4 x float> %add31, %1
  %add35 = add i32 %sub29, 2
  %arrayidx36 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %add35
  %2 = load <4 x float> addrspace(1)* %arrayidx36, align 16
  %add37 = fadd <4 x float> %add34, %2
  %add38 = add i32 %sub29, %width
  %arrayidx.1 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %add38
  %3 = load <4 x float> addrspace(1)* %arrayidx.1, align 16
  %add31.1 = fadd <4 x float> %add37, %3
  %add32.1 = add i32 %add38, 1
  %arrayidx33.1 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %add32.1
  %4 = load <4 x float> addrspace(1)* %arrayidx33.1, align 16
  %add34.1 = fadd <4 x float> %add31.1, %4
  %add35.1 = add i32 %add38, 2
  %arrayidx36.1 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %add35.1
  %5 = load <4 x float> addrspace(1)* %arrayidx36.1, align 16
  %add37.1 = fadd <4 x float> %add34.1, %5
  %add38.1 = add i32 %add38, %width
  %arrayidx.2 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %add38.1
  %6 = load <4 x float> addrspace(1)* %arrayidx.2, align 16
  %add31.2 = fadd <4 x float> %add37.1, %6
  %add32.2 = add i32 %add38.1, 1
  %arrayidx33.2 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %add32.2
  %7 = load <4 x float> addrspace(1)* %arrayidx33.2, align 16
  %add34.2 = fadd <4 x float> %add31.2, %7
  %add35.2 = add i32 %add38.1, 2
  %arrayidx36.2 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %add35.2
  %8 = load <4 x float> addrspace(1)* %arrayidx36.2, align 16
  %add37.2 = fadd <4 x float> %add34.2, %8
  %sub20.count_y.0 = add i32 %count_y.0, %sub20
  %sub24.count_x.0 = add i32 %count_x.0, %sub24
  %mul39 = mul i32 %index_y.0, %width
  %add40 = add i32 %mul39, %index_x.0
  %div41 = fdiv <4 x float> %add37.2, <float 9.000000e+00, float 9.000000e+00, float 9.000000e+00, float 9.000000e+00>
  %arrayidx42 = getelementptr inbounds <4 x float> addrspace(1)* %output, i32 %add40
  store <4 x float> %div41, <4 x float> addrspace(1)* %arrayidx42, align 16
  %sourceIndex.030 = add i32 %add40, 1
  %cmp5431 = icmp eq i32 %sub20.count_y.0, 1
  br i1 %cmp5431, label %if.end17.for.cond114.preheader_crit_edge, label %for.cond57.preheader.lr.ph

if.end17.for.cond114.preheader_crit_edge:         ; preds = %if.end17
  %cmp11518.pre = icmp ugt i32 %sub24.count_x.0, 1
  br i1 %cmp11518.pre, label %for.body116.lr.ph, label %for.end141

for.cond57.preheader.lr.ph:                       ; preds = %if.end17
  %add48 = add i32 %index_y.0, 1
  %mul49 = mul i32 %add48, %width
  %sub51 = add i32 %add28, %mul49
  %cmp5824 = icmp ugt i32 %sub24.count_x.0, 1
  %9 = icmp ugt i32 %div6, 1
  %umax52 = select i1 %9, i32 %div6, i32 1
  %10 = add i32 %count_x.0, %umax52
  %11 = icmp ugt i32 %div8, 1
  %umax53 = select i1 %11, i32 %div8, i32 1
  %12 = mul i32 %umax53, %width
  %13 = add i32 %10, %12
  %14 = add i32 %13, %sub24
  %15 = add i32 %count_y.0, %umax53
  %16 = add i32 %15, %sub20
  %17 = add i32 %16, -2
  %18 = mul i32 %17, %width
  %19 = add i32 %umax52, %18
  %20 = add i32 %16, -1
  %21 = mul i32 %20, %width
  %22 = add i32 %umax52, %21
  %23 = add i32 %sub20.count_y.0, -1
  br label %for.cond57.preheader

for.cond57.preheader:                             ; preds = %for.end84, %for.cond57.preheader.lr.ph
  %indvars.iv = phi i32 [ %14, %for.cond57.preheader.lr.ph ], [ %indvars.iv.next, %for.end84 ]
  %add5638 = phi i32 [ %add35, %for.cond57.preheader.lr.ph ], [ %add56, %for.end84 ]
  %sourceIndex.037 = phi i32 [ %sourceIndex.030, %for.cond57.preheader.lr.ph ], [ %sourceIndex.0, %for.end84 ]
  %row.036 = phi i32 [ 0, %for.cond57.preheader.lr.ph ], [ %inc86, %for.end84 ]
  %bottomRowIndex.035 = phi i32 [ %sub51, %for.cond57.preheader.lr.ph ], [ %add85, %for.end84 ]
  %topRowIndex.034 = phi i32 [ %sub29, %for.cond57.preheader.lr.ph ], [ %add110, %for.end84 ]
  %firstBlockAccumulator.133 = phi <4 x float> [ %add37.2, %for.cond57.preheader.lr.ph ], [ %add102, %for.end84 ]
  br i1 %cmp5824, label %for.body59, label %for.end84

for.cond114.preheader:                            ; preds = %for.end84
  %24 = add i32 %19, 1
  %25 = add i32 %22, 1
  %26 = add i32 %19, -1
  br i1 %cmp5824, label %for.body116.lr.ph, label %for.end141

for.body116.lr.ph:                                ; preds = %if.end17.for.cond114.preheader_crit_edge, %for.cond114.preheader
  %firstBlockAccumulator.1.lcssa64 = phi <4 x float> [ %add37.2, %if.end17.for.cond114.preheader_crit_edge ], [ %add102, %for.cond114.preheader ]
  %topRowIndex.0.lcssa63 = phi i32 [ %sub29, %if.end17.for.cond114.preheader_crit_edge ], [ %26, %for.cond114.preheader ]
  %sourceIndex.0.lcssa62 = phi i32 [ %sourceIndex.030, %if.end17.for.cond114.preheader_crit_edge ], [ %25, %for.cond114.preheader ]
  %add56.lcssa61 = phi i32 [ %add35, %if.end17.for.cond114.preheader_crit_edge ], [ %24, %for.cond114.preheader ]
  %27 = add i32 %count_x.0, %sourceIndex.0.lcssa62
  %28 = add i32 %27, %sub24
  %29 = add i32 %28, -1
  br label %for.body116

for.body59:                                       ; preds = %for.cond57.preheader, %for.body59
  %colorAccumulator.128 = phi <4 x float> [ %add76, %for.body59 ], [ %firstBlockAccumulator.133, %for.cond57.preheader ]
  %sourceIndex.127 = phi i32 [ %inc80, %for.body59 ], [ %sourceIndex.037, %for.cond57.preheader ]
  %leftColumnIndex.026 = phi i32 [ %inc81, %for.body59 ], [ %topRowIndex.034, %for.cond57.preheader ]
  %rightColumnIndex.025 = phi i32 [ %inc60, %for.body59 ], [ %add5638, %for.cond57.preheader ]
  %inc60 = add i32 %rightColumnIndex.025, 1
  %arrayidx61 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %leftColumnIndex.026
  %30 = load <4 x float> addrspace(1)* %arrayidx61, align 16
  %sub62 = fsub <4 x float> %colorAccumulator.128, %30
  %add63 = add i32 %leftColumnIndex.026, %width
  %arrayidx64 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %inc60
  %31 = load <4 x float> addrspace(1)* %arrayidx64, align 16
  %add65 = fadd <4 x float> %sub62, %31
  %add66 = add i32 %inc60, %width
  %arrayidx67 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %add63
  %32 = load <4 x float> addrspace(1)* %arrayidx67, align 16
  %sub68 = fsub <4 x float> %add65, %32
  %add69 = add i32 %add63, %width
  %arrayidx70 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %add66
  %33 = load <4 x float> addrspace(1)* %arrayidx70, align 16
  %add71 = fadd <4 x float> %sub68, %33
  %add72 = add i32 %add66, %width
  %arrayidx73 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %add69
  %34 = load <4 x float> addrspace(1)* %arrayidx73, align 16
  %sub74 = fsub <4 x float> %add71, %34
  %arrayidx75 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %add72
  %35 = load <4 x float> addrspace(1)* %arrayidx75, align 16
  %add76 = fadd <4 x float> %sub74, %35
  %div78 = fdiv <4 x float> %add76, <float 9.000000e+00, float 9.000000e+00, float 9.000000e+00, float 9.000000e+00>
  %arrayidx79 = getelementptr inbounds <4 x float> addrspace(1)* %output, i32 %sourceIndex.127
  store <4 x float> %div78, <4 x float> addrspace(1)* %arrayidx79, align 16
  %inc80 = add i32 %sourceIndex.127, 1
  %inc81 = add i32 %leftColumnIndex.026, 1
  %exitcond54 = icmp eq i32 %inc80, %indvars.iv
  br i1 %exitcond54, label %for.end84, label %for.body59

for.end84:                                        ; preds = %for.body59, %for.cond57.preheader
  %add85 = add i32 %bottomRowIndex.035, %width
  %inc86 = add i32 %row.036, 1
  %arrayidx87 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %topRowIndex.034
  %36 = load <4 x float> addrspace(1)* %arrayidx87, align 16
  %sub88 = fsub <4 x float> %firstBlockAccumulator.133, %36
  %inc89 = add i32 %topRowIndex.034, 1
  %arrayidx90 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %add85
  %37 = load <4 x float> addrspace(1)* %arrayidx90, align 16
  %add91 = fadd <4 x float> %sub88, %37
  %inc92 = add i32 %add85, 1
  %arrayidx93 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %inc89
  %38 = load <4 x float> addrspace(1)* %arrayidx93, align 16
  %sub94 = fsub <4 x float> %add91, %38
  %arrayidx96 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %inc92
  %39 = load <4 x float> addrspace(1)* %arrayidx96, align 16
  %add97 = fadd <4 x float> %sub94, %39
  %inc98 = add i32 %add85, 2
  %arrayidx99 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %add5638
  %40 = load <4 x float> addrspace(1)* %arrayidx99, align 16
  %sub100 = fsub <4 x float> %add97, %40
  %arrayidx101 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %inc98
  %41 = load <4 x float> addrspace(1)* %arrayidx101, align 16
  %add102 = fadd <4 x float> %sub100, %41
  %add103 = add i32 %inc86, %index_y.0
  %mul104 = mul i32 %add103, %width
  %add105 = add i32 %mul104, %index_x.0
  %div107 = fdiv <4 x float> %add102, <float 9.000000e+00, float 9.000000e+00, float 9.000000e+00, float 9.000000e+00>
  %arrayidx108 = getelementptr inbounds <4 x float> addrspace(1)* %output, i32 %add105
  store <4 x float> %div107, <4 x float> addrspace(1)* %arrayidx108, align 16
  %add110 = add i32 %topRowIndex.034, %width
  %sourceIndex.0 = add i32 %add105, 1
  %add56 = add i32 %add110, 2
  %indvars.iv.next = add i32 %indvars.iv, %width
  %exitcond59 = icmp eq i32 %inc86, %23
  br i1 %exitcond59, label %for.cond114.preheader, label %for.cond57.preheader

for.body116:                                      ; preds = %for.body116, %for.body116.lr.ph
  %colorAccumulator.222 = phi <4 x float> [ %firstBlockAccumulator.1.lcssa64, %for.body116.lr.ph ], [ %add133, %for.body116 ]
  %sourceIndex.221 = phi i32 [ %sourceIndex.0.lcssa62, %for.body116.lr.ph ], [ %inc137, %for.body116 ]
  %leftColumnIndex.120 = phi i32 [ %topRowIndex.0.lcssa63, %for.body116.lr.ph ], [ %inc138, %for.body116 ]
  %rightColumnIndex.119 = phi i32 [ %add56.lcssa61, %for.body116.lr.ph ], [ %inc117, %for.body116 ]
  %inc117 = add i32 %rightColumnIndex.119, 1
  %arrayidx118 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %leftColumnIndex.120
  %42 = load <4 x float> addrspace(1)* %arrayidx118, align 16
  %sub119 = fsub <4 x float> %colorAccumulator.222, %42
  %add120 = add i32 %leftColumnIndex.120, %width
  %arrayidx121 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %inc117
  %43 = load <4 x float> addrspace(1)* %arrayidx121, align 16
  %add122 = fadd <4 x float> %sub119, %43
  %add123 = add i32 %inc117, %width
  %arrayidx124 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %add120
  %44 = load <4 x float> addrspace(1)* %arrayidx124, align 16
  %sub125 = fsub <4 x float> %add122, %44
  %add126 = add i32 %add120, %width
  %arrayidx127 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %add123
  %45 = load <4 x float> addrspace(1)* %arrayidx127, align 16
  %add128 = fadd <4 x float> %sub125, %45
  %add129 = add i32 %add123, %width
  %arrayidx130 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %add126
  %46 = load <4 x float> addrspace(1)* %arrayidx130, align 16
  %sub131 = fsub <4 x float> %add128, %46
  %arrayidx132 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %add129
  %47 = load <4 x float> addrspace(1)* %arrayidx132, align 16
  %add133 = fadd <4 x float> %sub131, %47
  %div135 = fdiv <4 x float> %add133, <float 9.000000e+00, float 9.000000e+00, float 9.000000e+00, float 9.000000e+00>
  %arrayidx136 = getelementptr inbounds <4 x float> addrspace(1)* %output, i32 %sourceIndex.221
  store <4 x float> %div135, <4 x float> addrspace(1)* %arrayidx136, align 16
  %inc137 = add i32 %sourceIndex.221, 1
  %inc138 = add i32 %leftColumnIndex.120, 1
  %exitcond51 = icmp eq i32 %inc137, %29
  br i1 %exitcond51, label %for.end141, label %for.body116

for.end141:                                       ; preds = %for.body116, %if.end17.for.cond114.preheader_crit_edge, %for.cond114.preheader
  %topEdge.0.not = xor i1 %cmp18, true
  %leftEdge.0.not = xor i1 %cmp22, true
  %brmerge = or i1 %topEdge.0.not, %leftEdge.0.not
  br i1 %brmerge, label %if.end156, label %if.then143

if.then143:                                       ; preds = %for.end141
  %48 = load <4 x float> addrspace(1)* %input, align 16
  %add145 = fadd <4 x float> %48, zeroinitializer
  %arrayidx146 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 1
  %49 = load <4 x float> addrspace(1)* %arrayidx146, align 16
  %add147 = fadd <4 x float> %add145, %49
  %arrayidx148 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %width
  %50 = load <4 x float> addrspace(1)* %arrayidx148, align 16
  %add149 = fadd <4 x float> %add147, %50
  %add150 = add i32 %width, 1
  %arrayidx151 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %add150
  %51 = load <4 x float> addrspace(1)* %arrayidx151, align 16
  %add152 = fadd <4 x float> %add149, %51
  %div154 = fdiv <4 x float> %add152, <float 9.000000e+00, float 9.000000e+00, float 9.000000e+00, float 9.000000e+00>
  store <4 x float> %div154, <4 x float> addrspace(1)* %output, align 16
  br label %if.end156

if.end156:                                        ; preds = %for.end141, %if.then143
  br i1 %cmp18, label %for.cond160.preheader, label %if.end212

for.cond160.preheader:                            ; preds = %if.end156
  %add161 = add i32 %sub24.count_x.0, %index_x.0
  %cmp16216 = icmp ult i32 %index_x.0, %add161
  br i1 %cmp16216, label %for.body163.lr.ph, label %if.end189

for.body163.lr.ph:                                ; preds = %for.cond160.preheader
  %52 = icmp ugt i32 %div6, 1
  %umax49 = select i1 %52, i32 %div6, i32 1
  %53 = add i32 %count_x.0, %umax49
  %54 = add i32 %53, %sub24
  br label %for.body163

for.body163:                                      ; preds = %for.body163, %for.body163.lr.ph
  %column159.017 = phi i32 [ %index_x.0, %for.body163.lr.ph ], [ %add169, %for.body163 ]
  %sub164 = add nsw i32 %column159.017, -1
  %arrayidx165 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %sub164
  %55 = load <4 x float> addrspace(1)* %arrayidx165, align 16
  %add166 = fadd <4 x float> %55, zeroinitializer
  %arrayidx167 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %column159.017
  %56 = load <4 x float> addrspace(1)* %arrayidx167, align 16
  %add168 = fadd <4 x float> %add166, %56
  %add169 = add nsw i32 %column159.017, 1
  %arrayidx170 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %add169
  %57 = load <4 x float> addrspace(1)* %arrayidx170, align 16
  %add171 = fadd <4 x float> %add168, %57
  %add172 = add i32 %column159.017, %width
  %sub173 = add i32 %add172, -1
  %arrayidx174 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %sub173
  %58 = load <4 x float> addrspace(1)* %arrayidx174, align 16
  %add175 = fadd <4 x float> %add171, %58
  %arrayidx177 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %add172
  %59 = load <4 x float> addrspace(1)* %arrayidx177, align 16
  %add178 = fadd <4 x float> %add175, %59
  %add180 = add i32 %add172, 1
  %arrayidx181 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %add180
  %60 = load <4 x float> addrspace(1)* %arrayidx181, align 16
  %add182 = fadd <4 x float> %add178, %60
  %div184 = fdiv <4 x float> %add182, <float 9.000000e+00, float 9.000000e+00, float 9.000000e+00, float 9.000000e+00>
  %arrayidx185 = getelementptr inbounds <4 x float> addrspace(1)* %output, i32 %column159.017
  store <4 x float> %div184, <4 x float> addrspace(1)* %arrayidx185, align 16
  %exitcond50 = icmp eq i32 %add169, %54
  br i1 %exitcond50, label %if.end189, label %for.body163

if.end189:                                        ; preds = %for.body163, %for.cond160.preheader
  %rightEdge.0.not = xor i1 %rightEdge.0, true
  %brmerge2 = or i1 %topEdge.0.not, %rightEdge.0.not
  br i1 %brmerge2, label %if.end212, label %if.then193

if.then193:                                       ; preds = %if.end189
  %sub194 = add i32 %width, -2
  %arrayidx195 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %sub194
  %61 = load <4 x float> addrspace(1)* %arrayidx195, align 16
  %add196 = fadd <4 x float> %61, zeroinitializer
  %sub197 = add i32 %width, -1
  %arrayidx198 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %sub197
  %62 = load <4 x float> addrspace(1)* %arrayidx198, align 16
  %add199 = fadd <4 x float> %add196, %62
  %add200 = shl i32 %width, 1
  %sub201 = add i32 %add200, -2
  %arrayidx202 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %sub201
  %63 = load <4 x float> addrspace(1)* %arrayidx202, align 16
  %add203 = fadd <4 x float> %add199, %63
  %sub205 = add i32 %add200, -1
  %arrayidx206 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %sub205
  %64 = load <4 x float> addrspace(1)* %arrayidx206, align 16
  %add207 = fadd <4 x float> %add203, %64
  %div209 = fdiv <4 x float> %add207, <float 9.000000e+00, float 9.000000e+00, float 9.000000e+00, float 9.000000e+00>
  %arrayidx211 = getelementptr inbounds <4 x float> addrspace(1)* %output, i32 %sub197
  store <4 x float> %div209, <4 x float> addrspace(1)* %arrayidx211, align 16
  br label %if.end212

if.end212:                                        ; preds = %if.end156, %if.end189, %if.then193
  br i1 %cmp22, label %for.cond216.preheader, label %if.end252

for.cond216.preheader:                            ; preds = %if.end212
  %add217 = add i32 %sub20.count_y.0, %index_y.0
  %cmp21814 = icmp ult i32 %index_y.0, %add217
  br i1 %cmp21814, label %for.body219.lr.ph, label %if.end252

for.body219.lr.ph:                                ; preds = %for.cond216.preheader
  %65 = icmp ugt i32 %div8, 1
  %umax47 = select i1 %65, i32 %div8, i32 1
  %66 = add i32 %count_y.0, %umax47
  %67 = add i32 %66, %sub20
  br label %for.body219

for.body219:                                      ; preds = %for.body219, %for.body219.lr.ph
  %row215.015 = phi i32 [ %index_y.0, %for.body219.lr.ph ], [ %add227, %for.body219 ]
  %sub220 = add nsw i32 %row215.015, -1
  %mul221 = mul i32 %sub220, %width
  %arrayidx222 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %mul221
  %68 = load <4 x float> addrspace(1)* %arrayidx222, align 16
  %add223 = fadd <4 x float> %68, zeroinitializer
  %mul224 = mul i32 %row215.015, %width
  %arrayidx225 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %mul224
  %69 = load <4 x float> addrspace(1)* %arrayidx225, align 16
  %add226 = fadd <4 x float> %add223, %69
  %add227 = add nsw i32 %row215.015, 1
  %mul228 = mul i32 %add227, %width
  %arrayidx229 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %mul228
  %70 = load <4 x float> addrspace(1)* %arrayidx229, align 16
  %add230 = fadd <4 x float> %add226, %70
  %add233 = add i32 %mul221, 1
  %arrayidx234 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %add233
  %71 = load <4 x float> addrspace(1)* %arrayidx234, align 16
  %add235 = fadd <4 x float> %add230, %71
  %add237 = add i32 %mul224, 1
  %arrayidx238 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %add237
  %72 = load <4 x float> addrspace(1)* %arrayidx238, align 16
  %add239 = fadd <4 x float> %add235, %72
  %add242 = add i32 %mul228, 1
  %arrayidx243 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %add242
  %73 = load <4 x float> addrspace(1)* %arrayidx243, align 16
  %add244 = fadd <4 x float> %add239, %73
  %div246 = fdiv <4 x float> %add244, <float 9.000000e+00, float 9.000000e+00, float 9.000000e+00, float 9.000000e+00>
  %arrayidx248 = getelementptr inbounds <4 x float> addrspace(1)* %output, i32 %mul224
  store <4 x float> %div246, <4 x float> addrspace(1)* %arrayidx248, align 16
  %exitcond48 = icmp eq i32 %add227, %67
  br i1 %exitcond48, label %if.end252, label %for.body219

if.end252:                                        ; preds = %for.cond216.preheader, %for.body219, %if.end212
  %bottomEdge.0.not = xor i1 %bottomEdge.0, true
  %brmerge4 = or i1 %bottomEdge.0.not, %leftEdge.0.not
  br i1 %brmerge4, label %if.end280, label %if.then256

if.then256:                                       ; preds = %if.end252
  %sub257 = add i32 %height, -2
  %mul258 = mul i32 %sub257, %width
  %arrayidx259 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %mul258
  %74 = load <4 x float> addrspace(1)* %arrayidx259, align 16
  %add260 = fadd <4 x float> %74, zeroinitializer
  %add263 = add i32 %mul258, 1
  %arrayidx264 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %add263
  %75 = load <4 x float> addrspace(1)* %arrayidx264, align 16
  %add265 = fadd <4 x float> %add260, %75
  %sub266 = add i32 %height, -1
  %mul267 = mul i32 %sub266, %width
  %arrayidx268 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %mul267
  %76 = load <4 x float> addrspace(1)* %arrayidx268, align 16
  %add269 = fadd <4 x float> %add265, %76
  %add272 = add i32 %mul267, 1
  %arrayidx273 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %add272
  %77 = load <4 x float> addrspace(1)* %arrayidx273, align 16
  %add274 = fadd <4 x float> %add269, %77
  %div276 = fdiv <4 x float> %add274, <float 9.000000e+00, float 9.000000e+00, float 9.000000e+00, float 9.000000e+00>
  %arrayidx279 = getelementptr inbounds <4 x float> addrspace(1)* %output, i32 %mul267
  store <4 x float> %div276, <4 x float> addrspace(1)* %arrayidx279, align 16
  br label %if.end280

if.end280:                                        ; preds = %if.end252, %if.then256
  br i1 %bottomEdge.0, label %for.cond284.preheader, label %if.end331

for.cond284.preheader:                            ; preds = %if.end280
  %add285 = add i32 %sub24.count_x.0, %index_x.0
  %cmp28612 = icmp ult i32 %index_x.0, %add285
  br i1 %cmp28612, label %for.body287.lr.ph, label %if.end331

for.body287.lr.ph:                                ; preds = %for.cond284.preheader
  %sub288 = add i32 %height, -2
  %mul289 = mul i32 %sub288, %width
  %sub305 = add i32 %height, -1
  %mul306 = mul i32 %sub305, %width
  %78 = icmp ugt i32 %div6, 1
  %umax45 = select i1 %78, i32 %div6, i32 1
  %79 = add i32 %count_x.0, %umax45
  %80 = add i32 %79, %sub24
  br label %for.body287

for.body287:                                      ; preds = %for.body287, %for.body287.lr.ph
  %column283.013 = phi i32 [ %index_x.0, %for.body287.lr.ph ], [ %inc329, %for.body287 ]
  %add290 = add i32 %column283.013, %mul289
  %sub291 = add i32 %add290, -1
  %arrayidx292 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %sub291
  %81 = load <4 x float> addrspace(1)* %arrayidx292, align 16
  %add293 = fadd <4 x float> %81, zeroinitializer
  %arrayidx297 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %add290
  %82 = load <4 x float> addrspace(1)* %arrayidx297, align 16
  %add298 = fadd <4 x float> %add293, %82
  %add302 = add i32 %add290, 1
  %arrayidx303 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %add302
  %83 = load <4 x float> addrspace(1)* %arrayidx303, align 16
  %add304 = fadd <4 x float> %add298, %83
  %add307 = add i32 %column283.013, %mul306
  %sub308 = add i32 %add307, -1
  %arrayidx309 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %sub308
  %84 = load <4 x float> addrspace(1)* %arrayidx309, align 16
  %add310 = fadd <4 x float> %add304, %84
  %arrayidx314 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %add307
  %85 = load <4 x float> addrspace(1)* %arrayidx314, align 16
  %add315 = fadd <4 x float> %add310, %85
  %add319 = add i32 %add307, 1
  %arrayidx320 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %add319
  %86 = load <4 x float> addrspace(1)* %arrayidx320, align 16
  %add321 = fadd <4 x float> %add315, %86
  %div323 = fdiv <4 x float> %add321, <float 9.000000e+00, float 9.000000e+00, float 9.000000e+00, float 9.000000e+00>
  %arrayidx327 = getelementptr inbounds <4 x float> addrspace(1)* %output, i32 %add307
  store <4 x float> %div323, <4 x float> addrspace(1)* %arrayidx327, align 16
  %inc329 = add nsw i32 %column283.013, 1
  %exitcond46 = icmp eq i32 %inc329, %80
  br i1 %exitcond46, label %if.end331, label %for.body287

if.end331:                                        ; preds = %for.cond284.preheader, %for.body287, %if.end280
  br i1 %rightEdge.0, label %for.cond335.preheader, label %if.end404

for.cond335.preheader:                            ; preds = %if.end331
  %add336 = add i32 %sub20.count_y.0, %index_y.0
  %cmp33710 = icmp ult i32 %index_y.0, %add336
  br i1 %cmp33710, label %for.body338.lr.ph, label %if.end376

for.body338.lr.ph:                                ; preds = %for.cond335.preheader
  %87 = icmp ugt i32 %div8, 1
  %umax = select i1 %87, i32 %div8, i32 1
  %88 = add i32 %count_y.0, %umax
  %89 = add i32 %88, %sub20
  br label %for.body338

for.body338:                                      ; preds = %for.body338, %for.body338.lr.ph
  %row334.011 = phi i32 [ %index_y.0, %for.body338.lr.ph ], [ %add343, %for.body338 ]
  %mul339 = mul i32 %row334.011, %width
  %sub340 = add i32 %mul339, -1
  %arrayidx341 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %sub340
  %90 = load <4 x float> addrspace(1)* %arrayidx341, align 16
  %add342 = fadd <4 x float> %90, zeroinitializer
  %add343 = add nsw i32 %row334.011, 1
  %mul344 = mul i32 %add343, %width
  %sub345 = add i32 %mul344, -1
  %arrayidx346 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %sub345
  %91 = load <4 x float> addrspace(1)* %arrayidx346, align 16
  %add347 = fadd <4 x float> %add342, %91
  %add348 = add nsw i32 %row334.011, 2
  %mul349 = mul i32 %add348, %width
  %sub350 = add i32 %mul349, -1
  %arrayidx351 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %sub350
  %92 = load <4 x float> addrspace(1)* %arrayidx351, align 16
  %add352 = fadd <4 x float> %add347, %92
  %sub354 = add i32 %mul339, -2
  %arrayidx355 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %sub354
  %93 = load <4 x float> addrspace(1)* %arrayidx355, align 16
  %add356 = fadd <4 x float> %add352, %93
  %sub359 = add i32 %mul344, -2
  %arrayidx360 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %sub359
  %94 = load <4 x float> addrspace(1)* %arrayidx360, align 16
  %add361 = fadd <4 x float> %add356, %94
  %sub364 = add i32 %mul349, -2
  %arrayidx365 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %sub364
  %95 = load <4 x float> addrspace(1)* %arrayidx365, align 16
  %add366 = fadd <4 x float> %add361, %95
  %div368 = fdiv <4 x float> %add366, <float 9.000000e+00, float 9.000000e+00, float 9.000000e+00, float 9.000000e+00>
  %arrayidx372 = getelementptr inbounds <4 x float> addrspace(1)* %output, i32 %sub345
  store <4 x float> %div368, <4 x float> addrspace(1)* %arrayidx372, align 16
  %exitcond = icmp eq i32 %add343, %89
  br i1 %exitcond, label %if.end376, label %for.body338

if.end376:                                        ; preds = %for.body338, %for.cond335.preheader
  br i1 %bottomEdge.0, label %if.then380, label %if.end404

if.then380:                                       ; preds = %if.end376
  %sub381 = add i32 %height, -1
  %mul382 = mul i32 %sub381, %width
  %sub383 = add i32 %mul382, -2
  %arrayidx384 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %sub383
  %96 = load <4 x float> addrspace(1)* %arrayidx384, align 16
  %add385 = fadd <4 x float> %96, zeroinitializer
  %sub388 = add i32 %mul382, -1
  %arrayidx389 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %sub388
  %97 = load <4 x float> addrspace(1)* %arrayidx389, align 16
  %add390 = fadd <4 x float> %add385, %97
  %mul391 = mul i32 %height, %width
  %sub392 = add i32 %mul391, -2
  %arrayidx393 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %sub392
  %98 = load <4 x float> addrspace(1)* %arrayidx393, align 16
  %add394 = fadd <4 x float> %add390, %98
  %sub396 = add i32 %mul391, -1
  %arrayidx397 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %sub396
  %99 = load <4 x float> addrspace(1)* %arrayidx397, align 16
  %add398 = fadd <4 x float> %add394, %99
  %div400 = fdiv <4 x float> %add398, <float 9.000000e+00, float 9.000000e+00, float 9.000000e+00, float 9.000000e+00>
  %arrayidx403 = getelementptr inbounds <4 x float> addrspace(1)* %output, i32 %sub396
  store <4 x float> %div400, <4 x float> addrspace(1)* %arrayidx403, align 16
  br label %if.end404

if.end404:                                        ; preds = %if.end376, %if.end331, %if.then380
  ret void
}

define [7 x i32] @WG.boundaries.wlSimpleBoxBlur_GPU(<4 x float> addrspace(1)*, <4 x float> addrspace(1)*, i32, i32, i32) {
entry:
  %5 = call i32 @get_local_size(i32 0)
  %6 = call i32 @get_base_global_id.(i32 0)
  %7 = call i32 @get_local_size(i32 1)
  %8 = call i32 @get_base_global_id.(i32 1)
  %9 = call i32 @get_local_size(i32 2)
  %10 = call i32 @get_base_global_id.(i32 2)
  %11 = insertvalue [7 x i32] undef, i32 %5, 2
  %12 = insertvalue [7 x i32] %11, i32 %6, 1
  %13 = insertvalue [7 x i32] %12, i32 %7, 4
  %14 = insertvalue [7 x i32] %13, i32 %8, 3
  %15 = insertvalue [7 x i32] %14, i32 %9, 6
  %16 = insertvalue [7 x i32] %15, i32 %10, 5
  %17 = insertvalue [7 x i32] %16, i32 1, 0
  ret [7 x i32] %17
}

declare i32 @get_local_size(i32)

declare i32 @get_base_global_id.(i32)

define [7 x i32] @WG.boundaries.wlSimpleBoxBlur_CPU(<4 x float> addrspace(1)*, <4 x float> addrspace(1)*, i32, i32, i32) {
entry:
  %5 = call i32 @get_local_size(i32 0)
  %6 = call i32 @get_base_global_id.(i32 0)
  %7 = call i32 @get_local_size(i32 1)
  %8 = call i32 @get_base_global_id.(i32 1)
  %9 = call i32 @get_local_size(i32 2)
  %10 = call i32 @get_base_global_id.(i32 2)
  %11 = tail call i32 @get_global_size(i32 1) nounwind readnone
  %12 = udiv i32 %3, %11
  %13 = icmp eq i32 %12, 0
  %14 = xor i1 %13, true
  %15 = and i1 true, %14
  %zext_cast = zext i1 %15 to i32
  %16 = insertvalue [7 x i32] undef, i32 %5, 2
  %17 = insertvalue [7 x i32] %16, i32 %6, 1
  %18 = insertvalue [7 x i32] %17, i32 %7, 4
  %19 = insertvalue [7 x i32] %18, i32 %8, 3
  %20 = insertvalue [7 x i32] %19, i32 %9, 6
  %21 = insertvalue [7 x i32] %20, i32 %10, 5
  %22 = insertvalue [7 x i32] %21, i32 %zext_cast, 0
  ret [7 x i32] %22
}

define [7 x i32] @WG.boundaries.wlSimpleBoxBlur_image2d(%struct._image2d_t.0*, <4 x float> addrspace(1)*, i32) {
entry:
  %3 = call i32 @get_local_size(i32 0)
  %4 = call i32 @get_base_global_id.(i32 0)
  %5 = call i32 @get_local_size(i32 1)
  %6 = call i32 @get_base_global_id.(i32 1)
  %7 = call i32 @get_local_size(i32 2)
  %8 = call i32 @get_base_global_id.(i32 2)
  %9 = insertvalue [7 x i32] undef, i32 %3, 2
  %10 = insertvalue [7 x i32] %9, i32 %4, 1
  %11 = insertvalue [7 x i32] %10, i32 %5, 4
  %12 = insertvalue [7 x i32] %11, i32 %6, 3
  %13 = insertvalue [7 x i32] %12, i32 %7, 6
  %14 = insertvalue [7 x i32] %13, i32 %8, 5
  %15 = insertvalue [7 x i32] %14, i32 1, 0
  ret [7 x i32] %15
}

define [7 x i32] @WG.boundaries.wlSimpleBoxBlur_Optimized_CPU(<4 x float> addrspace(1)*, <4 x float> addrspace(1)*, i32, i32, i32) {
entry:
  %5 = call i32 @get_local_size(i32 0)
  %6 = call i32 @get_base_global_id.(i32 0)
  %7 = call i32 @get_local_size(i32 1)
  %8 = call i32 @get_base_global_id.(i32 1)
  %9 = call i32 @get_local_size(i32 2)
  %10 = call i32 @get_base_global_id.(i32 2)
  %11 = insertvalue [7 x i32] undef, i32 %5, 2
  %12 = insertvalue [7 x i32] %11, i32 %6, 1
  %13 = insertvalue [7 x i32] %12, i32 %7, 4
  %14 = insertvalue [7 x i32] %13, i32 %8, 3
  %15 = insertvalue [7 x i32] %14, i32 %9, 6
  %16 = insertvalue [7 x i32] %15, i32 %10, 5
  %17 = insertvalue [7 x i32] %16, i32 1, 0
  ret [7 x i32] %17
}

!opencl.kernels = !{!0, !8, !9, !17}
!opencl.build.options = !{!18}
!cl.noBarrierPath.kernels = !{!19}

!0 = metadata !{void (<4 x float> addrspace(1)*, <4 x float> addrspace(1)*, i32, i32, i32)* @wlSimpleBoxBlur_GPU, metadata !1, metadata !2}
!1 = metadata !{metadata !"image_access_qualifier", i32 3, i32 3, i32 3, i32 3, i32 3}
!2 = metadata !{metadata !"cl_kernel_arg_info", metadata !3, metadata !4, metadata !5, metadata !6, metadata !7}
!3 = metadata !{i32 0, i32 0, i32 3, i32 3, i32 3}
!4 = metadata !{i32 3, i32 3, i32 3, i32 3, i32 3}
!5 = metadata !{metadata !"float4 *", metadata !"float4 *", metadata !"uint", metadata !"uint", metadata !"uint"}
!6 = metadata !{i32 0, i32 0, i32 1, i32 1, i32 1}
!7 = metadata !{metadata !"input", metadata !"output", metadata !"width", metadata !"height", metadata !"buffer_size"}
!8 = metadata !{void (<4 x float> addrspace(1)*, <4 x float> addrspace(1)*, i32, i32, i32)* @wlSimpleBoxBlur_CPU, metadata !1, metadata !2}
!9 = metadata !{void (%struct._image2d_t.0*, <4 x float> addrspace(1)*, i32)* @wlSimpleBoxBlur_image2d, metadata !10, metadata !11}
!10 = metadata !{metadata !"image_access_qualifier", i32 0, i32 3, i32 3}
!11 = metadata !{metadata !"cl_kernel_arg_info", metadata !12, metadata !13, metadata !14, metadata !15, metadata !16}
!12 = metadata !{i32 3, i32 0, i32 3}
!13 = metadata !{i32 0, i32 3, i32 3}
!14 = metadata !{metadata !"image2d_t", metadata !"float4 *", metadata !"uint"}
!15 = metadata !{i32 0, i32 0, i32 1}
!16 = metadata !{metadata !"inputImage", metadata !"output", metadata !"rowCountPerGlobalID"}
!17 = metadata !{void (<4 x float> addrspace(1)*, <4 x float> addrspace(1)*, i32, i32, i32)* @wlSimpleBoxBlur_Optimized_CPU, metadata !1, metadata !2}
!18 = metadata !{metadata !"-cl-kernel-arg-info"}
!19 = metadata !{metadata !"wlSimpleBoxBlur_GPU", metadata !"wlSimpleBoxBlur_CPU", metadata !"wlSimpleBoxBlur_image2d", metadata !"wlSimpleBoxBlur_Optimized_CPU"}
