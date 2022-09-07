; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers

; RUN: opt -whole-program-assume -dtransanalysis -debug-only=dtransanalysis -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtransanalysis>' -debug-only=dtransanalysis -disable-output %s 2>&1 | FileCheck %s

; This test verifies that a bitcast of a function pointer in a call instruction with var args
; is handled as a bitcast of the arguments.

; Test for checking var args
declare noalias i8* @malloc(i64)

declare void @llvm.va_start(i8*)
declare void @llvm.va_end(i8*)

declare void @llvm.memcpy.p0i8.p0i8.i64(i8* nocapture writeonly, i8* nocapture readonly, i64, i1) #2

; Test for checking var args are accessed with the same type
%struct.test2 = type { i32 }
%struct.__va_list_tag = type { i32, i32, i8*, i8* }

%struct.test3 = type { i32 }

define void @doNothing(i32 %n, ...) {
entry:
  %n.addr = alloca i32, align 4
  %vl = alloca [1 x %struct.__va_list_tag], align 16
  %val = alloca %struct.test2, align 4
  store i32 %n, i32* %n.addr, align 4
  %arraydecay = getelementptr inbounds [1 x %struct.__va_list_tag], [1 x %struct.__va_list_tag]* %vl, i32 0, i32 0
  %arraydecay1 = bitcast %struct.__va_list_tag* %arraydecay to i8*
  call void @llvm.va_start(i8* %arraydecay1)
  %arraydecay2 = getelementptr inbounds [1 x %struct.__va_list_tag], [1 x %struct.__va_list_tag]* %vl, i32 0, i32 0
  %gp_offset_p = getelementptr inbounds %struct.__va_list_tag, %struct.__va_list_tag* %arraydecay2, i32 0, i32 0
  %gp_offset = load i32, i32* %gp_offset_p, align 16
  %fits_in_gp = icmp ule i32 %gp_offset, 40
  br i1 %fits_in_gp, label %vaarg.in_reg, label %vaarg.in_mem

vaarg.in_reg:                                     ; preds = %entry
  %0 = getelementptr inbounds %struct.__va_list_tag, %struct.__va_list_tag* %arraydecay2, i32 0, i32 3
  %reg_save_area = load i8*, i8** %0, align 16
  %1 = getelementptr i8, i8* %reg_save_area, i32 %gp_offset
  %2 = bitcast i8* %1 to %struct.test2*
  %3 = add i32 %gp_offset, 8
  store i32 %3, i32* %gp_offset_p, align 16
  br label %vaarg.end

vaarg.in_mem:                                     ; preds = %entry
  %overflow_arg_area_p = getelementptr inbounds %struct.__va_list_tag, %struct.__va_list_tag* %arraydecay2, i32 0, i32 2
  %overflow_arg_area = load i8*, i8** %overflow_arg_area_p, align 8
  %4 = bitcast i8* %overflow_arg_area to %struct.test2*
  %overflow_arg_area.next = getelementptr i8, i8* %overflow_arg_area, i32 8
  store i8* %overflow_arg_area.next, i8** %overflow_arg_area_p, align 8
  br label %vaarg.end

vaarg.end:                                        ; preds = %vaarg.in_mem, %vaarg.in_reg
  %vaarg.addr = phi %struct.test2* [ %2, %vaarg.in_reg ], [ %4, %vaarg.in_mem ]
  %5 = bitcast %struct.test2* %val to i8*
  %6 = bitcast %struct.test2* %vaarg.addr to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 4 %5, i8* align 4 %6, i64 4, i1 false)
  %arraydecay3 = getelementptr inbounds [1 x %struct.__va_list_tag], [1 x %struct.__va_list_tag]* %vl, i32 0, i32 0
  %arraydecay34 = bitcast %struct.__va_list_tag* %arraydecay3 to i8*
  call void @llvm.va_end(i8* %arraydecay34)
  ret void

}

; Test that %struct.test2 isn't marked as Mismatched argument use casting due
; to vararg
define void @test(%struct.test2* %vp) {
  %p = call i8* @malloc(i64 16)
  call void bitcast (void (i32, ...)* @doNothing
                       to void (i32, %struct.test2*)*)(
                         i32 10, %struct.test2* %vp)
  ret void
}

; CHECK-NOT: Mismatched argument use -- bitcast function

; Test that %struct.test3 is marked as Mismatched argument use due to
; a vararg Type mismatch
define void @test2(%struct.test3* %vp) {
  %p = call i8* @malloc(i64 16)
  call void bitcast (void (i32, ...)* @doNothing
                       to void (i32, %struct.test3*)*)(
                         i32 10, %struct.test3* %vp)
  ret void
}

; CHECK: Mismatched argument use -- bitcast function
