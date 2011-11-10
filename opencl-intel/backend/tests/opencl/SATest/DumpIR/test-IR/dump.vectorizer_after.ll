; ModuleID = 'Program'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

define void @intel_median(<4 x i8> addrspace(1)* nocapture %pSrc, i32 addrspace(1)* nocapture %pDst, i32 %iImageWidth, i32 %iImageHeight) nounwind {
  %iPixels = alloca [9 x <4 x i32>], align 16
  %1 = call i32 @get_global_id(i32 0) nounwind
  %2 = icmp sgt i32 %iImageWidth, 0
  br i1 %2, label %bb.nph16, label %._crit_edge17

bb.nph16:                                         ; preds = %0
  %tmp43 = add i32 %1, 3
  %tmp44 = mul i32 %tmp43, %iImageWidth
  %tmp47 = add i32 %1, 2
  %tmp48 = mul i32 %tmp47, %iImageWidth
  %tmp51 = add i32 %1, 1
  %tmp52 = mul i32 %tmp51, %iImageWidth
  %tmp55 = add i32 %tmp44, -1
  %tmp58 = add i32 %tmp48, -1
  %tmp61 = add i32 %tmp52, -1
  %tmp64 = add i32 %tmp44, 1
  %tmp67 = add i32 %tmp48, 1
  %tmp70 = add i32 %tmp52, 1
  br label %bb.nph12

bb.nph12:                                         ; preds = %bb.nph16, %._crit_edge13
  %3 = phi <4 x i32> [ undef, %bb.nph16 ], [ %101, %._crit_edge13 ]
  %4 = phi <4 x i32> [ undef, %bb.nph16 ], [ %91, %._crit_edge13 ]
  %5 = phi <4 x i32> [ undef, %bb.nph16 ], [ %81, %._crit_edge13 ]
  %6 = phi <4 x i32> [ undef, %bb.nph16 ], [ %71, %._crit_edge13 ]
  %7 = phi <4 x i32> [ undef, %bb.nph16 ], [ %61, %._crit_edge13 ]
  %8 = phi <4 x i32> [ undef, %bb.nph16 ], [ %51, %._crit_edge13 ]
  %9 = phi <4 x i32> [ undef, %bb.nph16 ], [ %41, %._crit_edge13 ]
  %10 = phi <4 x i32> [ undef, %bb.nph16 ], [ %31, %._crit_edge13 ]
  %11 = phi <4 x i32> [ undef, %bb.nph16 ], [ %21, %._crit_edge13 ]
  %x.015 = phi i32 [ 0, %bb.nph16 ], [ %152, %._crit_edge13 ]
  %tmp49 = add i32 %tmp48, %x.015
  %scevgep73 = getelementptr i32 addrspace(1)* %pDst, i32 %tmp49
  %tmp71 = add i32 %tmp70, %x.015
  %tmp68 = add i32 %tmp67, %x.015
  %tmp65 = add i32 %tmp64, %x.015
  %tmp62 = add i32 %tmp61, %x.015
  %tmp59 = add i32 %tmp58, %x.015
  %tmp56 = add i32 %tmp55, %x.015
  %tmp53 = add i32 %tmp52, %x.015
  %tmp45 = add i32 %tmp44, %x.015
  %scevgep = getelementptr <4 x i8> addrspace(1)* %pSrc, i32 %tmp71
  %scevgep.1 = getelementptr <4 x i8> addrspace(1)* %pSrc, i32 %tmp68
  %scevgep.2 = getelementptr <4 x i8> addrspace(1)* %pSrc, i32 %tmp65
  %scevgep26 = getelementptr <4 x i8> addrspace(1)* %pSrc, i32 %tmp62
  %scevgep26.1 = getelementptr <4 x i8> addrspace(1)* %pSrc, i32 %tmp59
  %scevgep26.2 = getelementptr <4 x i8> addrspace(1)* %pSrc, i32 %tmp56
  %scevgep29 = getelementptr <4 x i8> addrspace(1)* %pSrc, i32 %tmp53
  %scevgep29.1 = getelementptr <4 x i8> addrspace(1)* %pSrc, i32 %tmp49
  %scevgep29.2 = getelementptr <4 x i8> addrspace(1)* %pSrc, i32 %tmp45
  %scevgep31 = getelementptr [9 x <4 x i32>]* %iPixels, i32 0, i32 0
  %scevgep33 = getelementptr [9 x <4 x i32>]* %iPixels, i32 0, i32 1
  %scevgep35 = getelementptr [9 x <4 x i32>]* %iPixels, i32 0, i32 2
  %12 = load <4 x i8> addrspace(1)* %scevgep26, align 4
  %13 = extractelement <4 x i8> %12, i32 0
  %14 = zext i8 %13 to i32
  %15 = insertelement <4 x i32> %11, i32 %14, i32 0
  %16 = extractelement <4 x i8> %12, i32 1
  %17 = zext i8 %16 to i32
  %18 = insertelement <4 x i32> %15, i32 %17, i32 1
  %19 = extractelement <4 x i8> %12, i32 2
  %20 = zext i8 %19 to i32
  %21 = insertelement <4 x i32> %18, i32 %20, i32 2
  store <4 x i32> %21, <4 x i32>* %scevgep31, align 16
  %22 = load <4 x i8> addrspace(1)* %scevgep29, align 4
  %23 = extractelement <4 x i8> %22, i32 0
  %24 = zext i8 %23 to i32
  %25 = insertelement <4 x i32> %10, i32 %24, i32 0
  %26 = extractelement <4 x i8> %22, i32 1
  %27 = zext i8 %26 to i32
  %28 = insertelement <4 x i32> %25, i32 %27, i32 1
  %29 = extractelement <4 x i8> %22, i32 2
  %30 = zext i8 %29 to i32
  %31 = insertelement <4 x i32> %28, i32 %30, i32 2
  store <4 x i32> %31, <4 x i32>* %scevgep33, align 16
  %32 = load <4 x i8> addrspace(1)* %scevgep, align 4
  %33 = extractelement <4 x i8> %32, i32 0
  %34 = zext i8 %33 to i32
  %35 = insertelement <4 x i32> %9, i32 %34, i32 0
  %36 = extractelement <4 x i8> %32, i32 1
  %37 = zext i8 %36 to i32
  %38 = insertelement <4 x i32> %35, i32 %37, i32 1
  %39 = extractelement <4 x i8> %32, i32 2
  %40 = zext i8 %39 to i32
  %41 = insertelement <4 x i32> %38, i32 %40, i32 2
  store <4 x i32> %41, <4 x i32>* %scevgep35, align 16
  %scevgep31.1 = getelementptr [9 x <4 x i32>]* %iPixels, i32 0, i32 3
  %scevgep33.1 = getelementptr [9 x <4 x i32>]* %iPixels, i32 0, i32 4
  %scevgep35.1 = getelementptr [9 x <4 x i32>]* %iPixels, i32 0, i32 5
  %42 = load <4 x i8> addrspace(1)* %scevgep26.1, align 4
  %43 = extractelement <4 x i8> %42, i32 0
  %44 = zext i8 %43 to i32
  %45 = insertelement <4 x i32> %8, i32 %44, i32 0
  %46 = extractelement <4 x i8> %42, i32 1
  %47 = zext i8 %46 to i32
  %48 = insertelement <4 x i32> %45, i32 %47, i32 1
  %49 = extractelement <4 x i8> %42, i32 2
  %50 = zext i8 %49 to i32
  %51 = insertelement <4 x i32> %48, i32 %50, i32 2
  store <4 x i32> %51, <4 x i32>* %scevgep31.1, align 16
  %52 = load <4 x i8> addrspace(1)* %scevgep29.1, align 4
  %53 = extractelement <4 x i8> %52, i32 0
  %54 = zext i8 %53 to i32
  %55 = insertelement <4 x i32> %7, i32 %54, i32 0
  %56 = extractelement <4 x i8> %52, i32 1
  %57 = zext i8 %56 to i32
  %58 = insertelement <4 x i32> %55, i32 %57, i32 1
  %59 = extractelement <4 x i8> %52, i32 2
  %60 = zext i8 %59 to i32
  %61 = insertelement <4 x i32> %58, i32 %60, i32 2
  store <4 x i32> %61, <4 x i32>* %scevgep33.1, align 16
  %62 = load <4 x i8> addrspace(1)* %scevgep.1, align 4
  %63 = extractelement <4 x i8> %62, i32 0
  %64 = zext i8 %63 to i32
  %65 = insertelement <4 x i32> %6, i32 %64, i32 0
  %66 = extractelement <4 x i8> %62, i32 1
  %67 = zext i8 %66 to i32
  %68 = insertelement <4 x i32> %65, i32 %67, i32 1
  %69 = extractelement <4 x i8> %62, i32 2
  %70 = zext i8 %69 to i32
  %71 = insertelement <4 x i32> %68, i32 %70, i32 2
  store <4 x i32> %71, <4 x i32>* %scevgep35.1, align 16
  %scevgep31.2 = getelementptr [9 x <4 x i32>]* %iPixels, i32 0, i32 6
  %scevgep33.2 = getelementptr [9 x <4 x i32>]* %iPixels, i32 0, i32 7
  %scevgep35.2 = getelementptr [9 x <4 x i32>]* %iPixels, i32 0, i32 8
  %72 = load <4 x i8> addrspace(1)* %scevgep26.2, align 4
  %73 = extractelement <4 x i8> %72, i32 0
  %74 = zext i8 %73 to i32
  %75 = insertelement <4 x i32> %5, i32 %74, i32 0
  %76 = extractelement <4 x i8> %72, i32 1
  %77 = zext i8 %76 to i32
  %78 = insertelement <4 x i32> %75, i32 %77, i32 1
  %79 = extractelement <4 x i8> %72, i32 2
  %80 = zext i8 %79 to i32
  %81 = insertelement <4 x i32> %78, i32 %80, i32 2
  store <4 x i32> %81, <4 x i32>* %scevgep31.2, align 16
  %82 = load <4 x i8> addrspace(1)* %scevgep29.2, align 4
  %83 = extractelement <4 x i8> %82, i32 0
  %84 = zext i8 %83 to i32
  %85 = insertelement <4 x i32> %4, i32 %84, i32 0
  %86 = extractelement <4 x i8> %82, i32 1
  %87 = zext i8 %86 to i32
  %88 = insertelement <4 x i32> %85, i32 %87, i32 1
  %89 = extractelement <4 x i8> %82, i32 2
  %90 = zext i8 %89 to i32
  %91 = insertelement <4 x i32> %88, i32 %90, i32 2
  store <4 x i32> %91, <4 x i32>* %scevgep33.2, align 16
  %92 = load <4 x i8> addrspace(1)* %scevgep.2, align 4
  %93 = extractelement <4 x i8> %92, i32 0
  %94 = zext i8 %93 to i32
  %95 = insertelement <4 x i32> %3, i32 %94, i32 0
  %96 = extractelement <4 x i8> %92, i32 1
  %97 = zext i8 %96 to i32
  %98 = insertelement <4 x i32> %95, i32 %97, i32 1
  %99 = extractelement <4 x i8> %92, i32 2
  %100 = zext i8 %99 to i32
  %101 = insertelement <4 x i32> %98, i32 %100, i32 2
  store <4 x i32> %101, <4 x i32>* %scevgep35.2, align 16
  br label %102

; <label>:102                                     ; preds = %102, %bb.nph12
  %iMax.08 = phi <4 x i32> [ <i32 255, i32 255, i32 255, i32 255>, %bb.nph12 ], [ %138, %102 ]
  %iMin.09 = phi <4 x i32> [ zeroinitializer, %bb.nph12 ], [ %135, %102 ]
  %iYes.010 = phi <4 x i32> [ <i32 128, i32 128, i32 128, i32 128>, %bb.nph12 ], [ %140, %102 ]
  %iSearch.011 = phi i32 [ 0, %bb.nph12 ], [ %141, %102 ]
  %103 = icmp slt <4 x i32> %iYes.010, %31
  %104 = icmp slt <4 x i32> %iYes.010, %21
  %105 = icmp slt <4 x i32> %iYes.010, %41
  %106 = sext <4 x i1> %103 to <4 x i32>
  %107 = sext <4 x i1> %104 to <4 x i32>
  %108 = sext <4 x i1> %105 to <4 x i32>
  %109 = add nsw <4 x i32> %107, %106
  %110 = icmp slt <4 x i32> %iYes.010, %51
  %111 = add nsw <4 x i32> %109, %108
  %112 = sext <4 x i1> %110 to <4 x i32>
  %113 = icmp slt <4 x i32> %iYes.010, %61
  %114 = add nsw <4 x i32> %111, %112
  %115 = sext <4 x i1> %113 to <4 x i32>
  %116 = icmp slt <4 x i32> %iYes.010, %71
  %117 = icmp slt <4 x i32> %iYes.010, %81
  %118 = add nsw <4 x i32> %114, %115
  %119 = sext <4 x i1> %116 to <4 x i32>
  %120 = icmp slt <4 x i32> %iYes.010, %91
  %121 = sext <4 x i1> %117 to <4 x i32>
  %122 = add nsw <4 x i32> %118, %119
  %123 = icmp slt <4 x i32> %iYes.010, %101
  %124 = sext <4 x i1> %120 to <4 x i32>
  %125 = add nsw <4 x i32> %122, %121
  %126 = sext <4 x i1> %123 to <4 x i32>
  %127 = add nsw <4 x i32> %125, %124
  %128 = add nsw <4 x i32> %127, %126
  %129 = sub nsw <4 x i32> zeroinitializer, %128
  %130 = icmp sgt <4 x i32> %129, <i32 4, i32 4, i32 4, i32 4>
  %131 = sext <4 x i1> %130 to <4 x i32>
  %132 = and <4 x i32> %iYes.010, %131
  %133 = xor <4 x i32> %131, <i32 -1, i32 -1, i32 -1, i32 -1>
  %134 = and <4 x i32> %iMin.09, %133
  %135 = or <4 x i32> %132, %134
  %136 = and <4 x i32> %iYes.010, %133
  %137 = and <4 x i32> %iMax.08, %131
  %138 = or <4 x i32> %136, %137
  %139 = add nsw <4 x i32> %138, %135
  %140 = ashr <4 x i32> %139, <i32 1, i32 1, i32 1, i32 1>
  %141 = add nsw i32 %iSearch.011, 1
  %exitcond = icmp eq i32 %141, 8
  br i1 %exitcond, label %._crit_edge13, label %102

._crit_edge13:                                    ; preds = %102
  %142 = extractelement <4 x i32> %140, i32 0
  %143 = and i32 %142, 255
  %144 = extractelement <4 x i32> %140, i32 1
  %145 = shl i32 %144, 8
  %146 = and i32 %145, 65280
  %147 = extractelement <4 x i32> %140, i32 2
  %148 = shl i32 %147, 16
  %149 = and i32 %148, 16711680
  %150 = or i32 %149, %143
  %151 = or i32 %150, %146
  store i32 %151, i32 addrspace(1)* %scevgep73, align 4
  %152 = add nsw i32 %x.015, 1
  %exitcond42 = icmp eq i32 %152, %iImageWidth
  br i1 %exitcond42, label %._crit_edge17.loopexit, label %bb.nph12

._crit_edge17.loopexit:                           ; preds = %._crit_edge13
  br label %._crit_edge17

._crit_edge17:                                    ; preds = %._crit_edge17.loopexit, %0
  ret void
}

declare i32 @get_global_id(i32)

define void @intel_median_scalar(i8 addrspace(1)* nocapture %pSrc, i32 addrspace(1)* nocapture %pDst, i32 %iImageWidth, i32 %iImageHeight) nounwind {
  %iResult = alloca [4 x i32], align 4
  %iPixels = alloca [9 x i32], align 4
  %1 = call i32 @get_global_id(i32 0) nounwind
  %2 = icmp sgt i32 %iImageWidth, 0
  br i1 %2, label %bb.nph20, label %._crit_edge21

bb.nph20:                                         ; preds = %0
  %3 = getelementptr inbounds [4 x i32]* %iResult, i32 0, i32 0
  %4 = getelementptr inbounds [4 x i32]* %iResult, i32 0, i32 1
  %5 = getelementptr inbounds [4 x i32]* %iResult, i32 0, i32 2
  %tmp97 = add i32 %1, 1
  %tmp98 = mul i32 %tmp97, %iImageWidth
  %tmp99 = shl i32 %tmp98, 2
  %tmp100 = add i32 %tmp99, 4
  %tmp104 = shl i32 %1, 2
  %tmp105 = add i32 %tmp104, 8
  %tmp106 = mul i32 %tmp105, %iImageWidth
  %tmp107 = add i32 %tmp106, 4
  %tmp111 = add i32 %tmp104, 12
  %tmp112 = mul i32 %tmp111, %iImageWidth
  %tmp113 = add i32 %tmp112, 4
  %tmp126 = add i32 %tmp99, -4
  %tmp130 = add i32 %tmp106, -4
  %tmp134 = add i32 %tmp112, -4
  %tmp138 = add i32 %1, 2
  %tmp139 = mul i32 %tmp138, %iImageWidth
  br label %bb.nph17

bb.nph17:                                         ; preds = %bb.nph20, %._crit_edge18
  %x.019 = phi i32 [ 0, %bb.nph20 ], [ %69, %._crit_edge18 ]
  %tmp = shl i32 %x.019, 2
  %tmp101 = add i32 %tmp100, %tmp
  %tmp108 = add i32 %tmp107, %tmp
  %tmp114 = add i32 %tmp113, %tmp
  %tmp117 = add i32 %tmp99, %tmp
  %tmp120 = add i32 %tmp106, %tmp
  %tmp123 = add i32 %tmp112, %tmp
  %tmp127 = add i32 %tmp126, %tmp
  %tmp131 = add i32 %tmp130, %tmp
  %tmp135 = add i32 %tmp134, %tmp
  %tmp140 = add i32 %tmp139, %x.019
  %scevgep141 = getelementptr i32 addrspace(1)* %pDst, i32 %tmp140
  br label %bb.nph12

bb.nph12:                                         ; preds = %bb.nph17, %._crit_edge13
  %ch.016 = phi i32 [ 0, %bb.nph17 ], [ %58, %._crit_edge13 ]
  %scevgep95 = getelementptr [4 x i32]* %iResult, i32 0, i32 %ch.016
  %tmp136 = add i32 %tmp135, %ch.016
  %tmp132 = add i32 %tmp131, %ch.016
  %tmp128 = add i32 %tmp127, %ch.016
  %tmp124 = add i32 %tmp123, %ch.016
  %tmp121 = add i32 %tmp120, %ch.016
  %tmp118 = add i32 %tmp117, %ch.016
  %tmp115 = add i32 %tmp114, %ch.016
  %tmp109 = add i32 %tmp108, %ch.016
  %tmp102 = add i32 %tmp101, %ch.016
  %scevgep39.2 = getelementptr i8 addrspace(1)* %pSrc, i32 %tmp136
  %scevgep39.1 = getelementptr i8 addrspace(1)* %pSrc, i32 %tmp132
  %scevgep39 = getelementptr i8 addrspace(1)* %pSrc, i32 %tmp128
  %scevgep34.2 = getelementptr i8 addrspace(1)* %pSrc, i32 %tmp124
  %scevgep34.1 = getelementptr i8 addrspace(1)* %pSrc, i32 %tmp121
  %scevgep34 = getelementptr i8 addrspace(1)* %pSrc, i32 %tmp118
  %scevgep.2 = getelementptr i8 addrspace(1)* %pSrc, i32 %tmp115
  %scevgep.1 = getelementptr i8 addrspace(1)* %pSrc, i32 %tmp109
  %scevgep = getelementptr i8 addrspace(1)* %pSrc, i32 %tmp102
  %scevgep41 = getelementptr [9 x i32]* %iPixels, i32 0, i32 0
  %scevgep43 = getelementptr [9 x i32]* %iPixels, i32 0, i32 1
  %scevgep45 = getelementptr [9 x i32]* %iPixels, i32 0, i32 2
  %6 = load i8 addrspace(1)* %scevgep39, align 1
  %7 = zext i8 %6 to i32
  store i32 %7, i32* %scevgep41, align 4
  %8 = load i8 addrspace(1)* %scevgep34, align 1
  %9 = zext i8 %8 to i32
  store i32 %9, i32* %scevgep43, align 4
  %10 = load i8 addrspace(1)* %scevgep, align 1
  %11 = zext i8 %10 to i32
  store i32 %11, i32* %scevgep45, align 4
  %scevgep41.1 = getelementptr [9 x i32]* %iPixels, i32 0, i32 3
  %scevgep43.1 = getelementptr [9 x i32]* %iPixels, i32 0, i32 4
  %scevgep45.1 = getelementptr [9 x i32]* %iPixels, i32 0, i32 5
  %12 = load i8 addrspace(1)* %scevgep39.1, align 1
  %13 = zext i8 %12 to i32
  store i32 %13, i32* %scevgep41.1, align 4
  %14 = load i8 addrspace(1)* %scevgep34.1, align 1
  %15 = zext i8 %14 to i32
  store i32 %15, i32* %scevgep43.1, align 4
  %16 = load i8 addrspace(1)* %scevgep.1, align 1
  %17 = zext i8 %16 to i32
  store i32 %17, i32* %scevgep45.1, align 4
  %scevgep41.2 = getelementptr [9 x i32]* %iPixels, i32 0, i32 6
  %scevgep43.2 = getelementptr [9 x i32]* %iPixels, i32 0, i32 7
  %scevgep45.2 = getelementptr [9 x i32]* %iPixels, i32 0, i32 8
  %18 = load i8 addrspace(1)* %scevgep39.2, align 1
  %19 = zext i8 %18 to i32
  store i32 %19, i32* %scevgep41.2, align 4
  %20 = load i8 addrspace(1)* %scevgep34.2, align 1
  %21 = zext i8 %20 to i32
  store i32 %21, i32* %scevgep43.2, align 4
  %22 = load i8 addrspace(1)* %scevgep.2, align 1
  %23 = zext i8 %22 to i32
  store i32 %23, i32* %scevgep45.2, align 4
  br label %24

; <label>:24                                      ; preds = %24, %bb.nph12
  %iYes.08 = phi i32 [ 128, %bb.nph12 ], [ %56, %24 ]
  %iMin.09 = phi i32 [ 0, %bb.nph12 ], [ %52, %24 ]
  %iMax.010 = phi i32 [ 255, %bb.nph12 ], [ %54, %24 ]
  %iSearch.011 = phi i32 [ 0, %bb.nph12 ], [ %57, %24 ]
  %25 = icmp slt i32 %iYes.08, %9
  %26 = icmp slt i32 %iYes.08, %7
  %27 = icmp slt i32 %iYes.08, %11
  %28 = zext i1 %25 to i32
  %29 = zext i1 %26 to i32
  %30 = add nsw i32 %29, %28
  %31 = zext i1 %27 to i32
  %32 = icmp slt i32 %iYes.08, %13
  %33 = add nsw i32 %30, %31
  %34 = zext i1 %32 to i32
  %35 = icmp slt i32 %iYes.08, %15
  %36 = zext i1 %35 to i32
  %37 = icmp slt i32 %iYes.08, %17
  %38 = add nsw i32 %34, %33
  %39 = icmp slt i32 %iYes.08, %19
  %40 = zext i1 %37 to i32
  %41 = add nsw i32 %38, %36
  %42 = icmp slt i32 %iYes.08, %21
  %43 = zext i1 %39 to i32
  %44 = add nsw i32 %41, %40
  %45 = add nsw i32 %43, %44
  %46 = icmp slt i32 %iYes.08, %23
  %47 = zext i1 %42 to i32
  %48 = add nsw i32 %45, %47
  %49 = zext i1 %46 to i32
  %50 = add nsw i32 %48, %49
  %51 = icmp sgt i32 %50, 4
  %52 = select i1 %51, i32 %iYes.08, i32 %iMin.09
  %53 = icmp slt i32 %50, 5
  %54 = select i1 %53, i32 %iYes.08, i32 %iMax.010
  %55 = add nsw i32 %54, %52
  %56 = ashr i32 %55, 1
  %57 = add nsw i32 %iSearch.011, 1
  %exitcond = icmp eq i32 %57, 8
  br i1 %exitcond, label %._crit_edge13, label %24

._crit_edge13:                                    ; preds = %24
  store i32 %56, i32* %scevgep95, align 4
  %58 = add nsw i32 %ch.016, 1
  %exitcond52 = icmp eq i32 %58, 3
  br i1 %exitcond52, label %._crit_edge18, label %bb.nph12

._crit_edge18:                                    ; preds = %._crit_edge13
  %59 = load i32* %3, align 4
  %60 = and i32 %59, 255
  %61 = load i32* %4, align 4
  %62 = shl i32 %61, 8
  %63 = and i32 %62, 65280
  %64 = load i32* %5, align 4
  %65 = shl i32 %64, 16
  %66 = and i32 %65, 16711680
  %67 = or i32 %63, %60
  %68 = or i32 %67, %66
  store i32 %68, i32 addrspace(1)* %scevgep141, align 4
  %69 = add nsw i32 %x.019, 1
  %exitcond96 = icmp eq i32 %69, %iImageWidth
  br i1 %exitcond96, label %._crit_edge21.loopexit, label %bb.nph17

._crit_edge21.loopexit:                           ; preds = %._crit_edge18
  br label %._crit_edge21

._crit_edge21:                                    ; preds = %._crit_edge21.loopexit, %0
  ret void
}

define void @__Vectorized_.intel_median(<4 x i8> addrspace(1)* nocapture %pSrc, i32 addrspace(1)* nocapture %pDst, i32 %iImageWidth, i32 %iImageHeight) nounwind {
  %temp = insertelement <4 x i32> undef, i32 %iImageWidth, i32 0
  %vector = shufflevector <4 x i32> %temp, <4 x i32> undef, <4 x i32> zeroinitializer
  %1 = call i32 @get_global_id(i32 0) nounwind
  %broadcast1 = insertelement <4 x i32> undef, i32 %1, i32 0
  %broadcast2 = shufflevector <4 x i32> %broadcast1, <4 x i32> undef, <4 x i32> zeroinitializer
  %2 = icmp sgt i32 %iImageWidth, 0
  br i1 %2, label %bb.nph16, label %._crit_edge17

bb.nph16:                                         ; preds = %0
  %tmp43100 = add <4 x i32> %broadcast2, <i32 3, i32 4, i32 5, i32 6>
  %tmp44101 = mul <4 x i32> %tmp43100, %vector
  %tmp47102 = add <4 x i32> %broadcast2, <i32 2, i32 3, i32 4, i32 5>
  %tmp48103 = mul <4 x i32> %tmp47102, %vector
  %tmp51104 = add <4 x i32> %broadcast2, <i32 1, i32 2, i32 3, i32 4>
  %tmp52105 = mul <4 x i32> %tmp51104, %vector
  %tmp55106 = add <4 x i32> %tmp44101, <i32 -1, i32 -1, i32 -1, i32 -1>
  %tmp58107 = add <4 x i32> %tmp48103, <i32 -1, i32 -1, i32 -1, i32 -1>
  %tmp61108 = add <4 x i32> %tmp52105, <i32 -1, i32 -1, i32 -1, i32 -1>
  %tmp64109 = add <4 x i32> %tmp44101, <i32 1, i32 1, i32 1, i32 1>
  %tmp67110 = add <4 x i32> %tmp48103, <i32 1, i32 1, i32 1, i32 1>
  %tmp70111 = add <4 x i32> %tmp52105, <i32 1, i32 1, i32 1, i32 1>
  br label %bb.nph12

bb.nph12:                                         ; preds = %._crit_edge13, %bb.nph16
  %x.015 = phi i32 [ 0, %bb.nph16 ], [ %229, %._crit_edge13 ]
  %temp112 = insertelement <4 x i32> undef, i32 %x.015, i32 0
  %vector113 = shufflevector <4 x i32> %temp112, <4 x i32> undef, <4 x i32> zeroinitializer
  %tmp49114 = add <4 x i32> %tmp48103, %vector113
  %extract = extractelement <4 x i32> %tmp49114, i32 0
  %extract115 = extractelement <4 x i32> %tmp49114, i32 1
  %extract116 = extractelement <4 x i32> %tmp49114, i32 2
  %extract117 = extractelement <4 x i32> %tmp49114, i32 3
  %3 = getelementptr i32 addrspace(1)* %pDst, i32 %extract
  %4 = getelementptr i32 addrspace(1)* %pDst, i32 %extract115
  %5 = getelementptr i32 addrspace(1)* %pDst, i32 %extract116
  %6 = getelementptr i32 addrspace(1)* %pDst, i32 %extract117
  %tmp71118 = add <4 x i32> %tmp70111, %vector113
  %extract126 = extractelement <4 x i32> %tmp71118, i32 0
  %extract127 = extractelement <4 x i32> %tmp71118, i32 1
  %extract128 = extractelement <4 x i32> %tmp71118, i32 2
  %extract129 = extractelement <4 x i32> %tmp71118, i32 3
  %tmp68119 = add <4 x i32> %tmp67110, %vector113
  %extract130 = extractelement <4 x i32> %tmp68119, i32 0
  %extract131 = extractelement <4 x i32> %tmp68119, i32 1
  %extract132 = extractelement <4 x i32> %tmp68119, i32 2
  %extract133 = extractelement <4 x i32> %tmp68119, i32 3
  %tmp65120 = add <4 x i32> %tmp64109, %vector113
  %extract134 = extractelement <4 x i32> %tmp65120, i32 0
  %extract135 = extractelement <4 x i32> %tmp65120, i32 1
  %extract136 = extractelement <4 x i32> %tmp65120, i32 2
  %extract137 = extractelement <4 x i32> %tmp65120, i32 3
  %tmp62121 = add <4 x i32> %tmp61108, %vector113
  %extract138 = extractelement <4 x i32> %tmp62121, i32 0
  %extract139 = extractelement <4 x i32> %tmp62121, i32 1
  %extract140 = extractelement <4 x i32> %tmp62121, i32 2
  %extract141 = extractelement <4 x i32> %tmp62121, i32 3
  %tmp59122 = add <4 x i32> %tmp58107, %vector113
  %extract142 = extractelement <4 x i32> %tmp59122, i32 0
  %extract143 = extractelement <4 x i32> %tmp59122, i32 1
  %extract144 = extractelement <4 x i32> %tmp59122, i32 2
  %extract145 = extractelement <4 x i32> %tmp59122, i32 3
  %tmp56123 = add <4 x i32> %tmp55106, %vector113
  %extract146 = extractelement <4 x i32> %tmp56123, i32 0
  %extract147 = extractelement <4 x i32> %tmp56123, i32 1
  %extract148 = extractelement <4 x i32> %tmp56123, i32 2
  %extract149 = extractelement <4 x i32> %tmp56123, i32 3
  %tmp53124 = add <4 x i32> %tmp52105, %vector113
  %extract150 = extractelement <4 x i32> %tmp53124, i32 0
  %extract151 = extractelement <4 x i32> %tmp53124, i32 1
  %extract152 = extractelement <4 x i32> %tmp53124, i32 2
  %extract153 = extractelement <4 x i32> %tmp53124, i32 3
  %tmp45125 = add <4 x i32> %tmp44101, %vector113
  %extract154 = extractelement <4 x i32> %tmp45125, i32 0
  %extract155 = extractelement <4 x i32> %tmp45125, i32 1
  %extract156 = extractelement <4 x i32> %tmp45125, i32 2
  %extract157 = extractelement <4 x i32> %tmp45125, i32 3
  %7 = getelementptr <4 x i8> addrspace(1)* %pSrc, i32 %extract126
  %8 = getelementptr <4 x i8> addrspace(1)* %pSrc, i32 %extract127
  %9 = getelementptr <4 x i8> addrspace(1)* %pSrc, i32 %extract128
  %10 = getelementptr <4 x i8> addrspace(1)* %pSrc, i32 %extract129
  %11 = getelementptr <4 x i8> addrspace(1)* %pSrc, i32 %extract130
  %12 = getelementptr <4 x i8> addrspace(1)* %pSrc, i32 %extract131
  %13 = getelementptr <4 x i8> addrspace(1)* %pSrc, i32 %extract132
  %14 = getelementptr <4 x i8> addrspace(1)* %pSrc, i32 %extract133
  %15 = getelementptr <4 x i8> addrspace(1)* %pSrc, i32 %extract134
  %16 = getelementptr <4 x i8> addrspace(1)* %pSrc, i32 %extract135
  %17 = getelementptr <4 x i8> addrspace(1)* %pSrc, i32 %extract136
  %18 = getelementptr <4 x i8> addrspace(1)* %pSrc, i32 %extract137
  %19 = getelementptr <4 x i8> addrspace(1)* %pSrc, i32 %extract138
  %20 = getelementptr <4 x i8> addrspace(1)* %pSrc, i32 %extract139
  %21 = getelementptr <4 x i8> addrspace(1)* %pSrc, i32 %extract140
  %22 = getelementptr <4 x i8> addrspace(1)* %pSrc, i32 %extract141
  %23 = getelementptr <4 x i8> addrspace(1)* %pSrc, i32 %extract142
  %24 = getelementptr <4 x i8> addrspace(1)* %pSrc, i32 %extract143
  %25 = getelementptr <4 x i8> addrspace(1)* %pSrc, i32 %extract144
  %26 = getelementptr <4 x i8> addrspace(1)* %pSrc, i32 %extract145
  %27 = getelementptr <4 x i8> addrspace(1)* %pSrc, i32 %extract146
  %28 = getelementptr <4 x i8> addrspace(1)* %pSrc, i32 %extract147
  %29 = getelementptr <4 x i8> addrspace(1)* %pSrc, i32 %extract148
  %30 = getelementptr <4 x i8> addrspace(1)* %pSrc, i32 %extract149
  %31 = getelementptr <4 x i8> addrspace(1)* %pSrc, i32 %extract150
  %32 = getelementptr <4 x i8> addrspace(1)* %pSrc, i32 %extract151
  %33 = getelementptr <4 x i8> addrspace(1)* %pSrc, i32 %extract152
  %34 = getelementptr <4 x i8> addrspace(1)* %pSrc, i32 %extract153
  %35 = getelementptr <4 x i8> addrspace(1)* %pSrc, i32 %extract
  %36 = getelementptr <4 x i8> addrspace(1)* %pSrc, i32 %extract115
  %37 = getelementptr <4 x i8> addrspace(1)* %pSrc, i32 %extract116
  %38 = getelementptr <4 x i8> addrspace(1)* %pSrc, i32 %extract117
  %39 = getelementptr <4 x i8> addrspace(1)* %pSrc, i32 %extract154
  %40 = getelementptr <4 x i8> addrspace(1)* %pSrc, i32 %extract155
  %41 = getelementptr <4 x i8> addrspace(1)* %pSrc, i32 %extract156
  %42 = getelementptr <4 x i8> addrspace(1)* %pSrc, i32 %extract157
  %43 = load <4 x i8> addrspace(1)* %19, align 4
  %44 = load <4 x i8> addrspace(1)* %20, align 4
  %45 = load <4 x i8> addrspace(1)* %21, align 4
  %46 = load <4 x i8> addrspace(1)* %22, align 4
  %shuffle0 = shufflevector <4 x i8> %43, <4 x i8> %44, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffle1 = shufflevector <4 x i8> %45, <4 x i8> %46, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffleMerge = shufflevector <4 x i8> %shuffle0, <4 x i8> %shuffle1, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffle0158 = shufflevector <4 x i8> %43, <4 x i8> %44, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffle1159 = shufflevector <4 x i8> %45, <4 x i8> %46, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffleMerge160 = shufflevector <4 x i8> %shuffle0158, <4 x i8> %shuffle1159, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %shuffle0161 = shufflevector <4 x i8> %43, <4 x i8> %44, <4 x i32> <i32 2, i32 3, i32 6, i32 7>
  %shuffle1162 = shufflevector <4 x i8> %45, <4 x i8> %46, <4 x i32> <i32 2, i32 3, i32 6, i32 7>
  %shuffleMerge163 = shufflevector <4 x i8> %shuffle0161, <4 x i8> %shuffle1162, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %47 = zext <4 x i8> %shuffleMerge to <4 x i32>
  %48 = zext <4 x i8> %shuffleMerge160 to <4 x i32>
  %49 = zext <4 x i8> %shuffleMerge163 to <4 x i32>
  %50 = load <4 x i8> addrspace(1)* %31, align 4
  %51 = load <4 x i8> addrspace(1)* %32, align 4
  %52 = load <4 x i8> addrspace(1)* %33, align 4
  %53 = load <4 x i8> addrspace(1)* %34, align 4
  %shuffle0164 = shufflevector <4 x i8> %50, <4 x i8> %51, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffle1165 = shufflevector <4 x i8> %52, <4 x i8> %53, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffleMerge166 = shufflevector <4 x i8> %shuffle0164, <4 x i8> %shuffle1165, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffle0167 = shufflevector <4 x i8> %50, <4 x i8> %51, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffle1168 = shufflevector <4 x i8> %52, <4 x i8> %53, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffleMerge169 = shufflevector <4 x i8> %shuffle0167, <4 x i8> %shuffle1168, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %shuffle0170 = shufflevector <4 x i8> %50, <4 x i8> %51, <4 x i32> <i32 2, i32 3, i32 6, i32 7>
  %shuffle1171 = shufflevector <4 x i8> %52, <4 x i8> %53, <4 x i32> <i32 2, i32 3, i32 6, i32 7>
  %shuffleMerge172 = shufflevector <4 x i8> %shuffle0170, <4 x i8> %shuffle1171, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %54 = zext <4 x i8> %shuffleMerge166 to <4 x i32>
  %55 = zext <4 x i8> %shuffleMerge169 to <4 x i32>
  %56 = zext <4 x i8> %shuffleMerge172 to <4 x i32>
  %57 = load <4 x i8> addrspace(1)* %7, align 4
  %58 = load <4 x i8> addrspace(1)* %8, align 4
  %59 = load <4 x i8> addrspace(1)* %9, align 4
  %60 = load <4 x i8> addrspace(1)* %10, align 4
  %shuffle0173 = shufflevector <4 x i8> %57, <4 x i8> %58, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffle1174 = shufflevector <4 x i8> %59, <4 x i8> %60, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffleMerge175 = shufflevector <4 x i8> %shuffle0173, <4 x i8> %shuffle1174, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffle0176 = shufflevector <4 x i8> %57, <4 x i8> %58, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffle1177 = shufflevector <4 x i8> %59, <4 x i8> %60, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffleMerge178 = shufflevector <4 x i8> %shuffle0176, <4 x i8> %shuffle1177, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %shuffle0179 = shufflevector <4 x i8> %57, <4 x i8> %58, <4 x i32> <i32 2, i32 3, i32 6, i32 7>
  %shuffle1180 = shufflevector <4 x i8> %59, <4 x i8> %60, <4 x i32> <i32 2, i32 3, i32 6, i32 7>
  %shuffleMerge181 = shufflevector <4 x i8> %shuffle0179, <4 x i8> %shuffle1180, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %61 = zext <4 x i8> %shuffleMerge175 to <4 x i32>
  %62 = zext <4 x i8> %shuffleMerge178 to <4 x i32>
  %63 = zext <4 x i8> %shuffleMerge181 to <4 x i32>
  %64 = load <4 x i8> addrspace(1)* %23, align 4
  %65 = load <4 x i8> addrspace(1)* %24, align 4
  %66 = load <4 x i8> addrspace(1)* %25, align 4
  %67 = load <4 x i8> addrspace(1)* %26, align 4
  %shuffle0182 = shufflevector <4 x i8> %64, <4 x i8> %65, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffle1183 = shufflevector <4 x i8> %66, <4 x i8> %67, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffleMerge184 = shufflevector <4 x i8> %shuffle0182, <4 x i8> %shuffle1183, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffle0185 = shufflevector <4 x i8> %64, <4 x i8> %65, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffle1186 = shufflevector <4 x i8> %66, <4 x i8> %67, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffleMerge187 = shufflevector <4 x i8> %shuffle0185, <4 x i8> %shuffle1186, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %shuffle0188 = shufflevector <4 x i8> %64, <4 x i8> %65, <4 x i32> <i32 2, i32 3, i32 6, i32 7>
  %shuffle1189 = shufflevector <4 x i8> %66, <4 x i8> %67, <4 x i32> <i32 2, i32 3, i32 6, i32 7>
  %shuffleMerge190 = shufflevector <4 x i8> %shuffle0188, <4 x i8> %shuffle1189, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %68 = zext <4 x i8> %shuffleMerge184 to <4 x i32>
  %69 = zext <4 x i8> %shuffleMerge187 to <4 x i32>
  %70 = zext <4 x i8> %shuffleMerge190 to <4 x i32>
  %71 = load <4 x i8> addrspace(1)* %35, align 4
  %72 = load <4 x i8> addrspace(1)* %36, align 4
  %73 = load <4 x i8> addrspace(1)* %37, align 4
  %74 = load <4 x i8> addrspace(1)* %38, align 4
  %shuffle0191 = shufflevector <4 x i8> %71, <4 x i8> %72, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffle1192 = shufflevector <4 x i8> %73, <4 x i8> %74, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffleMerge193 = shufflevector <4 x i8> %shuffle0191, <4 x i8> %shuffle1192, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffle0194 = shufflevector <4 x i8> %71, <4 x i8> %72, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffle1195 = shufflevector <4 x i8> %73, <4 x i8> %74, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffleMerge196 = shufflevector <4 x i8> %shuffle0194, <4 x i8> %shuffle1195, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %shuffle0197 = shufflevector <4 x i8> %71, <4 x i8> %72, <4 x i32> <i32 2, i32 3, i32 6, i32 7>
  %shuffle1198 = shufflevector <4 x i8> %73, <4 x i8> %74, <4 x i32> <i32 2, i32 3, i32 6, i32 7>
  %shuffleMerge199 = shufflevector <4 x i8> %shuffle0197, <4 x i8> %shuffle1198, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %75 = zext <4 x i8> %shuffleMerge193 to <4 x i32>
  %76 = zext <4 x i8> %shuffleMerge196 to <4 x i32>
  %77 = zext <4 x i8> %shuffleMerge199 to <4 x i32>
  %78 = load <4 x i8> addrspace(1)* %11, align 4
  %79 = load <4 x i8> addrspace(1)* %12, align 4
  %80 = load <4 x i8> addrspace(1)* %13, align 4
  %81 = load <4 x i8> addrspace(1)* %14, align 4
  %shuffle0200 = shufflevector <4 x i8> %78, <4 x i8> %79, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffle1201 = shufflevector <4 x i8> %80, <4 x i8> %81, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffleMerge202 = shufflevector <4 x i8> %shuffle0200, <4 x i8> %shuffle1201, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffle0203 = shufflevector <4 x i8> %78, <4 x i8> %79, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffle1204 = shufflevector <4 x i8> %80, <4 x i8> %81, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffleMerge205 = shufflevector <4 x i8> %shuffle0203, <4 x i8> %shuffle1204, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %shuffle0206 = shufflevector <4 x i8> %78, <4 x i8> %79, <4 x i32> <i32 2, i32 3, i32 6, i32 7>
  %shuffle1207 = shufflevector <4 x i8> %80, <4 x i8> %81, <4 x i32> <i32 2, i32 3, i32 6, i32 7>
  %shuffleMerge208 = shufflevector <4 x i8> %shuffle0206, <4 x i8> %shuffle1207, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %82 = zext <4 x i8> %shuffleMerge202 to <4 x i32>
  %83 = zext <4 x i8> %shuffleMerge205 to <4 x i32>
  %84 = zext <4 x i8> %shuffleMerge208 to <4 x i32>
  %85 = load <4 x i8> addrspace(1)* %27, align 4
  %86 = load <4 x i8> addrspace(1)* %28, align 4
  %87 = load <4 x i8> addrspace(1)* %29, align 4
  %88 = load <4 x i8> addrspace(1)* %30, align 4
  %shuffle0209 = shufflevector <4 x i8> %85, <4 x i8> %86, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffle1210 = shufflevector <4 x i8> %87, <4 x i8> %88, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffleMerge211 = shufflevector <4 x i8> %shuffle0209, <4 x i8> %shuffle1210, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffle0212 = shufflevector <4 x i8> %85, <4 x i8> %86, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffle1213 = shufflevector <4 x i8> %87, <4 x i8> %88, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffleMerge214 = shufflevector <4 x i8> %shuffle0212, <4 x i8> %shuffle1213, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %shuffle0215 = shufflevector <4 x i8> %85, <4 x i8> %86, <4 x i32> <i32 2, i32 3, i32 6, i32 7>
  %shuffle1216 = shufflevector <4 x i8> %87, <4 x i8> %88, <4 x i32> <i32 2, i32 3, i32 6, i32 7>
  %shuffleMerge217 = shufflevector <4 x i8> %shuffle0215, <4 x i8> %shuffle1216, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %89 = zext <4 x i8> %shuffleMerge211 to <4 x i32>
  %90 = zext <4 x i8> %shuffleMerge214 to <4 x i32>
  %91 = zext <4 x i8> %shuffleMerge217 to <4 x i32>
  %92 = load <4 x i8> addrspace(1)* %39, align 4
  %93 = load <4 x i8> addrspace(1)* %40, align 4
  %94 = load <4 x i8> addrspace(1)* %41, align 4
  %95 = load <4 x i8> addrspace(1)* %42, align 4
  %shuffle0218 = shufflevector <4 x i8> %92, <4 x i8> %93, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffle1219 = shufflevector <4 x i8> %94, <4 x i8> %95, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffleMerge220 = shufflevector <4 x i8> %shuffle0218, <4 x i8> %shuffle1219, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffle0221 = shufflevector <4 x i8> %92, <4 x i8> %93, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffle1222 = shufflevector <4 x i8> %94, <4 x i8> %95, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffleMerge223 = shufflevector <4 x i8> %shuffle0221, <4 x i8> %shuffle1222, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %shuffle0224 = shufflevector <4 x i8> %92, <4 x i8> %93, <4 x i32> <i32 2, i32 3, i32 6, i32 7>
  %shuffle1225 = shufflevector <4 x i8> %94, <4 x i8> %95, <4 x i32> <i32 2, i32 3, i32 6, i32 7>
  %shuffleMerge226 = shufflevector <4 x i8> %shuffle0224, <4 x i8> %shuffle1225, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %96 = zext <4 x i8> %shuffleMerge220 to <4 x i32>
  %97 = zext <4 x i8> %shuffleMerge223 to <4 x i32>
  %98 = zext <4 x i8> %shuffleMerge226 to <4 x i32>
  %99 = load <4 x i8> addrspace(1)* %15, align 4
  %100 = load <4 x i8> addrspace(1)* %16, align 4
  %101 = load <4 x i8> addrspace(1)* %17, align 4
  %102 = load <4 x i8> addrspace(1)* %18, align 4
  %shuffle0227 = shufflevector <4 x i8> %99, <4 x i8> %100, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffle1228 = shufflevector <4 x i8> %101, <4 x i8> %102, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffleMerge229 = shufflevector <4 x i8> %shuffle0227, <4 x i8> %shuffle1228, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffle0230 = shufflevector <4 x i8> %99, <4 x i8> %100, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffle1231 = shufflevector <4 x i8> %101, <4 x i8> %102, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffleMerge232 = shufflevector <4 x i8> %shuffle0230, <4 x i8> %shuffle1231, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %shuffle0233 = shufflevector <4 x i8> %99, <4 x i8> %100, <4 x i32> <i32 2, i32 3, i32 6, i32 7>
  %shuffle1234 = shufflevector <4 x i8> %101, <4 x i8> %102, <4 x i32> <i32 2, i32 3, i32 6, i32 7>
  %shuffleMerge235 = shufflevector <4 x i8> %shuffle0233, <4 x i8> %shuffle1234, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %103 = zext <4 x i8> %shuffleMerge229 to <4 x i32>
  %104 = zext <4 x i8> %shuffleMerge232 to <4 x i32>
  %105 = zext <4 x i8> %shuffleMerge235 to <4 x i32>
  br label %106

; <label>:106                                     ; preds = %106, %bb.nph12
  %vectorPHI = phi <4 x i32> [ <i32 255, i32 255, i32 255, i32 255>, %bb.nph12 ], [ %212, %106 ]
  %vectorPHI236 = phi <4 x i32> [ <i32 255, i32 255, i32 255, i32 255>, %bb.nph12 ], [ %213, %106 ]
  %vectorPHI237 = phi <4 x i32> [ <i32 255, i32 255, i32 255, i32 255>, %bb.nph12 ], [ %214, %106 ]
  %vectorPHI238 = phi <4 x i32> [ zeroinitializer, %bb.nph12 ], [ %203, %106 ]
  %vectorPHI239 = phi <4 x i32> [ zeroinitializer, %bb.nph12 ], [ %204, %106 ]
  %vectorPHI240 = phi <4 x i32> [ zeroinitializer, %bb.nph12 ], [ %205, %106 ]
  %vectorPHI241 = phi <4 x i32> [ <i32 128, i32 128, i32 128, i32 128>, %bb.nph12 ], [ %218, %106 ]
  %vectorPHI242 = phi <4 x i32> [ <i32 128, i32 128, i32 128, i32 128>, %bb.nph12 ], [ %219, %106 ]
  %vectorPHI243 = phi <4 x i32> [ <i32 128, i32 128, i32 128, i32 128>, %bb.nph12 ], [ %220, %106 ]
  %iSearch.011 = phi i32 [ 0, %bb.nph12 ], [ %221, %106 ]
  %107 = icmp sgt <4 x i32> %54, %vectorPHI241
  %108 = icmp sgt <4 x i32> %55, %vectorPHI242
  %109 = icmp sgt <4 x i32> %56, %vectorPHI243
  %110 = icmp sgt <4 x i32> %47, %vectorPHI241
  %111 = icmp sgt <4 x i32> %48, %vectorPHI242
  %112 = icmp sgt <4 x i32> %49, %vectorPHI243
  %113 = icmp sgt <4 x i32> %61, %vectorPHI241
  %114 = icmp sgt <4 x i32> %62, %vectorPHI242
  %115 = icmp sgt <4 x i32> %63, %vectorPHI243
  %116 = sext <4 x i1> %107 to <4 x i32>
  %117 = sext <4 x i1> %108 to <4 x i32>
  %118 = sext <4 x i1> %109 to <4 x i32>
  %119 = sext <4 x i1> %110 to <4 x i32>
  %120 = sext <4 x i1> %111 to <4 x i32>
  %121 = sext <4 x i1> %112 to <4 x i32>
  %122 = sext <4 x i1> %113 to <4 x i32>
  %123 = sext <4 x i1> %114 to <4 x i32>
  %124 = sext <4 x i1> %115 to <4 x i32>
  %125 = add nsw <4 x i32> %119, %116
  %126 = add nsw <4 x i32> %120, %117
  %127 = add nsw <4 x i32> %121, %118
  %128 = icmp sgt <4 x i32> %68, %vectorPHI241
  %129 = icmp sgt <4 x i32> %69, %vectorPHI242
  %130 = icmp sgt <4 x i32> %70, %vectorPHI243
  %131 = add nsw <4 x i32> %125, %122
  %132 = add nsw <4 x i32> %126, %123
  %133 = add nsw <4 x i32> %127, %124
  %134 = sext <4 x i1> %128 to <4 x i32>
  %135 = sext <4 x i1> %129 to <4 x i32>
  %136 = sext <4 x i1> %130 to <4 x i32>
  %137 = icmp sgt <4 x i32> %75, %vectorPHI241
  %138 = icmp sgt <4 x i32> %76, %vectorPHI242
  %139 = icmp sgt <4 x i32> %77, %vectorPHI243
  %140 = add nsw <4 x i32> %131, %134
  %141 = add nsw <4 x i32> %132, %135
  %142 = add nsw <4 x i32> %133, %136
  %143 = sext <4 x i1> %137 to <4 x i32>
  %144 = sext <4 x i1> %138 to <4 x i32>
  %145 = sext <4 x i1> %139 to <4 x i32>
  %146 = icmp sgt <4 x i32> %82, %vectorPHI241
  %147 = icmp sgt <4 x i32> %83, %vectorPHI242
  %148 = icmp sgt <4 x i32> %84, %vectorPHI243
  %149 = icmp sgt <4 x i32> %89, %vectorPHI241
  %150 = icmp sgt <4 x i32> %90, %vectorPHI242
  %151 = icmp sgt <4 x i32> %91, %vectorPHI243
  %152 = add nsw <4 x i32> %140, %143
  %153 = add nsw <4 x i32> %141, %144
  %154 = add nsw <4 x i32> %142, %145
  %155 = sext <4 x i1> %146 to <4 x i32>
  %156 = sext <4 x i1> %147 to <4 x i32>
  %157 = sext <4 x i1> %148 to <4 x i32>
  %158 = icmp sgt <4 x i32> %96, %vectorPHI241
  %159 = icmp sgt <4 x i32> %97, %vectorPHI242
  %160 = icmp sgt <4 x i32> %98, %vectorPHI243
  %161 = sext <4 x i1> %149 to <4 x i32>
  %162 = sext <4 x i1> %150 to <4 x i32>
  %163 = sext <4 x i1> %151 to <4 x i32>
  %164 = add nsw <4 x i32> %152, %155
  %165 = add nsw <4 x i32> %153, %156
  %166 = add nsw <4 x i32> %154, %157
  %167 = icmp sgt <4 x i32> %103, %vectorPHI241
  %168 = icmp sgt <4 x i32> %104, %vectorPHI242
  %169 = icmp sgt <4 x i32> %105, %vectorPHI243
  %170 = sext <4 x i1> %158 to <4 x i32>
  %171 = sext <4 x i1> %159 to <4 x i32>
  %172 = sext <4 x i1> %160 to <4 x i32>
  %173 = add nsw <4 x i32> %164, %161
  %174 = add nsw <4 x i32> %165, %162
  %175 = add nsw <4 x i32> %166, %163
  %176 = sext <4 x i1> %167 to <4 x i32>
  %177 = sext <4 x i1> %168 to <4 x i32>
  %178 = sext <4 x i1> %169 to <4 x i32>
  %179 = add nsw <4 x i32> %173, %170
  %180 = add nsw <4 x i32> %174, %171
  %181 = add nsw <4 x i32> %175, %172
  %182 = add nsw <4 x i32> %179, %176
  %183 = add nsw <4 x i32> %180, %177
  %184 = add nsw <4 x i32> %181, %178
  %185 = sub nsw <4 x i32> zeroinitializer, %182
  %186 = sub nsw <4 x i32> zeroinitializer, %183
  %187 = sub nsw <4 x i32> zeroinitializer, %184
  %188 = icmp sgt <4 x i32> %185, <i32 4, i32 4, i32 4, i32 4>
  %189 = icmp sgt <4 x i32> %186, <i32 4, i32 4, i32 4, i32 4>
  %190 = icmp sgt <4 x i32> %187, <i32 4, i32 4, i32 4, i32 4>
  %191 = sext <4 x i1> %188 to <4 x i32>
  %192 = sext <4 x i1> %189 to <4 x i32>
  %193 = sext <4 x i1> %190 to <4 x i32>
  %194 = and <4 x i32> %vectorPHI241, %191
  %195 = and <4 x i32> %vectorPHI242, %192
  %196 = and <4 x i32> %vectorPHI243, %193
  %197 = xor <4 x i32> %191, <i32 -1, i32 -1, i32 -1, i32 -1>
  %198 = xor <4 x i32> %192, <i32 -1, i32 -1, i32 -1, i32 -1>
  %199 = xor <4 x i32> %193, <i32 -1, i32 -1, i32 -1, i32 -1>
  %200 = and <4 x i32> %vectorPHI238, %197
  %201 = and <4 x i32> %vectorPHI239, %198
  %202 = and <4 x i32> %vectorPHI240, %199
  %203 = or <4 x i32> %194, %200
  %204 = or <4 x i32> %195, %201
  %205 = or <4 x i32> %196, %202
  %206 = and <4 x i32> %vectorPHI241, %197
  %207 = and <4 x i32> %vectorPHI242, %198
  %208 = and <4 x i32> %vectorPHI243, %199
  %209 = and <4 x i32> %vectorPHI, %191
  %210 = and <4 x i32> %vectorPHI236, %192
  %211 = and <4 x i32> %vectorPHI237, %193
  %212 = or <4 x i32> %206, %209
  %213 = or <4 x i32> %207, %210
  %214 = or <4 x i32> %208, %211
  %215 = add nsw <4 x i32> %212, %203
  %216 = add nsw <4 x i32> %213, %204
  %217 = add nsw <4 x i32> %214, %205
  %218 = ashr <4 x i32> %215, <i32 1, i32 1, i32 1, i32 1>
  %219 = ashr <4 x i32> %216, <i32 1, i32 1, i32 1, i32 1>
  %220 = ashr <4 x i32> %217, <i32 1, i32 1, i32 1, i32 1>
  %221 = add nsw i32 %iSearch.011, 1
  %exitcond = icmp eq i32 %221, 8
  br i1 %exitcond, label %._crit_edge13, label %106

._crit_edge13:                                    ; preds = %106
  %222 = and <4 x i32> %218, <i32 255, i32 255, i32 255, i32 255>
  %223 = shl <4 x i32> %219, <i32 8, i32 8, i32 8, i32 8>
  %224 = and <4 x i32> %223, <i32 65280, i32 65280, i32 65280, i32 65280>
  %225 = shl <4 x i32> %220, <i32 16, i32 16, i32 16, i32 16>
  %226 = and <4 x i32> %225, <i32 16711680, i32 16711680, i32 16711680, i32 16711680>
  %227 = or <4 x i32> %226, %222
  %228 = or <4 x i32> %227, %224
  %extract244 = extractelement <4 x i32> %228, i32 0
  %extract245 = extractelement <4 x i32> %228, i32 1
  %extract246 = extractelement <4 x i32> %228, i32 2
  %extract247 = extractelement <4 x i32> %228, i32 3
  store i32 %extract244, i32 addrspace(1)* %3, align 4
  store i32 %extract245, i32 addrspace(1)* %4, align 4
  store i32 %extract246, i32 addrspace(1)* %5, align 4
  store i32 %extract247, i32 addrspace(1)* %6, align 4
  %229 = add nsw i32 %x.015, 1
  %exitcond42 = icmp eq i32 %229, %iImageWidth
  br i1 %exitcond42, label %._crit_edge17, label %bb.nph12

._crit_edge17:                                    ; preds = %._crit_edge13, %0
  ret void
}

define void @__Vectorized_.intel_median_scalar(i8 addrspace(1)* nocapture %pSrc, i32 addrspace(1)* nocapture %pDst, i32 %iImageWidth, i32 %iImageHeight) nounwind {
  %temp = insertelement <4 x i32> undef, i32 %iImageWidth, i32 0
  %vector = shufflevector <4 x i32> %temp, <4 x i32> undef, <4 x i32> zeroinitializer
  %1 = alloca [4 x i32], align 4
  %2 = alloca [4 x i32], align 4
  %3 = alloca [4 x i32], align 4
  %4 = alloca [4 x i32], align 4
  %5 = call i32 @get_global_id(i32 0) nounwind
  %broadcast1 = insertelement <4 x i32> undef, i32 %5, i32 0
  %broadcast2 = shufflevector <4 x i32> %broadcast1, <4 x i32> undef, <4 x i32> zeroinitializer
  %6 = icmp sgt i32 %iImageWidth, 0
  br i1 %6, label %bb.nph20, label %._crit_edge21

bb.nph20:                                         ; preds = %0
  %7 = add <4 x i32> %broadcast2, <i32 0, i32 1, i32 2, i32 3>
  %8 = getelementptr inbounds [4 x i32]* %1, i32 0, i32 0
  %9 = getelementptr inbounds [4 x i32]* %2, i32 0, i32 0
  %10 = getelementptr inbounds [4 x i32]* %3, i32 0, i32 0
  %11 = getelementptr inbounds [4 x i32]* %4, i32 0, i32 0
  %12 = getelementptr inbounds [4 x i32]* %1, i32 0, i32 1
  %13 = getelementptr inbounds [4 x i32]* %2, i32 0, i32 1
  %14 = getelementptr inbounds [4 x i32]* %3, i32 0, i32 1
  %15 = getelementptr inbounds [4 x i32]* %4, i32 0, i32 1
  %16 = getelementptr inbounds [4 x i32]* %1, i32 0, i32 2
  %17 = getelementptr inbounds [4 x i32]* %2, i32 0, i32 2
  %18 = getelementptr inbounds [4 x i32]* %3, i32 0, i32 2
  %19 = getelementptr inbounds [4 x i32]* %4, i32 0, i32 2
  %tmp9742 = add <4 x i32> %broadcast2, <i32 1, i32 2, i32 3, i32 4>
  %tmp9843 = mul <4 x i32> %tmp9742, %vector
  %tmp9944 = shl <4 x i32> %tmp9843, <i32 2, i32 2, i32 2, i32 2>
  %tmp10045 = add <4 x i32> %tmp9944, <i32 4, i32 4, i32 4, i32 4>
  %tmp10446 = shl <4 x i32> %7, <i32 2, i32 2, i32 2, i32 2>
  %tmp10547 = add <4 x i32> %tmp10446, <i32 8, i32 8, i32 8, i32 8>
  %tmp10648 = mul <4 x i32> %tmp10547, %vector
  %tmp10749 = add <4 x i32> %tmp10648, <i32 4, i32 4, i32 4, i32 4>
  %tmp11150 = add <4 x i32> %tmp10446, <i32 12, i32 12, i32 12, i32 12>
  %tmp11251 = mul <4 x i32> %tmp11150, %vector
  %tmp11352 = add <4 x i32> %tmp11251, <i32 4, i32 4, i32 4, i32 4>
  %tmp12653 = add <4 x i32> %tmp9944, <i32 -4, i32 -4, i32 -4, i32 -4>
  %tmp13054 = add <4 x i32> %tmp10648, <i32 -4, i32 -4, i32 -4, i32 -4>
  %tmp13455 = add <4 x i32> %tmp11251, <i32 -4, i32 -4, i32 -4, i32 -4>
  %tmp13856 = add <4 x i32> %broadcast2, <i32 2, i32 3, i32 4, i32 5>
  %tmp13957 = mul <4 x i32> %tmp13856, %vector
  br label %bb.nph17

bb.nph17:                                         ; preds = %._crit_edge18, %bb.nph20
  %x.019 = phi i32 [ 0, %bb.nph20 ], [ %162, %._crit_edge18 ]
  %temp69 = insertelement <4 x i32> undef, i32 %x.019, i32 0
  %vector70 = shufflevector <4 x i32> %temp69, <4 x i32> undef, <4 x i32> zeroinitializer
  %tmp = shl i32 %x.019, 2
  %temp58 = insertelement <4 x i32> undef, i32 %tmp, i32 0
  %vector59 = shufflevector <4 x i32> %temp58, <4 x i32> undef, <4 x i32> zeroinitializer
  %tmp10160 = add <4 x i32> %tmp10045, %vector59
  %tmp10861 = add <4 x i32> %tmp10749, %vector59
  %tmp11462 = add <4 x i32> %tmp11352, %vector59
  %tmp11763 = add <4 x i32> %tmp9944, %vector59
  %tmp12064 = add <4 x i32> %tmp10648, %vector59
  %tmp12365 = add <4 x i32> %tmp11251, %vector59
  %tmp12766 = add <4 x i32> %tmp12653, %vector59
  %tmp13167 = add <4 x i32> %tmp13054, %vector59
  %tmp13568 = add <4 x i32> %tmp13455, %vector59
  %tmp14071 = add <4 x i32> %tmp13957, %vector70
  %extract = extractelement <4 x i32> %tmp14071, i32 0
  %extract72 = extractelement <4 x i32> %tmp14071, i32 1
  %extract73 = extractelement <4 x i32> %tmp14071, i32 2
  %extract74 = extractelement <4 x i32> %tmp14071, i32 3
  %20 = getelementptr i32 addrspace(1)* %pDst, i32 %extract
  %21 = getelementptr i32 addrspace(1)* %pDst, i32 %extract72
  %22 = getelementptr i32 addrspace(1)* %pDst, i32 %extract73
  %23 = getelementptr i32 addrspace(1)* %pDst, i32 %extract74
  br label %bb.nph12

bb.nph12:                                         ; preds = %._crit_edge13, %bb.nph17
  %ch.016 = phi i32 [ 0, %bb.nph17 ], [ %142, %._crit_edge13 ]
  %temp75 = insertelement <4 x i32> undef, i32 %ch.016, i32 0
  %vector76 = shufflevector <4 x i32> %temp75, <4 x i32> undef, <4 x i32> zeroinitializer
  %24 = getelementptr [4 x i32]* %1, i32 0, i32 %ch.016
  %25 = getelementptr [4 x i32]* %2, i32 0, i32 %ch.016
  %26 = getelementptr [4 x i32]* %3, i32 0, i32 %ch.016
  %27 = getelementptr [4 x i32]* %4, i32 0, i32 %ch.016
  %tmp13677 = add <4 x i32> %tmp13568, %vector76
  %extract86 = extractelement <4 x i32> %tmp13677, i32 0
  %extract87 = extractelement <4 x i32> %tmp13677, i32 1
  %extract88 = extractelement <4 x i32> %tmp13677, i32 2
  %extract89 = extractelement <4 x i32> %tmp13677, i32 3
  %tmp13278 = add <4 x i32> %tmp13167, %vector76
  %extract90 = extractelement <4 x i32> %tmp13278, i32 0
  %extract91 = extractelement <4 x i32> %tmp13278, i32 1
  %extract92 = extractelement <4 x i32> %tmp13278, i32 2
  %extract93 = extractelement <4 x i32> %tmp13278, i32 3
  %tmp12879 = add <4 x i32> %tmp12766, %vector76
  %extract94 = extractelement <4 x i32> %tmp12879, i32 0
  %extract95 = extractelement <4 x i32> %tmp12879, i32 1
  %extract96 = extractelement <4 x i32> %tmp12879, i32 2
  %extract97 = extractelement <4 x i32> %tmp12879, i32 3
  %tmp12480 = add <4 x i32> %tmp12365, %vector76
  %extract98 = extractelement <4 x i32> %tmp12480, i32 0
  %extract99 = extractelement <4 x i32> %tmp12480, i32 1
  %extract100 = extractelement <4 x i32> %tmp12480, i32 2
  %extract101 = extractelement <4 x i32> %tmp12480, i32 3
  %tmp12181 = add <4 x i32> %tmp12064, %vector76
  %extract102 = extractelement <4 x i32> %tmp12181, i32 0
  %extract103 = extractelement <4 x i32> %tmp12181, i32 1
  %extract104 = extractelement <4 x i32> %tmp12181, i32 2
  %extract105 = extractelement <4 x i32> %tmp12181, i32 3
  %tmp11882 = add <4 x i32> %tmp11763, %vector76
  %extract106 = extractelement <4 x i32> %tmp11882, i32 0
  %extract107 = extractelement <4 x i32> %tmp11882, i32 1
  %extract108 = extractelement <4 x i32> %tmp11882, i32 2
  %extract109 = extractelement <4 x i32> %tmp11882, i32 3
  %tmp11583 = add <4 x i32> %tmp11462, %vector76
  %extract110 = extractelement <4 x i32> %tmp11583, i32 0
  %extract111 = extractelement <4 x i32> %tmp11583, i32 1
  %extract112 = extractelement <4 x i32> %tmp11583, i32 2
  %extract113 = extractelement <4 x i32> %tmp11583, i32 3
  %tmp10984 = add <4 x i32> %tmp10861, %vector76
  %extract114 = extractelement <4 x i32> %tmp10984, i32 0
  %extract115 = extractelement <4 x i32> %tmp10984, i32 1
  %extract116 = extractelement <4 x i32> %tmp10984, i32 2
  %extract117 = extractelement <4 x i32> %tmp10984, i32 3
  %tmp10285 = add <4 x i32> %tmp10160, %vector76
  %extract118 = extractelement <4 x i32> %tmp10285, i32 0
  %extract119 = extractelement <4 x i32> %tmp10285, i32 1
  %extract120 = extractelement <4 x i32> %tmp10285, i32 2
  %extract121 = extractelement <4 x i32> %tmp10285, i32 3
  %28 = getelementptr i8 addrspace(1)* %pSrc, i32 %extract86
  %29 = getelementptr i8 addrspace(1)* %pSrc, i32 %extract87
  %30 = getelementptr i8 addrspace(1)* %pSrc, i32 %extract88
  %31 = getelementptr i8 addrspace(1)* %pSrc, i32 %extract89
  %32 = getelementptr i8 addrspace(1)* %pSrc, i32 %extract90
  %33 = getelementptr i8 addrspace(1)* %pSrc, i32 %extract91
  %34 = getelementptr i8 addrspace(1)* %pSrc, i32 %extract92
  %35 = getelementptr i8 addrspace(1)* %pSrc, i32 %extract93
  %36 = getelementptr i8 addrspace(1)* %pSrc, i32 %extract94
  %37 = getelementptr i8 addrspace(1)* %pSrc, i32 %extract95
  %38 = getelementptr i8 addrspace(1)* %pSrc, i32 %extract96
  %39 = getelementptr i8 addrspace(1)* %pSrc, i32 %extract97
  %40 = getelementptr i8 addrspace(1)* %pSrc, i32 %extract98
  %41 = getelementptr i8 addrspace(1)* %pSrc, i32 %extract99
  %42 = getelementptr i8 addrspace(1)* %pSrc, i32 %extract100
  %43 = getelementptr i8 addrspace(1)* %pSrc, i32 %extract101
  %44 = getelementptr i8 addrspace(1)* %pSrc, i32 %extract102
  %45 = getelementptr i8 addrspace(1)* %pSrc, i32 %extract103
  %46 = getelementptr i8 addrspace(1)* %pSrc, i32 %extract104
  %47 = getelementptr i8 addrspace(1)* %pSrc, i32 %extract105
  %48 = getelementptr i8 addrspace(1)* %pSrc, i32 %extract106
  %49 = getelementptr i8 addrspace(1)* %pSrc, i32 %extract107
  %50 = getelementptr i8 addrspace(1)* %pSrc, i32 %extract108
  %51 = getelementptr i8 addrspace(1)* %pSrc, i32 %extract109
  %52 = getelementptr i8 addrspace(1)* %pSrc, i32 %extract110
  %53 = getelementptr i8 addrspace(1)* %pSrc, i32 %extract111
  %54 = getelementptr i8 addrspace(1)* %pSrc, i32 %extract112
  %55 = getelementptr i8 addrspace(1)* %pSrc, i32 %extract113
  %56 = getelementptr i8 addrspace(1)* %pSrc, i32 %extract114
  %57 = getelementptr i8 addrspace(1)* %pSrc, i32 %extract115
  %58 = getelementptr i8 addrspace(1)* %pSrc, i32 %extract116
  %59 = getelementptr i8 addrspace(1)* %pSrc, i32 %extract117
  %60 = getelementptr i8 addrspace(1)* %pSrc, i32 %extract118
  %61 = getelementptr i8 addrspace(1)* %pSrc, i32 %extract119
  %62 = getelementptr i8 addrspace(1)* %pSrc, i32 %extract120
  %63 = getelementptr i8 addrspace(1)* %pSrc, i32 %extract121
  %64 = load i8 addrspace(1)* %36, align 1
  %65 = load i8 addrspace(1)* %37, align 1
  %66 = load i8 addrspace(1)* %38, align 1
  %67 = load i8 addrspace(1)* %39, align 1
  %temp.vect = insertelement <4 x i8> undef, i8 %64, i32 0
  %temp.vect122 = insertelement <4 x i8> %temp.vect, i8 %65, i32 1
  %temp.vect123 = insertelement <4 x i8> %temp.vect122, i8 %66, i32 2
  %temp.vect124 = insertelement <4 x i8> %temp.vect123, i8 %67, i32 3
  %68 = zext <4 x i8> %temp.vect124 to <4 x i32>
  %69 = load i8 addrspace(1)* %48, align 1
  %70 = load i8 addrspace(1)* %49, align 1
  %71 = load i8 addrspace(1)* %50, align 1
  %72 = load i8 addrspace(1)* %51, align 1
  %temp.vect125 = insertelement <4 x i8> undef, i8 %69, i32 0
  %temp.vect126 = insertelement <4 x i8> %temp.vect125, i8 %70, i32 1
  %temp.vect127 = insertelement <4 x i8> %temp.vect126, i8 %71, i32 2
  %temp.vect128 = insertelement <4 x i8> %temp.vect127, i8 %72, i32 3
  %73 = zext <4 x i8> %temp.vect128 to <4 x i32>
  %74 = load i8 addrspace(1)* %60, align 1
  %75 = load i8 addrspace(1)* %61, align 1
  %76 = load i8 addrspace(1)* %62, align 1
  %77 = load i8 addrspace(1)* %63, align 1
  %temp.vect129 = insertelement <4 x i8> undef, i8 %74, i32 0
  %temp.vect130 = insertelement <4 x i8> %temp.vect129, i8 %75, i32 1
  %temp.vect131 = insertelement <4 x i8> %temp.vect130, i8 %76, i32 2
  %temp.vect132 = insertelement <4 x i8> %temp.vect131, i8 %77, i32 3
  %78 = zext <4 x i8> %temp.vect132 to <4 x i32>
  %79 = load i8 addrspace(1)* %32, align 1
  %80 = load i8 addrspace(1)* %33, align 1
  %81 = load i8 addrspace(1)* %34, align 1
  %82 = load i8 addrspace(1)* %35, align 1
  %temp.vect133 = insertelement <4 x i8> undef, i8 %79, i32 0
  %temp.vect134 = insertelement <4 x i8> %temp.vect133, i8 %80, i32 1
  %temp.vect135 = insertelement <4 x i8> %temp.vect134, i8 %81, i32 2
  %temp.vect136 = insertelement <4 x i8> %temp.vect135, i8 %82, i32 3
  %83 = zext <4 x i8> %temp.vect136 to <4 x i32>
  %84 = load i8 addrspace(1)* %44, align 1
  %85 = load i8 addrspace(1)* %45, align 1
  %86 = load i8 addrspace(1)* %46, align 1
  %87 = load i8 addrspace(1)* %47, align 1
  %temp.vect137 = insertelement <4 x i8> undef, i8 %84, i32 0
  %temp.vect138 = insertelement <4 x i8> %temp.vect137, i8 %85, i32 1
  %temp.vect139 = insertelement <4 x i8> %temp.vect138, i8 %86, i32 2
  %temp.vect140 = insertelement <4 x i8> %temp.vect139, i8 %87, i32 3
  %88 = zext <4 x i8> %temp.vect140 to <4 x i32>
  %89 = load i8 addrspace(1)* %56, align 1
  %90 = load i8 addrspace(1)* %57, align 1
  %91 = load i8 addrspace(1)* %58, align 1
  %92 = load i8 addrspace(1)* %59, align 1
  %temp.vect141 = insertelement <4 x i8> undef, i8 %89, i32 0
  %temp.vect142 = insertelement <4 x i8> %temp.vect141, i8 %90, i32 1
  %temp.vect143 = insertelement <4 x i8> %temp.vect142, i8 %91, i32 2
  %temp.vect144 = insertelement <4 x i8> %temp.vect143, i8 %92, i32 3
  %93 = zext <4 x i8> %temp.vect144 to <4 x i32>
  %94 = load i8 addrspace(1)* %28, align 1
  %95 = load i8 addrspace(1)* %29, align 1
  %96 = load i8 addrspace(1)* %30, align 1
  %97 = load i8 addrspace(1)* %31, align 1
  %temp.vect145 = insertelement <4 x i8> undef, i8 %94, i32 0
  %temp.vect146 = insertelement <4 x i8> %temp.vect145, i8 %95, i32 1
  %temp.vect147 = insertelement <4 x i8> %temp.vect146, i8 %96, i32 2
  %temp.vect148 = insertelement <4 x i8> %temp.vect147, i8 %97, i32 3
  %98 = zext <4 x i8> %temp.vect148 to <4 x i32>
  %99 = load i8 addrspace(1)* %40, align 1
  %100 = load i8 addrspace(1)* %41, align 1
  %101 = load i8 addrspace(1)* %42, align 1
  %102 = load i8 addrspace(1)* %43, align 1
  %temp.vect149 = insertelement <4 x i8> undef, i8 %99, i32 0
  %temp.vect150 = insertelement <4 x i8> %temp.vect149, i8 %100, i32 1
  %temp.vect151 = insertelement <4 x i8> %temp.vect150, i8 %101, i32 2
  %temp.vect152 = insertelement <4 x i8> %temp.vect151, i8 %102, i32 3
  %103 = zext <4 x i8> %temp.vect152 to <4 x i32>
  %104 = load i8 addrspace(1)* %52, align 1
  %105 = load i8 addrspace(1)* %53, align 1
  %106 = load i8 addrspace(1)* %54, align 1
  %107 = load i8 addrspace(1)* %55, align 1
  %temp.vect153 = insertelement <4 x i8> undef, i8 %104, i32 0
  %temp.vect154 = insertelement <4 x i8> %temp.vect153, i8 %105, i32 1
  %temp.vect155 = insertelement <4 x i8> %temp.vect154, i8 %106, i32 2
  %temp.vect156 = insertelement <4 x i8> %temp.vect155, i8 %107, i32 3
  %108 = zext <4 x i8> %temp.vect156 to <4 x i32>
  br label %109

; <label>:109                                     ; preds = %109, %bb.nph12
  %vectorPHI = phi <4 x i32> [ <i32 128, i32 128, i32 128, i32 128>, %bb.nph12 ], [ %140, %109 ]
  %vectorPHI157 = phi <4 x i32> [ zeroinitializer, %bb.nph12 ], [ %132, %109 ]
  %vectorPHI158 = phi <4 x i32> [ <i32 255, i32 255, i32 255, i32 255>, %bb.nph12 ], [ %138, %109 ]
  %iSearch.011 = phi i32 [ 0, %bb.nph12 ], [ %141, %109 ]
  %110 = call <4 x i32> @llvm.x86.sse2.pcmpgt.d(<4 x i32> %73, <4 x i32> %vectorPHI)
  %111 = call <4 x i32> @llvm.x86.sse2.pcmpgt.d(<4 x i32> %68, <4 x i32> %vectorPHI)
  %112 = call <4 x i32> @llvm.x86.sse2.pcmpgt.d(<4 x i32> %78, <4 x i32> %vectorPHI)
  %msb = and <4 x i32> %110, <i32 1, i32 1, i32 1, i32 1>
  %msb194 = and <4 x i32> %111, <i32 1, i32 1, i32 1, i32 1>
  %113 = add nsw <4 x i32> %msb194, %msb
  %msb196 = and <4 x i32> %112, <i32 1, i32 1, i32 1, i32 1>
  %114 = call <4 x i32> @llvm.x86.sse2.pcmpgt.d(<4 x i32> %83, <4 x i32> %vectorPHI)
  %115 = add nsw <4 x i32> %113, %msb196
  %msb198 = and <4 x i32> %114, <i32 1, i32 1, i32 1, i32 1>
  %116 = call <4 x i32> @llvm.x86.sse2.pcmpgt.d(<4 x i32> %88, <4 x i32> %vectorPHI)
  %msb200 = and <4 x i32> %116, <i32 1, i32 1, i32 1, i32 1>
  %117 = call <4 x i32> @llvm.x86.sse2.pcmpgt.d(<4 x i32> %93, <4 x i32> %vectorPHI)
  %118 = add nsw <4 x i32> %msb198, %115
  %119 = call <4 x i32> @llvm.x86.sse2.pcmpgt.d(<4 x i32> %98, <4 x i32> %vectorPHI)
  %msb202 = and <4 x i32> %117, <i32 1, i32 1, i32 1, i32 1>
  %120 = add nsw <4 x i32> %118, %msb200
  %121 = call <4 x i32> @llvm.x86.sse2.pcmpgt.d(<4 x i32> %103, <4 x i32> %vectorPHI)
  %msb204 = and <4 x i32> %119, <i32 1, i32 1, i32 1, i32 1>
  %122 = add nsw <4 x i32> %120, %msb202
  %123 = add nsw <4 x i32> %msb204, %122
  %124 = call <4 x i32> @llvm.x86.sse2.pcmpgt.d(<4 x i32> %108, <4 x i32> %vectorPHI)
  %msb206 = and <4 x i32> %121, <i32 1, i32 1, i32 1, i32 1>
  %125 = add nsw <4 x i32> %123, %msb206
  %msb208 = and <4 x i32> %124, <i32 1, i32 1, i32 1, i32 1>
  %126 = add nsw <4 x i32> %125, %msb208
  %127 = call <4 x i32> @llvm.x86.sse2.pcmpgt.d(<4 x i32> %126, <4 x i32> <i32 4, i32 4, i32 4, i32 4>)
  %128 = bitcast <4 x i32> %vectorPHI to <4 x float>
  %129 = bitcast <4 x i32> %vectorPHI157 to <4 x float>
  %130 = bitcast <4 x i32> %127 to <4 x float>
  %131 = call <4 x float> @llvm.x86.sse41.blendvps(<4 x float> %129, <4 x float> %128, <4 x float> %130)
  %132 = bitcast <4 x float> %131 to <4 x i32>
  %133 = call <4 x i32> @llvm.x86.sse2.pcmpgt.d(<4 x i32> <i32 5, i32 5, i32 5, i32 5>, <4 x i32> %126)
  %134 = bitcast <4 x i32> %vectorPHI to <4 x float>
  %135 = bitcast <4 x i32> %vectorPHI158 to <4 x float>
  %136 = bitcast <4 x i32> %133 to <4 x float>
  %137 = call <4 x float> @llvm.x86.sse41.blendvps(<4 x float> %135, <4 x float> %134, <4 x float> %136)
  %138 = bitcast <4 x float> %137 to <4 x i32>
  %139 = add nsw <4 x i32> %138, %132
  %140 = ashr <4 x i32> %139, <i32 1, i32 1, i32 1, i32 1>
  %141 = add nsw i32 %iSearch.011, 1
  %exitcond = icmp eq i32 %141, 8
  br i1 %exitcond, label %._crit_edge13, label %109

._crit_edge13:                                    ; preds = %109
  %extract162 = extractelement <4 x i32> %140, i32 3
  %extract161 = extractelement <4 x i32> %140, i32 2
  %extract160 = extractelement <4 x i32> %140, i32 1
  %extract159 = extractelement <4 x i32> %140, i32 0
  store i32 %extract159, i32* %24, align 4
  store i32 %extract160, i32* %25, align 4
  store i32 %extract161, i32* %26, align 4
  store i32 %extract162, i32* %27, align 4
  %142 = add nsw i32 %ch.016, 1
  %exitcond52 = icmp eq i32 %142, 3
  br i1 %exitcond52, label %._crit_edge18, label %bb.nph12

._crit_edge18:                                    ; preds = %._crit_edge13
  %143 = load i32* %8, align 4
  %144 = load i32* %9, align 4
  %145 = load i32* %10, align 4
  %146 = load i32* %11, align 4
  %temp.vect163 = insertelement <4 x i32> undef, i32 %143, i32 0
  %temp.vect164 = insertelement <4 x i32> %temp.vect163, i32 %144, i32 1
  %temp.vect165 = insertelement <4 x i32> %temp.vect164, i32 %145, i32 2
  %temp.vect166 = insertelement <4 x i32> %temp.vect165, i32 %146, i32 3
  %147 = and <4 x i32> %temp.vect166, <i32 255, i32 255, i32 255, i32 255>
  %148 = load i32* %12, align 4
  %149 = load i32* %13, align 4
  %150 = load i32* %14, align 4
  %151 = load i32* %15, align 4
  %temp.vect167 = insertelement <4 x i32> undef, i32 %148, i32 0
  %temp.vect168 = insertelement <4 x i32> %temp.vect167, i32 %149, i32 1
  %temp.vect169 = insertelement <4 x i32> %temp.vect168, i32 %150, i32 2
  %temp.vect170 = insertelement <4 x i32> %temp.vect169, i32 %151, i32 3
  %152 = shl <4 x i32> %temp.vect170, <i32 8, i32 8, i32 8, i32 8>
  %153 = and <4 x i32> %152, <i32 65280, i32 65280, i32 65280, i32 65280>
  %154 = load i32* %16, align 4
  %155 = load i32* %17, align 4
  %156 = load i32* %18, align 4
  %157 = load i32* %19, align 4
  %temp.vect171 = insertelement <4 x i32> undef, i32 %154, i32 0
  %temp.vect172 = insertelement <4 x i32> %temp.vect171, i32 %155, i32 1
  %temp.vect173 = insertelement <4 x i32> %temp.vect172, i32 %156, i32 2
  %temp.vect174 = insertelement <4 x i32> %temp.vect173, i32 %157, i32 3
  %158 = shl <4 x i32> %temp.vect174, <i32 16, i32 16, i32 16, i32 16>
  %159 = and <4 x i32> %158, <i32 16711680, i32 16711680, i32 16711680, i32 16711680>
  %160 = or <4 x i32> %153, %147
  %161 = or <4 x i32> %160, %159
  %extract175 = extractelement <4 x i32> %161, i32 0
  %extract176 = extractelement <4 x i32> %161, i32 1
  %extract177 = extractelement <4 x i32> %161, i32 2
  %extract178 = extractelement <4 x i32> %161, i32 3
  store i32 %extract175, i32 addrspace(1)* %20, align 4
  store i32 %extract176, i32 addrspace(1)* %21, align 4
  store i32 %extract177, i32 addrspace(1)* %22, align 4
  store i32 %extract178, i32 addrspace(1)* %23, align 4
  %162 = add nsw i32 %x.019, 1
  %exitcond96 = icmp eq i32 %162, %iImageWidth
  br i1 %exitcond96, label %._crit_edge21, label %bb.nph17

._crit_edge21:                                    ; preds = %._crit_edge18, %0
  ret void
}

define <8 x i32> @local.avx256.pcmpeq.d(<8 x i32>, <8 x i32>) {
entry:
  %ALow = shufflevector <8 x i32> %0, <8 x i32> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %BLow = shufflevector <8 x i32> %1, <8 x i32> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %AHigh = shufflevector <8 x i32> %0, <8 x i32> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
  %BHigh = shufflevector <8 x i32> %1, <8 x i32> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
  %callLow = call <4 x i32> @llvm.x86.sse2.pcmpeq.d(<4 x i32> %ALow, <4 x i32> %BLow)
  %callLow1 = call <4 x i32> @llvm.x86.sse2.pcmpeq.d(<4 x i32> %AHigh, <4 x i32> %BHigh)
  %join = shufflevector <4 x i32> %callLow, <4 x i32> %callLow1, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  ret <8 x i32> %join
}

declare <4 x i32> @llvm.x86.sse2.pcmpeq.d(<4 x i32>, <4 x i32>) nounwind readnone

define <8 x i32> @local.avx256.pcmpgt.d(<8 x i32>, <8 x i32>) {
entry:
  %ALow = shufflevector <8 x i32> %0, <8 x i32> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %BLow = shufflevector <8 x i32> %1, <8 x i32> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %AHigh = shufflevector <8 x i32> %0, <8 x i32> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
  %BHigh = shufflevector <8 x i32> %1, <8 x i32> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
  %callLow = call <4 x i32> @llvm.x86.sse2.pcmpgt.d(<4 x i32> %ALow, <4 x i32> %BLow)
  %callLow1 = call <4 x i32> @llvm.x86.sse2.pcmpgt.d(<4 x i32> %AHigh, <4 x i32> %BHigh)
  %join = shufflevector <4 x i32> %callLow, <4 x i32> %callLow1, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  ret <8 x i32> %join
}

declare <4 x i32> @llvm.x86.sse2.pcmpgt.d(<4 x i32>, <4 x i32>) nounwind readnone

declare i1 @allOne(i1)

declare i1 @allZero(i1)

define <8 x i32> @local.avx256.pcmpeq.d1(<8 x i32>, <8 x i32>) {
entry:
  %ALow = shufflevector <8 x i32> %0, <8 x i32> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %BLow = shufflevector <8 x i32> %1, <8 x i32> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %AHigh = shufflevector <8 x i32> %0, <8 x i32> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
  %BHigh = shufflevector <8 x i32> %1, <8 x i32> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
  %callLow = call <4 x i32> @llvm.x86.sse2.pcmpeq.d(<4 x i32> %ALow, <4 x i32> %BLow)
  %callLow1 = call <4 x i32> @llvm.x86.sse2.pcmpeq.d(<4 x i32> %AHigh, <4 x i32> %BHigh)
  %join = shufflevector <4 x i32> %callLow, <4 x i32> %callLow1, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  ret <8 x i32> %join
}

define <8 x i32> @local.avx256.pcmpgt.d2(<8 x i32>, <8 x i32>) {
entry:
  %ALow = shufflevector <8 x i32> %0, <8 x i32> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %BLow = shufflevector <8 x i32> %1, <8 x i32> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %AHigh = shufflevector <8 x i32> %0, <8 x i32> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
  %BHigh = shufflevector <8 x i32> %1, <8 x i32> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
  %callLow = call <4 x i32> @llvm.x86.sse2.pcmpgt.d(<4 x i32> %ALow, <4 x i32> %BLow)
  %callLow1 = call <4 x i32> @llvm.x86.sse2.pcmpgt.d(<4 x i32> %AHigh, <4 x i32> %BHigh)
  %join = shufflevector <4 x i32> %callLow, <4 x i32> %callLow1, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  ret <8 x i32> %join
}

declare <4 x float> @llvm.x86.sse41.blendvps(<4 x float>, <4 x float>, <4 x float>) nounwind readnone

!opencl.kernels = !{!0, !2}

!0 = metadata !{void (<4 x i8> addrspace(1)*, i32 addrspace(1)*, i32, i32)* @intel_median, metadata !1, metadata !1, metadata !"", metadata !"uchar4 __attribute__((address_space(1))) *, unsigned int __attribute__((address_space(1))) *, int, int", metadata !"opencl_intel_median_locals_anchor"}
!1 = metadata !{i32 0, i32 0, i32 0}
!2 = metadata !{void (i8 addrspace(1)*, i32 addrspace(1)*, i32, i32)* @intel_median_scalar, metadata !1, metadata !1, metadata !"", metadata !"uchar __attribute__((address_space(1))) *, unsigned int __attribute__((address_space(1))) *, int, int", metadata !"opencl_intel_median_scalar_locals_anchor"}
