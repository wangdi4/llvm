; We detect functions that contain special keywords in their names.
; If there is a match we assume they are error handling functions,
; and thus add 'cold' attribute to them.

; RUN: opt < %s -S -inferattrs | FileCheck %s
; RUN: opt < %s -S -passes=inferattrs | FileCheck %s

; CHECK: define void @my_croak_func() #0 {
define void @my_croak_func() {
  ret void
}

; CHECK: define void @my_warn_func() #0 {
define void @my_warn_func() {
  ret void
}

; CHECK: declare void @my_croak_func1() #0
declare void @my_croak_func1()

; CHECK: define void @my_croak_optnone() #1 {
define void @my_croak_optnone() noinline optnone {
  ret void
}

; CHECK: define void @my_croa_k_func() {
define void @my_croa_k_func() {
  ret void
}

; CHECK: attributes #0 = { cold }
; CHECK: attributes #1 = { noinline optnone }
