; ModuleID = 'Program'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct._image2d_t.0 = type opaque

define void @wlSimpleBoxBlur_GPU(<4 x float> addrspace(1)* nocapture %input, <4 x float> addrspace(1)* nocapture %output, i32 %width, i32 %height, i32 %buffer_size) nounwind {
entry:
  %call1 = tail call i64 @get_global_id(i32 0) nounwind readnone
  %call2 = tail call i64 @get_global_id(i32 1) nounwind readnone
  %call5 = tail call i64 @get_global_size(i32 0) nounwind readnone
  %call6 = tail call i64 @get_global_size(i32 1) nounwind readnone
  %conv11 = zext i32 %width to i64
  %mul = mul i64 %call2, %conv11
  %add = add i64 %mul, %call1
  %sub = add i64 %call2, -1
  %mul13 = mul i64 %sub, %conv11
  %add14 = add i64 %mul13, %call1
  %add15 = add i64 %call2, 1
  %mul17 = mul i64 %add15, %conv11
  %add18 = add i64 %mul17, %call1
  %arrayidx = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %add
  %0 = load <4 x float> addrspace(1)* %arrayidx, align 16
  %cmp = icmp eq i64 %call1, 0
  br i1 %cmp, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  %sub20 = add i64 %add, -1
  %arrayidx21 = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %sub20
  %1 = load <4 x float> addrspace(1)* %arrayidx21, align 16
  %add22 = fadd <4 x float> %0, %1
  br label %if.end

if.end:                                           ; preds = %entry, %if.then
  %colorAccumulator.0 = phi <4 x float> [ %add22, %if.then ], [ %0, %entry ]
  %sub23 = add i64 %call5, -1
  %cmp24 = icmp ult i64 %call1, %sub23
  br i1 %cmp24, label %if.then26, label %if.end30

if.then26:                                        ; preds = %if.end
  %add27 = add i64 %add, 1
  %arrayidx28 = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %add27
  %2 = load <4 x float> addrspace(1)* %arrayidx28, align 16
  %add29 = fadd <4 x float> %colorAccumulator.0, %2
  br label %if.end30

if.end30:                                         ; preds = %if.then26, %if.end
  %colorAccumulator.1 = phi <4 x float> [ %add29, %if.then26 ], [ %colorAccumulator.0, %if.end ]
  %cmp31 = icmp eq i64 %call2, 0
  br i1 %cmp31, label %if.end51, label %if.then33

if.then33:                                        ; preds = %if.end30
  %arrayidx34 = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %add14
  %3 = load <4 x float> addrspace(1)* %arrayidx34, align 16
  %add35 = fadd <4 x float> %colorAccumulator.1, %3
  br i1 %cmp, label %if.end42, label %if.then38

if.then38:                                        ; preds = %if.then33
  %sub39 = add i64 %add14, -1
  %arrayidx40 = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %sub39
  %4 = load <4 x float> addrspace(1)* %arrayidx40, align 16
  %add41 = fadd <4 x float> %add35, %4
  br label %if.end42

if.end42:                                         ; preds = %if.then33, %if.then38
  %colorAccumulator.2 = phi <4 x float> [ %add41, %if.then38 ], [ %add35, %if.then33 ]
  br i1 %cmp24, label %if.then46, label %if.end51

if.then46:                                        ; preds = %if.end42
  %add47 = add i64 %add14, 1
  %arrayidx48 = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %add47
  %5 = load <4 x float> addrspace(1)* %arrayidx48, align 16
  %add49 = fadd <4 x float> %colorAccumulator.2, %5
  br label %if.end51

if.end51:                                         ; preds = %if.end30, %if.end42, %if.then46
  %colorAccumulator.3 = phi <4 x float> [ %add49, %if.then46 ], [ %colorAccumulator.2, %if.end42 ], [ %colorAccumulator.1, %if.end30 ]
  %sub52 = add i64 %call6, -1
  %cmp53 = icmp ult i64 %call2, %sub52
  br i1 %cmp53, label %if.then55, label %if.end73

if.then55:                                        ; preds = %if.end51
  %arrayidx56 = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %add18
  %6 = load <4 x float> addrspace(1)* %arrayidx56, align 16
  %add57 = fadd <4 x float> %colorAccumulator.3, %6
  br i1 %cmp, label %if.end64, label %if.then60

if.then60:                                        ; preds = %if.then55
  %sub61 = add i64 %add18, -1
  %arrayidx62 = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %sub61
  %7 = load <4 x float> addrspace(1)* %arrayidx62, align 16
  %add63 = fadd <4 x float> %add57, %7
  br label %if.end64

if.end64:                                         ; preds = %if.then55, %if.then60
  %colorAccumulator.4 = phi <4 x float> [ %add63, %if.then60 ], [ %add57, %if.then55 ]
  br i1 %cmp24, label %if.then68, label %if.end73

if.then68:                                        ; preds = %if.end64
  %add69 = add i64 %add18, 1
  %arrayidx70 = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %add69
  %8 = load <4 x float> addrspace(1)* %arrayidx70, align 16
  %add71 = fadd <4 x float> %colorAccumulator.4, %8
  br label %if.end73

if.end73:                                         ; preds = %if.end64, %if.then68, %if.end51
  %colorAccumulator.5 = phi <4 x float> [ %add71, %if.then68 ], [ %colorAccumulator.4, %if.end64 ], [ %colorAccumulator.3, %if.end51 ]
  %div = fdiv <4 x float> %colorAccumulator.5, <float 9.000000e+00, float 9.000000e+00, float 9.000000e+00, float 9.000000e+00>
  %arrayidx74 = getelementptr inbounds <4 x float> addrspace(1)* %output, i64 %add
  store <4 x float> %div, <4 x float> addrspace(1)* %arrayidx74, align 16
  ret void
}

declare i64 @get_global_id(i32) nounwind readnone

declare i64 @get_global_size(i32) nounwind readnone

define void @wlSimpleBoxBlur_CPU(<4 x float> addrspace(1)* nocapture %input, <4 x float> addrspace(1)* nocapture %output, i32 %width, i32 %height, i32 %buffer_size) nounwind {
entry:
  %call1 = tail call i64 @get_global_id(i32 0) nounwind readnone
  %call5 = tail call i64 @get_global_size(i32 0) nounwind readnone
  %call6 = tail call i64 @get_global_size(i32 1) nounwind readnone
  %conv11 = zext i32 %width to i64
  %div = udiv i64 %conv11, %call5
  %conv12 = zext i32 %height to i64
  %div13 = udiv i64 %conv12, %call6
  %mul = mul i64 %call1, %conv11
  %div15 = udiv i64 %mul, %call5
  %conv16 = trunc i64 %div15 to i32
  %call2 = tail call i64 @get_global_id(i32 1) nounwind readnone
  %mul18 = mul i64 %call2, %conv12
  %div19 = udiv i64 %mul18, %call6
  %conv20 = trunc i64 %div19 to i32
  %cmp261 = icmp eq i64 %div, 0
  %sub43 = add i32 %width, -1
  %sub72 = add i32 %height, -1
  br label %for.cond24.preheader

for.cond24.preheader:                             ; preds = %entry, %for.inc97
  %i.06 = phi i32 [ 0, %entry ], [ %inc98, %for.inc97 ]
  %index_y.05 = phi i32 [ %conv20, %entry ], [ %inc99.pre-phi, %for.inc97 ]
  br i1 %cmp261, label %for.cond24.preheader.for.inc97_crit_edge, label %for.body28.lr.ph

for.cond24.preheader.for.inc97_crit_edge:         ; preds = %for.cond24.preheader
  %inc99.pre = add i32 %index_y.05, 1
  br label %for.inc97

for.body28.lr.ph:                                 ; preds = %for.cond24.preheader
  %mul29 = mul i32 %index_y.05, %width
  %sub = add i32 %index_y.05, -1
  %mul31 = mul i32 %sub, %width
  %add34 = add i32 %index_y.05, 1
  %mul35 = mul i32 %add34, %width
  %cmp51 = icmp eq i32 %index_y.05, 0
  %cmp73 = icmp ult i32 %index_y.05, %sub72
  br label %for.body28

for.body28:                                       ; preds = %for.body28.lr.ph, %if.end93
  %index_x23.03 = phi i32 [ %conv16, %for.body28.lr.ph ], [ %inc96, %if.end93 ]
  %j.02 = phi i32 [ 0, %for.body28.lr.ph ], [ %inc, %if.end93 ]
  %add = add i32 %index_x23.03, %mul29
  %conv30 = zext i32 %add to i64
  %add32 = add i32 %index_x23.03, %mul31
  %conv33 = zext i32 %add32 to i64
  %add36 = add i32 %index_x23.03, %mul35
  %conv37 = zext i32 %add36 to i64
  %arrayidx = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %conv30
  %0 = load <4 x float> addrspace(1)* %arrayidx, align 16
  %cmp38 = icmp eq i32 %index_x23.03, 0
  br i1 %cmp38, label %if.end, label %if.then

if.then:                                          ; preds = %for.body28
  %sub40 = add i64 %conv30, -1
  %arrayidx41 = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %sub40
  %1 = load <4 x float> addrspace(1)* %arrayidx41, align 16
  %add42 = fadd <4 x float> %0, %1
  br label %if.end

if.end:                                           ; preds = %for.body28, %if.then
  %colorAccumulator.0 = phi <4 x float> [ %add42, %if.then ], [ %0, %for.body28 ]
  %cmp44 = icmp ult i32 %index_x23.03, %sub43
  br i1 %cmp44, label %if.then46, label %if.end50

if.then46:                                        ; preds = %if.end
  %add47 = add i64 %conv30, 1
  %arrayidx48 = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %add47
  %2 = load <4 x float> addrspace(1)* %arrayidx48, align 16
  %add49 = fadd <4 x float> %colorAccumulator.0, %2
  br label %if.end50

if.end50:                                         ; preds = %if.then46, %if.end
  %colorAccumulator.1 = phi <4 x float> [ %add49, %if.then46 ], [ %colorAccumulator.0, %if.end ]
  br i1 %cmp51, label %if.end71, label %if.then53

if.then53:                                        ; preds = %if.end50
  %arrayidx54 = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %conv33
  %3 = load <4 x float> addrspace(1)* %arrayidx54, align 16
  %add55 = fadd <4 x float> %colorAccumulator.1, %3
  br i1 %cmp38, label %if.end62, label %if.then58

if.then58:                                        ; preds = %if.then53
  %sub59 = add i64 %conv33, -1
  %arrayidx60 = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %sub59
  %4 = load <4 x float> addrspace(1)* %arrayidx60, align 16
  %add61 = fadd <4 x float> %add55, %4
  br label %if.end62

if.end62:                                         ; preds = %if.then53, %if.then58
  %colorAccumulator.2 = phi <4 x float> [ %add61, %if.then58 ], [ %add55, %if.then53 ]
  br i1 %cmp44, label %if.then66, label %if.end71

if.then66:                                        ; preds = %if.end62
  %add67 = add i64 %conv33, 1
  %arrayidx68 = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %add67
  %5 = load <4 x float> addrspace(1)* %arrayidx68, align 16
  %add69 = fadd <4 x float> %colorAccumulator.2, %5
  br label %if.end71

if.end71:                                         ; preds = %if.end50, %if.end62, %if.then66
  %colorAccumulator.3 = phi <4 x float> [ %add69, %if.then66 ], [ %colorAccumulator.2, %if.end62 ], [ %colorAccumulator.1, %if.end50 ]
  br i1 %cmp73, label %if.then75, label %if.end93

if.then75:                                        ; preds = %if.end71
  %arrayidx76 = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %conv37
  %6 = load <4 x float> addrspace(1)* %arrayidx76, align 16
  %add77 = fadd <4 x float> %colorAccumulator.3, %6
  br i1 %cmp38, label %if.end84, label %if.then80

if.then80:                                        ; preds = %if.then75
  %sub81 = add i64 %conv37, -1
  %arrayidx82 = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %sub81
  %7 = load <4 x float> addrspace(1)* %arrayidx82, align 16
  %add83 = fadd <4 x float> %add77, %7
  br label %if.end84

if.end84:                                         ; preds = %if.then75, %if.then80
  %colorAccumulator.4 = phi <4 x float> [ %add83, %if.then80 ], [ %add77, %if.then75 ]
  br i1 %cmp44, label %if.then88, label %if.end93

if.then88:                                        ; preds = %if.end84
  %add89 = add i64 %conv37, 1
  %arrayidx90 = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %add89
  %8 = load <4 x float> addrspace(1)* %arrayidx90, align 16
  %add91 = fadd <4 x float> %colorAccumulator.4, %8
  br label %if.end93

if.end93:                                         ; preds = %if.end84, %if.then88, %if.end71
  %colorAccumulator.5 = phi <4 x float> [ %add91, %if.then88 ], [ %colorAccumulator.4, %if.end84 ], [ %colorAccumulator.3, %if.end71 ]
  %div94 = fdiv <4 x float> %colorAccumulator.5, <float 9.000000e+00, float 9.000000e+00, float 9.000000e+00, float 9.000000e+00>
  %arrayidx95 = getelementptr inbounds <4 x float> addrspace(1)* %output, i64 %conv30
  store <4 x float> %div94, <4 x float> addrspace(1)* %arrayidx95, align 16
  %inc = add i32 %j.02, 1
  %inc96 = add i32 %index_x23.03, 1
  %conv25 = zext i32 %inc to i64
  %cmp26 = icmp ult i64 %conv25, %div
  br i1 %cmp26, label %for.body28, label %for.inc97

for.inc97:                                        ; preds = %if.end93, %for.cond24.preheader.for.inc97_crit_edge
  %inc99.pre-phi = phi i32 [ %inc99.pre, %for.cond24.preheader.for.inc97_crit_edge ], [ %add34, %if.end93 ]
  %inc98 = add i32 %i.06, 1
  %conv21 = zext i32 %inc98 to i64
  %cmp = icmp ult i64 %conv21, %div13
  br i1 %cmp, label %for.cond24.preheader, label %for.end100

for.end100:                                       ; preds = %for.inc97
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
  %call = tail call i64 @get_global_id(i32 0) nounwind readnone
  %conv = trunc i64 %call to i32
  %mul = mul i32 %conv, %rowCountPerGlobalID
  %call1 = tail call <2 x i32> @_Z13get_image_dimP10_image2d_t(%struct._image2d_t.0* %inputImage) nounwind readnone
  %add = add nsw i32 %mul, %rowCountPerGlobalID
  %0 = extractelement <2 x i32> %call1, i32 1
  %call2 = tail call i32 @_Z3minii(i32 %add, i32 %0) nounwind readnone
  %1 = extractelement <2 x i32> %call1, i32 0
  %cmp22 = icmp slt i32 %mul, %call2
  br i1 %cmp22, label %for.body.lr.ph, label %for.end40

for.body.lr.ph:                                   ; preds = %entry
  %mul3 = mul nsw i32 %mul, %1
  %cmp111 = icmp sgt i32 %1, 0
  br label %for.body

for.cond.loopexit:                                ; preds = %for.body13, %for.body
  %lowRightCrd.1.lcssa = phi <2 x i32> [ %10, %for.body ], [ %21, %for.body13 ]
  %lowLeftCrd.1.lcssa = phi <2 x i32> [ %9, %for.body ], [ %20, %for.body13 ]
  %lowCrd.1.lcssa = phi <2 x i32> [ %8, %for.body ], [ %19, %for.body13 ]
  %upRightCrd.1.lcssa = phi <2 x i32> [ %7, %for.body ], [ %18, %for.body13 ]
  %upLeftCrd.1.lcssa = phi <2 x i32> [ %6, %for.body ], [ %17, %for.body13 ]
  %index.1.lcssa = phi i32 [ %index.027, %for.body ], [ %12, %for.body13 ]
  %upCrd.1.lcssa = phi <2 x i32> [ %5, %for.body ], [ %16, %for.body13 ]
  %curCrd.1.lcssa = phi <2 x i32> [ %2, %for.body ], [ %13, %for.body13 ]
  %curLeftCrd.1.lcssa = phi <2 x i32> [ %3, %for.body ], [ %14, %for.body13 ]
  %curRightCrd.1.lcssa = phi <2 x i32> [ %4, %for.body ], [ %15, %for.body13 ]
  %exitcond42 = icmp eq i32 %add7, %call2
  br i1 %exitcond42, label %for.end40, label %for.body

for.body:                                         ; preds = %for.cond.loopexit, %for.body.lr.ph
  %lowRightCrd.033 = phi <2 x i32> [ undef, %for.body.lr.ph ], [ %lowRightCrd.1.lcssa, %for.cond.loopexit ]
  %lowLeftCrd.032 = phi <2 x i32> [ undef, %for.body.lr.ph ], [ %lowLeftCrd.1.lcssa, %for.cond.loopexit ]
  %lowCrd.031 = phi <2 x i32> [ undef, %for.body.lr.ph ], [ %lowCrd.1.lcssa, %for.cond.loopexit ]
  %row.030 = phi i32 [ %mul, %for.body.lr.ph ], [ %add7, %for.cond.loopexit ]
  %upRightCrd.029 = phi <2 x i32> [ undef, %for.body.lr.ph ], [ %upRightCrd.1.lcssa, %for.cond.loopexit ]
  %upLeftCrd.028 = phi <2 x i32> [ undef, %for.body.lr.ph ], [ %upLeftCrd.1.lcssa, %for.cond.loopexit ]
  %index.027 = phi i32 [ %mul3, %for.body.lr.ph ], [ %index.1.lcssa, %for.cond.loopexit ]
  %upCrd.026 = phi <2 x i32> [ undef, %for.body.lr.ph ], [ %upCrd.1.lcssa, %for.cond.loopexit ]
  %curCrd.025 = phi <2 x i32> [ undef, %for.body.lr.ph ], [ %curCrd.1.lcssa, %for.cond.loopexit ]
  %curLeftCrd.024 = phi <2 x i32> [ undef, %for.body.lr.ph ], [ %curLeftCrd.1.lcssa, %for.cond.loopexit ]
  %curRightCrd.023 = phi <2 x i32> [ undef, %for.body.lr.ph ], [ %curRightCrd.1.lcssa, %for.cond.loopexit ]
  %2 = insertelement <2 x i32> %curCrd.025, i32 %row.030, i32 1
  %3 = insertelement <2 x i32> %curLeftCrd.024, i32 %row.030, i32 1
  %4 = insertelement <2 x i32> %curRightCrd.023, i32 %row.030, i32 1
  %sub = add nsw i32 %row.030, -1
  %5 = insertelement <2 x i32> %upCrd.026, i32 %sub, i32 1
  %6 = insertelement <2 x i32> %upLeftCrd.028, i32 %sub, i32 1
  %7 = insertelement <2 x i32> %upRightCrd.029, i32 %sub, i32 1
  %add7 = add nsw i32 %row.030, 1
  %8 = insertelement <2 x i32> %lowCrd.031, i32 %add7, i32 1
  %9 = insertelement <2 x i32> %lowLeftCrd.032, i32 %add7, i32 1
  %10 = insertelement <2 x i32> %lowRightCrd.033, i32 %add7, i32 1
  br i1 %cmp111, label %for.body13.lr.ph, label %for.cond.loopexit

for.body13.lr.ph:                                 ; preds = %for.body
  %11 = sext i32 %index.027 to i64
  %12 = add i32 %1, %index.027
  br label %for.body13

for.body13:                                       ; preds = %for.body13, %for.body13.lr.ph
  %indvars.iv = phi i64 [ %11, %for.body13.lr.ph ], [ %indvars.iv.next, %for.body13 ]
  %col.012 = phi i32 [ 0, %for.body13.lr.ph ], [ %add15, %for.body13 ]
  %lowRightCrd.111 = phi <2 x i32> [ %10, %for.body13.lr.ph ], [ %21, %for.body13 ]
  %lowLeftCrd.110 = phi <2 x i32> [ %9, %for.body13.lr.ph ], [ %20, %for.body13 ]
  %lowCrd.19 = phi <2 x i32> [ %8, %for.body13.lr.ph ], [ %19, %for.body13 ]
  %upRightCrd.18 = phi <2 x i32> [ %7, %for.body13.lr.ph ], [ %18, %for.body13 ]
  %upLeftCrd.17 = phi <2 x i32> [ %6, %for.body13.lr.ph ], [ %17, %for.body13 ]
  %upCrd.15 = phi <2 x i32> [ %5, %for.body13.lr.ph ], [ %16, %for.body13 ]
  %curCrd.14 = phi <2 x i32> [ %2, %for.body13.lr.ph ], [ %13, %for.body13 ]
  %curLeftCrd.13 = phi <2 x i32> [ %3, %for.body13.lr.ph ], [ %14, %for.body13 ]
  %curRightCrd.12 = phi <2 x i32> [ %4, %for.body13.lr.ph ], [ %15, %for.body13 ]
  %13 = insertelement <2 x i32> %curCrd.14, i32 %col.012, i32 0
  %sub14 = add nsw i32 %col.012, -1
  %14 = insertelement <2 x i32> %curLeftCrd.13, i32 %sub14, i32 0
  %add15 = add nsw i32 %col.012, 1
  %15 = insertelement <2 x i32> %curRightCrd.12, i32 %add15, i32 0
  %16 = insertelement <2 x i32> %upCrd.15, i32 %col.012, i32 0
  %17 = insertelement <2 x i32> %upLeftCrd.17, i32 %sub14, i32 0
  %18 = insertelement <2 x i32> %upRightCrd.18, i32 %add15, i32 0
  %19 = insertelement <2 x i32> %lowCrd.19, i32 %col.012, i32 0
  %20 = insertelement <2 x i32> %lowLeftCrd.110, i32 %sub14, i32 0
  %21 = insertelement <2 x i32> %lowRightCrd.111, i32 %add15, i32 0
  %call20 = tail call <4 x float> @evaluatePixel(%struct._image2d_t.0* %inputImage, <2 x i32> %13)
  %call21 = tail call <4 x float> @evaluatePixel(%struct._image2d_t.0* %inputImage, <2 x i32> %14)
  %add22 = fadd <4 x float> %call20, %call21
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
  %call35 = tail call <4 x float> @evaluatePixel(%struct._image2d_t.0* %inputImage, <2 x i32> %21)
  %add36 = fadd <4 x float> %add34, %call35
  %div = fdiv <4 x float> %add36, <float 9.000000e+00, float 9.000000e+00, float 9.000000e+00, float 9.000000e+00>
  %indvars.iv.next = add i64 %indvars.iv, 1
  %arrayidx = getelementptr inbounds <4 x float> addrspace(1)* %output, i64 %indvars.iv
  store <4 x float> %div, <4 x float> addrspace(1)* %arrayidx, align 16
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %12
  br i1 %exitcond, label %for.cond.loopexit, label %for.body13

for.end40:                                        ; preds = %for.cond.loopexit, %entry
  ret void
}

declare <2 x i32> @_Z13get_image_dimP10_image2d_t(%struct._image2d_t.0*) nounwind readnone

declare i32 @_Z3minii(i32, i32) nounwind readnone

define void @wlSimpleBoxBlur_Optimized_CPU(<4 x float> addrspace(1)* %input, <4 x float> addrspace(1)* nocapture %output, i32 %width, i32 %height, i32 %buffer_size) nounwind {
entry:
  %call1 = tail call i64 @get_global_id(i32 0) nounwind readnone
  %call2 = tail call i64 @get_global_id(i32 1) nounwind readnone
  %call3 = tail call i64 @get_global_size(i32 0) nounwind readnone
  %call4 = tail call i64 @get_global_size(i32 1) nounwind readnone
  %conv5 = zext i32 %height to i64
  %div = udiv i64 %conv5, %call4
  %conv6 = zext i32 %width to i64
  %div7 = udiv i64 %conv6, %call3
  %mul = mul i64 %call1, %conv6
  %div9 = udiv i64 %mul, %call3
  %conv10 = trunc i64 %div9 to i32
  %mul12 = mul i64 %call2, %conv5
  %div13 = udiv i64 %mul12, %call4
  %conv14 = trunc i64 %div13 to i32
  %conv15 = and i64 %div13, 4294967295
  %add = add i64 %div, 1
  %add16 = add i64 %add, %conv15
  %cmp = icmp ult i64 %add16, %conv5
  br i1 %cmp, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  %sub = add i32 %height, -1
  %sub19 = sub i32 %sub, %conv14
  %conv20 = zext i32 %sub19 to i64
  br label %if.end

if.end:                                           ; preds = %entry, %if.then
  %count_y.0 = phi i64 [ %conv20, %if.then ], [ %div, %entry ]
  %bottomEdge.0 = phi i1 [ true, %if.then ], [ false, %entry ]
  %conv21 = and i64 %div9, 4294967295
  %add22 = add i64 %div7, 1
  %add23 = add i64 %add22, %conv21
  %cmp25 = icmp ult i64 %add23, %conv6
  br i1 %cmp25, label %if.end31, label %if.then27

if.then27:                                        ; preds = %if.end
  %sub28 = add i32 %width, -1
  %sub29 = sub i32 %sub28, %conv10
  %conv30 = zext i32 %sub29 to i64
  br label %if.end31

if.end31:                                         ; preds = %if.end, %if.then27
  %count_x.0 = phi i64 [ %conv30, %if.then27 ], [ %div7, %if.end ]
  %rightEdge.0 = phi i1 [ true, %if.then27 ], [ false, %if.end ]
  %cmp32 = icmp eq i32 %conv14, 0
  %index_y.0 = select i1 %cmp32, i32 1, i32 %conv14
  %cmp37 = icmp eq i32 %conv10, 0
  %index_x.0 = select i1 %cmp37, i32 1, i32 %conv10
  %sub42 = add i32 %index_y.0, -1
  %mul43 = mul i32 %sub42, %width
  %add44 = add i32 %index_x.0, -1
  %sub45 = add i32 %add44, %mul43
  %conv46 = zext i32 %sub45 to i64
  %sub40 = sext i1 %cmp37 to i64
  %sub35 = sext i1 %cmp32 to i64
  %arrayidx = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %conv46
  %0 = load <4 x float> addrspace(1)* %arrayidx, align 16
  %add49 = fadd <4 x float> %0, zeroinitializer
  %add50 = add i64 %conv46, 1
  %arrayidx51 = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %add50
  %1 = load <4 x float> addrspace(1)* %arrayidx51, align 16
  %add52 = fadd <4 x float> %add49, %1
  %add53 = add i64 %conv46, 2
  %arrayidx54 = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %add53
  %2 = load <4 x float> addrspace(1)* %arrayidx54, align 16
  %add55 = fadd <4 x float> %add52, %2
  %add57 = add i64 %conv46, %conv6
  %arrayidx.1 = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %add57
  %3 = load <4 x float> addrspace(1)* %arrayidx.1, align 16
  %add49.1 = fadd <4 x float> %add55, %3
  %add50.1 = add i64 %add57, 1
  %arrayidx51.1 = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %add50.1
  %4 = load <4 x float> addrspace(1)* %arrayidx51.1, align 16
  %add52.1 = fadd <4 x float> %add49.1, %4
  %add53.1 = add i64 %add57, 2
  %arrayidx54.1 = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %add53.1
  %5 = load <4 x float> addrspace(1)* %arrayidx54.1, align 16
  %add55.1 = fadd <4 x float> %add52.1, %5
  %add57.1 = add i64 %add57, %conv6
  %arrayidx.2 = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %add57.1
  %6 = load <4 x float> addrspace(1)* %arrayidx.2, align 16
  %add49.2 = fadd <4 x float> %add55.1, %6
  %add50.2 = add i64 %add57.1, 1
  %arrayidx51.2 = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %add50.2
  %7 = load <4 x float> addrspace(1)* %arrayidx51.2, align 16
  %add52.2 = fadd <4 x float> %add49.2, %7
  %add53.2 = add i64 %add57.1, 2
  %arrayidx54.2 = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %add53.2
  %8 = load <4 x float> addrspace(1)* %arrayidx54.2, align 16
  %add55.2 = fadd <4 x float> %add52.2, %8
  %sub35.count_y.0 = add i64 %count_y.0, %sub35
  %sub40.count_x.0 = add i64 %count_x.0, %sub40
  %mul58 = mul i32 %index_y.0, %width
  %add59 = add i32 %mul58, %index_x.0
  %conv60 = zext i32 %add59 to i64
  %div61 = fdiv <4 x float> %add55.2, <float 9.000000e+00, float 9.000000e+00, float 9.000000e+00, float 9.000000e+00>
  %arrayidx62 = getelementptr inbounds <4 x float> addrspace(1)* %output, i64 %conv60
  store <4 x float> %div61, <4 x float> addrspace(1)* %arrayidx62, align 16
  %sourceIndex.035 = add i64 %conv60, 1
  %sub76 = add i64 %sub35.count_y.0, -1
  %cmp7736 = icmp eq i64 %sub76, 0
  br i1 %cmp7736, label %if.end31.for.cond147.preheader_crit_edge, label %for.cond81.preheader.lr.ph

if.end31.for.cond147.preheader_crit_edge:         ; preds = %if.end31
  %cmp14923.pre = icmp ugt i64 %sub40.count_x.0, 1
  br i1 %cmp14923.pre, label %for.body151, label %for.end180

for.cond81.preheader.lr.ph:                       ; preds = %if.end31
  %add69 = add i32 %index_y.0, 1
  %mul70 = mul i32 %add69, %width
  %sub72 = add i32 %add44, %mul70
  %conv73 = zext i32 %sub72 to i64
  %cmp8329 = icmp ugt i64 %sub40.count_x.0, 1
  br label %for.cond81.preheader

for.cond81.preheader:                             ; preds = %for.cond81.preheader.lr.ph, %for.end114
  %add8043 = phi i64 [ %add53, %for.cond81.preheader.lr.ph ], [ %add80, %for.end114 ]
  %sourceIndex.042 = phi i64 [ %sourceIndex.035, %for.cond81.preheader.lr.ph ], [ %sourceIndex.0, %for.end114 ]
  %row.041 = phi i32 [ 0, %for.cond81.preheader.lr.ph ], [ %inc117, %for.end114 ]
  %bottomRowIndex.040 = phi i64 [ %conv73, %for.cond81.preheader.lr.ph ], [ %add116, %for.end114 ]
  %topRowIndex.039 = phi i64 [ %conv46, %for.cond81.preheader.lr.ph ], [ %add143, %for.end114 ]
  %firstBlockAccumulator.138 = phi <4 x float> [ %add55.2, %for.cond81.preheader.lr.ph ], [ %add133, %for.end114 ]
  br i1 %cmp8329, label %for.body85, label %for.end114

for.cond147.preheader:                            ; preds = %for.end114
  br i1 %cmp8329, label %for.body151, label %for.end180

for.body85:                                       ; preds = %for.cond81.preheader, %for.body85
  %column.034 = phi i32 [ %inc113, %for.body85 ], [ 1, %for.cond81.preheader ]
  %colorAccumulator.133 = phi <4 x float> [ %add106, %for.body85 ], [ %firstBlockAccumulator.138, %for.cond81.preheader ]
  %sourceIndex.132 = phi i64 [ %inc110, %for.body85 ], [ %sourceIndex.042, %for.cond81.preheader ]
  %leftColumnIndex.031 = phi i64 [ %inc111, %for.body85 ], [ %topRowIndex.039, %for.cond81.preheader ]
  %rightColumnIndex.030 = phi i64 [ %inc86, %for.body85 ], [ %add8043, %for.cond81.preheader ]
  %inc86 = add i64 %rightColumnIndex.030, 1
  %arrayidx87 = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %leftColumnIndex.031
  %9 = load <4 x float> addrspace(1)* %arrayidx87, align 16
  %sub88 = fsub <4 x float> %colorAccumulator.133, %9
  %add90 = add i64 %leftColumnIndex.031, %conv6
  %arrayidx91 = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %inc86
  %10 = load <4 x float> addrspace(1)* %arrayidx91, align 16
  %add92 = fadd <4 x float> %sub88, %10
  %add94 = add i64 %inc86, %conv6
  %arrayidx95 = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %add90
  %11 = load <4 x float> addrspace(1)* %arrayidx95, align 16
  %sub96 = fsub <4 x float> %add92, %11
  %add98 = add i64 %add90, %conv6
  %arrayidx99 = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %add94
  %12 = load <4 x float> addrspace(1)* %arrayidx99, align 16
  %add100 = fadd <4 x float> %sub96, %12
  %add102 = add i64 %add94, %conv6
  %arrayidx103 = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %add98
  %13 = load <4 x float> addrspace(1)* %arrayidx103, align 16
  %sub104 = fsub <4 x float> %add100, %13
  %arrayidx105 = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %add102
  %14 = load <4 x float> addrspace(1)* %arrayidx105, align 16
  %add106 = fadd <4 x float> %sub104, %14
  %div108 = fdiv <4 x float> %add106, <float 9.000000e+00, float 9.000000e+00, float 9.000000e+00, float 9.000000e+00>
  %arrayidx109 = getelementptr inbounds <4 x float> addrspace(1)* %output, i64 %sourceIndex.132
  store <4 x float> %div108, <4 x float> addrspace(1)* %arrayidx109, align 16
  %inc110 = add i64 %sourceIndex.132, 1
  %inc111 = add i64 %leftColumnIndex.031, 1
  %inc113 = add i32 %column.034, 1
  %conv82 = zext i32 %inc113 to i64
  %cmp83 = icmp ult i64 %conv82, %sub40.count_x.0
  br i1 %cmp83, label %for.body85, label %for.end114

for.end114:                                       ; preds = %for.body85, %for.cond81.preheader
  %add116 = add i64 %bottomRowIndex.040, %conv6
  %inc117 = add i32 %row.041, 1
  %arrayidx118 = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %topRowIndex.039
  %15 = load <4 x float> addrspace(1)* %arrayidx118, align 16
  %sub119 = fsub <4 x float> %firstBlockAccumulator.138, %15
  %inc120 = add i64 %topRowIndex.039, 1
  %arrayidx121 = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %add116
  %16 = load <4 x float> addrspace(1)* %arrayidx121, align 16
  %add122 = fadd <4 x float> %sub119, %16
  %inc123 = add i64 %add116, 1
  %arrayidx124 = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %inc120
  %17 = load <4 x float> addrspace(1)* %arrayidx124, align 16
  %sub125 = fsub <4 x float> %add122, %17
  %arrayidx127 = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %inc123
  %18 = load <4 x float> addrspace(1)* %arrayidx127, align 16
  %add128 = fadd <4 x float> %sub125, %18
  %inc129 = add i64 %add116, 2
  %arrayidx130 = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %add8043
  %19 = load <4 x float> addrspace(1)* %arrayidx130, align 16
  %sub131 = fsub <4 x float> %add128, %19
  %arrayidx132 = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %inc129
  %20 = load <4 x float> addrspace(1)* %arrayidx132, align 16
  %add133 = fadd <4 x float> %sub131, %20
  %add134 = add i32 %inc117, %index_y.0
  %mul135 = mul i32 %add134, %width
  %add136 = add i32 %mul135, %index_x.0
  %conv137 = zext i32 %add136 to i64
  %div139 = fdiv <4 x float> %add133, <float 9.000000e+00, float 9.000000e+00, float 9.000000e+00, float 9.000000e+00>
  %arrayidx140 = getelementptr inbounds <4 x float> addrspace(1)* %output, i64 %conv137
  store <4 x float> %div139, <4 x float> addrspace(1)* %arrayidx140, align 16
  %add143 = add i64 %topRowIndex.039, %conv6
  %sourceIndex.0 = add i64 %conv137, 1
  %conv75 = zext i32 %inc117 to i64
  %cmp77 = icmp ult i64 %conv75, %sub76
  %add80 = add i64 %add143, 2
  br i1 %cmp77, label %for.cond81.preheader, label %for.cond147.preheader

for.body151:                                      ; preds = %for.cond147.preheader, %if.end31.for.cond147.preheader_crit_edge, %for.body151
  %column146.028 = phi i32 [ %inc179, %for.body151 ], [ 1, %if.end31.for.cond147.preheader_crit_edge ], [ 1, %for.cond147.preheader ]
  %colorAccumulator.227 = phi <4 x float> [ %add172, %for.body151 ], [ %add55.2, %if.end31.for.cond147.preheader_crit_edge ], [ %add133, %for.cond147.preheader ]
  %sourceIndex.226 = phi i64 [ %inc176, %for.body151 ], [ %sourceIndex.035, %if.end31.for.cond147.preheader_crit_edge ], [ %sourceIndex.0, %for.cond147.preheader ]
  %leftColumnIndex.125 = phi i64 [ %inc177, %for.body151 ], [ %conv46, %if.end31.for.cond147.preheader_crit_edge ], [ %add143, %for.cond147.preheader ]
  %rightColumnIndex.124 = phi i64 [ %inc152, %for.body151 ], [ %add53, %if.end31.for.cond147.preheader_crit_edge ], [ %add80, %for.cond147.preheader ]
  %inc152 = add i64 %rightColumnIndex.124, 1
  %arrayidx153 = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %leftColumnIndex.125
  %21 = load <4 x float> addrspace(1)* %arrayidx153, align 16
  %sub154 = fsub <4 x float> %colorAccumulator.227, %21
  %add156 = add i64 %leftColumnIndex.125, %conv6
  %arrayidx157 = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %inc152
  %22 = load <4 x float> addrspace(1)* %arrayidx157, align 16
  %add158 = fadd <4 x float> %sub154, %22
  %add160 = add i64 %inc152, %conv6
  %arrayidx161 = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %add156
  %23 = load <4 x float> addrspace(1)* %arrayidx161, align 16
  %sub162 = fsub <4 x float> %add158, %23
  %add164 = add i64 %add156, %conv6
  %arrayidx165 = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %add160
  %24 = load <4 x float> addrspace(1)* %arrayidx165, align 16
  %add166 = fadd <4 x float> %sub162, %24
  %add168 = add i64 %add160, %conv6
  %arrayidx169 = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %add164
  %25 = load <4 x float> addrspace(1)* %arrayidx169, align 16
  %sub170 = fsub <4 x float> %add166, %25
  %arrayidx171 = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %add168
  %26 = load <4 x float> addrspace(1)* %arrayidx171, align 16
  %add172 = fadd <4 x float> %sub170, %26
  %div174 = fdiv <4 x float> %add172, <float 9.000000e+00, float 9.000000e+00, float 9.000000e+00, float 9.000000e+00>
  %arrayidx175 = getelementptr inbounds <4 x float> addrspace(1)* %output, i64 %sourceIndex.226
  store <4 x float> %div174, <4 x float> addrspace(1)* %arrayidx175, align 16
  %inc176 = add i64 %sourceIndex.226, 1
  %inc177 = add i64 %leftColumnIndex.125, 1
  %inc179 = add i32 %column146.028, 1
  %conv148 = zext i32 %inc179 to i64
  %cmp149 = icmp ult i64 %conv148, %sub40.count_x.0
  br i1 %cmp149, label %for.body151, label %for.end180

for.end180:                                       ; preds = %for.body151, %if.end31.for.cond147.preheader_crit_edge, %for.cond147.preheader
  %topEdge.0.not = xor i1 %cmp32, true
  %leftEdge.0.not = xor i1 %cmp37, true
  %brmerge = or i1 %topEdge.0.not, %leftEdge.0.not
  br i1 %brmerge, label %if.end198, label %if.then184

if.then184:                                       ; preds = %for.end180
  %27 = load <4 x float> addrspace(1)* %input, align 16
  %add186 = fadd <4 x float> %27, zeroinitializer
  %arrayidx187 = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 1
  %28 = load <4 x float> addrspace(1)* %arrayidx187, align 16
  %add188 = fadd <4 x float> %add186, %28
  %arrayidx189 = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %conv6
  %29 = load <4 x float> addrspace(1)* %arrayidx189, align 16
  %add190 = fadd <4 x float> %add188, %29
  %add191 = add i32 %width, 1
  %idxprom192 = zext i32 %add191 to i64
  %arrayidx193 = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %idxprom192
  %30 = load <4 x float> addrspace(1)* %arrayidx193, align 16
  %add194 = fadd <4 x float> %add190, %30
  %div196 = fdiv <4 x float> %add194, <float 9.000000e+00, float 9.000000e+00, float 9.000000e+00, float 9.000000e+00>
  store <4 x float> %div196, <4 x float> addrspace(1)* %output, align 16
  br label %if.end198

if.end198:                                        ; preds = %for.end180, %if.then184
  br i1 %cmp32, label %for.cond202.preheader, label %if.end271

for.cond202.preheader:                            ; preds = %if.end198
  %conv20319 = sext i32 %index_x.0 to i64
  %conv204 = zext i32 %index_x.0 to i64
  %add205 = add i64 %sub40.count_x.0, %conv204
  %cmp20620 = icmp ult i64 %conv20319, %add205
  br i1 %cmp20620, label %for.body208, label %if.end241

for.body208:                                      ; preds = %for.cond202.preheader, %for.body208
  %conv20322 = phi i64 [ %idxprom217, %for.body208 ], [ %conv20319, %for.cond202.preheader ]
  %column201.021 = phi i32 [ %add216, %for.body208 ], [ %index_x.0, %for.cond202.preheader ]
  %sub209 = add nsw i32 %column201.021, -1
  %idxprom210 = sext i32 %sub209 to i64
  %arrayidx211 = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %idxprom210
  %31 = load <4 x float> addrspace(1)* %arrayidx211, align 16
  %add212 = fadd <4 x float> %31, zeroinitializer
  %arrayidx214 = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %conv20322
  %32 = load <4 x float> addrspace(1)* %arrayidx214, align 16
  %add215 = fadd <4 x float> %add212, %32
  %add216 = add nsw i32 %column201.021, 1
  %idxprom217 = sext i32 %add216 to i64
  %arrayidx218 = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %idxprom217
  %33 = load <4 x float> addrspace(1)* %arrayidx218, align 16
  %add219 = fadd <4 x float> %add215, %33
  %add220 = add i32 %column201.021, %width
  %sub221 = add i32 %add220, -1
  %idxprom222 = zext i32 %sub221 to i64
  %arrayidx223 = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %idxprom222
  %34 = load <4 x float> addrspace(1)* %arrayidx223, align 16
  %add224 = fadd <4 x float> %add219, %34
  %idxprom226 = zext i32 %add220 to i64
  %arrayidx227 = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %idxprom226
  %35 = load <4 x float> addrspace(1)* %arrayidx227, align 16
  %add228 = fadd <4 x float> %add224, %35
  %add230 = add i32 %add220, 1
  %idxprom231 = zext i32 %add230 to i64
  %arrayidx232 = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %idxprom231
  %36 = load <4 x float> addrspace(1)* %arrayidx232, align 16
  %add233 = fadd <4 x float> %add228, %36
  %div235 = fdiv <4 x float> %add233, <float 9.000000e+00, float 9.000000e+00, float 9.000000e+00, float 9.000000e+00>
  %arrayidx237 = getelementptr inbounds <4 x float> addrspace(1)* %output, i64 %conv20322
  store <4 x float> %div235, <4 x float> addrspace(1)* %arrayidx237, align 16
  %cmp206 = icmp ult i64 %idxprom217, %add205
  br i1 %cmp206, label %for.body208, label %if.end241

if.end241:                                        ; preds = %for.body208, %for.cond202.preheader
  %rightEdge.0.not = xor i1 %rightEdge.0, true
  %brmerge2 = or i1 %topEdge.0.not, %rightEdge.0.not
  br i1 %brmerge2, label %if.end271, label %if.then247

if.then247:                                       ; preds = %if.end241
  %sub248 = add i32 %width, -2
  %idxprom249 = zext i32 %sub248 to i64
  %arrayidx250 = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %idxprom249
  %37 = load <4 x float> addrspace(1)* %arrayidx250, align 16
  %add251 = fadd <4 x float> %37, zeroinitializer
  %sub252 = add i32 %width, -1
  %idxprom253 = zext i32 %sub252 to i64
  %arrayidx254 = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %idxprom253
  %38 = load <4 x float> addrspace(1)* %arrayidx254, align 16
  %add255 = fadd <4 x float> %add251, %38
  %add256 = shl i32 %width, 1
  %sub257 = add i32 %add256, -2
  %idxprom258 = zext i32 %sub257 to i64
  %arrayidx259 = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %idxprom258
  %39 = load <4 x float> addrspace(1)* %arrayidx259, align 16
  %add260 = fadd <4 x float> %add255, %39
  %sub262 = add i32 %add256, -1
  %idxprom263 = zext i32 %sub262 to i64
  %arrayidx264 = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %idxprom263
  %40 = load <4 x float> addrspace(1)* %arrayidx264, align 16
  %add265 = fadd <4 x float> %add260, %40
  %div267 = fdiv <4 x float> %add265, <float 9.000000e+00, float 9.000000e+00, float 9.000000e+00, float 9.000000e+00>
  %arrayidx270 = getelementptr inbounds <4 x float> addrspace(1)* %output, i64 %idxprom253
  store <4 x float> %div267, <4 x float> addrspace(1)* %arrayidx270, align 16
  br label %if.end271

if.end271:                                        ; preds = %if.end198, %if.end241, %if.then247
  br i1 %cmp37, label %for.cond275.preheader, label %if.end321

for.cond275.preheader:                            ; preds = %if.end271
  %conv27616 = sext i32 %index_y.0 to i64
  %conv277 = zext i32 %index_y.0 to i64
  %add278 = add i64 %sub35.count_y.0, %conv277
  %cmp27917 = icmp ult i64 %conv27616, %add278
  br i1 %cmp27917, label %for.body281, label %if.end321

for.body281:                                      ; preds = %for.cond275.preheader, %for.body281
  %row274.018 = phi i32 [ %add291, %for.body281 ], [ %index_y.0, %for.cond275.preheader ]
  %sub282 = add nsw i32 %row274.018, -1
  %mul283 = mul i32 %sub282, %width
  %idxprom284 = zext i32 %mul283 to i64
  %arrayidx285 = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %idxprom284
  %41 = load <4 x float> addrspace(1)* %arrayidx285, align 16
  %add286 = fadd <4 x float> %41, zeroinitializer
  %mul287 = mul i32 %row274.018, %width
  %idxprom288 = zext i32 %mul287 to i64
  %arrayidx289 = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %idxprom288
  %42 = load <4 x float> addrspace(1)* %arrayidx289, align 16
  %add290 = fadd <4 x float> %add286, %42
  %add291 = add nsw i32 %row274.018, 1
  %mul292 = mul i32 %add291, %width
  %idxprom293 = zext i32 %mul292 to i64
  %arrayidx294 = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %idxprom293
  %43 = load <4 x float> addrspace(1)* %arrayidx294, align 16
  %add295 = fadd <4 x float> %add290, %43
  %add298 = add i32 %mul283, 1
  %idxprom299 = zext i32 %add298 to i64
  %arrayidx300 = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %idxprom299
  %44 = load <4 x float> addrspace(1)* %arrayidx300, align 16
  %add301 = fadd <4 x float> %add295, %44
  %add303 = add i32 %mul287, 1
  %idxprom304 = zext i32 %add303 to i64
  %arrayidx305 = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %idxprom304
  %45 = load <4 x float> addrspace(1)* %arrayidx305, align 16
  %add306 = fadd <4 x float> %add301, %45
  %add309 = add i32 %mul292, 1
  %idxprom310 = zext i32 %add309 to i64
  %arrayidx311 = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %idxprom310
  %46 = load <4 x float> addrspace(1)* %arrayidx311, align 16
  %add312 = fadd <4 x float> %add306, %46
  %div314 = fdiv <4 x float> %add312, <float 9.000000e+00, float 9.000000e+00, float 9.000000e+00, float 9.000000e+00>
  %arrayidx317 = getelementptr inbounds <4 x float> addrspace(1)* %output, i64 %idxprom288
  store <4 x float> %div314, <4 x float> addrspace(1)* %arrayidx317, align 16
  %conv276 = sext i32 %add291 to i64
  %cmp279 = icmp ult i64 %conv276, %add278
  br i1 %cmp279, label %for.body281, label %if.end321

if.end321:                                        ; preds = %for.cond275.preheader, %for.body281, %if.end271
  %bottomEdge.0.not = xor i1 %bottomEdge.0, true
  %brmerge4 = or i1 %bottomEdge.0.not, %leftEdge.0.not
  br i1 %brmerge4, label %if.end356, label %if.then327

if.then327:                                       ; preds = %if.end321
  %sub328 = add i32 %height, -2
  %mul329 = mul i32 %sub328, %width
  %idxprom330 = zext i32 %mul329 to i64
  %arrayidx331 = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %idxprom330
  %47 = load <4 x float> addrspace(1)* %arrayidx331, align 16
  %add332 = fadd <4 x float> %47, zeroinitializer
  %add335 = add i32 %mul329, 1
  %idxprom336 = zext i32 %add335 to i64
  %arrayidx337 = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %idxprom336
  %48 = load <4 x float> addrspace(1)* %arrayidx337, align 16
  %add338 = fadd <4 x float> %add332, %48
  %sub339 = add i32 %height, -1
  %mul340 = mul i32 %sub339, %width
  %idxprom341 = zext i32 %mul340 to i64
  %arrayidx342 = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %idxprom341
  %49 = load <4 x float> addrspace(1)* %arrayidx342, align 16
  %add343 = fadd <4 x float> %add338, %49
  %add346 = add i32 %mul340, 1
  %idxprom347 = zext i32 %add346 to i64
  %arrayidx348 = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %idxprom347
  %50 = load <4 x float> addrspace(1)* %arrayidx348, align 16
  %add349 = fadd <4 x float> %add343, %50
  %div351 = fdiv <4 x float> %add349, <float 9.000000e+00, float 9.000000e+00, float 9.000000e+00, float 9.000000e+00>
  %arrayidx355 = getelementptr inbounds <4 x float> addrspace(1)* %output, i64 %idxprom341
  store <4 x float> %div351, <4 x float> addrspace(1)* %arrayidx355, align 16
  br label %if.end356

if.end356:                                        ; preds = %if.end321, %if.then327
  br i1 %bottomEdge.0, label %for.cond360.preheader, label %if.end417

for.cond360.preheader:                            ; preds = %if.end356
  %conv36113 = sext i32 %index_x.0 to i64
  %conv362 = zext i32 %index_x.0 to i64
  %add363 = add i64 %sub40.count_x.0, %conv362
  %cmp36414 = icmp ult i64 %conv36113, %add363
  br i1 %cmp36414, label %for.body366.lr.ph, label %if.end417

for.body366.lr.ph:                                ; preds = %for.cond360.preheader
  %sub367 = add i32 %height, -2
  %mul368 = mul i32 %sub367, %width
  %sub387 = add i32 %height, -1
  %mul388 = mul i32 %sub387, %width
  %51 = icmp ugt i32 %conv10, 1
  %umax = select i1 %51, i32 %conv10, i32 1
  %52 = sext i32 %umax to i64
  %53 = zext i32 %umax to i64
  %54 = add i64 %sub40.count_x.0, %53
  br label %for.body366

for.body366:                                      ; preds = %for.body366, %for.body366.lr.ph
  %indvars.iv = phi i64 [ %52, %for.body366.lr.ph ], [ %indvars.iv.next, %for.body366 ]
  %55 = trunc i64 %indvars.iv to i32
  %add369 = add i32 %55, %mul368
  %sub370 = add i32 %add369, -1
  %idxprom371 = zext i32 %sub370 to i64
  %arrayidx372 = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %idxprom371
  %56 = load <4 x float> addrspace(1)* %arrayidx372, align 16
  %add373 = fadd <4 x float> %56, zeroinitializer
  %idxprom377 = zext i32 %add369 to i64
  %arrayidx378 = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %idxprom377
  %57 = load <4 x float> addrspace(1)* %arrayidx378, align 16
  %add379 = fadd <4 x float> %add373, %57
  %add383 = add i32 %add369, 1
  %idxprom384 = zext i32 %add383 to i64
  %arrayidx385 = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %idxprom384
  %58 = load <4 x float> addrspace(1)* %arrayidx385, align 16
  %add386 = fadd <4 x float> %add379, %58
  %add389 = add i32 %55, %mul388
  %sub390 = add i32 %add389, -1
  %idxprom391 = zext i32 %sub390 to i64
  %arrayidx392 = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %idxprom391
  %59 = load <4 x float> addrspace(1)* %arrayidx392, align 16
  %add393 = fadd <4 x float> %add386, %59
  %idxprom397 = zext i32 %add389 to i64
  %arrayidx398 = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %idxprom397
  %60 = load <4 x float> addrspace(1)* %arrayidx398, align 16
  %add399 = fadd <4 x float> %add393, %60
  %add403 = add i32 %add389, 1
  %idxprom404 = zext i32 %add403 to i64
  %arrayidx405 = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %idxprom404
  %61 = load <4 x float> addrspace(1)* %arrayidx405, align 16
  %add406 = fadd <4 x float> %add399, %61
  %div408 = fdiv <4 x float> %add406, <float 9.000000e+00, float 9.000000e+00, float 9.000000e+00, float 9.000000e+00>
  %arrayidx413 = getelementptr inbounds <4 x float> addrspace(1)* %output, i64 %idxprom397
  store <4 x float> %div408, <4 x float> addrspace(1)* %arrayidx413, align 16
  %indvars.iv.next = add i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %54
  br i1 %exitcond, label %if.end417, label %for.body366

if.end417:                                        ; preds = %for.cond360.preheader, %for.body366, %if.end356
  br i1 %rightEdge.0, label %for.cond421.preheader, label %if.end507

for.cond421.preheader:                            ; preds = %if.end417
  %conv42210 = sext i32 %index_y.0 to i64
  %conv423 = zext i32 %index_y.0 to i64
  %add424 = add i64 %sub35.count_y.0, %conv423
  %cmp42511 = icmp ult i64 %conv42210, %add424
  br i1 %cmp42511, label %for.body427, label %if.end472

for.body427:                                      ; preds = %for.cond421.preheader, %for.body427
  %row420.012 = phi i32 [ %add433, %for.body427 ], [ %index_y.0, %for.cond421.preheader ]
  %mul428 = mul i32 %row420.012, %width
  %sub429 = add i32 %mul428, -1
  %idxprom430 = zext i32 %sub429 to i64
  %arrayidx431 = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %idxprom430
  %62 = load <4 x float> addrspace(1)* %arrayidx431, align 16
  %add432 = fadd <4 x float> %62, zeroinitializer
  %add433 = add nsw i32 %row420.012, 1
  %mul434 = mul i32 %add433, %width
  %sub435 = add i32 %mul434, -1
  %idxprom436 = zext i32 %sub435 to i64
  %arrayidx437 = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %idxprom436
  %63 = load <4 x float> addrspace(1)* %arrayidx437, align 16
  %add438 = fadd <4 x float> %add432, %63
  %add439 = add nsw i32 %row420.012, 2
  %mul440 = mul i32 %add439, %width
  %sub441 = add i32 %mul440, -1
  %idxprom442 = zext i32 %sub441 to i64
  %arrayidx443 = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %idxprom442
  %64 = load <4 x float> addrspace(1)* %arrayidx443, align 16
  %add444 = fadd <4 x float> %add438, %64
  %sub446 = add i32 %mul428, -2
  %idxprom447 = zext i32 %sub446 to i64
  %arrayidx448 = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %idxprom447
  %65 = load <4 x float> addrspace(1)* %arrayidx448, align 16
  %add449 = fadd <4 x float> %add444, %65
  %sub452 = add i32 %mul434, -2
  %idxprom453 = zext i32 %sub452 to i64
  %arrayidx454 = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %idxprom453
  %66 = load <4 x float> addrspace(1)* %arrayidx454, align 16
  %add455 = fadd <4 x float> %add449, %66
  %sub458 = add i32 %mul440, -2
  %idxprom459 = zext i32 %sub458 to i64
  %arrayidx460 = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %idxprom459
  %67 = load <4 x float> addrspace(1)* %arrayidx460, align 16
  %add461 = fadd <4 x float> %add455, %67
  %div463 = fdiv <4 x float> %add461, <float 9.000000e+00, float 9.000000e+00, float 9.000000e+00, float 9.000000e+00>
  %arrayidx468 = getelementptr inbounds <4 x float> addrspace(1)* %output, i64 %idxprom436
  store <4 x float> %div463, <4 x float> addrspace(1)* %arrayidx468, align 16
  %conv422 = sext i32 %add433 to i64
  %cmp425 = icmp ult i64 %conv422, %add424
  br i1 %cmp425, label %for.body427, label %if.end472

if.end472:                                        ; preds = %for.body427, %for.cond421.preheader
  br i1 %bottomEdge.0, label %if.then478, label %if.end507

if.then478:                                       ; preds = %if.end472
  %sub479 = add i32 %height, -1
  %mul480 = mul i32 %sub479, %width
  %sub481 = add i32 %mul480, -2
  %idxprom482 = zext i32 %sub481 to i64
  %arrayidx483 = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %idxprom482
  %68 = load <4 x float> addrspace(1)* %arrayidx483, align 16
  %add484 = fadd <4 x float> %68, zeroinitializer
  %sub487 = add i32 %mul480, -1
  %idxprom488 = zext i32 %sub487 to i64
  %arrayidx489 = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %idxprom488
  %69 = load <4 x float> addrspace(1)* %arrayidx489, align 16
  %add490 = fadd <4 x float> %add484, %69
  %mul491 = mul i32 %height, %width
  %sub492 = add i32 %mul491, -2
  %idxprom493 = zext i32 %sub492 to i64
  %arrayidx494 = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %idxprom493
  %70 = load <4 x float> addrspace(1)* %arrayidx494, align 16
  %add495 = fadd <4 x float> %add490, %70
  %sub497 = add i32 %mul491, -1
  %idxprom498 = zext i32 %sub497 to i64
  %arrayidx499 = getelementptr inbounds <4 x float> addrspace(1)* %input, i64 %idxprom498
  %71 = load <4 x float> addrspace(1)* %arrayidx499, align 16
  %add500 = fadd <4 x float> %add495, %71
  %div502 = fdiv <4 x float> %add500, <float 9.000000e+00, float 9.000000e+00, float 9.000000e+00, float 9.000000e+00>
  %arrayidx506 = getelementptr inbounds <4 x float> addrspace(1)* %output, i64 %idxprom498
  store <4 x float> %div502, <4 x float> addrspace(1)* %arrayidx506, align 16
  br label %if.end507

if.end507:                                        ; preds = %if.end472, %if.end417, %if.then478
  ret void
}

define [7 x i64] @WG.boundaries.wlSimpleBoxBlur_GPU(<4 x float> addrspace(1)*, <4 x float> addrspace(1)*, i32, i32, i32) {
entry:
  %5 = call i64 @get_local_size(i32 0)
  %6 = call i64 @get_base_global_id.(i32 0)
  %7 = call i64 @get_local_size(i32 1)
  %8 = call i64 @get_base_global_id.(i32 1)
  %9 = call i64 @get_local_size(i32 2)
  %10 = call i64 @get_base_global_id.(i32 2)
  %11 = insertvalue [7 x i64] undef, i64 %5, 2
  %12 = insertvalue [7 x i64] %11, i64 %6, 1
  %13 = insertvalue [7 x i64] %12, i64 %7, 4
  %14 = insertvalue [7 x i64] %13, i64 %8, 3
  %15 = insertvalue [7 x i64] %14, i64 %9, 6
  %16 = insertvalue [7 x i64] %15, i64 %10, 5
  %17 = insertvalue [7 x i64] %16, i64 1, 0
  ret [7 x i64] %17
}

declare i64 @get_local_size(i32)

declare i64 @get_base_global_id.(i32)

define [7 x i64] @WG.boundaries.wlSimpleBoxBlur_CPU(<4 x float> addrspace(1)*, <4 x float> addrspace(1)*, i32, i32, i32) {
entry:
  %5 = call i64 @get_local_size(i32 0)
  %6 = call i64 @get_base_global_id.(i32 0)
  %7 = call i64 @get_local_size(i32 1)
  %8 = call i64 @get_base_global_id.(i32 1)
  %9 = call i64 @get_local_size(i32 2)
  %10 = call i64 @get_base_global_id.(i32 2)
  %11 = tail call i64 @get_global_size(i32 1) nounwind readnone
  %12 = zext i32 %3 to i64
  %13 = udiv i64 %12, %11
  %14 = icmp eq i64 %13, 0
  %15 = xor i1 %14, true
  %16 = and i1 true, %15
  %zext_cast = zext i1 %16 to i64
  %17 = insertvalue [7 x i64] undef, i64 %5, 2
  %18 = insertvalue [7 x i64] %17, i64 %6, 1
  %19 = insertvalue [7 x i64] %18, i64 %7, 4
  %20 = insertvalue [7 x i64] %19, i64 %8, 3
  %21 = insertvalue [7 x i64] %20, i64 %9, 6
  %22 = insertvalue [7 x i64] %21, i64 %10, 5
  %23 = insertvalue [7 x i64] %22, i64 %zext_cast, 0
  ret [7 x i64] %23
}

define [7 x i64] @WG.boundaries.wlSimpleBoxBlur_image2d(%struct._image2d_t.0*, <4 x float> addrspace(1)*, i32) {
entry:
  %3 = call i64 @get_local_size(i32 0)
  %4 = call i64 @get_base_global_id.(i32 0)
  %5 = call i64 @get_local_size(i32 1)
  %6 = call i64 @get_base_global_id.(i32 1)
  %7 = call i64 @get_local_size(i32 2)
  %8 = call i64 @get_base_global_id.(i32 2)
  %9 = insertvalue [7 x i64] undef, i64 %3, 2
  %10 = insertvalue [7 x i64] %9, i64 %4, 1
  %11 = insertvalue [7 x i64] %10, i64 %5, 4
  %12 = insertvalue [7 x i64] %11, i64 %6, 3
  %13 = insertvalue [7 x i64] %12, i64 %7, 6
  %14 = insertvalue [7 x i64] %13, i64 %8, 5
  %15 = insertvalue [7 x i64] %14, i64 1, 0
  ret [7 x i64] %15
}

define [7 x i64] @WG.boundaries.wlSimpleBoxBlur_Optimized_CPU(<4 x float> addrspace(1)*, <4 x float> addrspace(1)*, i32, i32, i32) {
entry:
  %5 = call i64 @get_local_size(i32 0)
  %6 = call i64 @get_base_global_id.(i32 0)
  %7 = call i64 @get_local_size(i32 1)
  %8 = call i64 @get_base_global_id.(i32 1)
  %9 = call i64 @get_local_size(i32 2)
  %10 = call i64 @get_base_global_id.(i32 2)
  %11 = insertvalue [7 x i64] undef, i64 %5, 2
  %12 = insertvalue [7 x i64] %11, i64 %6, 1
  %13 = insertvalue [7 x i64] %12, i64 %7, 4
  %14 = insertvalue [7 x i64] %13, i64 %8, 3
  %15 = insertvalue [7 x i64] %14, i64 %9, 6
  %16 = insertvalue [7 x i64] %15, i64 %10, 5
  %17 = insertvalue [7 x i64] %16, i64 1, 0
  ret [7 x i64] %17
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
