; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test cases for passing VarArg parameters which are safe for DTrans.

; The data structure used by the VarArgs interface
%struct._ZTS13__va_list_tag.__va_list_tag = type { i32, i32, i8*, i8* }
; TODO: This structure should also get marked as "Unhandled use" because of the
;       calls to llvm.va_start, but that will not occur until
;       "visitIntrinsicInst" inside the DTransSafetyAnalyzer is implemented.

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct._ZTS13__va_list_tag.__va_list_tag
; CHECK: Safety data: Local instance{{ *}}


; Test with a direct call to a VarArg function which does not use the argument.
%struct.test01 = type { i32 }
define void @test01(%struct.test01* %in) !dtrans_type !2 {
  tail call void (i8, ...) @test01va(i8 0, %struct.test01* %in)
  ret void
}

  define void @test01va(i8 %cnt, ...) {
    %cmp = icmp eq i8 %cnt, 0
    ret void
  }

  ; TODO: Currently, the safety analyzer marks this as "Unhandled use" because
  ;       checking whether the VarArg elements get used is not implemented yet.
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01
; CHECK: Safety data: Unhandled use{{ *}}


; Test with a bitcast function call. This case is safe because the parameter in
; the VarArg position is never referenced by the callee
%struct.test02 = type { i32 }
define void @doNothing1(i8*, ...) !dtrans_type !6 { ret void }
define void @test02(%struct.test02* %in) !dtrans_type !8 {
  %p = call i8* @malloc(i64 16)
  call void bitcast (void (i8*, ...)* @doNothing1
                       to void (i8*, %struct.test02*)*)(
                         i8* %p, %struct.test02* %in)
  ret void
}

  ; TODO: Currently, the safety analyzer marks this as "Unhandled use" because
  ;       checking whether the VarArg elements get used is not implemented yet.
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test02
; CHECK: Safety data: Unhandled use{{ *}}


; Test a case where the VarArg element is used with the expected type.
%struct.test03 = type { i32 }
define void @test03(%struct.test03* %in) !dtrans_type !11 {
  call void (i8, ...) @test03va(i8 1, %struct.test03* %in)
  ret void
}

define void @test03va(i8 %in, ...) {
entry:
  %args = alloca [1 x %struct._ZTS13__va_list_tag.__va_list_tag]
  %args.as.p8 = bitcast [1 x %struct._ZTS13__va_list_tag.__va_list_tag]* %args to i8*
  call void @llvm.va_start(i8* %args.as.p8)
  %gp_offset_p = getelementptr inbounds [1 x %struct._ZTS13__va_list_tag.__va_list_tag], [1 x %struct._ZTS13__va_list_tag.__va_list_tag]* %args, i64 0, i64 0, i32 0
  %gp_offset = load i32, i32* %gp_offset_p
  %fits_in_gp = icmp ult i32 %gp_offset, 41
  br i1 %fits_in_gp, label %vaarg.in_reg, label %vaarg.in_mem

vaarg.in_reg:                                     ; preds = %entry
  %f3 = getelementptr inbounds [1 x %struct._ZTS13__va_list_tag.__va_list_tag], [1 x %struct._ZTS13__va_list_tag.__va_list_tag]* %args, i64 0, i64 0, i32 3
  %reg_save_area = load i8*, i8** %f3
  %gp_offset.i64 = zext i32 %gp_offset to i64
  %reg_mem = getelementptr i8, i8* %reg_save_area, i64 %gp_offset.i64
  %gp_offset2 = add nuw nsw i32 %gp_offset, 8
  store i32 %gp_offset2, i32* %gp_offset_p
  br label %vaarg.end

vaarg.in_mem:                                     ; preds = %entry
  %overflow_arg_area_p = getelementptr inbounds [1 x %struct._ZTS13__va_list_tag.__va_list_tag], [1 x %struct._ZTS13__va_list_tag.__va_list_tag]* %args, i64 0, i64 0, i32 2
  %overflow_arg_area = load i8*, i8** %overflow_arg_area_p
  %overflow_arg_area.next = getelementptr i8, i8* %overflow_arg_area, i64 8
  store i8* %overflow_arg_area.next, i8** %overflow_arg_area_p
  br label %vaarg.end

vaarg.end:                                        ; preds = %vaarg.in_mem, %vaarg.in_reg
  %vaarg.addr.in = phi i8* [ %reg_mem, %vaarg.in_reg ], [ %overflow_arg_area, %vaarg.in_mem ]
  %vaarg.addr = bitcast i8* %vaarg.addr.in to %struct.test03**
  %pStruct = load %struct.test03*, %struct.test03** %vaarg.addr
  %R = getelementptr inbounds %struct.test03, %struct.test03* %pStruct, i64 0, i32 0
  store i32 5, i32* %R
  call void @llvm.va_end(i8* %args.as.p8)
  ret void
}
; TODO: This case could be treated as safe in the future, if necessary. The
;       local pointer analyzer DTrans implementation only partially supported
;       it, by not setting the "Mismatched argument use" safety flag, but still
;       had the "Bad casting" flag set.

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test03
; CHECK: Safety data: Unhandled use{{ *}}

declare i8* @malloc(i64)
declare void @llvm.va_start(i8*)
declare void @llvm.va_end(i8*)

!1 = !{i32 0, i32 0}  ; i32
!2 = !{!"F", i1 false, i32 1, !3, !4}  ; void (%struct.test01*)
!3 = !{!"void", i32 0}  ; void
!4 = !{!5, i32 1}  ; %struct.test01*
!5 = !{!"R", %struct.test01 zeroinitializer, i32 0}  ; %struct.test01
!6 = !{!"F", i1 true, i32 1, !3, !7}  ; void (i8*, ...)
!7 = !{i8 0, i32 1}  ; i8*
!8 = !{!"F", i1 false, i32 1, !3, !9}  ; void (%struct.test02*)
!9 = !{!10, i32 1}  ; %struct.test02*
!10 = !{!"R", %struct.test02 zeroinitializer, i32 0}  ; %struct.test02
!11 = !{!"F", i1 false, i32 1, !3, !12}  ; void (%struct.test03*)
!12 = !{!13, i32 1}  ; %struct.test03*
!13 = !{!"R", %struct.test03 zeroinitializer, i32 0}  ; %struct.test03
!14 = !{!"S", %struct.test01 zeroinitializer, i32 1, !1} ; { i32 }
!15 = !{!"S", %struct.test02 zeroinitializer, i32 1, !1} ; { i32 }
!16 = !{!"S", %struct._ZTS13__va_list_tag.__va_list_tag zeroinitializer, i32 4, !1, !1, !7, !7} ; { i32, i32, i8*, i8* }
!17 = !{!"S", %struct.test03 zeroinitializer, i32 1, !1} ; { i32 }

!dtrans_types = !{!14, !15, !16, !17}
