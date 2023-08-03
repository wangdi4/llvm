; RUN: opt -passes="gvn" %s -S | FileCheck %s

; CMPLRLLVM-10341 describes a PRE of loads in 557.xz, that must happen in
; block 121 below.
;
; The loads are available in 1 incoming edge, and unavailable in the other 3.
; We create a new block, copy the loads to that block, and connect the block
; into the 3 unavailable edges.
;
; This optimization was implemented previously but failed after a commit in
; jump threading (D110290) changed the incoming number of CFG edges and
; unavailable blocks.
;
; The test is somewhat large, because we want to make sure the new cost model
; actually kicks in for the real xz function.

; This is the new block with the PRE forced reloads.
; CHECK-LABEL: .split
; CHECK: [[PHI:%.*]] = phi i64
; CHECK: [[PHITRANS:%.*]] = getelementptr {{.*}} i64 %.ph
; CHECK: [[PRE:%.*]] = load i8, ptr [[PHITRANS]]
; CHECK: [[PHITRANS4:%.*]] = getelementptr {{.*}} i64 %.ph
; CHECK: [[PRE5:%.*]] = load i8, ptr [[PHITRANS4]]

; This is the source block where the loads were removed from.
; CHECK-LABEL: 113
; CHECK: phi {{.*}} [[PRE5]], %.split
; CHECK: phi {{.*}} [[PRE]], %.split
; CHECK-NOT: load
; CHECK-LABEL: 123

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.lzma_mf_s = type { ptr, i32, i32, i32, i32, i32, i32, i32, i32, i32, ptr, ptr, ptr, ptr, i32, i32, i32, i32, i32, i32, i32, i32, i32 }
%struct.lzma_match = type { i32, i32 }

@__Intel_PaddedMallocCounter = external hidden unnamed_addr global i32
@0 = external hidden unnamed_addr constant [16 x i8], align 1
@1 = external hidden unnamed_addr constant [10 x i8], align 1

; Function Attrs: nofree nosync nounwind uwtable
define hidden i32 @lzma_mf_bt2_find(ptr nocapture %0, ptr %1) #0 {
  %3 = getelementptr %struct.lzma_mf_s, ptr %0, i64 0, i32 5
  %4 = load i32, ptr %3, align 8, !tbaa !7
  %5 = getelementptr %struct.lzma_mf_s, ptr %0, i64 0, i32 8
  %6 = load i32, ptr %5, align 4, !tbaa !16
  %7 = sub i32 %6, %4
  %8 = getelementptr inbounds %struct.lzma_mf_s, ptr %0, i64 0, i32 18, !intel-tbaa !17
  %9 = load i32, ptr %8, align 8, !tbaa !17
  %10 = icmp ugt i32 %9, %7
  br i1 %10, label %11, label %22

11:                                               ; preds = %2
  %12 = icmp ult i32 %7, 2
  br i1 %12, label %17, label %13

13:                                               ; preds = %11
  %14 = getelementptr inbounds %struct.lzma_mf_s, ptr %0, i64 0, i32 20, !intel-tbaa !18
  %15 = load i32, ptr %14, align 8, !tbaa !18
  %16 = icmp eq i32 %15, 1
  br i1 %16, label %17, label %22

17:                                               ; preds = %13, %11
  %18 = add i32 %4, 1
  store i32 %18, ptr %3, align 8, !tbaa !7
  %19 = getelementptr inbounds %struct.lzma_mf_s, ptr %0, i64 0, i32 9, !intel-tbaa !19
  %20 = load i32, ptr %19, align 8, !tbaa !19
  %21 = add i32 %20, 1
  store i32 %21, ptr %19, align 8, !tbaa !19
  br label %180

22:                                               ; preds = %13, %2
  %23 = phi i32 [ %7, %13 ], [ %9, %2 ]
  %24 = getelementptr %struct.lzma_mf_s, ptr %0, i64 0, i32 0
  %25 = load ptr, ptr %24, align 8, !tbaa !20
  %26 = tail call ptr @llvm.ptr.annotation.p0.p0(ptr %25, ptr @0, ptr @1, i32 0, ptr null)
  %27 = zext i32 %4 to i64
  %28 = getelementptr inbounds i8, ptr %26, i64 %27, !intel-tbaa !21
  %29 = getelementptr inbounds %struct.lzma_mf_s, ptr %0, i64 0, i32 4, !intel-tbaa !22
  %30 = load i32, ptr %29, align 4, !tbaa !22
  %31 = add i32 %30, %4
  %32 = load i8, ptr %28, align 1, !tbaa !21
  %33 = zext i8 %32 to i64
  %34 = getelementptr inbounds i8, ptr %28, i64 1
  %35 = load i8, ptr %34, align 1, !tbaa !21
  %36 = zext i8 %35 to i64
  %37 = shl nuw nsw i64 %36, 8
  %38 = or i64 %37, %33
  %39 = getelementptr inbounds %struct.lzma_mf_s, ptr %0, i64 0, i32 12, !intel-tbaa !23
  %40 = load ptr, ptr %39, align 8, !tbaa !23
  %41 = getelementptr inbounds i32, ptr %40, i64 %38
  %42 = load i32, ptr %41, align 4, !tbaa !24
  store i32 %31, ptr %41, align 4, !tbaa !24
  %43 = getelementptr inbounds %struct.lzma_mf_s, ptr %0, i64 0, i32 17, !intel-tbaa !25
  %44 = load i32, ptr %43, align 4, !tbaa !25
  %45 = getelementptr inbounds %struct.lzma_mf_s, ptr %0, i64 0, i32 13, !intel-tbaa !26
  %46 = load ptr, ptr %45, align 8, !tbaa !26
  %47 = getelementptr inbounds %struct.lzma_mf_s, ptr %0, i64 0, i32 14, !intel-tbaa !27
  %48 = load i32, ptr %47, align 8, !tbaa !27
  %49 = getelementptr inbounds %struct.lzma_mf_s, ptr %0, i64 0, i32 15, !intel-tbaa !28
  %50 = load i32, ptr %49, align 4, !tbaa !28
  %51 = shl i32 %48, 1
  %52 = zext i32 %51 to i64
  %53 = getelementptr inbounds i32, ptr %46, i64 %52, !intel-tbaa !24
  %54 = getelementptr inbounds i32, ptr %53, i64 1, !intel-tbaa !24
  %55 = sub i32 %31, %42
  %56 = icmp ne i32 %44, 0
  %57 = icmp ult i32 %55, %50
  %58 = select i1 %56, i1 %57, i1 false
  br i1 %58, label %59, label %61

59:                                               ; preds = %22
  %60 = zext i32 %23 to i64
  br label %65

61:                                               ; preds = %134, %22
  %62 = phi ptr [ %1, %22 ], [ %123, %134 ]
  %63 = phi ptr [ %54, %22 ], [ %135, %134 ]
  %64 = phi ptr [ %53, %22 ], [ %136, %134 ]
  store i32 0, ptr %63, align 4, !tbaa !24
  store i32 0, ptr %64, align 4, !tbaa !24
  br label %145

65:                                               ; preds = %134, %59
  %66 = phi i32 [ %75, %134 ], [ %44, %59 ]
  %67 = phi i32 [ %141, %134 ], [ %55, %59 ]
  %68 = phi i32 [ %140, %134 ], [ %42, %59 ]
  %69 = phi i32 [ %138, %134 ], [ 0, %59 ]
  %70 = phi i32 [ %137, %134 ], [ 0, %59 ]
  %71 = phi ptr [ %136, %134 ], [ %53, %59 ]
  %72 = phi ptr [ %135, %134 ], [ %54, %59 ]
  %73 = phi i32 [ %124, %134 ], [ 1, %59 ]
  %74 = phi ptr [ %123, %134 ], [ %1, %59 ]
  %75 = add i32 %66, -1
  %76 = sub i32 %48, %67
  %77 = icmp ult i32 %48, %67
  %78 = select i1 %77, i32 %50, i32 0
  %79 = add i32 %76, %78
  %80 = shl i32 %79, 1
  %81 = zext i32 %80 to i64
  %82 = getelementptr inbounds i32, ptr %46, i64 %81, !intel-tbaa !24
  %83 = zext i32 %67 to i64
  %84 = sub nsw i64 0, %83
  %85 = getelementptr inbounds i8, ptr %28, i64 %84, !intel-tbaa !21
  %86 = icmp ult i32 %70, %69
  %87 = select i1 %86, i32 %70, i32 %69
  %88 = zext i32 %87 to i64
  %89 = getelementptr inbounds i8, ptr %85, i64 %88
  %90 = load i8, ptr %89, align 1, !tbaa !21
  %91 = getelementptr inbounds i8, ptr %28, i64 %88
  %92 = load i8, ptr %91, align 1, !tbaa !21
  %93 = icmp eq i8 %90, %92
  br i1 %93, label %94, label %121

94:                                               ; preds = %65
  %95 = add i32 %87, 1
  %96 = icmp eq i32 %95, %23
  br i1 %96, label %97, label %182

97:                                               ; preds = %265, %238, %94
  %98 = icmp ult i32 %73, %23
  br i1 %98, label %108, label %121

99:                                               ; preds = %261, %225, %194
  %100 = phi i32 [ %195, %194 ], [ %229, %225 ], [ %262, %261 ]
  %101 = phi i64 [ %197, %194 ], [ %233, %225 ], [ %264, %261 ]
  %102 = icmp ult i32 %73, %100
  br i1 %102, label %103, label %121

103:                                              ; preds = %99
  %104 = getelementptr inbounds %struct.lzma_match, ptr %74, i64 0, i32 0, !intel-tbaa !29
  store i32 %100, ptr %104, align 4, !tbaa !29
  %105 = add i32 %67, -1
  %106 = getelementptr inbounds %struct.lzma_match, ptr %74, i64 0, i32 1, !intel-tbaa !31
  store i32 %105, ptr %106, align 4, !tbaa !31
  %107 = getelementptr inbounds %struct.lzma_match, ptr %74, i64 1
  br label %121

108:                                              ; preds = %97
  %109 = phi i32 [ %67, %97 ]
  %110 = phi ptr [ %71, %97 ]
  %111 = phi ptr [ %72, %97 ]
  %112 = phi ptr [ %74, %97 ]
  %113 = phi ptr [ %82, %97 ]
  %114 = getelementptr inbounds %struct.lzma_match, ptr %112, i64 0, i32 0, !intel-tbaa !29
  store i32 %23, ptr %114, align 4, !tbaa !29
  %115 = add i32 %109, -1
  %116 = getelementptr inbounds %struct.lzma_match, ptr %112, i64 0, i32 1, !intel-tbaa !31
  store i32 %115, ptr %116, align 4, !tbaa !31
  %117 = getelementptr inbounds %struct.lzma_match, ptr %112, i64 1
  %118 = load i32, ptr %113, align 4, !tbaa !24
  store i32 %118, ptr %110, align 4, !tbaa !24
  %119 = getelementptr inbounds i32, ptr %113, i64 1
  %120 = load i32, ptr %119, align 4, !tbaa !24
  store i32 %120, ptr %111, align 4, !tbaa !24
  br label %145

121:                                              ; preds = %103, %99, %97, %65
  %122 = phi i64 [ %101, %103 ], [ %101, %99 ], [ %88, %65 ], [ %60, %97 ]
  %123 = phi ptr [ %107, %103 ], [ %74, %99 ], [ %74, %65 ], [ %74, %97 ]
  %124 = phi i32 [ %100, %103 ], [ %73, %99 ], [ %73, %65 ], [ %73, %97 ]
  %125 = phi i32 [ %100, %103 ], [ %100, %99 ], [ %87, %65 ], [ %23, %97 ]
  %126 = getelementptr inbounds i8, ptr %85, i64 %122
  %127 = load i8, ptr %126, align 1, !tbaa !21
  %128 = getelementptr inbounds i8, ptr %28, i64 %122
  %129 = load i8, ptr %128, align 1, !tbaa !21
  %130 = icmp ult i8 %127, %129
  br i1 %130, label %131, label %133

131:                                              ; preds = %121
  store i32 %68, ptr %71, align 4, !tbaa !24
  %132 = getelementptr inbounds i32, ptr %82, i64 1, !intel-tbaa !24
  br label %134

133:                                              ; preds = %121
  store i32 %68, ptr %72, align 4, !tbaa !24
  br label %134

134:                                              ; preds = %133, %131
  %135 = phi ptr [ %72, %131 ], [ %82, %133 ]
  %136 = phi ptr [ %132, %131 ], [ %71, %133 ]
  %137 = phi i32 [ %70, %131 ], [ %125, %133 ]
  %138 = phi i32 [ %125, %131 ], [ %69, %133 ]
  %139 = phi ptr [ %132, %131 ], [ %82, %133 ]
  %140 = load i32, ptr %139, align 4, !tbaa !24
  %141 = sub i32 %31, %140
  %142 = icmp ne i32 %75, 0
  %143 = icmp ult i32 %141, %50
  %144 = select i1 %142, i1 %143, i1 false
  br i1 %144, label %65, label %61

145:                                              ; preds = %108, %61
  %146 = phi ptr [ %117, %108 ], [ %62, %61 ]
  %147 = ptrtoint ptr %146 to i64
  %148 = ptrtoint ptr %1 to i64
  %149 = sub i64 %147, %148
  %150 = lshr exact i64 %149, 3
  %151 = trunc i64 %150 to i32
  %152 = load i32, ptr %47, align 8, !tbaa !27
  %153 = add i32 %152, 1
  %154 = load i32, ptr %49, align 4, !tbaa !28
  %155 = icmp eq i32 %153, %154
  %156 = select i1 %155, i32 0, i32 %153
  store i32 %156, ptr %47, align 8
  %157 = load i32, ptr %3, align 8, !tbaa !7
  %158 = add i32 %157, 1
  store i32 %158, ptr %3, align 8, !tbaa !7
  %159 = load i32, ptr %29, align 4, !tbaa !22
  %160 = add i32 %159, %158
  %161 = icmp eq i32 %160, -1
  br i1 %161, label %162, label %180

162:                                              ; preds = %145
  %163 = xor i32 %154, -1
  %164 = getelementptr inbounds %struct.lzma_mf_s, ptr %0, i64 0, i32 21, !intel-tbaa !32
  %165 = load i32, ptr %164, align 4, !tbaa !32
  %166 = getelementptr inbounds %struct.lzma_mf_s, ptr %0, i64 0, i32 22, !intel-tbaa !33
  %167 = load i32, ptr %166, align 8, !tbaa !33
  %168 = add i32 %167, %165
  %169 = icmp eq i32 %168, 0
  br i1 %169, label %177, label %170

170:                                              ; preds = %162
  %171 = zext i32 %168 to i64
  %172 = add i32 %167, %165
  %173 = zext i32 %172 to i64
  %174 = udiv i64 %173, 8
  %175 = shl i64 %174, 3
  %176 = icmp ult i64 0, %175
  br i1 %176, label %268, label %292

177:                                              ; preds = %313, %162
  %178 = phi i32 [ %314, %313 ], [ %159, %162 ]
  %179 = sub i32 %178, %163
  store i32 %179, ptr %29, align 4, !tbaa !22
  br label %180

180:                                              ; preds = %177, %145, %17
  %181 = phi i32 [ 0, %17 ], [ %151, %145 ], [ %151, %177 ]
  ret i32 %181

182:                                              ; preds = %94
  %183 = load i32, ptr @__Intel_PaddedMallocCounter, align 4
  %184 = icmp ult i32 %183, 250
  %185 = add i32 %87, 1
  %186 = zext i32 %185 to i64
  %187 = getelementptr inbounds i8, ptr %85, i64 %186
  %188 = load i8, ptr %187, align 1, !tbaa !21
  %189 = add i32 %87, 1
  %190 = zext i32 %189 to i64
  %191 = getelementptr inbounds i8, ptr %28, i64 %190
  %192 = load i8, ptr %191, align 1, !tbaa !21
  %193 = icmp ne i8 %188, %192
  br i1 %193, label %194, label %198

194:                                              ; preds = %182
  %195 = add i32 %87, 1
  %196 = add i32 %87, 1
  %197 = zext i32 %196 to i64
  br label %99

198:                                              ; preds = %182
  %199 = icmp ne i1 %184, false
  br i1 %199, label %200, label %237

200:                                              ; preds = %198
  %201 = sub i32 0, %87
  %202 = add i32 %201, %23
  %203 = add i32 %202, -1
  %204 = udiv i32 %203, 32
  %205 = shl i32 %204, 5
  %206 = icmp ult i32 0, %205
  br i1 %206, label %207, label %238

207:                                              ; preds = %200
  %208 = shl i32 %204, 5
  %209 = add i32 %208, -1
  br label %210

210:                                              ; preds = %234, %207
  %211 = phi i32 [ 0, %207 ], [ %235, %234 ]
  %212 = add i32 %189, %211
  %213 = zext i32 %212 to i64
  %214 = getelementptr inbounds i8, ptr %85, i64 %213
  %215 = bitcast ptr %214 to ptr
  %216 = load <32 x i8>, ptr %215, align 1, !tbaa !21
  %217 = add i32 %189, %211
  %218 = zext i32 %217 to i64
  %219 = getelementptr inbounds i8, ptr %28, i64 %218
  %220 = bitcast ptr %219 to ptr
  %221 = load <32 x i8>, ptr %220, align 1, !tbaa !21
  %222 = icmp ne <32 x i8> %216, %221
  %223 = bitcast <32 x i1> %222 to i32
  %224 = icmp ne i32 %223, 0
  br i1 %224, label %225, label %234

225:                                              ; preds = %210
  %226 = xor <32 x i1> %222, <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  %227 = bitcast <32 x i1> %222 to i32
  %228 = call i32 @llvm.cttz.i32(i32 %227, i1 false)
  %229 = add i32 %217, %228
  %230 = bitcast <32 x i1> %222 to i32
  %231 = call i32 @llvm.cttz.i32(i32 %230, i1 false)
  %232 = add i32 %217, %231
  %233 = zext i32 %232 to i64
  br label %99

234:                                              ; preds = %210
  %235 = add nuw i32 %211, 32
  %236 = icmp ule i32 %235, %209
  br i1 %236, label %210, label %238, !llvm.loop !34

237:                                              ; preds = %198
  br label %238

238:                                              ; preds = %237, %234, %200
  %239 = phi i32 [ %204, %234 ], [ %204, %200 ], [ 0, %237 ]
  %240 = shl i32 %239, 5
  %241 = sub i32 0, %87
  %242 = add i32 %241, %23
  %243 = add i32 %242, -1
  %244 = icmp ult i32 %240, %243
  br i1 %244, label %245, label %97

245:                                              ; preds = %238
  %246 = shl i32 %239, 5
  %247 = sub i32 0, %87
  %248 = add i32 %247, %23
  %249 = add i32 %248, -2
  br label %250

250:                                              ; preds = %265, %245
  %251 = phi i32 [ %246, %245 ], [ %266, %265 ]
  %252 = add i32 %189, %251
  %253 = zext i32 %252 to i64
  %254 = getelementptr inbounds i8, ptr %85, i64 %253
  %255 = load i8, ptr %254, align 1, !tbaa !21
  %256 = add i32 %189, %251
  %257 = zext i32 %256 to i64
  %258 = getelementptr inbounds i8, ptr %28, i64 %257
  %259 = load i8, ptr %258, align 1, !tbaa !21
  %260 = icmp ne i8 %255, %259
  br i1 %260, label %261, label %265

261:                                              ; preds = %250
  %262 = add i32 %189, %251
  %263 = add i32 %189, %251
  %264 = zext i32 %263 to i64
  br label %99

265:                                              ; preds = %250
  %266 = add nuw i32 %251, 1
  %267 = icmp ne i32 %251, %249
  br i1 %267, label %250, label %97, !llvm.loop !39

268:                                              ; preds = %170
  %269 = shl i64 %174, 3
  %270 = add i64 %269, -1
  br label %271

271:                                              ; preds = %271, %268
  %272 = phi i64 [ 0, %268 ], [ %290, %271 ]
  %273 = getelementptr inbounds i32, ptr %40, i64 %272
  %274 = bitcast ptr %273 to ptr
  %275 = load <8 x i32>, ptr %274, align 4, !tbaa !24
  %276 = sub i32 0, %154
  %277 = add i32 %276, -1
  %278 = insertelement <8 x i32> poison, i32 %277, i32 0
  %279 = shufflevector <8 x i32> %278, <8 x i32> poison, <8 x i32> zeroinitializer
  %280 = sub i32 0, %154
  %281 = add i32 %280, -1
  %282 = insertelement <8 x i32> poison, i32 %281, i32 0
  %283 = shufflevector <8 x i32> %282, <8 x i32> poison, <8 x i32> zeroinitializer
  %284 = icmp ult <8 x i32> %275, %279
  %285 = select <8 x i1> %284, <8 x i32> %275, <8 x i32> %283
  %286 = mul <8 x i32> %285, <i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1>
  %287 = getelementptr inbounds i32, ptr %40, i64 %272
  %288 = bitcast ptr %287 to ptr
  %289 = add <8 x i32> %275, %286
  store <8 x i32> %289, ptr %288, align 4, !tbaa !24
  %290 = add nuw nsw i64 %272, 8
  %291 = icmp sle i64 %290, %270
  br i1 %291, label %271, label %292, !llvm.loop !40

292:                                              ; preds = %271, %170
  %293 = shl i64 %174, 3
  %294 = add i32 %167, %165
  %295 = zext i32 %294 to i64
  %296 = icmp ult i64 %293, %295
  br i1 %296, label %297, label %313

297:                                              ; preds = %292
  %298 = shl i64 %174, 3
  %299 = add i32 %167, %165
  %300 = zext i32 %299 to i64
  %301 = add i64 %300, -1
  br label %302

302:                                              ; preds = %302, %297
  %303 = phi i64 [ %298, %297 ], [ %311, %302 ]
  %304 = getelementptr inbounds i32, ptr %40, i64 %303
  %305 = load i32, ptr %304, align 4, !tbaa !24
  %306 = getelementptr inbounds i32, ptr %40, i64 %303
  %307 = sub i32 -1, %154
  %308 = call i32 @llvm.umin.i32(i32 %305, i32 %307)
  %309 = sub i32 0, %308
  %310 = add i32 %305, %309
  store i32 %310, ptr %306, align 4, !tbaa !24
  %311 = add nuw nsw i64 %303, 1
  %312 = icmp ne i64 %303, %301
  br i1 %312, label %302, label %313, !llvm.loop !41

313:                                              ; preds = %302, %292
  %314 = load i32, ptr %29, align 4, !tbaa !22
  br label %177
}

; This function must exist, for the GVN xz code model to be enabled.

define i1 @__Intel_PaddedMallocInterface() local_unnamed_addr !dtrans.paddedmallocsize !43 {
  %1 = load i32, ptr @__Intel_PaddedMallocCounter, align 4
  %2 = icmp ult i32 %1, 250
  ret i1 %2
}

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i32 @llvm.umin.i32(i32, i32) #1

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i32 @llvm.cttz.i32(i32, i1 immarg) #1

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: readwrite)
declare ptr @llvm.ptr.annotation.p0.p0(ptr, ptr, ptr, i32, ptr) #2

attributes #0 = { nofree nosync nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+mmx,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3" "unsafe-fp-math"="true" }
attributes #1 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #2 = { nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: readwrite) }

!llvm.ident = !{!0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0}
!llvm.module.flags = !{!1, !2, !3, !4, !5, !6}

!0 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 1, !"Virtual Function Elim", i32 0}
!3 = !{i32 7, !"uwtable", i32 1}
!4 = !{i32 1, !"ThinLTO", i32 0}
!5 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!6 = !{i32 1, !"LTOPostLink", i32 1}
!7 = !{!8, !12, i64 24}
!8 = !{!"struct@lzma_mf_s", !9, i64 0, !12, i64 8, !12, i64 12, !12, i64 16, !12, i64 20, !12, i64 24, !12, i64 28, !12, i64 32, !12, i64 36, !12, i64 40, !13, i64 48, !14, i64 56, !15, i64 64, !15, i64 72, !12, i64 80, !12, i64 84, !12, i64 88, !12, i64 92, !12, i64 96, !12, i64 100, !10, i64 104, !12, i64 108, !12, i64 112}
!9 = !{!"pointer@_ZTSPh", !10, i64 0}
!10 = !{!"omnipotent char", !11, i64 0}
!11 = !{!"Simple C/C++ TBAA"}
!12 = !{!"int", !10, i64 0}
!13 = !{!"pointer@_ZTSPFjP9lzma_mf_sP10lzma_matchE", !10, i64 0}
!14 = !{!"pointer@_ZTSPFvP9lzma_mf_sjE", !10, i64 0}
!15 = !{!"pointer@_ZTSPj", !10, i64 0}
!16 = !{!8, !12, i64 36}
!17 = !{!8, !12, i64 96}
!18 = !{!8, !10, i64 104}
!19 = !{!8, !12, i64 40}
!20 = !{!8, !9, i64 0}
!21 = !{!10, !10, i64 0}
!22 = !{!8, !12, i64 20}
!23 = !{!8, !15, i64 64}
!24 = !{!12, !12, i64 0}
!25 = !{!8, !12, i64 92}
!26 = !{!8, !15, i64 72}
!27 = !{!8, !12, i64 80}
!28 = !{!8, !12, i64 84}
!29 = !{!30, !12, i64 0}
!30 = !{!"struct@", !12, i64 0, !12, i64 4}
!31 = !{!30, !12, i64 4}
!32 = !{!8, !12, i64 108}
!33 = !{!8, !12, i64 112}
!34 = distinct !{!34, !35, !36, !37, !38}
!35 = !{!"llvm.loop.mustprogress"}
!36 = !{!"llvm.loop.vectorize.width", i32 1}
!37 = !{!"llvm.loop.interleave.count", i32 1}
!38 = !{!"llvm.loop.unroll.disable"}
!39 = distinct !{!39, !35, !38, !36, !37}
!40 = distinct !{!40, !35, !36, !37, !38}
!41 = distinct !{!41, !35, !42, !38, !36, !37}
!42 = !{!"llvm.loop.intel.loopcount_maximum", i32 7}
!43 = !{i32 32}
