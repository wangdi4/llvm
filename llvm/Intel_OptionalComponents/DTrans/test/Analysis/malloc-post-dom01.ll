; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt  < %s -whole-program-assume -dtransanalysis -debug-only=dtransanalysis -disable-output 2>&1 | FileCheck %s
; RUN: opt  < %s -whole-program-assume -passes='require<dtransanalysis>' -debug-only=dtransanalysis -disable-output 2>&1 | FileCheck %s

; Check that x264_malloc0 and x264_malloc1, two variant forms of user malloc,
; are recognized as MallocPostDom functions.

; CHECK-DAG: Is MallocPostDom x264_malloc0
; CHECK-DAG: Is MallocPostDom x264_malloc1

declare dso_local noalias noundef align 16 i8* @malloc(i64 noundef) local_unnamed_addr

%struct.x264_t = type { %struct.x264_param_t }

%struct.x264_param_t = type { i32, i32, i32 }

@.str.156 = private unnamed_addr constant [26 x i8] c"malloc of size %d failed\0A\00", align 1

declare void @x264_log(%struct.x264_t* readonly %0, i32 %1, i8* %2, ...)

define internal i8* @x264_malloc0(i32 %0) #2 {
  %2 = sext i32 %0 to i64
  %3 = add nsw i64 %2, 27
  %4 = tail call noalias align 16 i8* @malloc(i64 %3) #42
  %5 = icmp eq i8* %4, null
  br i1 %5, label %15, label %6

6:                                                ; preds = %1
  %7 = getelementptr inbounds i8, i8* %4, i64 15
  %8 = getelementptr inbounds i8, i8* %7, i64 8
  %9 = getelementptr inbounds i8, i8* %8, i64 4
  %10 = getelementptr inbounds i8, i8* %9, i64 -11
  %11 = getelementptr inbounds i8, i8* %10, i64 -8
  %12 = bitcast i8* %11 to i8**
  store i8* %4, i8** %12, align 8
  %13 = getelementptr inbounds i8, i8* %11, i64 -4
  %14 = bitcast i8* %13 to i32*
  store i32 %0, i32* %14, align 4
  br label %16

15:                                               ; preds = %1
  tail call void (%struct.x264_t*, i32, i8*, ...) @x264_log(%struct.x264_t* null, i32 0, i8* getelementptr inbounds ([26 x i8], [26 x i8]* @.str.156, i64 0, i64 0), i32 %0)
  br label %16

16:                                               ; preds = %15, %6
  %17 = phi i8* [ null, %15 ], [ %10, %6 ]
  ret i8* %17
}

define internal i8* @x264_malloc1(i32 %0) #2 {
  %2 = sext i32 %0 to i64
  %3 = add nsw i64 %2, 27
  %4 = tail call noalias i8* @malloc(i64 %3) #39
  %5 = icmp eq i8* %4, null
  br i1 %5, label %18, label %6

6:                                                ; preds = %1
  %7 = getelementptr inbounds i8, i8* %4, i64 15
  %8 = getelementptr inbounds i8, i8* %7, i64 8
  %9 = getelementptr inbounds i8, i8* %8, i64 4
  %10 = ptrtoint i8* %9 to i64
  %11 = and i64 %10, 15
  %12 = sub nsw i64 0, %11
  %13 = getelementptr inbounds i8, i8* %9, i64 %12
  %14 = getelementptr inbounds i8, i8* %13, i64 -8
  %15 = bitcast i8* %14 to i8**
  store i8* %4, i8** %15, align 8
  %16 = getelementptr inbounds i8, i8* %14, i64 -4
  %17 = bitcast i8* %16 to i32*
  store i32 %0, i32* %17, align 4
  br label %19

18:                                               ; preds = %1
  tail call void (%struct.x264_t*, i32, i8*, ...) @x264_log(%struct.x264_t* null, i32 0, i8* getelementptr inbounds ([26 x i8], [26 x i8]* @.str.156, i64 0, i64 0), i32 %0)
  br label %19

19:                                               ; preds = %18, %6
  %20 = phi i8* [ null, %18 ], [ %13, %6 ]
  ret i8* %20
}


