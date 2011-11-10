; ModuleID = 'Program'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%struct._image2d_t = type opaque

define void @wlSimpleBoxBlur_GPU(<4 x float> addrspace(1)* nocapture %input, <4 x float> addrspace(1)* nocapture %output, i32 %width, i32 %height, i32 %buffer_size) nounwind {
; <label>:0
  %1 = tail call i32 (...)* @get_work_dim() nounwind
  %2 = tail call i32 @get_global_id(i32 0) nounwind
  %3 = tail call i32 @get_global_id(i32 1) nounwind
  %4 = tail call i32 @get_local_id(i32 0) nounwind
  %5 = tail call i32 @get_local_id(i32 1) nounwind
  %6 = tail call i32 @get_global_size(i32 0) nounwind
  %7 = tail call i32 @get_global_size(i32 1) nounwind
  %8 = tail call i32 @get_local_size(i32 0) nounwind
  %9 = tail call i32 @get_local_size(i32 1) nounwind
  %10 = tail call i32 @get_group_id(i32 0) nounwind
  %11 = tail call i32 @get_group_id(i32 1) nounwind
  %12 = mul i32 %3, %width
  %13 = add i32 %12, %2
  %14 = add i32 %3, -1
  %15 = mul i32 %14, %width
  %16 = add i32 %15, %2
  %17 = add i32 %3, 1
  %18 = mul i32 %17, %width
  %19 = add i32 %18, %2
  %20 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %13
  %21 = load <4 x float> addrspace(1)* %20, align 16
  %22 = icmp eq i32 %2, 0
  br i1 %22, label %28, label %23

; <label>:23                                      ; preds = %0
  %24 = add i32 %13, -1
  %25 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %24
  %26 = load <4 x float> addrspace(1)* %25, align 16
  %27 = fadd <4 x float> %21, %26
  br label %28

; <label>:28                                      ; preds = %0, %23
  %colorAccumulator.0 = phi <4 x float> [ %27, %23 ], [ %21, %0 ]
  %29 = add i32 %6, -1
  %30 = icmp ult i32 %2, %29
  br i1 %30, label %31, label %36

; <label>:31                                      ; preds = %28
  %32 = add i32 %13, 1
  %33 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %32
  %34 = load <4 x float> addrspace(1)* %33, align 16
  %35 = fadd <4 x float> %colorAccumulator.0, %34
  br label %36

; <label>:36                                      ; preds = %31, %28
  %colorAccumulator.1 = phi <4 x float> [ %35, %31 ], [ %colorAccumulator.0, %28 ]
  %37 = icmp eq i32 %3, 0
  br i1 %37, label %53, label %38

; <label>:38                                      ; preds = %36
  %39 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %16
  %40 = load <4 x float> addrspace(1)* %39, align 16
  %41 = fadd <4 x float> %colorAccumulator.1, %40
  br i1 %22, label %47, label %42

; <label>:42                                      ; preds = %38
  %43 = add i32 %16, -1
  %44 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %43
  %45 = load <4 x float> addrspace(1)* %44, align 16
  %46 = fadd <4 x float> %41, %45
  br label %47

; <label>:47                                      ; preds = %38, %42
  %colorAccumulator.3 = phi <4 x float> [ %46, %42 ], [ %41, %38 ]
  br i1 %30, label %48, label %53

; <label>:48                                      ; preds = %47
  %49 = add i32 %16, 1
  %50 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %49
  %51 = load <4 x float> addrspace(1)* %50, align 16
  %52 = fadd <4 x float> %colorAccumulator.3, %51
  br label %53

; <label>:53                                      ; preds = %36, %47, %48
  %colorAccumulator.2 = phi <4 x float> [ %52, %48 ], [ %colorAccumulator.3, %47 ], [ %colorAccumulator.1, %36 ]
  %54 = add i32 %7, -1
  %55 = icmp ult i32 %3, %54
  br i1 %55, label %56, label %71

; <label>:56                                      ; preds = %53
  %57 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %19
  %58 = load <4 x float> addrspace(1)* %57, align 16
  %59 = fadd <4 x float> %colorAccumulator.2, %58
  br i1 %22, label %65, label %60

; <label>:60                                      ; preds = %56
  %61 = add i32 %19, -1
  %62 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %61
  %63 = load <4 x float> addrspace(1)* %62, align 16
  %64 = fadd <4 x float> %59, %63
  br label %65

; <label>:65                                      ; preds = %56, %60
  %colorAccumulator.5 = phi <4 x float> [ %64, %60 ], [ %59, %56 ]
  br i1 %30, label %66, label %71

; <label>:66                                      ; preds = %65
  %67 = add i32 %19, 1
  %68 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %67
  %69 = load <4 x float> addrspace(1)* %68, align 16
  %70 = fadd <4 x float> %colorAccumulator.5, %69
  br label %71

; <label>:71                                      ; preds = %65, %66, %53
  %colorAccumulator.4 = phi <4 x float> [ %70, %66 ], [ %colorAccumulator.5, %65 ], [ %colorAccumulator.2, %53 ]
  %72 = fdiv <4 x float> %colorAccumulator.4, <float 9.000000e+000, float 9.000000e+000, float 9.000000e+000, float 9.000000e+000>
  %73 = getelementptr inbounds <4 x float> addrspace(1)* %output, i32 %13
  store <4 x float> %72, <4 x float> addrspace(1)* %73, align 16
  ret void
}

declare i32 @get_work_dim(...)

declare i32 @get_global_id(i32)

declare i32 @get_local_id(i32)

declare i32 @get_global_size(i32)

declare i32 @get_local_size(i32)

declare i32 @get_group_id(i32)

define void @wlSimpleBoxBlur_CPU(<4 x float> addrspace(1)* nocapture %input, <4 x float> addrspace(1)* nocapture %output, i32 %width, i32 %height, i32 %buffer_size) nounwind {
  %1 = tail call i32 (...)* @get_work_dim() nounwind
  %2 = tail call i32 @get_global_id(i32 0) nounwind
  %3 = tail call i32 @get_global_id(i32 1) nounwind
  %4 = tail call i32 @get_local_id(i32 0) nounwind
  %5 = tail call i32 @get_local_id(i32 1) nounwind
  %6 = tail call i32 @get_global_size(i32 0) nounwind
  %7 = tail call i32 @get_global_size(i32 1) nounwind
  %8 = tail call i32 @get_local_size(i32 0) nounwind
  %9 = tail call i32 @get_local_size(i32 1) nounwind
  %10 = tail call i32 @get_group_id(i32 0) nounwind
  %11 = tail call i32 @get_group_id(i32 1) nounwind
  %12 = icmp eq i32 %7, 0
  %13 = select i1 %12, i32 1, i32 %7
  %14 = udiv i32 %height, %13
  %15 = icmp eq i32 %14, 0
  br i1 %15, label %._crit_edge6, label %bb.nph5

bb.nph5:                                          ; preds = %0
  %16 = icmp eq i32 %6, 0
  %17 = select i1 %16, i32 1, i32 %6
  %18 = udiv i32 %width, %17
  %19 = icmp eq i32 %18, 0
  %20 = add i32 %width, -1
  %21 = add i32 %height, -1
  %tmp = icmp ugt i32 %6, 1
  %umax = select i1 %tmp, i32 %6, i32 1
  %tmp7 = udiv i32 %width, %umax
  %tmp8 = mul i32 %2, %width
  %tmp9 = udiv i32 %tmp8, %umax
  %tmp57 = icmp ugt i32 %7, 1
  %umax58 = select i1 %tmp57, i32 %7, i32 1
  %tmp59 = udiv i32 %height, %umax58
  %tmp66 = mul i32 %3, %height
  %tmp67 = udiv i32 %tmp66, %umax58
  %tmp68 = add i32 %tmp67, 1
  %tmp69 = mul i32 %tmp68, %width
  %tmp70 = add i32 %tmp9, %tmp69
  %tmp74 = add i32 %tmp70, -1
  %tmp78 = add i32 %tmp70, 1
  %tmp82 = add i32 %tmp67, -1
  %tmp83 = mul i32 %tmp82, %width
  %tmp84 = add i32 %tmp9, %tmp83
  %tmp88 = add i32 %tmp84, -1
  %tmp92 = add i32 %tmp84, 1
  %tmp96 = mul i32 %tmp67, %width
  %tmp97 = add i32 %tmp9, %tmp96
  %tmp102 = add i32 %tmp97, -1
  %tmp106 = add i32 %tmp97, 1
  br label %.preheader

.preheader:                                       ; preds = %._crit_edge, %bb.nph5
  %i.04 = phi i32 [ 0, %bb.nph5 ], [ %60, %._crit_edge ]
  %tmp61 = mul i32 %i.04, %width
  %tmp71 = add i32 %tmp70, %tmp61
  %tmp75 = add i32 %tmp74, %tmp61
  %tmp79 = add i32 %tmp78, %tmp61
  %tmp85 = add i32 %tmp84, %tmp61
  %tmp89 = add i32 %tmp88, %tmp61
  %tmp93 = add i32 %tmp92, %tmp61
  %tmp98 = add i32 %tmp97, %tmp61
  %tmp103 = add i32 %tmp102, %tmp61
  %tmp107 = add i32 %tmp106, %tmp61
  %index_y.03 = add i32 %tmp67, %i.04
  br i1 %19, label %._crit_edge, label %bb.nph

bb.nph:                                           ; preds = %.preheader
  %22 = icmp eq i32 %index_y.03, 0
  %23 = icmp ult i32 %index_y.03, %21
  br label %24

; <label>:24                                      ; preds = %57, %bb.nph
  %j.01 = phi i32 [ 0, %bb.nph ], [ %59, %57 ]
  %tmp72 = add i32 %tmp71, %j.01
  %scevgep56 = getelementptr <4 x float> addrspace(1)* %input, i32 %tmp72
  %tmp76 = add i32 %tmp75, %j.01
  %scevgep53 = getelementptr <4 x float> addrspace(1)* %input, i32 %tmp76
  %tmp80 = add i32 %tmp79, %j.01
  %scevgep49 = getelementptr <4 x float> addrspace(1)* %input, i32 %tmp80
  %tmp86 = add i32 %tmp85, %j.01
  %scevgep42 = getelementptr <4 x float> addrspace(1)* %input, i32 %tmp86
  %tmp90 = add i32 %tmp89, %j.01
  %scevgep39 = getelementptr <4 x float> addrspace(1)* %input, i32 %tmp90
  %tmp94 = add i32 %tmp93, %j.01
  %scevgep35 = getelementptr <4 x float> addrspace(1)* %input, i32 %tmp94
  %tmp99 = add i32 %tmp98, %j.01
  %scevgep28 = getelementptr <4 x float> addrspace(1)* %output, i32 %tmp99
  %scevgep27 = getelementptr <4 x float> addrspace(1)* %input, i32 %tmp99
  %tmp108 = add i32 %tmp107, %j.01
  %scevgep = getelementptr <4 x float> addrspace(1)* %input, i32 %tmp108
  %index_x1.02 = add i32 %tmp9, %j.01
  %25 = load <4 x float> addrspace(1)* %scevgep27, align 16
  %26 = icmp eq i32 %index_x1.02, 0
  br i1 %26, label %30, label %27

; <label>:27                                      ; preds = %24
  %tmp104 = add i32 %tmp103, %j.01
  %scevgep24 = getelementptr <4 x float> addrspace(1)* %input, i32 %tmp104
  %28 = load <4 x float> addrspace(1)* %scevgep24, align 16
  %29 = fadd <4 x float> %25, %28
  br label %30

; <label>:30                                      ; preds = %24, %27
  %colorAccumulator.0 = phi <4 x float> [ %29, %27 ], [ %25, %24 ]
  %31 = icmp ult i32 %index_x1.02, %20
  br i1 %31, label %32, label %35

; <label>:32                                      ; preds = %30
  %33 = load <4 x float> addrspace(1)* %scevgep, align 16
  %34 = fadd <4 x float> %colorAccumulator.0, %33
  br label %35

; <label>:35                                      ; preds = %32, %30
  %colorAccumulator.1 = phi <4 x float> [ %34, %32 ], [ %colorAccumulator.0, %30 ]
  br i1 %22, label %46, label %36

; <label>:36                                      ; preds = %35
  %37 = load <4 x float> addrspace(1)* %scevgep42, align 16
  %38 = fadd <4 x float> %colorAccumulator.1, %37
  br i1 %26, label %42, label %39

; <label>:39                                      ; preds = %36
  %40 = load <4 x float> addrspace(1)* %scevgep39, align 16
  %41 = fadd <4 x float> %38, %40
  br label %42

; <label>:42                                      ; preds = %36, %39
  %colorAccumulator.3 = phi <4 x float> [ %41, %39 ], [ %38, %36 ]
  br i1 %31, label %43, label %46

; <label>:43                                      ; preds = %42
  %44 = load <4 x float> addrspace(1)* %scevgep35, align 16
  %45 = fadd <4 x float> %colorAccumulator.3, %44
  br label %46

; <label>:46                                      ; preds = %35, %42, %43
  %colorAccumulator.2 = phi <4 x float> [ %45, %43 ], [ %colorAccumulator.3, %42 ], [ %colorAccumulator.1, %35 ]
  br i1 %23, label %47, label %57

; <label>:47                                      ; preds = %46
  %48 = load <4 x float> addrspace(1)* %scevgep56, align 16
  %49 = fadd <4 x float> %colorAccumulator.2, %48
  br i1 %26, label %53, label %50

; <label>:50                                      ; preds = %47
  %51 = load <4 x float> addrspace(1)* %scevgep53, align 16
  %52 = fadd <4 x float> %49, %51
  br label %53

; <label>:53                                      ; preds = %47, %50
  %colorAccumulator.5 = phi <4 x float> [ %52, %50 ], [ %49, %47 ]
  br i1 %31, label %54, label %57

; <label>:54                                      ; preds = %53
  %55 = load <4 x float> addrspace(1)* %scevgep49, align 16
  %56 = fadd <4 x float> %colorAccumulator.5, %55
  br label %57

; <label>:57                                      ; preds = %53, %54, %46
  %colorAccumulator.4 = phi <4 x float> [ %56, %54 ], [ %colorAccumulator.5, %53 ], [ %colorAccumulator.2, %46 ]
  %58 = fdiv <4 x float> %colorAccumulator.4, <float 9.000000e+000, float 9.000000e+000, float 9.000000e+000, float 9.000000e+000>
  store <4 x float> %58, <4 x float> addrspace(1)* %scevgep28, align 16
  %59 = add i32 %j.01, 1
  %exitcond = icmp eq i32 %59, %tmp7
  br i1 %exitcond, label %._crit_edge, label %24

._crit_edge:                                      ; preds = %57, %.preheader
  %60 = add i32 %i.04, 1
  %exitcond60 = icmp eq i32 %60, %tmp59
  br i1 %exitcond60, label %._crit_edge6, label %.preheader

._crit_edge6:                                     ; preds = %._crit_edge, %0
  ret void
}

define <4 x float> @evaluatePixel(%struct._image2d_t* %inputImage, <2 x i32> %outCrd) nounwind {
  %1 = tail call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %inputImage, i32 1, <2 x i32> %outCrd) nounwind
  ret <4 x float> %1
}

declare <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t*, i32, <2 x i32>)

define void @wlSimpleBoxBlur_image2d(%struct._image2d_t* %inputImage, <4 x float> addrspace(1)* nocapture %output, i32 %rowCountPerGlobalID) nounwind {
  %1 = tail call i32 @get_global_id(i32 0) nounwind
  %2 = mul i32 %1, %rowCountPerGlobalID
  %3 = tail call <2 x i32> @_Z13get_image_dimP10_image2d_t(%struct._image2d_t* %inputImage) nounwind
  %4 = add nsw i32 %2, %rowCountPerGlobalID
  %5 = extractelement <2 x i32> %3, i32 1
  %6 = tail call i32 @_Z3minii(i32 %4, i32 %5) nounwind
  %7 = icmp slt i32 %2, %6
  br i1 %7, label %bb.nph32, label %._crit_edge33

bb.nph32:                                         ; preds = %0
  %8 = extractelement <2 x i32> %3, i32 0
  %9 = mul nsw i32 %8, %2
  %10 = icmp sgt i32 %8, 0
  %tmp47 = sub i32 %6, %2
  %tmp50 = add i32 %2, -1
  %tmp52 = add i32 %2, 1
  br label %11

; <label>:11                                      ; preds = %49, %bb.nph32
  %indvar = phi i32 [ 0, %bb.nph32 ], [ %indvar.next, %49 ]
  %lowRightCrd.031 = phi <2 x i32> [ undef, %bb.nph32 ], [ %lowRightCrd.1.lcssa, %49 ]
  %lowLeftCrd.030 = phi <2 x i32> [ undef, %bb.nph32 ], [ %lowLeftCrd.1.lcssa, %49 ]
  %lowCrd.029 = phi <2 x i32> [ undef, %bb.nph32 ], [ %lowCrd.1.lcssa, %49 ]
  %upRightCrd.027 = phi <2 x i32> [ undef, %bb.nph32 ], [ %upRightCrd.1.lcssa, %49 ]
  %upLeftCrd.026 = phi <2 x i32> [ undef, %bb.nph32 ], [ %upLeftCrd.1.lcssa, %49 ]
  %index.125 = phi i32 [ %9, %bb.nph32 ], [ %index.0.lcssa, %49 ]
  %upCrd.024 = phi <2 x i32> [ undef, %bb.nph32 ], [ %upCrd.1.lcssa, %49 ]
  %curCrd.023 = phi <2 x i32> [ undef, %bb.nph32 ], [ %curCrd.1.lcssa, %49 ]
  %curLeftCrd.022 = phi <2 x i32> [ undef, %bb.nph32 ], [ %curLeftCrd.1.lcssa, %49 ]
  %curRightCrd.021 = phi <2 x i32> [ undef, %bb.nph32 ], [ %curRightCrd.1.lcssa, %49 ]
  %row.028 = add i32 %2, %indvar
  %tmp51 = add i32 %tmp50, %indvar
  %tmp53 = add i32 %tmp52, %indvar
  %12 = insertelement <2 x i32> %curCrd.023, i32 %row.028, i32 1
  %13 = insertelement <2 x i32> %curLeftCrd.022, i32 %row.028, i32 1
  %14 = insertelement <2 x i32> %curRightCrd.021, i32 %row.028, i32 1
  %15 = insertelement <2 x i32> %upCrd.024, i32 %tmp51, i32 1
  %16 = insertelement <2 x i32> %upLeftCrd.026, i32 %tmp51, i32 1
  %17 = insertelement <2 x i32> %upRightCrd.027, i32 %tmp51, i32 1
  %18 = insertelement <2 x i32> %lowCrd.029, i32 %tmp53, i32 1
  %19 = insertelement <2 x i32> %lowLeftCrd.030, i32 %tmp53, i32 1
  %20 = insertelement <2 x i32> %lowRightCrd.031, i32 %tmp53, i32 1
  br i1 %10, label %bb.nph, label %49

bb.nph:                                           ; preds = %11, %bb.nph
  %21 = phi i32 [ %tmp44, %bb.nph ], [ 0, %11 ]
  %lowRightCrd.110 = phi <2 x i32> [ %30, %bb.nph ], [ %20, %11 ]
  %lowLeftCrd.19 = phi <2 x i32> [ %29, %bb.nph ], [ %19, %11 ]
  %lowCrd.18 = phi <2 x i32> [ %28, %bb.nph ], [ %18, %11 ]
  %upRightCrd.17 = phi <2 x i32> [ %27, %bb.nph ], [ %17, %11 ]
  %upLeftCrd.16 = phi <2 x i32> [ %26, %bb.nph ], [ %16, %11 ]
  %upCrd.14 = phi <2 x i32> [ %25, %bb.nph ], [ %15, %11 ]
  %curCrd.13 = phi <2 x i32> [ %22, %bb.nph ], [ %12, %11 ]
  %curLeftCrd.12 = phi <2 x i32> [ %23, %bb.nph ], [ %13, %11 ]
  %curRightCrd.11 = phi <2 x i32> [ %24, %bb.nph ], [ %14, %11 ]
  %tmp43 = add i32 %21, -1
  %tmp44 = add i32 %21, 1
  %tmp45 = add i32 %index.125, %21
  %scevgep = getelementptr <4 x float> addrspace(1)* %output, i32 %tmp45
  %22 = insertelement <2 x i32> %curCrd.13, i32 %21, i32 0
  %23 = insertelement <2 x i32> %curLeftCrd.12, i32 %tmp43, i32 0
  %24 = insertelement <2 x i32> %curRightCrd.11, i32 %tmp44, i32 0
  %25 = insertelement <2 x i32> %upCrd.14, i32 %21, i32 0
  %26 = insertelement <2 x i32> %upLeftCrd.16, i32 %tmp43, i32 0
  %27 = insertelement <2 x i32> %upRightCrd.17, i32 %tmp44, i32 0
  %28 = insertelement <2 x i32> %lowCrd.18, i32 %21, i32 0
  %29 = insertelement <2 x i32> %lowLeftCrd.19, i32 %tmp43, i32 0
  %30 = insertelement <2 x i32> %lowRightCrd.110, i32 %tmp44, i32 0
  %31 = tail call <4 x float> @evaluatePixel(%struct._image2d_t* %inputImage, <2 x i32> %22)
  %32 = tail call <4 x float> @evaluatePixel(%struct._image2d_t* %inputImage, <2 x i32> %23)
  %33 = fadd <4 x float> %31, %32
  %34 = tail call <4 x float> @evaluatePixel(%struct._image2d_t* %inputImage, <2 x i32> %24)
  %35 = fadd <4 x float> %33, %34
  %36 = tail call <4 x float> @evaluatePixel(%struct._image2d_t* %inputImage, <2 x i32> %25)
  %37 = fadd <4 x float> %35, %36
  %38 = tail call <4 x float> @evaluatePixel(%struct._image2d_t* %inputImage, <2 x i32> %26)
  %39 = fadd <4 x float> %37, %38
  %40 = tail call <4 x float> @evaluatePixel(%struct._image2d_t* %inputImage, <2 x i32> %27)
  %41 = fadd <4 x float> %39, %40
  %42 = tail call <4 x float> @evaluatePixel(%struct._image2d_t* %inputImage, <2 x i32> %28)
  %43 = fadd <4 x float> %41, %42
  %44 = tail call <4 x float> @evaluatePixel(%struct._image2d_t* %inputImage, <2 x i32> %29)
  %45 = fadd <4 x float> %43, %44
  %46 = tail call <4 x float> @evaluatePixel(%struct._image2d_t* %inputImage, <2 x i32> %30)
  %47 = fadd <4 x float> %45, %46
  %48 = fdiv <4 x float> %47, <float 9.000000e+000, float 9.000000e+000, float 9.000000e+000, float 9.000000e+000>
  store <4 x float> %48, <4 x float> addrspace(1)* %scevgep, align 16
  %exitcond = icmp eq i32 %tmp44, %8
  br i1 %exitcond, label %._crit_edge, label %bb.nph

._crit_edge:                                      ; preds = %bb.nph
  %tmp = add i32 %8, %index.125
  br label %49

; <label>:49                                      ; preds = %._crit_edge, %11
  %lowRightCrd.1.lcssa = phi <2 x i32> [ %30, %._crit_edge ], [ %20, %11 ]
  %lowLeftCrd.1.lcssa = phi <2 x i32> [ %29, %._crit_edge ], [ %19, %11 ]
  %lowCrd.1.lcssa = phi <2 x i32> [ %28, %._crit_edge ], [ %18, %11 ]
  %upRightCrd.1.lcssa = phi <2 x i32> [ %27, %._crit_edge ], [ %17, %11 ]
  %upLeftCrd.1.lcssa = phi <2 x i32> [ %26, %._crit_edge ], [ %16, %11 ]
  %index.0.lcssa = phi i32 [ %tmp, %._crit_edge ], [ %index.125, %11 ]
  %upCrd.1.lcssa = phi <2 x i32> [ %25, %._crit_edge ], [ %15, %11 ]
  %curCrd.1.lcssa = phi <2 x i32> [ %22, %._crit_edge ], [ %12, %11 ]
  %curLeftCrd.1.lcssa = phi <2 x i32> [ %23, %._crit_edge ], [ %13, %11 ]
  %curRightCrd.1.lcssa = phi <2 x i32> [ %24, %._crit_edge ], [ %14, %11 ]
  %indvar.next = add i32 %indvar, 1
  %exitcond48 = icmp eq i32 %indvar.next, %tmp47
  br i1 %exitcond48, label %._crit_edge33, label %11

._crit_edge33:                                    ; preds = %49, %0
  ret void
}

declare <2 x i32> @_Z13get_image_dimP10_image2d_t(%struct._image2d_t*)

declare i32 @_Z3minii(i32, i32)

define void @wlSimpleBoxBlur_Optimized_CPU(<4 x float> addrspace(1)* %input, <4 x float> addrspace(1)* nocapture %output, i32 %width, i32 %height, i32 %buffer_size) nounwind {
; <label>:0
  %1 = tail call i32 (...)* @get_work_dim() nounwind
  %2 = tail call i32 @get_global_id(i32 0) nounwind
  %3 = tail call i32 @get_global_id(i32 1) nounwind
  %4 = tail call i32 @get_global_size(i32 0) nounwind
  %5 = tail call i32 @get_global_size(i32 1) nounwind
  %6 = icmp eq i32 %5, 0
  %7 = select i1 %6, i32 1, i32 %5
  %8 = udiv i32 %height, %7
  %9 = icmp eq i32 %4, 0
  %10 = select i1 %9, i32 1, i32 %4
  %11 = udiv i32 %width, %10
  %12 = mul i32 %2, %width
  %13 = udiv i32 %12, %10
  %14 = mul i32 %3, %height
  %15 = udiv i32 %14, %7
  %16 = add i32 %8, 1
  %17 = add i32 %16, %15
  %18 = icmp ult i32 %17, %height
  br i1 %18, label %22, label %19

; <label>:19                                      ; preds = %0
  %20 = add i32 %height, -1
  %21 = sub i32 %20, %15
  br label %22

; <label>:22                                      ; preds = %0, %19
  %count_y.0 = phi i32 [ %21, %19 ], [ %8, %0 ]
  %bottomEdge.0 = phi i8 [ 1, %19 ], [ 0, %0 ]
  %23 = add i32 %11, 1
  %24 = add i32 %23, %13
  %25 = icmp ult i32 %24, %width
  br i1 %25, label %29, label %26

; <label>:26                                      ; preds = %22
  %27 = add i32 %width, -1
  %28 = sub i32 %27, %13
  br label %29

; <label>:29                                      ; preds = %22, %26
  %count_x.0 = phi i32 [ %28, %26 ], [ %11, %22 ]
  %rightEdge.0 = phi i8 [ 1, %26 ], [ 0, %22 ]
  %30 = add i32 %count_y.0, -1
  %31 = icmp eq i32 %15, 0
  %.count_y.0 = select i1 %31, i32 %30, i32 %count_y.0
  %index_y.0 = select i1 %31, i32 1, i32 %15
  %32 = add i32 %count_x.0, -1
  %33 = icmp eq i32 %13, 0
  %.count_x.0 = select i1 %33, i32 %32, i32 %count_x.0
  %index_x.0 = select i1 %33, i32 1, i32 %13
  %tmp344 = icmp ugt i32 %4, 1
  %umax345 = select i1 %tmp344, i32 %4, i32 1
  %tmp346 = udiv i32 %12, %umax345
  %tmp347 = icmp ugt i32 %tmp346, 1
  %umax348 = select i1 %tmp347, i32 %tmp346, i32 1
  %tmp350 = icmp ugt i32 %5, 1
  %umax351 = select i1 %tmp350, i32 %5, i32 1
  %tmp352 = udiv i32 %14, %umax351
  %tmp353 = icmp ugt i32 %tmp352, 1
  %tmp352.op = add i32 %tmp352, -1
  %tmp355 = select i1 %tmp353, i32 %tmp352.op, i32 0
  %tmp356 = mul i32 %tmp355, %width
  %tmp357 = add i32 %umax348, %tmp356
  %tmp358 = add i32 %tmp357, -1
  %tmp363 = add i32 %tmp357, 1
  %tmp342.2 = shl i32 %width, 1
  %scevgep360 = getelementptr <4 x float> addrspace(1)* %input, i32 %tmp358
  %34 = load <4 x float> addrspace(1)* %scevgep360, align 16
  %scevgep362 = getelementptr <4 x float> addrspace(1)* %input, i32 %tmp357
  %35 = load <4 x float> addrspace(1)* %scevgep362, align 16
  %36 = fadd <4 x float> %34, zeroinitializer
  %scevgep365 = getelementptr <4 x float> addrspace(1)* %input, i32 %tmp363
  %tmp359.1 = add i32 %tmp358, %width
  %37 = load <4 x float> addrspace(1)* %scevgep365, align 16
  %38 = fadd <4 x float> %36, %35
  %scevgep360.1 = getelementptr <4 x float> addrspace(1)* %input, i32 %tmp359.1
  %tmp361.1 = add i32 %tmp357, %width
  %39 = fadd <4 x float> %38, %37
  %scevgep362.1 = getelementptr <4 x float> addrspace(1)* %input, i32 %tmp361.1
  %tmp364.1 = add i32 %tmp363, %width
  %40 = load <4 x float> addrspace(1)* %scevgep360.1, align 16
  %tmp359.2 = add i32 %tmp358, %tmp342.2
  %scevgep365.1 = getelementptr <4 x float> addrspace(1)* %input, i32 %tmp364.1
  %41 = fadd <4 x float> %39, %40
  %42 = load <4 x float> addrspace(1)* %scevgep362.1, align 16
  %tmp361.2 = add i32 %tmp357, %tmp342.2
  %scevgep360.2 = getelementptr <4 x float> addrspace(1)* %input, i32 %tmp359.2
  %43 = fadd <4 x float> %41, %42
  %44 = load <4 x float> addrspace(1)* %scevgep365.1, align 16
  %45 = load <4 x float> addrspace(1)* %scevgep360.2, align 16
  %tmp364.2 = add i32 %tmp363, %tmp342.2
  %scevgep362.2 = getelementptr <4 x float> addrspace(1)* %input, i32 %tmp361.2
  %46 = fadd <4 x float> %43, %44
  %47 = load <4 x float> addrspace(1)* %scevgep362.2, align 16
  %48 = fadd <4 x float> %46, %45
  %scevgep365.2 = getelementptr <4 x float> addrspace(1)* %input, i32 %tmp364.2
  %49 = load <4 x float> addrspace(1)* %scevgep365.2, align 16
  %50 = fadd <4 x float> %48, %47
  %51 = fadd <4 x float> %50, %49
  %52 = mul i32 %index_y.0, %width
  %53 = add i32 %52, %index_x.0
  %54 = fdiv <4 x float> %51, <float 9.000000e+000, float 9.000000e+000, float 9.000000e+000, float 9.000000e+000>
  %55 = getelementptr inbounds <4 x float> addrspace(1)* %output, i32 %53
  store <4 x float> %54, <4 x float> addrspace(1)* %55, align 16
  %56 = add i32 %index_y.0, -1
  %57 = mul i32 %56, %width
  %58 = add i32 %index_x.0, -1
  %59 = add i32 %58, %57
  %60 = icmp eq i32 %.count_y.0, 1
  %61 = add i32 %59, 2
  br i1 %60, label %.preheader20, label %bb.nph42

bb.nph42:                                         ; preds = %29
  %62 = icmp ugt i32 %.count_x.0, 1
  %tmp198 = add i32 %.count_x.0, -1
  %tmp256 = icmp ugt i32 %4, 1
  %umax257 = select i1 %tmp256, i32 %4, i32 1
  %tmp258 = udiv i32 %12, %umax257
  %tmp259 = icmp ugt i32 %tmp258, 1
  %umax260 = select i1 %tmp259, i32 %tmp258, i32 1
  %tmp262 = icmp ugt i32 %5, 1
  %umax263 = select i1 %tmp262, i32 %5, i32 1
  %tmp264 = udiv i32 %14, %umax263
  %tmp265 = icmp ugt i32 %tmp264, 1
  %umax266 = select i1 %tmp265, i32 %tmp264, i32 1
  %tmp267 = add i32 %.count_y.0, -1
  %tmp268 = icmp ugt i32 %tmp267, 1
  %umax269 = select i1 %tmp268, i32 %tmp267, i32 1
  %tmp270 = add i32 %umax266, %umax269
  %tmp271 = add i32 %tmp270, -1
  %tmp272 = mul i32 %tmp271, %width
  %tmp273 = add i32 %umax260, %tmp272
  %tmp275 = mul i32 %tmp270, %width
  %tmp276 = add i32 %umax260, %tmp275
  %tmp281 = add i32 %umax266, -1
  %tmp282 = mul i32 %tmp281, %width
  %tmp283 = add i32 %umax260, %tmp282
  %tmp284 = add i32 %tmp283, 2
  %tmp288 = mul i32 %umax266, %width
  %tmp289 = add i32 %umax260, %tmp288
  %tmp290 = add i32 %tmp289, 2
  %tmp294 = add i32 %umax266, 1
  %tmp295 = mul i32 %tmp294, %width
  %tmp296 = add i32 %umax260, %tmp295
  %tmp297 = add i32 %tmp296, 2
  %tmp301 = add i32 %tmp289, -1
  %tmp305 = add i32 %tmp296, -1
  %tmp310 = add i32 %tmp283, -1
  %tmp315 = add i32 %tmp289, 1
  %tmp322 = add i32 %umax266, 2
  %tmp323 = mul i32 %tmp322, %width
  %tmp324 = add i32 %umax260, %tmp323
  %tmp325 = add i32 %tmp324, 1
  %tmp330 = add i32 %tmp324, -1
  %tmp337 = add i32 %tmp283, 1
  br label %.preheader28

.preheader28:                                     ; preds = %._crit_edge35, %bb.nph42
  %row.040 = phi i32 [ 0, %bb.nph42 ], [ %77, %._crit_edge35 ]
  %firstBlockAccumulator.137 = phi <4 x float> [ %51, %bb.nph42 ], [ %89, %._crit_edge35 ]
  %tmp280 = mul i32 %row.040, %width
  %tmp285 = add i32 %tmp284, %tmp280
  %tmp291 = add i32 %tmp290, %tmp280
  %tmp298 = add i32 %tmp297, %tmp280
  %tmp302 = add i32 %tmp301, %tmp280
  %tmp306 = add i32 %tmp305, %tmp280
  %tmp311 = add i32 %tmp310, %tmp280
  %tmp316 = add i32 %tmp315, %tmp280
  %tmp320 = add i32 %tmp296, %tmp280
  %scevgep321 = getelementptr <4 x float> addrspace(1)* %output, i32 %tmp320
  %tmp326 = add i32 %tmp325, %tmp280
  %scevgep327 = getelementptr <4 x float> addrspace(1)* %input, i32 %tmp326
  %tmp328 = add i32 %tmp324, %tmp280
  %scevgep329 = getelementptr <4 x float> addrspace(1)* %input, i32 %tmp328
  %tmp331 = add i32 %tmp330, %tmp280
  %scevgep332 = getelementptr <4 x float> addrspace(1)* %input, i32 %tmp331
  %scevgep333 = getelementptr <4 x float> addrspace(1)* %input, i32 %tmp311
  %tmp335 = add i32 %tmp283, %tmp280
  %scevgep336 = getelementptr <4 x float> addrspace(1)* %input, i32 %tmp335
  %tmp338 = add i32 %tmp337, %tmp280
  %scevgep339 = getelementptr <4 x float> addrspace(1)* %input, i32 %tmp338
  br i1 %62, label %bb.nph34, label %._crit_edge35

.preheader20:                                     ; preds = %29, %..preheader20_crit_edge
  %.lcssa = phi i32 [ %tmp274, %..preheader20_crit_edge ], [ %61, %29 ]
  %sourceIndex.1.lcssa.in = phi i32 [ %tmp276, %..preheader20_crit_edge ], [ %53, %29 ]
  %topRowIndex.0.lcssa = phi i32 [ %tmp278, %..preheader20_crit_edge ], [ %59, %29 ]
  %firstBlockAccumulator.1.lcssa = phi <4 x float> [ %89, %..preheader20_crit_edge ], [ %51, %29 ]
  %sourceIndex.1.lcssa = add i32 %sourceIndex.1.lcssa.in, 1
  %63 = icmp ugt i32 %.count_x.0, 1
  br i1 %63, label %bb.nph26, label %._crit_edge27

bb.nph34:                                         ; preds = %.preheader28, %bb.nph34
  %indvar196 = phi i32 [ %indvar.next197, %bb.nph34 ], [ 0, %.preheader28 ]
  %colorAccumulator.032 = phi <4 x float> [ %75, %bb.nph34 ], [ %firstBlockAccumulator.137, %.preheader28 ]
  %tmp286 = add i32 %tmp285, %indvar196
  %scevgep250 = getelementptr <4 x float> addrspace(1)* %input, i32 %tmp286
  %tmp292 = add i32 %tmp291, %indvar196
  %scevgep245 = getelementptr <4 x float> addrspace(1)* %input, i32 %tmp292
  %tmp299 = add i32 %tmp298, %indvar196
  %scevgep241 = getelementptr <4 x float> addrspace(1)* %input, i32 %tmp299
  %tmp303 = add i32 %tmp302, %indvar196
  %scevgep237 = getelementptr <4 x float> addrspace(1)* %input, i32 %tmp303
  %tmp307 = add i32 %tmp306, %indvar196
  %scevgep232 = getelementptr <4 x float> addrspace(1)* %input, i32 %tmp307
  %tmp312 = add i32 %tmp311, %indvar196
  %scevgep225 = getelementptr <4 x float> addrspace(1)* %input, i32 %tmp312
  %tmp317 = add i32 %tmp316, %indvar196
  %scevgep218 = getelementptr <4 x float> addrspace(1)* %output, i32 %tmp317
  %64 = load <4 x float> addrspace(1)* %scevgep225, align 16
  %65 = fsub <4 x float> %colorAccumulator.032, %64
  %66 = load <4 x float> addrspace(1)* %scevgep250, align 16
  %67 = fadd <4 x float> %65, %66
  %68 = load <4 x float> addrspace(1)* %scevgep237, align 16
  %69 = fsub <4 x float> %67, %68
  %70 = load <4 x float> addrspace(1)* %scevgep245, align 16
  %71 = fadd <4 x float> %69, %70
  %72 = load <4 x float> addrspace(1)* %scevgep232, align 16
  %73 = fsub <4 x float> %71, %72
  %74 = load <4 x float> addrspace(1)* %scevgep241, align 16
  %75 = fadd <4 x float> %73, %74
  %76 = fdiv <4 x float> %75, <float 9.000000e+000, float 9.000000e+000, float 9.000000e+000, float 9.000000e+000>
  store <4 x float> %76, <4 x float> addrspace(1)* %scevgep218, align 16
  %indvar.next197 = add i32 %indvar196, 1
  %exitcond199 = icmp eq i32 %indvar.next197, %tmp198
  br i1 %exitcond199, label %._crit_edge35, label %bb.nph34

._crit_edge35:                                    ; preds = %bb.nph34, %.preheader28
  %77 = add i32 %row.040, 1
  %78 = load <4 x float> addrspace(1)* %scevgep333, align 16
  %79 = fsub <4 x float> %firstBlockAccumulator.137, %78
  %80 = load <4 x float> addrspace(1)* %scevgep332, align 16
  %81 = fadd <4 x float> %79, %80
  %82 = load <4 x float> addrspace(1)* %scevgep336, align 16
  %83 = fsub <4 x float> %81, %82
  %84 = load <4 x float> addrspace(1)* %scevgep329, align 16
  %85 = fadd <4 x float> %83, %84
  %86 = load <4 x float> addrspace(1)* %scevgep339, align 16
  %87 = fsub <4 x float> %85, %86
  %88 = load <4 x float> addrspace(1)* %scevgep327, align 16
  %89 = fadd <4 x float> %87, %88
  %90 = fdiv <4 x float> %89, <float 9.000000e+000, float 9.000000e+000, float 9.000000e+000, float 9.000000e+000>
  store <4 x float> %90, <4 x float> addrspace(1)* %scevgep321, align 16
  %exitcond279 = icmp eq i32 %77, %umax269
  br i1 %exitcond279, label %..preheader20_crit_edge, label %.preheader28

..preheader20_crit_edge:                          ; preds = %._crit_edge35
  %tmp274 = add i32 %tmp273, 1
  %tmp278 = add i32 %tmp273, -1
  br label %.preheader20

bb.nph26:                                         ; preds = %.preheader20
  %tmp172 = add i32 %.count_x.0, -1
  %tmp178 = shl i32 %width, 1
  %tmp179 = add i32 %topRowIndex.0.lcssa, %tmp178
  %tmp182 = add i32 %topRowIndex.0.lcssa, %width
  %tmp185 = add i32 %.lcssa, %tmp178
  %tmp186 = add i32 %tmp185, 1
  %tmp189 = add i32 %.lcssa, %width
  %tmp190 = add i32 %tmp189, 1
  %tmp193 = add i32 %.lcssa, 1
  br label %91

; <label>:91                                      ; preds = %91, %bb.nph26
  %indvar170 = phi i32 [ 0, %bb.nph26 ], [ %indvar.next171, %91 ]
  %colorAccumulator.224 = phi <4 x float> [ %firstBlockAccumulator.1.lcssa, %bb.nph26 ], [ %103, %91 ]
  %tmp174 = add i32 %sourceIndex.1.lcssa, %indvar170
  %scevgep175 = getelementptr <4 x float> addrspace(1)* %output, i32 %tmp174
  %tmp176 = add i32 %topRowIndex.0.lcssa, %indvar170
  %scevgep177 = getelementptr <4 x float> addrspace(1)* %input, i32 %tmp176
  %tmp180 = add i32 %tmp179, %indvar170
  %scevgep181 = getelementptr <4 x float> addrspace(1)* %input, i32 %tmp180
  %tmp183 = add i32 %tmp182, %indvar170
  %scevgep184 = getelementptr <4 x float> addrspace(1)* %input, i32 %tmp183
  %tmp187 = add i32 %tmp186, %indvar170
  %scevgep188 = getelementptr <4 x float> addrspace(1)* %input, i32 %tmp187
  %tmp191 = add i32 %tmp190, %indvar170
  %scevgep192 = getelementptr <4 x float> addrspace(1)* %input, i32 %tmp191
  %tmp194 = add i32 %tmp193, %indvar170
  %scevgep195 = getelementptr <4 x float> addrspace(1)* %input, i32 %tmp194
  %92 = load <4 x float> addrspace(1)* %scevgep177, align 16
  %93 = fsub <4 x float> %colorAccumulator.224, %92
  %94 = load <4 x float> addrspace(1)* %scevgep195, align 16
  %95 = fadd <4 x float> %93, %94
  %96 = load <4 x float> addrspace(1)* %scevgep184, align 16
  %97 = fsub <4 x float> %95, %96
  %98 = load <4 x float> addrspace(1)* %scevgep192, align 16
  %99 = fadd <4 x float> %97, %98
  %100 = load <4 x float> addrspace(1)* %scevgep181, align 16
  %101 = fsub <4 x float> %99, %100
  %102 = load <4 x float> addrspace(1)* %scevgep188, align 16
  %103 = fadd <4 x float> %101, %102
  %104 = fdiv <4 x float> %103, <float 9.000000e+000, float 9.000000e+000, float 9.000000e+000, float 9.000000e+000>
  store <4 x float> %104, <4 x float> addrspace(1)* %scevgep175, align 16
  %indvar.next171 = add i32 %indvar170, 1
  %exitcond173 = icmp eq i32 %indvar.next171, %tmp172
  br i1 %exitcond173, label %._crit_edge27, label %91

._crit_edge27:                                    ; preds = %91, %.preheader20
  %105 = or i32 %15, %13
  %or.cond = icmp eq i32 %105, 0
  br i1 %or.cond, label %106, label %120

; <label>:106                                     ; preds = %._crit_edge27
  %107 = load <4 x float> addrspace(1)* %input, align 16
  %108 = fadd <4 x float> %107, zeroinitializer
  %109 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 1
  %110 = load <4 x float> addrspace(1)* %109, align 16
  %111 = fadd <4 x float> %108, %110
  %112 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %width
  %113 = load <4 x float> addrspace(1)* %112, align 16
  %114 = fadd <4 x float> %111, %113
  %115 = add i32 %width, 1
  %116 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %115
  %117 = load <4 x float> addrspace(1)* %116, align 16
  %118 = fadd <4 x float> %114, %117
  %119 = fdiv <4 x float> %118, <float 9.000000e+000, float 9.000000e+000, float 9.000000e+000, float 9.000000e+000>
  store <4 x float> %119, <4 x float> addrspace(1)* %output, align 16
  br label %120

; <label>:120                                     ; preds = %._crit_edge27, %106
  br i1 %31, label %.preheader16, label %159

.preheader16:                                     ; preds = %120
  %121 = add i32 %.count_x.0, %index_x.0
  %122 = icmp ult i32 %index_x.0, %121
  br i1 %122, label %bb.nph18, label %._crit_edge19

bb.nph18:                                         ; preds = %.preheader16
  %tmp147 = icmp ugt i32 %4, 1
  %umax148 = select i1 %tmp147, i32 %4, i32 1
  %tmp149 = udiv i32 %12, %umax148
  %tmp150 = icmp ugt i32 %tmp149, 1
  %umax151 = select i1 %tmp150, i32 %tmp149, i32 1
  %tmp155 = add i32 %umax151, -1
  %tmp158 = add i32 %umax151, 1
  %tmp161 = add i32 %umax151, %width
  %tmp162 = add i32 %tmp161, -1
  %tmp167 = add i32 %tmp161, 1
  br label %123

; <label>:123                                     ; preds = %123, %bb.nph18
  %indvar143 = phi i32 [ 0, %bb.nph18 ], [ %indvar.next144, %123 ]
  %tmp152 = add i32 %umax151, %indvar143
  %scevgep153 = getelementptr <4 x float> addrspace(1)* %input, i32 %tmp152
  %scevgep154 = getelementptr <4 x float> addrspace(1)* %output, i32 %tmp152
  %tmp156 = add i32 %tmp155, %indvar143
  %scevgep157 = getelementptr <4 x float> addrspace(1)* %input, i32 %tmp156
  %tmp159 = add i32 %tmp158, %indvar143
  %scevgep160 = getelementptr <4 x float> addrspace(1)* %input, i32 %tmp159
  %tmp163 = add i32 %tmp162, %indvar143
  %scevgep164 = getelementptr <4 x float> addrspace(1)* %input, i32 %tmp163
  %tmp165 = add i32 %tmp161, %indvar143
  %scevgep166 = getelementptr <4 x float> addrspace(1)* %input, i32 %tmp165
  %tmp168 = add i32 %tmp167, %indvar143
  %scevgep169 = getelementptr <4 x float> addrspace(1)* %input, i32 %tmp168
  %124 = load <4 x float> addrspace(1)* %scevgep157, align 16
  %125 = fadd <4 x float> %124, zeroinitializer
  %126 = load <4 x float> addrspace(1)* %scevgep153, align 16
  %127 = fadd <4 x float> %125, %126
  %128 = load <4 x float> addrspace(1)* %scevgep160, align 16
  %129 = fadd <4 x float> %127, %128
  %130 = load <4 x float> addrspace(1)* %scevgep164, align 16
  %131 = fadd <4 x float> %129, %130
  %132 = load <4 x float> addrspace(1)* %scevgep166, align 16
  %133 = fadd <4 x float> %131, %132
  %134 = load <4 x float> addrspace(1)* %scevgep169, align 16
  %135 = fadd <4 x float> %133, %134
  %136 = fdiv <4 x float> %135, <float 9.000000e+000, float 9.000000e+000, float 9.000000e+000, float 9.000000e+000>
  store <4 x float> %136, <4 x float> addrspace(1)* %scevgep154, align 16
  %indvar.next144 = add i32 %indvar143, 1
  %exitcond145 = icmp eq i32 %indvar.next144, %.count_x.0
  br i1 %exitcond145, label %._crit_edge19, label %123

._crit_edge19:                                    ; preds = %123, %.preheader16
  %137 = xor i1 %31, true
  %138 = icmp eq i8 %rightEdge.0, 0
  %or.cond1 = or i1 %138, %137
  br i1 %or.cond1, label %159, label %139

; <label>:139                                     ; preds = %._crit_edge19
  %140 = add i32 %width, -2
  %141 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %140
  %142 = load <4 x float> addrspace(1)* %141, align 16
  %143 = fadd <4 x float> %142, zeroinitializer
  %144 = add i32 %width, -1
  %145 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %144
  %146 = load <4 x float> addrspace(1)* %145, align 16
  %147 = fadd <4 x float> %143, %146
  %148 = shl i32 %width, 1
  %149 = add i32 %148, -2
  %150 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %149
  %151 = load <4 x float> addrspace(1)* %150, align 16
  %152 = fadd <4 x float> %147, %151
  %153 = add i32 %148, -1
  %154 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %153
  %155 = load <4 x float> addrspace(1)* %154, align 16
  %156 = fadd <4 x float> %152, %155
  %157 = fdiv <4 x float> %156, <float 9.000000e+000, float 9.000000e+000, float 9.000000e+000, float 9.000000e+000>
  %158 = getelementptr inbounds <4 x float> addrspace(1)* %output, i32 %144
  store <4 x float> %157, <4 x float> addrspace(1)* %158, align 16
  br label %159

; <label>:159                                     ; preds = %120, %._crit_edge19, %139
  br i1 %33, label %.preheader12, label %.thread

.preheader12:                                     ; preds = %159
  %160 = add i32 %.count_y.0, %index_y.0
  %161 = icmp ult i32 %index_y.0, %160
  br i1 %161, label %bb.nph14, label %._crit_edge15

bb.nph14:                                         ; preds = %.preheader12
  %tmp117 = icmp ugt i32 %5, 1
  %umax118 = select i1 %tmp117, i32 %5, i32 1
  %tmp119 = udiv i32 %14, %umax118
  %tmp120 = icmp ugt i32 %tmp119, 1
  %umax121 = select i1 %tmp120, i32 %tmp119, i32 1
  %tmp122 = add i32 %umax121, -1
  %tmp123 = mul i32 %tmp122, %width
  %tmp126 = mul i32 %umax121, %width
  %tmp129 = add i32 %umax121, 1
  %tmp130 = mul i32 %tmp129, %width
  %tmp133 = add i32 %tmp123, 1
  %tmp136 = add i32 %tmp126, 1
  %tmp139 = add i32 %tmp130, 1
  br label %162

; <label>:162                                     ; preds = %162, %bb.nph14
  %indvar112 = phi i32 [ 0, %bb.nph14 ], [ %indvar.next113, %162 ]
  %tmp115 = mul i32 %indvar112, %width
  %tmp123366 = add i32 %tmp122, %indvar112
  %tmp124 = mul i32 %tmp123366, %width
  %scevgep125 = getelementptr <4 x float> addrspace(1)* %input, i32 %tmp124
  %tmp126367 = add i32 %umax121, %indvar112
  %tmp127 = mul i32 %tmp126367, %width
  %scevgep128 = getelementptr <4 x float> addrspace(1)* %input, i32 %tmp127
  %tmp130368 = add i32 %tmp129, %indvar112
  %tmp131 = mul i32 %tmp130368, %width
  %scevgep132 = getelementptr <4 x float> addrspace(1)* %input, i32 %tmp131
  %tmp134 = add i32 %tmp133, %tmp115
  %scevgep135 = getelementptr <4 x float> addrspace(1)* %input, i32 %tmp134
  %tmp137 = add i32 %tmp136, %tmp115
  %scevgep138 = getelementptr <4 x float> addrspace(1)* %input, i32 %tmp137
  %tmp140 = add i32 %tmp139, %tmp115
  %scevgep141 = getelementptr <4 x float> addrspace(1)* %input, i32 %tmp140
  %scevgep142 = getelementptr <4 x float> addrspace(1)* %output, i32 %tmp127
  %163 = load <4 x float> addrspace(1)* %scevgep125, align 16
  %164 = fadd <4 x float> %163, zeroinitializer
  %165 = load <4 x float> addrspace(1)* %scevgep128, align 16
  %166 = fadd <4 x float> %164, %165
  %167 = load <4 x float> addrspace(1)* %scevgep132, align 16
  %168 = fadd <4 x float> %166, %167
  %169 = load <4 x float> addrspace(1)* %scevgep135, align 16
  %170 = fadd <4 x float> %168, %169
  %171 = load <4 x float> addrspace(1)* %scevgep138, align 16
  %172 = fadd <4 x float> %170, %171
  %173 = load <4 x float> addrspace(1)* %scevgep141, align 16
  %174 = fadd <4 x float> %172, %173
  %175 = fdiv <4 x float> %174, <float 9.000000e+000, float 9.000000e+000, float 9.000000e+000, float 9.000000e+000>
  store <4 x float> %175, <4 x float> addrspace(1)* %scevgep142, align 16
  %indvar.next113 = add i32 %indvar112, 1
  %exitcond114 = icmp eq i32 %indvar.next113, %.count_y.0
  br i1 %exitcond114, label %._crit_edge15, label %162

._crit_edge15:                                    ; preds = %162, %.preheader12
  %176 = icmp eq i8 %bottomEdge.0, 0
  %177 = xor i1 %33, true
  %or.cond2 = or i1 %176, %177
  br i1 %or.cond2, label %.thread, label %178

; <label>:178                                     ; preds = %._crit_edge15
  %179 = add i32 %height, -2
  %180 = mul i32 %179, %width
  %181 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %180
  %182 = load <4 x float> addrspace(1)* %181, align 16
  %183 = fadd <4 x float> %182, zeroinitializer
  %184 = add i32 %180, 1
  %185 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %184
  %186 = load <4 x float> addrspace(1)* %185, align 16
  %187 = fadd <4 x float> %183, %186
  %188 = add i32 %height, -1
  %189 = mul i32 %188, %width
  %190 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %189
  %191 = load <4 x float> addrspace(1)* %190, align 16
  %192 = fadd <4 x float> %187, %191
  %193 = add i32 %189, 1
  %194 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %193
  %195 = load <4 x float> addrspace(1)* %194, align 16
  %196 = fadd <4 x float> %192, %195
  %197 = fdiv <4 x float> %196, <float 9.000000e+000, float 9.000000e+000, float 9.000000e+000, float 9.000000e+000>
  %198 = getelementptr inbounds <4 x float> addrspace(1)* %output, i32 %189
  store <4 x float> %197, <4 x float> addrspace(1)* %198, align 16
  br label %.thread

.thread:                                          ; preds = %159, %._crit_edge15, %178
  %199 = icmp eq i8 %bottomEdge.0, 0
  br i1 %199, label %.loopexit, label %.preheader

.preheader:                                       ; preds = %.thread
  %200 = add i32 %.count_x.0, %index_x.0
  %201 = icmp ult i32 %index_x.0, %200
  br i1 %201, label %bb.nph, label %.loopexit

bb.nph:                                           ; preds = %.preheader
  %tmp52 = icmp ugt i32 %4, 1
  %umax = select i1 %tmp52, i32 %4, i32 1
  %tmp53 = udiv i32 %12, %umax
  %tmp54 = icmp ugt i32 %tmp53, 1
  %umax55 = select i1 %tmp54, i32 %tmp53, i32 1
  %tmp56 = add i32 %height, -2
  %tmp57 = mul i32 %tmp56, %width
  %tmp58 = add i32 %umax55, %tmp57
  %tmp59 = add i32 %tmp58, -1
  %tmp63 = add i32 %tmp58, 1
  %tmp66 = add i32 %height, -1
  %tmp67 = mul i32 %tmp66, %width
  %tmp68 = add i32 %umax55, %tmp67
  %tmp69 = add i32 %tmp68, -1
  %tmp74 = add i32 %tmp68, 1
  br label %202

; <label>:202                                     ; preds = %202, %bb.nph
  %indvar = phi i32 [ 0, %bb.nph ], [ %indvar.next, %202 ]
  %tmp60 = add i32 %tmp59, %indvar
  %scevgep = getelementptr <4 x float> addrspace(1)* %input, i32 %tmp60
  %tmp61 = add i32 %tmp58, %indvar
  %scevgep62 = getelementptr <4 x float> addrspace(1)* %input, i32 %tmp61
  %tmp64 = add i32 %tmp63, %indvar
  %scevgep65 = getelementptr <4 x float> addrspace(1)* %input, i32 %tmp64
  %tmp70 = add i32 %tmp69, %indvar
  %scevgep71 = getelementptr <4 x float> addrspace(1)* %input, i32 %tmp70
  %tmp72 = add i32 %tmp68, %indvar
  %scevgep73 = getelementptr <4 x float> addrspace(1)* %input, i32 %tmp72
  %tmp75 = add i32 %tmp74, %indvar
  %scevgep76 = getelementptr <4 x float> addrspace(1)* %input, i32 %tmp75
  %scevgep77 = getelementptr <4 x float> addrspace(1)* %output, i32 %tmp72
  %203 = load <4 x float> addrspace(1)* %scevgep, align 16
  %204 = fadd <4 x float> %203, zeroinitializer
  %205 = load <4 x float> addrspace(1)* %scevgep62, align 16
  %206 = fadd <4 x float> %204, %205
  %207 = load <4 x float> addrspace(1)* %scevgep65, align 16
  %208 = fadd <4 x float> %206, %207
  %209 = load <4 x float> addrspace(1)* %scevgep71, align 16
  %210 = fadd <4 x float> %208, %209
  %211 = load <4 x float> addrspace(1)* %scevgep73, align 16
  %212 = fadd <4 x float> %210, %211
  %213 = load <4 x float> addrspace(1)* %scevgep76, align 16
  %214 = fadd <4 x float> %212, %213
  %215 = fdiv <4 x float> %214, <float 9.000000e+000, float 9.000000e+000, float 9.000000e+000, float 9.000000e+000>
  store <4 x float> %215, <4 x float> addrspace(1)* %scevgep77, align 16
  %indvar.next = add i32 %indvar, 1
  %exitcond = icmp eq i32 %indvar.next, %.count_x.0
  br i1 %exitcond, label %.loopexit, label %202

.loopexit:                                        ; preds = %.preheader, %202, %.thread
  %216 = icmp eq i8 %rightEdge.0, 0
  br i1 %216, label %.thread5, label %.preheader9

.preheader9:                                      ; preds = %.loopexit
  %217 = add i32 %.count_y.0, %index_y.0
  %218 = icmp ult i32 %index_y.0, %217
  br i1 %218, label %bb.nph11, label %._crit_edge

bb.nph11:                                         ; preds = %.preheader9
  %tmp83 = icmp ugt i32 %5, 1
  %umax84 = select i1 %tmp83, i32 %5, i32 1
  %tmp85 = udiv i32 %14, %umax84
  %tmp86 = icmp ugt i32 %tmp85, 1
  %umax87 = select i1 %tmp86, i32 %tmp85, i32 1
  %tmp88 = mul i32 %umax87, %width
  %tmp89 = add i32 %tmp88, -1
  %tmp92 = add i32 %umax87, 1
  %tmp93 = mul i32 %tmp92, %width
  %tmp94 = add i32 %tmp93, -1
  %tmp97 = add i32 %umax87, 2
  %tmp98 = mul i32 %tmp97, %width
  %tmp99 = add i32 %tmp98, -1
  %tmp102 = add i32 %tmp88, -2
  %tmp105 = add i32 %tmp93, -2
  %tmp108 = add i32 %tmp98, -2
  br label %219

; <label>:219                                     ; preds = %219, %bb.nph11
  %indvar78 = phi i32 [ 0, %bb.nph11 ], [ %indvar.next79, %219 ]
  %tmp81 = mul i32 %indvar78, %width
  %tmp90 = add i32 %tmp89, %tmp81
  %scevgep91 = getelementptr <4 x float> addrspace(1)* %input, i32 %tmp90
  %tmp95 = add i32 %tmp94, %tmp81
  %scevgep96 = getelementptr <4 x float> addrspace(1)* %input, i32 %tmp95
  %tmp100 = add i32 %tmp99, %tmp81
  %scevgep101 = getelementptr <4 x float> addrspace(1)* %input, i32 %tmp100
  %tmp103 = add i32 %tmp102, %tmp81
  %scevgep104 = getelementptr <4 x float> addrspace(1)* %input, i32 %tmp103
  %tmp106 = add i32 %tmp105, %tmp81
  %scevgep107 = getelementptr <4 x float> addrspace(1)* %input, i32 %tmp106
  %tmp109 = add i32 %tmp108, %tmp81
  %scevgep110 = getelementptr <4 x float> addrspace(1)* %input, i32 %tmp109
  %scevgep111 = getelementptr <4 x float> addrspace(1)* %output, i32 %tmp95
  %220 = load <4 x float> addrspace(1)* %scevgep91, align 16
  %221 = fadd <4 x float> %220, zeroinitializer
  %222 = load <4 x float> addrspace(1)* %scevgep96, align 16
  %223 = fadd <4 x float> %221, %222
  %224 = load <4 x float> addrspace(1)* %scevgep101, align 16
  %225 = fadd <4 x float> %223, %224
  %226 = load <4 x float> addrspace(1)* %scevgep104, align 16
  %227 = fadd <4 x float> %225, %226
  %228 = load <4 x float> addrspace(1)* %scevgep107, align 16
  %229 = fadd <4 x float> %227, %228
  %230 = load <4 x float> addrspace(1)* %scevgep110, align 16
  %231 = fadd <4 x float> %229, %230
  %232 = fdiv <4 x float> %231, <float 9.000000e+000, float 9.000000e+000, float 9.000000e+000, float 9.000000e+000>
  store <4 x float> %232, <4 x float> addrspace(1)* %scevgep111, align 16
  %indvar.next79 = add i32 %indvar78, 1
  %exitcond80 = icmp eq i32 %indvar.next79, %.count_y.0
  br i1 %exitcond80, label %._crit_edge, label %219

._crit_edge:                                      ; preds = %219, %.preheader9
  br i1 %199, label %.thread5, label %233

; <label>:233                                     ; preds = %._crit_edge
  %234 = add i32 %height, -1
  %235 = mul i32 %234, %width
  %236 = add i32 %235, -2
  %237 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %236
  %238 = load <4 x float> addrspace(1)* %237, align 16
  %239 = fadd <4 x float> %238, zeroinitializer
  %240 = add i32 %235, -1
  %241 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %240
  %242 = load <4 x float> addrspace(1)* %241, align 16
  %243 = fadd <4 x float> %239, %242
  %244 = mul i32 %height, %width
  %245 = add i32 %244, -2
  %246 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %245
  %247 = load <4 x float> addrspace(1)* %246, align 16
  %248 = fadd <4 x float> %243, %247
  %249 = add i32 %244, -1
  %250 = getelementptr inbounds <4 x float> addrspace(1)* %input, i32 %249
  %251 = load <4 x float> addrspace(1)* %250, align 16
  %252 = fadd <4 x float> %248, %251
  %253 = fdiv <4 x float> %252, <float 9.000000e+000, float 9.000000e+000, float 9.000000e+000, float 9.000000e+000>
  %254 = getelementptr inbounds <4 x float> addrspace(1)* %output, i32 %249
  store <4 x float> %253, <4 x float> addrspace(1)* %254, align 16
  br label %UnifiedReturnBlock

.thread5:                                         ; preds = %.loopexit, %._crit_edge
  br label %UnifiedReturnBlock

UnifiedReturnBlock:                               ; preds = %.thread5, %233
  ret void
}

!opencl.kernels = !{!0, !2, !3, !4}

!0 = metadata !{void (<4 x float> addrspace(1)*, <4 x float> addrspace(1)*, i32, i32, i32)* @wlSimpleBoxBlur_GPU, metadata !1, metadata !1, metadata !"", metadata !"float4 __attribute__((address_space(1))) *, float4 __attribute__((address_space(1))) *, uint const, uint const, uint const", metadata !"opencl_wlSimpleBoxBlur_GPU_locals_anchor"}
!1 = metadata !{i32 0, i32 0, i32 0}
!2 = metadata !{void (<4 x float> addrspace(1)*, <4 x float> addrspace(1)*, i32, i32, i32)* @wlSimpleBoxBlur_CPU, metadata !1, metadata !1, metadata !"", metadata !"float4 __attribute__((address_space(1))) *, float4 __attribute__((address_space(1))) *, uint const, uint const, uint const", metadata !"opencl_wlSimpleBoxBlur_CPU_locals_anchor"}
!3 = metadata !{void (%struct._image2d_t*, <4 x float> addrspace(1)*, i32)* @wlSimpleBoxBlur_image2d, metadata !1, metadata !1, metadata !"", metadata !"__rd image2d_t, float4 __attribute__((address_space(1))) *, uint const", metadata !"opencl_wlSimpleBoxBlur_image2d_locals_anchor"}
!4 = metadata !{void (<4 x float> addrspace(1)*, <4 x float> addrspace(1)*, i32, i32, i32)* @wlSimpleBoxBlur_Optimized_CPU, metadata !1, metadata !1, metadata !"", metadata !"float4 __attribute__((address_space(1))) *, float4 __attribute__((address_space(1))) *, uint const, uint const, uint const", metadata !"opencl_wlSimpleBoxBlur_Optimized_CPU_locals_anchor"}
