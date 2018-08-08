; RUN: opt -xmain-opt-level=3 -hir-ssa-deconstruction -hir-temp-cleanup -hir-loop-concatenation -hir-cg -print-before=hir-loop-concatenation -print-after=hir-loop-concatenation -S 2>&1 < %s | FileCheck %s
; RUN: opt -xmain-opt-level=3 -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir-framework>,hir-loop-concatenation,print<hir-framework>,hir-cg" -S 2>&1 < %s | FileCheck %s

; Look for 4 loops with trip count of 4 before the transformation.

; CHECK: Function

; CHECK: BEGIN REGION
; 1st loop is alloca write loop
; CHECK: DO i1 = 0, 3, 1
; CHECK: (%5)[0][i1][0] =

; CHECK-NOT: BEGIN REGION

; 2nd loop is alloca write loop
; CHECK: DO i1 = 0, 3, 1
; CHECK: %96 = (%5)[0][0][i1];

; CHECK: DO i1 = 0, 3, 1
; CHECK: DO i1 = 0, 3, 1


; Look for 2 loops with trip count of 8 and 4 after the transformation.

; CHECK: Function

; CHECK: BEGIN REGION { modified }

; Check that 1st loop is fused alloca write loop
; CHECK: DO i1 = 0, 7, 1
; CHECK: (%alloca{{.*}})[0][i1][0] =

; Check that 3rd loop is fused alloca read loop
; CHECK: DO i1 = 0, 3, 1
; CHECK: %96 = (%alloca{{.*}})[0][0][i1];


; CHECK-NOT: DO i

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind readonly uwtable
define hidden i32 @x264_pixel_satd_8x8(i8* nocapture readonly, i32, i8* nocapture readonly, i32) #0 {
  %5 = alloca [4 x [4 x i32]], align 16
  %6 = bitcast [4 x [4 x i32]]* %5 to i8*
  call void @llvm.lifetime.start.p0i8(i64 64, i8* nonnull %6) #2
  %7 = sext i32 %1 to i64
  %8 = sext i32 %3 to i64
  br label %10

; <label>:9:                                      ; preds = %10
  br label %92

; <label>:10:                                     ; preds = %10, %4
  %11 = phi i64 [ 0, %4 ], [ %88, %10 ]
  %12 = phi i8* [ %0, %4 ], [ %89, %10 ]
  %13 = phi i8* [ %2, %4 ], [ %90, %10 ]
  %14 = load i8, i8* %12, align 1, !tbaa !3
  %15 = zext i8 %14 to i32
  %16 = load i8, i8* %13, align 1, !tbaa !3
  %17 = zext i8 %16 to i32
  %18 = sub nsw i32 %15, %17
  %19 = getelementptr inbounds i8, i8* %12, i64 4
  %20 = load i8, i8* %19, align 1, !tbaa !3
  %21 = zext i8 %20 to i32
  %22 = getelementptr inbounds i8, i8* %13, i64 4
  %23 = load i8, i8* %22, align 1, !tbaa !3
  %24 = zext i8 %23 to i32
  %25 = sub nsw i32 %21, %24
  %26 = shl nsw i32 %25, 16
  %27 = add nsw i32 %26, %18
  %28 = getelementptr inbounds i8, i8* %12, i64 1
  %29 = load i8, i8* %28, align 1, !tbaa !3
  %30 = zext i8 %29 to i32
  %31 = getelementptr inbounds i8, i8* %13, i64 1
  %32 = load i8, i8* %31, align 1, !tbaa !3
  %33 = zext i8 %32 to i32
  %34 = sub nsw i32 %30, %33
  %35 = getelementptr inbounds i8, i8* %12, i64 5
  %36 = load i8, i8* %35, align 1, !tbaa !3
  %37 = zext i8 %36 to i32
  %38 = getelementptr inbounds i8, i8* %13, i64 5
  %39 = load i8, i8* %38, align 1, !tbaa !3
  %40 = zext i8 %39 to i32
  %41 = sub nsw i32 %37, %40
  %42 = shl nsw i32 %41, 16
  %43 = add nsw i32 %42, %34
  %44 = getelementptr inbounds i8, i8* %12, i64 2
  %45 = load i8, i8* %44, align 1, !tbaa !3
  %46 = zext i8 %45 to i32
  %47 = getelementptr inbounds i8, i8* %13, i64 2
  %48 = load i8, i8* %47, align 1, !tbaa !3
  %49 = zext i8 %48 to i32
  %50 = sub nsw i32 %46, %49
  %51 = getelementptr inbounds i8, i8* %12, i64 6
  %52 = load i8, i8* %51, align 1, !tbaa !3
  %53 = zext i8 %52 to i32
  %54 = getelementptr inbounds i8, i8* %13, i64 6
  %55 = load i8, i8* %54, align 1, !tbaa !3
  %56 = zext i8 %55 to i32
  %57 = sub nsw i32 %53, %56
  %58 = shl nsw i32 %57, 16
  %59 = add nsw i32 %58, %50
  %60 = getelementptr inbounds i8, i8* %12, i64 3
  %61 = load i8, i8* %60, align 1, !tbaa !3
  %62 = zext i8 %61 to i32
  %63 = getelementptr inbounds i8, i8* %13, i64 3
  %64 = load i8, i8* %63, align 1, !tbaa !3
  %65 = zext i8 %64 to i32
  %66 = sub nsw i32 %62, %65
  %67 = getelementptr inbounds i8, i8* %12, i64 7
  %68 = load i8, i8* %67, align 1, !tbaa !3
  %69 = zext i8 %68 to i32
  %70 = getelementptr inbounds i8, i8* %13, i64 7
  %71 = load i8, i8* %70, align 1, !tbaa !3
  %72 = zext i8 %71 to i32
  %73 = sub nsw i32 %69, %72
  %74 = shl nsw i32 %73, 16
  %75 = add nsw i32 %74, %66
  %76 = add nsw i32 %43, %27
  %77 = sub nsw i32 %27, %43
  %78 = add nsw i32 %75, %59
  %79 = sub nsw i32 %59, %75
  %80 = add nsw i32 %78, %76
  %81 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 %11, i64 0
  store i32 %80, i32* %81, align 16, !tbaa !6
  %82 = sub nsw i32 %76, %78
  %83 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 %11, i64 2
  store i32 %82, i32* %83, align 8, !tbaa !6
  %84 = add nsw i32 %79, %77
  %85 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 %11, i64 1
  store i32 %84, i32* %85, align 4, !tbaa !6
  %86 = sub nsw i32 %77, %79
  %87 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 %11, i64 3
  store i32 %86, i32* %87, align 4, !tbaa !6
  %88 = add nuw nsw i64 %11, 1
  %89 = getelementptr inbounds i8, i8* %12, i64 %7
  %90 = getelementptr inbounds i8, i8* %13, i64 %8
  %91 = icmp eq i64 %88, 4
  br i1 %91, label %9, label %10

; <label>:92:                                     ; preds = %92, %9
  %93 = phi i64 [ 0, %9 ], [ %135, %92 ]
  %94 = phi i32 [ 0, %9 ], [ %134, %92 ]
  %95 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 0, i64 %93
  %96 = load i32, i32* %95, align 4, !tbaa !6
  %97 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 1, i64 %93
  %98 = load i32, i32* %97, align 4, !tbaa !6
  %99 = add i32 %98, %96
  %100 = sub i32 %96, %98
  %101 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 2, i64 %93
  %102 = load i32, i32* %101, align 4, !tbaa !6
  %103 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 3, i64 %93
  %104 = load i32, i32* %103, align 4, !tbaa !6
  %105 = add i32 %104, %102
  %106 = sub i32 %102, %104
  %107 = add nsw i32 %105, %99
  %108 = sub nsw i32 %99, %105
  %109 = add nsw i32 %106, %100
  %110 = sub nsw i32 %100, %106
  %111 = lshr i32 %107, 15
  %112 = and i32 %111, 65537
  %113 = mul nuw i32 %112, 65535
  %114 = add i32 %113, %107
  %115 = xor i32 %114, %113
  %116 = lshr i32 %109, 15
  %117 = and i32 %116, 65537
  %118 = mul nuw i32 %117, 65535
  %119 = add i32 %118, %109
  %120 = xor i32 %119, %118
  %121 = lshr i32 %108, 15
  %122 = and i32 %121, 65537
  %123 = mul nuw i32 %122, 65535
  %124 = add i32 %123, %108
  %125 = xor i32 %124, %123
  %126 = lshr i32 %110, 15
  %127 = and i32 %126, 65537
  %128 = mul nuw i32 %127, 65535
  %129 = add i32 %128, %110
  %130 = xor i32 %129, %128
  %131 = add i32 %120, %94
  %132 = add i32 %131, %115
  %133 = add i32 %132, %125
  %134 = add i32 %133, %130
  %135 = add nuw nsw i64 %93, 1
  %136 = icmp eq i64 %135, 4
  br i1 %136, label %137, label %92

; <label>:137:                                    ; preds = %92
  %138 = phi i32 [ %134, %92 ]
  %139 = lshr i32 %138, 16
  call void @llvm.lifetime.end.p0i8(i64 64, i8* nonnull %6) #2
  %140 = shl nsw i32 %1, 2
  %141 = sext i32 %140 to i64
  %142 = getelementptr inbounds i8, i8* %0, i64 %141
  %143 = shl nsw i32 %3, 2
  %144 = sext i32 %143 to i64
  %145 = getelementptr inbounds i8, i8* %2, i64 %144
  call void @llvm.lifetime.start.p0i8(i64 64, i8* nonnull %6) #2
  br label %146

; <label>:146:                                    ; preds = %146, %137
  %147 = phi i64 [ 0, %137 ], [ %224, %146 ]
  %148 = phi i8* [ %142, %137 ], [ %225, %146 ]
  %149 = phi i8* [ %145, %137 ], [ %226, %146 ]
  %150 = load i8, i8* %148, align 1, !tbaa !3
  %151 = zext i8 %150 to i32
  %152 = load i8, i8* %149, align 1, !tbaa !3
  %153 = zext i8 %152 to i32
  %154 = sub nsw i32 %151, %153
  %155 = getelementptr inbounds i8, i8* %148, i64 4
  %156 = load i8, i8* %155, align 1, !tbaa !3
  %157 = zext i8 %156 to i32
  %158 = getelementptr inbounds i8, i8* %149, i64 4
  %159 = load i8, i8* %158, align 1, !tbaa !3
  %160 = zext i8 %159 to i32
  %161 = sub nsw i32 %157, %160
  %162 = shl nsw i32 %161, 16
  %163 = add nsw i32 %162, %154
  %164 = getelementptr inbounds i8, i8* %148, i64 1
  %165 = load i8, i8* %164, align 1, !tbaa !3
  %166 = zext i8 %165 to i32
  %167 = getelementptr inbounds i8, i8* %149, i64 1
  %168 = load i8, i8* %167, align 1, !tbaa !3
  %169 = zext i8 %168 to i32
  %170 = sub nsw i32 %166, %169
  %171 = getelementptr inbounds i8, i8* %148, i64 5
  %172 = load i8, i8* %171, align 1, !tbaa !3
  %173 = zext i8 %172 to i32
  %174 = getelementptr inbounds i8, i8* %149, i64 5
  %175 = load i8, i8* %174, align 1, !tbaa !3
  %176 = zext i8 %175 to i32
  %177 = sub nsw i32 %173, %176
  %178 = shl nsw i32 %177, 16
  %179 = add nsw i32 %178, %170
  %180 = getelementptr inbounds i8, i8* %148, i64 2
  %181 = load i8, i8* %180, align 1, !tbaa !3
  %182 = zext i8 %181 to i32
  %183 = getelementptr inbounds i8, i8* %149, i64 2
  %184 = load i8, i8* %183, align 1, !tbaa !3
  %185 = zext i8 %184 to i32
  %186 = sub nsw i32 %182, %185
  %187 = getelementptr inbounds i8, i8* %148, i64 6
  %188 = load i8, i8* %187, align 1, !tbaa !3
  %189 = zext i8 %188 to i32
  %190 = getelementptr inbounds i8, i8* %149, i64 6
  %191 = load i8, i8* %190, align 1, !tbaa !3
  %192 = zext i8 %191 to i32
  %193 = sub nsw i32 %189, %192
  %194 = shl nsw i32 %193, 16
  %195 = add nsw i32 %194, %186
  %196 = getelementptr inbounds i8, i8* %148, i64 3
  %197 = load i8, i8* %196, align 1, !tbaa !3
  %198 = zext i8 %197 to i32
  %199 = getelementptr inbounds i8, i8* %149, i64 3
  %200 = load i8, i8* %199, align 1, !tbaa !3
  %201 = zext i8 %200 to i32
  %202 = sub nsw i32 %198, %201
  %203 = getelementptr inbounds i8, i8* %148, i64 7
  %204 = load i8, i8* %203, align 1, !tbaa !3
  %205 = zext i8 %204 to i32
  %206 = getelementptr inbounds i8, i8* %149, i64 7
  %207 = load i8, i8* %206, align 1, !tbaa !3
  %208 = zext i8 %207 to i32
  %209 = sub nsw i32 %205, %208
  %210 = shl nsw i32 %209, 16
  %211 = add nsw i32 %210, %202
  %212 = add nsw i32 %179, %163
  %213 = sub nsw i32 %163, %179
  %214 = add nsw i32 %211, %195
  %215 = sub nsw i32 %195, %211
  %216 = add nsw i32 %214, %212
  %217 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 %147, i64 0
  store i32 %216, i32* %217, align 16, !tbaa !6
  %218 = sub nsw i32 %212, %214
  %219 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 %147, i64 2
  store i32 %218, i32* %219, align 8, !tbaa !6
  %220 = add nsw i32 %215, %213
  %221 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 %147, i64 1
  store i32 %220, i32* %221, align 4, !tbaa !6
  %222 = sub nsw i32 %213, %215
  %223 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 %147, i64 3
  store i32 %222, i32* %223, align 4, !tbaa !6
  %224 = add nuw nsw i64 %147, 1
  %225 = getelementptr inbounds i8, i8* %148, i64 %7
  %226 = getelementptr inbounds i8, i8* %149, i64 %8
  %227 = icmp eq i64 %224, 4
  br i1 %227, label %228, label %146

; <label>:228:                                    ; preds = %146
  %229 = and i32 %138, 65535
  %230 = add nuw nsw i32 %229, %139
  br label %231

; <label>:231:                                    ; preds = %231, %228
  %232 = phi i64 [ %274, %231 ], [ 0, %228 ]
  %233 = phi i32 [ %273, %231 ], [ 0, %228 ]
  %234 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 0, i64 %232
  %235 = load i32, i32* %234, align 4, !tbaa !6
  %236 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 1, i64 %232
  %237 = load i32, i32* %236, align 4, !tbaa !6
  %238 = add i32 %237, %235
  %239 = sub i32 %235, %237
  %240 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 2, i64 %232
  %241 = load i32, i32* %240, align 4, !tbaa !6
  %242 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 3, i64 %232
  %243 = load i32, i32* %242, align 4, !tbaa !6
  %244 = add i32 %243, %241
  %245 = sub i32 %241, %243
  %246 = add nsw i32 %244, %238
  %247 = sub nsw i32 %238, %244
  %248 = add nsw i32 %245, %239
  %249 = sub nsw i32 %239, %245
  %250 = lshr i32 %246, 15
  %251 = and i32 %250, 65537
  %252 = mul nuw i32 %251, 65535
  %253 = add i32 %252, %246
  %254 = xor i32 %253, %252
  %255 = lshr i32 %248, 15
  %256 = and i32 %255, 65537
  %257 = mul nuw i32 %256, 65535
  %258 = add i32 %257, %248
  %259 = xor i32 %258, %257
  %260 = lshr i32 %247, 15
  %261 = and i32 %260, 65537
  %262 = mul nuw i32 %261, 65535
  %263 = add i32 %262, %247
  %264 = xor i32 %263, %262
  %265 = lshr i32 %249, 15
  %266 = and i32 %265, 65537
  %267 = mul nuw i32 %266, 65535
  %268 = add i32 %267, %249
  %269 = xor i32 %268, %267
  %270 = add i32 %259, %233
  %271 = add i32 %270, %254
  %272 = add i32 %271, %264
  %273 = add i32 %272, %269
  %274 = add nuw nsw i64 %232, 1
  %275 = icmp eq i64 %274, 4
  br i1 %275, label %276, label %231

; <label>:276:                                    ; preds = %231
  %277 = phi i32 [ %273, %231 ]
  %278 = lshr i32 %230, 1
  %279 = and i32 %277, 65535
  %280 = lshr i32 %277, 16
  %281 = add nuw nsw i32 %279, %280
  %282 = lshr i32 %281, 1
  call void @llvm.lifetime.end.p0i8(i64 64, i8* nonnull %6) #2
  %283 = add nuw nsw i32 %282, %278
  ret i32 %283
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

attributes #0 = { nounwind readonly uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+aes,+avx,+avx2,+bmi,+bmi2,+cx16,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind }

!llvm.ident = !{!0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0}
!llvm.module.flags = !{!1, !2}

!0 = !{!"clang version 7.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 2a39b36f03a53f7ce6d2d38d930104b1873f7bdb) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 48e165e3a8b82bc93f11f8cfa317f62ebaa35efe)"}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 1, !"ThinLTO", i32 0}
!3 = !{!4, !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!7, !9, i64 0}
!7 = !{!"array@_ZTSA4_A4_j", !8, i64 0}
!8 = !{!"array@_ZTSA4_j", !9, i64 0}
!9 = !{!"int", !4, i64 0}
