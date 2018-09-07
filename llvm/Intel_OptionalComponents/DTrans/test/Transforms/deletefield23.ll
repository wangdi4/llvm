; RUN: opt  -whole-program-assume -dtrans-identify-unused-values=false -dtrans-deletefield -S -o - %s | FileCheck %s
; RUN: opt  -whole-program-assume -dtrans-identify-unused-values=false -passes=dtrans-deletefield -S -o - %s | FileCheck %s

; This test verifies that the dtrans delete pass is able to handle global
; variables with types that are dependent on a type having fields deleted.

%struct.del = type { i32, i64, i32 }
%struct.dep = type { i32, %struct.del* }
@g_del = private global %struct.del { i32 100, i64 1000, i32 10000 }, align 4
@g_dep = private global %struct.dep zeroinitializer, align 4


define i32 @main(i32 %argc, i8** %argv) {
  ; read dep.A
  %val = load i32, i32* getelementptr inbounds (%struct.dep,
                                                %struct.dep* @g_dep,
                                                i64 0, i32 0)

  ; Read and write a pointer to del from dep
  store %struct.del* @g_del, %struct.del** getelementptr (%struct.dep,
                                                          %struct.dep* @g_dep,
                                                          i64 0, i32 1)
  %p = load %struct.del*, %struct.del** getelementptr inbounds (%struct.dep,
                                                                %struct.dep*
                                                                    @g_dep,
                                                                i64 0, i32 1)

  ; read A and C
  %valA = load i32, i32* getelementptr inbounds (%struct.del,
                                                 %struct.del* @g_del,
                                                 i64 0, i32 0)
  %valC = load i32, i32* getelementptr inbounds (%struct.del,
                                                 %struct.del* @g_del,
                                                 i64 0, i32 2)

  ; write B
  store i64 3, i64* bitcast (i8* bitcast (i64* getelementptr
                                           (%struct.del, %struct.del* @g_del,
                                            i64 0, i32 1) to i8*) to i64*)

  ret i32 %valA
}

; CHECK: %__DFT_struct.del = type { i32, i32 }
; CHECK: %__DFDT_struct.dep = type { i32, %__DFT_struct.del* }

; CHECK: @g_del = private global %__DFT_struct.del { i32 100, i32 10000 }, align 4
; CHECK: @g_dep = private global %__DFDT_struct.dep zeroinitializer, align 4

; CHECK-LABEL: define i32 @main(i32 %argc, i8** %argv)
; CHECK: %val = load i32, i32* getelementptr inbounds (%__DFDT_struct.dep,
; CHECK-SAME:                                       %__DFDT_struct.dep* @g_dep,
; CHECK-SAME:                                       i64 0, i32 0)
; CHECK: store %__DFT_struct.del* @g_del, %__DFT_struct.del**
; CHECK-SAME:                             getelementptr inbounds
; CHECK-SAME:                                      (%__DFDT_struct.dep,
; CHECK-SAME:                                       %__DFDT_struct.dep* @g_dep,
; CHECK-SAME:                                       i64 0, i32 1)
; CHECK: %p = load %__DFT_struct.del*, %__DFT_struct.del**
; CHECK-SAME:                             getelementptr inbounds
; CHECK-SAME:                                      (%__DFDT_struct.dep,
; CHECK-SAME:                                       %__DFDT_struct.dep* @g_dep,
; CHECK-SAME:                                       i64 0, i32 1)
; CHECK: %valA = load i32, i32* getelementptr inbounds (%__DFT_struct.del,
; CHECK-SAME:                             %__DFT_struct.del* @g_del,
; CHECK-SAME:                             i64 0, i32 0)
; CHECK: %valC = load i32, i32* getelementptr inbounds (%__DFT_struct.del,
; CHECK-SAME:                             %__DFT_struct.del* @g_del,
; CHECK-SAME:                             i64 0, i32 1)
; CHECK-NOT: store i64 3, i64*
