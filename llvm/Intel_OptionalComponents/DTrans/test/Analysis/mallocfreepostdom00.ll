; RUN: opt  < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; Check that we don't see 'Address Taken' due to x264_free.
; Bit cast after call to @malloc in @main is essentially unused.

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
  %1 = tail call i8* @malloc(i64 416)
  %2 = bitcast i8* %1 to %struct.x264_t*
  %3 = getelementptr inbounds i8, i8* %1, i64 8
  %4 = bitcast i8* %3 to i8**
  store i8* %1, i8** %4, align 16
  call void @free(i8* %1)
  ret i32 0
}



