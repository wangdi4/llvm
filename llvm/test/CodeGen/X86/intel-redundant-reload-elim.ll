;RUN: llc < %s --stats -mtriple=x86_64-apple-darwin -enable-intel-advanced-opts=true -mcpu=core-avx2 2>&1 | FileCheck %s

; CHECK:  1 regalloc             - Number of eliminated reload instructions

%struct.a = type { ptr, ptr, %struct.list, ptr, ptr, %struct.list, %struct.list, [64 x i8], float, float, float, float, float, float, float, float, float, float, i16, i16, i16, i16, ptr, i32, i32 }
%struct.list = type { ptr, ptr }
%struct.b = type { ptr, ptr, ptr, ptr, i16, i16, float }
%struct.link = type { ptr, ptr }

@str = external hidden unnamed_addr constant [13 x i8], align 1
@MEM_callocN = external hidden unnamed_addr global ptr, align 8

; Function Attrs: nounwind uwtable
define hidden fastcc ptr @nlastrips_ctime_get_strip(ptr noundef %0, ptr nocapture noundef readonly %1, i16 noundef signext %2, float noundef nofpclass(nan inf) %3) {
  %5 = bitcast ptr %1 to ptr
  %6 = load ptr, ptr %5, align 8
  %7 = icmp eq ptr %6, null
  br i1 %7, label %104, label %8

8:                                                ; preds = %4
  br label %106

9:                                                ; preds = %146, %121
  %10 = phi ptr [ %107, %121 ], [ %130, %146 ]
  %11 = phi float [ %109, %121 ], [ %134, %146 ]
  %12 = icmp eq ptr %10, %6
  br i1 %12, label %13, label %22

13:                                               ; preds = %9
  %14 = getelementptr inbounds %struct.a, ptr %6, i64 0, i32 19
  %15 = load i16, ptr %14, align 2
  %16 = icmp eq i16 %15, 0
  br i1 %16, label %17, label %104

17:                                               ; preds = %13
  %18 = getelementptr inbounds %struct.a, ptr %6, i64 0, i32 23
  %19 = load i32, ptr %18, align 8
  %20 = and i32 %19, 4096
  %21 = icmp eq i32 %20, 0
  br i1 %21, label %48, label %104

22:                                               ; preds = %9
  %23 = getelementptr inbounds %struct.a, ptr %10, i64 0, i32 1
  %24 = load ptr, ptr %23, align 8
  %25 = getelementptr inbounds %struct.a, ptr %24, i64 0, i32 19
  %26 = load i16, ptr %25, align 2
  %27 = icmp eq i16 %26, 2
  br i1 %27, label %104, label %39

28:                                               ; preds = %150, %125
  %29 = phi ptr [ %107, %125 ], [ %130, %150 ]
  %30 = getelementptr inbounds %struct.a, ptr %29, i64 0, i32 19
  %31 = load i16, ptr %30, align 2
  %32 = icmp eq i16 %31, 2
  br i1 %32, label %104, label %39

33:                                               ; preds = %142, %138, %117, %113
  %34 = phi ptr [ %107, %113 ], [ %107, %117 ], [ %130, %138 ], [ %130, %142 ]
  %35 = getelementptr inbounds %struct.a, ptr %34, i64 0, i32 23
  %36 = load i32, ptr %35, align 8
  %37 = and i32 %36, 4096
  %38 = icmp eq i32 %37, 0
  br i1 %38, label %48, label %104

39:                                               ; preds = %28, %22
  %40 = phi ptr [ %24, %22 ], [ %29, %28 ]
  %41 = getelementptr inbounds %struct.a, ptr %40, i64 0, i32 23
  %42 = load i32, ptr %41, align 8
  %43 = and i32 %42, 4096
  %44 = icmp eq i32 %43, 0
  br i1 %44, label %45, label %104

45:                                               ; preds = %39
  %46 = getelementptr inbounds %struct.a, ptr %40, i64 0, i32 11
  %47 = load float, ptr %46, align 4
  br label %48

48:                                               ; preds = %45, %33, %17
  %49 = phi i16 [ 1, %45 ], [ 0, %33 ], [ -1, %17 ]
  %50 = phi ptr [ %40, %45 ], [ %34, %33 ], [ %6, %17 ]
  %51 = phi float [ %47, %45 ], [ %3, %33 ], [ %11, %17 ]
  tail call fastcc void @nlastrip_evaluate_controls(ptr noundef nonnull %50, float noundef nofpclass(nan inf) %51)
  %52 = getelementptr inbounds %struct.a, ptr %50, i64 0, i32 8
  %53 = load float, ptr %52, align 8
  %54 = fcmp fast ugt float %53, 0.000000e+00
  br i1 %54, label %55, label %104

55:                                               ; preds = %48
  %56 = getelementptr inbounds %struct.a, ptr %50, i64 0, i32 21
  %57 = load i16, ptr %56, align 2
  %58 = sext i16 %57 to i32
  switch i32 %58, label %78 [
    i32 0, label %59
    i32 1, label %63
  ]

59:                                               ; preds = %55
  %60 = getelementptr inbounds %struct.a, ptr %50, i64 0, i32 3
  %61 = load ptr, ptr %60, align 8
  %62 = icmp eq ptr %61, null
  br i1 %62, label %104, label %78

63:                                               ; preds = %55
  %64 = getelementptr inbounds %struct.a, ptr %50, i64 0, i32 1
  %65 = load ptr, ptr %64, align 8
  %66 = icmp eq ptr %65, null
  br i1 %66, label %104, label %67

67:                                               ; preds = %63
  %68 = bitcast ptr %50 to ptr
  %69 = load ptr, ptr %68, align 8
  %70 = icmp eq ptr %69, null
  br i1 %70, label %104, label %71

71:                                               ; preds = %67
  %72 = bitcast ptr %50 to ptr
  %73 = getelementptr inbounds %struct.a, ptr %50, i64 0, i32 10
  %74 = load float, ptr %73, align 8
  tail call fastcc void @nlastrip_evaluate_controls(ptr noundef nonnull %65, float noundef nofpclass(nan inf) %74)
  %75 = load ptr, ptr %72, align 8
  %76 = getelementptr inbounds %struct.a, ptr %50, i64 0, i32 11
  %77 = load float, ptr %76, align 4
  tail call fastcc void @nlastrip_evaluate_controls(ptr noundef %75, float noundef nofpclass(nan inf) %77)
  br label %78

78:                                               ; preds = %71, %59, %55
  %79 = load ptr, ptr @MEM_callocN, align 8
  %80 = tail call ptr %79(i64 noundef 40, ptr noundef nonnull @str)
  %81 = getelementptr inbounds %struct.b, ptr %80, i64 0, i32 3
  store ptr %50, ptr %81, align 8
  %82 = getelementptr inbounds %struct.b, ptr %80, i64 0, i32 5
  store i16 %49, ptr %82, align 2
  %83 = getelementptr inbounds %struct.b, ptr %80, i64 0, i32 4
  store i16 %2, ptr %83, align 8
  %84 = getelementptr inbounds %struct.a, ptr %50, i64 0, i32 9
  %85 = load float, ptr %84, align 4
  %86 = getelementptr inbounds %struct.b, ptr %80, i64 0, i32 6
  store float %85, ptr %86, align 4
  %87 = icmp eq ptr %0, null
  br i1 %87, label %104, label %88

88:                                               ; preds = %78
  %89 = bitcast ptr %80 to ptr
  store ptr null, ptr %89, align 8
  %90 = getelementptr inbounds %struct.list, ptr %0, i64 0, i32 1
  %91 = load ptr, ptr %90, align 8
  %92 = getelementptr inbounds %struct.link, ptr %80, i64 0, i32 1
  store ptr %91, ptr %92, align 8
  %93 = icmp eq ptr %91, null
  br i1 %93, label %96, label %94

94:                                               ; preds = %88
  %95 = bitcast ptr %91 to ptr
  store ptr %80, ptr %95, align 8
  br label %96

96:                                               ; preds = %94, %88
  %97 = bitcast ptr %0 to ptr
  %98 = load ptr, ptr %97, align 8
  %99 = icmp eq ptr %98, null
  br i1 %99, label %100, label %102

100:                                              ; preds = %96
  %101 = bitcast ptr %0 to ptr
  store ptr %80, ptr %101, align 8
  br label %102

102:                                              ; preds = %100, %96
  %103 = getelementptr inbounds i8, ptr %0, i64 8
  store ptr %80, ptr %103, align 8
  br label %104

104:                                              ; preds = %154, %129, %102, %78, %67, %63, %59, %48, %39, %33, %28, %22, %17, %13, %4
  %105 = phi ptr [ null, %13 ], [ null, %22 ], [ null, %28 ], [ null, %33 ], [ null, %39 ], [ null, %17 ], [ null, %48 ], [ null, %59 ], [ null, %67 ], [ null, %63 ], [ %80, %102 ], [ %80, %78 ], [ null, %4 ], [ null, %129 ], [ null, %154 ]
  ret ptr %105

106:                                              ; preds = %154, %8
  %107 = phi ptr [ %6, %8 ], [ %155, %154 ]
  %108 = getelementptr inbounds %struct.a, ptr %107, i64 0, i32 10
  %109 = load float, ptr %108, align 8
  %110 = getelementptr inbounds %struct.a, ptr %107, i64 0, i32 11
  %111 = load float, ptr %110, align 4
  %112 = fcmp fast olt float %109, %111
  br i1 %112, label %113, label %117

113:                                              ; preds = %106
  %114 = fcmp fast oge float %111, %3
  %115 = fcmp fast ole float %109, %3
  %116 = select i1 %115, i1 %114, i1 false
  br i1 %116, label %33, label %121

117:                                              ; preds = %106
  %118 = fcmp fast oge float %109, %3
  %119 = fcmp fast ole float %111, %3
  %120 = select i1 %119, i1 %118, i1 false
  br i1 %120, label %33, label %121

121:                                              ; preds = %117, %113
  %122 = fcmp fast ogt float %109, %3
  br i1 %122, label %9, label %123

123:                                              ; preds = %121
  %124 = fcmp fast olt float %111, %3
  br i1 %124, label %125, label %129

125:                                              ; preds = %123
  %126 = getelementptr inbounds i8, ptr %1, i64 8
  %127 = load ptr, ptr %126, align 8
  %128 = icmp eq ptr %107, %127
  br i1 %128, label %28, label %129

129:                                              ; preds = %125, %123
  %130 = load ptr, ptr %107, align 8
  %131 = icmp eq ptr %130, null
  br i1 %131, label %104, label %132

132:                                              ; preds = %129
  %133 = getelementptr inbounds %struct.a, ptr %130, i64 0, i32 10
  %134 = load float, ptr %133, align 8
  %135 = getelementptr inbounds %struct.a, ptr %130, i64 0, i32 11
  %136 = load float, ptr %135, align 4
  %137 = fcmp fast olt float %134, %136
  br i1 %137, label %138, label %142

138:                                              ; preds = %132
  %139 = fcmp fast oge float %136, %3
  %140 = fcmp fast ole float %134, %3
  %141 = select i1 %140, i1 %139, i1 false
  br i1 %141, label %33, label %146

142:                                              ; preds = %132
  %143 = fcmp fast oge float %134, %3
  %144 = fcmp fast ole float %136, %3
  %145 = select i1 %144, i1 %143, i1 false
  br i1 %145, label %33, label %146

146:                                              ; preds = %142, %138
  %147 = fcmp fast ogt float %134, %3
  br i1 %147, label %9, label %148

148:                                              ; preds = %146
  %149 = fcmp fast olt float %136, %3
  br i1 %149, label %150, label %154

150:                                              ; preds = %148
  %151 = getelementptr inbounds i8, ptr %1, i64 8
  %152 = load ptr, ptr %151, align 8
  %153 = icmp eq ptr %130, %152
  br i1 %153, label %28, label %154

154:                                              ; preds = %150, %148
  %155 = load ptr, ptr %130, align 8
  %156 = icmp eq ptr %155, null
  br i1 %156, label %104, label %106
}

; Function Attrs: nounwind uwtable
declare hidden fastcc void @nlastrip_evaluate_controls(ptr noundef, float noundef nofpclass(nan inf))
