; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt  < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt  < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; Check that we see 'No safety issues' with @malloc and @free
; Bit casting of malloc result is used directly in later instructions in @main
;
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

define dso_local i32 @main() #2 {
  %1 = tail call i8* @malloc(i64 416)
  %2 = bitcast i8* %1 to %struct.x264_t*
  %3 = getelementptr inbounds %struct.x264_t, %struct.x264_t* %2, i32 0, i32 1, i32 0
  store %struct.x264_t* %2, %struct.x264_t** %3, align 16
  call void @free(i8* %1)
  ret i32 0
}
