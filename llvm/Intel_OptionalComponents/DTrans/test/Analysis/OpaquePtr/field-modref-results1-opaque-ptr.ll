; REQUIRES: asserts

; RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -dtrans-outofboundsok=true -passes='require<dtrans-fieldmodrefop-analysis>' -dtrans-fieldmodref-eval -disable-output 2>&1 | FileCheck %s

; This test is to check the getModRefInfo interface results that indicates
; whether a field member will be Mod, Ref, ModRef, or NoModRef when a function
; call is made.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.test01 = type { i32, ptr }
define "intel_dtrans_func_index"="1" ptr @test01() !intel.dtrans.func.type !4 {
  %st = call ptr @malloc(i64 16)

  %scalar_field_addr = getelementptr %struct.test01, ptr %st, i64 0, i32 0
  store i32 8, ptr %scalar_field_addr

  %array_field_addr = getelementptr %struct.test01, ptr %st, i64 0, i32 1

  %ar1_mem = call ptr @malloc(i64 64)
  %cmp1 = icmp eq ptr %ar1_mem, null
  br i1 %cmp1, label %no_mem1, label %good1

no_mem1:
  store ptr null, ptr %array_field_addr
  br label %done

good1:
  call void @llvm.memset.p0i8.i64(ptr %ar1_mem, i8 0, i64 64, i1 false)

  store ptr %ar1_mem, ptr %array_field_addr
  br label %done

done:
  ret ptr %st
}

; Function that will contain calls to check results for.
define void @test01process(ptr "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !5 {
  %scalar_field_addr = getelementptr %struct.test01, ptr %in, i64 0, i32 0
  %array_field_addr = getelementptr %struct.test01, ptr %in, i64 0, i32 1
  %scalar_field_value = load i32, ptr %scalar_field_addr
  %array_begin = load ptr, ptr %array_field_addr

  ; Should report REF for both fields
  call void @test01readers1(ptr %in)

  ; Also, should report REF for both fields
  call void @test01readers2(ptr %in)

  ; Also, should report REF for second field
  call void @test01readers3(ptr %in)

  ; Should report MOD for both fields.
  call void @test01writers1(ptr %in)

  ; Should report MODREF for the second field since values within the memory
  ; pointed to by it are changing.
  ; This is a more conservative answer than necessary because the field itself
  ; is not changing, but for now to simplify interfacing with the loop opt it
  ; was decided that queries on the field would also report the status of the
  ; elements in the array when the field is a dynamic array.
  call void @test01writers2(ptr %in)

  ; Should report NOMODREF for both fields.
  call void @test01none()
  ret void
}

; Function that reads values of structure.
; References: Field 0, Field 1 address.
; Modifies: None
define void @test01readers1(ptr "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !6 {
  %scalar_field_addr = getelementptr %struct.test01, ptr %in, i64 0, i32 0
  load i32, ptr %scalar_field_addr
  %array_field_addr = getelementptr %struct.test01, ptr %in, i64 0, i32 1
  %array_begin = load ptr, ptr %array_field_addr
  %array_elem_addr = getelementptr i32, ptr %array_begin, i64 0

  ret void
}

; Function that does not read values of structure, but calls a function that does.
define void @test01readers2(ptr "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !7 {
  call void @test01readers1(ptr %in)
  ret void
}

; Function that reads elements of the array within the structure
; References: Field 1, and elements of the array.
; Modifies: None
define void @test01readers3(ptr "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !8 {
  %array_field_addr = getelementptr %struct.test01, ptr %in, i64 0, i32 1
  %array_begin = load ptr, ptr %array_field_addr
  %array_elem_addr = getelementptr i32, ptr %array_begin, i64 0
  %element_value = load i32, ptr %array_elem_addr

  ret void
}

; Function that writes values of structure
; References: Field 0, Field 1 address
; Modifies: Field 0, Field 1 address
define void @test01writers1(ptr "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !9 {
  %scalar_field_addr = getelementptr %struct.test01, ptr %in, i64 0, i32 0
  store i32 1, ptr %scalar_field_addr

  %array_field_addr = getelementptr %struct.test01, ptr %in, i64 0, i32 1
  store ptr null, ptr %array_field_addr
  ret void
}

; Function that writes elements of the array within the structure
; References: Field 1 address.
; Modifies: elements of the field 1 array.
define void @test01writers2(ptr "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !10 {
  %array_field_addr = getelementptr %struct.test01, ptr %in, i64 0, i32 1
  %array_begin = load ptr, ptr %array_field_addr
  %array_elem_addr = getelementptr i32, ptr %array_begin, i64 0
  call void @llvm.memset.p0i8.i64(ptr %array_elem_addr, i8 0, i64 64, i1 false)
  ret void
}

; Function that does not use structure
define void @test01none() {
  ret void
}

; Test checking mod/ref information for items not tracked by the analysis to
; verify the interface returns a conservative answer.
@glob1 = internal global i32 zeroinitializer
%struct.test02 = type { ptr }
define void @test02process(ptr "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !13 {
  %ptr_field_addr = getelementptr %struct.test02, ptr %in, i64 0, i32 0
  %st = load ptr, ptr %ptr_field_addr

  ; Load that does not come from GEP, should result in mod ref;
  %glob_val = load i32, ptr @glob1

  ; Should report MODREF for structure field and global since they are not
  ; candidates for this analysis.
  call void @test01none()
  ret void
}


define i32 @main() {
  %st = call ptr @test01()
  call void @test01process(ptr %st)

  %st2 = alloca %struct.test02
  call void @test02process(ptr %st2)

  ret i32 0
}

declare !intel.dtrans.func.type !15 "intel_dtrans_func_index"="1" ptr @malloc(i64) #0
declare !intel.dtrans.func.type !16 void @llvm.memset.p0i8.i64(ptr "intel_dtrans_func_index"="1", i8, i64, i1)

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }

; CHECK: FieldModRefQuery: - Ref        : [test01process]   %scalar_field_value = load i32, ptr %scalar_field_addr, align 4 --   call void @test01readers1(ptr %in)
; CHECK: FieldModRefQuery: - Ref        : [test01process]   %array_begin = load ptr, ptr %array_field_addr, align 8 --   call void @test01readers1(ptr %in)
; CHECK: FieldModRefQuery: - Ref        : [test01process]   %scalar_field_value = load i32, ptr %scalar_field_addr, align 4 --   call void @test01readers2(ptr %in)
; CHECK: FieldModRefQuery: - Ref        : [test01process]   %array_begin = load ptr, ptr %array_field_addr, align 8 --   call void @test01readers2(ptr %in)
; CHECK: FieldModRefQuery: - NoModRef   : [test01process]   %scalar_field_value = load i32, ptr %scalar_field_addr, align 4 --   call void @test01readers3(ptr %in)
; CHECK: FieldModRefQuery: - Ref        : [test01process]   %array_begin = load ptr, ptr %array_field_addr, align 8 --   call void @test01readers3(ptr %in)
; CHECK: FieldModRefQuery: - Mod        : [test01process]   %scalar_field_value = load i32, ptr %scalar_field_addr, align 4 --   call void @test01writers1(ptr %in)
; CHECK: FieldModRefQuery: - Mod        : [test01process]   %array_begin = load ptr, ptr %array_field_addr, align 8 --   call void @test01writers1(ptr %in)
; CHECK: FieldModRefQuery: - NoModRef   : [test01process]   %scalar_field_value = load i32, ptr %scalar_field_addr, align 4 --   call void @test01writers2(ptr %in)
; CHECK: FieldModRefQuery: - ModRef     : [test01process]   %array_begin = load ptr, ptr %array_field_addr, align 8 --   call void @test01writers2(ptr %in)
; CHECK: FieldModRefQuery: - NoModRef   : [test01process]   %scalar_field_value = load i32, ptr %scalar_field_addr, align 4 --   call void @test01none()
; CHECK: FieldModRefQuery: - NoModRef   : [test01process]   %array_begin = load ptr, ptr %array_field_addr, align 8 --   call void @test01none()
; CHECK: FieldModRefQuery: - ModRef     : [test02process]   %st = load ptr, ptr %ptr_field_addr, align 8 --   call void @test01none()
; CHECK: FieldModRefQuery: - ModRef     : [test02process]   %glob_val = load i32, ptr @glob1, align 4 --   call void @test01none()


!1 = !{i32 0, i32 0}  ; i32
!2 = !{i32 0, i32 1}  ; i32*
!3 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!4 = distinct !{!3}
!5 = distinct !{!3}
!6 = distinct !{!3}
!7 = distinct !{!3}
!8 = distinct !{!3}
!9 = distinct !{!3}
!10 = distinct !{!3}
!11 = !{%struct.test02 zeroinitializer, i32 2}  ; %struct.test02**
!12 = !{%struct.test02 zeroinitializer, i32 1}  ; %struct.test02*
!13 = distinct !{!12}
!14 = !{i8 0, i32 1}  ; i8*
!15 = distinct !{!14}
!16 = distinct !{!14}
!17 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !2} ; { i32, i32* }
!18 = !{!"S", %struct.test02 zeroinitializer, i32 1, !11} ; { %struct.test02** }

!intel.dtrans.types = !{!17, !18}

