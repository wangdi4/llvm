; Verify that GVN-PRE splits a partially redundant load, for the case in
; CMPLRLLVM-10341.
; The correctness of the transformation is covered by other GVN tests. The
; commit in this case is a cost model change only.

; RUN: opt -passes="require<memoryssa>,gvn" < %s -S | FileCheck %s

; Block 93 has the loads that we want to PRE. It will be split to ".split"
; and "86".
; There should be at least 2 loads in the split block.

; CHECK-LABEL: .split:
; CHECK: [[PHITRANSX:%.*]] = getelementptr
; CHECK-NEXT: [[PREX:%.*]] = load i8, ptr [[PHITRANSX]]
; CHECK-NEXT: [[PHITRANSY:%.*]] = getelementptr
; CHECK-NEXT: [[PREY:%.*]] = load i8, ptr [[PHITRANSY]]
; CHECK: br label

; Make sure all the loads are moved out of the original block.
; CHECK-LABEL: 8{{.}}:
; CHECK-NEXT: phi i8 [ [[PREY]], %.split ]
; CHECK-NEXT: phi i8 [ [[PREX]], %.split ]
; CHECK-NOT: load
; CHECK: br i1

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.lzma_match = type { i32, i32 }
%struct.lzma_mf_s = type { ptr, i32, i32, i32, i32, i32, i32, i32, i32, i32, ptr, ptr, ptr, ptr, i32, i32, i32, i32, i32, i32, i32, i32, i32 }

@0 = private unnamed_addr constant [16 x i8] c"padded 32 bytes\00", align 1
@1 = private unnamed_addr constant [10 x i8] c"ld-temp.o\00", align 1
@__Intel_PaddedMallocCounter = unnamed_addr global i32 0

define dso_local i1 @__Intel_PaddedMallocInterface() local_unnamed_addr {
  %1 = load i32, ptr @__Intel_PaddedMallocCounter, align 4
  %2 = icmp ult i32 %1, 250
  ret i1 %2
}

; Function Attrs: nofree noinline norecurse nounwind uwtable
define dso_local fastcc ptr @bt_find_func(i32 %0, i32 %1, ptr nocapture readonly %2, i32 %3, i32 %4, ptr nocapture %5, i32 %6, i32 %7, ptr %8, i32 %9) unnamed_addr #0 {
  %11 = tail call ptr @llvm.ptr.annotation.p0.p0(ptr %2, ptr @0, ptr @1, i32 0, ptr null)
  %12 = shl i32 %6, 1
  %13 = zext i32 %12 to i64
  %14 = getelementptr inbounds i32, ptr %5, i64 %13, !intel-tbaa !0
  %15 = getelementptr inbounds i32, ptr %14, i64 1, !intel-tbaa !0
  %16 = sub i32 %1, %3
  %17 = icmp ne i32 %4, 0
  %18 = icmp ult i32 %16, %7
  %19 = and i1 %17, %18
  br i1 %19, label %20, label %25

20:                                               ; preds = %10
  br label %29

21:                                               ; preds = %106
  %22 = phi ptr [ %107, %106 ]
  %23 = phi ptr [ %108, %106 ]
  %24 = phi ptr [ %94, %106 ]
  br label %25

25:                                               ; preds = %21, %10
  %26 = phi ptr [ %8, %10 ], [ %24, %21 ]
  %27 = phi ptr [ %15, %10 ], [ %22, %21 ]
  %28 = phi ptr [ %14, %10 ], [ %23, %21 ]
  store i32 0, ptr %27, align 4, !tbaa !0
  store i32 0, ptr %28, align 4, !tbaa !0
  br label %117

29:                                               ; preds = %106, %20
  %30 = phi i32 [ %39, %106 ], [ %4, %20 ]
  %31 = phi i32 [ %113, %106 ], [ %16, %20 ]
  %32 = phi i32 [ %112, %106 ], [ %3, %20 ]
  %33 = phi i32 [ %110, %106 ], [ 0, %20 ]
  %34 = phi i32 [ %109, %106 ], [ 0, %20 ]
  %35 = phi ptr [ %108, %106 ], [ %14, %20 ]
  %36 = phi ptr [ %107, %106 ], [ %15, %20 ]
  %37 = phi i32 [ %95, %106 ], [ %9, %20 ]
  %38 = phi ptr [ %94, %106 ], [ %8, %20 ]
  %39 = add i32 %30, -1
  %40 = sub i32 %6, %31
  %41 = icmp ugt i32 %31, %6
  %42 = select i1 %41, i32 %7, i32 0
  %43 = add i32 %40, %42
  %44 = shl i32 %43, 1
  %45 = zext i32 %44 to i64
  %46 = getelementptr inbounds i32, ptr %5, i64 %45, !intel-tbaa !0
  %47 = zext i32 %31 to i64
  %48 = sub nsw i64 0, %47
  %49 = getelementptr inbounds i8, ptr %11, i64 %48, !intel-tbaa !4
  %50 = icmp ult i32 %34, %33
  %51 = select i1 %50, i32 %34, i32 %33
  %52 = zext i32 %51 to i64
  %53 = getelementptr inbounds i8, ptr %49, i64 %52
  %54 = load i8, ptr %53, align 1, !tbaa !4
  %55 = getelementptr inbounds i8, ptr %11, i64 %52
  %56 = load i8, ptr %55, align 1, !tbaa !4
  %57 = icmp eq i8 %54, %56
  br i1 %57, label %58, label %93

58:                                               ; preds = %29
  %59 = add i32 %51, 1
  %60 = icmp eq i32 %59, %0
  br i1 %60, label %76, label %61

61:                                               ; preds = %58
  br label %65

62:                                               ; preds = %65
  %63 = add i32 %66, 1
  %64 = icmp eq i32 %63, %0
  br i1 %64, label %73, label %65

65:                                               ; preds = %62, %61
  %66 = phi i32 [ %63, %62 ], [ %59, %61 ]
  %67 = zext i32 %66 to i64
  %68 = getelementptr inbounds i8, ptr %49, i64 %67
  %69 = load i8, ptr %68, align 1, !tbaa !4
  %70 = getelementptr inbounds i8, ptr %11, i64 %67
  %71 = load i8, ptr %70, align 1, !tbaa !4
  %72 = icmp eq i8 %69, %71
  br i1 %72, label %62, label %73

73:                                               ; preds = %65, %62
  %74 = phi i32 [ %66, %65 ], [ %0, %62 ]
  %75 = phi i1 [ false, %65 ], [ true, %62 ]
  br label %76

76:                                               ; preds = %73, %58
  %77 = phi i32 [ %0, %58 ], [ %74, %73 ]
  %78 = phi i1 [ true, %58 ], [ %75, %73 ]
  %79 = icmp ult i32 %37, %77
  br i1 %79, label %80, label %93

80:                                               ; preds = %76
  %81 = getelementptr inbounds %struct.lzma_match, ptr %38, i64 0, i32 0, !intel-tbaa !5
  store i32 %77, ptr %81, align 4, !tbaa !5
  %82 = add i32 %31, -1
  %83 = getelementptr inbounds %struct.lzma_match, ptr %38, i64 0, i32 1, !intel-tbaa !7
  store i32 %82, ptr %83, align 4, !tbaa !7
  %84 = getelementptr inbounds %struct.lzma_match, ptr %38, i64 1
  br i1 %78, label %85, label %93

85:                                               ; preds = %80
  %86 = phi ptr [ %35, %80 ]
  %87 = phi ptr [ %36, %80 ]
  %88 = phi ptr [ %46, %80 ]
  %89 = phi ptr [ %84, %80 ]
  %90 = load i32, ptr %88, align 4, !tbaa !0
  store i32 %90, ptr %86, align 4, !tbaa !0
  %91 = getelementptr inbounds i32, ptr %88, i64 1
  %92 = load i32, ptr %91, align 4, !tbaa !0
  store i32 %92, ptr %87, align 4, !tbaa !0
  br label %117

93:                                               ; preds = %80, %76, %29
  %94 = phi ptr [ %84, %80 ], [ %38, %76 ], [ %38, %29 ]
  %95 = phi i32 [ %77, %80 ], [ %37, %76 ], [ %37, %29 ]
  %96 = phi i32 [ %77, %80 ], [ %77, %76 ], [ %51, %29 ]
  %97 = zext i32 %96 to i64
  %98 = getelementptr inbounds i8, ptr %49, i64 %97
  %99 = load i8, ptr %98, align 1, !tbaa !4
  %100 = getelementptr inbounds i8, ptr %11, i64 %97
  %101 = load i8, ptr %100, align 1, !tbaa !4
  %102 = icmp ult i8 %99, %101
  br i1 %102, label %103, label %105

103:                                              ; preds = %93
  store i32 %32, ptr %35, align 4, !tbaa !0
  %104 = getelementptr inbounds i32, ptr %46, i64 1, !intel-tbaa !0
  br label %106

105:                                              ; preds = %93
  store i32 %32, ptr %36, align 4, !tbaa !0
  br label %106

106:                                              ; preds = %105, %103
  %107 = phi ptr [ %36, %103 ], [ %46, %105 ]
  %108 = phi ptr [ %104, %103 ], [ %35, %105 ]
  %109 = phi i32 [ %34, %103 ], [ %96, %105 ]
  %110 = phi i32 [ %96, %103 ], [ %33, %105 ]
  %111 = phi ptr [ %104, %103 ], [ %46, %105 ]
  %112 = load i32, ptr %111, align 4, !tbaa !0
  %113 = sub i32 %1, %112
  %114 = icmp ne i32 %39, 0
  %115 = icmp ult i32 %113, %7
  %116 = and i1 %114, %115
  br i1 %116, label %29, label %21

117:                                              ; preds = %85, %25
  %118 = phi ptr [ %26, %25 ], [ %89, %85 ]
  ret ptr %118
}

; This test just makes sure the compiler doesn't crash during load PRE
; when load and phi node are in different basic blocks:
; bb1:
;  %phi = phi i32 [ %l, %if.else ], [ %l, %while.end ], [ %s, %if.else10 ]
;  ...
; bb2:
;  %0 = zext i32 %phi to i64
;  %1 = getelementptr inbounds i8, i8* %a, i64 %0
;  %2 = load i8, i8* %1, align 1, !tbaa !19
; CHECK-LABEL: @lzma_mf_bt2_find_no_crash

define internal i32 @lzma_mf_bt2_find_no_crash(ptr nocapture %0, ptr %1) {
  %3 = getelementptr %struct.lzma_mf_s, ptr %0, i64 0, i32 5
  %4 = load i32, ptr %3, align 4
  %5 = getelementptr %struct.lzma_mf_s, ptr %0, i64 0, i32 8
  %6 = load i32, ptr %5, align 4
  %7 = sub i32 %6, %4
  %8 = getelementptr inbounds %struct.lzma_mf_s, ptr %0, i64 0, i32 18
  %9 = load i32, ptr %8, align 4
  %10 = icmp ugt i32 %9, %7
  br i1 %10, label %11, label %22

11:                                               ; preds = %2
  %12 = icmp ult i32 %7, 2
  br i1 %12, label %17, label %13

13:                                               ; preds = %11
  %14 = getelementptr inbounds %struct.lzma_mf_s, ptr %0, i64 0, i32 20
  %15 = load i32, ptr %14, align 4
  %16 = icmp eq i32 %15, 1
  br i1 %16, label %17, label %22

17:                                               ; preds = %13, %11
  %18 = add i32 %4, 1
  store i32 %18, ptr %3, align 4
  %19 = getelementptr inbounds %struct.lzma_mf_s, ptr %0, i64 0, i32 9
  %20 = load i32, ptr %19, align 4
  %21 = add i32 %20, 1
  store i32 %21, ptr %19, align 4
  br label %187

22:                                               ; preds = %13, %2
  %23 = phi i32 [ %7, %13 ], [ %9, %2 ]
  %24 = getelementptr %struct.lzma_mf_s, ptr %0, i64 0, i32 0
  %25 = load ptr, ptr %24, align 8
  %26 = tail call ptr @llvm.ptr.annotation.p0.p0(ptr %25, ptr @0, ptr @1, i32 0, ptr null)
  %27 = zext i32 %4 to i64
  %28 = getelementptr inbounds i8, ptr %26, i64 %27
  %29 = getelementptr inbounds %struct.lzma_mf_s, ptr %0, i64 0, i32 4
  %30 = load i32, ptr %29, align 4
  %31 = add i32 %30, %4
  %32 = load i8, ptr %28, align 1
  %33 = zext i8 %32 to i64
  %34 = getelementptr inbounds i8, ptr %28, i64 1
  %35 = load i8, ptr %34, align 1
  %36 = zext i8 %35 to i64
  %37 = shl nuw nsw i64 %36, 8
  %38 = or i64 %37, %33
  %39 = getelementptr inbounds %struct.lzma_mf_s, ptr %0, i64 0, i32 12
  %40 = load ptr, ptr %39, align 8
  %41 = getelementptr inbounds i32, ptr %40, i64 %38
  %42 = load i32, ptr %41, align 4
  store i32 %31, ptr %41, align 4
  %43 = getelementptr inbounds %struct.lzma_mf_s, ptr %0, i64 0, i32 17
  %44 = load i32, ptr %43, align 4
  %45 = getelementptr inbounds %struct.lzma_mf_s, ptr %0, i64 0, i32 13
  %46 = load ptr, ptr %45, align 8
  %47 = getelementptr inbounds %struct.lzma_mf_s, ptr %0, i64 0, i32 14
  %48 = load i32, ptr %47, align 4
  %49 = getelementptr inbounds %struct.lzma_mf_s, ptr %0, i64 0, i32 15
  %50 = load i32, ptr %49, align 4
  %51 = shl i32 %48, 1
  %52 = zext i32 %51 to i64
  %53 = getelementptr inbounds i32, ptr %46, i64 %52
  %54 = getelementptr inbounds i32, ptr %53, i64 1
  %55 = sub i32 %31, %42
  %56 = icmp ne i32 %44, 0
  %57 = icmp ult i32 %55, %50
  %58 = and i1 %56, %57
  br i1 %58, label %63, label %59

59:                                               ; preds = %141, %22
  %60 = phi ptr [ %1, %22 ], [ %132, %141 ]
  %61 = phi ptr [ %54, %22 ], [ %142, %141 ]
  %62 = phi ptr [ %53, %22 ], [ %143, %141 ]
  store i32 0, ptr %61, align 4
  store i32 0, ptr %62, align 4
  br label %152

63:                                               ; preds = %141, %22
  %64 = phi i32 [ %73, %141 ], [ %44, %22 ]
  %65 = phi i32 [ %148, %141 ], [ %55, %22 ]
  %66 = phi i32 [ %147, %141 ], [ %42, %22 ]
  %67 = phi i32 [ %145, %141 ], [ 0, %22 ]
  %68 = phi i32 [ %144, %141 ], [ 0, %22 ]
  %69 = phi ptr [ %143, %141 ], [ %53, %22 ]
  %70 = phi ptr [ %142, %141 ], [ %54, %22 ]
  %71 = phi i32 [ %133, %141 ], [ 1, %22 ]
  %72 = phi ptr [ %132, %141 ], [ %1, %22 ]
  %73 = add i32 %64, -1
  %74 = sub i32 %48, %65
  %75 = icmp ult i32 %48, %65
  %76 = select i1 %75, i32 %50, i32 0
  %77 = add i32 %74, %76
  %78 = shl i32 %77, 1
  %79 = zext i32 %78 to i64
  %80 = getelementptr inbounds i32, ptr %46, i64 %79
  %81 = zext i32 %65 to i64
  %82 = sub nsw i64 0, %81
  %83 = getelementptr inbounds i8, ptr %28, i64 %82
  %84 = icmp ult i32 %68, %67
  %85 = select i1 %84, i32 %68, i32 %67
  %86 = zext i32 %85 to i64
  %87 = getelementptr inbounds i8, ptr %83, i64 %86
  %88 = load i8, ptr %87, align 1
  %89 = getelementptr inbounds i8, ptr %28, i64 %86
  %90 = load i8, ptr %89, align 1
  %91 = icmp eq i8 %88, %90
  br i1 %91, label %92, label %129

92:                                               ; preds = %63
  %93 = add i32 %85, 1
  %94 = icmp eq i32 %93, %23
  br i1 %94, label %106, label %98

95:                                               ; preds = %98
  %96 = add i32 %99, 1
  %97 = icmp eq i32 %96, %23
  br i1 %97, label %106, label %98

98:                                               ; preds = %95, %92
  %99 = phi i32 [ %96, %95 ], [ %93, %92 ]
  %100 = zext i32 %99 to i64
  %101 = getelementptr inbounds i8, ptr %83, i64 %100
  %102 = load i8, ptr %101, align 1
  %103 = getelementptr inbounds i8, ptr %28, i64 %100
  %104 = load i8, ptr %103, align 1
  %105 = icmp eq i8 %102, %104
  br i1 %105, label %95, label %106

106:                                              ; preds = %98, %95, %92
  %107 = phi i32 [ %23, %92 ], [ %99, %98 ], [ %23, %95 ]
  %108 = phi i1 [ true, %92 ], [ false, %98 ], [ true, %95 ]
  %109 = icmp ult i32 %71, %107
  br i1 %109, label %110, label %123

110:                                              ; preds = %106
  %111 = getelementptr inbounds %struct.lzma_match, ptr %72, i64 0, i32 0
  store i32 %107, ptr %111, align 4
  %112 = add i32 %65, -1
  %113 = getelementptr inbounds %struct.lzma_match, ptr %72, i64 0, i32 1
  store i32 %112, ptr %113, align 4
  %114 = getelementptr inbounds %struct.lzma_match, ptr %72, i64 1
  br i1 %108, label %115, label %123

115:                                              ; preds = %110
  %116 = phi ptr [ %69, %110 ]
  %117 = phi ptr [ %70, %110 ]
  %118 = phi ptr [ %80, %110 ]
  %119 = phi ptr [ %114, %110 ]
  %120 = load i32, ptr %118, align 4
  store i32 %120, ptr %116, align 4
  %121 = getelementptr inbounds i32, ptr %118, i64 1
  %122 = load i32, ptr %121, align 4
  store i32 %122, ptr %117, align 4
  br label %152

123:                                              ; preds = %110, %106
  %124 = phi ptr [ %72, %106 ], [ %114, %110 ]
  %125 = phi i32 [ %71, %106 ], [ %107, %110 ]
  %126 = zext i32 %107 to i64
  %127 = getelementptr inbounds i8, ptr %83, i64 %126
  %128 = load i8, ptr %127, align 1
  br label %129

129:                                              ; preds = %123, %63
  %130 = phi i64 [ %126, %123 ], [ %86, %63 ]
  %131 = phi i8 [ %88, %63 ], [ %128, %123 ]
  %132 = phi ptr [ %72, %63 ], [ %124, %123 ]
  %133 = phi i32 [ %71, %63 ], [ %125, %123 ]
  %134 = phi i32 [ %85, %63 ], [ %107, %123 ]
  %135 = getelementptr inbounds i8, ptr %28, i64 %130
  %136 = load i8, ptr %135, align 1
  %137 = icmp ult i8 %131, %136
  br i1 %137, label %138, label %140

138:                                              ; preds = %129
  store i32 %66, ptr %69, align 4
  %139 = getelementptr inbounds i32, ptr %80, i64 1
  br label %141

140:                                              ; preds = %129
  store i32 %66, ptr %70, align 4
  br label %141

141:                                              ; preds = %140, %138
  %142 = phi ptr [ %70, %138 ], [ %80, %140 ]
  %143 = phi ptr [ %139, %138 ], [ %69, %140 ]
  %144 = phi i32 [ %68, %138 ], [ %134, %140 ]
  %145 = phi i32 [ %134, %138 ], [ %67, %140 ]
  %146 = phi ptr [ %139, %138 ], [ %80, %140 ]
  %147 = load i32, ptr %146, align 4
  %148 = sub i32 %31, %147
  %149 = icmp ne i32 %73, 0
  %150 = icmp ult i32 %148, %50
  %151 = and i1 %149, %150
  br i1 %151, label %63, label %59

152:                                              ; preds = %115, %59
  %153 = phi ptr [ %60, %59 ], [ %119, %115 ]
  %154 = ptrtoint ptr %153 to i64
  %155 = ptrtoint ptr %1 to i64
  %156 = sub i64 %154, %155
  %157 = lshr exact i64 %156, 3
  %158 = trunc i64 %157 to i32
  %159 = load i32, ptr %47, align 4
  %160 = add i32 %159, 1
  %161 = load i32, ptr %49, align 4
  %162 = icmp eq i32 %160, %161
  %163 = select i1 %162, i32 0, i32 %160
  store i32 %163, ptr %47, align 4
  %164 = load i32, ptr %3, align 4
  %165 = add i32 %164, 1
  store i32 %165, ptr %3, align 4
  %166 = load i32, ptr %29, align 4
  %167 = add i32 %166, %165
  %168 = icmp eq i32 %167, -1
  br i1 %168, label %169, label %187

169:                                              ; preds = %152
  %170 = xor i32 %161, -1
  %171 = getelementptr inbounds %struct.lzma_mf_s, ptr %0, i64 0, i32 21
  %172 = load i32, ptr %171, align 4
  %173 = getelementptr inbounds %struct.lzma_mf_s, ptr %0, i64 0, i32 22
  %174 = load i32, ptr %173, align 4
  %175 = add i32 %174, %172
  %176 = icmp eq i32 %175, 0
  br i1 %176, label %184, label %177

177:                                              ; preds = %169
  %178 = zext i32 %175 to i64
  %179 = add i32 %174, %172
  %180 = zext i32 %179 to i64
  %181 = udiv i64 %180, 4
  %182 = shl i64 %181, 2
  %183 = icmp ult i64 0, %182
  br i1 %183, label %189, label %206

184:                                              ; preds = %226, %169
  %185 = phi i32 [ %227, %226 ], [ %166, %169 ]
  %186 = sub i32 %185, %170
  store i32 %186, ptr %29, align 4
  br label %187

187:                                              ; preds = %184, %152, %17
  %188 = phi i32 [ 0, %17 ], [ %158, %152 ], [ %158, %184 ]
  ret i32 %188

189:                                              ; preds = %177
  %190 = shl i64 %181, 2
  %191 = add i64 %190, -1
  br label %192

192:                                              ; preds = %192, %189
  %193 = phi i64 [ 0, %189 ], [ %204, %192 ]
  %194 = getelementptr inbounds i32, ptr %40, i64 %193
  %195 = bitcast ptr %194 to ptr
  %196 = load <4 x i32>, ptr %195, align 16
  %197 = sub i32 0, %161
  %198 = add i32 %197, -1
  %199 = insertelement <4 x i32> undef, i32 %198, i32 0
  %200 = shufflevector <4 x i32> %199, <4 x i32> undef, <4 x i32> zeroinitializer
  %201 = call <4 x i32> @llvm.usub.sat.v4i32(<4 x i32> %196, <4 x i32> %200)
  %202 = getelementptr inbounds i32, ptr %40, i64 %193
  %203 = bitcast ptr %202 to ptr
  store <4 x i32> %201, ptr %203, align 16
  %204 = add nuw nsw i64 %193, 4
  %205 = icmp sle i64 %204, %191
  br i1 %205, label %192, label %206

206:                                              ; preds = %192, %177
  %207 = shl i64 %181, 2
  %208 = add i32 %174, %172
  %209 = zext i32 %208 to i64
  %210 = icmp ult i64 %207, %209
  br i1 %210, label %211, label %226

211:                                              ; preds = %206
  %212 = shl i64 %181, 2
  %213 = add i32 %174, %172
  %214 = zext i32 %213 to i64
  %215 = add i64 %214, -1
  br label %216

216:                                              ; preds = %216, %211
  %217 = phi i64 [ %212, %211 ], [ %224, %216 ]
  %218 = getelementptr inbounds i32, ptr %40, i64 %217
  %219 = load i32, ptr %218, align 4
  %220 = sub i32 0, %161
  %221 = add i32 %220, -1
  %222 = tail call i32 @llvm.usub.sat.i32(i32 %219, i32 %221)
  %223 = getelementptr inbounds i32, ptr %40, i64 %217
  store i32 %222, ptr %223, align 4
  %224 = add nuw nsw i64 %217, 1
  %225 = icmp ne i64 %217, %215
  br i1 %225, label %216, label %226

226:                                              ; preds = %216, %206
  %227 = load i32, ptr %29, align 4
  br label %184
}

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare <4 x i32> @llvm.usub.sat.v4i32(<4 x i32>, <4 x i32>) #1

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i32 @llvm.usub.sat.i32(i32, i32) #1

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: readwrite)
declare ptr @llvm.ptr.annotation.p0.p0(ptr, ptr, ptr, i32, ptr) #2

attributes #0 = { nofree noinline norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+mmx,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #1 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #2 = { nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: readwrite) }

!0 = !{!1, !1, i64 0}
!1 = !{!"int", !2, i64 0}
!2 = !{!"omnipotent char", !3, i64 0}
!3 = !{!"Simple C/C++ TBAA"}
!4 = !{!2, !2, i64 0}
!5 = !{!6, !1, i64 0}
!6 = !{!"struct@", !1, i64 0, !1, i64 4}
!7 = !{!6, !1, i64 4}
