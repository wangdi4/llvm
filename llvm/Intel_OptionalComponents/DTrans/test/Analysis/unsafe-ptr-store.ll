; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; The test checks that 'unsafe pointer store' issue was not triggered by assigning an array valiable to the pointer structure feild if underlying types are compatible.

%struct.S1 = type { i8* }
%struct.S2 = type { i8* }
%struct.S3 = type { i8* }
%struct.SP = type { [10 x i8]* }
%struct.SPP = type { [20 x i8]* }
%struct.SPPP = type { [10 x i8]* }

@char10_arr = external dso_local global [10 x i8], align 1
@int10_arr = external dso_local global [10 x i32], align 16

define dso_local i32 @foo_1(%struct.S1* nocapture %s1) local_unnamed_addr {
entry:
  %ptr1 = getelementptr inbounds %struct.S1, %struct.S1* %s1, i64 0, i32 0
  store i8* getelementptr inbounds ([10 x i8], [10 x i8]* @char10_arr, i64 0, i64 0), i8** %ptr1, align 8
  ret i32 0
}

; CHECK:  LLVMType: %struct.S1 = type { i8* }
; CHECK:  Safety data: No issues found


define dso_local i32 @foo_2(%struct.S2* nocapture %s2) local_unnamed_addr {
entry:
  %ptr2 = getelementptr inbounds %struct.S2, %struct.S2* %s2, i64 0, i32 0
  store i8* bitcast ([10 x i32]* @int10_arr to i8*), i8** %ptr2, align 8
  ret i32 0
}

; CHECK:  LLVMType: %struct.S2 = type { i8* }
; CHECK:  Safety data: Unsafe pointer store


define dso_local i32 @foo_3(i32* %int_ptr, %struct.S3* nocapture %s3) local_unnamed_addr {
entry:
  %ptr3 = getelementptr inbounds %struct.S3, %struct.S3* %s3, i64 0, i32 0
  %tmp = bitcast i8** %ptr3 to i32**
  store i32* %int_ptr, i32** %tmp, align 8
  ret i32 0
}

; CHECK:  LLVMType: %struct.S3 = type { i8* }
; CHECK:  Safety data: Bad casting | Mismatched element access


define dso_local i32 @foo_p(%struct.SP* nocapture %sp) local_unnamed_addr {
entry:
  %ptrP = getelementptr inbounds %struct.SP, %struct.SP* %sp, i64 0, i32 0
  store [10 x i8]* @char10_arr, [10 x i8]** %ptrP, align 8
  ret i32 0
}

; CHECK:  LLVMType: %struct.SP = type { [10 x i8]* }
; CHECK:  Safety data: No issues found


define dso_local i32 @foo_pp(%struct.SPP* nocapture %spp) local_unnamed_addr {
entry:
  %ptrPP = getelementptr inbounds %struct.SPP, %struct.SPP* %spp, i64 0, i32 0
  store [20 x i8]* bitcast ([10 x i8]* @char10_arr to [20 x i8]*), [20 x i8]** %ptrPP, align 8
  ret i32 0
}

; CHECK:  LLVMType: %struct.SPP = type { [20 x i8]* }
; CHECK:  Safety data: Unsafe pointer store


define dso_local i32 @foo_ppp(%struct.SPPP* nocapture %sppp) local_unnamed_addr {
entry:
  %ptrPPP = getelementptr inbounds %struct.SPPP, %struct.SPPP* %sppp, i64 0, i32 0
  store [10 x i8]* bitcast ([10 x i32]* @int10_arr to [10 x i8]*), [10 x i8]** %ptrPPP, align 8
  ret i32 0
}

; CHECK:  LLVMType: %struct.SPPP = type { [10 x i8]* }
; CHECK:  Safety data: Unsafe pointer store

