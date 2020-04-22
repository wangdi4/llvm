; REQUIRES: asserts
; RUN: opt < %s -qsortrecognizer -debug-only=qsortrecognizer -qsort-unit-test -qsort-test-compare-func -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -passes='module(qsortrecognizer)' -debug-only=qsortrecognizer -qsort-unit-test -qsort-test-compare-func -disable-output 2>&1 | FileCheck %s

; This test checks that the compare function was found when debug instructions
; are included. This is the same test as qsort-compare-func01.ll but with debug
; instructions and metadata.

; CHECK: QsortRec: Compare Function found: arc_compare
; CHECK: PASSED QSORT RECOGNITION UNIT TESTS

%struct.arc = type { i32, i64, %struct.node*, %struct.node*, i16, %struct.arc*, %struct.arc*, i64, i64 }
%struct.node = type { i64, i32, %struct.node*, %struct.node*, %struct.node*, %struct.node*, %struct.arc*, %struct.arc*, %struct.arc*, %struct.arc*, i64, i64, i32, i32 }
%struct.basket = type { %struct.arc*, i64, i64, i64 }

declare void @llvm.dbg.value(metadata %0, metadata %1, metadata %2)
declare void @llvm.dbg.label(metadata %0)

define internal i32 @arc_compare(%struct.arc** nocapture readonly %arg, %struct.arc** nocapture readonly %arg1) {
bb:
  %tmp = load %struct.arc*, %struct.arc** %arg, align 8
  %tmp2 = getelementptr inbounds %struct.arc, %struct.arc* %tmp, i64 0, i32 7
  %tmp3 = load i64, i64* %tmp2, align 8
  %tmp4 = load %struct.arc*, %struct.arc** %arg1, align 8
  %tmp5 = getelementptr inbounds %struct.arc, %struct.arc* %tmp4, i64 0, i32 7
  %tmp6 = load i64, i64* %tmp5, align 8
  %tmp7 = icmp sgt i64 %tmp3, %tmp6
  br i1 %tmp7, label %bb17, label %bb8

bb8:                                              ; preds = %bb
  %tmp9 = icmp slt i64 %tmp3, %tmp6
  br i1 %tmp9, label %bb17, label %bb10

bb10:                                             ; preds = %bb8
  %tmp11 = getelementptr inbounds %struct.arc, %struct.arc* %tmp, i64 0, i32 0
  %tmp12 = load i32, i32* %tmp11, align 8
  %tmp13 = getelementptr inbounds %struct.arc, %struct.arc* %tmp4, i64 0, i32 0
  %tmp14 = load i32, i32* %tmp13, align 8
  %tmp15 = icmp slt i32 %tmp12, %tmp14
  %tmp16 = select i1 %tmp15, i32 -1, i32 1
  br label %bb17

bb17:                                             ; preds = %bb10, %bb8, %bb
  %tmp18 = phi i32 [ 1, %bb ], [ -1, %bb8 ], [ %tmp16, %bb10 ]
  ret i32 %tmp18
}

; Function Attrs: nounwind uwtable
define internal fastcc void @qsort_sorter(i8* %0, i64 %1) unnamed_addr {
  call void @llvm.dbg.value(metadata i8* %0, metadata !563, metadata !DIExpression())
  call void @llvm.dbg.value(metadata i64 %1, metadata !564, metadata !DIExpression())
  call void @llvm.dbg.value(metadata i64 8, metadata !565, metadata !DIExpression()), !dbg !665
  call void @llvm.dbg.value(metadata i32 (i8*, i8*)* bitcast (i32 (%struct.arc**, %struct.arc**)* @arc_compare to i32 (i8*, i8*)*), metadata !566, metadata !DIExpression())
  call void @llvm.dbg.label(metadata !664)
  %3 = ptrtoint i8* %0 to i64
  %4 = icmp ult i64 %1, 7
  br i1 %4, label %5, label %32

5:                                                ; preds = %315, %2
  %6 = phi i64 [ %1, %2 ], [ %318, %315 ]
  %7 = phi i8* [ %0, %2 ], [ %317, %315 ]
  call void @llvm.dbg.value(metadata i64 %6, metadata !564, metadata !DIExpression())
  call void @llvm.dbg.value(metadata i8* %7, metadata !563, metadata !DIExpression())
  call void @llvm.dbg.value(metadata i8* undef, metadata !572, metadata !DIExpression())
  %8 = shl i64 %6, 3
  %9 = getelementptr inbounds i8, i8* %7, i64 %8
  %10 = icmp sgt i64 %8, 8
  br i1 %10, label %11, label %321

11:                                               ; preds = %5
  %12 = getelementptr inbounds i8, i8* %7, i64 8
  call void @llvm.dbg.value(metadata i8* %12, metadata !572, metadata !DIExpression())
  br label %13

13:                                               ; preds = %29, %11
  %14 = phi i8* [ %12, %11 ], [ %30, %29 ]
  call void @llvm.dbg.value(metadata i8* %14, metadata !571, metadata !DIExpression())
  %15 = icmp ugt i8* %14, %7
  br i1 %15, label %16, label %29

16:                                               ; preds = %13, %23
  %17 = phi i8* [ %18, %23 ], [ %14, %13 ]
  call void @llvm.dbg.value(metadata i8* %17, metadata !571, metadata !DIExpression())
  %18 = getelementptr inbounds i8, i8* %17, i64 -8
  %19 = bitcast i8* %18 to %struct.arc**
  %20 = bitcast i8* %17 to %struct.arc**
  %21 = tail call i32 @arc_compare(%struct.arc** nonnull %19, %struct.arc** %20) #12
  %22 = icmp sgt i32 %21, 0
  br i1 %22, label %23, label %29

23:                                               ; preds = %16
  %24 = bitcast i8* %17 to i64*
  %25 = load i64, i64* %24, align 8
  call void @llvm.dbg.value(metadata i64 %25, metadata !580, metadata !DIExpression())
  %26 = bitcast i8* %18 to i64*
  %27 = load i64, i64* %26, align 8
  store i64 %27, i64* %24, align 8
  store i64 %25, i64* %26, align 8
  call void @llvm.dbg.value(metadata i8* %18, metadata !571, metadata !DIExpression())
  %28 = icmp ugt i8* %18, %7
  br i1 %28, label %16, label %29

29:                                               ; preds = %23, %16, %13
  call void @llvm.dbg.value(metadata i8* undef, metadata !572, metadata !DIExpression())
  %30 = getelementptr inbounds i8, i8* %14, i64 8
  call void @llvm.dbg.value(metadata i8* %30, metadata !572, metadata !DIExpression())
  %31 = icmp ult i8* %30, %9
  br i1 %31, label %13, label %321

32:                                               ; preds = %2, %315
  %33 = phi i64 [ %319, %315 ], [ %3, %2 ]
  %34 = phi i8* [ %317, %315 ], [ %0, %2 ]
  %35 = phi i64 [ %318, %315 ], [ %1, %2 ]
  call void @llvm.dbg.value(metadata i8* %34, metadata !563, metadata !DIExpression())
  call void @llvm.dbg.value(metadata i64 %35, metadata !564, metadata !DIExpression())
  %36 = lshr i64 %35, 1
  %37 = shl i64 %36, 3
  %38 = getelementptr inbounds i8, i8* %34, i64 %37
  call void @llvm.dbg.value(metadata i8* %38, metadata !572, metadata !DIExpression()), !dbg !665
  %39 = icmp eq i64 %35, 7
  br i1 %39, label %156, label %40

40:                                               ; preds = %32
  call void @llvm.dbg.value(metadata i8* %34, metadata !571, metadata !DIExpression())
  %41 = shl i64 %35, 3
  %42 = add i64 %41, -8
  %43 = getelementptr inbounds i8, i8* %34, i64 %42
  call void @llvm.dbg.value(metadata i8* %43, metadata !573, metadata !DIExpression())
  %44 = icmp ugt i64 %35, 40
  br i1 %44, label %45, label %129

45:                                               ; preds = %40
  %46 = and i64 %35, -8
  call void @llvm.dbg.value(metadata i64 %46, metadata !574, metadata !DIExpression())
  %47 = getelementptr inbounds i8, i8* %34, i64 %46
  %48 = shl i64 %46, 1
  %49 = getelementptr inbounds i8, i8* %34, i64 %48
  call void @llvm.dbg.value(metadata i8* %34, metadata !706, metadata !DIExpression())
  call void @llvm.dbg.value(metadata i8* %47, metadata !711, metadata !DIExpression())
  call void @llvm.dbg.value(metadata i8* %49, metadata !712, metadata !DIExpression())
  call void @llvm.dbg.value(metadata i32 (i8*, i8*)* bitcast (i32 (%struct.arc**, %struct.arc**)* @arc_compare to i32 (i8*, i8*)*), metadata !713, metadata !DIExpression())
  %50 = bitcast i8* %34 to %struct.arc**
  %51 = bitcast i8* %47 to %struct.arc**
  %52 = tail call i32 @arc_compare(%struct.arc** %50, %struct.arc** %51) #12
  %53 = icmp slt i32 %52, 0
  %54 = bitcast i8* %47 to %struct.arc**
  %55 = bitcast i8* %49 to %struct.arc**
  %56 = tail call i32 @arc_compare(%struct.arc** %54, %struct.arc** %55) #12
  br i1 %53, label %57, label %65

57:                                               ; preds = %45
  %58 = icmp slt i32 %56, 0
  br i1 %58, label %73, label %59

59:                                               ; preds = %57
  %60 = bitcast i8* %34 to %struct.arc**
  %61 = bitcast i8* %49 to %struct.arc**
  %62 = tail call i32 @arc_compare(%struct.arc** %60, %struct.arc** %61) #12
  %63 = icmp slt i32 %62, 0
  %64 = select i1 %63, i8* %49, i8* %34
  br label %73

65:                                               ; preds = %45
  %66 = icmp sgt i32 %56, 0
  br i1 %66, label %73, label %67

67:                                               ; preds = %65
  %68 = bitcast i8* %34 to %struct.arc**
  %69 = bitcast i8* %49 to %struct.arc**
  %70 = tail call i32 @arc_compare(%struct.arc** %68, %struct.arc** %69) #12
  %71 = icmp slt i32 %70, 0
  %72 = select i1 %71, i8* %34, i8* %49
  br label %73

73:                                               ; preds = %67, %65, %59, %57
  %74 = phi i8* [ %64, %59 ], [ %72, %67 ], [ %47, %57 ], [ %47, %65 ]
  call void @llvm.dbg.value(metadata i8* %74, metadata !571, metadata !DIExpression())
  %75 = sub i64 0, %46
  %76 = getelementptr inbounds i8, i8* %38, i64 %75
  %77 = getelementptr inbounds i8, i8* %38, i64 %46
  call void @llvm.dbg.value(metadata i8* %76, metadata !706, metadata !DIExpression())
  call void @llvm.dbg.value(metadata i8* %38, metadata !711, metadata !DIExpression())
  call void @llvm.dbg.value(metadata i8* %77, metadata !712, metadata !DIExpression())
  call void @llvm.dbg.value(metadata i32 (i8*, i8*)* bitcast (i32 (%struct.arc**, %struct.arc**)* @arc_compare to i32 (i8*, i8*)*), metadata !713, metadata !DIExpression())
  %78 = bitcast i8* %76 to %struct.arc**
  %79 = bitcast i8* %38 to %struct.arc**
  %80 = tail call i32 @arc_compare(%struct.arc** %78, %struct.arc** %79) #12
  %81 = icmp slt i32 %80, 0
  %82 = bitcast i8* %38 to %struct.arc**
  %83 = bitcast i8* %77 to %struct.arc**
  %84 = tail call i32 @arc_compare(%struct.arc** %82, %struct.arc** %83) #12
  br i1 %81, label %85, label %93

85:                                               ; preds = %73
  %86 = icmp slt i32 %84, 0
  br i1 %86, label %101, label %87

87:                                               ; preds = %85
  %88 = bitcast i8* %76 to %struct.arc**
  %89 = bitcast i8* %77 to %struct.arc**
  %90 = tail call i32 @arc_compare(%struct.arc** %88, %struct.arc** %89) #12
  %91 = icmp slt i32 %90, 0
  %92 = select i1 %91, i8* %77, i8* %76
  br label %101

93:                                               ; preds = %73
  %94 = icmp sgt i32 %84, 0
  br i1 %94, label %101, label %95

95:                                               ; preds = %93
  %96 = bitcast i8* %76 to %struct.arc**
  %97 = bitcast i8* %77 to %struct.arc**
  %98 = tail call i32 @arc_compare(%struct.arc** %96, %struct.arc** %97) #12
  %99 = icmp slt i32 %98, 0
  %100 = select i1 %99, i8* %76, i8* %77
  br label %101

101:                                              ; preds = %95, %93, %87, %85
  %102 = phi i8* [ %92, %87 ], [ %100, %95 ], [ %38, %85 ], [ %38, %93 ]
  call void @llvm.dbg.value(metadata i8* %102, metadata !572, metadata !DIExpression())
  %103 = sub i64 0, %48
  %104 = getelementptr inbounds i8, i8* %43, i64 %103
  %105 = getelementptr inbounds i8, i8* %43, i64 %75
  call void @llvm.dbg.value(metadata i8* %104, metadata !706, metadata !DIExpression())
  call void @llvm.dbg.value(metadata i8* %105, metadata !711, metadata !DIExpression())
  call void @llvm.dbg.value(metadata i8* %43, metadata !712, metadata !DIExpression())
  call void @llvm.dbg.value(metadata i32 (i8*, i8*)* bitcast (i32 (%struct.arc**, %struct.arc**)* @arc_compare to i32 (i8*, i8*)*), metadata !713, metadata !DIExpression())
  %106 = bitcast i8* %104 to %struct.arc**
  %107 = bitcast i8* %105 to %struct.arc**
  %108 = tail call i32 @arc_compare(%struct.arc** %106, %struct.arc** %107) #12
  %109 = icmp slt i32 %108, 0
  %110 = bitcast i8* %105 to %struct.arc**
  %111 = bitcast i8* %43 to %struct.arc**
  %112 = tail call i32 @arc_compare(%struct.arc** %110, %struct.arc** %111) #12
  br i1 %109, label %113, label %121

113:                                              ; preds = %101
  %114 = icmp slt i32 %112, 0
  br i1 %114, label %129, label %115

115:                                              ; preds = %113
  %116 = bitcast i8* %104 to %struct.arc**
  %117 = bitcast i8* %43 to %struct.arc**
  %118 = tail call i32 @arc_compare(%struct.arc** %116, %struct.arc** %117) #12
  %119 = icmp slt i32 %118, 0
  %120 = select i1 %119, i8* %43, i8* %104
  br label %129

121:                                              ; preds = %101
  %122 = icmp sgt i32 %112, 0
  br i1 %122, label %129, label %123

123:                                              ; preds = %121
  %124 = bitcast i8* %104 to %struct.arc**
  %125 = bitcast i8* %43 to %struct.arc**
  %126 = tail call i32 @arc_compare(%struct.arc** %124, %struct.arc** %125) #12
  %127 = icmp slt i32 %126, 0
  %128 = select i1 %127, i8* %104, i8* %43
  br label %129

129:                                              ; preds = %123, %121, %115, %113, %40
  %130 = phi i8* [ %43, %40 ], [ %120, %115 ], [ %128, %123 ], [ %105, %113 ], [ %105, %121 ]
  %131 = phi i8* [ %38, %40 ], [ %102, %115 ], [ %102, %123 ], [ %102, %113 ], [ %102, %121 ]
  %132 = phi i8* [ %34, %40 ], [ %74, %115 ], [ %74, %123 ], [ %74, %113 ], [ %74, %121 ]
  call void @llvm.dbg.value(metadata i8* %104, metadata !706, metadata !DIExpression())
  call void @llvm.dbg.value(metadata i8* %105, metadata !711, metadata !DIExpression())
  call void @llvm.dbg.value(metadata i8* %43, metadata !712, metadata !DIExpression())
  call void @llvm.dbg.value(metadata i32 (i8*, i8*)* bitcast (i32 (%struct.arc**, %struct.arc**)* @arc_compare to i32 (i8*, i8*)*), metadata !713, metadata !DIExpression())
  %133 = bitcast i8* %132 to %struct.arc**
  %134 = bitcast i8* %131 to %struct.arc**
  %135 = tail call i32 @arc_compare(%struct.arc** %133, %struct.arc** %134) #12
  %136 = icmp slt i32 %135, 0
  %137 = bitcast i8* %131 to %struct.arc**
  %138 = bitcast i8* %130 to %struct.arc**
  %139 = tail call i32 @arc_compare(%struct.arc** %137, %struct.arc** %138) #12
  br i1 %136, label %140, label %148

140:                                              ; preds = %129
  %141 = icmp slt i32 %139, 0
  br i1 %141, label %156, label %142

142:                                              ; preds = %140
  %143 = bitcast i8* %132 to %struct.arc**
  %144 = bitcast i8* %130 to %struct.arc**
  %145 = tail call i32 @arc_compare(%struct.arc** %143, %struct.arc** %144) #12
  %146 = icmp slt i32 %145, 0
  %147 = select i1 %146, i8* %130, i8* %132
  br label %156

148:                                              ; preds = %129
  %149 = icmp sgt i32 %139, 0
  br i1 %149, label %156, label %150

150:                                              ; preds = %148
  %151 = bitcast i8* %132 to %struct.arc**
  %152 = bitcast i8* %130 to %struct.arc**
  %153 = tail call i32 @arc_compare(%struct.arc** %151, %struct.arc** %152) #12
  %154 = icmp slt i32 %153, 0
  %155 = select i1 %154, i8* %132, i8* %130
  br label %156

156:                                              ; preds = %32, %140, %142, %148, %150
  %157 = phi i8* [ %38, %32 ], [ %147, %142 ], [ %155, %150 ], [ %131, %140 ], [ %131, %148 ]
  call void @llvm.dbg.value(metadata i8* %157, metadata !572, metadata !DIExpression())
  %158 = bitcast i8* %34 to i64*
  %159 = load i64, i64* %158, align 8
  call void @llvm.dbg.value(metadata i64 %159, metadata !599, metadata !DIExpression())
  %160 = bitcast i8* %157 to i64*
  %161 = load i64, i64* %160, align 8
  store i64 %161, i64* %158, align 8
  store i64 %159, i64* %160, align 8
  %162 = getelementptr inbounds i8, i8* %34, i64 8
  call void @llvm.dbg.value(metadata i8* %162, metadata !568, metadata !DIExpression())
  call void @llvm.dbg.value(metadata i8* %162, metadata !567, metadata !DIExpression())
  %163 = shl i64 %35, 3
  %164 = add i64 %163, -8
  %165 = getelementptr inbounds i8, i8* %34, i64 %164
  call void @llvm.dbg.value(metadata i8* %165, metadata !570, metadata !DIExpression())
  call void @llvm.dbg.value(metadata i8* %165, metadata !569, metadata !DIExpression())
  br label %166

166:                                              ; preds = %220, %156
  %167 = phi i32 [ 0, %156 ], [ 1, %220 ]
  %168 = phi i8* [ %165, %156 ], [ %201, %220 ]
  %169 = phi i8* [ %165, %156 ], [ %226, %220 ]
  %170 = phi i8* [ %162, %156 ], [ %225, %220 ]
  %171 = phi i8* [ %162, %156 ], [ %197, %220 ]
  call void @llvm.dbg.value(metadata i8* %171, metadata !567, metadata !DIExpression())
  call void @llvm.dbg.value(metadata i8* %170, metadata !568, metadata !DIExpression())
  call void @llvm.dbg.value(metadata i8* %169, metadata !569, metadata !DIExpression())
  call void @llvm.dbg.value(metadata i8* %168, metadata !570, metadata !DIExpression())
  call void @llvm.dbg.value(metadata i32 %167, metadata !579, metadata !DIExpression())
  %172 = icmp ugt i8* %170, %169
  br i1 %172, label %194, label %173

173:                                              ; preds = %166, %189
  %174 = phi i8* [ %191, %189 ], [ %171, %166 ]
  %175 = phi i8* [ %192, %189 ], [ %170, %166 ]
  %176 = phi i32 [ %190, %189 ], [ %167, %166 ]
  call void @llvm.dbg.value(metadata i8* %174, metadata !567, metadata !DIExpression())
  call void @llvm.dbg.value(metadata i8* %175, metadata !568, metadata !DIExpression())
  call void @llvm.dbg.value(metadata i32 %176, metadata !579, metadata !DIExpression())
  %177 = bitcast i8* %175 to %struct.arc**
  %178 = bitcast i8* %34 to %struct.arc**
  %179 = tail call i32 @arc_compare(%struct.arc** %177, %struct.arc** %178) #12
  call void @llvm.dbg.value(metadata i32 %179, metadata !576, metadata !DIExpression())
  %180 = icmp slt i32 %179, 1
  br i1 %180, label %181, label %194

181:                                              ; preds = %173
  %182 = icmp eq i32 %179, 0
  br i1 %182, label %183, label %189

183:                                              ; preds = %181
  call void @llvm.dbg.value(metadata i32 1, metadata !579, metadata !DIExpression())
  %184 = bitcast i8* %174 to i64*
  %185 = load i64, i64* %184, align 8
  call void @llvm.dbg.value(metadata i64 %185, metadata !606, metadata !DIExpression())
  %186 = bitcast i8* %175 to i64*
  %187 = load i64, i64* %186, align 8
  store i64 %187, i64* %184, align 8
  store i64 %185, i64* %186, align 8
  %188 = getelementptr inbounds i8, i8* %174, i64 8
  call void @llvm.dbg.value(metadata i8* %188, metadata !567, metadata !DIExpression())
  br label %189

189:                                              ; preds = %183, %181
  %190 = phi i32 [ 1, %183 ], [ %176, %181 ]
  %191 = phi i8* [ %188, %183 ], [ %174, %181 ]
  %192 = getelementptr inbounds i8, i8* %175, i64 8
  call void @llvm.dbg.value(metadata i8* %191, metadata !567, metadata !DIExpression())
  call void @llvm.dbg.value(metadata i8* %192, metadata !568, metadata !DIExpression())
  call void @llvm.dbg.value(metadata i32 %190, metadata !579, metadata !DIExpression())
  %193 = icmp ugt i8* %192, %169
  br i1 %193, label %194, label %173

194:                                              ; preds = %189, %173, %166
  %195 = phi i32 [ %167, %166 ], [ %176, %173 ], [ %190, %189 ]
  %196 = phi i8* [ %170, %166 ], [ %175, %173 ], [ %192, %189 ]
  %197 = phi i8* [ %171, %166 ], [ %174, %173 ], [ %191, %189 ]
  call void @llvm.dbg.value(metadata i32 %195, metadata !579, metadata !DIExpression())
  call void @llvm.dbg.value(metadata i32 %195, metadata !579, metadata !DIExpression())
  call void @llvm.dbg.value(metadata i8* %196, metadata !568, metadata !DIExpression())
  call void @llvm.dbg.value(metadata i8* %196, metadata !568, metadata !DIExpression())
  call void @llvm.dbg.value(metadata i8* %197, metadata !567, metadata !DIExpression())
  call void @llvm.dbg.value(metadata i8* %197, metadata !567, metadata !DIExpression())
  call void @llvm.dbg.value(metadata i8* %169, metadata !569, metadata !DIExpression())
  call void @llvm.dbg.value(metadata i8* %168, metadata !570, metadata !DIExpression())
  %198 = icmp ugt i8* %196, %169
  br i1 %198, label %227, label %199

199:                                              ; preds = %194, %215
  %200 = phi i8* [ %218, %215 ], [ %169, %194 ]
  %201 = phi i8* [ %217, %215 ], [ %168, %194 ]
  %202 = phi i32 [ %216, %215 ], [ %195, %194 ]
  call void @llvm.dbg.value(metadata i8* %200, metadata !569, metadata !DIExpression())
  call void @llvm.dbg.value(metadata i8* %201, metadata !570, metadata !DIExpression())
  call void @llvm.dbg.value(metadata i32 %202, metadata !579, metadata !DIExpression())
  %203 = bitcast i8* %200 to %struct.arc**
  %204 = bitcast i8* %34 to %struct.arc**
  %205 = tail call i32 @arc_compare(%struct.arc** %203, %struct.arc** %204) #12
  call void @llvm.dbg.value(metadata i32 %205, metadata !576, metadata !DIExpression())
  %206 = icmp sgt i32 %205, -1
  br i1 %206, label %207, label %220

207:                                              ; preds = %199
  %208 = icmp eq i32 %205, 0
  br i1 %208, label %209, label %215

209:                                              ; preds = %207
  call void @llvm.dbg.value(metadata i32 1, metadata !579, metadata !DIExpression())
  %210 = bitcast i8* %200 to i64*
  %211 = load i64, i64* %210, align 8
  call void @llvm.dbg.value(metadata i64 %211, metadata !625, metadata !DIExpression())
  %212 = bitcast i8* %201 to i64*
  %213 = load i64, i64* %212, align 8
  store i64 %213, i64* %210, align 8
  store i64 %211, i64* %212, align 8
  %214 = getelementptr inbounds i8, i8* %201, i64 -8
  call void @llvm.dbg.value(metadata i64 %211, metadata !625, metadata !DIExpression())
  br label %215

215:                                              ; preds = %209, %207
  %216 = phi i32 [ 1, %209 ], [ %202, %207 ]
  %217 = phi i8* [ %214, %209 ], [ %201, %207 ]
  %218 = getelementptr inbounds i8, i8* %200, i64 -8
  call void @llvm.dbg.value(metadata i8* %218, metadata !569, metadata !DIExpression())
  call void @llvm.dbg.value(metadata i8* %217, metadata !570, metadata !DIExpression())
  call void @llvm.dbg.value(metadata i32 %216, metadata !579, metadata !DIExpression())
  %219 = icmp ugt i8* %196, %218
  br i1 %219, label %227, label %199

220:                                              ; preds = %199
  call void @llvm.dbg.value(metadata i8* %201, metadata !570, metadata !DIExpression())
  call void @llvm.dbg.value(metadata i8* %200, metadata !569, metadata !DIExpression())
  %221 = bitcast i8* %196 to i64*
  %222 = load i64, i64* %221, align 8
  call void @llvm.dbg.value(metadata i64 %222, metadata !638, metadata !DIExpression())
  %223 = bitcast i8* %200 to i64*
  %224 = load i64, i64* %223, align 8
  store i64 %224, i64* %221, align 8
  store i64 %222, i64* %223, align 8
  call void @llvm.dbg.value(metadata i32 1, metadata !579, metadata !DIExpression())
  %225 = getelementptr inbounds i8, i8* %196, i64 8
  call void @llvm.dbg.value(metadata i8* %225, metadata !568, metadata !DIExpression())
  %226 = getelementptr inbounds i8, i8* %200, i64 -8
  call void @llvm.dbg.value(metadata i8* %226, metadata !569, metadata !DIExpression())
  br label %166

227:                                              ; preds = %215, %194
  %228 = phi i32 [ %216, %215 ], [ %195, %194 ]
  %229 = phi i8* [ %217, %215 ], [ %168, %194 ]
  %230 = phi i8* [ %218, %215 ], [ %169, %194 ]
  call void @llvm.dbg.value(metadata i32 %228, metadata !579, metadata !DIExpression())
  call void @llvm.dbg.value(metadata i8* %229, metadata !570, metadata !DIExpression())
  call void @llvm.dbg.value(metadata i8* %230, metadata !569, metadata !DIExpression())
  call void @llvm.dbg.value(metadata i8* %196, metadata !568, metadata !DIExpression())
  call void @llvm.dbg.value(metadata i8* %197, metadata !567, metadata !DIExpression())
  %231 = icmp eq i32 %228, 0
  %232 = shl i64 %35, 3
  %233 = getelementptr inbounds i8, i8* %34, i64 %232
  br i1 %231, label %234, label %255

234:                                              ; preds = %227
  call void @llvm.dbg.value(metadata i8* %34, metadata !563, metadata !DIExpression())
  call void @llvm.dbg.value(metadata i64 %35, metadata !564, metadata !DIExpression())
  call void @llvm.dbg.value(metadata i8* %34, metadata !563, metadata !DIExpression())
  call void @llvm.dbg.value(metadata i64 %35, metadata !564, metadata !DIExpression())
  call void @llvm.dbg.value(metadata i8* %34, metadata !563, metadata !DIExpression())
  call void @llvm.dbg.value(metadata i64 %35, metadata !564, metadata !DIExpression())
  call void @llvm.dbg.value(metadata i8* %34, metadata !563, metadata !DIExpression())
  call void @llvm.dbg.value(metadata i64 %35, metadata !564, metadata !DIExpression())
  call void @llvm.dbg.value(metadata i64 %35, metadata !564, metadata !DIExpression())
  call void @llvm.dbg.value(metadata i8* %34, metadata !563, metadata !DIExpression())
  call void @llvm.dbg.value(metadata i8* %162, metadata !572, metadata !DIExpression())
  %235 = icmp sgt i64 %232, 8
  br i1 %235, label %236, label %321

236:                                              ; preds = %252, %234
  %237 = phi i8* [ %253, %252 ], [ %162, %234 ]
  call void @llvm.dbg.value(metadata i8* %237, metadata !572, metadata !DIExpression())
  call void @llvm.dbg.value(metadata i8* %237, metadata !571, metadata !DIExpression())
  %238 = icmp ugt i8* %237, %34
  br i1 %238, label %239, label %252

239:                                              ; preds = %236, %246
  %240 = phi i8* [ %241, %246 ], [ %237, %236 ]
  call void @llvm.dbg.value(metadata i8* %240, metadata !571, metadata !DIExpression())
  %241 = getelementptr inbounds i8, i8* %240, i64 -8
  %242 = bitcast i8* %241 to %struct.arc**
  %243 = bitcast i8* %240 to %struct.arc**
  %244 = tail call i32 @arc_compare(%struct.arc** nonnull %242, %struct.arc** %243) #12
  %245 = icmp sgt i32 %244, 0
  br i1 %245, label %246, label %252

246:                                              ; preds = %239
  %247 = bitcast i8* %240 to i64*
  %248 = load i64, i64* %247, align 8
  call void @llvm.dbg.value(metadata i64 %248, metadata !645, metadata !DIExpression())
  %249 = bitcast i8* %241 to i64*
  %250 = load i64, i64* %249, align 8
  store i64 %250, i64* %247, align 8
  store i64 %248, i64* %249, align 8
  call void @llvm.dbg.value(metadata i8* %241, metadata !571, metadata !DIExpression())
  %251 = icmp ugt i8* %241, %34
  br i1 %251, label %239, label %252

252:                                              ; preds = %246, %239, %236
  %253 = getelementptr inbounds i8, i8* %237, i64 8
  call void @llvm.dbg.value(metadata i8* %253, metadata !572, metadata !DIExpression())
  %254 = icmp ult i8* %253, %233
  br i1 %254, label %236, label %321

255:                                              ; preds = %227
  call void @llvm.dbg.value(metadata i8* %233, metadata !573, metadata !DIExpression())
  %256 = ptrtoint i8* %197 to i64
  %257 = sub i64 %256, %33
  %258 = ptrtoint i8* %196 to i64
  %259 = sub i64 %258, %256
  %260 = icmp slt i64 %257, %259
  %261 = select i1 %260, i64 %257, i64 %259
  call void @llvm.dbg.value(metadata i64 %261, metadata !575, metadata !DIExpression())
  %262 = icmp eq i64 %261, 0
  br i1 %262, label %281, label %263

263:                                              ; preds = %255
  %264 = sub i64 0, %261
  %265 = getelementptr inbounds i8, i8* %196, i64 %264
  call void @llvm.dbg.value(metadata i8* %34, metadata !829, metadata !DIExpression())
  call void @llvm.dbg.value(metadata i8* %265, metadata !834, metadata !DIExpression())
  call void @llvm.dbg.value(metadata i32 undef, metadata !835, metadata !DIExpression())
  call void @llvm.dbg.value(metadata i32 undef, metadata !836, metadata !DIExpression())
  call void @llvm.dbg.value(metadata i32 undef, metadata !837, metadata !DIExpression())
  %266 = shl i64 %261, 32
  %267 = ashr exact i64 %266, 32
  %268 = lshr i64 %267, 3
  call void @llvm.dbg.value(metadata i64 %268, metadata !863, metadata !DIExpression())
  %269 = bitcast i8* %34 to i64*
  call void @llvm.dbg.value(metadata i64* %269, metadata !865, metadata !DIExpression())
  %270 = bitcast i8* %265 to i64*
  call void @llvm.dbg.value(metadata i64* %270, metadata !866, metadata !DIExpression())
  br label %271

271:                                              ; preds = %271, %263
  %272 = phi i64* [ %270, %263 ], [ %278, %271 ]
  %273 = phi i64* [ %269, %263 ], [ %277, %271 ]
  %274 = phi i64 [ %268, %263 ], [ %279, %271 ]
  call void @llvm.dbg.value(metadata i64 %274, metadata !863, metadata !DIExpression())
  call void @llvm.dbg.value(metadata i64* %273, metadata !865, metadata !DIExpression())
  call void @llvm.dbg.value(metadata i64* %272, metadata !866, metadata !DIExpression())
  %275 = load i64, i64* %273, align 8
  call void @llvm.dbg.value(metadata i64 %275, metadata !869, metadata !DIExpression())
  %276 = load i64, i64* %272, align 8
  %277 = getelementptr inbounds i64, i64* %273, i64 1
  call void @llvm.dbg.value(metadata i64 %275, metadata !869, metadata !DIExpression())
  store i64 %276, i64* %273, align 8
  %278 = getelementptr inbounds i64, i64* %272, i64 1
  call void @llvm.dbg.value(metadata i64* %278, metadata !866, metadata !DIExpression())
  store i64 %275, i64* %272, align 8
  %279 = add nsw i64 %274, -1
  call void @llvm.dbg.value(metadata i64 %279, metadata !863, metadata !DIExpression())
  %280 = icmp sgt i64 %279, 0
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
  call void @llvm.dbg.value(metadata i64 %279, metadata !863, metadata !DIExpression())
  %290 = icmp eq i64 %289, 0
  br i1 %290, label %309, label %291

291:                                              ; preds = %281
  %292 = sub i64 0, %289
  %293 = getelementptr inbounds i8, i8* %233, i64 %292
  call void @llvm.dbg.value(metadata i8* %196, metadata !829, metadata !DIExpression())
  call void @llvm.dbg.value(metadata i8* %293, metadata !834, metadata !DIExpression())
  call void @llvm.dbg.value(metadata i32 undef, metadata !835, metadata !DIExpression())
  call void @llvm.dbg.value(metadata i32 undef, metadata !836, metadata !DIExpression())
  call void @llvm.dbg.value(metadata i32 undef, metadata !837, metadata !DIExpression())
  %294 = shl i64 %289, 32
  %295 = ashr exact i64 %294, 32
  %296 = lshr i64 %295, 3
  call void @llvm.dbg.value(metadata i64 %296, metadata !863, metadata !DIExpression())
  %297 = bitcast i8* %196 to i64*
  call void @llvm.dbg.value(metadata i64* %297, metadata !865, metadata !DIExpression())
  %298 = bitcast i8* %293 to i64*
  call void @llvm.dbg.value(metadata i64* %298, metadata !866, metadata !DIExpression())
  br label %299

299:                                              ; preds = %299, %291
  %300 = phi i64* [ %298, %291 ], [ %306, %299 ]
  %301 = phi i64* [ %297, %291 ], [ %305, %299 ]
  %302 = phi i64 [ %296, %291 ], [ %307, %299 ]
  call void @llvm.dbg.value(metadata i64 %302, metadata !863, metadata !DIExpression())
  call void @llvm.dbg.value(metadata i64* %301, metadata !865, metadata !DIExpression())
  call void @llvm.dbg.value(metadata i64* %300, metadata !866, metadata !DIExpression())
  %303 = load i64, i64* %301, align 8
  call void @llvm.dbg.value(metadata i64 %303, metadata !869, metadata !DIExpression())
  %304 = load i64, i64* %300, align 8
  %305 = getelementptr inbounds i64, i64* %301, i64 1
  call void @llvm.dbg.value(metadata i64* %305, metadata !865, metadata !DIExpression())
  store i64 %304, i64* %301, align 8
  %306 = getelementptr inbounds i64, i64* %300, i64 1
  call void @llvm.dbg.value(metadata i64* %306, metadata !866, metadata !DIExpression())
  store i64 %303, i64* %300, align 8
  %307 = add nsw i64 %302, -1
  call void @llvm.dbg.value(metadata i64 %307, metadata !863, metadata !DIExpression())
  %308 = icmp sgt i64 %307, 0
  br i1 %308, label %299, label %309

309:                                              ; preds = %299, %281
  call void @llvm.dbg.value(metadata i64 %259, metadata !575, metadata !DIExpression())
  %310 = icmp ugt i64 %259, 8
  br i1 %310, label %311, label %313

311:                                              ; preds = %309
  %312 = lshr i64 %259, 3
  tail call fastcc void @qsort_sorter(i8* %34, i64 %312)
  br label %313

313:                                              ; preds = %311, %309
  call void @llvm.dbg.value(metadata i64 %284, metadata !575, metadata !DIExpression())
  %314 = icmp ugt i64 %284, 8
  br i1 %314, label %315, label %321

315:                                              ; preds = %313
  %316 = sub i64 0, %284
  %317 = getelementptr inbounds i8, i8* %233, i64 %316
  %318 = lshr i64 %284, 3
  call void @llvm.dbg.value(metadata i8* %317, metadata !563, metadata !DIExpression())
  call void @llvm.dbg.value(metadata i64 %318, metadata !564, metadata !DIExpression())
  call void @llvm.dbg.label(metadata !664)
  %319 = ptrtoint i8* %317 to i64
  call void @llvm.dbg.value(metadata i32 0, metadata !577, metadata !DIExpression())
  call void @llvm.dbg.value(metadata i32 0, metadata !579, metadata !DIExpression())
  %320 = icmp ult i64 %318, 7
  br i1 %320, label %5, label %32

321:                                              ; preds = %313, %252, %234, %29, %5
  ret void
}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(name: "basket_sizes", scope: !2, file: !3, line: 31, type: !57, isLocal: true, isDefinition: true)
!2 = distinct !DICompileUnit(language: DW_LANG_C99, file: !3, producer: "clang based Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !4, retainedTypes: !5, globals: !58, nameTableKind: None)
!3 = !DIFile(filename: "psimplex.c", directory: "/export/iusers/ayrivera/dev-505-debug/test/benchspec/CPU/505.mcf_r/build/build_base_core_avx512.0000")
!4 = !{}
!5 = !{!6, !11, !12, !57, !13}
!6 = !DIDerivedType(tag: DW_TAG_typedef, name: "flow_t", file: !7, line: 97, baseType: !8)
!7 = !DIFile(filename: "./defines.h", directory: "/export/iusers/ayrivera/dev-505-debug/test/benchspec/CPU/505.mcf_r/build/build_base_core_avx512.0000")
!8 = !DIDerivedType(tag: DW_TAG_typedef, name: "int64_t", file: !9, line: 197, baseType: !10)
!9 = !DIFile(filename: "/usr/include/sys/types.h", directory: "")
!10 = !DIBasicType(name: "long int", size: 64, encoding: DW_ATE_signed)
!11 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !12, size: 64)
!12 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !13, size: 64)
!13 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !14, size: 64)
!14 = !DIDerivedType(tag: DW_TAG_typedef, name: "BASKET", file: !7, line: 146, baseType: !15)
!15 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "basket", file: !7, line: 140, size: 256, elements: !16)
!16 = !{!17, !54, !55, !56}
!17 = !DIDerivedType(tag: DW_TAG_member, name: "a", scope: !15, file: !7, line: 142, baseType: !18, size: 64)
!18 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !19, size: 64)
!19 = !DIDerivedType(tag: DW_TAG_typedef, name: "arc_t", file: !7, line: 137, baseType: !20)
!20 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "arc", file: !7, line: 168, size: 576, elements: !21)
!21 = !{!22, !24, !26, !47, !48, !50, !51, !52, !53}
!22 = !DIDerivedType(tag: DW_TAG_member, name: "id", scope: !20, file: !7, line: 170, baseType: !23, size: 32)
!23 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!24 = !DIDerivedType(tag: DW_TAG_member, name: "cost", scope: !20, file: !7, line: 171, baseType: !25, size: 64, offset: 64)
!25 = !DIDerivedType(tag: DW_TAG_typedef, name: "cost_t", file: !7, line: 98, baseType: !8)
!26 = !DIDerivedType(tag: DW_TAG_member, name: "tail", scope: !20, file: !7, line: 172, baseType: !27, size: 64, offset: 128)
!27 = !DIDerivedType(tag: DW_TAG_typedef, name: "node_p", file: !7, line: 135, baseType: !28)
!28 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !29, size: 64)
!29 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "node", file: !7, line: 149, size: 832, elements: !30)
!30 = !{!31, !32, !33, !34, !35, !36, !37, !40, !41, !42, !43, !44, !45, !46}
!31 = !DIDerivedType(tag: DW_TAG_member, name: "potential", scope: !29, file: !7, line: 151, baseType: !25, size: 64)
!32 = !DIDerivedType(tag: DW_TAG_member, name: "orientation", scope: !29, file: !7, line: 152, baseType: !23, size: 32, offset: 64)
!33 = !DIDerivedType(tag: DW_TAG_member, name: "child", scope: !29, file: !7, line: 153, baseType: !27, size: 64, offset: 128)
!34 = !DIDerivedType(tag: DW_TAG_member, name: "pred", scope: !29, file: !7, line: 154, baseType: !27, size: 64, offset: 192)
!35 = !DIDerivedType(tag: DW_TAG_member, name: "sibling", scope: !29, file: !7, line: 155, baseType: !27, size: 64, offset: 256)
!36 = !DIDerivedType(tag: DW_TAG_member, name: "sibling_prev", scope: !29, file: !7, line: 156, baseType: !27, size: 64, offset: 320)
!37 = !DIDerivedType(tag: DW_TAG_member, name: "basic_arc", scope: !29, file: !7, line: 157, baseType: !38, size: 64, offset: 384)
!38 = !DIDerivedType(tag: DW_TAG_typedef, name: "arc_p", file: !7, line: 138, baseType: !39)
!39 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !20, size: 64)
!40 = !DIDerivedType(tag: DW_TAG_member, name: "firstout", scope: !29, file: !7, line: 158, baseType: !38, size: 64, offset: 448)
!41 = !DIDerivedType(tag: DW_TAG_member, name: "firstin", scope: !29, file: !7, line: 158, baseType: !38, size: 64, offset: 512)
!42 = !DIDerivedType(tag: DW_TAG_member, name: "arc_tmp", scope: !29, file: !7, line: 159, baseType: !38, size: 64, offset: 576)
!43 = !DIDerivedType(tag: DW_TAG_member, name: "flow", scope: !29, file: !7, line: 160, baseType: !6, size: 64, offset: 640)
!44 = !DIDerivedType(tag: DW_TAG_member, name: "depth", scope: !29, file: !7, line: 161, baseType: !8, size: 64, offset: 704)
!45 = !DIDerivedType(tag: DW_TAG_member, name: "number", scope: !29, file: !7, line: 162, baseType: !23, size: 32, offset: 768)
!46 = !DIDerivedType(tag: DW_TAG_member, name: "time", scope: !29, file: !7, line: 163, baseType: !23, size: 32, offset: 800)
!47 = !DIDerivedType(tag: DW_TAG_member, name: "head", scope: !20, file: !7, line: 172, baseType: !27, size: 64, offset: 192)
!48 = !DIDerivedType(tag: DW_TAG_member, name: "ident", scope: !20, file: !7, line: 173, baseType: !49, size: 16, offset: 256)
!49 = !DIBasicType(name: "short", size: 16, encoding: DW_ATE_signed)
!50 = !DIDerivedType(tag: DW_TAG_member, name: "nextout", scope: !20, file: !7, line: 174, baseType: !38, size: 64, offset: 320)
!51 = !DIDerivedType(tag: DW_TAG_member, name: "nextin", scope: !20, file: !7, line: 174, baseType: !38, size: 64, offset: 384)
!52 = !DIDerivedType(tag: DW_TAG_member, name: "flow", scope: !20, file: !7, line: 175, baseType: !6, size: 64, offset: 448)
!53 = !DIDerivedType(tag: DW_TAG_member, name: "org_cost", scope: !20, file: !7, line: 176, baseType: !25, size: 64, offset: 512)
!54 = !DIDerivedType(tag: DW_TAG_member, name: "cost", scope: !15, file: !7, line: 143, baseType: !25, size: 64, offset: 64)
!55 = !DIDerivedType(tag: DW_TAG_member, name: "abs_cost", scope: !15, file: !7, line: 144, baseType: !25, size: 64, offset: 128)
!56 = !DIDerivedType(tag: DW_TAG_member, name: "number", scope: !15, file: !7, line: 145, baseType: !8, size: 64, offset: 192)
!57 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !8, size: 64)
!58 = !{!59, !61, !0, !63, !65}
!59 = !DIGlobalVariableExpression(var: !60, expr: !DIExpression())
!60 = distinct !DIGlobalVariable(name: "opt_basket", scope: !2, file: !3, line: 29, type: !12, isLocal: true, isDefinition: true)
!61 = !DIGlobalVariableExpression(var: !62, expr: !DIExpression())
!62 = distinct !DIGlobalVariable(name: "perm_p", scope: !2, file: !3, line: 30, type: !11, isLocal: true, isDefinition: true)
!63 = !DIGlobalVariableExpression(var: !64, expr: !DIExpression())
!64 = distinct !DIGlobalVariable(name: "basket", scope: !2, file: !3, line: 33, type: !13, isLocal: true, isDefinition: true)
!65 = !DIGlobalVariableExpression(var: !66, expr: !DIExpression())
!66 = distinct !DIGlobalVariable(name: "opt", scope: !2, file: !3, line: 32, type: !8, isLocal: true, isDefinition: true)
!67 = !DIGlobalVariableExpression(var: !66, expr: !DIExpression(DW_OP_deref_size, 1, DW_OP_constu, 1, DW_OP_mul, DW_OP_constu, 0, DW_OP_plus, DW_OP_stack_value))
!68 = !DIGlobalVariableExpression(var: !69, expr: !DIExpression())
!69 = distinct !DIGlobalVariable(name: "net", scope: !70, file: !71, line: 30, type: !81, isLocal: false, isDefinition: true)
!70 = distinct !DICompileUnit(language: DW_LANG_C99, file: !71, producer: "clang based Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !4, retainedTypes: !72, globals: !80, nameTableKind: None)
!71 = !DIFile(filename: "mcf.c", directory: "/export/iusers/ayrivera/dev-505-debug/test/benchspec/CPU/505.mcf_r/build/build_base_core_avx512.0000")
!72 = !{!25, !73, !74, !8, !23, !77}
!73 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: null, size: 64)
!74 = !DIDerivedType(tag: DW_TAG_typedef, name: "size_t", file: !75, line: 51, baseType: !76)
!75 = !DIFile(filename: "deploy/linux_debug/lib/clang/10.0.0/include/stddef.h", directory: "/export/iusers/ayrivera/dev-505-debug")
!76 = !DIBasicType(name: "long unsigned int", size: 64, encoding: DW_ATE_unsigned)
!77 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !78, size: 64)
!78 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !79, size: 64)
!79 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_signed_char)
!80 = !{!68}
!81 = !DIDerivedType(tag: DW_TAG_typedef, name: "network_t", file: !7, line: 206, baseType: !82)
!82 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "network", file: !7, line: 181, size: 5184, elements: !83)
!83 = !{!84, !88, !89, !90, !91, !92, !93, !94, !95, !96, !97, !98, !99, !100, !101, !102, !103, !104, !105, !106, !108, !109, !141, !142, !143, !144, !145, !146, !147, !148, !149, !150, !151}
!84 = !DIDerivedType(tag: DW_TAG_member, name: "inputfile", scope: !82, file: !7, line: 183, baseType: !85, size: 1600)
!85 = !DICompositeType(tag: DW_TAG_array_type, baseType: !79, size: 1600, elements: !86)
!86 = !{!87}
!87 = !DISubrange(count: 200)
!88 = !DIDerivedType(tag: DW_TAG_member, name: "clustfile", scope: !82, file: !7, line: 184, baseType: !85, size: 1600, offset: 1600)
!89 = !DIDerivedType(tag: DW_TAG_member, name: "n", scope: !82, file: !7, line: 185, baseType: !8, size: 64, offset: 3200)
!90 = !DIDerivedType(tag: DW_TAG_member, name: "n_trips", scope: !82, file: !7, line: 185, baseType: !8, size: 64, offset: 3264)
!91 = !DIDerivedType(tag: DW_TAG_member, name: "max_m", scope: !82, file: !7, line: 186, baseType: !8, size: 64, offset: 3328)
!92 = !DIDerivedType(tag: DW_TAG_member, name: "m", scope: !82, file: !7, line: 186, baseType: !8, size: 64, offset: 3392)
!93 = !DIDerivedType(tag: DW_TAG_member, name: "m_org", scope: !82, file: !7, line: 186, baseType: !8, size: 64, offset: 3456)
!94 = !DIDerivedType(tag: DW_TAG_member, name: "m_impl", scope: !82, file: !7, line: 186, baseType: !8, size: 64, offset: 3520)
!95 = !DIDerivedType(tag: DW_TAG_member, name: "max_residual_new_m", scope: !82, file: !7, line: 187, baseType: !8, size: 64, offset: 3584)
!96 = !DIDerivedType(tag: DW_TAG_member, name: "max_new_m", scope: !82, file: !7, line: 187, baseType: !8, size: 64, offset: 3648)
!97 = !DIDerivedType(tag: DW_TAG_member, name: "primal_unbounded", scope: !82, file: !7, line: 189, baseType: !8, size: 64, offset: 3712)
!98 = !DIDerivedType(tag: DW_TAG_member, name: "dual_unbounded", scope: !82, file: !7, line: 190, baseType: !8, size: 64, offset: 3776)
!99 = !DIDerivedType(tag: DW_TAG_member, name: "perturbed", scope: !82, file: !7, line: 191, baseType: !8, size: 64, offset: 3840)
!100 = !DIDerivedType(tag: DW_TAG_member, name: "feasible", scope: !82, file: !7, line: 192, baseType: !8, size: 64, offset: 3904)
!101 = !DIDerivedType(tag: DW_TAG_member, name: "eps", scope: !82, file: !7, line: 193, baseType: !8, size: 64, offset: 3968)
!102 = !DIDerivedType(tag: DW_TAG_member, name: "opt_tol", scope: !82, file: !7, line: 194, baseType: !8, size: 64, offset: 4032)
!103 = !DIDerivedType(tag: DW_TAG_member, name: "feas_tol", scope: !82, file: !7, line: 195, baseType: !8, size: 64, offset: 4096)
!104 = !DIDerivedType(tag: DW_TAG_member, name: "pert_val", scope: !82, file: !7, line: 196, baseType: !8, size: 64, offset: 4160)
!105 = !DIDerivedType(tag: DW_TAG_member, name: "bigM", scope: !82, file: !7, line: 197, baseType: !8, size: 64, offset: 4224)
!106 = !DIDerivedType(tag: DW_TAG_member, name: "optcost", scope: !82, file: !7, line: 198, baseType: !107, size: 64, offset: 4288)
!107 = !DIBasicType(name: "double", size: 64, encoding: DW_ATE_float)
!108 = !DIDerivedType(tag: DW_TAG_member, name: "ignore_impl", scope: !82, file: !7, line: 199, baseType: !25, size: 64, offset: 4352)
!109 = !DIDerivedType(tag: DW_TAG_member, name: "nodes", scope: !82, file: !7, line: 200, baseType: !110, size: 64, offset: 4416)
!110 = !DIDerivedType(tag: DW_TAG_typedef, name: "node_p", file: !7, line: 135, baseType: !111)
!111 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !112, size: 64)
!112 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "node", file: !7, line: 149, size: 832, elements: !113)
!113 = !{!114, !115, !116, !117, !118, !119, !120, !134, !135, !136, !137, !138, !139, !140}
!114 = !DIDerivedType(tag: DW_TAG_member, name: "potential", scope: !112, file: !7, line: 151, baseType: !25, size: 64)
!115 = !DIDerivedType(tag: DW_TAG_member, name: "orientation", scope: !112, file: !7, line: 152, baseType: !23, size: 32, offset: 64)
!116 = !DIDerivedType(tag: DW_TAG_member, name: "child", scope: !112, file: !7, line: 153, baseType: !110, size: 64, offset: 128)
!117 = !DIDerivedType(tag: DW_TAG_member, name: "pred", scope: !112, file: !7, line: 154, baseType: !110, size: 64, offset: 192)
!118 = !DIDerivedType(tag: DW_TAG_member, name: "sibling", scope: !112, file: !7, line: 155, baseType: !110, size: 64, offset: 256)
!119 = !DIDerivedType(tag: DW_TAG_member, name: "sibling_prev", scope: !112, file: !7, line: 156, baseType: !110, size: 64, offset: 320)
!120 = !DIDerivedType(tag: DW_TAG_member, name: "basic_arc", scope: !112, file: !7, line: 157, baseType: !121, size: 64, offset: 384)
!121 = !DIDerivedType(tag: DW_TAG_typedef, name: "arc_p", file: !7, line: 138, baseType: !122)
!122 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !123, size: 64)
!123 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "arc", file: !7, line: 168, size: 576, elements: !124)
!124 = !{!125, !126, !127, !128, !129, !130, !131, !132, !133}
!125 = !DIDerivedType(tag: DW_TAG_member, name: "id", scope: !123, file: !7, line: 170, baseType: !23, size: 32)
!126 = !DIDerivedType(tag: DW_TAG_member, name: "cost", scope: !123, file: !7, line: 171, baseType: !25, size: 64, offset: 64)
!127 = !DIDerivedType(tag: DW_TAG_member, name: "tail", scope: !123, file: !7, line: 172, baseType: !110, size: 64, offset: 128)
!128 = !DIDerivedType(tag: DW_TAG_member, name: "head", scope: !123, file: !7, line: 172, baseType: !110, size: 64, offset: 192)
!129 = !DIDerivedType(tag: DW_TAG_member, name: "ident", scope: !123, file: !7, line: 173, baseType: !49, size: 16, offset: 256)
!130 = !DIDerivedType(tag: DW_TAG_member, name: "nextout", scope: !123, file: !7, line: 174, baseType: !121, size: 64, offset: 320)
!131 = !DIDerivedType(tag: DW_TAG_member, name: "nextin", scope: !123, file: !7, line: 174, baseType: !121, size: 64, offset: 384)
!132 = !DIDerivedType(tag: DW_TAG_member, name: "flow", scope: !123, file: !7, line: 175, baseType: !6, size: 64, offset: 448)
!133 = !DIDerivedType(tag: DW_TAG_member, name: "org_cost", scope: !123, file: !7, line: 176, baseType: !25, size: 64, offset: 512)
!134 = !DIDerivedType(tag: DW_TAG_member, name: "firstout", scope: !112, file: !7, line: 158, baseType: !121, size: 64, offset: 448)
!135 = !DIDerivedType(tag: DW_TAG_member, name: "firstin", scope: !112, file: !7, line: 158, baseType: !121, size: 64, offset: 512)
!136 = !DIDerivedType(tag: DW_TAG_member, name: "arc_tmp", scope: !112, file: !7, line: 159, baseType: !121, size: 64, offset: 576)
!137 = !DIDerivedType(tag: DW_TAG_member, name: "flow", scope: !112, file: !7, line: 160, baseType: !6, size: 64, offset: 640)
!138 = !DIDerivedType(tag: DW_TAG_member, name: "depth", scope: !112, file: !7, line: 161, baseType: !8, size: 64, offset: 704)
!139 = !DIDerivedType(tag: DW_TAG_member, name: "number", scope: !112, file: !7, line: 162, baseType: !23, size: 32, offset: 768)
!140 = !DIDerivedType(tag: DW_TAG_member, name: "time", scope: !112, file: !7, line: 163, baseType: !23, size: 32, offset: 800)
!141 = !DIDerivedType(tag: DW_TAG_member, name: "stop_nodes", scope: !82, file: !7, line: 200, baseType: !110, size: 64, offset: 4480)
!142 = !DIDerivedType(tag: DW_TAG_member, name: "arcs", scope: !82, file: !7, line: 201, baseType: !121, size: 64, offset: 4544)
!143 = !DIDerivedType(tag: DW_TAG_member, name: "stop_arcs", scope: !82, file: !7, line: 201, baseType: !121, size: 64, offset: 4608)
!144 = !DIDerivedType(tag: DW_TAG_member, name: "sorted_arcs", scope: !82, file: !7, line: 201, baseType: !121, size: 64, offset: 4672)
!145 = !DIDerivedType(tag: DW_TAG_member, name: "dummy_arcs", scope: !82, file: !7, line: 202, baseType: !121, size: 64, offset: 4736)
!146 = !DIDerivedType(tag: DW_TAG_member, name: "stop_dummy", scope: !82, file: !7, line: 202, baseType: !121, size: 64, offset: 4800)
!147 = !DIDerivedType(tag: DW_TAG_member, name: "iterations", scope: !82, file: !7, line: 203, baseType: !8, size: 64, offset: 4864)
!148 = !DIDerivedType(tag: DW_TAG_member, name: "bound_exchanges", scope: !82, file: !7, line: 204, baseType: !8, size: 64, offset: 4928)
!149 = !DIDerivedType(tag: DW_TAG_member, name: "nr_group", scope: !82, file: !7, line: 205, baseType: !8, size: 64, offset: 4992)
!150 = !DIDerivedType(tag: DW_TAG_member, name: "full_groups", scope: !82, file: !7, line: 205, baseType: !8, size: 64, offset: 5056)
!151 = !DIDerivedType(tag: DW_TAG_member, name: "max_elems", scope: !82, file: !7, line: 205, baseType: !8, size: 64, offset: 5120)
!152 = !DIGlobalVariableExpression(var: !153, expr: !DIExpression())
!153 = distinct !DIGlobalVariable(name: "full_group_end_arc", scope: !154, file: !155, line: 30, type: !163, isLocal: true, isDefinition: true)
!154 = distinct !DICompileUnit(language: DW_LANG_C99, file: !155, producer: "clang based Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !4, retainedTypes: !156, globals: !162, nameTableKind: None)
!155 = !DIFile(filename: "pbeampp.c", directory: "/export/iusers/ayrivera/dev-505-debug/test/benchspec/CPU/505.mcf_r/build/build_base_core_avx512.0000")
!156 = !{!157}
!157 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !158, size: 64)
!158 = !DISubroutineType(types: !159)
!159 = !{!23, !160, !160}
!160 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !161, size: 64)
!161 = !DIDerivedType(tag: DW_TAG_const_type, baseType: null)
!162 = !{!152}
!163 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !164, size: 64)
!164 = !DIDerivedType(tag: DW_TAG_typedef, name: "arc_t", file: !7, line: 137, baseType: !165)
!165 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "arc", file: !7, line: 168, size: 576, elements: !166)
!166 = !{!167, !168, !169, !190, !191, !192, !193, !194, !195}
!167 = !DIDerivedType(tag: DW_TAG_member, name: "id", scope: !165, file: !7, line: 170, baseType: !23, size: 32)
!168 = !DIDerivedType(tag: DW_TAG_member, name: "cost", scope: !165, file: !7, line: 171, baseType: !25, size: 64, offset: 64)
!169 = !DIDerivedType(tag: DW_TAG_member, name: "tail", scope: !165, file: !7, line: 172, baseType: !170, size: 64, offset: 128)
!170 = !DIDerivedType(tag: DW_TAG_typedef, name: "node_p", file: !7, line: 135, baseType: !171)
!171 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !172, size: 64)
!172 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "node", file: !7, line: 149, size: 832, elements: !173)
!173 = !{!174, !175, !176, !177, !178, !179, !180, !183, !184, !185, !186, !187, !188, !189}
!174 = !DIDerivedType(tag: DW_TAG_member, name: "potential", scope: !172, file: !7, line: 151, baseType: !25, size: 64)
!175 = !DIDerivedType(tag: DW_TAG_member, name: "orientation", scope: !172, file: !7, line: 152, baseType: !23, size: 32, offset: 64)
!176 = !DIDerivedType(tag: DW_TAG_member, name: "child", scope: !172, file: !7, line: 153, baseType: !170, size: 64, offset: 128)
!177 = !DIDerivedType(tag: DW_TAG_member, name: "pred", scope: !172, file: !7, line: 154, baseType: !170, size: 64, offset: 192)
!178 = !DIDerivedType(tag: DW_TAG_member, name: "sibling", scope: !172, file: !7, line: 155, baseType: !170, size: 64, offset: 256)
!179 = !DIDerivedType(tag: DW_TAG_member, name: "sibling_prev", scope: !172, file: !7, line: 156, baseType: !170, size: 64, offset: 320)
!180 = !DIDerivedType(tag: DW_TAG_member, name: "basic_arc", scope: !172, file: !7, line: 157, baseType: !181, size: 64, offset: 384)
!181 = !DIDerivedType(tag: DW_TAG_typedef, name: "arc_p", file: !7, line: 138, baseType: !182)
!182 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !165, size: 64)
!183 = !DIDerivedType(tag: DW_TAG_member, name: "firstout", scope: !172, file: !7, line: 158, baseType: !181, size: 64, offset: 448)
!184 = !DIDerivedType(tag: DW_TAG_member, name: "firstin", scope: !172, file: !7, line: 158, baseType: !181, size: 64, offset: 512)
!185 = !DIDerivedType(tag: DW_TAG_member, name: "arc_tmp", scope: !172, file: !7, line: 159, baseType: !181, size: 64, offset: 576)
!186 = !DIDerivedType(tag: DW_TAG_member, name: "flow", scope: !172, file: !7, line: 160, baseType: !6, size: 64, offset: 640)
!187 = !DIDerivedType(tag: DW_TAG_member, name: "depth", scope: !172, file: !7, line: 161, baseType: !8, size: 64, offset: 704)
!188 = !DIDerivedType(tag: DW_TAG_member, name: "number", scope: !172, file: !7, line: 162, baseType: !23, size: 32, offset: 768)
!189 = !DIDerivedType(tag: DW_TAG_member, name: "time", scope: !172, file: !7, line: 163, baseType: !23, size: 32, offset: 800)
!190 = !DIDerivedType(tag: DW_TAG_member, name: "head", scope: !165, file: !7, line: 172, baseType: !170, size: 64, offset: 192)
!191 = !DIDerivedType(tag: DW_TAG_member, name: "ident", scope: !165, file: !7, line: 173, baseType: !49, size: 16, offset: 256)
!192 = !DIDerivedType(tag: DW_TAG_member, name: "nextout", scope: !165, file: !7, line: 174, baseType: !181, size: 64, offset: 320)
!193 = !DIDerivedType(tag: DW_TAG_member, name: "nextin", scope: !165, file: !7, line: 174, baseType: !181, size: 64, offset: 384)
!194 = !DIDerivedType(tag: DW_TAG_member, name: "flow", scope: !165, file: !7, line: 175, baseType: !6, size: 64, offset: 448)
!195 = !DIDerivedType(tag: DW_TAG_member, name: "org_cost", scope: !165, file: !7, line: 176, baseType: !25, size: 64, offset: 512)
!196 = distinct !DICompileUnit(language: DW_LANG_C99, file: !197, producer: "clang based Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !4, retainedTypes: !198, globals: !234, nameTableKind: None)
!197 = !DIFile(filename: "mcfutil.c", directory: "/export/iusers/ayrivera/dev-505-debug/test/benchspec/CPU/505.mcf_r/build/build_base_core_avx512.0000")
!198 = !{!73, !199, !232, !6, !107, !25}
!199 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !200, size: 64)
!200 = !DIDerivedType(tag: DW_TAG_typedef, name: "node_t", file: !7, line: 134, baseType: !201)
!201 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "node", file: !7, line: 149, size: 832, elements: !202)
!202 = !{!203, !204, !205, !208, !209, !210, !211, !225, !226, !227, !228, !229, !230, !231}
!203 = !DIDerivedType(tag: DW_TAG_member, name: "potential", scope: !201, file: !7, line: 151, baseType: !25, size: 64)
!204 = !DIDerivedType(tag: DW_TAG_member, name: "orientation", scope: !201, file: !7, line: 152, baseType: !23, size: 32, offset: 64)
!205 = !DIDerivedType(tag: DW_TAG_member, name: "child", scope: !201, file: !7, line: 153, baseType: !206, size: 64, offset: 128)
!206 = !DIDerivedType(tag: DW_TAG_typedef, name: "node_p", file: !7, line: 135, baseType: !207)
!207 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !201, size: 64)
!208 = !DIDerivedType(tag: DW_TAG_member, name: "pred", scope: !201, file: !7, line: 154, baseType: !206, size: 64, offset: 192)
!209 = !DIDerivedType(tag: DW_TAG_member, name: "sibling", scope: !201, file: !7, line: 155, baseType: !206, size: 64, offset: 256)
!210 = !DIDerivedType(tag: DW_TAG_member, name: "sibling_prev", scope: !201, file: !7, line: 156, baseType: !206, size: 64, offset: 320)
!211 = !DIDerivedType(tag: DW_TAG_member, name: "basic_arc", scope: !201, file: !7, line: 157, baseType: !212, size: 64, offset: 384)
!212 = !DIDerivedType(tag: DW_TAG_typedef, name: "arc_p", file: !7, line: 138, baseType: !213)
!213 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !214, size: 64)
!214 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "arc", file: !7, line: 168, size: 576, elements: !215)
!215 = !{!216, !217, !218, !219, !220, !221, !222, !223, !224}
!216 = !DIDerivedType(tag: DW_TAG_member, name: "id", scope: !214, file: !7, line: 170, baseType: !23, size: 32)
!217 = !DIDerivedType(tag: DW_TAG_member, name: "cost", scope: !214, file: !7, line: 171, baseType: !25, size: 64, offset: 64)
!218 = !DIDerivedType(tag: DW_TAG_member, name: "tail", scope: !214, file: !7, line: 172, baseType: !206, size: 64, offset: 128)
!219 = !DIDerivedType(tag: DW_TAG_member, name: "head", scope: !214, file: !7, line: 172, baseType: !206, size: 64, offset: 192)
!220 = !DIDerivedType(tag: DW_TAG_member, name: "ident", scope: !214, file: !7, line: 173, baseType: !49, size: 16, offset: 256)
!221 = !DIDerivedType(tag: DW_TAG_member, name: "nextout", scope: !214, file: !7, line: 174, baseType: !212, size: 64, offset: 320)
!222 = !DIDerivedType(tag: DW_TAG_member, name: "nextin", scope: !214, file: !7, line: 174, baseType: !212, size: 64, offset: 384)
!223 = !DIDerivedType(tag: DW_TAG_member, name: "flow", scope: !214, file: !7, line: 175, baseType: !6, size: 64, offset: 448)
!224 = !DIDerivedType(tag: DW_TAG_member, name: "org_cost", scope: !214, file: !7, line: 176, baseType: !25, size: 64, offset: 512)
!225 = !DIDerivedType(tag: DW_TAG_member, name: "firstout", scope: !201, file: !7, line: 158, baseType: !212, size: 64, offset: 448)
!226 = !DIDerivedType(tag: DW_TAG_member, name: "firstin", scope: !201, file: !7, line: 158, baseType: !212, size: 64, offset: 512)
!227 = !DIDerivedType(tag: DW_TAG_member, name: "arc_tmp", scope: !201, file: !7, line: 159, baseType: !212, size: 64, offset: 576)
!228 = !DIDerivedType(tag: DW_TAG_member, name: "flow", scope: !201, file: !7, line: 160, baseType: !6, size: 64, offset: 640)
!229 = !DIDerivedType(tag: DW_TAG_member, name: "depth", scope: !201, file: !7, line: 161, baseType: !8, size: 64, offset: 704)
!230 = !DIDerivedType(tag: DW_TAG_member, name: "number", scope: !201, file: !7, line: 162, baseType: !23, size: 32, offset: 768)
!231 = !DIDerivedType(tag: DW_TAG_member, name: "time", scope: !201, file: !7, line: 163, baseType: !23, size: 32, offset: 800)
!232 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !233, size: 64)
!233 = !DIDerivedType(tag: DW_TAG_typedef, name: "arc_t", file: !7, line: 137, baseType: !214)
!234 = !{!235, !237}
!235 = !DIGlobalVariableExpression(var: !236, expr: !DIExpression())
!236 = distinct !DIGlobalVariable(name: "old_group", scope: !196, file: !197, line: 121, type: !8, isLocal: true, isDefinition: true)
!237 = !DIGlobalVariableExpression(var: !238, expr: !DIExpression())
!238 = distinct !DIGlobalVariable(name: "old_Arc", scope: !196, file: !197, line: 122, type: !8, isLocal: true, isDefinition: true)
!239 = distinct !DICompileUnit(language: DW_LANG_C99, file: !240, producer: "clang based Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !4, retainedTypes: !241, nameTableKind: None)
!240 = !DIFile(filename: "readmin.c", directory: "/export/iusers/ayrivera/dev-505-debug/test/benchspec/CPU/505.mcf_r/build/build_base_core_avx512.0000")
!241 = !{!73, !8, !107, !242, !275, !6, !25, !79}
!242 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !243, size: 64)
!243 = !DIDerivedType(tag: DW_TAG_typedef, name: "node_t", file: !7, line: 134, baseType: !244)
!244 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "node", file: !7, line: 149, size: 832, elements: !245)
!245 = !{!246, !247, !248, !251, !252, !253, !254, !268, !269, !270, !271, !272, !273, !274}
!246 = !DIDerivedType(tag: DW_TAG_member, name: "potential", scope: !244, file: !7, line: 151, baseType: !25, size: 64)
!247 = !DIDerivedType(tag: DW_TAG_member, name: "orientation", scope: !244, file: !7, line: 152, baseType: !23, size: 32, offset: 64)
!248 = !DIDerivedType(tag: DW_TAG_member, name: "child", scope: !244, file: !7, line: 153, baseType: !249, size: 64, offset: 128)
!249 = !DIDerivedType(tag: DW_TAG_typedef, name: "node_p", file: !7, line: 135, baseType: !250)
!250 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !244, size: 64)
!251 = !DIDerivedType(tag: DW_TAG_member, name: "pred", scope: !244, file: !7, line: 154, baseType: !249, size: 64, offset: 192)
!252 = !DIDerivedType(tag: DW_TAG_member, name: "sibling", scope: !244, file: !7, line: 155, baseType: !249, size: 64, offset: 256)
!253 = !DIDerivedType(tag: DW_TAG_member, name: "sibling_prev", scope: !244, file: !7, line: 156, baseType: !249, size: 64, offset: 320)
!254 = !DIDerivedType(tag: DW_TAG_member, name: "basic_arc", scope: !244, file: !7, line: 157, baseType: !255, size: 64, offset: 384)
!255 = !DIDerivedType(tag: DW_TAG_typedef, name: "arc_p", file: !7, line: 138, baseType: !256)
!256 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !257, size: 64)
!257 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "arc", file: !7, line: 168, size: 576, elements: !258)
!258 = !{!259, !260, !261, !262, !263, !264, !265, !266, !267}
!259 = !DIDerivedType(tag: DW_TAG_member, name: "id", scope: !257, file: !7, line: 170, baseType: !23, size: 32)
!260 = !DIDerivedType(tag: DW_TAG_member, name: "cost", scope: !257, file: !7, line: 171, baseType: !25, size: 64, offset: 64)
!261 = !DIDerivedType(tag: DW_TAG_member, name: "tail", scope: !257, file: !7, line: 172, baseType: !249, size: 64, offset: 128)
!262 = !DIDerivedType(tag: DW_TAG_member, name: "head", scope: !257, file: !7, line: 172, baseType: !249, size: 64, offset: 192)
!263 = !DIDerivedType(tag: DW_TAG_member, name: "ident", scope: !257, file: !7, line: 173, baseType: !49, size: 16, offset: 256)
!264 = !DIDerivedType(tag: DW_TAG_member, name: "nextout", scope: !257, file: !7, line: 174, baseType: !255, size: 64, offset: 320)
!265 = !DIDerivedType(tag: DW_TAG_member, name: "nextin", scope: !257, file: !7, line: 174, baseType: !255, size: 64, offset: 384)
!266 = !DIDerivedType(tag: DW_TAG_member, name: "flow", scope: !257, file: !7, line: 175, baseType: !6, size: 64, offset: 448)
!267 = !DIDerivedType(tag: DW_TAG_member, name: "org_cost", scope: !257, file: !7, line: 176, baseType: !25, size: 64, offset: 512)
!268 = !DIDerivedType(tag: DW_TAG_member, name: "firstout", scope: !244, file: !7, line: 158, baseType: !255, size: 64, offset: 448)
!269 = !DIDerivedType(tag: DW_TAG_member, name: "firstin", scope: !244, file: !7, line: 158, baseType: !255, size: 64, offset: 512)
!270 = !DIDerivedType(tag: DW_TAG_member, name: "arc_tmp", scope: !244, file: !7, line: 159, baseType: !255, size: 64, offset: 576)
!271 = !DIDerivedType(tag: DW_TAG_member, name: "flow", scope: !244, file: !7, line: 160, baseType: !6, size: 64, offset: 640)
!272 = !DIDerivedType(tag: DW_TAG_member, name: "depth", scope: !244, file: !7, line: 161, baseType: !8, size: 64, offset: 704)
!273 = !DIDerivedType(tag: DW_TAG_member, name: "number", scope: !244, file: !7, line: 162, baseType: !23, size: 32, offset: 768)
!274 = !DIDerivedType(tag: DW_TAG_member, name: "time", scope: !244, file: !7, line: 163, baseType: !23, size: 32, offset: 800)
!275 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !276, size: 64)
!276 = !DIDerivedType(tag: DW_TAG_typedef, name: "arc_t", file: !7, line: 137, baseType: !257)
!277 = distinct !DICompileUnit(language: DW_LANG_C99, file: !278, producer: "clang based Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !4, retainedTypes: !279, nameTableKind: None)
!278 = !DIFile(filename: "implicit.c", directory: "/export/iusers/ayrivera/dev-505-debug/test/benchspec/CPU/505.mcf_r/build/build_base_core_avx512.0000")
!279 = !{!280, !57, !313, !6, !25, !281, !315, !73, !8, !157}
!280 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !281, size: 64)
!281 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !282, size: 64)
!282 = !DIDerivedType(tag: DW_TAG_typedef, name: "arc_p", file: !7, line: 138, baseType: !283)
!283 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !284, size: 64)
!284 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "arc", file: !7, line: 168, size: 576, elements: !285)
!285 = !{!286, !287, !288, !307, !308, !309, !310, !311, !312}
!286 = !DIDerivedType(tag: DW_TAG_member, name: "id", scope: !284, file: !7, line: 170, baseType: !23, size: 32)
!287 = !DIDerivedType(tag: DW_TAG_member, name: "cost", scope: !284, file: !7, line: 171, baseType: !25, size: 64, offset: 64)
!288 = !DIDerivedType(tag: DW_TAG_member, name: "tail", scope: !284, file: !7, line: 172, baseType: !289, size: 64, offset: 128)
!289 = !DIDerivedType(tag: DW_TAG_typedef, name: "node_p", file: !7, line: 135, baseType: !290)
!290 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !291, size: 64)
!291 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "node", file: !7, line: 149, size: 832, elements: !292)
!292 = !{!293, !294, !295, !296, !297, !298, !299, !300, !301, !302, !303, !304, !305, !306}
!293 = !DIDerivedType(tag: DW_TAG_member, name: "potential", scope: !291, file: !7, line: 151, baseType: !25, size: 64)
!294 = !DIDerivedType(tag: DW_TAG_member, name: "orientation", scope: !291, file: !7, line: 152, baseType: !23, size: 32, offset: 64)
!295 = !DIDerivedType(tag: DW_TAG_member, name: "child", scope: !291, file: !7, line: 153, baseType: !289, size: 64, offset: 128)
!296 = !DIDerivedType(tag: DW_TAG_member, name: "pred", scope: !291, file: !7, line: 154, baseType: !289, size: 64, offset: 192)
!297 = !DIDerivedType(tag: DW_TAG_member, name: "sibling", scope: !291, file: !7, line: 155, baseType: !289, size: 64, offset: 256)
!298 = !DIDerivedType(tag: DW_TAG_member, name: "sibling_prev", scope: !291, file: !7, line: 156, baseType: !289, size: 64, offset: 320)
!299 = !DIDerivedType(tag: DW_TAG_member, name: "basic_arc", scope: !291, file: !7, line: 157, baseType: !282, size: 64, offset: 384)
!300 = !DIDerivedType(tag: DW_TAG_member, name: "firstout", scope: !291, file: !7, line: 158, baseType: !282, size: 64, offset: 448)
!301 = !DIDerivedType(tag: DW_TAG_member, name: "firstin", scope: !291, file: !7, line: 158, baseType: !282, size: 64, offset: 512)
!302 = !DIDerivedType(tag: DW_TAG_member, name: "arc_tmp", scope: !291, file: !7, line: 159, baseType: !282, size: 64, offset: 576)
!303 = !DIDerivedType(tag: DW_TAG_member, name: "flow", scope: !291, file: !7, line: 160, baseType: !6, size: 64, offset: 640)
!304 = !DIDerivedType(tag: DW_TAG_member, name: "depth", scope: !291, file: !7, line: 161, baseType: !8, size: 64, offset: 704)
!305 = !DIDerivedType(tag: DW_TAG_member, name: "number", scope: !291, file: !7, line: 162, baseType: !23, size: 32, offset: 768)
!306 = !DIDerivedType(tag: DW_TAG_member, name: "time", scope: !291, file: !7, line: 163, baseType: !23, size: 32, offset: 800)
!307 = !DIDerivedType(tag: DW_TAG_member, name: "head", scope: !284, file: !7, line: 172, baseType: !289, size: 64, offset: 192)
!308 = !DIDerivedType(tag: DW_TAG_member, name: "ident", scope: !284, file: !7, line: 173, baseType: !49, size: 16, offset: 256)
!309 = !DIDerivedType(tag: DW_TAG_member, name: "nextout", scope: !284, file: !7, line: 174, baseType: !282, size: 64, offset: 320)
!310 = !DIDerivedType(tag: DW_TAG_member, name: "nextin", scope: !284, file: !7, line: 174, baseType: !282, size: 64, offset: 384)
!311 = !DIDerivedType(tag: DW_TAG_member, name: "flow", scope: !284, file: !7, line: 175, baseType: !6, size: 64, offset: 448)
!312 = !DIDerivedType(tag: DW_TAG_member, name: "org_cost", scope: !284, file: !7, line: 176, baseType: !25, size: 64, offset: 512)
!313 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !314, size: 64)
!314 = !DIDerivedType(tag: DW_TAG_typedef, name: "arc_t", file: !7, line: 137, baseType: !284)
!315 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !316, size: 64)
!316 = !DIDerivedType(tag: DW_TAG_typedef, name: "list_elem", file: !7, line: 212, baseType: !317)
!317 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "list_elem", file: !7, line: 208, size: 128, elements: !318)
!318 = !{!319, !320}
!319 = !DIDerivedType(tag: DW_TAG_member, name: "arc", scope: !317, file: !7, line: 210, baseType: !313, size: 64)
!320 = !DIDerivedType(tag: DW_TAG_member, name: "next", scope: !317, file: !7, line: 211, baseType: !321, size: 64, offset: 64)
!321 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !317, size: 64)
!322 = distinct !DICompileUnit(language: DW_LANG_C99, file: !323, producer: "clang based Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !4, retainedTypes: !324, nameTableKind: None)
!323 = !DIFile(filename: "pstart.c", directory: "/export/iusers/ayrivera/dev-505-debug/test/benchspec/CPU/505.mcf_r/build/build_base_core_avx512.0000")
!324 = !{!25, !8, !6}
!325 = distinct !DICompileUnit(language: DW_LANG_C99, file: !326, producer: "clang based Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !4, retainedTypes: !327, nameTableKind: None)
!326 = !DIFile(filename: "output.c", directory: "/export/iusers/ayrivera/dev-505-debug/test/benchspec/CPU/505.mcf_r/build/build_base_core_avx512.0000")
!327 = !{!73}
!328 = distinct !DICompileUnit(language: DW_LANG_C99, file: !329, producer: "clang based Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !4, nameTableKind: None)
!329 = !DIFile(filename: "treeup.c", directory: "/export/iusers/ayrivera/dev-505-debug/test/benchspec/CPU/505.mcf_r/build/build_base_core_avx512.0000")
!330 = distinct !DICompileUnit(language: DW_LANG_C99, file: !331, producer: "clang based Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !4, retainedTypes: !332, nameTableKind: None)
!331 = !DIFile(filename: "pbla.c", directory: "/export/iusers/ayrivera/dev-505-debug/test/benchspec/CPU/505.mcf_r/build/build_base_core_avx512.0000")
!332 = !{!6}
!333 = distinct !DICompileUnit(language: DW_LANG_C99, file: !334, producer: "clang based Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !4, retainedTypes: !332, nameTableKind: None)
!334 = !DIFile(filename: "pflowup.c", directory: "/export/iusers/ayrivera/dev-505-debug/test/benchspec/CPU/505.mcf_r/build/build_base_core_avx512.0000")
!335 = distinct !DICompileUnit(language: DW_LANG_C99, file: !336, producer: "clang based Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !4, retainedTypes: !337, nameTableKind: None)
!336 = !DIFile(filename: "spec_qsort/spec_qsort/spec_qsort.c", directory: "/export/iusers/ayrivera/dev-505-debug/test/benchspec/CPU/505.mcf_r/build/build_base_core_avx512.0000")
!337 = !{!78, !338, !339, !8}
!338 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !10, size: 64)
!339 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !23, size: 64)
!340 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!341 = !{i32 7, !"Dwarf Version", i32 4}
!342 = !{i32 2, !"Debug Info Version", i32 3}
!343 = !{i32 1, !"wchar_size", i32 4}
!344 = !{i32 1, !"ThinLTO", i32 0}
!345 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!346 = !{i32 1, !"LTOPostLink", i32 1}
!347 = distinct !DISubprogram(name: "global_opt", scope: !71, file: !71, line: 37, type: !348, scopeLine: 41, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !70, retainedNodes: !350)
!348 = !DISubroutineType(types: !349)
!349 = !{!8}
!350 = !{!351, !352}
!351 = !DILocalVariable(name: "new_arcs", scope: !347, file: !71, line: 42, type: !8)
!352 = !DILocalVariable(name: "residual_nb_it", scope: !347, file: !71, line: 43, type: !8)
!353 = !DILocation(line: 0, scope: !347)
!354 = !DILocation(line: 48, column: 3, scope: !347)
!355 = !DILocation(line: 51, column: 63, scope: !356)
!356 = distinct !DILexicalBlock(scope: !347, file: !71, line: 49, column: 3)
!357 = !{!358, !362, i64 424}
!358 = !{!"struct@network", !359, i64 0, !359, i64 200, !362, i64 400, !362, i64 408, !362, i64 416, !362, i64 424, !362, i64 432, !362, i64 440, !362, i64 448, !362, i64 456, !362, i64 464, !362, i64 472, !362, i64 480, !362, i64 488, !362, i64 496, !362, i64 504, !362, i64 512, !362, i64 520, !362, i64 528, !363, i64 536, !362, i64 544, !364, i64 552, !364, i64 560, !365, i64 568, !365, i64 576, !365, i64 584, !365, i64 592, !365, i64 600, !362, i64 608, !362, i64 616, !362, i64 624, !362, i64 632, !362, i64 640}
!359 = !{!"array@_ZTSA200_c", !360, i64 0}
!360 = !{!"omnipotent char", !361, i64 0}
!361 = !{!"Simple C/C++ TBAA"}
!362 = !{!"long", !360, i64 0}
!363 = !{!"double", !360, i64 0}
!364 = !{!"pointer@_ZTSP4node", !360, i64 0}
!365 = !{!"pointer@_ZTSP3arc", !360, i64 0}
!366 = !DILocation(line: 51, column: 5, scope: !356)
!367 = !DILocation(line: 54, column: 5, scope: !356)
!368 = !DILocation(line: 57, column: 63, scope: !356)
!369 = !{!358, !362, i64 608}
!370 = !DILocation(line: 57, column: 5, scope: !356)
!371 = !DILocation(line: 58, column: 53, scope: !356)
!372 = !DILocation(line: 58, column: 5, scope: !356)
!373 = !DILocation(line: 67, column: 13, scope: !374)
!374 = distinct !DILexicalBlock(scope: !356, file: !71, line: 67, column: 9)
!375 = !{!358, !362, i64 440}
!376 = !DILocation(line: 67, column: 9, scope: !374)
!377 = !DILocation(line: 67, column: 9, scope: !356)
!378 = !DILocation(line: 69, column: 18, scope: !379)
!379 = distinct !DILexicalBlock(scope: !374, file: !71, line: 68, column: 5)
!380 = !DILocation(line: 71, column: 21, scope: !381)
!381 = distinct !DILexicalBlock(scope: !379, file: !71, line: 71, column: 12)
!382 = !DILocation(line: 71, column: 12, scope: !379)
!383 = !DILocation(line: 74, column: 9, scope: !384)
!384 = distinct !DILexicalBlock(scope: !381, file: !71, line: 72, column: 7)
!385 = !DILocation(line: 77, column: 9, scope: !384)
!386 = !DILocation(line: 80, column: 11, scope: !387)
!387 = distinct !DILexicalBlock(scope: !379, file: !71, line: 80, column: 11)
!388 = !DILocation(line: 80, column: 11, scope: !379)
!389 = !DILocation(line: 81, column: 9, scope: !387)
!390 = !DILocation(line: 86, column: 58, scope: !391)
!391 = distinct !DILexicalBlock(scope: !374, file: !71, line: 85, column: 5)
!392 = !DILocation(line: 86, column: 7, scope: !391)
!393 = !DILocation(line: 90, column: 16, scope: !356)
!394 = !DILocation(line: 93, column: 9, scope: !356)
!395 = !DILocation(line: 94, column: 7, scope: !396)
!396 = distinct !DILexicalBlock(scope: !356, file: !71, line: 93, column: 9)
!397 = !DILocation(line: 97, column: 18, scope: !398)
!398 = distinct !DILexicalBlock(scope: !356, file: !71, line: 97, column: 9)
!399 = !DILocation(line: 97, column: 9, scope: !356)
!400 = !DILocation(line: 100, column: 7, scope: !401)
!401 = distinct !DILexicalBlock(scope: !398, file: !71, line: 98, column: 5)
!402 = !DILocation(line: 103, column: 7, scope: !401)
!403 = !DILocation(line: 113, column: 17, scope: !347)
!404 = !DILocation(line: 113, column: 15, scope: !347)
!405 = !{!358, !363, i64 536}
!406 = !DILocation(line: 114, column: 3, scope: !347)
!407 = distinct !DISubprogram(name: "main", scope: !71, file: !71, line: 125, type: !408, scopeLine: 131, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !70, retainedNodes: !410)
!408 = !DISubroutineType(types: !409)
!409 = !{!23, !23, !77}
!410 = !{!411, !412, !413, !414}
!411 = !DILocalVariable(name: "argc", arg: 1, scope: !407, file: !71, line: 125, type: !23)
!412 = !DILocalVariable(name: "argv", arg: 2, scope: !407, file: !71, line: 125, type: !77)
!413 = !DILocalVariable(name: "outnum", scope: !407, file: !71, line: 132, type: !23)
!414 = !DILocalVariable(name: "outfile", scope: !407, file: !71, line: 133, type: !415)
!415 = !DICompositeType(tag: DW_TAG_array_type, baseType: !79, size: 640, elements: !416)
!416 = !{!417}
!417 = !DISubrange(count: 80)
!418 = !DILocation(line: 0, scope: !407)
!419 = !DILocation(line: 133, column: 3, scope: !407)
!420 = !DILocation(line: 133, column: 8, scope: !407)
!421 = !DILocation(line: 134, column: 12, scope: !422)
!422 = distinct !DILexicalBlock(scope: !407, file: !71, line: 134, column: 7)
!423 = !DILocation(line: 134, column: 7, scope: !407)
!424 = !DILocation(line: 142, column: 3, scope: !407)
!425 = !DILocation(line: 143, column: 3, scope: !407)
!426 = !DILocation(line: 144, column: 3, scope: !407)
!427 = !DILocation(line: 145, column: 3, scope: !407)
!428 = !DILocation(line: 146, column: 3, scope: !407)
!429 = !DILocation(line: 148, column: 3, scope: !407)
!430 = !DILocation(line: 152, column: 3, scope: !407)
!431 = !DILocation(line: 153, column: 12, scope: !407)
!432 = !{!358, !362, i64 528}
!433 = !DILocation(line: 156, column: 26, scope: !407)
!434 = !{!435, !435, i64 0}
!435 = !{!"pointer@_ZTSPc", !360, i64 0}
!436 = !DILocation(line: 156, column: 3, scope: !407)
!437 = !DILocation(line: 157, column: 12, scope: !438)
!438 = distinct !DILexicalBlock(scope: !407, file: !71, line: 157, column: 7)
!439 = !DILocation(line: 157, column: 7, scope: !407)
!440 = !DILocation(line: 158, column: 20, scope: !441)
!441 = distinct !DILexicalBlock(scope: !438, file: !71, line: 157, column: 18)
!442 = !DILocalVariable(name: "__nptr", arg: 1, scope: !443, file: !444, line: 278, type: !447)
!443 = distinct !DISubprogram(name: "atoi", scope: !444, file: !444, line: 278, type: !445, scopeLine: 279, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !70, retainedNodes: !449)
!444 = !DIFile(filename: "/usr/include/stdlib.h", directory: "")
!445 = !DISubroutineType(types: !446)
!446 = !{!23, !447}
!447 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !448, size: 64)
!448 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !79)
!449 = !{!442}
!450 = !DILocation(line: 0, scope: !443, inlinedAt: !451)
!451 = distinct !DILocation(line: 158, column: 15, scope: !441)
!452 = !DILocation(line: 280, column: 16, scope: !443, inlinedAt: !451)
!453 = !DILocation(line: 280, column: 10, scope: !443, inlinedAt: !451)
!454 = !DILocation(line: 159, column: 6, scope: !441)
!455 = !DILocation(line: 160, column: 3, scope: !441)
!456 = !DILocation(line: 161, column: 6, scope: !457)
!457 = distinct !DILexicalBlock(scope: !438, file: !71, line: 160, column: 10)
!458 = !DILocation(line: 164, column: 7, scope: !459)
!459 = distinct !DILexicalBlock(scope: !407, file: !71, line: 164, column: 7)
!460 = !DILocation(line: 164, column: 7, scope: !407)
!461 = !DILocation(line: 166, column: 5, scope: !462)
!462 = distinct !DILexicalBlock(scope: !459, file: !71, line: 165, column: 3)
!463 = !DILocation(line: 167, column: 5, scope: !462)
!464 = !DILocation(line: 168, column: 5, scope: !462)
!465 = !DILocation(line: 180, column: 61, scope: !407)
!466 = !{!358, !362, i64 408}
!467 = !DILocation(line: 180, column: 3, scope: !407)
!468 = !DILocation(line: 184, column: 3, scope: !407)
!469 = !DILocation(line: 185, column: 3, scope: !407)
!470 = !DILocation(line: 189, column: 7, scope: !471)
!471 = distinct !DILexicalBlock(scope: !407, file: !71, line: 189, column: 7)
!472 = !DILocation(line: 189, column: 7, scope: !407)
!473 = !DILocation(line: 204, column: 3, scope: !407)
!474 = !DILocation(line: 208, column: 3, scope: !407)
!475 = !DILocation(line: 209, column: 1, scope: !407)
!476 = distinct !DISubprogram(name: "markBaskets", scope: !3, file: !3, line: 36, type: !477, scopeLine: 41, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !2, retainedNodes: !479)
!477 = !DISubroutineType(types: !478)
!478 = !{null, !8}
!479 = !{!480, !481, !482, !483, !484, !526}
!480 = !DILocalVariable(name: "num_threads", arg: 1, scope: !476, file: !3, line: 36, type: !8)
!481 = !DILocalVariable(name: "i", scope: !476, file: !3, line: 42, type: !8)
!482 = !DILocalVariable(name: "j", scope: !476, file: !3, line: 42, type: !8)
!483 = !DILocalVariable(name: "max_pos", scope: !476, file: !3, line: 42, type: !8)
!484 = !DILocalVariable(name: "max", scope: !476, file: !3, line: 43, type: !485)
!485 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !486, size: 64)
!486 = !DIDerivedType(tag: DW_TAG_typedef, name: "BASKET", file: !7, line: 146, baseType: !487)
!487 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "basket", file: !7, line: 140, size: 256, elements: !488)
!488 = !{!489, !523, !524, !525}
!489 = !DIDerivedType(tag: DW_TAG_member, name: "a", scope: !487, file: !7, line: 142, baseType: !490, size: 64)
!490 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !491, size: 64)
!491 = !DIDerivedType(tag: DW_TAG_typedef, name: "arc_t", file: !7, line: 137, baseType: !492)
!492 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "arc", file: !7, line: 168, size: 576, elements: !493)
!493 = !{!494, !495, !496, !517, !518, !519, !520, !521, !522}
!494 = !DIDerivedType(tag: DW_TAG_member, name: "id", scope: !492, file: !7, line: 170, baseType: !23, size: 32)
!495 = !DIDerivedType(tag: DW_TAG_member, name: "cost", scope: !492, file: !7, line: 171, baseType: !25, size: 64, offset: 64)
!496 = !DIDerivedType(tag: DW_TAG_member, name: "tail", scope: !492, file: !7, line: 172, baseType: !497, size: 64, offset: 128)
!497 = !DIDerivedType(tag: DW_TAG_typedef, name: "node_p", file: !7, line: 135, baseType: !498)
!498 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !499, size: 64)
!499 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "node", file: !7, line: 149, size: 832, elements: !500)
!500 = !{!501, !502, !503, !504, !505, !506, !507, !510, !511, !512, !513, !514, !515, !516}
!501 = !DIDerivedType(tag: DW_TAG_member, name: "potential", scope: !499, file: !7, line: 151, baseType: !25, size: 64)
!502 = !DIDerivedType(tag: DW_TAG_member, name: "orientation", scope: !499, file: !7, line: 152, baseType: !23, size: 32, offset: 64)
!503 = !DIDerivedType(tag: DW_TAG_member, name: "child", scope: !499, file: !7, line: 153, baseType: !497, size: 64, offset: 128)
!504 = !DIDerivedType(tag: DW_TAG_member, name: "pred", scope: !499, file: !7, line: 154, baseType: !497, size: 64, offset: 192)
!505 = !DIDerivedType(tag: DW_TAG_member, name: "sibling", scope: !499, file: !7, line: 155, baseType: !497, size: 64, offset: 256)
!506 = !DIDerivedType(tag: DW_TAG_member, name: "sibling_prev", scope: !499, file: !7, line: 156, baseType: !497, size: 64, offset: 320)
!507 = !DIDerivedType(tag: DW_TAG_member, name: "basic_arc", scope: !499, file: !7, line: 157, baseType: !508, size: 64, offset: 384)
!508 = !DIDerivedType(tag: DW_TAG_typedef, name: "arc_p", file: !7, line: 138, baseType: !509)
!509 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !492, size: 64)
!510 = !DIDerivedType(tag: DW_TAG_member, name: "firstout", scope: !499, file: !7, line: 158, baseType: !508, size: 64, offset: 448)
!511 = !DIDerivedType(tag: DW_TAG_member, name: "firstin", scope: !499, file: !7, line: 158, baseType: !508, size: 64, offset: 512)
!512 = !DIDerivedType(tag: DW_TAG_member, name: "arc_tmp", scope: !499, file: !7, line: 159, baseType: !508, size: 64, offset: 576)
!513 = !DIDerivedType(tag: DW_TAG_member, name: "flow", scope: !499, file: !7, line: 160, baseType: !6, size: 64, offset: 640)
!514 = !DIDerivedType(tag: DW_TAG_member, name: "depth", scope: !499, file: !7, line: 161, baseType: !8, size: 64, offset: 704)
!515 = !DIDerivedType(tag: DW_TAG_member, name: "number", scope: !499, file: !7, line: 162, baseType: !23, size: 32, offset: 768)
!516 = !DIDerivedType(tag: DW_TAG_member, name: "time", scope: !499, file: !7, line: 163, baseType: !23, size: 32, offset: 800)
!517 = !DIDerivedType(tag: DW_TAG_member, name: "head", scope: !492, file: !7, line: 172, baseType: !497, size: 64, offset: 192)
!518 = !DIDerivedType(tag: DW_TAG_member, name: "ident", scope: !492, file: !7, line: 173, baseType: !49, size: 16, offset: 256)
!519 = !DIDerivedType(tag: DW_TAG_member, name: "nextout", scope: !492, file: !7, line: 174, baseType: !508, size: 64, offset: 320)
!520 = !DIDerivedType(tag: DW_TAG_member, name: "nextin", scope: !492, file: !7, line: 174, baseType: !508, size: 64, offset: 384)
!521 = !DIDerivedType(tag: DW_TAG_member, name: "flow", scope: !492, file: !7, line: 175, baseType: !6, size: 64, offset: 448)
!522 = !DIDerivedType(tag: DW_TAG_member, name: "org_cost", scope: !492, file: !7, line: 176, baseType: !25, size: 64, offset: 512)
!523 = !DIDerivedType(tag: DW_TAG_member, name: "cost", scope: !487, file: !7, line: 143, baseType: !25, size: 64, offset: 64)
!524 = !DIDerivedType(tag: DW_TAG_member, name: "abs_cost", scope: !487, file: !7, line: 144, baseType: !25, size: 64, offset: 128)
!525 = !DIDerivedType(tag: DW_TAG_member, name: "number", scope: !487, file: !7, line: 145, baseType: !8, size: 64, offset: 192)
!526 = !DILocalVariable(name: "act", scope: !476, file: !3, line: 43, type: !13)
!527 = !DILocation(line: 0, scope: !476)
!528 = !DILocation(line: 46, column: 13, scope: !529)
!529 = distinct !DILexicalBlock(scope: !530, file: !3, line: 46, column: 11)
!530 = distinct !DILexicalBlock(scope: !531, file: !3, line: 45, column: 27)
!531 = distinct !DILexicalBlock(scope: !532, file: !3, line: 45, column: 5)
!532 = distinct !DILexicalBlock(scope: !476, file: !3, line: 45, column: 5)
!533 = !{!534, !534, i64 0}
!534 = !{!"pointer@_ZTSPPP6basket", !360, i64 0}
!535 = !DILocation(line: 45, column: 5, scope: !532)
!536 = !{!537, !537, i64 0}
!537 = !{!"pointer@_ZTSPP6basket", !360, i64 0}
!538 = !DILocation(line: 46, column: 12, scope: !529)
!539 = !{!540, !540, i64 0}
!540 = !{!"pointer@_ZTSP6basket", !360, i64 0}
!541 = !DILocation(line: 46, column: 25, scope: !529)
!542 = !{!543, !362, i64 24}
!543 = !{!"struct@basket", !365, i64 0, !362, i64 8, !362, i64 16, !362, i64 24}
!544 = !DILocation(line: 46, column: 32, scope: !529)
!545 = !DILocation(line: 63, column: 12, scope: !546)
!546 = distinct !DILexicalBlock(scope: !530, file: !3, line: 63, column: 11)
!547 = !DILocation(line: 63, column: 11, scope: !530)
!548 = !DILocation(line: 67, column: 14, scope: !530)
!549 = !DILocation(line: 67, column: 21, scope: !530)
!550 = !DILocation(line: 68, column: 26, scope: !530)
!551 = !DILocation(line: 45, column: 23, scope: !531)
!552 = !DILocation(line: 45, column: 17, scope: !531)
!553 = distinct !{!553, !535, !554}
!554 = !DILocation(line: 69, column: 5, scope: !532)
!555 = !DILocation(line: 70, column: 1, scope: !476)
!556 = distinct !DISubprogram(name: "spec_qsort", scope: !557, file: !557, line: 115, type: !558, scopeLine: 116, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !335, retainedNodes: !562)
!557 = !DIFile(filename: "spec_qsort/spec_qsort.c", directory: "/export/iusers/ayrivera/dev-505-debug/test/benchspec/CPU/505.mcf_r/build/build_base_core_avx512.0000")
!558 = !DISubroutineType(types: !559)
!559 = !{null, !73, !74, !74, !560}
!560 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !561, size: 64)
!561 = !DIDerivedType(tag: DW_TAG_typedef, name: "cmp_t", file: !557, line: 56, baseType: !158)
!562 = !{!563, !564, !565, !566, !567, !568, !569, !570, !571, !572, !573, !574, !575, !576, !577, !578, !579, !580, !589, !599, !602, !606, !615, !625, !631, !638, !641, !645, !654, !664}
!563 = !DILocalVariable(name: "a", arg: 1, scope: !556, file: !557, line: 115, type: !73)
!564 = !DILocalVariable(name: "n", arg: 2, scope: !556, file: !557, line: 115, type: !74)
!565 = !DILocalVariable(name: "es", arg: 3, scope: !556, file: !557, line: 115, type: !74)
!566 = !DILocalVariable(name: "cmp", arg: 4, scope: !556, file: !557, line: 115, type: !560)
!567 = !DILocalVariable(name: "pa", scope: !556, file: !557, line: 117, type: !78)
!568 = !DILocalVariable(name: "pb", scope: !556, file: !557, line: 117, type: !78)
!569 = !DILocalVariable(name: "pc", scope: !556, file: !557, line: 117, type: !78)
!570 = !DILocalVariable(name: "pd", scope: !556, file: !557, line: 117, type: !78)
!571 = !DILocalVariable(name: "pl", scope: !556, file: !557, line: 117, type: !78)
!572 = !DILocalVariable(name: "pm", scope: !556, file: !557, line: 117, type: !78)
!573 = !DILocalVariable(name: "pn", scope: !556, file: !557, line: 117, type: !78)
!574 = !DILocalVariable(name: "d", scope: !556, file: !557, line: 118, type: !74)
!575 = !DILocalVariable(name: "r", scope: !556, file: !557, line: 118, type: !74)
!576 = !DILocalVariable(name: "cmp_result", scope: !556, file: !557, line: 119, type: !23)
!577 = !DILocalVariable(name: "swaptype_long", scope: !556, file: !557, line: 120, type: !23)
!578 = !DILocalVariable(name: "swaptype_int", scope: !556, file: !557, line: 120, type: !23)
!579 = !DILocalVariable(name: "swap_cnt", scope: !556, file: !557, line: 120, type: !23)
!580 = !DILocalVariable(name: "t", scope: !581, file: !557, line: 130, type: !10)
!581 = distinct !DILexicalBlock(scope: !582, file: !557, line: 130, column: 33)
!582 = distinct !DILexicalBlock(scope: !583, file: !557, line: 130, column: 33)
!583 = distinct !DILexicalBlock(scope: !584, file: !557, line: 127, column: 25)
!584 = distinct !DILexicalBlock(scope: !585, file: !557, line: 127, column: 25)
!585 = distinct !DILexicalBlock(scope: !586, file: !557, line: 126, column: 17)
!586 = distinct !DILexicalBlock(scope: !587, file: !557, line: 126, column: 17)
!587 = distinct !DILexicalBlock(scope: !588, file: !557, line: 125, column: 20)
!588 = distinct !DILexicalBlock(scope: !556, file: !557, line: 125, column: 13)
!589 = !DILocalVariable(name: "t", scope: !590, file: !557, line: 130, type: !23)
!590 = distinct !DILexicalBlock(scope: !591, file: !557, line: 130, column: 33)
!591 = distinct !DILexicalBlock(scope: !592, file: !557, line: 130, column: 33)
!592 = distinct !DILexicalBlock(scope: !593, file: !557, line: 130, column: 33)
!593 = distinct !DILexicalBlock(scope: !594, file: !557, line: 127, column: 25)
!594 = distinct !DILexicalBlock(scope: !595, file: !557, line: 127, column: 25)
!595 = distinct !DILexicalBlock(scope: !596, file: !557, line: 126, column: 17)
!596 = distinct !DILexicalBlock(scope: !597, file: !557, line: 126, column: 17)
!597 = distinct !DILexicalBlock(scope: !598, file: !557, line: 125, column: 20)
!598 = distinct !DILexicalBlock(scope: !556, file: !557, line: 125, column: 13)
!599 = !DILocalVariable(name: "t", scope: !600, file: !557, line: 145, type: !10)
!600 = distinct !DILexicalBlock(scope: !601, file: !557, line: 145, column: 9)
!601 = distinct !DILexicalBlock(scope: !556, file: !557, line: 145, column: 9)
!602 = !DILocalVariable(name: "t", scope: !603, file: !557, line: 145, type: !23)
!603 = distinct !DILexicalBlock(scope: !604, file: !557, line: 145, column: 9)
!604 = distinct !DILexicalBlock(scope: !605, file: !557, line: 145, column: 9)
!605 = distinct !DILexicalBlock(scope: !556, file: !557, line: 145, column: 9)
!606 = !DILocalVariable(name: "t", scope: !607, file: !557, line: 153, type: !10)
!607 = distinct !DILexicalBlock(scope: !608, file: !557, line: 153, column: 33)
!608 = distinct !DILexicalBlock(scope: !609, file: !557, line: 153, column: 33)
!609 = distinct !DILexicalBlock(scope: !610, file: !557, line: 151, column: 46)
!610 = distinct !DILexicalBlock(scope: !611, file: !557, line: 151, column: 29)
!611 = distinct !DILexicalBlock(scope: !612, file: !557, line: 150, column: 68)
!612 = distinct !DILexicalBlock(scope: !613, file: !557, line: 149, column: 18)
!613 = distinct !DILexicalBlock(scope: !614, file: !557, line: 149, column: 9)
!614 = distinct !DILexicalBlock(scope: !556, file: !557, line: 149, column: 9)
!615 = !DILocalVariable(name: "t", scope: !616, file: !557, line: 153, type: !23)
!616 = distinct !DILexicalBlock(scope: !617, file: !557, line: 153, column: 33)
!617 = distinct !DILexicalBlock(scope: !618, file: !557, line: 153, column: 33)
!618 = distinct !DILexicalBlock(scope: !619, file: !557, line: 153, column: 33)
!619 = distinct !DILexicalBlock(scope: !620, file: !557, line: 151, column: 46)
!620 = distinct !DILexicalBlock(scope: !621, file: !557, line: 151, column: 29)
!621 = distinct !DILexicalBlock(scope: !622, file: !557, line: 150, column: 68)
!622 = distinct !DILexicalBlock(scope: !623, file: !557, line: 149, column: 18)
!623 = distinct !DILexicalBlock(scope: !624, file: !557, line: 149, column: 9)
!624 = distinct !DILexicalBlock(scope: !556, file: !557, line: 149, column: 9)
!625 = !DILocalVariable(name: "t", scope: !626, file: !557, line: 161, type: !10)
!626 = distinct !DILexicalBlock(scope: !627, file: !557, line: 161, column: 33)
!627 = distinct !DILexicalBlock(scope: !628, file: !557, line: 161, column: 33)
!628 = distinct !DILexicalBlock(scope: !629, file: !557, line: 159, column: 46)
!629 = distinct !DILexicalBlock(scope: !630, file: !557, line: 159, column: 29)
!630 = distinct !DILexicalBlock(scope: !612, file: !557, line: 158, column: 68)
!631 = !DILocalVariable(name: "t", scope: !632, file: !557, line: 161, type: !23)
!632 = distinct !DILexicalBlock(scope: !633, file: !557, line: 161, column: 33)
!633 = distinct !DILexicalBlock(scope: !634, file: !557, line: 161, column: 33)
!634 = distinct !DILexicalBlock(scope: !635, file: !557, line: 161, column: 33)
!635 = distinct !DILexicalBlock(scope: !636, file: !557, line: 159, column: 46)
!636 = distinct !DILexicalBlock(scope: !637, file: !557, line: 159, column: 29)
!637 = distinct !DILexicalBlock(scope: !622, file: !557, line: 158, column: 68)
!638 = !DILocalVariable(name: "t", scope: !639, file: !557, line: 168, type: !10)
!639 = distinct !DILexicalBlock(scope: !640, file: !557, line: 168, column: 17)
!640 = distinct !DILexicalBlock(scope: !612, file: !557, line: 168, column: 17)
!641 = !DILocalVariable(name: "t", scope: !642, file: !557, line: 168, type: !23)
!642 = distinct !DILexicalBlock(scope: !643, file: !557, line: 168, column: 17)
!643 = distinct !DILexicalBlock(scope: !644, file: !557, line: 168, column: 17)
!644 = distinct !DILexicalBlock(scope: !622, file: !557, line: 168, column: 17)
!645 = !DILocalVariable(name: "t", scope: !646, file: !557, line: 178, type: !10)
!646 = distinct !DILexicalBlock(scope: !647, file: !557, line: 178, column: 33)
!647 = distinct !DILexicalBlock(scope: !648, file: !557, line: 178, column: 33)
!648 = distinct !DILexicalBlock(scope: !649, file: !557, line: 175, column: 25)
!649 = distinct !DILexicalBlock(scope: !650, file: !557, line: 175, column: 25)
!650 = distinct !DILexicalBlock(scope: !651, file: !557, line: 174, column: 17)
!651 = distinct !DILexicalBlock(scope: !652, file: !557, line: 174, column: 17)
!652 = distinct !DILexicalBlock(scope: !653, file: !557, line: 173, column: 28)
!653 = distinct !DILexicalBlock(scope: !556, file: !557, line: 173, column: 13)
!654 = !DILocalVariable(name: "t", scope: !655, file: !557, line: 178, type: !23)
!655 = distinct !DILexicalBlock(scope: !656, file: !557, line: 178, column: 33)
!656 = distinct !DILexicalBlock(scope: !657, file: !557, line: 178, column: 33)
!657 = distinct !DILexicalBlock(scope: !658, file: !557, line: 178, column: 33)
!658 = distinct !DILexicalBlock(scope: !659, file: !557, line: 175, column: 25)
!659 = distinct !DILexicalBlock(scope: !660, file: !557, line: 175, column: 25)
!660 = distinct !DILexicalBlock(scope: !661, file: !557, line: 174, column: 17)
!661 = distinct !DILexicalBlock(scope: !662, file: !557, line: 174, column: 17)
!662 = distinct !DILexicalBlock(scope: !663, file: !557, line: 173, column: 28)
!663 = distinct !DILexicalBlock(scope: !556, file: !557, line: 173, column: 13)
!664 = !DILabel(scope: !556, name: "loop", file: !557, line: 122)
!665 = !DILocation(line: 0, scope: !556)
!666 = !DILocation(line: 122, column: 1, scope: !556)
!667 = !DILocation(line: 122, column: 9, scope: !556)
!668 = !DILocation(line: 125, column: 15, scope: !588)
!669 = !DILocation(line: 125, column: 13, scope: !556)
!670 = !DILocation(line: 126, column: 62, scope: !585)
!671 = !DILocation(line: 126, column: 58, scope: !585)
!672 = !{!360, !360, i64 0}
!673 = !DILocation(line: 126, column: 46, scope: !585)
!674 = !DILocation(line: 126, column: 17, scope: !586)
!675 = !DILocation(line: 0, scope: !586)
!676 = !DILocation(line: 128, column: 33, scope: !583)
!677 = !DILocation(line: 128, column: 45, scope: !583)
!678 = !DILocation(line: 128, column: 55, scope: !583)
!679 = !DILocation(line: 128, column: 48, scope: !583)
!680 = !DILocation(line: 128, column: 65, scope: !583)
!681 = !DILocation(line: 127, column: 25, scope: !584)
!682 = !DILocation(line: 130, column: 33, scope: !581)
!683 = !{!362, !362, i64 0}
!684 = !DILocation(line: 0, scope: !581)
!685 = distinct !{!685, !681, !686}
!686 = !DILocation(line: 130, column: 33, scope: !584)
!687 = distinct !{!687, !674, !688}
!688 = !DILocation(line: 130, column: 33, scope: !586)
!689 = !DILocation(line: 133, column: 29, scope: !556)
!690 = !DILocation(line: 133, column: 34, scope: !556)
!691 = !DILocation(line: 133, column: 24, scope: !556)
!692 = !DILocation(line: 134, column: 15, scope: !693)
!693 = distinct !DILexicalBlock(scope: !556, file: !557, line: 134, column: 13)
!694 = !DILocation(line: 134, column: 13, scope: !556)
!695 = !DILocation(line: 136, column: 42, scope: !696)
!696 = distinct !DILexicalBlock(scope: !693, file: !557, line: 134, column: 20)
!697 = !DILocation(line: 136, column: 32, scope: !696)
!698 = !DILocation(line: 137, column: 23, scope: !699)
!699 = distinct !DILexicalBlock(scope: !696, file: !557, line: 137, column: 21)
!700 = !DILocation(line: 137, column: 21, scope: !696)
!701 = !DILocation(line: 138, column: 37, scope: !702)
!702 = distinct !DILexicalBlock(scope: !699, file: !557, line: 137, column: 29)
!703 = !DILocation(line: 139, column: 42, scope: !702)
!704 = !DILocation(line: 139, column: 54, scope: !702)
!705 = !DILocation(line: 139, column: 50, scope: !702)
!706 = !DILocalVariable(name: "a", arg: 1, scope: !707, file: !557, line: 107, type: !78)
!707 = distinct !DISubprogram(name: "med3", scope: !557, file: !557, line: 107, type: !708, scopeLine: 108, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition | DISPFlagOptimized, unit: !335, retainedNodes: !710)
!708 = !DISubroutineType(types: !709)
!709 = !{!78, !78, !78, !78, !560}
!710 = !{!706, !711, !712, !713}
!711 = !DILocalVariable(name: "b", arg: 2, scope: !707, file: !557, line: 107, type: !78)
!712 = !DILocalVariable(name: "c", arg: 3, scope: !707, file: !557, line: 107, type: !78)
!713 = !DILocalVariable(name: "cmp", arg: 4, scope: !707, file: !557, line: 107, type: !560)
!714 = !DILocation(line: 0, scope: !707, inlinedAt: !715)
!715 = distinct !DILocation(line: 139, column: 30, scope: !702)
!716 = !DILocation(line: 109, column: 16, scope: !707, inlinedAt: !715)
!717 = !DILocation(line: 109, column: 26, scope: !707, inlinedAt: !715)
!718 = !DILocation(line: 110, column: 27, scope: !707, inlinedAt: !715)
!719 = !DILocation(line: 110, column: 17, scope: !707, inlinedAt: !715)
!720 = !DILocation(line: 110, column: 38, scope: !707, inlinedAt: !715)
!721 = !DILocation(line: 110, column: 48, scope: !707, inlinedAt: !715)
!722 = !DILocation(line: 111, column: 27, scope: !707, inlinedAt: !715)
!723 = !DILocation(line: 111, column: 17, scope: !707, inlinedAt: !715)
!724 = !DILocation(line: 111, column: 38, scope: !707, inlinedAt: !715)
!725 = !DILocation(line: 111, column: 48, scope: !707, inlinedAt: !715)
!726 = !DILocation(line: 140, column: 38, scope: !702)
!727 = !DILocation(line: 140, column: 50, scope: !702)
!728 = !DILocation(line: 0, scope: !707, inlinedAt: !729)
!729 = distinct !DILocation(line: 140, column: 30, scope: !702)
!730 = !DILocation(line: 109, column: 16, scope: !707, inlinedAt: !729)
!731 = !DILocation(line: 109, column: 26, scope: !707, inlinedAt: !729)
!732 = !DILocation(line: 110, column: 27, scope: !707, inlinedAt: !729)
!733 = !DILocation(line: 110, column: 17, scope: !707, inlinedAt: !729)
!734 = !DILocation(line: 110, column: 38, scope: !707, inlinedAt: !729)
!735 = !DILocation(line: 110, column: 48, scope: !707, inlinedAt: !729)
!736 = !DILocation(line: 111, column: 27, scope: !707, inlinedAt: !729)
!737 = !DILocation(line: 111, column: 17, scope: !707, inlinedAt: !729)
!738 = !DILocation(line: 111, column: 38, scope: !707, inlinedAt: !729)
!739 = !DILocation(line: 111, column: 48, scope: !707, inlinedAt: !729)
!740 = !DILocation(line: 141, column: 38, scope: !702)
!741 = !DILocation(line: 141, column: 50, scope: !702)
!742 = !DILocation(line: 0, scope: !707, inlinedAt: !743)
!743 = distinct !DILocation(line: 141, column: 30, scope: !702)
!744 = !DILocation(line: 109, column: 16, scope: !707, inlinedAt: !743)
!745 = !DILocation(line: 109, column: 26, scope: !707, inlinedAt: !743)
!746 = !DILocation(line: 110, column: 27, scope: !707, inlinedAt: !743)
!747 = !DILocation(line: 110, column: 17, scope: !707, inlinedAt: !743)
!748 = !DILocation(line: 110, column: 38, scope: !707, inlinedAt: !743)
!749 = !DILocation(line: 110, column: 48, scope: !707, inlinedAt: !743)
!750 = !DILocation(line: 111, column: 27, scope: !707, inlinedAt: !743)
!751 = !DILocation(line: 111, column: 17, scope: !707, inlinedAt: !743)
!752 = !DILocation(line: 111, column: 38, scope: !707, inlinedAt: !743)
!753 = !DILocation(line: 111, column: 48, scope: !707, inlinedAt: !743)
!754 = !DILocation(line: 0, scope: !707, inlinedAt: !755)
!755 = distinct !DILocation(line: 143, column: 22, scope: !696)
!756 = !DILocation(line: 109, column: 16, scope: !707, inlinedAt: !755)
!757 = !DILocation(line: 109, column: 26, scope: !707, inlinedAt: !755)
!758 = !DILocation(line: 110, column: 27, scope: !707, inlinedAt: !755)
!759 = !DILocation(line: 110, column: 17, scope: !707, inlinedAt: !755)
!760 = !DILocation(line: 110, column: 38, scope: !707, inlinedAt: !755)
!761 = !DILocation(line: 110, column: 48, scope: !707, inlinedAt: !755)
!762 = !DILocation(line: 111, column: 27, scope: !707, inlinedAt: !755)
!763 = !DILocation(line: 111, column: 17, scope: !707, inlinedAt: !755)
!764 = !DILocation(line: 111, column: 38, scope: !707, inlinedAt: !755)
!765 = !DILocation(line: 111, column: 48, scope: !707, inlinedAt: !755)
!766 = !DILocation(line: 145, column: 9, scope: !600)
!767 = !DILocation(line: 0, scope: !600)
!768 = !DILocation(line: 146, column: 29, scope: !556)
!769 = !DILocation(line: 148, column: 39, scope: !556)
!770 = !DILocation(line: 148, column: 29, scope: !556)
!771 = !DILocation(line: 149, column: 9, scope: !556)
!772 = !DILocation(line: 150, column: 27, scope: !612)
!773 = !DILocation(line: 150, column: 33, scope: !612)
!774 = !DILocation(line: 150, column: 50, scope: !612)
!775 = !DILocation(line: 150, column: 62, scope: !612)
!776 = !DILocation(line: 150, column: 17, scope: !612)
!777 = !DILocation(line: 151, column: 40, scope: !610)
!778 = !DILocation(line: 151, column: 29, scope: !611)
!779 = !DILocation(line: 153, column: 33, scope: !607)
!780 = !DILocation(line: 0, scope: !607)
!781 = !DILocation(line: 154, column: 36, scope: !609)
!782 = !DILocation(line: 155, column: 25, scope: !609)
!783 = !DILocation(line: 156, column: 28, scope: !611)
!784 = distinct !{!784, !776, !785}
!785 = !DILocation(line: 157, column: 17, scope: !612)
!786 = !DILocation(line: 158, column: 27, scope: !612)
!787 = !DILocation(line: 158, column: 33, scope: !612)
!788 = !DILocation(line: 158, column: 50, scope: !612)
!789 = !DILocation(line: 158, column: 62, scope: !612)
!790 = !DILocation(line: 158, column: 17, scope: !612)
!791 = !DILocation(line: 159, column: 40, scope: !629)
!792 = !DILocation(line: 159, column: 29, scope: !630)
!793 = !DILocation(line: 161, column: 33, scope: !626)
!794 = !DILocation(line: 0, scope: !626)
!795 = !DILocation(line: 162, column: 36, scope: !628)
!796 = !DILocation(line: 163, column: 25, scope: !628)
!797 = !DILocation(line: 164, column: 28, scope: !630)
!798 = distinct !{!798, !790, !799}
!799 = !DILocation(line: 165, column: 17, scope: !612)
!800 = !DILocation(line: 168, column: 17, scope: !639)
!801 = !DILocation(line: 0, scope: !639)
!802 = !DILocation(line: 170, column: 20, scope: !612)
!803 = !DILocation(line: 171, column: 20, scope: !612)
!804 = !DILocation(line: 149, column: 9, scope: !613)
!805 = distinct !{!805, !806, !807}
!806 = !DILocation(line: 149, column: 9, scope: !614)
!807 = !DILocation(line: 172, column: 9, scope: !614)
!808 = !DILocation(line: 173, column: 22, scope: !653)
!809 = !DILocation(line: 173, column: 13, scope: !556)
!810 = !DILocation(line: 174, column: 46, scope: !650)
!811 = !DILocation(line: 174, column: 17, scope: !651)
!812 = !DILocation(line: 176, column: 33, scope: !648)
!813 = !DILocation(line: 176, column: 45, scope: !648)
!814 = !DILocation(line: 176, column: 55, scope: !648)
!815 = !DILocation(line: 176, column: 48, scope: !648)
!816 = !DILocation(line: 176, column: 65, scope: !648)
!817 = !DILocation(line: 175, column: 25, scope: !649)
!818 = !DILocation(line: 178, column: 33, scope: !646)
!819 = !DILocation(line: 0, scope: !646)
!820 = distinct !{!820, !817, !821}
!821 = !DILocation(line: 178, column: 33, scope: !649)
!822 = !DILocation(line: 174, column: 71, scope: !650)
!823 = distinct !{!823, !811, !824}
!824 = !DILocation(line: 178, column: 33, scope: !651)
!825 = !DILocation(line: 183, column: 13, scope: !556)
!826 = !DILocation(line: 184, column: 9, scope: !827)
!827 = distinct !DILexicalBlock(scope: !556, file: !557, line: 184, column: 9)
!828 = !DILocation(line: 184, column: 9, scope: !556)
!829 = !DILocalVariable(name: "a", arg: 1, scope: !830, file: !557, line: 81, type: !78)
!830 = distinct !DISubprogram(name: "swapfunc", scope: !557, file: !557, line: 81, type: !831, scopeLine: 82, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition | DISPFlagOptimized, unit: !335, retainedNodes: !833)
!831 = !DISubroutineType(types: !832)
!832 = !{null, !78, !78, !23, !23, !23}
!833 = !{!829, !834, !835, !836, !837, !838, !841, !842, !843, !845, !848, !849, !850, !852, !854, !855, !856}
!834 = !DILocalVariable(name: "b", arg: 2, scope: !830, file: !557, line: 81, type: !78)
!835 = !DILocalVariable(name: "n", arg: 3, scope: !830, file: !557, line: 81, type: !23)
!836 = !DILocalVariable(name: "swaptype_long", arg: 4, scope: !830, file: !557, line: 81, type: !23)
!837 = !DILocalVariable(name: "swaptype_int", arg: 5, scope: !830, file: !557, line: 81, type: !23)
!838 = !DILocalVariable(name: "i", scope: !839, file: !557, line: 84, type: !10)
!839 = distinct !DILexicalBlock(scope: !840, file: !557, line: 84, column: 17)
!840 = distinct !DILexicalBlock(scope: !830, file: !557, line: 83, column: 13)
!841 = !DILocalVariable(name: "pi", scope: !839, file: !557, line: 84, type: !338)
!842 = !DILocalVariable(name: "pj", scope: !839, file: !557, line: 84, type: !338)
!843 = !DILocalVariable(name: "t", scope: !844, file: !557, line: 84, type: !10)
!844 = distinct !DILexicalBlock(scope: !839, file: !557, line: 84, column: 17)
!845 = !DILocalVariable(name: "i", scope: !846, file: !557, line: 86, type: !10)
!846 = distinct !DILexicalBlock(scope: !847, file: !557, line: 86, column: 17)
!847 = distinct !DILexicalBlock(scope: !840, file: !557, line: 85, column: 18)
!848 = !DILocalVariable(name: "pi", scope: !846, file: !557, line: 86, type: !339)
!849 = !DILocalVariable(name: "pj", scope: !846, file: !557, line: 86, type: !339)
!850 = !DILocalVariable(name: "t", scope: !851, file: !557, line: 86, type: !23)
!851 = distinct !DILexicalBlock(scope: !846, file: !557, line: 86, column: 17)
!852 = !DILocalVariable(name: "i", scope: !853, file: !557, line: 88, type: !10)
!853 = distinct !DILexicalBlock(scope: !847, file: !557, line: 88, column: 17)
!854 = !DILocalVariable(name: "pi", scope: !853, file: !557, line: 88, type: !78)
!855 = !DILocalVariable(name: "pj", scope: !853, file: !557, line: 88, type: !78)
!856 = !DILocalVariable(name: "t", scope: !857, file: !557, line: 88, type: !79)
!857 = distinct !DILexicalBlock(scope: !853, file: !557, line: 88, column: 17)
!858 = !DILocation(line: 0, scope: !830, inlinedAt: !859)
!859 = distinct !DILocation(line: 184, column: 9, scope: !827)
!860 = !DILocation(line: 84, column: 17, scope: !861, inlinedAt: !859)
!861 = distinct !DILexicalBlock(scope: !862, file: !557, line: 84, column: 17)
!862 = distinct !DILexicalBlock(scope: !830, file: !557, line: 83, column: 13)
!863 = !DILocalVariable(name: "i", scope: !861, file: !557, line: 84, type: !10)
!864 = !DILocation(line: 0, scope: !861, inlinedAt: !859)
!865 = !DILocalVariable(name: "pi", scope: !861, file: !557, line: 84, type: !338)
!866 = !DILocalVariable(name: "pj", scope: !861, file: !557, line: 84, type: !338)
!867 = !DILocation(line: 84, column: 17, scope: !868, inlinedAt: !859)
!868 = distinct !DILexicalBlock(scope: !861, file: !557, line: 84, column: 17)
!869 = !DILocalVariable(name: "t", scope: !868, file: !557, line: 84, type: !10)
!870 = !DILocation(line: 0, scope: !868, inlinedAt: !859)
!871 = distinct !{!871, !860, !860}
!872 = !DILocation(line: 185, column: 13, scope: !556)
!873 = !DILocation(line: 186, column: 9, scope: !874)
!874 = distinct !DILexicalBlock(scope: !556, file: !557, line: 186, column: 9)
!875 = !DILocation(line: 186, column: 9, scope: !556)
!876 = !DILocation(line: 0, scope: !830, inlinedAt: !877)
!877 = distinct !DILocation(line: 186, column: 9, scope: !874)
!878 = !DILocation(line: 84, column: 17, scope: !861, inlinedAt: !877)
!879 = !DILocation(line: 0, scope: !861, inlinedAt: !877)
!880 = !DILocation(line: 84, column: 17, scope: !868, inlinedAt: !877)
!881 = !DILocation(line: 0, scope: !868, inlinedAt: !877)
!882 = distinct !{!882, !878, !878}
!883 = !DILocation(line: 187, column: 27, scope: !884)
!884 = distinct !DILexicalBlock(scope: !556, file: !557, line: 187, column: 13)
!885 = !DILocation(line: 187, column: 13, scope: !556)
!886 = !DILocation(line: 188, column: 33, scope: !884)
!887 = !DILocation(line: 188, column: 17, scope: !884)
!888 = !DILocation(line: 189, column: 27, scope: !889)
!889 = distinct !DILexicalBlock(scope: !556, file: !557, line: 189, column: 13)
!890 = !DILocation(line: 189, column: 13, scope: !556)
!891 = !DILocation(line: 191, column: 24, scope: !892)
!892 = distinct !DILexicalBlock(scope: !889, file: !557, line: 189, column: 33)
!893 = !DILocation(line: 192, column: 23, scope: !892)
!894 = !DILocation(line: 196, column: 1, scope: !556)
!895 = distinct !DISubprogram(name: "spec_qsort", scope: !557, file: !557, line: 115, type: !558, scopeLine: 116, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !335, retainedNodes: !896)
!896 = !{!897, !898, !899, !900, !901, !902, !903, !904, !905, !906, !907, !908, !909, !910, !911, !912, !913, !914, !923, !933, !936, !940, !949, !959, !965, !972, !975, !979, !988, !998}
!897 = !DILocalVariable(name: "a", arg: 1, scope: !895, file: !557, line: 115, type: !73)
!898 = !DILocalVariable(name: "n", arg: 2, scope: !895, file: !557, line: 115, type: !74)
!899 = !DILocalVariable(name: "es", arg: 3, scope: !895, file: !557, line: 115, type: !74)
!900 = !DILocalVariable(name: "cmp", arg: 4, scope: !895, file: !557, line: 115, type: !560)
!901 = !DILocalVariable(name: "pa", scope: !895, file: !557, line: 117, type: !78)
!902 = !DILocalVariable(name: "pb", scope: !895, file: !557, line: 117, type: !78)
!903 = !DILocalVariable(name: "pc", scope: !895, file: !557, line: 117, type: !78)
!904 = !DILocalVariable(name: "pd", scope: !895, file: !557, line: 117, type: !78)
!905 = !DILocalVariable(name: "pl", scope: !895, file: !557, line: 117, type: !78)
!906 = !DILocalVariable(name: "pm", scope: !895, file: !557, line: 117, type: !78)
!907 = !DILocalVariable(name: "pn", scope: !895, file: !557, line: 117, type: !78)
!908 = !DILocalVariable(name: "d", scope: !895, file: !557, line: 118, type: !74)
!909 = !DILocalVariable(name: "r", scope: !895, file: !557, line: 118, type: !74)
!910 = !DILocalVariable(name: "cmp_result", scope: !895, file: !557, line: 119, type: !23)
!911 = !DILocalVariable(name: "swaptype_long", scope: !895, file: !557, line: 120, type: !23)
!912 = !DILocalVariable(name: "swaptype_int", scope: !895, file: !557, line: 120, type: !23)
!913 = !DILocalVariable(name: "swap_cnt", scope: !895, file: !557, line: 120, type: !23)
!914 = !DILocalVariable(name: "t", scope: !915, file: !557, line: 130, type: !10)
!915 = distinct !DILexicalBlock(scope: !916, file: !557, line: 130, column: 33)
!916 = distinct !DILexicalBlock(scope: !917, file: !557, line: 130, column: 33)
!917 = distinct !DILexicalBlock(scope: !918, file: !557, line: 127, column: 25)
!918 = distinct !DILexicalBlock(scope: !919, file: !557, line: 127, column: 25)
!919 = distinct !DILexicalBlock(scope: !920, file: !557, line: 126, column: 17)
!920 = distinct !DILexicalBlock(scope: !921, file: !557, line: 126, column: 17)
!921 = distinct !DILexicalBlock(scope: !922, file: !557, line: 125, column: 20)
!922 = distinct !DILexicalBlock(scope: !895, file: !557, line: 125, column: 13)
!923 = !DILocalVariable(name: "t", scope: !924, file: !557, line: 130, type: !23)
!924 = distinct !DILexicalBlock(scope: !925, file: !557, line: 130, column: 33)
!925 = distinct !DILexicalBlock(scope: !926, file: !557, line: 130, column: 33)
!926 = distinct !DILexicalBlock(scope: !927, file: !557, line: 130, column: 33)
!927 = distinct !DILexicalBlock(scope: !928, file: !557, line: 127, column: 25)
!928 = distinct !DILexicalBlock(scope: !929, file: !557, line: 127, column: 25)
!929 = distinct !DILexicalBlock(scope: !930, file: !557, line: 126, column: 17)
!930 = distinct !DILexicalBlock(scope: !931, file: !557, line: 126, column: 17)
!931 = distinct !DILexicalBlock(scope: !932, file: !557, line: 125, column: 20)
!932 = distinct !DILexicalBlock(scope: !895, file: !557, line: 125, column: 13)
!933 = !DILocalVariable(name: "t", scope: !934, file: !557, line: 145, type: !10)
!934 = distinct !DILexicalBlock(scope: !935, file: !557, line: 145, column: 9)
!935 = distinct !DILexicalBlock(scope: !895, file: !557, line: 145, column: 9)
!936 = !DILocalVariable(name: "t", scope: !937, file: !557, line: 145, type: !23)
!937 = distinct !DILexicalBlock(scope: !938, file: !557, line: 145, column: 9)
!938 = distinct !DILexicalBlock(scope: !939, file: !557, line: 145, column: 9)
!939 = distinct !DILexicalBlock(scope: !895, file: !557, line: 145, column: 9)
!940 = !DILocalVariable(name: "t", scope: !941, file: !557, line: 153, type: !10)
!941 = distinct !DILexicalBlock(scope: !942, file: !557, line: 153, column: 33)
!942 = distinct !DILexicalBlock(scope: !943, file: !557, line: 153, column: 33)
!943 = distinct !DILexicalBlock(scope: !944, file: !557, line: 151, column: 46)
!944 = distinct !DILexicalBlock(scope: !945, file: !557, line: 151, column: 29)
!945 = distinct !DILexicalBlock(scope: !946, file: !557, line: 150, column: 68)
!946 = distinct !DILexicalBlock(scope: !947, file: !557, line: 149, column: 18)
!947 = distinct !DILexicalBlock(scope: !948, file: !557, line: 149, column: 9)
!948 = distinct !DILexicalBlock(scope: !895, file: !557, line: 149, column: 9)
!949 = !DILocalVariable(name: "t", scope: !950, file: !557, line: 153, type: !23)
!950 = distinct !DILexicalBlock(scope: !951, file: !557, line: 153, column: 33)
!951 = distinct !DILexicalBlock(scope: !952, file: !557, line: 153, column: 33)
!952 = distinct !DILexicalBlock(scope: !953, file: !557, line: 153, column: 33)
!953 = distinct !DILexicalBlock(scope: !954, file: !557, line: 151, column: 46)
!954 = distinct !DILexicalBlock(scope: !955, file: !557, line: 151, column: 29)
!955 = distinct !DILexicalBlock(scope: !956, file: !557, line: 150, column: 68)
!956 = distinct !DILexicalBlock(scope: !957, file: !557, line: 149, column: 18)
!957 = distinct !DILexicalBlock(scope: !958, file: !557, line: 149, column: 9)
!958 = distinct !DILexicalBlock(scope: !895, file: !557, line: 149, column: 9)
!959 = !DILocalVariable(name: "t", scope: !960, file: !557, line: 161, type: !10)
!960 = distinct !DILexicalBlock(scope: !961, file: !557, line: 161, column: 33)
!961 = distinct !DILexicalBlock(scope: !962, file: !557, line: 161, column: 33)
!962 = distinct !DILexicalBlock(scope: !963, file: !557, line: 159, column: 46)
!963 = distinct !DILexicalBlock(scope: !964, file: !557, line: 159, column: 29)
!964 = distinct !DILexicalBlock(scope: !946, file: !557, line: 158, column: 68)
!965 = !DILocalVariable(name: "t", scope: !966, file: !557, line: 161, type: !23)
!966 = distinct !DILexicalBlock(scope: !967, file: !557, line: 161, column: 33)
!967 = distinct !DILexicalBlock(scope: !968, file: !557, line: 161, column: 33)
!968 = distinct !DILexicalBlock(scope: !969, file: !557, line: 161, column: 33)
!969 = distinct !DILexicalBlock(scope: !970, file: !557, line: 159, column: 46)
!970 = distinct !DILexicalBlock(scope: !971, file: !557, line: 159, column: 29)
!971 = distinct !DILexicalBlock(scope: !956, file: !557, line: 158, column: 68)
!972 = !DILocalVariable(name: "t", scope: !973, file: !557, line: 168, type: !10)
!973 = distinct !DILexicalBlock(scope: !974, file: !557, line: 168, column: 17)
!974 = distinct !DILexicalBlock(scope: !946, file: !557, line: 168, column: 17)
!975 = !DILocalVariable(name: "t", scope: !976, file: !557, line: 168, type: !23)
!976 = distinct !DILexicalBlock(scope: !977, file: !557, line: 168, column: 17)
!977 = distinct !DILexicalBlock(scope: !978, file: !557, line: 168, column: 17)
!978 = distinct !DILexicalBlock(scope: !956, file: !557, line: 168, column: 17)
!979 = !DILocalVariable(name: "t", scope: !980, file: !557, line: 178, type: !10)
!980 = distinct !DILexicalBlock(scope: !981, file: !557, line: 178, column: 33)
!981 = distinct !DILexicalBlock(scope: !982, file: !557, line: 178, column: 33)
!982 = distinct !DILexicalBlock(scope: !983, file: !557, line: 175, column: 25)
!983 = distinct !DILexicalBlock(scope: !984, file: !557, line: 175, column: 25)
!984 = distinct !DILexicalBlock(scope: !985, file: !557, line: 174, column: 17)
!985 = distinct !DILexicalBlock(scope: !986, file: !557, line: 174, column: 17)
!986 = distinct !DILexicalBlock(scope: !987, file: !557, line: 173, column: 28)
!987 = distinct !DILexicalBlock(scope: !895, file: !557, line: 173, column: 13)
!988 = !DILocalVariable(name: "t", scope: !989, file: !557, line: 178, type: !23)
!989 = distinct !DILexicalBlock(scope: !990, file: !557, line: 178, column: 33)
!990 = distinct !DILexicalBlock(scope: !991, file: !557, line: 178, column: 33)
!991 = distinct !DILexicalBlock(scope: !992, file: !557, line: 178, column: 33)
!992 = distinct !DILexicalBlock(scope: !993, file: !557, line: 175, column: 25)
!993 = distinct !DILexicalBlock(scope: !994, file: !557, line: 175, column: 25)
!994 = distinct !DILexicalBlock(scope: !995, file: !557, line: 174, column: 17)
!995 = distinct !DILexicalBlock(scope: !996, file: !557, line: 174, column: 17)
!996 = distinct !DILexicalBlock(scope: !997, file: !557, line: 173, column: 28)
!997 = distinct !DILexicalBlock(scope: !895, file: !557, line: 173, column: 13)
!998 = !DILabel(scope: !895, name: "loop", file: !557, line: 122)
!999 = !DILocation(line: 0, scope: !895)
!1000 = !DILocation(line: 122, column: 1, scope: !895)

