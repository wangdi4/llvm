; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -whole-program-assume -dtrans-outofboundsok=true -dtrans-fieldmodref-analysis -dtrans-fieldmodref-eval -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -dtrans-outofboundsok=true -passes='require<dtrans-fieldmodref-analysis>' -dtrans-fieldmodref-eval -disable-output 2>&1 | FileCheck %s

; This test is to check the getModRefInfo interface results that indicates
; whether a field member will be Mod, Ref, ModRef, or NoModRef when a function
; call is made.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.test01 = type { i32, i32* }
define %struct.test01* @test01() {
  %st_mem = call i8* @malloc(i64 16)
  %st = bitcast i8* %st_mem to %struct.test01*

  %scalar_field_addr = getelementptr %struct.test01, %struct.test01* %st, i64 0, i32 0
  store i32 8, i32* %scalar_field_addr

  %array_field_addr = getelementptr %struct.test01, %struct.test01* %st, i64 0, i32 1

  %ar1_mem = call i8* @malloc(i64 64)
  %cmp1 = icmp eq i8* %ar1_mem, null
  br i1 %cmp1, label %no_mem1, label %good1

no_mem1:
  store i32* null, i32** %array_field_addr
  br label %done

good1:
  call void @llvm.memset.p0i8.i64(i8* %ar1_mem, i8 0, i64 64, i1 false)

  %ar1_mem2 = bitcast i8* %ar1_mem to i32*
  store i32* %ar1_mem2, i32** %array_field_addr
  br label %done

done:
  ret %struct.test01* %st
}

; Function that will contain calls to check results for.
define void @test01process(%struct.test01* %in) {
  %scalar_field_addr = getelementptr %struct.test01, %struct.test01* %in, i64 0, i32 0
  %array_field_addr = getelementptr %struct.test01, %struct.test01* %in, i64 0, i32 1
  %scalar_field_value = load i32, i32* %scalar_field_addr
  %array_begin = load i32*, i32** %array_field_addr

  ; Should report REF for both fields
  call void @test01readers1(%struct.test01* %in)

  ; Also, should report REF for both fields
  call void @test01readers2(%struct.test01* %in)

  ; Also, should report REF for second field
  call void @test01readers3(%struct.test01* %in)

  ; Should report MOD for both fields.
  call void @test01writers1(%struct.test01* %in)

  ; Should report MODREF for the second field since values within are changing.
  ; This is a more conservative answer than necessary because the field itself
  ; is not changing, but for now to simplify interfacing with the loop opt it
  ; was decided that queries on the field would also report the status of the
  ; elements in the array when the field is a dynamic array.
  call void @test01writers2(%struct.test01* %in)

  ; Should report NOMODREF for both fields.
  call void @test01none()
  ret void
}

; Function that reads values of structure.
; References: Field 0, Field 1 address.
; Modifies: None
define void @test01readers1(%struct.test01* %in) {
  %scalar_field_addr = getelementptr %struct.test01, %struct.test01* %in, i64 0, i32 0
  load i32, i32* %scalar_field_addr
  %array_field_addr = getelementptr %struct.test01, %struct.test01* %in, i64 0, i32 1
  %array_begin = load i32*, i32** %array_field_addr
  %array_elem_addr = getelementptr i32, i32* %array_begin, i64 0

  ret void
}

; Function that does not read values of structure, but calls a function that does.
define void @test01readers2(%struct.test01* %in) {
  call void @test01readers1(%struct.test01* %in)
  ret void
}

; Function that reads elements of the array within the structure
; References: Field 1, and elements of the array.
; Modifies: None
define void @test01readers3(%struct.test01* %in) {
  %array_field_addr = getelementptr %struct.test01, %struct.test01* %in, i64 0, i32 1
  %array_begin = load i32*, i32** %array_field_addr
  %array_elem_addr = getelementptr i32, i32* %array_begin, i64 0
  %element_value = load i32, i32* %array_elem_addr

  ret void
}

; Function that writes values of structure
; References: Field 0, Field 1 address
; Modifies: Field 0, Field 1 address
define void @test01writers1(%struct.test01* %in) {
  %scalar_field_addr = getelementptr %struct.test01, %struct.test01* %in, i64 0, i32 0
  store i32 1, i32* %scalar_field_addr

  %array_field_addr = getelementptr %struct.test01, %struct.test01* %in, i64 0, i32 1
  store i32* null, i32** %array_field_addr
  ret void
}

; Function that writes elements of the array within the structure
; References: Field 1 address.
; Modifies: elements of the field 1 array.
define void @test01writers2(%struct.test01* %in) {
  %array_field_addr = getelementptr %struct.test01, %struct.test01* %in, i64 0, i32 1
  %array_begin = load i32*, i32** %array_field_addr
  %array_elem_addr = getelementptr i32, i32* %array_begin, i64 0
  %array_elem_addr_i8 = bitcast i32* %array_elem_addr to i8*
  call void @llvm.memset.p0i8.i64(i8* %array_elem_addr_i8, i8 0, i64 64, i1 false)
  ret void
}

; Function that does not use structure
define void @test01none() {
  ret void
}

; Test checking mod-ref information for items not tracked by the analysis to
; verify the interface returns a conservative answer.
@glob1 = internal global i32 zeroinitializer
%struct.test02 = type { %struct.test02** }
define void @test02process(%struct.test02* %in) {
  %ptr_field_addr = getelementptr %struct.test02, %struct.test02* %in, i64 0, i32 0
  %st = load %struct.test02**, %struct.test02*** %ptr_field_addr

  ; Load that does not come from GEP, should result in mod ref;
  %glob_val = load i32, i32* @glob1

  ; Should report MODREF for structure field and global since they are not
  ; candidates for this analysis.
  call void @test01none()
  ret void
}


define i32 @main() {
  %st = call %struct.test01* @test01()
  call void @test01process(%struct.test01* %st)

  %st2 = alloca %struct.test02
  call void @test02process(%struct.test02* %st2)

  ret i32 0
}

declare i8* @malloc(i64)
declare void @llvm.memset.p0i8.i64(i8*, i8, i64, i1)

; CHECK: FieldModRefQuery: - Ref        : [test01process]   %scalar_field_value = load i32, i32* %scalar_field_addr, align 4 --   call void @test01readers1(%struct.test01* %in)
; CHECK: FieldModRefQuery: - Ref        : [test01process]   %array_begin = load i32*, i32** %array_field_addr, align 8 --   call void @test01readers1(%struct.test01* %in)
; CHECK: FieldModRefQuery: - Ref        : [test01process]   %scalar_field_value = load i32, i32* %scalar_field_addr, align 4 --   call void @test01readers2(%struct.test01* %in)
; CHECK: FieldModRefQuery: - Ref        : [test01process]   %array_begin = load i32*, i32** %array_field_addr, align 8 --   call void @test01readers2(%struct.test01* %in)
; CHECK: FieldModRefQuery: - NoModRef   : [test01process]   %scalar_field_value = load i32, i32* %scalar_field_addr, align 4 --   call void @test01readers3(%struct.test01* %in)
; CHECK: FieldModRefQuery: - Ref        : [test01process]   %array_begin = load i32*, i32** %array_field_addr, align 8 --   call void @test01readers3(%struct.test01* %in)
; CHECK: FieldModRefQuery: - Mod        : [test01process]   %scalar_field_value = load i32, i32* %scalar_field_addr, align 4 --   call void @test01writers1(%struct.test01* %in)
; CHECK: FieldModRefQuery: - Mod        : [test01process]   %array_begin = load i32*, i32** %array_field_addr, align 8 --   call void @test01writers1(%struct.test01* %in)
; CHECK: FieldModRefQuery: - NoModRef   : [test01process]   %scalar_field_value = load i32, i32* %scalar_field_addr, align 4 --   call void @test01writers2(%struct.test01* %in)
; CHECK: FieldModRefQuery: - ModRef     : [test01process]   %array_begin = load i32*, i32** %array_field_addr, align 8 --   call void @test01writers2(%struct.test01* %in)
; CHECK: FieldModRefQuery: - NoModRef   : [test01process]   %scalar_field_value = load i32, i32* %scalar_field_addr, align 4 --   call void @test01none()
; CHECK: FieldModRefQuery: - NoModRef   : [test01process]   %array_begin = load i32*, i32** %array_field_addr, align 8 --   call void @test01none()
; CHECK: FieldModRefQuery: - ModRef     : [test02process]   %st = load %struct.test02**, %struct.test02*** %ptr_field_addr, align 8 --   call void @test01none()
; CHECK: FieldModRefQuery: - ModRef     : [test02process]   %glob_val = load i32, i32* @glob1, align 4 --   call void @test01none()
