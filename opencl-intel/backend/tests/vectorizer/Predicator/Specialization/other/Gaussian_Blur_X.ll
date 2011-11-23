; RUN: llvm-as %s -o %t.bc
; RUN: opt -phicanon -predicate -specialize -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'CyberLink.50.bin'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"

; This module was already processed by -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -mergereturn -loopsimplify passes

; CHECK: @GaussianBlur_X
; CHECK-NOT: %{{[a-z\.0-9]}} %{{[a-z\.0-9]}} %{{[a-z\.0-9]}}
; CHECK: header{{[0-9]*}}:
; CHECK:   br i1 %jumpover{{[0-9]*}}, label %footer{{[0-9]*}}
; CHECK: header{{[0-9]*}}:
; CHECK:   br i1 %jumpover{{[0-9]*}}, label %footer{{[0-9]*}}
; CHECK: header{{[0-9]*}}:
; CHECK:   br i1 %jumpover{{[0-9]*}}, label %footer{{[0-9]*}}
; CHECK: header{{[0-9]*}}:
; CHECK:   br i1 %jumpover{{[0-9]*}}, label %footer{{[0-9]*}}
; CHECK: footer{{[0-9]*}}:                                        ; preds = %header{{[0-9]*}}
; CHECK: header{{[0-9]*}}:
; CHECK:   br i1 %jumpover{{[0-9]*}}, label %footer{{[0-9]*}}
; CHECK: footer{{[0-9]*}}:                                        ; preds = %header{{[0-9]*}}
; CHECK: header{{[0-9]*}}:
; CHECK:   br i1 %jumpover{{[0-9]*}}, label %footer{{[0-9]*}}
; CHECK: header{{[0-9]*}}:
; CHECK:   br i1 %jumpover{{[0-9]*}}, label %footer{{[0-9]*}}
; CHECK: header{{[0-9]*}}:
; CHECK:   br i1 %jumpover{{[0-9]*}}, label %footer{{[0-9]*}}
; CHECK: footer{{[0-9]*}}:                                        ; preds = %header{{[0-9]*}}
; CHECK: footer{{[0-9]*}}:                                        ; preds = %header{{[0-9]*}}
; CHECK: footer{{[0-9]*}}:                                        ; preds = %header{{[0-9]*}}
; CHECK: footer{{[0-9]*}}:                                        ; preds = %header{{[0-9]*}}
; CHECK: footer{{[0-9]*}}:                                        ; preds = %header{{[0-9]*}}
; CHECK: footer{{[0-9]*}}:                                        ; preds = %header{{[0-9]*}}
; CHECK: ret

%struct._image2d_t = type opaque

@imageSampler = addrspace(2) global i32 1, align 4

define void @GaussianBlur_Clean(%struct._image2d_t* %dst) nounwind {
  %1 = tail call i32 @get_global_id(i32 0) nounwind
  %2 = insertelement <2 x i32> undef, i32 %1, i32 0
  %3 = tail call i32 @get_global_id(i32 1) nounwind
  %4 = insertelement <2 x i32> %2, i32 %3, i32 1
  tail call void @_Z13write_imageuiP10_image2d_tDv2_iDv4_j(%struct._image2d_t* %dst, <2 x i32> %4, <4 x i32> zeroinitializer) nounwind
  ret void
}

declare i32 @get_global_id(i32)

declare void @_Z13write_imageuiP10_image2d_tDv2_iDv4_j(%struct._image2d_t*, <2 x i32>, <4 x i32>)

define void @GaussianBlur_X(%struct._image2d_t* %src, %struct._image2d_t* %dst, i32 addrspace(1)* %Table, i32 addrspace(1)* %SumWidth, i32 %width, i32 %height, i32 %BlurRadius, i32 %nRealWidth) nounwind {
; <label>:0
  %1 = tail call i32 @get_global_id(i32 0) nounwind
  %2 = tail call i32 @get_global_id(i32 1) nounwind
  %3 = tail call i32 @get_global_id(i32 0) nounwind
  %4 = tail call i32 @get_global_id(i32 1) nounwind
  %5 = mul i32 %4, %nRealWidth
  %6 = add i32 %5, %3
  %7 = icmp eq i32 %nRealWidth, 0
  %8 = icmp eq i32 %6, -2147483648
  %9 = icmp eq i32 %nRealWidth, -1
  %10 = and i1 %8, %9
  %11 = or i1 %7, %10
  %12 = select i1 %11, i32 1, i32 %nRealWidth
  %13 = sdiv i32 %6, %12
  %14 = srem i32 %6, %12
  %15 = icmp slt i32 %14, %BlurRadius
  %16 = icmp slt i32 %13, %BlurRadius
  %or.cond = or i1 %15, %16
  br i1 %or.cond, label %126, label %17

; <label>:17                                      ; preds = %0
  %18 = add nsw i32 %BlurRadius, %width
  %19 = icmp slt i32 %14, %18
  br i1 %19, label %20, label %126

; <label>:20                                      ; preds = %17
  %21 = add nsw i32 %BlurRadius, %height
  %22 = icmp slt i32 %13, %21
  br i1 %22, label %23, label %126

; <label>:23                                      ; preds = %20
  %24 = shl i32 %BlurRadius, 1
  %25 = or i32 %24, 1
  %26 = icmp sgt i32 %25, 0
  br i1 %26, label %bb.nph, label %._crit_edge

bb.nph:                                           ; preds = %23
  %tmp18 = add i32 %3, %5
  %tmp19 = sub i32 %tmp18, %BlurRadius
  br i1 %9, label %bb.nph.split.us.split.split.us.preheader, label %bb.nph.bb.nph.split_crit_edge

bb.nph.split.us.split.split.us.preheader:         ; preds = %bb.nph
  br label %bb.nph.split.us.split.split.us

bb.nph.bb.nph.split_crit_edge:                    ; preds = %bb.nph
  br i1 %7, label %bb.nph.split.split.us.preheader, label %bb.nph.split.split.bb.nph.split.split.split_crit_edge.preheader

bb.nph.split.split.us.preheader:                  ; preds = %bb.nph.bb.nph.split_crit_edge
  br label %bb.nph.split.split.us

bb.nph.split.split.bb.nph.split.split.split_crit_edge.preheader: ; preds = %bb.nph.bb.nph.split_crit_edge
  br label %bb.nph.split.split.bb.nph.split.split.split_crit_edge

bb.nph.split.us.split.split.us:                   ; preds = %bb.nph.split.us.split.split.us.preheader, %bb.nph.split.us.split.split.us
  %storemerge4.us.us12 = phi i32 [ %53, %bb.nph.split.us.split.split.us ], [ 0, %bb.nph.split.us.split.split.us.preheader ]
  %sum.02.us.us14 = phi <4 x i32> [ %52, %bb.nph.split.us.split.split.us ], [ zeroinitializer, %bb.nph.split.us.split.split.us.preheader ]
  %scevgep = getelementptr i32 addrspace(1)* %Table, i32 %storemerge4.us.us12
  %nShiftIndex.03.us.us13 = add i32 %tmp19, %storemerge4.us.us12
  %27 = icmp eq i32 %nShiftIndex.03.us.us13, -2147483648
  %28 = or i1 %7, %27
  %29 = select i1 %28, i32 1, i32 %nRealWidth
  %30 = sdiv i32 %nShiftIndex.03.us.us13, %29
  %31 = select i1 %27, i32 1, i32 %nRealWidth
  %32 = srem i32 %nShiftIndex.03.us.us13, %31
  %33 = insertelement <2 x i32> undef, i32 %32, i32 0
  %34 = insertelement <2 x i32> %33, i32 %30, i32 1
  %35 = load i32 addrspace(2)* @imageSampler, align 4
  %36 = tail call <4 x i32> @_Z12read_imageuiP10_image2d_tjDv2_i(%struct._image2d_t* %src, i32 %35, <2 x i32> %34) nounwind
  %37 = load i32 addrspace(1)* %scevgep, align 4
  %38 = extractelement <4 x i32> %36, i32 0
  %39 = mul i32 %38, %37
  %40 = extractelement <4 x i32> %sum.02.us.us14, i32 0
  %41 = add i32 %39, %40
  %42 = insertelement <4 x i32> %sum.02.us.us14, i32 %41, i32 0
  %43 = extractelement <4 x i32> %36, i32 1
  %44 = mul i32 %43, %37
  %45 = extractelement <4 x i32> %sum.02.us.us14, i32 1
  %46 = add i32 %44, %45
  %47 = insertelement <4 x i32> %42, i32 %46, i32 1
  %48 = extractelement <4 x i32> %36, i32 2
  %49 = mul i32 %48, %37
  %50 = extractelement <4 x i32> %sum.02.us.us14, i32 2
  %51 = add i32 %49, %50
  %52 = insertelement <4 x i32> %47, i32 %51, i32 2
  %53 = add nsw i32 %storemerge4.us.us12, 1
  %exitcond = icmp eq i32 %53, %25
  br i1 %exitcond, label %._crit_edge.loopexit73, label %bb.nph.split.us.split.split.us

bb.nph.split.split.us:                            ; preds = %bb.nph.split.split.us.preheader, %bb.nph.split.split.us
  %storemerge4.us6 = phi i32 [ %75, %bb.nph.split.split.us ], [ 0, %bb.nph.split.split.us.preheader ]
  %sum.02.us8 = phi <4 x i32> [ %74, %bb.nph.split.split.us ], [ zeroinitializer, %bb.nph.split.split.us.preheader ]
  %scevgep43 = getelementptr i32 addrspace(1)* %Table, i32 %storemerge4.us6
  %nShiftIndex.03.us7 = add i32 %tmp19, %storemerge4.us6
  %54 = select i1 %7, i32 1, i32 %nRealWidth
  %55 = sdiv i32 %nShiftIndex.03.us7, %54
  %56 = insertelement <2 x i32> <i32 0, i32 undef>, i32 %55, i32 1
  %57 = load i32 addrspace(2)* @imageSampler, align 4
  %58 = tail call <4 x i32> @_Z12read_imageuiP10_image2d_tjDv2_i(%struct._image2d_t* %src, i32 %57, <2 x i32> %56) nounwind
  %59 = load i32 addrspace(1)* %scevgep43, align 4
  %60 = extractelement <4 x i32> %58, i32 0
  %61 = mul i32 %60, %59
  %62 = extractelement <4 x i32> %sum.02.us8, i32 0
  %63 = add i32 %61, %62
  %64 = insertelement <4 x i32> %sum.02.us8, i32 %63, i32 0
  %65 = extractelement <4 x i32> %58, i32 1
  %66 = mul i32 %65, %59
  %67 = extractelement <4 x i32> %sum.02.us8, i32 1
  %68 = add i32 %66, %67
  %69 = insertelement <4 x i32> %64, i32 %68, i32 1
  %70 = extractelement <4 x i32> %58, i32 2
  %71 = mul i32 %70, %59
  %72 = extractelement <4 x i32> %sum.02.us8, i32 2
  %73 = add i32 %71, %72
  %74 = insertelement <4 x i32> %69, i32 %73, i32 2
  %75 = add nsw i32 %storemerge4.us6, 1
  %exitcond42 = icmp eq i32 %75, %25
  br i1 %exitcond42, label %._crit_edge.loopexit72, label %bb.nph.split.split.us

bb.nph.split.split.bb.nph.split.split.split_crit_edge: ; preds = %bb.nph.split.split.bb.nph.split.split.split_crit_edge.preheader, %bb.nph.split.split.bb.nph.split.split.split_crit_edge
  %storemerge4 = phi i32 [ %99, %bb.nph.split.split.bb.nph.split.split.split_crit_edge ], [ 0, %bb.nph.split.split.bb.nph.split.split.split_crit_edge.preheader ]
  %sum.02 = phi <4 x i32> [ %98, %bb.nph.split.split.bb.nph.split.split.split_crit_edge ], [ zeroinitializer, %bb.nph.split.split.bb.nph.split.split.split_crit_edge.preheader ]
  %scevgep61 = getelementptr i32 addrspace(1)* %Table, i32 %storemerge4
  %nShiftIndex.03 = add i32 %tmp19, %storemerge4
  %76 = select i1 %7, i32 1, i32 %nRealWidth
  %77 = sdiv i32 %nShiftIndex.03, %76
  %78 = srem i32 %nShiftIndex.03, %nRealWidth
  %79 = insertelement <2 x i32> undef, i32 %78, i32 0
  %80 = insertelement <2 x i32> %79, i32 %77, i32 1
  %81 = load i32 addrspace(2)* @imageSampler, align 4
  %82 = tail call <4 x i32> @_Z12read_imageuiP10_image2d_tjDv2_i(%struct._image2d_t* %src, i32 %81, <2 x i32> %80) nounwind
  %83 = load i32 addrspace(1)* %scevgep61, align 4
  %84 = extractelement <4 x i32> %82, i32 0
  %85 = mul i32 %84, %83
  %86 = extractelement <4 x i32> %sum.02, i32 0
  %87 = add i32 %85, %86
  %88 = insertelement <4 x i32> %sum.02, i32 %87, i32 0
  %89 = extractelement <4 x i32> %82, i32 1
  %90 = mul i32 %89, %83
  %91 = extractelement <4 x i32> %sum.02, i32 1
  %92 = add i32 %90, %91
  %93 = insertelement <4 x i32> %88, i32 %92, i32 1
  %94 = extractelement <4 x i32> %82, i32 2
  %95 = mul i32 %94, %83
  %96 = extractelement <4 x i32> %sum.02, i32 2
  %97 = add i32 %95, %96
  %98 = insertelement <4 x i32> %93, i32 %97, i32 2
  %99 = add nsw i32 %storemerge4, 1
  %exitcond60 = icmp eq i32 %99, %25
  br i1 %exitcond60, label %._crit_edge.loopexit, label %bb.nph.split.split.bb.nph.split.split.split_crit_edge

._crit_edge.loopexit:                             ; preds = %bb.nph.split.split.bb.nph.split.split.split_crit_edge
  br label %._crit_edge

._crit_edge.loopexit72:                           ; preds = %bb.nph.split.split.us
  br label %._crit_edge

._crit_edge.loopexit73:                           ; preds = %bb.nph.split.us.split.split.us
  br label %._crit_edge

._crit_edge:                                      ; preds = %._crit_edge.loopexit73, %._crit_edge.loopexit72, %._crit_edge.loopexit, %23
  %sum.0.lcssa = phi <4 x i32> [ zeroinitializer, %23 ], [ %98, %._crit_edge.loopexit ], [ %74, %._crit_edge.loopexit72 ], [ %52, %._crit_edge.loopexit73 ]
  %100 = sub nsw i32 %14, %BlurRadius
  %101 = getelementptr inbounds i32 addrspace(1)* %SumWidth, i32 %100
  %102 = load i32 addrspace(1)* %101, align 4
  %103 = extractelement <4 x i32> %sum.0.lcssa, i32 0
  %104 = icmp eq i32 %102, 0
  %105 = icmp eq i32 %103, -2147483648
  %106 = icmp eq i32 %102, -1
  %107 = and i1 %105, %106
  %108 = or i1 %104, %107
  %109 = select i1 %108, i32 1, i32 %102
  %110 = sdiv i32 %103, %109
  %111 = insertelement <4 x i32> undef, i32 %110, i32 0
  %112 = extractelement <4 x i32> %sum.0.lcssa, i32 1
  %113 = icmp eq i32 %112, -2147483648
  %114 = and i1 %113, %106
  %115 = or i1 %104, %114
  %116 = select i1 %115, i32 1, i32 %102
  %117 = sdiv i32 %112, %116
  %118 = insertelement <4 x i32> %111, i32 %117, i32 1
  %119 = extractelement <4 x i32> %sum.0.lcssa, i32 2
  %120 = icmp eq i32 %119, -2147483648
  %121 = and i1 %120, %106
  %122 = or i1 %104, %121
  %123 = select i1 %122, i32 1, i32 %102
  %124 = sdiv i32 %119, %123
  %125 = insertelement <4 x i32> %118, i32 %124, i32 2
  br label %126

; <label>:126                                     ; preds = %17, %20, %0, %._crit_edge
  %storemerge1 = phi <4 x i32> [ %125, %._crit_edge ], [ <i32 0, i32 0, i32 0, i32 undef>, %0 ], [ <i32 0, i32 0, i32 0, i32 undef>, %20 ], [ <i32 0, i32 0, i32 0, i32 undef>, %17 ]
  %127 = tail call i32 @get_global_id(i32 0) nounwind
  %128 = insertelement <2 x i32> undef, i32 %127, i32 0
  %129 = tail call i32 @get_global_id(i32 1) nounwind
  %130 = insertelement <2 x i32> %128, i32 %129, i32 1
  tail call void @_Z13write_imageuiP10_image2d_tDv2_iDv4_j(%struct._image2d_t* %dst, <2 x i32> %130, <4 x i32> %storemerge1) nounwind
  ret void
}

declare <4 x i32> @_Z12read_imageuiP10_image2d_tjDv2_i(%struct._image2d_t*, i32, <2 x i32>)
