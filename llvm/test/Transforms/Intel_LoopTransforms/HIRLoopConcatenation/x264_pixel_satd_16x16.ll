; RUN: opt -xmain-opt-level=3 -hir-ssa-deconstruction -hir-temp-cleanup -hir-loop-concatenation -hir-cg -print-before=hir-loop-concatenation -print-after=hir-loop-concatenation -S 2>&1 < %s | FileCheck %s
; RUN: opt -xmain-opt-level=3 -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir-framework>,hir-loop-concatenation,print<hir-framework>,hir-cg" -S 2>&1 < %s | FileCheck %s

; Look for 16 loops with trip count of 4 before the transformation.

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
; CHECK: DO i1 = 0, 3, 1
; CHECK: DO i1 = 0, 3, 1
; CHECK: DO i1 = 0, 3, 1
; CHECK: DO i1 = 0, 3, 1
; CHECK: DO i1 = 0, 3, 1
; CHECK: DO i1 = 0, 3, 1
; CHECK: DO i1 = 0, 3, 1
; CHECK: DO i1 = 0, 3, 1
; CHECK: DO i1 = 0, 3, 1
; CHECK: DO i1 = 0, 3, 1
; CHECK: DO i1 = 0, 3, 1
; CHECK: DO i1 = 0, 3, 1


; Look for 4 loops with trip count of 16, 8, 8 and 4 after the transformation.

; CHECK: Function

; CHECK: BEGIN REGION { modified }

; Check that 1st loop is fused alloca write loop
; CHECK: DO i1 = 0, 15, 1
; CHECK: (%alloca{{.*}})[0][i1][0] =

; Check that 2nd loop is alloca initialization loop
; CHECK: DO i1 = 0, 7, 1
; CHECK: (%alloca{{.*}})[0][i1] = 0;

; Check that 3rd loop is fused alloca read loop
; CHECK: DO i1 = 0, 7, 1
; CHECK: %96 = (%alloca{{.*}})[0][0][i1];

; Check that 4th loop is reduction loop
; CHECK: DO i1 = 0, 3, 1
; CHECK: %94 = %94  +  (%alloca{{.*}})[0][i1];

; CHECK-NOT: DO i1

; Check 'inbounds' keyword on the alloca GEP after code gen.
; CHECK: getelementptr inbounds [16 x [8 x i32]], [16 x [8 x i32]]*

; Verify that the reduction loop is not unrolled by pre-vec complete unroller.
; RUN: opt -xmain-opt-level=3 -hir-ssa-deconstruction -hir-temp-cleanup -hir-loop-concatenation -hir-pre-vec-complete-unroll -print-after=hir-pre-vec-complete-unroll 2>&1 < %s | FileCheck %s -check-prefix=PREVEC_UNROLL
; R;U;N: opt -xmain-opt-level=3 -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-concatenation,hir-pre-vec-complete-unroll,print<hir-framework>" 2>&1 < %s | FileCheck %s -check-prefix=PREVEC_UNROLL

; PREVEC_UNROLL: DO i1 = 0, 3, 1

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind readonly uwtable
define hidden i32 @x264_pixel_satd_16x16(i8* nocapture readonly, i32, i8* nocapture readonly, i32) #0 {
  %5 = alloca [4 x [4 x i32]], align 16
  %6 = bitcast [4 x [4 x i32]]* %5 to i8*
  call void @llvm.lifetime.start.p0i8(i64 64, i8* nonnull %6) #2
  %7 = sext i32 %1 to i64
  %8 = sext i32 %3 to i64
  br label %9

; <label>:9:                                      ; preds = %9, %4
  %10 = phi i64 [ 0, %4 ], [ %87, %9 ]
  %11 = phi i8* [ %0, %4 ], [ %88, %9 ]
  %12 = phi i8* [ %2, %4 ], [ %89, %9 ]
  %13 = load i8, i8* %11, align 1, !tbaa !3
  %14 = zext i8 %13 to i32
  %15 = load i8, i8* %12, align 1, !tbaa !3
  %16 = zext i8 %15 to i32
  %17 = sub nsw i32 %14, %16
  %18 = getelementptr inbounds i8, i8* %11, i64 4
  %19 = load i8, i8* %18, align 1, !tbaa !3
  %20 = zext i8 %19 to i32
  %21 = getelementptr inbounds i8, i8* %12, i64 4
  %22 = load i8, i8* %21, align 1, !tbaa !3
  %23 = zext i8 %22 to i32
  %24 = sub nsw i32 %20, %23
  %25 = shl nsw i32 %24, 16
  %26 = add nsw i32 %25, %17
  %27 = getelementptr inbounds i8, i8* %11, i64 1
  %28 = load i8, i8* %27, align 1, !tbaa !3
  %29 = zext i8 %28 to i32
  %30 = getelementptr inbounds i8, i8* %12, i64 1
  %31 = load i8, i8* %30, align 1, !tbaa !3
  %32 = zext i8 %31 to i32
  %33 = sub nsw i32 %29, %32
  %34 = getelementptr inbounds i8, i8* %11, i64 5
  %35 = load i8, i8* %34, align 1, !tbaa !3
  %36 = zext i8 %35 to i32
  %37 = getelementptr inbounds i8, i8* %12, i64 5
  %38 = load i8, i8* %37, align 1, !tbaa !3
  %39 = zext i8 %38 to i32
  %40 = sub nsw i32 %36, %39
  %41 = shl nsw i32 %40, 16
  %42 = add nsw i32 %41, %33
  %43 = getelementptr inbounds i8, i8* %11, i64 2
  %44 = load i8, i8* %43, align 1, !tbaa !3
  %45 = zext i8 %44 to i32
  %46 = getelementptr inbounds i8, i8* %12, i64 2
  %47 = load i8, i8* %46, align 1, !tbaa !3
  %48 = zext i8 %47 to i32
  %49 = sub nsw i32 %45, %48
  %50 = getelementptr inbounds i8, i8* %11, i64 6
  %51 = load i8, i8* %50, align 1, !tbaa !3
  %52 = zext i8 %51 to i32
  %53 = getelementptr inbounds i8, i8* %12, i64 6
  %54 = load i8, i8* %53, align 1, !tbaa !3
  %55 = zext i8 %54 to i32
  %56 = sub nsw i32 %52, %55
  %57 = shl nsw i32 %56, 16
  %58 = add nsw i32 %57, %49
  %59 = getelementptr inbounds i8, i8* %11, i64 3
  %60 = load i8, i8* %59, align 1, !tbaa !3
  %61 = zext i8 %60 to i32
  %62 = getelementptr inbounds i8, i8* %12, i64 3
  %63 = load i8, i8* %62, align 1, !tbaa !3
  %64 = zext i8 %63 to i32
  %65 = sub nsw i32 %61, %64
  %66 = getelementptr inbounds i8, i8* %11, i64 7
  %67 = load i8, i8* %66, align 1, !tbaa !3
  %68 = zext i8 %67 to i32
  %69 = getelementptr inbounds i8, i8* %12, i64 7
  %70 = load i8, i8* %69, align 1, !tbaa !3
  %71 = zext i8 %70 to i32
  %72 = sub nsw i32 %68, %71
  %73 = shl nsw i32 %72, 16
  %74 = add nsw i32 %73, %65
  %75 = add nsw i32 %42, %26
  %76 = sub nsw i32 %26, %42
  %77 = add nsw i32 %74, %58
  %78 = sub nsw i32 %58, %74
  %79 = add nsw i32 %77, %75
  %80 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 %10, i64 0
  store i32 %79, i32* %80, align 16, !tbaa !6
  %81 = sub nsw i32 %75, %77
  %82 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 %10, i64 2
  store i32 %81, i32* %82, align 8, !tbaa !6
  %83 = add nsw i32 %78, %76
  %84 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 %10, i64 1
  store i32 %83, i32* %84, align 4, !tbaa !6
  %85 = sub nsw i32 %76, %78
  %86 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 %10, i64 3
  store i32 %85, i32* %86, align 4, !tbaa !6
  %87 = add nuw nsw i64 %10, 1
  %88 = getelementptr inbounds i8, i8* %11, i64 %7
  %89 = getelementptr inbounds i8, i8* %12, i64 %8
  %90 = icmp eq i64 %87, 4
  br i1 %90, label %91, label %9

; <label>:91:                                     ; preds = %9
  br label %92

; <label>:92:                                     ; preds = %92, %91
  %93 = phi i64 [ %135, %92 ], [ 0, %91 ]
  %94 = phi i32 [ %134, %92 ], [ 0, %91 ]
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
  %131 = add i32 %125, %94
  %132 = add i32 %131, %130
  %133 = add i32 %115, %120
  %134 = add i32 %133, %132
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
  %270 = add i32 %264, %233
  %271 = add i32 %270, %269
  %272 = add i32 %254, %259
  %273 = add i32 %272, %271
  %274 = add nuw nsw i64 %232, 1
  %275 = icmp eq i64 %274, 4
  br i1 %275, label %276, label %231

; <label>:276:                                    ; preds = %231
  %277 = phi i32 [ %273, %231 ]
  %278 = lshr i32 %230, 1
  %279 = lshr i32 %277, 16
  call void @llvm.lifetime.end.p0i8(i64 64, i8* nonnull %6) #2
  %280 = getelementptr inbounds i8, i8* %0, i64 8
  %281 = getelementptr inbounds i8, i8* %2, i64 8
  call void @llvm.lifetime.start.p0i8(i64 64, i8* nonnull %6) #2
  br label %282

; <label>:282:                                    ; preds = %282, %276
  %283 = phi i64 [ 0, %276 ], [ %360, %282 ]
  %284 = phi i8* [ %280, %276 ], [ %361, %282 ]
  %285 = phi i8* [ %281, %276 ], [ %362, %282 ]
  %286 = load i8, i8* %284, align 1, !tbaa !3
  %287 = zext i8 %286 to i32
  %288 = load i8, i8* %285, align 1, !tbaa !3
  %289 = zext i8 %288 to i32
  %290 = sub nsw i32 %287, %289
  %291 = getelementptr inbounds i8, i8* %284, i64 4
  %292 = load i8, i8* %291, align 1, !tbaa !3
  %293 = zext i8 %292 to i32
  %294 = getelementptr inbounds i8, i8* %285, i64 4
  %295 = load i8, i8* %294, align 1, !tbaa !3
  %296 = zext i8 %295 to i32
  %297 = sub nsw i32 %293, %296
  %298 = shl nsw i32 %297, 16
  %299 = add nsw i32 %298, %290
  %300 = getelementptr inbounds i8, i8* %284, i64 1
  %301 = load i8, i8* %300, align 1, !tbaa !3
  %302 = zext i8 %301 to i32
  %303 = getelementptr inbounds i8, i8* %285, i64 1
  %304 = load i8, i8* %303, align 1, !tbaa !3
  %305 = zext i8 %304 to i32
  %306 = sub nsw i32 %302, %305
  %307 = getelementptr inbounds i8, i8* %284, i64 5
  %308 = load i8, i8* %307, align 1, !tbaa !3
  %309 = zext i8 %308 to i32
  %310 = getelementptr inbounds i8, i8* %285, i64 5
  %311 = load i8, i8* %310, align 1, !tbaa !3
  %312 = zext i8 %311 to i32
  %313 = sub nsw i32 %309, %312
  %314 = shl nsw i32 %313, 16
  %315 = add nsw i32 %314, %306
  %316 = getelementptr inbounds i8, i8* %284, i64 2
  %317 = load i8, i8* %316, align 1, !tbaa !3
  %318 = zext i8 %317 to i32
  %319 = getelementptr inbounds i8, i8* %285, i64 2
  %320 = load i8, i8* %319, align 1, !tbaa !3
  %321 = zext i8 %320 to i32
  %322 = sub nsw i32 %318, %321
  %323 = getelementptr inbounds i8, i8* %284, i64 6
  %324 = load i8, i8* %323, align 1, !tbaa !3
  %325 = zext i8 %324 to i32
  %326 = getelementptr inbounds i8, i8* %285, i64 6
  %327 = load i8, i8* %326, align 1, !tbaa !3
  %328 = zext i8 %327 to i32
  %329 = sub nsw i32 %325, %328
  %330 = shl nsw i32 %329, 16
  %331 = add nsw i32 %330, %322
  %332 = getelementptr inbounds i8, i8* %284, i64 3
  %333 = load i8, i8* %332, align 1, !tbaa !3
  %334 = zext i8 %333 to i32
  %335 = getelementptr inbounds i8, i8* %285, i64 3
  %336 = load i8, i8* %335, align 1, !tbaa !3
  %337 = zext i8 %336 to i32
  %338 = sub nsw i32 %334, %337
  %339 = getelementptr inbounds i8, i8* %284, i64 7
  %340 = load i8, i8* %339, align 1, !tbaa !3
  %341 = zext i8 %340 to i32
  %342 = getelementptr inbounds i8, i8* %285, i64 7
  %343 = load i8, i8* %342, align 1, !tbaa !3
  %344 = zext i8 %343 to i32
  %345 = sub nsw i32 %341, %344
  %346 = shl nsw i32 %345, 16
  %347 = add nsw i32 %346, %338
  %348 = add nsw i32 %315, %299
  %349 = sub nsw i32 %299, %315
  %350 = add nsw i32 %347, %331
  %351 = sub nsw i32 %331, %347
  %352 = add nsw i32 %350, %348
  %353 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 %283, i64 0
  store i32 %352, i32* %353, align 16, !tbaa !6
  %354 = sub nsw i32 %348, %350
  %355 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 %283, i64 2
  store i32 %354, i32* %355, align 8, !tbaa !6
  %356 = add nsw i32 %351, %349
  %357 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 %283, i64 1
  store i32 %356, i32* %357, align 4, !tbaa !6
  %358 = sub nsw i32 %349, %351
  %359 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 %283, i64 3
  store i32 %358, i32* %359, align 4, !tbaa !6
  %360 = add nuw nsw i64 %283, 1
  %361 = getelementptr inbounds i8, i8* %284, i64 %7
  %362 = getelementptr inbounds i8, i8* %285, i64 %8
  %363 = icmp eq i64 %360, 4
  br i1 %363, label %364, label %282

; <label>:364:                                    ; preds = %282
  %365 = and i32 %277, 65535
  %366 = add nuw nsw i32 %365, %279
  br label %367

; <label>:367:                                    ; preds = %367, %364
  %368 = phi i64 [ %410, %367 ], [ 0, %364 ]
  %369 = phi i32 [ %409, %367 ], [ 0, %364 ]
  %370 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 0, i64 %368
  %371 = load i32, i32* %370, align 4, !tbaa !6
  %372 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 1, i64 %368
  %373 = load i32, i32* %372, align 4, !tbaa !6
  %374 = add i32 %373, %371
  %375 = sub i32 %371, %373
  %376 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 2, i64 %368
  %377 = load i32, i32* %376, align 4, !tbaa !6
  %378 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 3, i64 %368
  %379 = load i32, i32* %378, align 4, !tbaa !6
  %380 = add i32 %379, %377
  %381 = sub i32 %377, %379
  %382 = add nsw i32 %380, %374
  %383 = sub nsw i32 %374, %380
  %384 = add nsw i32 %381, %375
  %385 = sub nsw i32 %375, %381
  %386 = lshr i32 %382, 15
  %387 = and i32 %386, 65537
  %388 = mul nuw i32 %387, 65535
  %389 = add i32 %388, %382
  %390 = xor i32 %389, %388
  %391 = lshr i32 %384, 15
  %392 = and i32 %391, 65537
  %393 = mul nuw i32 %392, 65535
  %394 = add i32 %393, %384
  %395 = xor i32 %394, %393
  %396 = lshr i32 %383, 15
  %397 = and i32 %396, 65537
  %398 = mul nuw i32 %397, 65535
  %399 = add i32 %398, %383
  %400 = xor i32 %399, %398
  %401 = lshr i32 %385, 15
  %402 = and i32 %401, 65537
  %403 = mul nuw i32 %402, 65535
  %404 = add i32 %403, %385
  %405 = xor i32 %404, %403
  %406 = add i32 %400, %369
  %407 = add i32 %406, %405
  %408 = add i32 %390, %395
  %409 = add i32 %408, %407
  %410 = add nuw nsw i64 %368, 1
  %411 = icmp eq i64 %410, 4
  br i1 %411, label %412, label %367

; <label>:412:                                    ; preds = %367
  %413 = phi i32 [ %409, %367 ]
  %414 = lshr i32 %366, 1
  %415 = lshr i32 %413, 16
  call void @llvm.lifetime.end.p0i8(i64 64, i8* nonnull %6) #2
  %416 = getelementptr inbounds i8, i8* %280, i64 %141
  %417 = getelementptr inbounds i8, i8* %281, i64 %144
  call void @llvm.lifetime.start.p0i8(i64 64, i8* nonnull %6) #2
  br label %418

; <label>:418:                                    ; preds = %418, %412
  %419 = phi i64 [ 0, %412 ], [ %496, %418 ]
  %420 = phi i8* [ %416, %412 ], [ %497, %418 ]
  %421 = phi i8* [ %417, %412 ], [ %498, %418 ]
  %422 = load i8, i8* %420, align 1, !tbaa !3
  %423 = zext i8 %422 to i32
  %424 = load i8, i8* %421, align 1, !tbaa !3
  %425 = zext i8 %424 to i32
  %426 = sub nsw i32 %423, %425
  %427 = getelementptr inbounds i8, i8* %420, i64 4
  %428 = load i8, i8* %427, align 1, !tbaa !3
  %429 = zext i8 %428 to i32
  %430 = getelementptr inbounds i8, i8* %421, i64 4
  %431 = load i8, i8* %430, align 1, !tbaa !3
  %432 = zext i8 %431 to i32
  %433 = sub nsw i32 %429, %432
  %434 = shl nsw i32 %433, 16
  %435 = add nsw i32 %434, %426
  %436 = getelementptr inbounds i8, i8* %420, i64 1
  %437 = load i8, i8* %436, align 1, !tbaa !3
  %438 = zext i8 %437 to i32
  %439 = getelementptr inbounds i8, i8* %421, i64 1
  %440 = load i8, i8* %439, align 1, !tbaa !3
  %441 = zext i8 %440 to i32
  %442 = sub nsw i32 %438, %441
  %443 = getelementptr inbounds i8, i8* %420, i64 5
  %444 = load i8, i8* %443, align 1, !tbaa !3
  %445 = zext i8 %444 to i32
  %446 = getelementptr inbounds i8, i8* %421, i64 5
  %447 = load i8, i8* %446, align 1, !tbaa !3
  %448 = zext i8 %447 to i32
  %449 = sub nsw i32 %445, %448
  %450 = shl nsw i32 %449, 16
  %451 = add nsw i32 %450, %442
  %452 = getelementptr inbounds i8, i8* %420, i64 2
  %453 = load i8, i8* %452, align 1, !tbaa !3
  %454 = zext i8 %453 to i32
  %455 = getelementptr inbounds i8, i8* %421, i64 2
  %456 = load i8, i8* %455, align 1, !tbaa !3
  %457 = zext i8 %456 to i32
  %458 = sub nsw i32 %454, %457
  %459 = getelementptr inbounds i8, i8* %420, i64 6
  %460 = load i8, i8* %459, align 1, !tbaa !3
  %461 = zext i8 %460 to i32
  %462 = getelementptr inbounds i8, i8* %421, i64 6
  %463 = load i8, i8* %462, align 1, !tbaa !3
  %464 = zext i8 %463 to i32
  %465 = sub nsw i32 %461, %464
  %466 = shl nsw i32 %465, 16
  %467 = add nsw i32 %466, %458
  %468 = getelementptr inbounds i8, i8* %420, i64 3
  %469 = load i8, i8* %468, align 1, !tbaa !3
  %470 = zext i8 %469 to i32
  %471 = getelementptr inbounds i8, i8* %421, i64 3
  %472 = load i8, i8* %471, align 1, !tbaa !3
  %473 = zext i8 %472 to i32
  %474 = sub nsw i32 %470, %473
  %475 = getelementptr inbounds i8, i8* %420, i64 7
  %476 = load i8, i8* %475, align 1, !tbaa !3
  %477 = zext i8 %476 to i32
  %478 = getelementptr inbounds i8, i8* %421, i64 7
  %479 = load i8, i8* %478, align 1, !tbaa !3
  %480 = zext i8 %479 to i32
  %481 = sub nsw i32 %477, %480
  %482 = shl nsw i32 %481, 16
  %483 = add nsw i32 %482, %474
  %484 = add nsw i32 %451, %435
  %485 = sub nsw i32 %435, %451
  %486 = add nsw i32 %483, %467
  %487 = sub nsw i32 %467, %483
  %488 = add nsw i32 %486, %484
  %489 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 %419, i64 0
  store i32 %488, i32* %489, align 16, !tbaa !6
  %490 = sub nsw i32 %484, %486
  %491 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 %419, i64 2
  store i32 %490, i32* %491, align 8, !tbaa !6
  %492 = add nsw i32 %487, %485
  %493 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 %419, i64 1
  store i32 %492, i32* %493, align 4, !tbaa !6
  %494 = sub nsw i32 %485, %487
  %495 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 %419, i64 3
  store i32 %494, i32* %495, align 4, !tbaa !6
  %496 = add nuw nsw i64 %419, 1
  %497 = getelementptr inbounds i8, i8* %420, i64 %7
  %498 = getelementptr inbounds i8, i8* %421, i64 %8
  %499 = icmp eq i64 %496, 4
  br i1 %499, label %500, label %418

; <label>:500:                                    ; preds = %418
  %501 = and i32 %413, 65535
  %502 = add nuw nsw i32 %501, %415
  br label %503

; <label>:503:                                    ; preds = %503, %500
  %504 = phi i64 [ %546, %503 ], [ 0, %500 ]
  %505 = phi i32 [ %545, %503 ], [ 0, %500 ]
  %506 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 0, i64 %504
  %507 = load i32, i32* %506, align 4, !tbaa !6
  %508 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 1, i64 %504
  %509 = load i32, i32* %508, align 4, !tbaa !6
  %510 = add i32 %509, %507
  %511 = sub i32 %507, %509
  %512 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 2, i64 %504
  %513 = load i32, i32* %512, align 4, !tbaa !6
  %514 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 3, i64 %504
  %515 = load i32, i32* %514, align 4, !tbaa !6
  %516 = add i32 %515, %513
  %517 = sub i32 %513, %515
  %518 = add nsw i32 %516, %510
  %519 = sub nsw i32 %510, %516
  %520 = add nsw i32 %517, %511
  %521 = sub nsw i32 %511, %517
  %522 = lshr i32 %518, 15
  %523 = and i32 %522, 65537
  %524 = mul nuw i32 %523, 65535
  %525 = add i32 %524, %518
  %526 = xor i32 %525, %524
  %527 = lshr i32 %520, 15
  %528 = and i32 %527, 65537
  %529 = mul nuw i32 %528, 65535
  %530 = add i32 %529, %520
  %531 = xor i32 %530, %529
  %532 = lshr i32 %519, 15
  %533 = and i32 %532, 65537
  %534 = mul nuw i32 %533, 65535
  %535 = add i32 %534, %519
  %536 = xor i32 %535, %534
  %537 = lshr i32 %521, 15
  %538 = and i32 %537, 65537
  %539 = mul nuw i32 %538, 65535
  %540 = add i32 %539, %521
  %541 = xor i32 %540, %539
  %542 = add i32 %536, %505
  %543 = add i32 %542, %541
  %544 = add i32 %526, %531
  %545 = add i32 %544, %543
  %546 = add nuw nsw i64 %504, 1
  %547 = icmp eq i64 %546, 4
  br i1 %547, label %548, label %503

; <label>:548:                                    ; preds = %503
  %549 = phi i32 [ %545, %503 ]
  %550 = lshr i32 %502, 1
  %551 = lshr i32 %549, 16
  call void @llvm.lifetime.end.p0i8(i64 64, i8* nonnull %6) #2
  %552 = shl nsw i32 %1, 3
  %553 = sext i32 %552 to i64
  %554 = getelementptr inbounds i8, i8* %0, i64 %553
  %555 = shl nsw i32 %3, 3
  %556 = sext i32 %555 to i64
  %557 = getelementptr inbounds i8, i8* %2, i64 %556
  call void @llvm.lifetime.start.p0i8(i64 64, i8* nonnull %6) #2
  br label %558

; <label>:558:                                    ; preds = %558, %548
  %559 = phi i64 [ 0, %548 ], [ %636, %558 ]
  %560 = phi i8* [ %554, %548 ], [ %637, %558 ]
  %561 = phi i8* [ %557, %548 ], [ %638, %558 ]
  %562 = load i8, i8* %560, align 1, !tbaa !3
  %563 = zext i8 %562 to i32
  %564 = load i8, i8* %561, align 1, !tbaa !3
  %565 = zext i8 %564 to i32
  %566 = sub nsw i32 %563, %565
  %567 = getelementptr inbounds i8, i8* %560, i64 4
  %568 = load i8, i8* %567, align 1, !tbaa !3
  %569 = zext i8 %568 to i32
  %570 = getelementptr inbounds i8, i8* %561, i64 4
  %571 = load i8, i8* %570, align 1, !tbaa !3
  %572 = zext i8 %571 to i32
  %573 = sub nsw i32 %569, %572
  %574 = shl nsw i32 %573, 16
  %575 = add nsw i32 %574, %566
  %576 = getelementptr inbounds i8, i8* %560, i64 1
  %577 = load i8, i8* %576, align 1, !tbaa !3
  %578 = zext i8 %577 to i32
  %579 = getelementptr inbounds i8, i8* %561, i64 1
  %580 = load i8, i8* %579, align 1, !tbaa !3
  %581 = zext i8 %580 to i32
  %582 = sub nsw i32 %578, %581
  %583 = getelementptr inbounds i8, i8* %560, i64 5
  %584 = load i8, i8* %583, align 1, !tbaa !3
  %585 = zext i8 %584 to i32
  %586 = getelementptr inbounds i8, i8* %561, i64 5
  %587 = load i8, i8* %586, align 1, !tbaa !3
  %588 = zext i8 %587 to i32
  %589 = sub nsw i32 %585, %588
  %590 = shl nsw i32 %589, 16
  %591 = add nsw i32 %590, %582
  %592 = getelementptr inbounds i8, i8* %560, i64 2
  %593 = load i8, i8* %592, align 1, !tbaa !3
  %594 = zext i8 %593 to i32
  %595 = getelementptr inbounds i8, i8* %561, i64 2
  %596 = load i8, i8* %595, align 1, !tbaa !3
  %597 = zext i8 %596 to i32
  %598 = sub nsw i32 %594, %597
  %599 = getelementptr inbounds i8, i8* %560, i64 6
  %600 = load i8, i8* %599, align 1, !tbaa !3
  %601 = zext i8 %600 to i32
  %602 = getelementptr inbounds i8, i8* %561, i64 6
  %603 = load i8, i8* %602, align 1, !tbaa !3
  %604 = zext i8 %603 to i32
  %605 = sub nsw i32 %601, %604
  %606 = shl nsw i32 %605, 16
  %607 = add nsw i32 %606, %598
  %608 = getelementptr inbounds i8, i8* %560, i64 3
  %609 = load i8, i8* %608, align 1, !tbaa !3
  %610 = zext i8 %609 to i32
  %611 = getelementptr inbounds i8, i8* %561, i64 3
  %612 = load i8, i8* %611, align 1, !tbaa !3
  %613 = zext i8 %612 to i32
  %614 = sub nsw i32 %610, %613
  %615 = getelementptr inbounds i8, i8* %560, i64 7
  %616 = load i8, i8* %615, align 1, !tbaa !3
  %617 = zext i8 %616 to i32
  %618 = getelementptr inbounds i8, i8* %561, i64 7
  %619 = load i8, i8* %618, align 1, !tbaa !3
  %620 = zext i8 %619 to i32
  %621 = sub nsw i32 %617, %620
  %622 = shl nsw i32 %621, 16
  %623 = add nsw i32 %622, %614
  %624 = add nsw i32 %591, %575
  %625 = sub nsw i32 %575, %591
  %626 = add nsw i32 %623, %607
  %627 = sub nsw i32 %607, %623
  %628 = add nsw i32 %626, %624
  %629 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 %559, i64 0
  store i32 %628, i32* %629, align 16, !tbaa !6
  %630 = sub nsw i32 %624, %626
  %631 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 %559, i64 2
  store i32 %630, i32* %631, align 8, !tbaa !6
  %632 = add nsw i32 %627, %625
  %633 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 %559, i64 1
  store i32 %632, i32* %633, align 4, !tbaa !6
  %634 = sub nsw i32 %625, %627
  %635 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 %559, i64 3
  store i32 %634, i32* %635, align 4, !tbaa !6
  %636 = add nuw nsw i64 %559, 1
  %637 = getelementptr inbounds i8, i8* %560, i64 %7
  %638 = getelementptr inbounds i8, i8* %561, i64 %8
  %639 = icmp eq i64 %636, 4
  br i1 %639, label %640, label %558

; <label>:640:                                    ; preds = %558
  %641 = and i32 %549, 65535
  %642 = add nuw nsw i32 %641, %551
  br label %643

; <label>:643:                                    ; preds = %643, %640
  %644 = phi i64 [ %686, %643 ], [ 0, %640 ]
  %645 = phi i32 [ %685, %643 ], [ 0, %640 ]
  %646 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 0, i64 %644
  %647 = load i32, i32* %646, align 4, !tbaa !6
  %648 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 1, i64 %644
  %649 = load i32, i32* %648, align 4, !tbaa !6
  %650 = add i32 %649, %647
  %651 = sub i32 %647, %649
  %652 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 2, i64 %644
  %653 = load i32, i32* %652, align 4, !tbaa !6
  %654 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 3, i64 %644
  %655 = load i32, i32* %654, align 4, !tbaa !6
  %656 = add i32 %655, %653
  %657 = sub i32 %653, %655
  %658 = add nsw i32 %656, %650
  %659 = sub nsw i32 %650, %656
  %660 = add nsw i32 %657, %651
  %661 = sub nsw i32 %651, %657
  %662 = lshr i32 %658, 15
  %663 = and i32 %662, 65537
  %664 = mul nuw i32 %663, 65535
  %665 = add i32 %664, %658
  %666 = xor i32 %665, %664
  %667 = lshr i32 %660, 15
  %668 = and i32 %667, 65537
  %669 = mul nuw i32 %668, 65535
  %670 = add i32 %669, %660
  %671 = xor i32 %670, %669
  %672 = lshr i32 %659, 15
  %673 = and i32 %672, 65537
  %674 = mul nuw i32 %673, 65535
  %675 = add i32 %674, %659
  %676 = xor i32 %675, %674
  %677 = lshr i32 %661, 15
  %678 = and i32 %677, 65537
  %679 = mul nuw i32 %678, 65535
  %680 = add i32 %679, %661
  %681 = xor i32 %680, %679
  %682 = add i32 %676, %645
  %683 = add i32 %682, %681
  %684 = add i32 %666, %671
  %685 = add i32 %684, %683
  %686 = add nuw nsw i64 %644, 1
  %687 = icmp eq i64 %686, 4
  br i1 %687, label %688, label %643

; <label>:688:                                    ; preds = %643
  %689 = phi i32 [ %685, %643 ]
  %690 = lshr i32 %642, 1
  %691 = lshr i32 %689, 16
  call void @llvm.lifetime.end.p0i8(i64 64, i8* nonnull %6) #2
  %692 = mul nsw i32 %1, 12
  %693 = sext i32 %692 to i64
  %694 = getelementptr inbounds i8, i8* %0, i64 %693
  %695 = mul nsw i32 %3, 12
  %696 = sext i32 %695 to i64
  %697 = getelementptr inbounds i8, i8* %2, i64 %696
  call void @llvm.lifetime.start.p0i8(i64 64, i8* nonnull %6) #2
  br label %698

; <label>:698:                                    ; preds = %698, %688
  %699 = phi i64 [ 0, %688 ], [ %776, %698 ]
  %700 = phi i8* [ %694, %688 ], [ %777, %698 ]
  %701 = phi i8* [ %697, %688 ], [ %778, %698 ]
  %702 = load i8, i8* %700, align 1, !tbaa !3
  %703 = zext i8 %702 to i32
  %704 = load i8, i8* %701, align 1, !tbaa !3
  %705 = zext i8 %704 to i32
  %706 = sub nsw i32 %703, %705
  %707 = getelementptr inbounds i8, i8* %700, i64 4
  %708 = load i8, i8* %707, align 1, !tbaa !3
  %709 = zext i8 %708 to i32
  %710 = getelementptr inbounds i8, i8* %701, i64 4
  %711 = load i8, i8* %710, align 1, !tbaa !3
  %712 = zext i8 %711 to i32
  %713 = sub nsw i32 %709, %712
  %714 = shl nsw i32 %713, 16
  %715 = add nsw i32 %714, %706
  %716 = getelementptr inbounds i8, i8* %700, i64 1
  %717 = load i8, i8* %716, align 1, !tbaa !3
  %718 = zext i8 %717 to i32
  %719 = getelementptr inbounds i8, i8* %701, i64 1
  %720 = load i8, i8* %719, align 1, !tbaa !3
  %721 = zext i8 %720 to i32
  %722 = sub nsw i32 %718, %721
  %723 = getelementptr inbounds i8, i8* %700, i64 5
  %724 = load i8, i8* %723, align 1, !tbaa !3
  %725 = zext i8 %724 to i32
  %726 = getelementptr inbounds i8, i8* %701, i64 5
  %727 = load i8, i8* %726, align 1, !tbaa !3
  %728 = zext i8 %727 to i32
  %729 = sub nsw i32 %725, %728
  %730 = shl nsw i32 %729, 16
  %731 = add nsw i32 %730, %722
  %732 = getelementptr inbounds i8, i8* %700, i64 2
  %733 = load i8, i8* %732, align 1, !tbaa !3
  %734 = zext i8 %733 to i32
  %735 = getelementptr inbounds i8, i8* %701, i64 2
  %736 = load i8, i8* %735, align 1, !tbaa !3
  %737 = zext i8 %736 to i32
  %738 = sub nsw i32 %734, %737
  %739 = getelementptr inbounds i8, i8* %700, i64 6
  %740 = load i8, i8* %739, align 1, !tbaa !3
  %741 = zext i8 %740 to i32
  %742 = getelementptr inbounds i8, i8* %701, i64 6
  %743 = load i8, i8* %742, align 1, !tbaa !3
  %744 = zext i8 %743 to i32
  %745 = sub nsw i32 %741, %744
  %746 = shl nsw i32 %745, 16
  %747 = add nsw i32 %746, %738
  %748 = getelementptr inbounds i8, i8* %700, i64 3
  %749 = load i8, i8* %748, align 1, !tbaa !3
  %750 = zext i8 %749 to i32
  %751 = getelementptr inbounds i8, i8* %701, i64 3
  %752 = load i8, i8* %751, align 1, !tbaa !3
  %753 = zext i8 %752 to i32
  %754 = sub nsw i32 %750, %753
  %755 = getelementptr inbounds i8, i8* %700, i64 7
  %756 = load i8, i8* %755, align 1, !tbaa !3
  %757 = zext i8 %756 to i32
  %758 = getelementptr inbounds i8, i8* %701, i64 7
  %759 = load i8, i8* %758, align 1, !tbaa !3
  %760 = zext i8 %759 to i32
  %761 = sub nsw i32 %757, %760
  %762 = shl nsw i32 %761, 16
  %763 = add nsw i32 %762, %754
  %764 = add nsw i32 %731, %715
  %765 = sub nsw i32 %715, %731
  %766 = add nsw i32 %763, %747
  %767 = sub nsw i32 %747, %763
  %768 = add nsw i32 %766, %764
  %769 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 %699, i64 0
  store i32 %768, i32* %769, align 16, !tbaa !6
  %770 = sub nsw i32 %764, %766
  %771 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 %699, i64 2
  store i32 %770, i32* %771, align 8, !tbaa !6
  %772 = add nsw i32 %767, %765
  %773 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 %699, i64 1
  store i32 %772, i32* %773, align 4, !tbaa !6
  %774 = sub nsw i32 %765, %767
  %775 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 %699, i64 3
  store i32 %774, i32* %775, align 4, !tbaa !6
  %776 = add nuw nsw i64 %699, 1
  %777 = getelementptr inbounds i8, i8* %700, i64 %7
  %778 = getelementptr inbounds i8, i8* %701, i64 %8
  %779 = icmp eq i64 %776, 4
  br i1 %779, label %780, label %698

; <label>:780:                                    ; preds = %698
  %781 = and i32 %689, 65535
  %782 = add nuw nsw i32 %781, %691
  br label %783

; <label>:783:                                    ; preds = %783, %780
  %784 = phi i64 [ %826, %783 ], [ 0, %780 ]
  %785 = phi i32 [ %825, %783 ], [ 0, %780 ]
  %786 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 0, i64 %784
  %787 = load i32, i32* %786, align 4, !tbaa !6
  %788 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 1, i64 %784
  %789 = load i32, i32* %788, align 4, !tbaa !6
  %790 = add i32 %789, %787
  %791 = sub i32 %787, %789
  %792 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 2, i64 %784
  %793 = load i32, i32* %792, align 4, !tbaa !6
  %794 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 3, i64 %784
  %795 = load i32, i32* %794, align 4, !tbaa !6
  %796 = add i32 %795, %793
  %797 = sub i32 %793, %795
  %798 = add nsw i32 %796, %790
  %799 = sub nsw i32 %790, %796
  %800 = add nsw i32 %797, %791
  %801 = sub nsw i32 %791, %797
  %802 = lshr i32 %798, 15
  %803 = and i32 %802, 65537
  %804 = mul nuw i32 %803, 65535
  %805 = add i32 %804, %798
  %806 = xor i32 %805, %804
  %807 = lshr i32 %800, 15
  %808 = and i32 %807, 65537
  %809 = mul nuw i32 %808, 65535
  %810 = add i32 %809, %800
  %811 = xor i32 %810, %809
  %812 = lshr i32 %799, 15
  %813 = and i32 %812, 65537
  %814 = mul nuw i32 %813, 65535
  %815 = add i32 %814, %799
  %816 = xor i32 %815, %814
  %817 = lshr i32 %801, 15
  %818 = and i32 %817, 65537
  %819 = mul nuw i32 %818, 65535
  %820 = add i32 %819, %801
  %821 = xor i32 %820, %819
  %822 = add i32 %816, %785
  %823 = add i32 %822, %821
  %824 = add i32 %806, %811
  %825 = add i32 %824, %823
  %826 = add nuw nsw i64 %784, 1
  %827 = icmp eq i64 %826, 4
  br i1 %827, label %828, label %783

; <label>:828:                                    ; preds = %783
  %829 = phi i32 [ %825, %783 ]
  %830 = lshr i32 %782, 1
  %831 = lshr i32 %829, 16
  call void @llvm.lifetime.end.p0i8(i64 64, i8* nonnull %6) #2
  %832 = getelementptr inbounds i8, i8* %280, i64 %553
  %833 = getelementptr inbounds i8, i8* %281, i64 %556
  call void @llvm.lifetime.start.p0i8(i64 64, i8* nonnull %6) #2
  br label %834

; <label>:834:                                    ; preds = %834, %828
  %835 = phi i64 [ 0, %828 ], [ %912, %834 ]
  %836 = phi i8* [ %832, %828 ], [ %913, %834 ]
  %837 = phi i8* [ %833, %828 ], [ %914, %834 ]
  %838 = load i8, i8* %836, align 1, !tbaa !3
  %839 = zext i8 %838 to i32
  %840 = load i8, i8* %837, align 1, !tbaa !3
  %841 = zext i8 %840 to i32
  %842 = sub nsw i32 %839, %841
  %843 = getelementptr inbounds i8, i8* %836, i64 4
  %844 = load i8, i8* %843, align 1, !tbaa !3
  %845 = zext i8 %844 to i32
  %846 = getelementptr inbounds i8, i8* %837, i64 4
  %847 = load i8, i8* %846, align 1, !tbaa !3
  %848 = zext i8 %847 to i32
  %849 = sub nsw i32 %845, %848
  %850 = shl nsw i32 %849, 16
  %851 = add nsw i32 %850, %842
  %852 = getelementptr inbounds i8, i8* %836, i64 1
  %853 = load i8, i8* %852, align 1, !tbaa !3
  %854 = zext i8 %853 to i32
  %855 = getelementptr inbounds i8, i8* %837, i64 1
  %856 = load i8, i8* %855, align 1, !tbaa !3
  %857 = zext i8 %856 to i32
  %858 = sub nsw i32 %854, %857
  %859 = getelementptr inbounds i8, i8* %836, i64 5
  %860 = load i8, i8* %859, align 1, !tbaa !3
  %861 = zext i8 %860 to i32
  %862 = getelementptr inbounds i8, i8* %837, i64 5
  %863 = load i8, i8* %862, align 1, !tbaa !3
  %864 = zext i8 %863 to i32
  %865 = sub nsw i32 %861, %864
  %866 = shl nsw i32 %865, 16
  %867 = add nsw i32 %866, %858
  %868 = getelementptr inbounds i8, i8* %836, i64 2
  %869 = load i8, i8* %868, align 1, !tbaa !3
  %870 = zext i8 %869 to i32
  %871 = getelementptr inbounds i8, i8* %837, i64 2
  %872 = load i8, i8* %871, align 1, !tbaa !3
  %873 = zext i8 %872 to i32
  %874 = sub nsw i32 %870, %873
  %875 = getelementptr inbounds i8, i8* %836, i64 6
  %876 = load i8, i8* %875, align 1, !tbaa !3
  %877 = zext i8 %876 to i32
  %878 = getelementptr inbounds i8, i8* %837, i64 6
  %879 = load i8, i8* %878, align 1, !tbaa !3
  %880 = zext i8 %879 to i32
  %881 = sub nsw i32 %877, %880
  %882 = shl nsw i32 %881, 16
  %883 = add nsw i32 %882, %874
  %884 = getelementptr inbounds i8, i8* %836, i64 3
  %885 = load i8, i8* %884, align 1, !tbaa !3
  %886 = zext i8 %885 to i32
  %887 = getelementptr inbounds i8, i8* %837, i64 3
  %888 = load i8, i8* %887, align 1, !tbaa !3
  %889 = zext i8 %888 to i32
  %890 = sub nsw i32 %886, %889
  %891 = getelementptr inbounds i8, i8* %836, i64 7
  %892 = load i8, i8* %891, align 1, !tbaa !3
  %893 = zext i8 %892 to i32
  %894 = getelementptr inbounds i8, i8* %837, i64 7
  %895 = load i8, i8* %894, align 1, !tbaa !3
  %896 = zext i8 %895 to i32
  %897 = sub nsw i32 %893, %896
  %898 = shl nsw i32 %897, 16
  %899 = add nsw i32 %898, %890
  %900 = add nsw i32 %867, %851
  %901 = sub nsw i32 %851, %867
  %902 = add nsw i32 %899, %883
  %903 = sub nsw i32 %883, %899
  %904 = add nsw i32 %902, %900
  %905 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 %835, i64 0
  store i32 %904, i32* %905, align 16, !tbaa !6
  %906 = sub nsw i32 %900, %902
  %907 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 %835, i64 2
  store i32 %906, i32* %907, align 8, !tbaa !6
  %908 = add nsw i32 %903, %901
  %909 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 %835, i64 1
  store i32 %908, i32* %909, align 4, !tbaa !6
  %910 = sub nsw i32 %901, %903
  %911 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 %835, i64 3
  store i32 %910, i32* %911, align 4, !tbaa !6
  %912 = add nuw nsw i64 %835, 1
  %913 = getelementptr inbounds i8, i8* %836, i64 %7
  %914 = getelementptr inbounds i8, i8* %837, i64 %8
  %915 = icmp eq i64 %912, 4
  br i1 %915, label %916, label %834

; <label>:916:                                    ; preds = %834
  %917 = and i32 %829, 65535
  %918 = add nuw nsw i32 %917, %831
  br label %919

; <label>:919:                                    ; preds = %919, %916
  %920 = phi i64 [ %962, %919 ], [ 0, %916 ]
  %921 = phi i32 [ %961, %919 ], [ 0, %916 ]
  %922 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 0, i64 %920
  %923 = load i32, i32* %922, align 4, !tbaa !6
  %924 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 1, i64 %920
  %925 = load i32, i32* %924, align 4, !tbaa !6
  %926 = add i32 %925, %923
  %927 = sub i32 %923, %925
  %928 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 2, i64 %920
  %929 = load i32, i32* %928, align 4, !tbaa !6
  %930 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 3, i64 %920
  %931 = load i32, i32* %930, align 4, !tbaa !6
  %932 = add i32 %931, %929
  %933 = sub i32 %929, %931
  %934 = add nsw i32 %932, %926
  %935 = sub nsw i32 %926, %932
  %936 = add nsw i32 %933, %927
  %937 = sub nsw i32 %927, %933
  %938 = lshr i32 %934, 15
  %939 = and i32 %938, 65537
  %940 = mul nuw i32 %939, 65535
  %941 = add i32 %940, %934
  %942 = xor i32 %941, %940
  %943 = lshr i32 %936, 15
  %944 = and i32 %943, 65537
  %945 = mul nuw i32 %944, 65535
  %946 = add i32 %945, %936
  %947 = xor i32 %946, %945
  %948 = lshr i32 %935, 15
  %949 = and i32 %948, 65537
  %950 = mul nuw i32 %949, 65535
  %951 = add i32 %950, %935
  %952 = xor i32 %951, %950
  %953 = lshr i32 %937, 15
  %954 = and i32 %953, 65537
  %955 = mul nuw i32 %954, 65535
  %956 = add i32 %955, %937
  %957 = xor i32 %956, %955
  %958 = add i32 %952, %921
  %959 = add i32 %958, %957
  %960 = add i32 %942, %947
  %961 = add i32 %960, %959
  %962 = add nuw nsw i64 %920, 1
  %963 = icmp eq i64 %962, 4
  br i1 %963, label %964, label %919

; <label>:964:                                    ; preds = %919
  %965 = phi i32 [ %961, %919 ]
  %966 = lshr i32 %918, 1
  %967 = lshr i32 %965, 16
  call void @llvm.lifetime.end.p0i8(i64 64, i8* nonnull %6) #2
  %968 = getelementptr inbounds i8, i8* %280, i64 %693
  %969 = getelementptr inbounds i8, i8* %281, i64 %696
  call void @llvm.lifetime.start.p0i8(i64 64, i8* nonnull %6) #2
  br label %970

; <label>:970:                                    ; preds = %970, %964
  %971 = phi i64 [ 0, %964 ], [ %1048, %970 ]
  %972 = phi i8* [ %968, %964 ], [ %1049, %970 ]
  %973 = phi i8* [ %969, %964 ], [ %1050, %970 ]
  %974 = load i8, i8* %972, align 1, !tbaa !3
  %975 = zext i8 %974 to i32
  %976 = load i8, i8* %973, align 1, !tbaa !3
  %977 = zext i8 %976 to i32
  %978 = sub nsw i32 %975, %977
  %979 = getelementptr inbounds i8, i8* %972, i64 4
  %980 = load i8, i8* %979, align 1, !tbaa !3
  %981 = zext i8 %980 to i32
  %982 = getelementptr inbounds i8, i8* %973, i64 4
  %983 = load i8, i8* %982, align 1, !tbaa !3
  %984 = zext i8 %983 to i32
  %985 = sub nsw i32 %981, %984
  %986 = shl nsw i32 %985, 16
  %987 = add nsw i32 %986, %978
  %988 = getelementptr inbounds i8, i8* %972, i64 1
  %989 = load i8, i8* %988, align 1, !tbaa !3
  %990 = zext i8 %989 to i32
  %991 = getelementptr inbounds i8, i8* %973, i64 1
  %992 = load i8, i8* %991, align 1, !tbaa !3
  %993 = zext i8 %992 to i32
  %994 = sub nsw i32 %990, %993
  %995 = getelementptr inbounds i8, i8* %972, i64 5
  %996 = load i8, i8* %995, align 1, !tbaa !3
  %997 = zext i8 %996 to i32
  %998 = getelementptr inbounds i8, i8* %973, i64 5
  %999 = load i8, i8* %998, align 1, !tbaa !3
  %1000 = zext i8 %999 to i32
  %1001 = sub nsw i32 %997, %1000
  %1002 = shl nsw i32 %1001, 16
  %1003 = add nsw i32 %1002, %994
  %1004 = getelementptr inbounds i8, i8* %972, i64 2
  %1005 = load i8, i8* %1004, align 1, !tbaa !3
  %1006 = zext i8 %1005 to i32
  %1007 = getelementptr inbounds i8, i8* %973, i64 2
  %1008 = load i8, i8* %1007, align 1, !tbaa !3
  %1009 = zext i8 %1008 to i32
  %1010 = sub nsw i32 %1006, %1009
  %1011 = getelementptr inbounds i8, i8* %972, i64 6
  %1012 = load i8, i8* %1011, align 1, !tbaa !3
  %1013 = zext i8 %1012 to i32
  %1014 = getelementptr inbounds i8, i8* %973, i64 6
  %1015 = load i8, i8* %1014, align 1, !tbaa !3
  %1016 = zext i8 %1015 to i32
  %1017 = sub nsw i32 %1013, %1016
  %1018 = shl nsw i32 %1017, 16
  %1019 = add nsw i32 %1018, %1010
  %1020 = getelementptr inbounds i8, i8* %972, i64 3
  %1021 = load i8, i8* %1020, align 1, !tbaa !3
  %1022 = zext i8 %1021 to i32
  %1023 = getelementptr inbounds i8, i8* %973, i64 3
  %1024 = load i8, i8* %1023, align 1, !tbaa !3
  %1025 = zext i8 %1024 to i32
  %1026 = sub nsw i32 %1022, %1025
  %1027 = getelementptr inbounds i8, i8* %972, i64 7
  %1028 = load i8, i8* %1027, align 1, !tbaa !3
  %1029 = zext i8 %1028 to i32
  %1030 = getelementptr inbounds i8, i8* %973, i64 7
  %1031 = load i8, i8* %1030, align 1, !tbaa !3
  %1032 = zext i8 %1031 to i32
  %1033 = sub nsw i32 %1029, %1032
  %1034 = shl nsw i32 %1033, 16
  %1035 = add nsw i32 %1034, %1026
  %1036 = add nsw i32 %1003, %987
  %1037 = sub nsw i32 %987, %1003
  %1038 = add nsw i32 %1035, %1019
  %1039 = sub nsw i32 %1019, %1035
  %1040 = add nsw i32 %1038, %1036
  %1041 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 %971, i64 0
  store i32 %1040, i32* %1041, align 16, !tbaa !6
  %1042 = sub nsw i32 %1036, %1038
  %1043 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 %971, i64 2
  store i32 %1042, i32* %1043, align 8, !tbaa !6
  %1044 = add nsw i32 %1039, %1037
  %1045 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 %971, i64 1
  store i32 %1044, i32* %1045, align 4, !tbaa !6
  %1046 = sub nsw i32 %1037, %1039
  %1047 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 %971, i64 3
  store i32 %1046, i32* %1047, align 4, !tbaa !6
  %1048 = add nuw nsw i64 %971, 1
  %1049 = getelementptr inbounds i8, i8* %972, i64 %7
  %1050 = getelementptr inbounds i8, i8* %973, i64 %8
  %1051 = icmp eq i64 %1048, 4
  br i1 %1051, label %1052, label %970

; <label>:1052:                                   ; preds = %970
  %1053 = and i32 %965, 65535
  %1054 = add nuw nsw i32 %1053, %967
  br label %1055

; <label>:1055:                                   ; preds = %1055, %1052
  %1056 = phi i64 [ %1098, %1055 ], [ 0, %1052 ]
  %1057 = phi i32 [ %1097, %1055 ], [ 0, %1052 ]
  %1058 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 0, i64 %1056
  %1059 = load i32, i32* %1058, align 4, !tbaa !6
  %1060 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 1, i64 %1056
  %1061 = load i32, i32* %1060, align 4, !tbaa !6
  %1062 = add i32 %1061, %1059
  %1063 = sub i32 %1059, %1061
  %1064 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 2, i64 %1056
  %1065 = load i32, i32* %1064, align 4, !tbaa !6
  %1066 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 3, i64 %1056
  %1067 = load i32, i32* %1066, align 4, !tbaa !6
  %1068 = add i32 %1067, %1065
  %1069 = sub i32 %1065, %1067
  %1070 = add nsw i32 %1068, %1062
  %1071 = sub nsw i32 %1062, %1068
  %1072 = add nsw i32 %1069, %1063
  %1073 = sub nsw i32 %1063, %1069
  %1074 = lshr i32 %1070, 15
  %1075 = and i32 %1074, 65537
  %1076 = mul nuw i32 %1075, 65535
  %1077 = add i32 %1076, %1070
  %1078 = xor i32 %1077, %1076
  %1079 = lshr i32 %1072, 15
  %1080 = and i32 %1079, 65537
  %1081 = mul nuw i32 %1080, 65535
  %1082 = add i32 %1081, %1072
  %1083 = xor i32 %1082, %1081
  %1084 = lshr i32 %1071, 15
  %1085 = and i32 %1084, 65537
  %1086 = mul nuw i32 %1085, 65535
  %1087 = add i32 %1086, %1071
  %1088 = xor i32 %1087, %1086
  %1089 = lshr i32 %1073, 15
  %1090 = and i32 %1089, 65537
  %1091 = mul nuw i32 %1090, 65535
  %1092 = add i32 %1091, %1073
  %1093 = xor i32 %1092, %1091
  %1094 = add i32 %1088, %1057
  %1095 = add i32 %1094, %1093
  %1096 = add i32 %1078, %1083
  %1097 = add i32 %1096, %1095
  %1098 = add nuw nsw i64 %1056, 1
  %1099 = icmp eq i64 %1098, 4
  br i1 %1099, label %1100, label %1055

; <label>:1100:                                   ; preds = %1055
  %1101 = phi i32 [ %1097, %1055 ]
  %1102 = lshr i32 %1054, 1
  %1103 = and i32 %1101, 65535
  %1104 = lshr i32 %1101, 16
  %1105 = add nuw nsw i32 %1103, %1104
  %1106 = lshr i32 %1105, 1
  call void @llvm.lifetime.end.p0i8(i64 64, i8* nonnull %6) #2
  %1107 = add nuw nsw i32 %414, %278
  %1108 = add nuw nsw i32 %1107, %550
  %1109 = add nuw nsw i32 %1108, %690
  %1110 = add nuw nsw i32 %1109, %830
  %1111 = add nuw i32 %1110, %966
  %1112 = add nuw nsw i32 %1102, %1106
  %1113 = add i32 %1112, %1111
  ret i32 %1113
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

!0 = !{!"clang version 8.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang a53756907774b7d85a523756d285be3e3ac08d1c) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 83f888c43ae98f3186f3cabcb84faa2f86917625)"}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 1, !"ThinLTO", i32 0}
!3 = !{!4, !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!7, !9, i64 0}
!7 = !{!"array@_ZTSA4_A4_j", !8, i64 0}
!8 = !{!"array@_ZTSA4_j", !9, i64 0}
!9 = !{!"int", !4, i64 0}
