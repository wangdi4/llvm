; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test cases for passing VarArg parameters which are safe for DTrans.

; The data structure used by the VarArgs interface
%struct._ZTS13__va_list_tag.__va_list_tag = type { i32, i32, ptr, ptr }
; TODO: This structure should also get marked as "Unhandled use" because of the
;       calls to llvm.va_start, but that will not occur until
;       "visitIntrinsicInst" inside the DTransSafetyAnalyzer is implemented.

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct._ZTS13__va_list_tag.__va_list_tag
; CHECK: Safety data: Local instance{{ *}}
; CHECK: End LLVMType: %struct._ZTS13__va_list_tag.__va_list_tag


; Test with a direct call to a VarArg function which does not use the argument.
%struct.test01 = type { i32 }
define void @test01(ptr "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !4 {
  tail call void (i8, ...) @test01va(i8 0, ptr %in)
  ret void
}

  define void @test01va(i8 %cnt, ...) {
    %cmp = icmp eq i8 %cnt, 0
    ret void
  }

  ; TODO: Currently, the safety analyzer marks this as "Unhandled use" because
  ;       checking whether the VarArg elements get used is not implemented yet.
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: Safety data: Unhandled use{{ *}}
; CHECK: End LLVMType: %struct.test01


; Test with a bitcast function call. This case is safe because the parameter in
; the VarArg position is never referenced by the callee
%struct.test02 = type { i32 }
define void @doNothing1(ptr "intel_dtrans_func_index"="1", ...) !intel.dtrans.func.type !5 { ret void }
define void @test02(ptr "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !7 {
  %p = call ptr @malloc(i64 16)
  call void bitcast (ptr @doNothing1
                       to ptr)(
                         ptr %p, ptr %in)
  ret void
}

  ; TODO: Currently, the safety analyzer marks this as "Unhandled use" because
  ;       checking whether the VarArg elements get used is not implemented yet.
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02
; CHECK: Safety data: Unhandled use{{ *}}
; CHECK: End LLVMType: %struct.test02


; Test a case where the VarArg element is used with the expected type.
%struct.test03 = type { i32 }
define void @test03(ptr "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !9 {
  call void (i8, ...) @test03va(i8 1, ptr %in)
  ret void
}

define void @test03va(i8 %in, ...) {
entry:
  %args = alloca [1 x %struct._ZTS13__va_list_tag.__va_list_tag]
  %args.as.p8 = bitcast ptr %args to ptr
  call void @llvm.va_start(ptr %args.as.p8)
  %gp_offset_p = getelementptr inbounds [1 x %struct._ZTS13__va_list_tag.__va_list_tag], ptr %args, i64 0, i64 0, i32 0
  %gp_offset = load i32, ptr %gp_offset_p
  %fits_in_gp = icmp ult i32 %gp_offset, 41
  br i1 %fits_in_gp, label %vaarg.in_reg, label %vaarg.in_mem

vaarg.in_reg:                                     ; preds = %entry
  %f3 = getelementptr inbounds [1 x %struct._ZTS13__va_list_tag.__va_list_tag], ptr %args, i64 0, i64 0, i32 3
  %reg_save_area = load ptr, ptr %f3
  %gp_offset.i64 = zext i32 %gp_offset to i64
  %reg_mem = getelementptr i8, ptr %reg_save_area, i64 %gp_offset.i64
  %gp_offset2 = add nuw nsw i32 %gp_offset, 8
  store i32 %gp_offset2, ptr %gp_offset_p
  br label %vaarg.end

vaarg.in_mem:                                     ; preds = %entry
  %overflow_arg_area_p = getelementptr inbounds [1 x %struct._ZTS13__va_list_tag.__va_list_tag], ptr %args, i64 0, i64 0, i32 2
  %overflow_arg_area = load ptr, ptr %overflow_arg_area_p
  %overflow_arg_area.next = getelementptr i8, ptr %overflow_arg_area, i64 8
  store ptr %overflow_arg_area.next, ptr %overflow_arg_area_p
  br label %vaarg.end

vaarg.end:                                        ; preds = %vaarg.in_mem, %vaarg.in_reg
  %vaarg.addr.in = phi ptr [ %reg_mem, %vaarg.in_reg ], [ %overflow_arg_area, %vaarg.in_mem ]
  %pStruct = load ptr, ptr %vaarg.addr.in
  %R = getelementptr inbounds %struct.test03, ptr %pStruct, i64 0, i32 0
  store i32 5, ptr %R
  call void @llvm.va_end(ptr %args.as.p8)
  ret void
}
; TODO: This case could be treated as safe in the future, if necessary. The
;       local pointer analyzer DTrans implementation only partially supported
;       it, by not setting the "Mismatched argument use" safety flag, but still
;       had the "Bad casting" flag set.

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test03
; CHECK: Safety data: Bad casting | Unhandled use{{ *}}
; CHECK: End LLVMType: %struct.test03

declare !intel.dtrans.func.type !10 "intel_dtrans_func_index"="1" ptr @malloc(i64)
declare !intel.dtrans.func.type !11 void @llvm.va_start(ptr "intel_dtrans_func_index"="1")
declare !intel.dtrans.func.type !12 void @llvm.va_end(ptr "intel_dtrans_func_index"="1")

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i8 0, i32 1}  ; i8*
!3 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!4 = distinct !{!3}
!5 = distinct !{!2}
!6 = !{%struct.test02 zeroinitializer, i32 1}  ; %struct.test02*
!7 = distinct !{!6}
!8 = !{%struct.test03 zeroinitializer, i32 1}  ; %struct.test03*
!9 = distinct !{!8}
!10 = distinct !{!2}
!11 = distinct !{!2}
!12 = distinct !{!2}
!13 = !{!"S", %struct._ZTS13__va_list_tag.__va_list_tag zeroinitializer, i32 4, !1, !1, !2, !2} ; { i32, i32, i8*, i8* }
!14 = !{!"S", %struct.test01 zeroinitializer, i32 1, !1} ; { i32 }
!15 = !{!"S", %struct.test02 zeroinitializer, i32 1, !1} ; { i32 }
!16 = !{!"S", %struct.test03 zeroinitializer, i32 1, !1} ; { i32 }

!intel.dtrans.types = !{!13, !14, !15, !16}
