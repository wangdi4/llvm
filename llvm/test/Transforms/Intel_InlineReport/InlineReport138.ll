; RUN: opt -passes='function(instcombine),print<inline-report>' -disable-output -inline-report=0xe807 < %s 2>&1 | FileCheck %s
; RUN: opt -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='function(instcombine)' -inline-report=0xe886 -S | opt -passes='inlinereportemitter' -inline-report=0xe886 -S 2>&1 | FileCheck %s

; Check that various calls to fputc are deleted as dead code.

; CHECK-LABEL: COMPILE FUNC: i_send_buf_as_file
; CHECK: EXTERN: t_printf
; CHECK: DELETE: fputc {{.*}}Dead code{{.*}}
; CHECK: EXTERN: fputc
; CHECK: DELETE: fputc {{.*}}Dead code{{.*}}
; CHECK: EXTERN: fputc
; CHECK: DELETE: fputc {{.*}}Dead code{{.*}}
; CHECK: EXTERN: fputc
; CHECK: DELETE: fputc {{.*}}Dead code{{.*}}
; CHECK: EXTERN: fputc
; CHECK: EXTERN: fputc
; CHECK: EXTERN: fputc
; CHECK: DELETE: fputc {{.*}}Dead code{{.*}}
; CHECK: EXTERN: fputc
; CHECK: DELETE: fputc {{.*}}Dead code{{.*}}
; CHECK: EXTERN: fputc
; CHECK: DELETE: fputc {{.*}}Dead code{{.*}}
; CHECK: EXTERN: fputc
; CHECK: EXTERN: fputc
; CHECK: EXTERN: fputc
; CHECK: EXTERN: fputc
; CHECK: EXTERN: fputc
; CHECK: EXTERN: fputc
; CHECK: EXTERN: fputc
; CHECK: EXTERN: t_printf

@.str.1.89 = private unnamed_addr constant [14 x i8] c"begin %lo %s\0A\00", align 1
@.str.86 = private unnamed_addr constant [35 x i8] c"Uuencode buffer parameters error.\0A\00", align 1
@.str.2.90 = private unnamed_addr constant [6 x i8] c"end\0A\0A\00", align 1
@uu_std = internal unnamed_addr constant [64 x i8] c"`!\22#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_", align 16

%struct._IO_FILE = type { i32, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, %struct._IO_marker*, %struct._IO_FILE*, i32, i32, i64, i16, i8, [1 x i8], i8*, i64, %struct._IO_codecvt*, %struct._IO_wide_data*, %struct._IO_FILE*, i8*, i64, i32, [20 x i8] }
%struct._IO_marker = type opaque
%struct._IO_codecvt = type opaque
%struct._IO_wide_data = type opaque

@stdout = external dso_local local_unnamed_addr global %struct._IO_FILE*, align 8

declare void @t_printf(i8* nocapture noundef readonly %0, ...) 

declare void @llvm.lifetime.start.p0i8(i64 immarg %0, i8* nocapture %1)

declare noundef i32 @fputc(i32 noundef %0, %struct._IO_FILE* nocapture noundef %1)

declare void @llvm.lifetime.end.p0i8(i64 immarg %0, i8* nocapture %1)

; Function Attrs: nounwind uwtable
define internal i32 @i_send_buf_as_file(i8* noundef %0, i64 noundef %1, i8* noundef %2) #0 {
  %4 = alloca i8, align 1
  %5 = alloca i8, align 1
  %6 = alloca i8, align 1
  %7 = alloca i8, align 1
  %8 = alloca i8, align 1
  %9 = alloca i8, align 1
  %10 = alloca i8, align 1
  %11 = alloca i8, align 1
  %12 = alloca i8, align 1
  %13 = alloca i8, align 1
  %14 = alloca i8, align 1
  %15 = alloca i8, align 1
  %16 = alloca i8, align 1
  %17 = alloca i8, align 1
  %18 = alloca i8, align 1
  %19 = alloca i8, align 1
  %20 = alloca [80 x i8], align 16
  %21 = trunc i64 %1 to i32
  tail call void (i8*, ...) @t_printf(i8* noundef getelementptr inbounds ([14 x i8], [14 x i8]* @.str.1.89, i64 0, i64 0), i64 noundef 384, i8* noundef %2) #27
  %22 = getelementptr inbounds [80 x i8], [80 x i8]* %20, i64 0, i64 0
  call void @llvm.lifetime.start.p0i8(i64 80, i8* nonnull %22) #27
  %23 = icmp sgt i32 %21, 0
  %24 = icmp ne i8* %0, null
  %25 = and i1 %23, %24
  br i1 %25, label %27, label %26

26:                                               ; preds = %3
  tail call void (i8*, ...) @t_printf(i8* noundef getelementptr inbounds ([35 x i8], [35 x i8]* @.str.86, i64 0, i64 0)) #27
  br label %221

27:                                               ; preds = %3
  %28 = and i64 %1, 4294967295
  br label %29

29:                                               ; preds = %144, %27
  %30 = phi i64 [ %46, %144 ], [ 0, %27 ]
  br label %31

31:                                               ; preds = %35, %29
  %32 = phi i64 [ 0, %29 ], [ %39, %35 ]
  %33 = add nuw nsw i64 %32, %30
  %34 = icmp slt i64 %33, %28
  br i1 %34, label %35, label %41

35:                                               ; preds = %31
  %36 = getelementptr inbounds i8, i8* %0, i64 %33
  %37 = load i8, i8* %36, align 1
  %38 = getelementptr inbounds [80 x i8], [80 x i8]* %20, i64 0, i64 %32
  store i8 %37, i8* %38, align 1
  %39 = add nuw nsw i64 %32, 1
  %40 = icmp eq i64 %39, 45
  br i1 %40, label %44, label %31

41:                                               ; preds = %31
  %42 = trunc i64 %32 to i32
  %43 = icmp eq i32 %42, 0
  br i1 %43, label %214, label %44

44:                                               ; preds = %41, %35
  %45 = phi i32 [ %42, %41 ], [ 45, %35 ]
  %46 = add nuw i64 %30, 45
  %47 = and i32 %45, 63
  %48 = zext i32 %47 to i64
  %49 = getelementptr inbounds [64 x i8], [64 x i8]* @uu_std, i64 0, i64 %48
  %50 = load i8, i8* %49, align 1
  call void @llvm.lifetime.start.p0i8(i64 1, i8* nonnull %18)
  store i8 %50, i8* %18, align 1
  call void @llvm.lifetime.start.p0i8(i64 1, i8* nonnull %19) #27
  store i8 13, i8* %19, align 1
  br i1 false, label %51, label %56

51:                                               ; preds = %44
  %52 = load %struct._IO_FILE*, %struct._IO_FILE** @stdout, align 8
  %53 = load i8, i8* %19, align 1
  %54 = sext i8 %53 to i32
  %55 = call i32 @fputc(i32 %54, %struct._IO_FILE* %52)
  br label %56

56:                                               ; preds = %51, %44
  %57 = load %struct._IO_FILE*, %struct._IO_FILE** @stdout, align 8
  %58 = load i8, i8* %18, align 1
  %59 = sext i8 %58 to i32
  %60 = call i32 @fputc(i32 %59, %struct._IO_FILE* %57)
  call void @llvm.lifetime.end.p0i8(i64 1, i8* nonnull %19) #27
  call void @llvm.lifetime.end.p0i8(i64 1, i8* nonnull %18)
  %61 = icmp sgt i32 %45, 2
  br i1 %61, label %62, label %140

62:                                               ; preds = %132, %56
  %63 = phi i32 [ %137, %132 ], [ %45, %56 ]
  %64 = phi i8* [ %138, %132 ], [ %22, %56 ]
  %65 = load i8, i8* %64, align 1
  %66 = lshr i8 %65, 2
  %67 = zext i8 %66 to i64
  %68 = getelementptr inbounds [64 x i8], [64 x i8]* @uu_std, i64 0, i64 %67
  %69 = load i8, i8* %68, align 1
  call void @llvm.lifetime.start.p0i8(i64 1, i8* nonnull %16)
  store i8 %69, i8* %16, align 1
  call void @llvm.lifetime.start.p0i8(i64 1, i8* nonnull %17) #27
  store i8 13, i8* %17, align 1
  br i1 false, label %70, label %75

70:                                               ; preds = %62
  %71 = load %struct._IO_FILE*, %struct._IO_FILE** @stdout, align 8
  %72 = load i8, i8* %17, align 1
  %73 = sext i8 %72 to i32
  %74 = call i32 @fputc(i32 %73, %struct._IO_FILE* %71)
  br label %75

75:                                               ; preds = %70, %62
  %76 = load %struct._IO_FILE*, %struct._IO_FILE** @stdout, align 8
  %77 = load i8, i8* %16, align 1
  %78 = sext i8 %77 to i32
  %79 = call i32 @fputc(i32 %78, %struct._IO_FILE* %76)
  call void @llvm.lifetime.end.p0i8(i64 1, i8* nonnull %17) #27
  call void @llvm.lifetime.end.p0i8(i64 1, i8* nonnull %16)
  %80 = load i8, i8* %64, align 1
  %81 = sext i8 %80 to i64
  %82 = shl nsw i64 %81, 4
  %83 = and i64 %82, 48
  %84 = getelementptr inbounds i8, i8* %64, i64 1
  %85 = load i8, i8* %84, align 1
  %86 = lshr i8 %85, 4
  %87 = zext i8 %86 to i64
  %88 = or i64 %83, %87
  %89 = getelementptr inbounds [64 x i8], [64 x i8]* @uu_std, i64 0, i64 %88
  %90 = load i8, i8* %89, align 1
  call void @llvm.lifetime.start.p0i8(i64 1, i8* nonnull %14)
  store i8 %90, i8* %14, align 1
  call void @llvm.lifetime.start.p0i8(i64 1, i8* nonnull %15) #27
  store i8 13, i8* %15, align 1
  br i1 false, label %91, label %96

91:                                               ; preds = %75
  %92 = load %struct._IO_FILE*, %struct._IO_FILE** @stdout, align 8
  %93 = load i8, i8* %15, align 1
  %94 = sext i8 %93 to i32
  %95 = call i32 @fputc(i32 %94, %struct._IO_FILE* %92)
  br label %96

96:                                               ; preds = %91, %75
  %97 = load %struct._IO_FILE*, %struct._IO_FILE** @stdout, align 8
  %98 = load i8, i8* %14, align 1
  %99 = sext i8 %98 to i32
  %100 = call i32 @fputc(i32 %99, %struct._IO_FILE* %97)
  call void @llvm.lifetime.end.p0i8(i64 1, i8* nonnull %15) #27
  call void @llvm.lifetime.end.p0i8(i64 1, i8* nonnull %14)
  %101 = load i8, i8* %84, align 1
  %102 = sext i8 %101 to i64
  %103 = shl nsw i64 %102, 2
  %104 = and i64 %103, 60
  %105 = getelementptr inbounds i8, i8* %64, i64 2
  %106 = load i8, i8* %105, align 1
  %107 = lshr i8 %106, 6
  %108 = zext i8 %107 to i64
  %109 = or i64 %104, %108
  %110 = getelementptr inbounds [64 x i8], [64 x i8]* @uu_std, i64 0, i64 %109
  %111 = load i8, i8* %110, align 1
  call void @llvm.lifetime.start.p0i8(i64 1, i8* nonnull %12)
  store i8 %111, i8* %12, align 1
  call void @llvm.lifetime.start.p0i8(i64 1, i8* nonnull %13) #27
  store i8 13, i8* %13, align 1
  br i1 false, label %112, label %117

112:                                              ; preds = %96
  %113 = load %struct._IO_FILE*, %struct._IO_FILE** @stdout, align 8
  %114 = load i8, i8* %13, align 1
  %115 = sext i8 %114 to i32
  %116 = call i32 @fputc(i32 %115, %struct._IO_FILE* %113)
  br label %117

117:                                              ; preds = %112, %96
  %118 = load %struct._IO_FILE*, %struct._IO_FILE** @stdout, align 8
  %119 = load i8, i8* %12, align 1
  %120 = sext i8 %119 to i32
  %121 = call i32 @fputc(i32 %120, %struct._IO_FILE* %118)
  call void @llvm.lifetime.end.p0i8(i64 1, i8* nonnull %13) #27
  call void @llvm.lifetime.end.p0i8(i64 1, i8* nonnull %12)
  %122 = load i8, i8* %105, align 1
  %123 = and i8 %122, 63
  %124 = zext i8 %123 to i64
  %125 = getelementptr inbounds [64 x i8], [64 x i8]* @uu_std, i64 0, i64 %124
  %126 = load i8, i8* %125, align 1
  call void @llvm.lifetime.start.p0i8(i64 1, i8* nonnull %10)
  store i8 %126, i8* %10, align 1
  call void @llvm.lifetime.start.p0i8(i64 1, i8* nonnull %11) #27
  store i8 13, i8* %11, align 1
  br i1 false, label %127, label %132

127:                                              ; preds = %117
  %128 = load %struct._IO_FILE*, %struct._IO_FILE** @stdout, align 8
  %129 = load i8, i8* %11, align 1
  %130 = sext i8 %129 to i32
  %131 = call i32 @fputc(i32 %130, %struct._IO_FILE* %128)
  br label %132

132:                                              ; preds = %127, %117
  %133 = load %struct._IO_FILE*, %struct._IO_FILE** @stdout, align 8
  %134 = load i8, i8* %10, align 1
  %135 = sext i8 %134 to i32
  %136 = call i32 @fputc(i32 %135, %struct._IO_FILE* %133)
  call void @llvm.lifetime.end.p0i8(i64 1, i8* nonnull %11) #27
  call void @llvm.lifetime.end.p0i8(i64 1, i8* nonnull %10)
  %137 = add nsw i32 %63, -3
  %138 = getelementptr inbounds i8, i8* %64, i64 3
  %139 = icmp sgt i32 %63, 5
  br i1 %139, label %62, label %140

140:                                              ; preds = %132, %56
  %141 = phi i8* [ %22, %56 ], [ %138, %132 ]
  %142 = phi i32 [ %45, %56 ], [ %137, %132 ]
  %143 = icmp eq i32 %142, 0
  br i1 %143, label %144, label %149

144:                                              ; preds = %140
  %145 = load %struct._IO_FILE*, %struct._IO_FILE** @stdout, align 8
  %146 = call i32 @fputc(i32 13, %struct._IO_FILE* %145)
  %147 = load %struct._IO_FILE*, %struct._IO_FILE** @stdout, align 8
  %148 = call i32 @fputc(i32 10, %struct._IO_FILE* %147)
  br label %29

149:                                              ; preds = %140
  %150 = load i8, i8* %141, align 1
  %151 = icmp eq i32 %142, 1
  br i1 %151, label %156, label %152

152:                                              ; preds = %149
  %153 = getelementptr inbounds i8, i8* %141, i64 1
  %154 = load i8, i8* %153, align 1
  %155 = sext i8 %154 to i32
  br label %156

156:                                              ; preds = %152, %149
  %157 = phi i32 [ %155, %152 ], [ 0, %149 ]
  %158 = sext i8 %150 to i32
  %159 = lshr i32 %158, 2
  %160 = and i32 %159, 63
  %161 = zext i32 %160 to i64
  %162 = getelementptr inbounds [64 x i8], [64 x i8]* @uu_std, i64 0, i64 %161
  %163 = load i8, i8* %162, align 1
  call void @llvm.lifetime.start.p0i8(i64 1, i8* nonnull %8)
  store i8 %163, i8* %8, align 1
  call void @llvm.lifetime.start.p0i8(i64 1, i8* nonnull %9) #27
  store i8 13, i8* %9, align 1
  br i1 false, label %164, label %169

164:                                              ; preds = %156
  %165 = load %struct._IO_FILE*, %struct._IO_FILE** @stdout, align 8
  %166 = load i8, i8* %9, align 1
  %167 = sext i8 %166 to i32
  %168 = call i32 @fputc(i32 %167, %struct._IO_FILE* %165)
  br label %169

169:                                              ; preds = %164, %156
  %170 = load %struct._IO_FILE*, %struct._IO_FILE** @stdout, align 8
  %171 = load i8, i8* %8, align 1
  %172 = sext i8 %171 to i32
  %173 = call i32 @fputc(i32 %172, %struct._IO_FILE* %170)
  call void @llvm.lifetime.end.p0i8(i64 1, i8* nonnull %9) #27
  call void @llvm.lifetime.end.p0i8(i64 1, i8* nonnull %8)
  %174 = shl nsw i32 %158, 4
  %175 = and i32 %174, 48
  %176 = lshr i32 %157, 4
  %177 = and i32 %176, 15
  %178 = or i32 %175, %177
  %179 = zext i32 %178 to i64
  %180 = getelementptr inbounds [64 x i8], [64 x i8]* @uu_std, i64 0, i64 %179
  %181 = load i8, i8* %180, align 1
  call void @llvm.lifetime.start.p0i8(i64 1, i8* nonnull %6)
  store i8 %181, i8* %6, align 1
  call void @llvm.lifetime.start.p0i8(i64 1, i8* nonnull %7) #27
  store i8 13, i8* %7, align 1
  br i1 false, label %182, label %187

182:                                              ; preds = %169
  %183 = load %struct._IO_FILE*, %struct._IO_FILE** @stdout, align 8
  %184 = load i8, i8* %7, align 1
  %185 = sext i8 %184 to i32
  %186 = call i32 @fputc(i32 %185, %struct._IO_FILE* %183)
  br label %187

187:                                              ; preds = %182, %169
  %188 = load %struct._IO_FILE*, %struct._IO_FILE** @stdout, align 8
  %189 = load i8, i8* %6, align 1
  %190 = sext i8 %189 to i32
  %191 = call i32 @fputc(i32 %190, %struct._IO_FILE* %188)
  call void @llvm.lifetime.end.p0i8(i64 1, i8* nonnull %7) #27
  call void @llvm.lifetime.end.p0i8(i64 1, i8* nonnull %6)
  %192 = shl nsw i32 %157, 2
  %193 = and i32 %192, 60
  %194 = select i1 %151, i32 0, i32 %193
  %195 = zext i32 %194 to i64
  %196 = getelementptr [64 x i8], [64 x i8]* @uu_std, i64 0, i64 %195
  %197 = load i8, i8* %196, align 4
  call void @llvm.lifetime.start.p0i8(i64 1, i8* nonnull %4)
  store i8 %197, i8* %4, align 1
  call void @llvm.lifetime.start.p0i8(i64 1, i8* nonnull %5) #27
  store i8 13, i8* %5, align 1
  br i1 false, label %198, label %203

198:                                              ; preds = %187
  %199 = load %struct._IO_FILE*, %struct._IO_FILE** @stdout, align 8
  %200 = load i8, i8* %5, align 1
  %201 = sext i8 %200 to i32
  %202 = call i32 @fputc(i32 %201, %struct._IO_FILE* %199)
  br label %203

203:                                              ; preds = %198, %187
  %204 = load %struct._IO_FILE*, %struct._IO_FILE** @stdout, align 8
  %205 = load i8, i8* %4, align 1
  %206 = sext i8 %205 to i32
  %207 = call i32 @fputc(i32 %206, %struct._IO_FILE* %204)
  call void @llvm.lifetime.end.p0i8(i64 1, i8* nonnull %5) #27
  call void @llvm.lifetime.end.p0i8(i64 1, i8* nonnull %4)
  %208 = load %struct._IO_FILE*, %struct._IO_FILE** @stdout, align 8
  %209 = call i32 @fputc(i32 96, %struct._IO_FILE* %208)
  %210 = load %struct._IO_FILE*, %struct._IO_FILE** @stdout, align 8
  %211 = call i32 @fputc(i32 13, %struct._IO_FILE* %210)
  %212 = load %struct._IO_FILE*, %struct._IO_FILE** @stdout, align 8
  %213 = call i32 @fputc(i32 10, %struct._IO_FILE* %212)
  br label %214

214:                                              ; preds = %203, %41
  %215 = load %struct._IO_FILE*, %struct._IO_FILE** @stdout, align 8
  %216 = call i32 @fputc(i32 96, %struct._IO_FILE* %215)
  %217 = load %struct._IO_FILE*, %struct._IO_FILE** @stdout, align 8
  %218 = call i32 @fputc(i32 13, %struct._IO_FILE* %217)
  %219 = load %struct._IO_FILE*, %struct._IO_FILE** @stdout, align 8
  %220 = call i32 @fputc(i32 10, %struct._IO_FILE* %219)
  br label %221

221:                                              ; preds = %26, %214
  call void @llvm.lifetime.end.p0i8(i64 80, i8* nonnull %22) #27
  tail call void (i8*, ...) @t_printf(i8* noundef getelementptr inbounds ([6 x i8], [6 x i8]* @.str.2.90, i64 0, i64 0)) #27
  ret i32 0
}
