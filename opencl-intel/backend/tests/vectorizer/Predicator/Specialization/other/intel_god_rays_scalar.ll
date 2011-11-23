; RUN: llvm-as %s -o %t.bc
; RUN: opt -phicanon -loopsimplify -predicate -specialize -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'intel_god_rays_scalar.ll'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"

; This module was already processed by -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -mergereturn -loopsimplify passes

; CHECK: @intel_god_rays_scalar
; CHECK-NOT: %{{[a-z\.0-9]}} %{{[a-z\.0-9]}} %{{[a-z\.0-9]}}
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
; CHECK: footer{{[0-9]*}}:                                        ; preds = %header{{[0-9]*}}
; CHECK: header{{[0-9]*}}:
; CHECK:   br i1 %jumpover{{[0-9]*}}, label %footer{{[0-9]*}}
; CHECK: footer{{[0-9]*}}:                                        ; preds = %header{{[0-9]*}}
; CHECK: footer{{[0-9]*}}:                                        ; preds = %header{{[0-9]*}}
; CHECK: ret

define void @evaluateRayScalar(float addrspace(1)* nocapture %inputImage, float addrspace(1)* nocapture %output, i32 %in_RayNum, <2 x i32> %imgSize, i32 %blend) nounwind {
; <label>:0
  %1 = extractelement <2 x i32> %imgSize, i32 0
  %2 = sdiv i32 %1, 2
  %3 = extractelement <2 x i32> %imgSize, i32 1
  %4 = sdiv i32 %3, 2
  %5 = mul nsw i32 %in_RayNum, 15
  %6 = add nsw i32 %5, 15
  %7 = add nsw i32 %1, -1
  %8 = add nsw i32 %3, -1
  %9 = icmp slt i32 %3, -1
  %.not = icmp sgt i32 %1, -2
  %10 = icmp sgt i32 %4, %8
  %or.cond23 = or i1 %9, %10
  %11 = sub nsw i32 0, %4
  %12 = sub nsw i32 %4, %8
  %13 = select i1 %9, i32 %11, i32 %12
  %14 = shl i32 %13, 1
  %15 = add i32 %14, -2
  %16 = icmp eq i32 %blend, 1
  %17 = icmp slt i32 %1, -1
  %18 = icmp sgt i32 %2, %7
  %or.cond26 = or i1 %17, %18
  %19 = sub nsw i32 0, %2
  %20 = sub nsw i32 %2, %7
  %21 = select i1 %17, i32 %19, i32 %20
  %22 = shl i32 %21, 1
  %23 = add i32 %22, -2
  %.not9 = icmp sgt i32 %3, -2
  %24 = add nsw i32 %1, -2
  %.not11 = icmp sle i32 %2, %7
  %25 = add nsw i32 %3, -2
  %.not14 = icmp sle i32 %4, %8
  %26 = add i32 %7, %3
  %tmp44 = shl i32 %1, 1
  %tmp45 = add i32 %3, %tmp44
  %tmp46 = add i32 %tmp45, -3
  %tmp48 = sub i32 %tmp46, %5
  %tmp50 = shl i32 %3, 1
  %tmp51 = add i32 %tmp44, %tmp50
  %tmp52 = add i32 %tmp51, -4
  %tmp53 = sub i32 %tmp52, %5
  %tmp55 = add i32 %5, 3
  %tmp56 = sub i32 %tmp55, %3
  %tmp57 = sub i32 %tmp56, %tmp44
  %tmp59 = add i32 %5, 2
  %tmp60 = sub i32 %tmp59, %1
  %tmp61 = sub i32 %tmp60, %3
  %tmp63 = add i32 %5, 1
  %tmp64 = sub i32 %tmp63, %1
  %tmp66 = sub i32 %5, %1
  %tmp69 = add i32 %5, -1
  br label %27

; <label>:27                                      ; preds = %._crit_edge58, %0
  %indvar = phi i32 [ %indvar.next, %._crit_edge58 ], [ 0, %0 ]
  %28 = phi i32 [ %399, %._crit_edge58 ], [ %5, %0 ]
  %tmp111 = sub i32 %tmp48, %indvar
  %tmp103 = sub i32 %tmp53, %indvar
  %tmp107 = add i32 %tmp57, %indvar
  %tmp96 = add i32 %tmp61, %indvar
  %tmp92 = add i32 %tmp64, %indvar
  %tmp89 = add i32 %tmp66, %indvar
  %i.0 = add i32 %5, %indvar
  %tmp86 = add i32 %tmp69, %indvar
  %29 = icmp slt i32 %i.0, %6
  br i1 %29, label %30, label %400

; <label>:30                                      ; preds = %27
  %31 = icmp slt i32 %i.0, %7
  br i1 %31, label %32, label %37

; <label>:32                                      ; preds = %30
  br i1 %9, label %33, label %._crit_edge115

; <label>:33                                      ; preds = %32
  %34 = icmp eq i32 %i.0, 0
  %or.cond8 = and i1 %.not, %34
  br i1 %or.cond8, label %._crit_edge115, label %400

._crit_edge115:                                   ; preds = %32, %33
  %35 = icmp eq i32 %i.0, 0
  br i1 %35, label %36, label %62

; <label>:36                                      ; preds = %._crit_edge115
  br label %62

; <label>:37                                      ; preds = %30
  %38 = sub nsw i32 %28, %7
  %39 = icmp slt i32 %tmp92, %8
  br i1 %39, label %40, label %45

; <label>:40                                      ; preds = %37
  br i1 %18, label %41, label %._crit_edge114

; <label>:41                                      ; preds = %40
  %42 = icmp eq i32 %i.0, %7
  %or.cond10 = and i1 %.not9, %42
  br i1 %or.cond10, label %._crit_edge114, label %400

._crit_edge114:                                   ; preds = %40, %41
  %43 = icmp eq i32 %i.0, %7
  %. = select i1 %43, i32 %24, i32 %7
  br i1 %43, label %44, label %62

; <label>:44                                      ; preds = %._crit_edge114
  br label %62

; <label>:45                                      ; preds = %37
  %46 = sub nsw i32 %38, %8
  %47 = icmp slt i32 %tmp96, %7
  br i1 %47, label %48, label %54

; <label>:48                                      ; preds = %45
  br i1 %10, label %49, label %._crit_edge113

; <label>:49                                      ; preds = %48
  %50 = icmp eq i32 %tmp92, %8
  %or.cond12 = and i1 %.not11, %50
  br i1 %or.cond12, label %._crit_edge113, label %400

._crit_edge113:                                   ; preds = %48, %49
  %51 = sub i32 %1, %46
  %52 = icmp eq i32 %tmp92, %8
  %.13 = select i1 %52, i32 %25, i32 %8
  br i1 %52, label %53, label %62

; <label>:53                                      ; preds = %._crit_edge113
  br label %62

; <label>:54                                      ; preds = %45
  %55 = icmp slt i32 %tmp107, %8
  br i1 %55, label %56, label %400

; <label>:56                                      ; preds = %54
  br i1 %17, label %57, label %._crit_edge112

; <label>:57                                      ; preds = %56
  %58 = icmp eq i32 %tmp96, %7
  %or.cond15 = and i1 %.not14, %58
  br i1 %or.cond15, label %._crit_edge112, label %400

._crit_edge112:                                   ; preds = %56, %57
  %59 = sub i32 %26, %46
  %60 = icmp eq i32 %tmp96, %7
  br i1 %60, label %61, label %62

; <label>:61                                      ; preds = %._crit_edge112
  br label %62

; <label>:62                                      ; preds = %61, %._crit_edge112, %53, %._crit_edge113, %44, %._crit_edge114, %36, %._crit_edge115
  %y_dst_s.0 = phi i32 [ 1, %36 ], [ 0, %._crit_edge115 ], [ 0, %44 ], [ %tmp89, %._crit_edge114 ], [ %.13, %53 ], [ %.13, %._crit_edge113 ], [ %8, %61 ], [ %59, %._crit_edge112 ]
  %x_dst_s.0 = phi i32 [ 0, %36 ], [ %tmp86, %._crit_edge115 ], [ %., %44 ], [ %., %._crit_edge114 ], [ %7, %53 ], [ %51, %._crit_edge113 ], [ 1, %61 ], [ 0, %._crit_edge112 ]
  %y_dst.0 = phi i32 [ 0, %36 ], [ 0, %._crit_edge115 ], [ %38, %44 ], [ %38, %._crit_edge114 ], [ %8, %53 ], [ %8, %._crit_edge113 ], [ %tmp103, %61 ], [ %tmp103, %._crit_edge112 ]
  %x_dst.0 = phi i32 [ %28, %36 ], [ %28, %._crit_edge115 ], [ %7, %44 ], [ %7, %._crit_edge114 ], [ %tmp111, %53 ], [ %tmp111, %._crit_edge113 ], [ 0, %61 ], [ 0, %._crit_edge112 ]
  %63 = sub nsw i32 %x_dst.0, %2
  %64 = sub nsw i32 %y_dst.0, %4
  %65 = sub nsw i32 %x_dst_s.0, %2
  %66 = sub nsw i32 %y_dst_s.0, %4
  %67 = icmp sgt i32 %63, 0
  %68 = zext i1 %67 to i32
  %69 = icmp sgt i32 %64, 0
  %70 = zext i1 %69 to i32
  %71 = icmp sgt i32 %65, 0
  %72 = zext i1 %71 to i32
  %73 = icmp sgt i32 %66, 0
  %74 = zext i1 %73 to i32
  %75 = sub nsw i32 0, %63
  %76 = icmp slt i32 %63, 0
  %.16 = select i1 %76, i32 %75, i32 %63
  %xstep.0 = select i1 %76, i32 -1, i32 %68
  %77 = sub nsw i32 0, %64
  %78 = icmp slt i32 %64, 0
  %.17 = select i1 %78, i32 %77, i32 %64
  %ystep.0 = select i1 %78, i32 -1, i32 %70
  %79 = sub nsw i32 0, %65
  %80 = icmp slt i32 %65, 0
  %.18 = select i1 %80, i32 %79, i32 %65
  %xstep_s.0 = select i1 %80, i32 -1, i32 %72
  %81 = sub nsw i32 0, %66
  %82 = icmp slt i32 %66, 0
  %.19 = select i1 %82, i32 %81, i32 %66
  %ystep_s.0 = select i1 %82, i32 -1, i32 %74
  %83 = icmp slt i32 %.16, %.17
  %84 = tail call float @_Z13convert_floati(i32 %.16) nounwind
  %85 = tail call float @_Z13convert_floati(i32 %.16) nounwind
  %86 = fmul float %84, %85
  %87 = tail call float @_Z13convert_floati(i32 %.17) nounwind
  %88 = tail call float @_Z13convert_floati(i32 %.17) nounwind
  %89 = fmul float %87, %88
  %90 = fadd float %86, %89
  %91 = tail call float @_Z4sqrtf(float %90) nounwind
  %92 = fmul float %91, 0xBF847AE140000000
  br i1 %83, label %149, label %93

; <label>:93                                      ; preds = %62
  %94 = tail call float @_Z13convert_floati(i32 %.16) nounwind
  %95 = fdiv float %92, %94
  %96 = tail call float @_Z3expf(float %95) nounwind
  %97 = shl i32 %.17, 1
  %98 = sub nsw i32 %97, %.16
  br i1 %or.cond26, label %99, label %118

; <label>:99                                      ; preds = %93
  %100 = mul nsw i32 %.17, %23
  %101 = add nsw i32 %98, %100
  %102 = shl i32 %.16, 1
  %103 = icmp eq i32 %102, 0
  %104 = select i1 %103, i32 1, i32 %102
  %105 = sdiv i32 %101, %104
  %106 = mul nsw i32 %105, %102
  %107 = sub nsw i32 %101, %106
  %108 = icmp sgt i32 %107, 0
  %109 = zext i1 %108 to i32
  %.21 = add i32 %109, %105
  %110 = mul nsw i32 %xstep.0, %21
  %111 = add nsw i32 %110, %2
  %112 = mul nsw i32 %.21, %ystep.0
  %113 = add nsw i32 %112, %4
  %114 = mul nsw i32 %.17, %22
  %115 = mul nsw i32 %.21, %102
  %116 = add i32 %98, %114
  %117 = sub i32 %116, %115
  br label %118

; <label>:118                                     ; preds = %99, %93
  %di.1 = phi i32 [ %117, %99 ], [ %98, %93 ]
  %y.0 = phi i32 [ %113, %99 ], [ %4, %93 ]
  %x.0 = phi i32 [ %111, %99 ], [ %2, %93 ]
  %119 = icmp slt i32 %y.0, 0
  %120 = icmp sgt i32 %y.0, %8
  %or.cond22 = or i1 %119, %120
  br i1 %or.cond22, label %121, label %146

; <label>:121                                     ; preds = %118
  %122 = sub nsw i32 0, %y.0
  %123 = sub nsw i32 %y.0, %8
  %124 = select i1 %119, i32 %122, i32 %123
  %125 = shl i32 %124, 1
  %126 = add i32 %125, -2
  %127 = mul nsw i32 %126, %.16
  %128 = sub nsw i32 %di.1, %127
  %129 = icmp eq i32 %97, 0
  %130 = select i1 %129, i32 1, i32 %97
  %131 = sdiv i32 %128, %130
  %132 = sub nsw i32 1, %131
  %133 = srem i32 %128, %130
  %134 = icmp ne i32 %133, 0
  %135 = icmp slt i32 %128, 0
  %or.cond = and i1 %134, %135
  %136 = zext i1 %or.cond to i32
  %dx_crop4.0 = add i32 %136, %132
  %137 = mul nsw i32 %dx_crop4.0, %xstep.0
  %138 = add nsw i32 %137, %x.0
  %139 = mul nsw i32 %124, %ystep.0
  %140 = add nsw i32 %139, %y.0
  %141 = mul nsw i32 %dx_crop4.0, %97
  %142 = shl i32 %.16, 1
  %143 = mul nsw i32 %142, %124
  %144 = sub i32 %di.1, %143
  %145 = add nsw i32 %144, %141
  br label %146

; <label>:146                                     ; preds = %121, %118
  %di.2 = phi i32 [ %145, %121 ], [ %di.1, %118 ]
  %y.2 = phi i32 [ %140, %121 ], [ %y.0, %118 ]
  %x.2 = phi i32 [ %138, %121 ], [ %x.0, %118 ]
  %147 = sub nsw i32 %x_dst.0, %x.2
  %148 = tail call i32 @_Z3absi(i32 %147) nounwind
  br label %205

; <label>:149                                     ; preds = %62
  %150 = tail call float @_Z13convert_floati(i32 %.17) nounwind
  %151 = fdiv float %92, %150
  %152 = tail call float @_Z3expf(float %151) nounwind
  %153 = shl i32 %.16, 1
  %154 = sub nsw i32 %153, %.17
  br i1 %or.cond23, label %155, label %174

; <label>:155                                     ; preds = %149
  %156 = mul nsw i32 %.16, %15
  %157 = add nsw i32 %154, %156
  %158 = shl i32 %.17, 1
  %159 = icmp eq i32 %158, 0
  %160 = select i1 %159, i32 1, i32 %158
  %161 = sdiv i32 %157, %160
  %162 = mul nsw i32 %161, %158
  %163 = sub nsw i32 %157, %162
  %164 = icmp sgt i32 %163, 0
  %165 = zext i1 %164 to i32
  %.24 = add i32 %165, %161
  %166 = mul nsw i32 %.24, %xstep.0
  %167 = add nsw i32 %166, %2
  %168 = mul nsw i32 %ystep.0, %13
  %169 = add nsw i32 %168, %4
  %170 = mul nsw i32 %.16, %14
  %171 = mul nsw i32 %.24, %158
  %172 = add i32 %154, %170
  %173 = sub i32 %172, %171
  br label %174

; <label>:174                                     ; preds = %155, %149
  %di.3 = phi i32 [ %173, %155 ], [ %154, %149 ]
  %y.3 = phi i32 [ %169, %155 ], [ %4, %149 ]
  %x.3 = phi i32 [ %167, %155 ], [ %2, %149 ]
  %175 = icmp slt i32 %x.3, 0
  %176 = icmp sgt i32 %x.3, %7
  %or.cond25 = or i1 %175, %176
  br i1 %or.cond25, label %177, label %202

; <label>:177                                     ; preds = %174
  %178 = sub nsw i32 0, %x.3
  %179 = sub nsw i32 %x.3, %7
  %180 = select i1 %175, i32 %178, i32 %179
  %181 = shl i32 %180, 1
  %182 = add i32 %181, -2
  %183 = mul nsw i32 %182, %.17
  %184 = sub nsw i32 %di.3, %183
  %185 = icmp eq i32 %153, 0
  %186 = select i1 %185, i32 1, i32 %153
  %187 = sdiv i32 %184, %186
  %188 = sub nsw i32 1, %187
  %189 = srem i32 %184, %186
  %190 = icmp ne i32 %189, 0
  %191 = icmp slt i32 %184, 0
  %or.cond3 = and i1 %190, %191
  %192 = zext i1 %or.cond3 to i32
  %dy_crop12.0 = add i32 %192, %188
  %193 = mul nsw i32 %180, %xstep.0
  %194 = add nsw i32 %193, %x.3
  %195 = mul nsw i32 %dy_crop12.0, %ystep.0
  %196 = add nsw i32 %195, %y.3
  %197 = mul nsw i32 %dy_crop12.0, %153
  %198 = shl i32 %.17, 1
  %199 = mul nsw i32 %198, %180
  %200 = sub i32 %di.3, %199
  %201 = add nsw i32 %200, %197
  br label %202

; <label>:202                                     ; preds = %177, %174
  %di.4 = phi i32 [ %201, %177 ], [ %di.3, %174 ]
  %y.4 = phi i32 [ %196, %177 ], [ %y.3, %174 ]
  %x.4 = phi i32 [ %194, %177 ], [ %x.3, %174 ]
  %203 = sub nsw i32 %y_dst.0, %y.4
  %204 = tail call i32 @_Z3absi(i32 %203) nounwind
  br label %205

; <label>:205                                     ; preds = %202, %146
  %steps.0 = phi i32 [ %148, %146 ], [ %204, %202 ]
  %di.0 = phi i32 [ %di.2, %146 ], [ %di.4, %202 ]
  %FixedDecay.0 = phi float [ %96, %146 ], [ %152, %202 ]
  %y.1 = phi i32 [ %y.2, %146 ], [ %y.4, %202 ]
  %x.1 = phi i32 [ %x.2, %146 ], [ %x.4, %202 ]
  %206 = icmp slt i32 %.18, %.19
  br i1 %206, label %273, label %207

; <label>:207                                     ; preds = %205
  %208 = shl i32 %.19, 1
  %209 = sub nsw i32 %208, %.18
  br i1 %or.cond26, label %210, label %229

; <label>:210                                     ; preds = %207
  %211 = mul nsw i32 %.19, %23
  %212 = add nsw i32 %209, %211
  %213 = shl i32 %.18, 1
  %214 = icmp eq i32 %213, 0
  %215 = select i1 %214, i32 1, i32 %213
  %216 = sdiv i32 %212, %215
  %217 = mul nsw i32 %216, %213
  %218 = sub nsw i32 %212, %217
  %219 = icmp sgt i32 %218, 0
  %220 = zext i1 %219 to i32
  %.27 = add i32 %220, %216
  %221 = mul nsw i32 %xstep_s.0, %21
  %222 = add nsw i32 %221, %2
  %223 = mul nsw i32 %.27, %ystep_s.0
  %224 = add nsw i32 %223, %4
  %225 = mul nsw i32 %.19, %22
  %226 = mul nsw i32 %.27, %213
  %227 = add i32 %209, %225
  %228 = sub i32 %227, %226
  br label %229

; <label>:229                                     ; preds = %210, %207
  %di_s.1 = phi i32 [ %228, %210 ], [ %209, %207 ]
  %y_s.0 = phi i32 [ %224, %210 ], [ %4, %207 ]
  %x_s.0 = phi i32 [ %222, %210 ], [ %2, %207 ]
  %230 = icmp slt i32 %y_s.0, 0
  %231 = icmp sgt i32 %y_s.0, %8
  %or.cond28 = or i1 %230, %231
  br i1 %or.cond28, label %232, label %257

; <label>:232                                     ; preds = %229
  %233 = sub nsw i32 0, %y_s.0
  %234 = sub nsw i32 %y_s.0, %8
  %235 = select i1 %230, i32 %233, i32 %234
  %236 = shl i32 %235, 1
  %237 = add i32 %236, -2
  %238 = mul nsw i32 %237, %.18
  %239 = sub nsw i32 %di_s.1, %238
  %240 = icmp eq i32 %208, 0
  %241 = select i1 %240, i32 1, i32 %208
  %242 = sdiv i32 %239, %241
  %243 = sub nsw i32 1, %242
  %244 = srem i32 %239, %241
  %245 = icmp ne i32 %244, 0
  %246 = icmp slt i32 %239, 0
  %or.cond5 = and i1 %245, %246
  %247 = zext i1 %or.cond5 to i32
  %dx_crop_s17.0 = add i32 %247, %243
  %248 = mul nsw i32 %dx_crop_s17.0, %xstep_s.0
  %249 = add nsw i32 %248, %x_s.0
  %250 = mul nsw i32 %235, %ystep_s.0
  %251 = add nsw i32 %250, %y_s.0
  %252 = mul nsw i32 %dx_crop_s17.0, %208
  %253 = shl i32 %.18, 1
  %254 = mul nsw i32 %253, %235
  %255 = sub i32 %di_s.1, %254
  %256 = add nsw i32 %255, %252
  br label %257

; <label>:257                                     ; preds = %232, %229
  %di_s.2 = phi i32 [ %256, %232 ], [ %di_s.1, %229 ]
  %y_s.2 = phi i32 [ %251, %232 ], [ %y_s.0, %229 ]
  %x_s.2 = phi i32 [ %249, %232 ], [ %x_s.0, %229 ]
  %258 = sub nsw i32 %x.1, %x_s.2
  %259 = mul nsw i32 %258, %xstep_s.0
  %260 = icmp sgt i32 %259, 0
  br i1 %260, label %261, label %269

; <label>:261                                     ; preds = %257
  %262 = mul nsw i32 %259, %208
  %263 = add nsw i32 %262, %di_s.2
  %264 = icmp sgt i32 %263, %208
  br i1 %264, label %265, label %269

; <label>:265                                     ; preds = %261
  %266 = shl i32 %.18, 1
  %267 = sub nsw i32 %263, %266
  %268 = add nsw i32 %y_s.2, %ystep_s.0
  br label %269

; <label>:269                                     ; preds = %265, %261, %257
  %di_s.3 = phi i32 [ %267, %265 ], [ %263, %261 ], [ %di_s.2, %257 ]
  %y_s.3 = phi i32 [ %268, %265 ], [ %y_s.2, %261 ], [ %y_s.2, %257 ]
  %x_s.3 = phi i32 [ %x.1, %265 ], [ %x.1, %261 ], [ %x_s.2, %257 ]
  %270 = sub nsw i32 %x_dst_s.0, %x_s.3
  %271 = tail call i32 @_Z3absi(i32 %270) nounwind
  %272 = sub i32 %271, %259
  br label %bb.nph57

; <label>:273                                     ; preds = %205
  %274 = shl i32 %.18, 1
  %275 = sub nsw i32 %274, %.19
  br i1 %or.cond23, label %276, label %295

; <label>:276                                     ; preds = %273
  %277 = mul nsw i32 %.18, %15
  %278 = add nsw i32 %275, %277
  %279 = shl i32 %.19, 1
  %280 = icmp eq i32 %279, 0
  %281 = select i1 %280, i32 1, i32 %279
  %282 = sdiv i32 %278, %281
  %283 = mul nsw i32 %282, %279
  %284 = sub nsw i32 %278, %283
  %285 = icmp sgt i32 %284, 0
  %286 = zext i1 %285 to i32
  %.30 = add i32 %286, %282
  %287 = mul nsw i32 %.30, %xstep_s.0
  %288 = add nsw i32 %287, %2
  %289 = mul nsw i32 %ystep_s.0, %13
  %290 = add nsw i32 %289, %4
  %291 = mul nsw i32 %.18, %14
  %292 = mul nsw i32 %.30, %279
  %293 = add i32 %275, %291
  %294 = sub i32 %293, %292
  br label %295

; <label>:295                                     ; preds = %276, %273
  %di_s.4 = phi i32 [ %294, %276 ], [ %275, %273 ]
  %y_s.4 = phi i32 [ %290, %276 ], [ %4, %273 ]
  %x_s.4 = phi i32 [ %288, %276 ], [ %2, %273 ]
  %296 = icmp slt i32 %x_s.4, 0
  %297 = icmp sgt i32 %x_s.4, %7
  %or.cond31 = or i1 %296, %297
  br i1 %or.cond31, label %298, label %323

; <label>:298                                     ; preds = %295
  %299 = sub nsw i32 0, %x_s.4
  %300 = sub nsw i32 %x_s.4, %7
  %301 = select i1 %296, i32 %299, i32 %300
  %302 = shl i32 %301, 1
  %303 = add i32 %302, -2
  %304 = mul nsw i32 %303, %.19
  %305 = sub nsw i32 %di_s.4, %304
  %306 = icmp eq i32 %274, 0
  %307 = select i1 %306, i32 1, i32 %274
  %308 = sdiv i32 %305, %307
  %309 = sub nsw i32 1, %308
  %310 = srem i32 %305, %307
  %311 = icmp ne i32 %310, 0
  %312 = icmp slt i32 %305, 0
  %or.cond7 = and i1 %311, %312
  %313 = zext i1 %or.cond7 to i32
  %dy_crop_s25.0 = add i32 %313, %309
  %314 = mul nsw i32 %301, %xstep_s.0
  %315 = add nsw i32 %314, %x_s.4
  %316 = mul nsw i32 %dy_crop_s25.0, %ystep_s.0
  %317 = add nsw i32 %316, %y_s.4
  %318 = mul nsw i32 %dy_crop_s25.0, %274
  %319 = shl i32 %.19, 1
  %320 = mul nsw i32 %319, %301
  %321 = sub i32 %di_s.4, %320
  %322 = add nsw i32 %321, %318
  br label %323

; <label>:323                                     ; preds = %298, %295
  %di_s.5 = phi i32 [ %322, %298 ], [ %di_s.4, %295 ]
  %y_s.5 = phi i32 [ %317, %298 ], [ %y_s.4, %295 ]
  %x_s.5 = phi i32 [ %315, %298 ], [ %x_s.4, %295 ]
  %324 = sub nsw i32 %y.1, %y_s.5
  %325 = mul nsw i32 %324, %ystep_s.0
  %326 = icmp sgt i32 %325, 0
  br i1 %326, label %327, label %335

; <label>:327                                     ; preds = %323
  %328 = mul nsw i32 %325, %274
  %329 = add nsw i32 %328, %di_s.5
  %330 = icmp sgt i32 %329, %274
  br i1 %330, label %331, label %335

; <label>:331                                     ; preds = %327
  %332 = shl i32 %.19, 1
  %333 = sub nsw i32 %329, %332
  %334 = add nsw i32 %x_s.5, %ystep_s.0
  br label %335

; <label>:335                                     ; preds = %331, %327, %323
  %di_s.6 = phi i32 [ %333, %331 ], [ %329, %327 ], [ %di_s.5, %323 ]
  %y_s.6 = phi i32 [ %y.1, %331 ], [ %y.1, %327 ], [ %y_s.5, %323 ]
  %x_s.6 = phi i32 [ %334, %331 ], [ %x_s.5, %327 ], [ %x_s.5, %323 ]
  %336 = sub nsw i32 %y_dst_s.0, %y_s.6
  %337 = tail call i32 @_Z3absi(i32 %336) nounwind
  %338 = sub i32 %337, %325
  br label %bb.nph57

bb.nph57:                                         ; preds = %335, %269
  %di_s.0 = phi i32 [ %di_s.3, %269 ], [ %di_s.6, %335 ]
  %y_s.1 = phi i32 [ %y_s.3, %269 ], [ %y_s.6, %335 ]
  %x_s.1 = phi i32 [ %x_s.3, %269 ], [ %x_s.6, %335 ]
  %steps_begin.0 = phi i32 [ %259, %269 ], [ %325, %335 ]
  %steps_lsat.0 = phi i32 [ %272, %269 ], [ %338, %335 ]
  %339 = fsub float 1.000000e+000, %FixedDecay.0
  %340 = or i32 %x_dst.0, %y_dst.0
  %341 = icmp eq i32 %340, 0
  %342 = icmp eq i32 %x.1, %2
  %or.cond34 = and i1 %341, %342
  %343 = icmp eq i32 %y.1, %4
  %or.cond35 = and i1 %or.cond34, %343
  %344 = icmp sgt i32 %steps.0, 0
  %345 = shl i32 %.17, 1
  %346 = shl i32 %.16, 1
  %347 = shl i32 %.19, 1
  %348 = shl i32 %.18, 1
  %notlhs = icmp ne i32 %x.1, %x_s.1
  %notrhs = icmp ne i32 %y.1, %y_s.1
  %or.cond32.not = or i1 %notrhs, %notlhs
  %brmerge = or i1 %or.cond32.not, %or.cond35
  %tmp38 = mul i32 %1, %y.1
  %tmp39 = add i32 %x.1, %tmp38
  %tmp40 = shl i32 %tmp39, 2
  br label %349

; <label>:349                                     ; preds = %._crit_edge, %bb.nph57
  %ch.056 = phi i32 [ 0, %bb.nph57 ], [ %398, %._crit_edge ]
  %tmp41 = add i32 %tmp40, %ch.056
  %scevgep = getelementptr float addrspace(1)* %inputImage, i32 %tmp41
  %350 = load float addrspace(1)* %scevgep, align 4
  %351 = fmul float %350, 0x3FF1C71C80000000
  %352 = fmul float %351, %339
  br i1 %brmerge, label %353, label %.preheader

; <label>:353                                     ; preds = %349
  %scevgep67 = getelementptr float addrspace(1)* %output, i32 %tmp41
  %354 = fmul float %352, 3.000000e+000
  %355 = fadd float %354, %351
  store float %355, float addrspace(1)* %scevgep67, align 4
  br label %.preheader

.preheader:                                       ; preds = %353, %349
  br i1 %344, label %bb.nph.preheader, label %._crit_edge

bb.nph.preheader:                                 ; preds = %.preheader
  br label %bb.nph

bb.nph:                                           ; preds = %bb.nph.preheader, %396
  %356 = phi i32 [ %397, %396 ], [ 0, %bb.nph.preheader ]
  %summ.054 = phi float [ %385, %396 ], [ %352, %bb.nph.preheader ]
  %steps_begin.253 = phi i32 [ %steps_begin.1, %396 ], [ %steps_begin.0, %bb.nph.preheader ]
  %x.652 = phi i32 [ %x.5, %396 ], [ %x.1, %bb.nph.preheader ]
  %y.751 = phi i32 [ %y.6, %396 ], [ %y.1, %bb.nph.preheader ]
  %x_s.850 = phi i32 [ %x_s.7, %396 ], [ %x_s.1, %bb.nph.preheader ]
  %y_s.949 = phi i32 [ %y_s.8, %396 ], [ %y_s.1, %bb.nph.preheader ]
  %di.748 = phi i32 [ %di.6, %396 ], [ %di.0, %bb.nph.preheader ]
  %di_s.947 = phi i32 [ %di_s.8, %396 ], [ %di_s.0, %bb.nph.preheader ]
  %357 = icmp sgt i32 %di.748, -1
  br i1 %83, label %361, label %358

; <label>:358                                     ; preds = %bb.nph
  %.op = sub i32 0, %346
  %.neg12 = select i1 %357, i32 %.op, i32 0
  %359 = select i1 %357, i32 %ystep.0, i32 0
  %di.5 = add i32 %di.748, %345
  %360 = add nsw i32 %di.5, %.neg12
  br label %364

; <label>:361                                     ; preds = %bb.nph
  %.op18 = sub i32 0, %345
  %.neg13 = select i1 %357, i32 %.op18, i32 0
  %362 = select i1 %357, i32 %xstep.0, i32 0
  %di.8 = add i32 %di.748, %346
  %363 = add nsw i32 %di.8, %.neg13
  br label %364

; <label>:364                                     ; preds = %361, %358
  %di.6 = phi i32 [ %360, %358 ], [ %363, %361 ]
  %.pn = phi i32 [ %359, %358 ], [ %ystep.0, %361 ]
  %xstep.0.pn = phi i32 [ %xstep.0, %358 ], [ %362, %361 ]
  %x.5 = add i32 %xstep.0.pn, %x.652
  %y.6 = add i32 %.pn, %y.751
  %365 = icmp sgt i32 %steps_begin.253, -1
  br i1 %365, label %366, label %377

; <label>:366                                     ; preds = %364
  br i1 %206, label %372, label %367

; <label>:367                                     ; preds = %366
  %368 = add nsw i32 %x_s.850, %xstep_s.0
  %369 = icmp sgt i32 %di_s.947, -1
  %.op16 = sub i32 0, %348
  %.neg14 = select i1 %369, i32 %.op16, i32 0
  %370 = select i1 %369, i32 %ystep_s.0, i32 0
  %y_s.7 = add i32 %370, %y_s.949
  %di_s.7 = add i32 %di_s.947, %347
  %371 = add nsw i32 %di_s.7, %.neg14
  br label %379

; <label>:372                                     ; preds = %366
  %373 = add nsw i32 %y_s.949, %ystep_s.0
  %374 = icmp sgt i32 %di_s.947, -1
  %.op17 = sub i32 0, %347
  %.neg15 = select i1 %374, i32 %.op17, i32 0
  %375 = select i1 %374, i32 %xstep_s.0, i32 0
  %x_s.9 = add i32 %375, %x_s.850
  %di_s.10 = add i32 %di_s.947, %348
  %376 = add nsw i32 %di_s.10, %.neg15
  br label %379

; <label>:377                                     ; preds = %364
  %378 = add nsw i32 %steps_begin.253, 1
  br label %379

; <label>:379                                     ; preds = %377, %372, %367
  %di_s.8 = phi i32 [ %371, %367 ], [ %376, %372 ], [ %di_s.947, %377 ]
  %y_s.8 = phi i32 [ %y_s.7, %367 ], [ %373, %372 ], [ %y_s.949, %377 ]
  %x_s.7 = phi i32 [ %368, %367 ], [ %x_s.9, %372 ], [ %x_s.850, %377 ]
  %steps_begin.1 = phi i32 [ %steps_begin.253, %367 ], [ %steps_begin.253, %372 ], [ %378, %377 ]
  %tmp34 = mul i32 %1, %y.6
  %tmp35 = add i32 %x.5, %tmp34
  %tmp36 = shl i32 %tmp35, 2
  %tmp37 = add i32 %ch.056, %tmp36
  %380 = getelementptr inbounds float addrspace(1)* %inputImage, i32 %tmp37
  %381 = load float addrspace(1)* %380, align 4
  %382 = fmul float %summ.054, %FixedDecay.0
  %383 = fmul float %381, 0x3FF1C71C80000000
  %384 = fmul float %383, %339
  %385 = fadd float %382, %384
  %386 = icmp eq i32 %x.5, %x_s.7
  %387 = icmp eq i32 %y.6, %y_s.8
  %or.cond36 = and i1 %386, %387
  %388 = icmp slt i32 %356, %steps_lsat.0
  %or.cond37 = and i1 %or.cond36, %388
  br i1 %or.cond37, label %396, label %389

; <label>:389                                     ; preds = %379
  %390 = fmul float %385, 3.000000e+000
  br i1 %16, label %391, label %394

; <label>:391                                     ; preds = %389
  %392 = fadd float %390, %381
  %393 = getelementptr inbounds float addrspace(1)* %output, i32 %tmp37
  store float %392, float addrspace(1)* %393, align 4
  br label %396

; <label>:394                                     ; preds = %389
  %395 = getelementptr inbounds float addrspace(1)* %output, i32 %tmp37
  store float %390, float addrspace(1)* %395, align 4
  br label %396

; <label>:396                                     ; preds = %394, %391, %379
  %397 = add nsw i32 %356, 1
  %exitcond19 = icmp eq i32 %397, %steps.0
  br i1 %exitcond19, label %._crit_edge.loopexit, label %bb.nph

._crit_edge.loopexit:                             ; preds = %396
  br label %._crit_edge

._crit_edge:                                      ; preds = %._crit_edge.loopexit, %.preheader
  %398 = add nsw i32 %ch.056, 1
  %exitcond = icmp eq i32 %398, 4
  br i1 %exitcond, label %._crit_edge58, label %349

._crit_edge58:                                    ; preds = %._crit_edge
  %399 = add nsw i32 %28, 1
  %indvar.next = add i32 %indvar, 1
  br label %27

; <label>:400                                     ; preds = %57, %54, %49, %41, %33, %27
  ret void
}

declare i32 @get_global_id(i32)

define void @intel_god_rays_scalar(<4 x float> addrspace(1)* nocapture %inputImage, <4 x float> addrspace(1)* nocapture %output, i32 %width, i32 %height, i32 %blend) nounwind {
; <label>:0
  %1 = tail call i32 @get_global_id(i32 0) nounwind
  %2 = sdiv i32 %width, 2
  %3 = sdiv i32 %height, 2
  %4 = mul nsw i32 %1, 15
  %5 = add nsw i32 %4, 15
  %6 = add nsw i32 %width, -1
  %7 = add nsw i32 %height, -1
  %8 = icmp slt i32 %height, -1
  %.not.i = icmp sgt i32 %width, -2
  %9 = icmp sgt i32 %3, %7
  %or.cond23.i = or i1 %8, %9
  %10 = sub nsw i32 0, %3
  %11 = sub nsw i32 %3, %7
  %12 = select i1 %8, i32 %10, i32 %11
  %13 = shl i32 %12, 1
  %14 = add i32 %13, -2
  %15 = icmp eq i32 %blend, 1
  %16 = icmp slt i32 %width, -1
  %17 = icmp sgt i32 %2, %6
  %or.cond26.i = or i1 %16, %17
  %18 = sub nsw i32 0, %2
  %19 = sub nsw i32 %2, %6
  %20 = select i1 %16, i32 %18, i32 %19
  %21 = shl i32 %20, 1
  %22 = add i32 %21, -2
  %.not9.i = icmp sgt i32 %height, -2
  %23 = add nsw i32 %width, -2
  %.not11.i = icmp sle i32 %2, %6
  %24 = add nsw i32 %height, -2
  %.not14.i = icmp sle i32 %3, %7
  %25 = add i32 %6, %height
  %tmp45 = shl i32 %width, 1
  %tmp46 = add i32 %tmp45, %height
  %tmp47 = add i32 %tmp46, -3
  %tmp49 = sub i32 %tmp47, %4
  %tmp51 = shl i32 %height, 1
  %tmp52 = add i32 %tmp51, %tmp45
  %tmp53 = add i32 %tmp52, -4
  %tmp54 = sub i32 %tmp53, %4
  %tmp56 = add i32 %4, 3
  %tmp57 = sub i32 %tmp56, %height
  %tmp58 = sub i32 %tmp57, %tmp45
  %tmp60 = add i32 %4, 2
  %tmp61 = sub i32 %tmp60, %height
  %tmp62 = sub i32 %tmp61, %width
  %tmp64 = add i32 %4, 1
  %tmp65 = sub i32 %tmp64, %width
  %tmp67 = sub i32 %4, %width
  %tmp70 = add i32 %4, -1
  br label %26

; <label>:26                                      ; preds = %._crit_edge58.i, %0
  %indvar.i = phi i32 [ %indvar.next.i, %._crit_edge58.i ], [ 0, %0 ]
  %27 = phi i32 [ %398, %._crit_edge58.i ], [ %4, %0 ]
  %tmp111.i = sub i32 %tmp49, %indvar.i
  %tmp103.i = sub i32 %tmp54, %indvar.i
  %tmp107.i = add i32 %tmp58, %indvar.i
  %tmp96.i = add i32 %tmp62, %indvar.i
  %tmp92.i = add i32 %tmp65, %indvar.i
  %tmp89.i = add i32 %tmp67, %indvar.i
  %i.0.i = add i32 %4, %indvar.i
  %tmp86.i = add i32 %tmp70, %indvar.i
  %28 = icmp slt i32 %i.0.i, %5
  br i1 %28, label %29, label %evaluateRayScalar.exit

; <label>:29                                      ; preds = %26
  %30 = icmp slt i32 %i.0.i, %6
  br i1 %30, label %31, label %36

; <label>:31                                      ; preds = %29
  br i1 %8, label %32, label %._crit_edge115.i

; <label>:32                                      ; preds = %31
  %33 = icmp eq i32 %i.0.i, 0
  %or.cond8.i = and i1 %.not.i, %33
  br i1 %or.cond8.i, label %._crit_edge115.i, label %evaluateRayScalar.exit

._crit_edge115.i:                                 ; preds = %31, %32
  %34 = icmp eq i32 %i.0.i, 0
  br i1 %34, label %35, label %61

; <label>:35                                      ; preds = %._crit_edge115.i
  br label %61

; <label>:36                                      ; preds = %29
  %37 = sub nsw i32 %27, %6
  %38 = icmp slt i32 %tmp92.i, %7
  br i1 %38, label %39, label %44

; <label>:39                                      ; preds = %36
  br i1 %17, label %40, label %._crit_edge114.i

; <label>:40                                      ; preds = %39
  %41 = icmp eq i32 %i.0.i, %6
  %or.cond10.i = and i1 %.not9.i, %41
  br i1 %or.cond10.i, label %._crit_edge114.i, label %evaluateRayScalar.exit

._crit_edge114.i:                                 ; preds = %39, %40
  %42 = icmp eq i32 %i.0.i, %6
  %..i = select i1 %42, i32 %23, i32 %6
  br i1 %42, label %43, label %61

; <label>:43                                      ; preds = %._crit_edge114.i
  br label %61

; <label>:44                                      ; preds = %36
  %45 = sub nsw i32 %37, %7
  %46 = icmp slt i32 %tmp96.i, %6
  br i1 %46, label %47, label %53

; <label>:47                                      ; preds = %44
  br i1 %9, label %48, label %._crit_edge113.i

; <label>:48                                      ; preds = %47
  %49 = icmp eq i32 %tmp92.i, %7
  %or.cond12.i = and i1 %.not11.i, %49
  br i1 %or.cond12.i, label %._crit_edge113.i, label %evaluateRayScalar.exit

._crit_edge113.i:                                 ; preds = %47, %48
  %50 = sub i32 %width, %45
  %51 = icmp eq i32 %tmp92.i, %7
  %.13.i = select i1 %51, i32 %24, i32 %7
  br i1 %51, label %52, label %61

; <label>:52                                      ; preds = %._crit_edge113.i
  br label %61

; <label>:53                                      ; preds = %44
  %54 = icmp slt i32 %tmp107.i, %7
  br i1 %54, label %55, label %evaluateRayScalar.exit

; <label>:55                                      ; preds = %53
  br i1 %16, label %56, label %._crit_edge112.i

; <label>:56                                      ; preds = %55
  %57 = icmp eq i32 %tmp96.i, %6
  %or.cond15.i = and i1 %.not14.i, %57
  br i1 %or.cond15.i, label %._crit_edge112.i, label %evaluateRayScalar.exit

._crit_edge112.i:                                 ; preds = %55, %56
  %58 = sub i32 %25, %45
  %59 = icmp eq i32 %tmp96.i, %6
  br i1 %59, label %60, label %61

; <label>:60                                      ; preds = %._crit_edge112.i
  br label %61

; <label>:61                                      ; preds = %60, %._crit_edge112.i, %52, %._crit_edge113.i, %43, %._crit_edge114.i, %35, %._crit_edge115.i
  %y_dst_s.0.i = phi i32 [ 1, %35 ], [ 0, %._crit_edge115.i ], [ 0, %43 ], [ %tmp89.i, %._crit_edge114.i ], [ %.13.i, %52 ], [ %.13.i, %._crit_edge113.i ], [ %7, %60 ], [ %58, %._crit_edge112.i ]
  %x_dst_s.0.i = phi i32 [ 0, %35 ], [ %tmp86.i, %._crit_edge115.i ], [ %..i, %43 ], [ %..i, %._crit_edge114.i ], [ %6, %52 ], [ %50, %._crit_edge113.i ], [ 1, %60 ], [ 0, %._crit_edge112.i ]
  %y_dst.0.i = phi i32 [ 0, %35 ], [ 0, %._crit_edge115.i ], [ %37, %43 ], [ %37, %._crit_edge114.i ], [ %7, %52 ], [ %7, %._crit_edge113.i ], [ %tmp103.i, %60 ], [ %tmp103.i, %._crit_edge112.i ]
  %x_dst.0.i = phi i32 [ %27, %35 ], [ %27, %._crit_edge115.i ], [ %6, %43 ], [ %6, %._crit_edge114.i ], [ %tmp111.i, %52 ], [ %tmp111.i, %._crit_edge113.i ], [ 0, %60 ], [ 0, %._crit_edge112.i ]
  %62 = sub nsw i32 %x_dst.0.i, %2
  %63 = sub nsw i32 %y_dst.0.i, %3
  %64 = sub nsw i32 %x_dst_s.0.i, %2
  %65 = sub nsw i32 %y_dst_s.0.i, %3
  %66 = icmp sgt i32 %62, 0
  %67 = zext i1 %66 to i32
  %68 = icmp sgt i32 %63, 0
  %69 = zext i1 %68 to i32
  %70 = icmp sgt i32 %64, 0
  %71 = zext i1 %70 to i32
  %72 = icmp sgt i32 %65, 0
  %73 = zext i1 %72 to i32
  %74 = sub nsw i32 0, %62
  %75 = icmp slt i32 %62, 0
  %.16.i = select i1 %75, i32 %74, i32 %62
  %xstep.0.i = select i1 %75, i32 -1, i32 %67
  %76 = sub nsw i32 0, %63
  %77 = icmp slt i32 %63, 0
  %.17.i = select i1 %77, i32 %76, i32 %63
  %ystep.0.i = select i1 %77, i32 -1, i32 %69
  %78 = sub nsw i32 0, %64
  %79 = icmp slt i32 %64, 0
  %.18.i = select i1 %79, i32 %78, i32 %64
  %xstep_s.0.i = select i1 %79, i32 -1, i32 %71
  %80 = sub nsw i32 0, %65
  %81 = icmp slt i32 %65, 0
  %.19.i = select i1 %81, i32 %80, i32 %65
  %ystep_s.0.i = select i1 %81, i32 -1, i32 %73
  %82 = icmp slt i32 %.16.i, %.17.i
  %83 = tail call float @_Z13convert_floati(i32 %.16.i) nounwind
  %84 = tail call float @_Z13convert_floati(i32 %.16.i) nounwind
  %85 = fmul float %83, %84
  %86 = tail call float @_Z13convert_floati(i32 %.17.i) nounwind
  %87 = tail call float @_Z13convert_floati(i32 %.17.i) nounwind
  %88 = fmul float %86, %87
  %89 = fadd float %85, %88
  %90 = tail call float @_Z4sqrtf(float %89) nounwind
  %91 = fmul float %90, 0xBF847AE140000000
  br i1 %82, label %148, label %92

; <label>:92                                      ; preds = %61
  %93 = tail call float @_Z13convert_floati(i32 %.16.i) nounwind
  %94 = fdiv float %91, %93
  %95 = tail call float @_Z3expf(float %94) nounwind
  %96 = shl i32 %.17.i, 1
  %97 = sub nsw i32 %96, %.16.i
  br i1 %or.cond26.i, label %98, label %117

; <label>:98                                      ; preds = %92
  %99 = mul nsw i32 %.17.i, %22
  %100 = add nsw i32 %97, %99
  %101 = shl i32 %.16.i, 1
  %102 = icmp eq i32 %101, 0
  %103 = select i1 %102, i32 1, i32 %101
  %104 = sdiv i32 %100, %103
  %105 = mul nsw i32 %104, %101
  %106 = sub nsw i32 %100, %105
  %107 = icmp sgt i32 %106, 0
  %108 = zext i1 %107 to i32
  %.21.i = add i32 %108, %104
  %109 = mul nsw i32 %xstep.0.i, %20
  %110 = add nsw i32 %109, %2
  %111 = mul nsw i32 %.21.i, %ystep.0.i
  %112 = add nsw i32 %111, %3
  %113 = mul nsw i32 %.17.i, %21
  %114 = mul nsw i32 %.21.i, %101
  %115 = add i32 %97, %113
  %116 = sub i32 %115, %114
  br label %117

; <label>:117                                     ; preds = %98, %92
  %di.1.i = phi i32 [ %116, %98 ], [ %97, %92 ]
  %y.0.i = phi i32 [ %112, %98 ], [ %3, %92 ]
  %x.0.i = phi i32 [ %110, %98 ], [ %2, %92 ]
  %118 = icmp slt i32 %y.0.i, 0
  %119 = icmp sgt i32 %y.0.i, %7
  %or.cond22.i = or i1 %118, %119
  br i1 %or.cond22.i, label %120, label %145

; <label>:120                                     ; preds = %117
  %121 = sub nsw i32 0, %y.0.i
  %122 = sub nsw i32 %y.0.i, %7
  %123 = select i1 %118, i32 %121, i32 %122
  %124 = shl i32 %123, 1
  %125 = add i32 %124, -2
  %126 = mul nsw i32 %125, %.16.i
  %127 = sub nsw i32 %di.1.i, %126
  %128 = icmp eq i32 %96, 0
  %129 = select i1 %128, i32 1, i32 %96
  %130 = sdiv i32 %127, %129
  %131 = sub nsw i32 1, %130
  %132 = srem i32 %127, %129
  %133 = icmp ne i32 %132, 0
  %134 = icmp slt i32 %127, 0
  %or.cond.i = and i1 %133, %134
  %135 = zext i1 %or.cond.i to i32
  %dx_crop4.0.i = add i32 %135, %131
  %136 = mul nsw i32 %dx_crop4.0.i, %xstep.0.i
  %137 = add nsw i32 %136, %x.0.i
  %138 = mul nsw i32 %123, %ystep.0.i
  %139 = add nsw i32 %138, %y.0.i
  %140 = mul nsw i32 %dx_crop4.0.i, %96
  %141 = shl i32 %.16.i, 1
  %142 = mul nsw i32 %141, %123
  %143 = sub i32 %di.1.i, %142
  %144 = add nsw i32 %143, %140
  br label %145

; <label>:145                                     ; preds = %120, %117
  %di.2.i = phi i32 [ %144, %120 ], [ %di.1.i, %117 ]
  %y.2.i = phi i32 [ %139, %120 ], [ %y.0.i, %117 ]
  %x.2.i = phi i32 [ %137, %120 ], [ %x.0.i, %117 ]
  %146 = sub nsw i32 %x_dst.0.i, %x.2.i
  %147 = tail call i32 @_Z3absi(i32 %146) nounwind
  br label %204

; <label>:148                                     ; preds = %61
  %149 = tail call float @_Z13convert_floati(i32 %.17.i) nounwind
  %150 = fdiv float %91, %149
  %151 = tail call float @_Z3expf(float %150) nounwind
  %152 = shl i32 %.16.i, 1
  %153 = sub nsw i32 %152, %.17.i
  br i1 %or.cond23.i, label %154, label %173

; <label>:154                                     ; preds = %148
  %155 = mul nsw i32 %.16.i, %14
  %156 = add nsw i32 %153, %155
  %157 = shl i32 %.17.i, 1
  %158 = icmp eq i32 %157, 0
  %159 = select i1 %158, i32 1, i32 %157
  %160 = sdiv i32 %156, %159
  %161 = mul nsw i32 %160, %157
  %162 = sub nsw i32 %156, %161
  %163 = icmp sgt i32 %162, 0
  %164 = zext i1 %163 to i32
  %.24.i = add i32 %164, %160
  %165 = mul nsw i32 %.24.i, %xstep.0.i
  %166 = add nsw i32 %165, %2
  %167 = mul nsw i32 %ystep.0.i, %12
  %168 = add nsw i32 %167, %3
  %169 = mul nsw i32 %.16.i, %13
  %170 = mul nsw i32 %.24.i, %157
  %171 = add i32 %153, %169
  %172 = sub i32 %171, %170
  br label %173

; <label>:173                                     ; preds = %154, %148
  %di.3.i = phi i32 [ %172, %154 ], [ %153, %148 ]
  %y.3.i = phi i32 [ %168, %154 ], [ %3, %148 ]
  %x.3.i = phi i32 [ %166, %154 ], [ %2, %148 ]
  %174 = icmp slt i32 %x.3.i, 0
  %175 = icmp sgt i32 %x.3.i, %6
  %or.cond25.i = or i1 %174, %175
  br i1 %or.cond25.i, label %176, label %201

; <label>:176                                     ; preds = %173
  %177 = sub nsw i32 0, %x.3.i
  %178 = sub nsw i32 %x.3.i, %6
  %179 = select i1 %174, i32 %177, i32 %178
  %180 = shl i32 %179, 1
  %181 = add i32 %180, -2
  %182 = mul nsw i32 %181, %.17.i
  %183 = sub nsw i32 %di.3.i, %182
  %184 = icmp eq i32 %152, 0
  %185 = select i1 %184, i32 1, i32 %152
  %186 = sdiv i32 %183, %185
  %187 = sub nsw i32 1, %186
  %188 = srem i32 %183, %185
  %189 = icmp ne i32 %188, 0
  %190 = icmp slt i32 %183, 0
  %or.cond3.i = and i1 %189, %190
  %191 = zext i1 %or.cond3.i to i32
  %dy_crop12.0.i = add i32 %191, %187
  %192 = mul nsw i32 %179, %xstep.0.i
  %193 = add nsw i32 %192, %x.3.i
  %194 = mul nsw i32 %dy_crop12.0.i, %ystep.0.i
  %195 = add nsw i32 %194, %y.3.i
  %196 = mul nsw i32 %dy_crop12.0.i, %152
  %197 = shl i32 %.17.i, 1
  %198 = mul nsw i32 %197, %179
  %199 = sub i32 %di.3.i, %198
  %200 = add nsw i32 %199, %196
  br label %201

; <label>:201                                     ; preds = %176, %173
  %di.4.i = phi i32 [ %200, %176 ], [ %di.3.i, %173 ]
  %y.4.i = phi i32 [ %195, %176 ], [ %y.3.i, %173 ]
  %x.4.i = phi i32 [ %193, %176 ], [ %x.3.i, %173 ]
  %202 = sub nsw i32 %y_dst.0.i, %y.4.i
  %203 = tail call i32 @_Z3absi(i32 %202) nounwind
  br label %204

; <label>:204                                     ; preds = %201, %145
  %steps.0.i = phi i32 [ %147, %145 ], [ %203, %201 ]
  %di.0.i = phi i32 [ %di.2.i, %145 ], [ %di.4.i, %201 ]
  %FixedDecay.0.i = phi float [ %95, %145 ], [ %151, %201 ]
  %y.1.i = phi i32 [ %y.2.i, %145 ], [ %y.4.i, %201 ]
  %x.1.i = phi i32 [ %x.2.i, %145 ], [ %x.4.i, %201 ]
  %205 = icmp slt i32 %.18.i, %.19.i
  br i1 %205, label %272, label %206

; <label>:206                                     ; preds = %204
  %207 = shl i32 %.19.i, 1
  %208 = sub nsw i32 %207, %.18.i
  br i1 %or.cond26.i, label %209, label %228

; <label>:209                                     ; preds = %206
  %210 = mul nsw i32 %.19.i, %22
  %211 = add nsw i32 %208, %210
  %212 = shl i32 %.18.i, 1
  %213 = icmp eq i32 %212, 0
  %214 = select i1 %213, i32 1, i32 %212
  %215 = sdiv i32 %211, %214
  %216 = mul nsw i32 %215, %212
  %217 = sub nsw i32 %211, %216
  %218 = icmp sgt i32 %217, 0
  %219 = zext i1 %218 to i32
  %.27.i = add i32 %219, %215
  %220 = mul nsw i32 %xstep_s.0.i, %20
  %221 = add nsw i32 %220, %2
  %222 = mul nsw i32 %.27.i, %ystep_s.0.i
  %223 = add nsw i32 %222, %3
  %224 = mul nsw i32 %.19.i, %21
  %225 = mul nsw i32 %.27.i, %212
  %226 = add i32 %208, %224
  %227 = sub i32 %226, %225
  br label %228

; <label>:228                                     ; preds = %209, %206
  %di_s.1.i = phi i32 [ %227, %209 ], [ %208, %206 ]
  %y_s.0.i = phi i32 [ %223, %209 ], [ %3, %206 ]
  %x_s.0.i = phi i32 [ %221, %209 ], [ %2, %206 ]
  %229 = icmp slt i32 %y_s.0.i, 0
  %230 = icmp sgt i32 %y_s.0.i, %7
  %or.cond28.i = or i1 %229, %230
  br i1 %or.cond28.i, label %231, label %256

; <label>:231                                     ; preds = %228
  %232 = sub nsw i32 0, %y_s.0.i
  %233 = sub nsw i32 %y_s.0.i, %7
  %234 = select i1 %229, i32 %232, i32 %233
  %235 = shl i32 %234, 1
  %236 = add i32 %235, -2
  %237 = mul nsw i32 %236, %.18.i
  %238 = sub nsw i32 %di_s.1.i, %237
  %239 = icmp eq i32 %207, 0
  %240 = select i1 %239, i32 1, i32 %207
  %241 = sdiv i32 %238, %240
  %242 = sub nsw i32 1, %241
  %243 = srem i32 %238, %240
  %244 = icmp ne i32 %243, 0
  %245 = icmp slt i32 %238, 0
  %or.cond5.i = and i1 %244, %245
  %246 = zext i1 %or.cond5.i to i32
  %dx_crop_s17.0.i = add i32 %246, %242
  %247 = mul nsw i32 %dx_crop_s17.0.i, %xstep_s.0.i
  %248 = add nsw i32 %247, %x_s.0.i
  %249 = mul nsw i32 %234, %ystep_s.0.i
  %250 = add nsw i32 %249, %y_s.0.i
  %251 = mul nsw i32 %dx_crop_s17.0.i, %207
  %252 = shl i32 %.18.i, 1
  %253 = mul nsw i32 %252, %234
  %254 = sub i32 %di_s.1.i, %253
  %255 = add nsw i32 %254, %251
  br label %256

; <label>:256                                     ; preds = %231, %228
  %di_s.2.i = phi i32 [ %255, %231 ], [ %di_s.1.i, %228 ]
  %y_s.2.i = phi i32 [ %250, %231 ], [ %y_s.0.i, %228 ]
  %x_s.2.i = phi i32 [ %248, %231 ], [ %x_s.0.i, %228 ]
  %257 = sub nsw i32 %x.1.i, %x_s.2.i
  %258 = mul nsw i32 %257, %xstep_s.0.i
  %259 = icmp sgt i32 %258, 0
  br i1 %259, label %260, label %268

; <label>:260                                     ; preds = %256
  %261 = mul nsw i32 %258, %207
  %262 = add nsw i32 %261, %di_s.2.i
  %263 = icmp sgt i32 %262, %207
  br i1 %263, label %264, label %268

; <label>:264                                     ; preds = %260
  %265 = shl i32 %.18.i, 1
  %266 = sub nsw i32 %262, %265
  %267 = add nsw i32 %y_s.2.i, %ystep_s.0.i
  br label %268

; <label>:268                                     ; preds = %264, %260, %256
  %di_s.3.i = phi i32 [ %266, %264 ], [ %262, %260 ], [ %di_s.2.i, %256 ]
  %y_s.3.i = phi i32 [ %267, %264 ], [ %y_s.2.i, %260 ], [ %y_s.2.i, %256 ]
  %x_s.3.i = phi i32 [ %x.1.i, %264 ], [ %x.1.i, %260 ], [ %x_s.2.i, %256 ]
  %269 = sub nsw i32 %x_dst_s.0.i, %x_s.3.i
  %270 = tail call i32 @_Z3absi(i32 %269) nounwind
  %271 = sub i32 %270, %258
  br label %bb.nph57.i

; <label>:272                                     ; preds = %204
  %273 = shl i32 %.18.i, 1
  %274 = sub nsw i32 %273, %.19.i
  br i1 %or.cond23.i, label %275, label %294

; <label>:275                                     ; preds = %272
  %276 = mul nsw i32 %.18.i, %14
  %277 = add nsw i32 %274, %276
  %278 = shl i32 %.19.i, 1
  %279 = icmp eq i32 %278, 0
  %280 = select i1 %279, i32 1, i32 %278
  %281 = sdiv i32 %277, %280
  %282 = mul nsw i32 %281, %278
  %283 = sub nsw i32 %277, %282
  %284 = icmp sgt i32 %283, 0
  %285 = zext i1 %284 to i32
  %.30.i = add i32 %285, %281
  %286 = mul nsw i32 %.30.i, %xstep_s.0.i
  %287 = add nsw i32 %286, %2
  %288 = mul nsw i32 %ystep_s.0.i, %12
  %289 = add nsw i32 %288, %3
  %290 = mul nsw i32 %.18.i, %13
  %291 = mul nsw i32 %.30.i, %278
  %292 = add i32 %274, %290
  %293 = sub i32 %292, %291
  br label %294

; <label>:294                                     ; preds = %275, %272
  %di_s.4.i = phi i32 [ %293, %275 ], [ %274, %272 ]
  %y_s.4.i = phi i32 [ %289, %275 ], [ %3, %272 ]
  %x_s.4.i = phi i32 [ %287, %275 ], [ %2, %272 ]
  %295 = icmp slt i32 %x_s.4.i, 0
  %296 = icmp sgt i32 %x_s.4.i, %6
  %or.cond31.i = or i1 %295, %296
  br i1 %or.cond31.i, label %297, label %322

; <label>:297                                     ; preds = %294
  %298 = sub nsw i32 0, %x_s.4.i
  %299 = sub nsw i32 %x_s.4.i, %6
  %300 = select i1 %295, i32 %298, i32 %299
  %301 = shl i32 %300, 1
  %302 = add i32 %301, -2
  %303 = mul nsw i32 %302, %.19.i
  %304 = sub nsw i32 %di_s.4.i, %303
  %305 = icmp eq i32 %273, 0
  %306 = select i1 %305, i32 1, i32 %273
  %307 = sdiv i32 %304, %306
  %308 = sub nsw i32 1, %307
  %309 = srem i32 %304, %306
  %310 = icmp ne i32 %309, 0
  %311 = icmp slt i32 %304, 0
  %or.cond7.i = and i1 %310, %311
  %312 = zext i1 %or.cond7.i to i32
  %dy_crop_s25.0.i = add i32 %312, %308
  %313 = mul nsw i32 %300, %xstep_s.0.i
  %314 = add nsw i32 %313, %x_s.4.i
  %315 = mul nsw i32 %dy_crop_s25.0.i, %ystep_s.0.i
  %316 = add nsw i32 %315, %y_s.4.i
  %317 = mul nsw i32 %dy_crop_s25.0.i, %273
  %318 = shl i32 %.19.i, 1
  %319 = mul nsw i32 %318, %300
  %320 = sub i32 %di_s.4.i, %319
  %321 = add nsw i32 %320, %317
  br label %322

; <label>:322                                     ; preds = %297, %294
  %di_s.5.i = phi i32 [ %321, %297 ], [ %di_s.4.i, %294 ]
  %y_s.5.i = phi i32 [ %316, %297 ], [ %y_s.4.i, %294 ]
  %x_s.5.i = phi i32 [ %314, %297 ], [ %x_s.4.i, %294 ]
  %323 = sub nsw i32 %y.1.i, %y_s.5.i
  %324 = mul nsw i32 %323, %ystep_s.0.i
  %325 = icmp sgt i32 %324, 0
  br i1 %325, label %326, label %334

; <label>:326                                     ; preds = %322
  %327 = mul nsw i32 %324, %273
  %328 = add nsw i32 %327, %di_s.5.i
  %329 = icmp sgt i32 %328, %273
  br i1 %329, label %330, label %334

; <label>:330                                     ; preds = %326
  %331 = shl i32 %.19.i, 1
  %332 = sub nsw i32 %328, %331
  %333 = add nsw i32 %x_s.5.i, %ystep_s.0.i
  br label %334

; <label>:334                                     ; preds = %330, %326, %322
  %di_s.6.i = phi i32 [ %332, %330 ], [ %328, %326 ], [ %di_s.5.i, %322 ]
  %y_s.6.i = phi i32 [ %y.1.i, %330 ], [ %y.1.i, %326 ], [ %y_s.5.i, %322 ]
  %x_s.6.i = phi i32 [ %333, %330 ], [ %x_s.5.i, %326 ], [ %x_s.5.i, %322 ]
  %335 = sub nsw i32 %y_dst_s.0.i, %y_s.6.i
  %336 = tail call i32 @_Z3absi(i32 %335) nounwind
  %337 = sub i32 %336, %324
  br label %bb.nph57.i

bb.nph57.i:                                       ; preds = %334, %268
  %di_s.0.i = phi i32 [ %di_s.3.i, %268 ], [ %di_s.6.i, %334 ]
  %y_s.1.i = phi i32 [ %y_s.3.i, %268 ], [ %y_s.6.i, %334 ]
  %x_s.1.i = phi i32 [ %x_s.3.i, %268 ], [ %x_s.6.i, %334 ]
  %steps_begin.0.i = phi i32 [ %258, %268 ], [ %324, %334 ]
  %steps_lsat.0.i = phi i32 [ %271, %268 ], [ %337, %334 ]
  %338 = fsub float 1.000000e+000, %FixedDecay.0.i
  %339 = or i32 %x_dst.0.i, %y_dst.0.i
  %340 = icmp eq i32 %339, 0
  %341 = icmp eq i32 %x.1.i, %2
  %or.cond34.i = and i1 %340, %341
  %342 = icmp eq i32 %y.1.i, %3
  %or.cond35.i = and i1 %or.cond34.i, %342
  %343 = icmp sgt i32 %steps.0.i, 0
  %344 = shl i32 %.17.i, 1
  %345 = shl i32 %.16.i, 1
  %346 = shl i32 %.19.i, 1
  %347 = shl i32 %.18.i, 1
  %notlhs = icmp ne i32 %x.1.i, %x_s.1.i
  %notrhs = icmp ne i32 %y.1.i, %y_s.1.i
  %or.cond32.not.i = or i1 %notrhs, %notlhs
  %brmerge.i = or i1 %or.cond32.not.i, %or.cond35.i
  %tmp35 = mul i32 %y.1.i, %width
  %tmp36 = add i32 %x.1.i, %tmp35
  %scevgep = getelementptr <4 x float> addrspace(1)* %output, i32 %tmp36
  %scevgep37 = bitcast <4 x float> addrspace(1)* %scevgep to i8 addrspace(1)*
  %scevgep40 = getelementptr <4 x float> addrspace(1)* %inputImage, i32 %tmp36
  %scevgep4041 = bitcast <4 x float> addrspace(1)* %scevgep40 to i8 addrspace(1)*
  br label %348

; <label>:348                                     ; preds = %._crit_edge.i, %bb.nph57.i
  %ch.056.i = phi i32 [ 0, %bb.nph57.i ], [ %397, %._crit_edge.i ]
  %tmp38 = shl i32 %ch.056.i, 2
  %uglygep42 = getelementptr i8 addrspace(1)* %scevgep4041, i32 %tmp38
  %scevgep.i = bitcast i8 addrspace(1)* %uglygep42 to float addrspace(1)*
  %349 = load float addrspace(1)* %scevgep.i, align 4
  %350 = fmul float %349, 0x3FF1C71C80000000
  %351 = fmul float %350, %338
  br i1 %brmerge.i, label %352, label %.preheader.i

; <label>:352                                     ; preds = %348
  %uglygep = getelementptr i8 addrspace(1)* %scevgep37, i32 %tmp38
  %scevgep67.i = bitcast i8 addrspace(1)* %uglygep to float addrspace(1)*
  %353 = fmul float %351, 3.000000e+000
  %354 = fadd float %353, %350
  store float %354, float addrspace(1)* %scevgep67.i, align 4
  br label %.preheader.i

.preheader.i:                                     ; preds = %352, %348
  br i1 %343, label %bb.nph.i.preheader, label %._crit_edge.i

bb.nph.i.preheader:                               ; preds = %.preheader.i
  br label %bb.nph.i

bb.nph.i:                                         ; preds = %bb.nph.i.preheader, %395
  %355 = phi i32 [ %396, %395 ], [ 0, %bb.nph.i.preheader ]
  %summ.054.i = phi float [ %384, %395 ], [ %351, %bb.nph.i.preheader ]
  %steps_begin.253.i = phi i32 [ %steps_begin.1.i, %395 ], [ %steps_begin.0.i, %bb.nph.i.preheader ]
  %x.652.i = phi i32 [ %x.5.i, %395 ], [ %x.1.i, %bb.nph.i.preheader ]
  %y.751.i = phi i32 [ %y.6.i, %395 ], [ %y.1.i, %bb.nph.i.preheader ]
  %x_s.850.i = phi i32 [ %x_s.7.i, %395 ], [ %x_s.1.i, %bb.nph.i.preheader ]
  %y_s.949.i = phi i32 [ %y_s.8.i, %395 ], [ %y_s.1.i, %bb.nph.i.preheader ]
  %di.748.i = phi i32 [ %di.6.i, %395 ], [ %di.0.i, %bb.nph.i.preheader ]
  %di_s.947.i = phi i32 [ %di_s.8.i, %395 ], [ %di_s.0.i, %bb.nph.i.preheader ]
  %356 = icmp sgt i32 %di.748.i, -1
  br i1 %82, label %360, label %357

; <label>:357                                     ; preds = %bb.nph.i
  %.op = sub i32 0, %345
  %.neg10 = select i1 %356, i32 %.op, i32 0
  %358 = select i1 %356, i32 %ystep.0.i, i32 0
  %di.5.i = add i32 %di.748.i, %344
  %359 = add nsw i32 %di.5.i, %.neg10
  br label %363

; <label>:360                                     ; preds = %bb.nph.i
  %.op16 = sub i32 0, %344
  %.neg11 = select i1 %356, i32 %.op16, i32 0
  %361 = select i1 %356, i32 %xstep.0.i, i32 0
  %di.8.i = add i32 %di.748.i, %345
  %362 = add nsw i32 %di.8.i, %.neg11
  br label %363

; <label>:363                                     ; preds = %360, %357
  %di.6.i = phi i32 [ %359, %357 ], [ %362, %360 ]
  %.pn = phi i32 [ %358, %357 ], [ %ystep.0.i, %360 ]
  %xstep.0.i.pn = phi i32 [ %xstep.0.i, %357 ], [ %361, %360 ]
  %x.5.i = add i32 %xstep.0.i.pn, %x.652.i
  %y.6.i = add i32 %.pn, %y.751.i
  %364 = icmp sgt i32 %steps_begin.253.i, -1
  br i1 %364, label %365, label %376

; <label>:365                                     ; preds = %363
  br i1 %205, label %371, label %366

; <label>:366                                     ; preds = %365
  %367 = add nsw i32 %x_s.850.i, %xstep_s.0.i
  %368 = icmp sgt i32 %di_s.947.i, -1
  %.op14 = sub i32 0, %347
  %.neg12 = select i1 %368, i32 %.op14, i32 0
  %369 = select i1 %368, i32 %ystep_s.0.i, i32 0
  %y_s.7.i = add i32 %369, %y_s.949.i
  %di_s.7.i = add i32 %di_s.947.i, %346
  %370 = add nsw i32 %di_s.7.i, %.neg12
  br label %378

; <label>:371                                     ; preds = %365
  %372 = add nsw i32 %y_s.949.i, %ystep_s.0.i
  %373 = icmp sgt i32 %di_s.947.i, -1
  %.op15 = sub i32 0, %346
  %.neg13 = select i1 %373, i32 %.op15, i32 0
  %374 = select i1 %373, i32 %xstep_s.0.i, i32 0
  %x_s.9.i = add i32 %374, %x_s.850.i
  %di_s.10.i = add i32 %di_s.947.i, %347
  %375 = add nsw i32 %di_s.10.i, %.neg13
  br label %378

; <label>:376                                     ; preds = %363
  %377 = add nsw i32 %steps_begin.253.i, 1
  br label %378

; <label>:378                                     ; preds = %376, %371, %366
  %di_s.8.i = phi i32 [ %370, %366 ], [ %375, %371 ], [ %di_s.947.i, %376 ]
  %y_s.8.i = phi i32 [ %y_s.7.i, %366 ], [ %372, %371 ], [ %y_s.949.i, %376 ]
  %x_s.7.i = phi i32 [ %367, %366 ], [ %x_s.9.i, %371 ], [ %x_s.850.i, %376 ]
  %steps_begin.1.i = phi i32 [ %steps_begin.253.i, %366 ], [ %steps_begin.253.i, %371 ], [ %377, %376 ]
  %tmp31 = mul i32 %y.6.i, %width
  %tmp32 = add i32 %x.5.i, %tmp31
  %tmp33 = shl i32 %tmp32, 2
  %tmp34 = add i32 %ch.056.i, %tmp33
  %379 = getelementptr inbounds <4 x float> addrspace(1)* %inputImage, i32 0, i32 %tmp34
  %380 = load float addrspace(1)* %379, align 4
  %381 = fmul float %summ.054.i, %FixedDecay.0.i
  %382 = fmul float %380, 0x3FF1C71C80000000
  %383 = fmul float %382, %338
  %384 = fadd float %381, %383
  %385 = icmp eq i32 %x.5.i, %x_s.7.i
  %386 = icmp eq i32 %y.6.i, %y_s.8.i
  %or.cond36.i = and i1 %385, %386
  %387 = icmp slt i32 %355, %steps_lsat.0.i
  %or.cond37.i = and i1 %or.cond36.i, %387
  br i1 %or.cond37.i, label %395, label %388

; <label>:388                                     ; preds = %378
  %389 = fmul float %384, 3.000000e+000
  br i1 %15, label %390, label %393

; <label>:390                                     ; preds = %388
  %391 = fadd float %389, %380
  %392 = getelementptr inbounds <4 x float> addrspace(1)* %output, i32 0, i32 %tmp34
  store float %391, float addrspace(1)* %392, align 4
  br label %395

; <label>:393                                     ; preds = %388
  %394 = getelementptr inbounds <4 x float> addrspace(1)* %output, i32 0, i32 %tmp34
  store float %389, float addrspace(1)* %394, align 4
  br label %395

; <label>:395                                     ; preds = %393, %390, %378
  %396 = add nsw i32 %355, 1
  %exitcond = icmp eq i32 %396, %steps.0.i
  br i1 %exitcond, label %._crit_edge.i.loopexit, label %bb.nph.i

._crit_edge.i.loopexit:                           ; preds = %395
  br label %._crit_edge.i

._crit_edge.i:                                    ; preds = %._crit_edge.i.loopexit, %.preheader.i
  %397 = add nsw i32 %ch.056.i, 1
  %exitcond17 = icmp eq i32 %397, 4
  br i1 %exitcond17, label %._crit_edge58.i, label %348

._crit_edge58.i:                                  ; preds = %._crit_edge.i
  %398 = add nsw i32 %27, 1
  %indvar.next.i = add i32 %indvar.i, 1
  br label %26

evaluateRayScalar.exit:                           ; preds = %56, %53, %48, %40, %32, %26
  ret void
}

declare float @_Z3expf(float)

declare float @_Z4sqrtf(float)

declare float @_Z13convert_floati(i32)

declare i32 @_Z3absi(i32)
