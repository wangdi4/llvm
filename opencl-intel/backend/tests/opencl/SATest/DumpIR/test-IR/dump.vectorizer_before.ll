; ModuleID = 'Program'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S32"
target triple = "i686-pc-win32"

%struct._image2d_t.0 = type opaque

define void @wlSimpleBoxBlur_GPU(<4 x float> addrspace(1)* nocapture %input, <4 x float> addrspace(1)* nocapture %output, i32 %width, i32 %height, i32 %buffer_size) nounwind {
  %1 = tail call i32 @get_global_id(i32 0) nounwind readnone
  %2 = tail call i32 @get_global_id(i32 1) nounwind readnone
  %3 = tail call i32 @get_global_size(i32 0) nounwind readnone
  %4 = tail call i32 @get_global_size(i32 1) nounwind readnone
  %5 = mul i32 %2, %width
  %6 = add i32 %5, %1
  %7 = add i32 %2, -1
  %8 = mul i32 %7, %width
  %9 = add i32 %8, %1
  %10 = add i32 %2, 1
  %11 = mul i32 %10, %width
  %12 = add i32 %11, %1
  %13 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %6
  %14 = load <4 x float> addrspace(1)* %13, align 16
  %15 = icmp eq i32 %1, 0
  br i1 %15, label %21, label %16

; <label>:16                                      ; preds = %0
  %17 = add i32 %6, -1
  %18 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %17
  %19 = load <4 x float> addrspace(1)* %18, align 16
  %20 = fadd <4 x float> %14, %19
  br label %21

; <label>:21                                      ; preds = %0, %16
  %colorAccumulator.0 = phi <4 x float> [ %20, %16 ], [ %14, %0 ]
  %22 = add i32 %3, -1
  %23 = icmp ult i32 %1, %22
  br i1 %23, label %24, label %29

; <label>:24                                      ; preds = %21
  %25 = add i32 %6, 1
  %26 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %25
  %27 = load <4 x float> addrspace(1)* %26, align 16
  %28 = fadd <4 x float> %colorAccumulator.0, %27
  br label %29

; <label>:29                                      ; preds = %24, %21
  %colorAccumulator.1 = phi <4 x float> [ %28, %24 ], [ %colorAccumulator.0, %21 ]
  %30 = icmp eq i32 %2, 0
  br i1 %30, label %46, label %31

; <label>:31                                      ; preds = %29
  %32 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %9
  %33 = load <4 x float> addrspace(1)* %32, align 16
  %34 = fadd <4 x float> %colorAccumulator.1, %33
  br i1 %15, label %40, label %35

; <label>:35                                      ; preds = %31
  %36 = add i32 %9, -1
  %37 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %36
  %38 = load <4 x float> addrspace(1)* %37, align 16
  %39 = fadd <4 x float> %34, %38
  br label %40

; <label>:40                                      ; preds = %31, %35
  %colorAccumulator.2 = phi <4 x float> [ %39, %35 ], [ %34, %31 ]
  br i1 %23, label %41, label %46

; <label>:41                                      ; preds = %40
  %42 = add i32 %9, 1
  %43 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %42
  %44 = load <4 x float> addrspace(1)* %43, align 16
  %45 = fadd <4 x float> %colorAccumulator.2, %44
  br label %46

; <label>:46                                      ; preds = %29, %40, %41
  %colorAccumulator.3 = phi <4 x float> [ %45, %41 ], [ %colorAccumulator.2, %40 ], [ %colorAccumulator.1, %29 ]
  %47 = add i32 %4, -1
  %48 = icmp ult i32 %2, %47
  br i1 %48, label %49, label %64

; <label>:49                                      ; preds = %46
  %50 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %12
  %51 = load <4 x float> addrspace(1)* %50, align 16
  %52 = fadd <4 x float> %colorAccumulator.3, %51
  br i1 %15, label %58, label %53

; <label>:53                                      ; preds = %49
  %54 = add i32 %12, -1
  %55 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %54
  %56 = load <4 x float> addrspace(1)* %55, align 16
  %57 = fadd <4 x float> %52, %56
  br label %58

; <label>:58                                      ; preds = %49, %53
  %colorAccumulator.4 = phi <4 x float> [ %57, %53 ], [ %52, %49 ]
  br i1 %23, label %59, label %64

; <label>:59                                      ; preds = %58
  %60 = add i32 %12, 1
  %61 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %60
  %62 = load <4 x float> addrspace(1)* %61, align 16
  %63 = fadd <4 x float> %colorAccumulator.4, %62
  br label %64

; <label>:64                                      ; preds = %58, %59, %46
  %colorAccumulator.5 = phi <4 x float> [ %63, %59 ], [ %colorAccumulator.4, %58 ], [ %colorAccumulator.3, %46 ]
  %65 = fdiv <4 x float> %colorAccumulator.5, <float 9.000000e+00, float 9.000000e+00, float 9.000000e+00, float 9.000000e+00>
  %66 = getelementptr inbounds <4 x float> addrspace(1)* %output, i32 %6
  store <4 x float> %65, <4 x float> addrspace(1)* %66, align 16
  ret void
}

declare i32 @get_global_id(i32) nounwind readnone

declare i32 @get_global_size(i32) nounwind readnone

define void @wlSimpleBoxBlur_CPU(<4 x float> addrspace(1)* nocapture %input, <4 x float> addrspace(1)* nocapture %output, i32 %width, i32 %height, i32 %buffer_size) nounwind {
  %1 = tail call i32 @get_global_id(i32 0) nounwind readnone
  %2 = tail call i32 @get_global_size(i32 0) nounwind readnone
  %3 = tail call i32 @get_global_size(i32 1) nounwind readnone
  %4 = udiv i32 %width, %2
  %5 = udiv i32 %height, %3
  %6 = mul i32 %1, %width
  %7 = udiv i32 %6, %2
  %8 = icmp eq i32 %5, 0
  br i1 %8, label %._crit_edge5, label %.preheader.lr.ph

.preheader.lr.ph:                                 ; preds = %0
  %9 = tail call i32 @get_global_id(i32 1) nounwind readnone
  %10 = mul i32 %9, %height
  %11 = udiv i32 %10, %3
  %12 = icmp eq i32 %4, 0
  %13 = add i32 %width, -1
  %14 = add i32 %height, -1
  br label %.preheader

.preheader:                                       ; preds = %.preheader.lr.ph, %._crit_edge
  %i.04 = phi i32 [ 0, %.preheader.lr.ph ], [ %79, %._crit_edge ]
  %index_y.03 = phi i32 [ %11, %.preheader.lr.ph ], [ %.pre-phi, %._crit_edge ]
  br i1 %12, label %.preheader._crit_edge, label %.lr.ph

.preheader._crit_edge:                            ; preds = %.preheader
  %.pre = add i32 %index_y.03, 1
  br label %._crit_edge

.lr.ph:                                           ; preds = %.preheader
  %15 = mul i32 %index_y.03, %width
  %16 = add i32 %index_y.03, -1
  %17 = mul i32 %16, %width
  %18 = add i32 %index_y.03, 1
  %19 = mul i32 %18, %width
  %20 = icmp eq i32 %index_y.03, 0
  %21 = icmp ult i32 %index_y.03, %14
  br label %22

; <label>:22                                      ; preds = %.lr.ph, %73
  %index_x1.02 = phi i32 [ %7, %.lr.ph ], [ %77, %73 ]
  %j.01 = phi i32 [ 0, %.lr.ph ], [ %76, %73 ]
  %23 = add i32 %index_x1.02, %15
  %24 = add i32 %index_x1.02, %17
  %25 = add i32 %index_x1.02, %19
  %26 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %23
  %27 = load <4 x float> addrspace(1)* %26, align 16
  %28 = icmp eq i32 %index_x1.02, 0
  br i1 %28, label %34, label %29

; <label>:29                                      ; preds = %22
  %30 = add i32 %23, -1
  %31 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %30
  %32 = load <4 x float> addrspace(1)* %31, align 16
  %33 = fadd <4 x float> %27, %32
  br label %34

; <label>:34                                      ; preds = %22, %29
  %colorAccumulator.0 = phi <4 x float> [ %33, %29 ], [ %27, %22 ]
  %35 = icmp ult i32 %index_x1.02, %13
  br i1 %35, label %36, label %41

; <label>:36                                      ; preds = %34
  %37 = add i32 %23, 1
  %38 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %37
  %39 = load <4 x float> addrspace(1)* %38, align 16
  %40 = fadd <4 x float> %colorAccumulator.0, %39
  br label %41

; <label>:41                                      ; preds = %36, %34
  %colorAccumulator.1 = phi <4 x float> [ %40, %36 ], [ %colorAccumulator.0, %34 ]
  br i1 %20, label %57, label %42

; <label>:42                                      ; preds = %41
  %43 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %24
  %44 = load <4 x float> addrspace(1)* %43, align 16
  %45 = fadd <4 x float> %colorAccumulator.1, %44
  br i1 %28, label %51, label %46

; <label>:46                                      ; preds = %42
  %47 = add i32 %24, -1
  %48 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %47
  %49 = load <4 x float> addrspace(1)* %48, align 16
  %50 = fadd <4 x float> %45, %49
  br label %51

; <label>:51                                      ; preds = %42, %46
  %colorAccumulator.2 = phi <4 x float> [ %50, %46 ], [ %45, %42 ]
  br i1 %35, label %52, label %57

; <label>:52                                      ; preds = %51
  %53 = add i32 %24, 1
  %54 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %53
  %55 = load <4 x float> addrspace(1)* %54, align 16
  %56 = fadd <4 x float> %colorAccumulator.2, %55
  br label %57

; <label>:57                                      ; preds = %41, %51, %52
  %colorAccumulator.3 = phi <4 x float> [ %56, %52 ], [ %colorAccumulator.2, %51 ], [ %colorAccumulator.1, %41 ]
  br i1 %21, label %58, label %73

; <label>:58                                      ; preds = %57
  %59 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %25
  %60 = load <4 x float> addrspace(1)* %59, align 16
  %61 = fadd <4 x float> %colorAccumulator.3, %60
  br i1 %28, label %67, label %62

; <label>:62                                      ; preds = %58
  %63 = add i32 %25, -1
  %64 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %63
  %65 = load <4 x float> addrspace(1)* %64, align 16
  %66 = fadd <4 x float> %61, %65
  br label %67

; <label>:67                                      ; preds = %58, %62
  %colorAccumulator.4 = phi <4 x float> [ %66, %62 ], [ %61, %58 ]
  br i1 %35, label %68, label %73

; <label>:68                                      ; preds = %67
  %69 = add i32 %25, 1
  %70 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %69
  %71 = load <4 x float> addrspace(1)* %70, align 16
  %72 = fadd <4 x float> %colorAccumulator.4, %71
  br label %73

; <label>:73                                      ; preds = %67, %68, %57
  %colorAccumulator.5 = phi <4 x float> [ %72, %68 ], [ %colorAccumulator.4, %67 ], [ %colorAccumulator.3, %57 ]
  %74 = fdiv <4 x float> %colorAccumulator.5, <float 9.000000e+00, float 9.000000e+00, float 9.000000e+00, float 9.000000e+00>
  %75 = getelementptr inbounds <4 x float> addrspace(1)* %output, i32 %23
  store <4 x float> %74, <4 x float> addrspace(1)* %75, align 16
  %76 = add i32 %j.01, 1
  %77 = add i32 %index_x1.02, 1
  %78 = icmp ult i32 %76, %4
  br i1 %78, label %22, label %._crit_edge

._crit_edge:                                      ; preds = %73, %.preheader._crit_edge
  %.pre-phi = phi i32 [ %.pre, %.preheader._crit_edge ], [ %18, %73 ]
  %79 = add i32 %i.04, 1
  %80 = icmp ult i32 %79, %5
  br i1 %80, label %.preheader, label %._crit_edge5

._crit_edge5:                                     ; preds = %._crit_edge, %0
  ret void
}

define <4 x float> @evaluatePixel(%struct._image2d_t.0* %inputImage, <2 x i32> %outCrd) nounwind {
  %1 = tail call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t.0* %inputImage, i32 1, <2 x i32> %outCrd) nounwind readnone
  ret <4 x float> %1
}

declare <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t.0*, i32, <2 x i32>) nounwind readnone

define void @wlSimpleBoxBlur_image2d(%struct._image2d_t.0* %inputImage, <4 x float> addrspace(1)* nocapture %output, i32 %rowCountPerGlobalID) nounwind {
  %1 = tail call i32 @get_global_id(i32 0) nounwind readnone
  %2 = mul i32 %1, %rowCountPerGlobalID
  %3 = tail call <2 x i32> @_Z13get_image_dimP10_image2d_t(%struct._image2d_t.0* %inputImage) nounwind readnone
  %4 = add nsw i32 %2, %rowCountPerGlobalID
  %5 = extractelement <2 x i32> %3, i32 1
  %6 = tail call i32 @_Z3minii(i32 %4, i32 %5) nounwind readnone
  %7 = extractelement <2 x i32> %3, i32 0
  %8 = icmp slt i32 %2, %6
  br i1 %8, label %.lr.ph32, label %._crit_edge

.lr.ph32:                                         ; preds = %0
  %9 = mul nsw i32 %7, %2
  %10 = icmp sgt i32 %7, 0
  br label %12

..loopexit_crit_edge:                             ; preds = %.lr.ph
  %11 = add i32 %7, %index.025
  br label %.loopexit

.loopexit:                                        ; preds = %..loopexit_crit_edge, %12
  %lowRightCrd.1.lcssa = phi <2 x i32> [ %34, %..loopexit_crit_edge ], [ %23, %12 ]
  %lowLeftCrd.1.lcssa = phi <2 x i32> [ %33, %..loopexit_crit_edge ], [ %22, %12 ]
  %lowCrd.1.lcssa = phi <2 x i32> [ %32, %..loopexit_crit_edge ], [ %21, %12 ]
  %upRightCrd.1.lcssa = phi <2 x i32> [ %31, %..loopexit_crit_edge ], [ %19, %12 ]
  %upLeftCrd.1.lcssa = phi <2 x i32> [ %30, %..loopexit_crit_edge ], [ %18, %12 ]
  %index.1.lcssa = phi i32 [ %11, %..loopexit_crit_edge ], [ %index.025, %12 ]
  %upCrd.1.lcssa = phi <2 x i32> [ %29, %..loopexit_crit_edge ], [ %17, %12 ]
  %curCrd.1.lcssa = phi <2 x i32> [ %24, %..loopexit_crit_edge ], [ %13, %12 ]
  %curLeftCrd.1.lcssa = phi <2 x i32> [ %26, %..loopexit_crit_edge ], [ %14, %12 ]
  %curRightCrd.1.lcssa = phi <2 x i32> [ %28, %..loopexit_crit_edge ], [ %15, %12 ]
  %exitcond42 = icmp eq i32 %20, %6
  br i1 %exitcond42, label %._crit_edge, label %12

; <label>:12                                      ; preds = %.loopexit, %.lr.ph32
  %lowRightCrd.031 = phi <2 x i32> [ undef, %.lr.ph32 ], [ %lowRightCrd.1.lcssa, %.loopexit ]
  %lowLeftCrd.030 = phi <2 x i32> [ undef, %.lr.ph32 ], [ %lowLeftCrd.1.lcssa, %.loopexit ]
  %lowCrd.029 = phi <2 x i32> [ undef, %.lr.ph32 ], [ %lowCrd.1.lcssa, %.loopexit ]
  %row.028 = phi i32 [ %2, %.lr.ph32 ], [ %20, %.loopexit ]
  %upRightCrd.027 = phi <2 x i32> [ undef, %.lr.ph32 ], [ %upRightCrd.1.lcssa, %.loopexit ]
  %upLeftCrd.026 = phi <2 x i32> [ undef, %.lr.ph32 ], [ %upLeftCrd.1.lcssa, %.loopexit ]
  %index.025 = phi i32 [ %9, %.lr.ph32 ], [ %index.1.lcssa, %.loopexit ]
  %upCrd.024 = phi <2 x i32> [ undef, %.lr.ph32 ], [ %upCrd.1.lcssa, %.loopexit ]
  %curCrd.023 = phi <2 x i32> [ undef, %.lr.ph32 ], [ %curCrd.1.lcssa, %.loopexit ]
  %curLeftCrd.022 = phi <2 x i32> [ undef, %.lr.ph32 ], [ %curLeftCrd.1.lcssa, %.loopexit ]
  %curRightCrd.021 = phi <2 x i32> [ undef, %.lr.ph32 ], [ %curRightCrd.1.lcssa, %.loopexit ]
  %13 = insertelement <2 x i32> %curCrd.023, i32 %row.028, i32 1
  %14 = insertelement <2 x i32> %curLeftCrd.022, i32 %row.028, i32 1
  %15 = insertelement <2 x i32> %curRightCrd.021, i32 %row.028, i32 1
  %16 = add nsw i32 %row.028, -1
  %17 = insertelement <2 x i32> %upCrd.024, i32 %16, i32 1
  %18 = insertelement <2 x i32> %upLeftCrd.026, i32 %16, i32 1
  %19 = insertelement <2 x i32> %upRightCrd.027, i32 %16, i32 1
  %20 = add nsw i32 %row.028, 1
  %21 = insertelement <2 x i32> %lowCrd.029, i32 %20, i32 1
  %22 = insertelement <2 x i32> %lowLeftCrd.030, i32 %20, i32 1
  %23 = insertelement <2 x i32> %lowRightCrd.031, i32 %20, i32 1
  br i1 %10, label %.lr.ph, label %.loopexit

.lr.ph:                                           ; preds = %12, %.lr.ph
  %col.011 = phi i32 [ %27, %.lr.ph ], [ 0, %12 ]
  %lowRightCrd.110 = phi <2 x i32> [ %34, %.lr.ph ], [ %23, %12 ]
  %lowLeftCrd.19 = phi <2 x i32> [ %33, %.lr.ph ], [ %22, %12 ]
  %lowCrd.18 = phi <2 x i32> [ %32, %.lr.ph ], [ %21, %12 ]
  %upRightCrd.17 = phi <2 x i32> [ %31, %.lr.ph ], [ %19, %12 ]
  %upLeftCrd.16 = phi <2 x i32> [ %30, %.lr.ph ], [ %18, %12 ]
  %index.15 = phi i32 [ %53, %.lr.ph ], [ %index.025, %12 ]
  %upCrd.14 = phi <2 x i32> [ %29, %.lr.ph ], [ %17, %12 ]
  %curCrd.13 = phi <2 x i32> [ %24, %.lr.ph ], [ %13, %12 ]
  %curLeftCrd.12 = phi <2 x i32> [ %26, %.lr.ph ], [ %14, %12 ]
  %curRightCrd.11 = phi <2 x i32> [ %28, %.lr.ph ], [ %15, %12 ]
  %24 = insertelement <2 x i32> %curCrd.13, i32 %col.011, i32 0
  %25 = add nsw i32 %col.011, -1
  %26 = insertelement <2 x i32> %curLeftCrd.12, i32 %25, i32 0
  %27 = add nsw i32 %col.011, 1
  %28 = insertelement <2 x i32> %curRightCrd.11, i32 %27, i32 0
  %29 = insertelement <2 x i32> %upCrd.14, i32 %col.011, i32 0
  %30 = insertelement <2 x i32> %upLeftCrd.16, i32 %25, i32 0
  %31 = insertelement <2 x i32> %upRightCrd.17, i32 %27, i32 0
  %32 = insertelement <2 x i32> %lowCrd.18, i32 %col.011, i32 0
  %33 = insertelement <2 x i32> %lowLeftCrd.19, i32 %25, i32 0
  %34 = insertelement <2 x i32> %lowRightCrd.110, i32 %27, i32 0
  %35 = tail call <4 x float> @evaluatePixel(%struct._image2d_t.0* %inputImage, <2 x i32> %24)
  %36 = tail call <4 x float> @evaluatePixel(%struct._image2d_t.0* %inputImage, <2 x i32> %26)
  %37 = fadd <4 x float> %35, %36
  %38 = tail call <4 x float> @evaluatePixel(%struct._image2d_t.0* %inputImage, <2 x i32> %28)
  %39 = fadd <4 x float> %37, %38
  %40 = tail call <4 x float> @evaluatePixel(%struct._image2d_t.0* %inputImage, <2 x i32> %29)
  %41 = fadd <4 x float> %39, %40
  %42 = tail call <4 x float> @evaluatePixel(%struct._image2d_t.0* %inputImage, <2 x i32> %30)
  %43 = fadd <4 x float> %41, %42
  %44 = tail call <4 x float> @evaluatePixel(%struct._image2d_t.0* %inputImage, <2 x i32> %31)
  %45 = fadd <4 x float> %43, %44
  %46 = tail call <4 x float> @evaluatePixel(%struct._image2d_t.0* %inputImage, <2 x i32> %32)
  %47 = fadd <4 x float> %45, %46
  %48 = tail call <4 x float> @evaluatePixel(%struct._image2d_t.0* %inputImage, <2 x i32> %33)
  %49 = fadd <4 x float> %47, %48
  %50 = tail call <4 x float> @evaluatePixel(%struct._image2d_t.0* %inputImage, <2 x i32> %34)
  %51 = fadd <4 x float> %49, %50
  %52 = fdiv <4 x float> %51, <float 9.000000e+00, float 9.000000e+00, float 9.000000e+00, float 9.000000e+00>
  %53 = add nsw i32 %index.15, 1
  %54 = getelementptr inbounds <4 x float> addrspace(1)* %output, i32 %index.15
  store <4 x float> %52, <4 x float> addrspace(1)* %54, align 16
  %exitcond = icmp eq i32 %27, %7
  br i1 %exitcond, label %..loopexit_crit_edge, label %.lr.ph

._crit_edge:                                      ; preds = %.loopexit, %0
  ret void
}

declare <2 x i32> @_Z13get_image_dimP10_image2d_t(%struct._image2d_t.0*) nounwind readnone

declare i32 @_Z3minii(i32, i32) nounwind readnone

define void @wlSimpleBoxBlur_Optimized_CPU(<4 x float> addrspace(1)* %input, <4 x float> addrspace(1)* nocapture %output, i32 %width, i32 %height, i32 %buffer_size) nounwind {
  %1 = tail call i32 @get_global_id(i32 0) nounwind readnone
  %2 = tail call i32 @get_global_id(i32 1) nounwind readnone
  %3 = tail call i32 @get_global_size(i32 0) nounwind readnone
  %4 = tail call i32 @get_global_size(i32 1) nounwind readnone
  %5 = udiv i32 %height, %4
  %6 = udiv i32 %width, %3
  %7 = mul i32 %1, %width
  %8 = udiv i32 %7, %3
  %9 = mul i32 %2, %height
  %10 = udiv i32 %9, %4
  %11 = add i32 %5, 1
  %12 = add i32 %11, %10
  %13 = icmp ult i32 %12, %height
  br i1 %13, label %17, label %14

; <label>:14                                      ; preds = %0
  %15 = add i32 %height, -1
  %16 = sub i32 %15, %10
  br label %17

; <label>:17                                      ; preds = %0, %14
  %count_y.0 = phi i32 [ %16, %14 ], [ %5, %0 ]
  %bottomEdge.0 = phi i1 [ true, %14 ], [ false, %0 ]
  %18 = add i32 %6, 1
  %19 = add i32 %18, %8
  %20 = icmp ult i32 %19, %width
  br i1 %20, label %24, label %21

; <label>:21                                      ; preds = %17
  %22 = add i32 %width, -1
  %23 = sub i32 %22, %8
  br label %24

; <label>:24                                      ; preds = %17, %21
  %count_x.0 = phi i32 [ %23, %21 ], [ %6, %17 ]
  %rightEdge.0 = phi i1 [ true, %21 ], [ false, %17 ]
  %25 = icmp eq i32 %10, 0
  %index_y.0 = select i1 %25, i32 1, i32 %10
  %26 = icmp eq i32 %8, 0
  %index_x.0 = select i1 %26, i32 1, i32 %8
  %27 = add i32 %index_y.0, -1
  %28 = mul i32 %27, %width
  %29 = add i32 %index_x.0, -1
  %30 = add i32 %29, %28
  %31 = sext i1 %26 to i32
  %32 = sext i1 %25 to i32
  %33 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %30
  %34 = load <4 x float> addrspace(1)* %33, align 16
  %35 = fadd <4 x float> %34, zeroinitializer
  %36 = add i32 %index_x.0, %28
  %37 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %36
  %38 = load <4 x float> addrspace(1)* %37, align 16
  %39 = fadd <4 x float> %35, %38
  %40 = add i32 %30, 2
  %41 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %40
  %42 = load <4 x float> addrspace(1)* %41, align 16
  %43 = fadd <4 x float> %39, %42
  %44 = add i32 %30, %width
  %45 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %44
  %46 = load <4 x float> addrspace(1)* %45, align 16
  %47 = fadd <4 x float> %43, %46
  %48 = add i32 %44, 1
  %49 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %48
  %50 = load <4 x float> addrspace(1)* %49, align 16
  %51 = fadd <4 x float> %47, %50
  %52 = add i32 %44, 2
  %53 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %52
  %54 = load <4 x float> addrspace(1)* %53, align 16
  %55 = fadd <4 x float> %51, %54
  %56 = add i32 %44, %width
  %57 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %56
  %58 = load <4 x float> addrspace(1)* %57, align 16
  %59 = fadd <4 x float> %55, %58
  %60 = add i32 %56, 1
  %61 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %60
  %62 = load <4 x float> addrspace(1)* %61, align 16
  %63 = fadd <4 x float> %59, %62
  %64 = add i32 %56, 2
  %65 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %64
  %66 = load <4 x float> addrspace(1)* %65, align 16
  %67 = fadd <4 x float> %63, %66
  %.count_y.0 = add i32 %count_y.0, %32
  %.count_x.0 = add i32 %count_x.0, %31
  %68 = mul i32 %index_y.0, %width
  %69 = add i32 %68, %index_x.0
  %70 = fdiv <4 x float> %67, <float 9.000000e+00, float 9.000000e+00, float 9.000000e+00, float 9.000000e+00>
  %71 = getelementptr inbounds <4 x float> addrspace(1)* %output, i32 %69
  store <4 x float> %70, <4 x float> addrspace(1)* %71, align 16
  %sourceIndex.039 = add i32 %69, 1
  %72 = icmp eq i32 %.count_y.0, 1
  br i1 %72, label %..preheader23_crit_edge70, label %.preheader31.lr.ph

..preheader23_crit_edge70:                        ; preds = %24
  %.pre = icmp ugt i32 %.count_x.0, 1
  br i1 %.pre, label %.lr.ph29, label %._crit_edge30

.preheader31.lr.ph:                               ; preds = %24
  %73 = add i32 %index_y.0, 1
  %74 = mul i32 %73, %width
  %75 = add i32 %29, %74
  %76 = icmp ugt i32 %.count_x.0, 1
  %77 = icmp ugt i32 %8, 1
  %umax58 = select i1 %77, i32 %8, i32 1
  %78 = add i32 %count_x.0, %umax58
  %79 = icmp ugt i32 %10, 1
  %umax59 = select i1 %79, i32 %10, i32 1
  %80 = mul i32 %umax59, %width
  %81 = add i32 %78, %80
  %82 = add i32 %81, %31
  %83 = add i32 %count_y.0, %umax59
  %84 = add i32 %83, %32
  %85 = add i32 %84, -2
  %86 = mul i32 %85, %width
  %87 = add i32 %umax58, %86
  %88 = add i32 %84, -1
  %89 = mul i32 %88, %width
  %90 = add i32 %umax58, %89
  %91 = add i32 %.count_y.0, -1
  br label %.preheader31

.preheader31:                                     ; preds = %._crit_edge38, %.preheader31.lr.ph
  %indvars.iv = phi i32 [ %82, %.preheader31.lr.ph ], [ %indvars.iv.next, %._crit_edge38 ]
  %92 = phi i32 [ %40, %.preheader31.lr.ph ], [ %155, %._crit_edge38 ]
  %sourceIndex.044 = phi i32 [ %sourceIndex.039, %.preheader31.lr.ph ], [ %sourceIndex.0, %._crit_edge38 ]
  %row.043 = phi i32 [ 0, %.preheader31.lr.ph ], [ %127, %._crit_edge38 ]
  %bottomRowIndex.042 = phi i32 [ %75, %.preheader31.lr.ph ], [ %126, %._crit_edge38 ]
  %topRowIndex.041 = phi i32 [ %30, %.preheader31.lr.ph ], [ %154, %._crit_edge38 ]
  %firstBlockAccumulator.140 = phi <4 x float> [ %67, %.preheader31.lr.ph ], [ %148, %._crit_edge38 ]
  br i1 %76, label %.lr.ph37, label %._crit_edge38

.preheader23:                                     ; preds = %._crit_edge38
  %93 = add i32 %87, 1
  %94 = add i32 %90, 1
  %95 = add i32 %87, -1
  br i1 %76, label %.lr.ph29, label %._crit_edge30

.lr.ph29:                                         ; preds = %..preheader23_crit_edge70, %.preheader23
  %firstBlockAccumulator.1.lcssa74 = phi <4 x float> [ %67, %..preheader23_crit_edge70 ], [ %148, %.preheader23 ]
  %topRowIndex.0.lcssa73 = phi i32 [ %30, %..preheader23_crit_edge70 ], [ %95, %.preheader23 ]
  %sourceIndex.0.lcssa72 = phi i32 [ %sourceIndex.039, %..preheader23_crit_edge70 ], [ %94, %.preheader23 ]
  %.lcssa71 = phi i32 [ %40, %..preheader23_crit_edge70 ], [ %93, %.preheader23 ]
  %96 = add i32 %count_x.0, %sourceIndex.0.lcssa72
  %97 = add i32 %96, %31
  %98 = add i32 %97, -1
  br label %156

.lr.ph37:                                         ; preds = %.preheader31, %.lr.ph37
  %colorAccumulator.135 = phi <4 x float> [ %121, %.lr.ph37 ], [ %firstBlockAccumulator.140, %.preheader31 ]
  %sourceIndex.134 = phi i32 [ %124, %.lr.ph37 ], [ %sourceIndex.044, %.preheader31 ]
  %leftColumnIndex.033 = phi i32 [ %125, %.lr.ph37 ], [ %topRowIndex.041, %.preheader31 ]
  %rightColumnIndex.032 = phi i32 [ %99, %.lr.ph37 ], [ %92, %.preheader31 ]
  %99 = add i32 %rightColumnIndex.032, 1
  %100 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %leftColumnIndex.033
  %101 = load <4 x float> addrspace(1)* %100, align 16
  %102 = fsub <4 x float> %colorAccumulator.135, %101
  %103 = add i32 %leftColumnIndex.033, %width
  %104 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %99
  %105 = load <4 x float> addrspace(1)* %104, align 16
  %106 = fadd <4 x float> %102, %105
  %107 = add i32 %99, %width
  %108 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %103
  %109 = load <4 x float> addrspace(1)* %108, align 16
  %110 = fsub <4 x float> %106, %109
  %111 = add i32 %103, %width
  %112 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %107
  %113 = load <4 x float> addrspace(1)* %112, align 16
  %114 = fadd <4 x float> %110, %113
  %115 = add i32 %107, %width
  %116 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %111
  %117 = load <4 x float> addrspace(1)* %116, align 16
  %118 = fsub <4 x float> %114, %117
  %119 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %115
  %120 = load <4 x float> addrspace(1)* %119, align 16
  %121 = fadd <4 x float> %118, %120
  %122 = fdiv <4 x float> %121, <float 9.000000e+00, float 9.000000e+00, float 9.000000e+00, float 9.000000e+00>
  %123 = getelementptr inbounds <4 x float> addrspace(1)* %output, i32 %sourceIndex.134
  store <4 x float> %122, <4 x float> addrspace(1)* %123, align 16
  %124 = add i32 %sourceIndex.134, 1
  %125 = add i32 %leftColumnIndex.033, 1
  %exitcond60 = icmp eq i32 %124, %indvars.iv
  br i1 %exitcond60, label %._crit_edge38, label %.lr.ph37

._crit_edge38:                                    ; preds = %.lr.ph37, %.preheader31
  %126 = add i32 %bottomRowIndex.042, %width
  %127 = add i32 %row.043, 1
  %128 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %topRowIndex.041
  %129 = load <4 x float> addrspace(1)* %128, align 16
  %130 = fsub <4 x float> %firstBlockAccumulator.140, %129
  %131 = add i32 %topRowIndex.041, 1
  %132 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %126
  %133 = load <4 x float> addrspace(1)* %132, align 16
  %134 = fadd <4 x float> %130, %133
  %135 = add i32 %126, 1
  %136 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %131
  %137 = load <4 x float> addrspace(1)* %136, align 16
  %138 = fsub <4 x float> %134, %137
  %139 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %135
  %140 = load <4 x float> addrspace(1)* %139, align 16
  %141 = fadd <4 x float> %138, %140
  %142 = add i32 %126, 2
  %143 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %92
  %144 = load <4 x float> addrspace(1)* %143, align 16
  %145 = fsub <4 x float> %141, %144
  %146 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %142
  %147 = load <4 x float> addrspace(1)* %146, align 16
  %148 = fadd <4 x float> %145, %147
  %149 = add i32 %127, %index_y.0
  %150 = mul i32 %149, %width
  %151 = add i32 %150, %index_x.0
  %152 = fdiv <4 x float> %148, <float 9.000000e+00, float 9.000000e+00, float 9.000000e+00, float 9.000000e+00>
  %153 = getelementptr inbounds <4 x float> addrspace(1)* %output, i32 %151
  store <4 x float> %152, <4 x float> addrspace(1)* %153, align 16
  %154 = add i32 %topRowIndex.041, %width
  %sourceIndex.0 = add i32 %151, 1
  %155 = add i32 %154, 2
  %indvars.iv.next = add i32 %indvars.iv, %width
  %exitcond67 = icmp eq i32 %127, %91
  br i1 %exitcond67, label %.preheader23, label %.preheader31

; <label>:156                                     ; preds = %156, %.lr.ph29
  %colorAccumulator.227 = phi <4 x float> [ %firstBlockAccumulator.1.lcssa74, %.lr.ph29 ], [ %179, %156 ]
  %sourceIndex.226 = phi i32 [ %sourceIndex.0.lcssa72, %.lr.ph29 ], [ %182, %156 ]
  %leftColumnIndex.125 = phi i32 [ %topRowIndex.0.lcssa73, %.lr.ph29 ], [ %183, %156 ]
  %rightColumnIndex.124 = phi i32 [ %.lcssa71, %.lr.ph29 ], [ %157, %156 ]
  %157 = add i32 %rightColumnIndex.124, 1
  %158 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %leftColumnIndex.125
  %159 = load <4 x float> addrspace(1)* %158, align 16
  %160 = fsub <4 x float> %colorAccumulator.227, %159
  %161 = add i32 %leftColumnIndex.125, %width
  %162 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %157
  %163 = load <4 x float> addrspace(1)* %162, align 16
  %164 = fadd <4 x float> %160, %163
  %165 = add i32 %157, %width
  %166 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %161
  %167 = load <4 x float> addrspace(1)* %166, align 16
  %168 = fsub <4 x float> %164, %167
  %169 = add i32 %161, %width
  %170 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %165
  %171 = load <4 x float> addrspace(1)* %170, align 16
  %172 = fadd <4 x float> %168, %171
  %173 = add i32 %165, %width
  %174 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %169
  %175 = load <4 x float> addrspace(1)* %174, align 16
  %176 = fsub <4 x float> %172, %175
  %177 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %173
  %178 = load <4 x float> addrspace(1)* %177, align 16
  %179 = fadd <4 x float> %176, %178
  %180 = fdiv <4 x float> %179, <float 9.000000e+00, float 9.000000e+00, float 9.000000e+00, float 9.000000e+00>
  %181 = getelementptr inbounds <4 x float> addrspace(1)* %output, i32 %sourceIndex.226
  store <4 x float> %180, <4 x float> addrspace(1)* %181, align 16
  %182 = add i32 %sourceIndex.226, 1
  %183 = add i32 %leftColumnIndex.125, 1
  %exitcond57 = icmp eq i32 %182, %98
  br i1 %exitcond57, label %._crit_edge30, label %156

._crit_edge30:                                    ; preds = %156, %..preheader23_crit_edge70, %.preheader23
  %topEdge.0.not = xor i1 %25, true
  %leftEdge.0.not = xor i1 %26, true
  %brmerge = or i1 %topEdge.0.not, %leftEdge.0.not
  br i1 %brmerge, label %198, label %184

; <label>:184                                     ; preds = %._crit_edge30
  %185 = load <4 x float> addrspace(1)* %input, align 16
  %186 = fadd <4 x float> %185, zeroinitializer
  %187 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 1
  %188 = load <4 x float> addrspace(1)* %187, align 16
  %189 = fadd <4 x float> %186, %188
  %190 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %width
  %191 = load <4 x float> addrspace(1)* %190, align 16
  %192 = fadd <4 x float> %189, %191
  %193 = add i32 %width, 1
  %194 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %193
  %195 = load <4 x float> addrspace(1)* %194, align 16
  %196 = fadd <4 x float> %192, %195
  %197 = fdiv <4 x float> %196, <float 9.000000e+00, float 9.000000e+00, float 9.000000e+00, float 9.000000e+00>
  store <4 x float> %197, <4 x float> addrspace(1)* %output, align 16
  br label %198

; <label>:198                                     ; preds = %._crit_edge30, %184
  br i1 %25, label %.preheader19, label %.critedge

.preheader19:                                     ; preds = %198
  %199 = add i32 %.count_x.0, %index_x.0
  %200 = icmp ult i32 %index_x.0, %199
  br i1 %200, label %.lr.ph21, label %._crit_edge22

.lr.ph21:                                         ; preds = %.preheader19
  %201 = icmp ugt i32 %8, 1
  %umax55 = select i1 %201, i32 %8, i32 1
  %202 = add i32 %count_x.0, %umax55
  %203 = add i32 %202, %31
  br label %204

; <label>:204                                     ; preds = %204, %.lr.ph21
  %column2.020 = phi i32 [ %index_x.0, %.lr.ph21 ], [ %212, %204 ]
  %205 = add nsw i32 %column2.020, -1
  %206 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %205
  %207 = load <4 x float> addrspace(1)* %206, align 16
  %208 = fadd <4 x float> %207, zeroinitializer
  %209 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %column2.020
  %210 = load <4 x float> addrspace(1)* %209, align 16
  %211 = fadd <4 x float> %208, %210
  %212 = add nsw i32 %column2.020, 1
  %213 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %212
  %214 = load <4 x float> addrspace(1)* %213, align 16
  %215 = fadd <4 x float> %211, %214
  %216 = add i32 %column2.020, %width
  %217 = add i32 %216, -1
  %218 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %217
  %219 = load <4 x float> addrspace(1)* %218, align 16
  %220 = fadd <4 x float> %215, %219
  %221 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %216
  %222 = load <4 x float> addrspace(1)* %221, align 16
  %223 = fadd <4 x float> %220, %222
  %224 = add i32 %216, 1
  %225 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %224
  %226 = load <4 x float> addrspace(1)* %225, align 16
  %227 = fadd <4 x float> %223, %226
  %228 = fdiv <4 x float> %227, <float 9.000000e+00, float 9.000000e+00, float 9.000000e+00, float 9.000000e+00>
  %229 = getelementptr inbounds <4 x float> addrspace(1)* %output, i32 %column2.020
  store <4 x float> %228, <4 x float> addrspace(1)* %229, align 16
  %exitcond56 = icmp eq i32 %212, %203
  br i1 %exitcond56, label %._crit_edge22, label %204

._crit_edge22:                                    ; preds = %204, %.preheader19
  %rightEdge.0.not = xor i1 %rightEdge.0, true
  %brmerge2 = or i1 %topEdge.0.not, %rightEdge.0.not
  br i1 %brmerge2, label %.critedge, label %230

; <label>:230                                     ; preds = %._crit_edge22
  %231 = add i32 %width, -2
  %232 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %231
  %233 = load <4 x float> addrspace(1)* %232, align 16
  %234 = fadd <4 x float> %233, zeroinitializer
  %235 = add i32 %width, -1
  %236 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %235
  %237 = load <4 x float> addrspace(1)* %236, align 16
  %238 = fadd <4 x float> %234, %237
  %239 = shl i32 %width, 1
  %240 = add i32 %239, -2
  %241 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %240
  %242 = load <4 x float> addrspace(1)* %241, align 16
  %243 = fadd <4 x float> %238, %242
  %244 = add i32 %239, -1
  %245 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %244
  %246 = load <4 x float> addrspace(1)* %245, align 16
  %247 = fadd <4 x float> %243, %246
  %248 = fdiv <4 x float> %247, <float 9.000000e+00, float 9.000000e+00, float 9.000000e+00, float 9.000000e+00>
  %249 = getelementptr inbounds <4 x float> addrspace(1)* %output, i32 %235
  store <4 x float> %248, <4 x float> addrspace(1)* %249, align 16
  br label %.critedge

.critedge:                                        ; preds = %198, %._crit_edge22, %230
  br i1 %26, label %.preheader15, label %.loopexit16

.preheader15:                                     ; preds = %.critedge
  %250 = add i32 %.count_y.0, %index_y.0
  %251 = icmp ult i32 %index_y.0, %250
  br i1 %251, label %.lr.ph18, label %.loopexit16

.lr.ph18:                                         ; preds = %.preheader15
  %252 = icmp ugt i32 %10, 1
  %umax53 = select i1 %252, i32 %10, i32 1
  %253 = add i32 %count_y.0, %umax53
  %254 = add i32 %253, %32
  br label %255

; <label>:255                                     ; preds = %255, %.lr.ph18
  %row3.017 = phi i32 [ %index_y.0, %.lr.ph18 ], [ %265, %255 ]
  %256 = add nsw i32 %row3.017, -1
  %257 = mul i32 %256, %width
  %258 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %257
  %259 = load <4 x float> addrspace(1)* %258, align 16
  %260 = fadd <4 x float> %259, zeroinitializer
  %261 = mul i32 %row3.017, %width
  %262 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %261
  %263 = load <4 x float> addrspace(1)* %262, align 16
  %264 = fadd <4 x float> %260, %263
  %265 = add nsw i32 %row3.017, 1
  %266 = mul i32 %265, %width
  %267 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %266
  %268 = load <4 x float> addrspace(1)* %267, align 16
  %269 = fadd <4 x float> %264, %268
  %270 = add i32 %257, 1
  %271 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %270
  %272 = load <4 x float> addrspace(1)* %271, align 16
  %273 = fadd <4 x float> %269, %272
  %274 = add i32 %261, 1
  %275 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %274
  %276 = load <4 x float> addrspace(1)* %275, align 16
  %277 = fadd <4 x float> %273, %276
  %278 = add i32 %266, 1
  %279 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %278
  %280 = load <4 x float> addrspace(1)* %279, align 16
  %281 = fadd <4 x float> %277, %280
  %282 = fdiv <4 x float> %281, <float 9.000000e+00, float 9.000000e+00, float 9.000000e+00, float 9.000000e+00>
  %283 = getelementptr inbounds <4 x float> addrspace(1)* %output, i32 %261
  store <4 x float> %282, <4 x float> addrspace(1)* %283, align 16
  %exitcond54 = icmp eq i32 %265, %254
  br i1 %exitcond54, label %.loopexit16, label %255

.loopexit16:                                      ; preds = %.preheader15, %255, %.critedge
  %bottomEdge.0.not = xor i1 %bottomEdge.0, true
  %brmerge4 = or i1 %bottomEdge.0.not, %leftEdge.0.not
  br i1 %brmerge4, label %305, label %284

; <label>:284                                     ; preds = %.loopexit16
  %285 = add i32 %height, -2
  %286 = mul i32 %285, %width
  %287 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %286
  %288 = load <4 x float> addrspace(1)* %287, align 16
  %289 = fadd <4 x float> %288, zeroinitializer
  %290 = add i32 %286, 1
  %291 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %290
  %292 = load <4 x float> addrspace(1)* %291, align 16
  %293 = fadd <4 x float> %289, %292
  %294 = add i32 %height, -1
  %295 = mul i32 %294, %width
  %296 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %295
  %297 = load <4 x float> addrspace(1)* %296, align 16
  %298 = fadd <4 x float> %293, %297
  %299 = add i32 %295, 1
  %300 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %299
  %301 = load <4 x float> addrspace(1)* %300, align 16
  %302 = fadd <4 x float> %298, %301
  %303 = fdiv <4 x float> %302, <float 9.000000e+00, float 9.000000e+00, float 9.000000e+00, float 9.000000e+00>
  %304 = getelementptr inbounds <4 x float> addrspace(1)* %output, i32 %295
  store <4 x float> %303, <4 x float> addrspace(1)* %304, align 16
  br label %305

; <label>:305                                     ; preds = %.loopexit16, %284
  br i1 %bottomEdge.0, label %.preheader12, label %.loopexit

.preheader12:                                     ; preds = %305
  %306 = add i32 %.count_x.0, %index_x.0
  %307 = icmp ult i32 %index_x.0, %306
  br i1 %307, label %.lr.ph14, label %.loopexit

.lr.ph14:                                         ; preds = %.preheader12
  %308 = add i32 %height, -2
  %309 = mul i32 %308, %width
  %310 = add i32 %height, -1
  %311 = mul i32 %310, %width
  %312 = icmp ugt i32 %8, 1
  %umax51 = select i1 %312, i32 %8, i32 1
  %313 = add i32 %count_x.0, %umax51
  %314 = add i32 %313, %31
  br label %315

; <label>:315                                     ; preds = %315, %.lr.ph14
  %column4.013 = phi i32 [ %index_x.0, %.lr.ph14 ], [ %342, %315 ]
  %316 = add i32 %column4.013, %309
  %317 = add i32 %316, -1
  %318 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %317
  %319 = load <4 x float> addrspace(1)* %318, align 16
  %320 = fadd <4 x float> %319, zeroinitializer
  %321 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %316
  %322 = load <4 x float> addrspace(1)* %321, align 16
  %323 = fadd <4 x float> %320, %322
  %324 = add i32 %316, 1
  %325 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %324
  %326 = load <4 x float> addrspace(1)* %325, align 16
  %327 = fadd <4 x float> %323, %326
  %328 = add i32 %column4.013, %311
  %329 = add i32 %328, -1
  %330 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %329
  %331 = load <4 x float> addrspace(1)* %330, align 16
  %332 = fadd <4 x float> %327, %331
  %333 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %328
  %334 = load <4 x float> addrspace(1)* %333, align 16
  %335 = fadd <4 x float> %332, %334
  %336 = add i32 %328, 1
  %337 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %336
  %338 = load <4 x float> addrspace(1)* %337, align 16
  %339 = fadd <4 x float> %335, %338
  %340 = fdiv <4 x float> %339, <float 9.000000e+00, float 9.000000e+00, float 9.000000e+00, float 9.000000e+00>
  %341 = getelementptr inbounds <4 x float> addrspace(1)* %output, i32 %328
  store <4 x float> %340, <4 x float> addrspace(1)* %341, align 16
  %342 = add nsw i32 %column4.013, 1
  %exitcond52 = icmp eq i32 %342, %314
  br i1 %exitcond52, label %.loopexit, label %315

.loopexit:                                        ; preds = %.preheader12, %315, %305
  br i1 %rightEdge.0, label %.preheader, label %.thread

.preheader:                                       ; preds = %.loopexit
  %343 = add i32 %.count_y.0, %index_y.0
  %344 = icmp ult i32 %index_y.0, %343
  br i1 %344, label %.lr.ph, label %._crit_edge

.lr.ph:                                           ; preds = %.preheader
  %345 = icmp ugt i32 %10, 1
  %umax = select i1 %345, i32 %10, i32 1
  %346 = add i32 %count_y.0, %umax
  %347 = add i32 %346, %32
  br label %348

; <label>:348                                     ; preds = %348, %.lr.ph
  %row5.011 = phi i32 [ %index_y.0, %.lr.ph ], [ %354, %348 ]
  %349 = mul i32 %row5.011, %width
  %350 = add i32 %349, -1
  %351 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %350
  %352 = load <4 x float> addrspace(1)* %351, align 16
  %353 = fadd <4 x float> %352, zeroinitializer
  %354 = add nsw i32 %row5.011, 1
  %355 = mul i32 %354, %width
  %356 = add i32 %355, -1
  %357 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %356
  %358 = load <4 x float> addrspace(1)* %357, align 16
  %359 = fadd <4 x float> %353, %358
  %360 = add nsw i32 %row5.011, 2
  %361 = mul i32 %360, %width
  %362 = add i32 %361, -1
  %363 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %362
  %364 = load <4 x float> addrspace(1)* %363, align 16
  %365 = fadd <4 x float> %359, %364
  %366 = add i32 %349, -2
  %367 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %366
  %368 = load <4 x float> addrspace(1)* %367, align 16
  %369 = fadd <4 x float> %365, %368
  %370 = add i32 %355, -2
  %371 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %370
  %372 = load <4 x float> addrspace(1)* %371, align 16
  %373 = fadd <4 x float> %369, %372
  %374 = add i32 %361, -2
  %375 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %374
  %376 = load <4 x float> addrspace(1)* %375, align 16
  %377 = fadd <4 x float> %373, %376
  %378 = fdiv <4 x float> %377, <float 9.000000e+00, float 9.000000e+00, float 9.000000e+00, float 9.000000e+00>
  %379 = getelementptr inbounds <4 x float> addrspace(1)* %output, i32 %356
  store <4 x float> %378, <4 x float> addrspace(1)* %379, align 16
  %exitcond = icmp eq i32 %354, %347
  br i1 %exitcond, label %._crit_edge, label %348

._crit_edge:                                      ; preds = %348, %.preheader
  br i1 %bottomEdge.0, label %380, label %.thread

; <label>:380                                     ; preds = %._crit_edge
  %381 = add i32 %height, -1
  %382 = mul i32 %381, %width
  %383 = add i32 %382, -2
  %384 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %383
  %385 = load <4 x float> addrspace(1)* %384, align 16
  %386 = fadd <4 x float> %385, zeroinitializer
  %387 = add i32 %382, -1
  %388 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %387
  %389 = load <4 x float> addrspace(1)* %388, align 16
  %390 = fadd <4 x float> %386, %389
  %391 = mul i32 %height, %width
  %392 = add i32 %391, -2
  %393 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %392
  %394 = load <4 x float> addrspace(1)* %393, align 16
  %395 = fadd <4 x float> %390, %394
  %396 = add i32 %391, -1
  %397 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %396
  %398 = load <4 x float> addrspace(1)* %397, align 16
  %399 = fadd <4 x float> %395, %398
  %400 = fdiv <4 x float> %399, <float 9.000000e+00, float 9.000000e+00, float 9.000000e+00, float 9.000000e+00>
  %401 = getelementptr inbounds <4 x float> addrspace(1)* %output, i32 %396
  store <4 x float> %400, <4 x float> addrspace(1)* %401, align 16
  br label %.thread

.thread:                                          ; preds = %._crit_edge, %.loopexit, %380
  ret void
}

!opencl.kernels = !{!0, !1, !2, !3}
!opencl.cl_kernel_arg_info = !{!4, !10, !11, !17}
!opencl.build.options = !{!18}

!0 = metadata !{void (<4 x float> addrspace(1)*, <4 x float> addrspace(1)*, i32, i32, i32)* @wlSimpleBoxBlur_GPU}
!1 = metadata !{void (<4 x float> addrspace(1)*, <4 x float> addrspace(1)*, i32, i32, i32)* @wlSimpleBoxBlur_CPU}
!2 = metadata !{void (%struct._image2d_t.0*, <4 x float> addrspace(1)*, i32)* @wlSimpleBoxBlur_image2d}
!3 = metadata !{void (<4 x float> addrspace(1)*, <4 x float> addrspace(1)*, i32, i32, i32)* @wlSimpleBoxBlur_Optimized_CPU}
!4 = metadata !{metadata !"cl_kernel_arg_info", void (<4 x float> addrspace(1)*, <4 x float> addrspace(1)*, i32, i32, i32)* @wlSimpleBoxBlur_GPU, metadata !5, metadata !6, metadata !7, metadata !8, metadata !9}
!5 = metadata !{i32 1, i32 1, i32 0, i32 0, i32 0}
!6 = metadata !{i32 3, i32 3, i32 3, i32 3, i32 3}
!7 = metadata !{metadata !"float4*", metadata !"float4*", metadata !"uint", metadata !"uint", metadata !"uint"}
!8 = metadata !{i32 0, i32 0, i32 1, i32 1, i32 1}
!9 = metadata !{metadata !"input", metadata !"output", metadata !"width", metadata !"height", metadata !"buffer_size"}
!10 = metadata !{metadata !"cl_kernel_arg_info", void (<4 x float> addrspace(1)*, <4 x float> addrspace(1)*, i32, i32, i32)* @wlSimpleBoxBlur_CPU, metadata !5, metadata !6, metadata !7, metadata !8, metadata !9}
!11 = metadata !{metadata !"cl_kernel_arg_info", void (%struct._image2d_t.0*, <4 x float> addrspace(1)*, i32)* @wlSimpleBoxBlur_image2d, metadata !12, metadata !13, metadata !14, metadata !15, metadata !16}
!12 = metadata !{i32 0, i32 1, i32 0}
!13 = metadata !{i32 0, i32 3, i32 3}
!14 = metadata !{metadata !"image2d_t", metadata !"float4*", metadata !"uint"}
!15 = metadata !{i32 0, i32 0, i32 1}
!16 = metadata !{metadata !"inputImage", metadata !"output", metadata !"rowCountPerGlobalID"}
!17 = metadata !{metadata !"cl_kernel_arg_info", void (<4 x float> addrspace(1)*, <4 x float> addrspace(1)*, i32, i32, i32)* @wlSimpleBoxBlur_Optimized_CPU, metadata !5, metadata !6, metadata !7, metadata !8, metadata !9}
!18 = metadata !{metadata !"-cl-kernel-arg-info"}
