; RUN: opt  < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; Check that we don't see 'Address Taken' due to x264_free.
; Bit cast after call to @x264_malloc in @main is essentially unused.
; This is a version of mallocfreepostdom01.ll with early GEP lowering off.


; CHECK: DTRANS_StructInfo:
; CHECK:  LLVMType: %struct.x264_t = type { i32, [50 x %struct.x264_t*], float }
; CHECK:  Safety data: No issues found
; CHECK: DTRANS_ArrayInfo:
; CHECK:   LLVMType: [50 x %struct.x264_t*]
; CHECK:  Safety data: No issues found

@.str.288 = private unnamed_addr constant [6 x i8] c"Hello\0A"

%struct.x264_t = type { i32, [50 x %struct.x264_t*], float }

declare void @free(i8* nocapture)

declare noalias i8* @malloc(i64)

declare dso_local i32 @printf(i8* nocapture readonly, ...)

define internal i8* @x264_malloc(i32) {
  %2 = add nsw i32 %0, 15
  %3 = sext i32 %2 to i64
  %4 = add nsw i64 %3, 12
  %5 = tail call noalias i8* @malloc(i64 %4)
  %6 = icmp eq i8* %5, null
  br i1 %6, label %19, label %7

; <label>:7:
  %8 = getelementptr inbounds i8, i8* %5, i64 15
  %9 = getelementptr inbounds i8, i8* %8, i64 8
  %10 = getelementptr inbounds i8, i8* %9, i64 4
  %11 = ptrtoint i8* %10 to i64
  %12 = and i64 %11, 15
  %13 = sub nsw i64 0, %12
  %14 = getelementptr inbounds i8, i8* %10, i64 %13
  %15 = getelementptr inbounds i8, i8* %14, i64 -8
  %16 = bitcast i8* %15 to i8**
  store i8* %5, i8** %16, align 8
  %17 = getelementptr inbounds i8, i8* %15, i64 -4
  %18 = bitcast i8* %17 to i32*
  store i32 %0, i32* %18, align 4
  br label %20

; <label>:19:
  %call = call i32 (i8*, ...)  @printf(i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str.288, i32 0, i32 0))
  br label %20

; <label>:20:
  %21 = phi i8* [ %14, %7 ], [ null, %19 ]
  ret i8* %21
}

define internal void @x264_free(i8* readonly) {
  %2 = icmp eq i8* %0, null
  br i1 %2, label %7, label %3

; <label>:3:
  %4 = getelementptr inbounds i8, i8* %0, i64 -8
  %5 = bitcast i8* %4 to i8**
  %6 = load i8*, i8** %5, align 8
  tail call void @free(i8* %6) #11
  br label %7

; <label>:7:
  ret void
}

define dso_local i32 @main() #2 {
  %1 = tail call i8* @x264_malloc(i32 416)
  %2 = bitcast i8* %1 to %struct.x264_t*
  %3 = getelementptr inbounds i8, i8* %1, i64 8
  %4 = bitcast i8* %3 to i8**
  store i8* %1, i8** %4, align 16
  call void @x264_free(i8* %1)
  ret i32 0
}



