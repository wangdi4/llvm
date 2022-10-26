; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; REQUIRES: asserts
; RUN: opt < %s -passes='module(qsortrecognizer)' -debug-only=qsortrecognizer -S 2>&1 | FileCheck %s

; Check that the qsortrecognizer does not recognize @spec_qsort.40 as a spec
; qsort, because @arc_compare.55.85.115 was not recognized as a qsort compare.

; CHECK-NOT: FOUND QSORT
; CHECK: define{{.*}}@spec_qsort.40
; CHECK-NOT: attributes{{.*}}"is-qsort"

%__SOADT___DFR___DFT_struct.arc = type { i64, i32, i32, i64, i64, i32, i16 }

define internal i32 @arc_compare.55.85.115(%__SOADT___DFR___DFT_struct.arc** nocapture readonly %0, %__SOADT___DFR___DFT_struct.arc** nocapture readonly %1) {
  %3 = load %__SOADT___DFR___DFT_struct.arc*, %__SOADT___DFR___DFT_struct.arc** %0, align 8
  %4 = getelementptr %__SOADT___DFR___DFT_struct.arc, %__SOADT___DFR___DFT_struct.arc* %3, i64 0, i32 0
  %5 = load i64, i64* %4, align 8
  %6 = load %__SOADT___DFR___DFT_struct.arc*, %__SOADT___DFR___DFT_struct.arc** %1, align 8
  %7 = getelementptr %__SOADT___DFR___DFT_struct.arc, %__SOADT___DFR___DFT_struct.arc* %6, i64 0, i32 0
  %8 = load i64, i64* %7, align 8
  %9 = icmp sgt i64 %5, %8
  br i1 %9, label %19, label %10

10:                                               ; preds = %2
  %11 = icmp slt i64 %5, %8
  br i1 %11, label %19, label %12

12:                                               ; preds = %10
  %13 = getelementptr inbounds %__SOADT___DFR___DFT_struct.arc, %__SOADT___DFR___DFT_struct.arc* %3, i64 0, i32 1
  %14 = load i32, i32* %13, align 8
  %15 = getelementptr inbounds %__SOADT___DFR___DFT_struct.arc, %__SOADT___DFR___DFT_struct.arc* %6, i64 0, i32 1
  %16 = load i32, i32* %15, align 8
  %17 = icmp slt i32 %14, %16
  %18 = select i1 %17, i32 -1, i32 1
  ret i32 %18

19:                                               ; preds = %10, %2
  %20 = phi i32 [ 1, %2 ], [ -1, %10 ]
  ret i32 %20
}

define internal fastcc void @spec_qsort.40(i8* %0, i64 %1) unnamed_addr #0 {
  %3 = ptrtoint i8* %0 to i64
  %4 = icmp ult i64 %1, 7
  br i1 %4, label %5, label %32

5:                                                ; preds = %315, %2
  %6 = phi i64 [ %1, %2 ], [ %318, %315 ]
  %7 = phi i8* [ %0, %2 ], [ %317, %315 ]
  %8 = shl i64 %6, 3
  %9 = getelementptr inbounds i8, i8* %7, i64 %8
  %10 = icmp sgt i64 %8, 8
  br i1 %10, label %11, label %321

11:                                               ; preds = %5
  %12 = getelementptr inbounds i8, i8* %7, i64 8
  br label %13

13:                                               ; preds = %29, %11
  %14 = phi i8* [ %12, %11 ], [ %30, %29 ]
  %15 = icmp ugt i8* %14, %7
  br i1 %15, label %16, label %29

16:                                               ; preds = %13, %23
  %17 = phi i8* [ %18, %23 ], [ %14, %13 ]
  %18 = getelementptr inbounds i8, i8* %17, i64 -8
  %19 = bitcast i8* %18 to %__SOADT___DFR___DFT_struct.arc**
  %20 = bitcast i8* %17 to %__SOADT___DFR___DFT_struct.arc**
  %21 = tail call i32 @arc_compare.55.85.115(%__SOADT___DFR___DFT_struct.arc** nonnull %19, %__SOADT___DFR___DFT_struct.arc** %20) #1
  %22 = icmp sgt i32 %21, 0
  br i1 %22, label %23, label %29

23:                                               ; preds = %16
  %24 = bitcast i8* %17 to i64*
  %25 = load i64, i64* %24, align 8
  %26 = bitcast i8* %18 to i64*
  %27 = load i64, i64* %26, align 8
  store i64 %27, i64* %24, align 8
  store i64 %25, i64* %26, align 8
  %28 = icmp ugt i8* %18, %7
  br i1 %28, label %16, label %29

29:                                               ; preds = %23, %16, %13
  %30 = getelementptr inbounds i8, i8* %14, i64 8
  %31 = icmp ult i8* %30, %9
  br i1 %31, label %13, label %321

32:                                               ; preds = %2, %315
  %33 = phi i64 [ %319, %315 ], [ %3, %2 ]
  %34 = phi i8* [ %317, %315 ], [ %0, %2 ]
  %35 = phi i64 [ %318, %315 ], [ %1, %2 ]
  %36 = lshr i64 %35, 1
  %37 = shl i64 %36, 3
  %38 = getelementptr inbounds i8, i8* %34, i64 %37
  %39 = icmp eq i64 %35, 7
  br i1 %39, label %156, label %40

40:                                               ; preds = %32
  %41 = shl i64 %35, 3
  %42 = add i64 %41, -8
  %43 = getelementptr inbounds i8, i8* %34, i64 %42
  %44 = icmp ugt i64 %35, 40
  br i1 %44, label %45, label %129

45:                                               ; preds = %40
  %46 = and i64 %35, -8
  %47 = getelementptr inbounds i8, i8* %34, i64 %46
  %48 = shl i64 %46, 1
  %49 = getelementptr inbounds i8, i8* %34, i64 %48
  %50 = bitcast i8* %34 to %__SOADT___DFR___DFT_struct.arc**
  %51 = bitcast i8* %47 to %__SOADT___DFR___DFT_struct.arc**
  %52 = tail call i32 @arc_compare.55.85.115(%__SOADT___DFR___DFT_struct.arc** %50, %__SOADT___DFR___DFT_struct.arc** %51) #1
  %53 = icmp slt i32 %52, 0
  %54 = bitcast i8* %47 to %__SOADT___DFR___DFT_struct.arc**
  %55 = bitcast i8* %49 to %__SOADT___DFR___DFT_struct.arc**
  %56 = tail call i32 @arc_compare.55.85.115(%__SOADT___DFR___DFT_struct.arc** %54, %__SOADT___DFR___DFT_struct.arc** %55) #1
  br i1 %53, label %57, label %65

57:                                               ; preds = %45
  %58 = icmp slt i32 %56, 0
  br i1 %58, label %73, label %59

59:                                               ; preds = %57
  %60 = bitcast i8* %34 to %__SOADT___DFR___DFT_struct.arc**
  %61 = bitcast i8* %49 to %__SOADT___DFR___DFT_struct.arc**
  %62 = tail call i32 @arc_compare.55.85.115(%__SOADT___DFR___DFT_struct.arc** %60, %__SOADT___DFR___DFT_struct.arc** %61) #1
  %63 = icmp slt i32 %62, 0
  %64 = select i1 %63, i8* %49, i8* %34
  br label %73

65:                                               ; preds = %45
  %66 = icmp sgt i32 %56, 0
  br i1 %66, label %73, label %67

67:                                               ; preds = %65
  %68 = bitcast i8* %34 to %__SOADT___DFR___DFT_struct.arc**
  %69 = bitcast i8* %49 to %__SOADT___DFR___DFT_struct.arc**
  %70 = tail call i32 @arc_compare.55.85.115(%__SOADT___DFR___DFT_struct.arc** %68, %__SOADT___DFR___DFT_struct.arc** %69) #1
  %71 = icmp slt i32 %70, 0
  %72 = select i1 %71, i8* %34, i8* %49
  br label %73

73:                                               ; preds = %67, %65, %59, %57
  %74 = phi i8* [ %64, %59 ], [ %72, %67 ], [ %47, %57 ], [ %47, %65 ]
  %75 = sub i64 0, %46
  %76 = getelementptr inbounds i8, i8* %38, i64 %75
  %77 = getelementptr inbounds i8, i8* %38, i64 %46
  %78 = bitcast i8* %76 to %__SOADT___DFR___DFT_struct.arc**
  %79 = bitcast i8* %38 to %__SOADT___DFR___DFT_struct.arc**
  %80 = tail call i32 @arc_compare.55.85.115(%__SOADT___DFR___DFT_struct.arc** %78, %__SOADT___DFR___DFT_struct.arc** %79) #1
  %81 = icmp slt i32 %80, 0
  %82 = bitcast i8* %38 to %__SOADT___DFR___DFT_struct.arc**
  %83 = bitcast i8* %77 to %__SOADT___DFR___DFT_struct.arc**
  %84 = tail call i32 @arc_compare.55.85.115(%__SOADT___DFR___DFT_struct.arc** %82, %__SOADT___DFR___DFT_struct.arc** %83) #1
  br i1 %81, label %85, label %93

85:                                               ; preds = %73
  %86 = icmp slt i32 %84, 0
  br i1 %86, label %101, label %87

87:                                               ; preds = %85
  %88 = bitcast i8* %76 to %__SOADT___DFR___DFT_struct.arc**
  %89 = bitcast i8* %77 to %__SOADT___DFR___DFT_struct.arc**
  %90 = tail call i32 @arc_compare.55.85.115(%__SOADT___DFR___DFT_struct.arc** %88, %__SOADT___DFR___DFT_struct.arc** %89) #1
  %91 = icmp slt i32 %90, 0
  %92 = select i1 %91, i8* %77, i8* %76
  br label %101

93:                                               ; preds = %73
  %94 = icmp sgt i32 %84, 0
  br i1 %94, label %101, label %95

95:                                               ; preds = %93
  %96 = bitcast i8* %76 to %__SOADT___DFR___DFT_struct.arc**
  %97 = bitcast i8* %77 to %__SOADT___DFR___DFT_struct.arc**
  %98 = tail call i32 @arc_compare.55.85.115(%__SOADT___DFR___DFT_struct.arc** %96, %__SOADT___DFR___DFT_struct.arc** %97) #1
  %99 = icmp slt i32 %98, 0
  %100 = select i1 %99, i8* %76, i8* %77
  br label %101

101:                                              ; preds = %95, %93, %87, %85
  %102 = phi i8* [ %92, %87 ], [ %100, %95 ], [ %38, %85 ], [ %38, %93 ]
  %103 = sub i64 0, %48
  %104 = getelementptr inbounds i8, i8* %43, i64 %103
  %105 = getelementptr inbounds i8, i8* %43, i64 %75
  %106 = bitcast i8* %104 to %__SOADT___DFR___DFT_struct.arc**
  %107 = bitcast i8* %105 to %__SOADT___DFR___DFT_struct.arc**
  %108 = tail call i32 @arc_compare.55.85.115(%__SOADT___DFR___DFT_struct.arc** %106, %__SOADT___DFR___DFT_struct.arc** %107) #1
  %109 = icmp slt i32 %108, 0
  %110 = bitcast i8* %105 to %__SOADT___DFR___DFT_struct.arc**
  %111 = bitcast i8* %43 to %__SOADT___DFR___DFT_struct.arc**
  %112 = tail call i32 @arc_compare.55.85.115(%__SOADT___DFR___DFT_struct.arc** %110, %__SOADT___DFR___DFT_struct.arc** %111) #1
  br i1 %109, label %113, label %121

113:                                              ; preds = %101
  %114 = icmp slt i32 %112, 0
  br i1 %114, label %129, label %115

115:                                              ; preds = %113
  %116 = bitcast i8* %104 to %__SOADT___DFR___DFT_struct.arc**
  %117 = bitcast i8* %43 to %__SOADT___DFR___DFT_struct.arc**
  %118 = tail call i32 @arc_compare.55.85.115(%__SOADT___DFR___DFT_struct.arc** %116, %__SOADT___DFR___DFT_struct.arc** %117) #1
  %119 = icmp slt i32 %118, 0
  %120 = select i1 %119, i8* %43, i8* %104
  br label %129

121:                                              ; preds = %101
  %122 = icmp sgt i32 %112, 0
  br i1 %122, label %129, label %123

123:                                              ; preds = %121
  %124 = bitcast i8* %104 to %__SOADT___DFR___DFT_struct.arc**
  %125 = bitcast i8* %43 to %__SOADT___DFR___DFT_struct.arc**
  %126 = tail call i32 @arc_compare.55.85.115(%__SOADT___DFR___DFT_struct.arc** %124, %__SOADT___DFR___DFT_struct.arc** %125) #1
  %127 = icmp slt i32 %126, 0
  %128 = select i1 %127, i8* %104, i8* %43
  br label %129

129:                                              ; preds = %123, %121, %115, %113, %40
  %130 = phi i8* [ %43, %40 ], [ %120, %115 ], [ %128, %123 ], [ %105, %113 ], [ %105, %121 ]
  %131 = phi i8* [ %38, %40 ], [ %102, %115 ], [ %102, %123 ], [ %102, %113 ], [ %102, %121 ]
  %132 = phi i8* [ %34, %40 ], [ %74, %115 ], [ %74, %123 ], [ %74, %113 ], [ %74, %121 ]
  %133 = bitcast i8* %132 to %__SOADT___DFR___DFT_struct.arc**
  %134 = bitcast i8* %131 to %__SOADT___DFR___DFT_struct.arc**
  %135 = tail call i32 @arc_compare.55.85.115(%__SOADT___DFR___DFT_struct.arc** %133, %__SOADT___DFR___DFT_struct.arc** %134) #1
  %136 = icmp slt i32 %135, 0
  %137 = bitcast i8* %131 to %__SOADT___DFR___DFT_struct.arc**
  %138 = bitcast i8* %130 to %__SOADT___DFR___DFT_struct.arc**
  %139 = tail call i32 @arc_compare.55.85.115(%__SOADT___DFR___DFT_struct.arc** %137, %__SOADT___DFR___DFT_struct.arc** %138) #1
  br i1 %136, label %140, label %148

140:                                              ; preds = %129
  %141 = icmp slt i32 %139, 0
  br i1 %141, label %156, label %142

142:                                              ; preds = %140
  %143 = bitcast i8* %132 to %__SOADT___DFR___DFT_struct.arc**
  %144 = bitcast i8* %130 to %__SOADT___DFR___DFT_struct.arc**
  %145 = tail call i32 @arc_compare.55.85.115(%__SOADT___DFR___DFT_struct.arc** %143, %__SOADT___DFR___DFT_struct.arc** %144) #1
  %146 = icmp slt i32 %145, 0
  %147 = select i1 %146, i8* %130, i8* %132
  br label %156

148:                                              ; preds = %129
  %149 = icmp sgt i32 %139, 0
  br i1 %149, label %156, label %150

150:                                              ; preds = %148
  %151 = bitcast i8* %132 to %__SOADT___DFR___DFT_struct.arc**
  %152 = bitcast i8* %130 to %__SOADT___DFR___DFT_struct.arc**
  %153 = tail call i32 @arc_compare.55.85.115(%__SOADT___DFR___DFT_struct.arc** %151, %__SOADT___DFR___DFT_struct.arc** %152) #1
  %154 = icmp slt i32 %153, 0
  %155 = select i1 %154, i8* %132, i8* %130
  br label %156

156:                                              ; preds = %32, %140, %142, %148, %150
  %157 = phi i8* [ %38, %32 ], [ %147, %142 ], [ %155, %150 ], [ %131, %140 ], [ %131, %148 ]
  %158 = bitcast i8* %34 to i64*
  %159 = load i64, i64* %158, align 8
  %160 = bitcast i8* %157 to i64*
  %161 = load i64, i64* %160, align 8
  store i64 %161, i64* %158, align 8
  store i64 %159, i64* %160, align 8
  %162 = getelementptr inbounds i8, i8* %34, i64 8
  %163 = shl i64 %35, 3
  %164 = add i64 %163, -8
  %165 = getelementptr inbounds i8, i8* %34, i64 %164
  br label %166

166:                                              ; preds = %220, %156
  %167 = phi i32 [ 0, %156 ], [ 1, %220 ]
  %168 = phi i8* [ %165, %156 ], [ %201, %220 ]
  %169 = phi i8* [ %165, %156 ], [ %226, %220 ]
  %170 = phi i8* [ %162, %156 ], [ %225, %220 ]
  %171 = phi i8* [ %162, %156 ], [ %197, %220 ]
  %172 = icmp ugt i8* %170, %169
  br i1 %172, label %194, label %173

173:                                              ; preds = %166, %189
  %174 = phi i8* [ %191, %189 ], [ %171, %166 ]
  %175 = phi i8* [ %192, %189 ], [ %170, %166 ]
  %176 = phi i32 [ %190, %189 ], [ %167, %166 ]
  %177 = bitcast i8* %175 to %__SOADT___DFR___DFT_struct.arc**
  %178 = bitcast i8* %34 to %__SOADT___DFR___DFT_struct.arc**
  %179 = tail call i32 @arc_compare.55.85.115(%__SOADT___DFR___DFT_struct.arc** %177, %__SOADT___DFR___DFT_struct.arc** %178) #1
  %180 = icmp slt i32 %179, 1
  br i1 %180, label %181, label %194

181:                                              ; preds = %173
  %182 = icmp eq i32 %179, 0
  br i1 %182, label %183, label %189

183:                                              ; preds = %181
  %184 = bitcast i8* %174 to i64*
  %185 = load i64, i64* %184, align 8
  %186 = bitcast i8* %175 to i64*
  %187 = load i64, i64* %186, align 8
  store i64 %187, i64* %184, align 8
  store i64 %185, i64* %186, align 8
  %188 = getelementptr inbounds i8, i8* %174, i64 8
  br label %189

189:                                              ; preds = %183, %181
  %190 = phi i32 [ 1, %183 ], [ %176, %181 ]
  %191 = phi i8* [ %188, %183 ], [ %174, %181 ]
  %192 = getelementptr inbounds i8, i8* %175, i64 8
  %193 = icmp ugt i8* %192, %169
  br i1 %193, label %194, label %173

194:                                              ; preds = %189, %173, %166
  %195 = phi i32 [ %167, %166 ], [ %176, %173 ], [ %190, %189 ]
  %196 = phi i8* [ %170, %166 ], [ %175, %173 ], [ %192, %189 ]
  %197 = phi i8* [ %171, %166 ], [ %174, %173 ], [ %191, %189 ]
  %198 = icmp ugt i8* %196, %169
  br i1 %198, label %227, label %199

199:                                              ; preds = %194, %215
  %200 = phi i8* [ %218, %215 ], [ %169, %194 ]
  %201 = phi i8* [ %217, %215 ], [ %168, %194 ]
  %202 = phi i32 [ %216, %215 ], [ %195, %194 ]
  %203 = bitcast i8* %200 to %__SOADT___DFR___DFT_struct.arc**
  %204 = bitcast i8* %34 to %__SOADT___DFR___DFT_struct.arc**
  %205 = tail call i32 @arc_compare.55.85.115(%__SOADT___DFR___DFT_struct.arc** %203, %__SOADT___DFR___DFT_struct.arc** %204) #1
  %206 = icmp sgt i32 %205, -1
  br i1 %206, label %207, label %220

207:                                              ; preds = %199
  %208 = icmp eq i32 %205, 0
  br i1 %208, label %209, label %215

209:                                              ; preds = %207
  %210 = bitcast i8* %200 to i64*
  %211 = load i64, i64* %210, align 8
  %212 = bitcast i8* %201 to i64*
  %213 = load i64, i64* %212, align 8
  store i64 %213, i64* %210, align 8
  store i64 %211, i64* %212, align 8
  %214 = getelementptr inbounds i8, i8* %201, i64 -8
  br label %215

215:                                              ; preds = %209, %207
  %216 = phi i32 [ 1, %209 ], [ %202, %207 ]
  %217 = phi i8* [ %214, %209 ], [ %201, %207 ]
  %218 = getelementptr inbounds i8, i8* %200, i64 -8
  %219 = icmp ugt i8* %196, %218
  br i1 %219, label %227, label %199

220:                                              ; preds = %199
  %221 = bitcast i8* %196 to i64*
  %222 = load i64, i64* %221, align 8
  %223 = bitcast i8* %200 to i64*
  %224 = load i64, i64* %223, align 8
  store i64 %224, i64* %221, align 8
  store i64 %222, i64* %223, align 8
  %225 = getelementptr inbounds i8, i8* %196, i64 8
  %226 = getelementptr inbounds i8, i8* %200, i64 -8
  br label %166

227:                                              ; preds = %215, %194
  %228 = phi i32 [ %216, %215 ], [ %195, %194 ]
  %229 = phi i8* [ %217, %215 ], [ %168, %194 ]
  %230 = phi i8* [ %218, %215 ], [ %169, %194 ]
  %231 = icmp eq i32 %228, 0
  %232 = shl i64 %35, 3
  %233 = getelementptr inbounds i8, i8* %34, i64 %232
  br i1 %231, label %234, label %255

234:                                              ; preds = %227
  %235 = icmp sgt i64 %232, 8
  br i1 %235, label %236, label %321

236:                                              ; preds = %252, %234
  %237 = phi i8* [ %253, %252 ], [ %162, %234 ]
  %238 = icmp ugt i8* %237, %34
  br i1 %238, label %239, label %252

239:                                              ; preds = %236, %246
  %240 = phi i8* [ %241, %246 ], [ %237, %236 ]
  %241 = getelementptr inbounds i8, i8* %240, i64 -8
  %242 = bitcast i8* %241 to %__SOADT___DFR___DFT_struct.arc**
  %243 = bitcast i8* %240 to %__SOADT___DFR___DFT_struct.arc**
  %244 = tail call i32 @arc_compare.55.85.115(%__SOADT___DFR___DFT_struct.arc** nonnull %242, %__SOADT___DFR___DFT_struct.arc** %243) #1
  %245 = icmp sgt i32 %244, 0
  br i1 %245, label %246, label %252

246:                                              ; preds = %239
  %247 = bitcast i8* %240 to i64*
  %248 = load i64, i64* %247, align 8
  %249 = bitcast i8* %241 to i64*
  %250 = load i64, i64* %249, align 8
  store i64 %250, i64* %247, align 8
  store i64 %248, i64* %249, align 8
  %251 = icmp ugt i8* %241, %34
  br i1 %251, label %239, label %252

252:                                              ; preds = %246, %239, %236
  %253 = getelementptr inbounds i8, i8* %237, i64 8
  %254 = icmp ult i8* %253, %233
  br i1 %254, label %236, label %321

255:                                              ; preds = %227
  %256 = ptrtoint i8* %197 to i64
  %257 = sub i64 %256, %33
  %258 = ptrtoint i8* %196 to i64
  %259 = sub i64 %258, %256
  %260 = icmp slt i64 %257, %259
  %261 = select i1 %260, i64 %257, i64 %259
  %262 = icmp eq i64 %261, 0
  br i1 %262, label %281, label %263

263:                                              ; preds = %255
  %264 = sub i64 0, %261
  %265 = getelementptr inbounds i8, i8* %196, i64 %264
  %266 = shl i64 %261, 32
  %267 = ashr exact i64 %266, 32
  %268 = lshr i64 %267, 3
  %269 = bitcast i8* %34 to i64*
  %270 = bitcast i8* %265 to i64*
  br label %271

271:                                              ; preds = %271, %263
  %272 = phi i64* [ %270, %263 ], [ %278, %271 ]
  %273 = phi i64* [ %269, %263 ], [ %277, %271 ]
  %274 = phi i64 [ %268, %263 ], [ %279, %271 ]
  %275 = load i64, i64* %273, align 8
  %276 = load i64, i64* %272, align 8
  %277 = getelementptr inbounds i64, i64* %273, i64 1
  store i64 %276, i64* %273, align 8
  %278 = getelementptr inbounds i64, i64* %272, i64 1
  store i64 %275, i64* %272, align 8
  %279 = add nsw i64 %274, -1
  %280 = icmp sgt i64 %274, 1
  br i1 %280, label %271, label %281

281:                                              ; preds = %271, %255
  %282 = ptrtoint i8* %229 to i64
  %283 = ptrtoint i8* %230 to i64
  %284 = sub i64 %282, %283
  %285 = ptrtoint i8* %233 to i64
  %286 = sub i64 %285, %282
  %287 = add i64 %286, -8
  %288 = icmp slt i64 %284, %287
  %289 = select i1 %288, i64 %284, i64 %287
  %290 = icmp eq i64 %289, 0
  br i1 %290, label %309, label %291

291:                                              ; preds = %281
  %292 = sub i64 0, %289
  %293 = getelementptr inbounds i8, i8* %233, i64 %292
  %294 = shl i64 %289, 32
  %295 = ashr exact i64 %294, 32
  %296 = lshr i64 %295, 3
  %297 = bitcast i8* %196 to i64*
  %298 = bitcast i8* %293 to i64*
  br label %299

299:                                              ; preds = %299, %291
  %300 = phi i64* [ %298, %291 ], [ %306, %299 ]
  %301 = phi i64* [ %297, %291 ], [ %305, %299 ]
  %302 = phi i64 [ %296, %291 ], [ %307, %299 ]
  %303 = load i64, i64* %301, align 8
  %304 = load i64, i64* %300, align 8
  %305 = getelementptr inbounds i64, i64* %301, i64 1
  store i64 %304, i64* %301, align 8
  %306 = getelementptr inbounds i64, i64* %300, i64 1
  store i64 %303, i64* %300, align 8
  %307 = add nsw i64 %302, -1
  %308 = icmp sgt i64 %302, 1
  br i1 %308, label %299, label %309

309:                                              ; preds = %299, %281
  %310 = icmp ugt i64 %259, 8
  br i1 %310, label %311, label %313

311:                                              ; preds = %309
  %312 = lshr i64 %259, 3
  tail call fastcc void @spec_qsort.40(i8* %34, i64 %312)
  br label %313

313:                                              ; preds = %311, %309
  %314 = icmp ugt i64 %284, 8
  br i1 %314, label %315, label %321

315:                                              ; preds = %313
  %316 = sub i64 0, %284
  %317 = getelementptr inbounds i8, i8* %233, i64 %316
  %318 = lshr i64 %284, 3
  %319 = ptrtoint i8* %317 to i64
  %320 = icmp ult i64 %318, 7
  br i1 %320, label %5, label %32

321:                                              ; preds = %313, %252, %234, %29, %5
  ret void
}

attributes #0 = { "is-qsort-spec_qsort" }
attributes #1 = { "must-be-qsort-compare" }

; end INTEL_FEATURE_SW_ADVANCED
