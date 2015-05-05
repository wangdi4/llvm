; RUN: opt -presucf=false -predicate -verify %s -S -o %t0.ll
; RUN: opt -presucf=true -predicate -verify %s -S -o %t1.ll
; RUN: FileCheck %s -input-file=%t0.ll -check-prefix=CHECK-UCFOFF
; RUN: FileCheck %s -input-file=%t1.ll -check-prefix=CHECK-UCFON

declare i32 @_Z13get_global_idj(i32) #1

; Test if-then uniform control flow is preserved
;
; CHECK-UCFOFF:      @ucf_if_then_test
; CHECK-UCFOFF-NOT:  br i1 %ucf_cond, label %then, label %ucf_exit
; CHECK-UCFOFF:      ret void
;
; CHECK-UCFON:       @ucf_if_then_test
; CHECK-UCFON:       br i1 %ucf_cond, label %then, label %ucf_exit
; CHECK-UCFON:       ret void
define void @ucf_if_then_test(float addrspace(1)* %inout, i32 %arg) {
entry:
  %gid = call i32 @_Z13get_global_idj(i32 0)
  %dcf_cond = icmp sgt i32 %gid, 16
  br i1 %dcf_cond, label %ucf_entry, label %return

ucf_entry:
  %gep = getelementptr float addrspace(1)* %inout, i32 %gid
  %ucf_cond = icmp sgt i32 %arg, 0
  br i1 %ucf_cond, label %then, label %ucf_exit

then:
  store float 0.000000e+00, float addrspace(1)* %gep, align 1
  br label %ucf_exit

ucf_exit:
  br label %return

return:
  ret void
}

; Test if-then-else uniform control flow is preserved
;
; CHECK-UCFOFF:      @ucf_if_then_else_test
; CHECK-UCFOFF-NOT:  br i1 %ucf_cond, label %then, label %else
; CHECK-UCFOFF:      br label %ucf_exit
; CHECK-UCFOFF-NOT:  br label %ucf_exit
; CHECK-UCFOFF:      ret void
;
; CHECK-UCFON:       @ucf_if_then_else_test
; CHECK-UCFON:       br i1 %ucf_cond, label %then, label %else
; CHECK-UCFON:       br label %ucf_exit
; CHECK-UCFON:       br label %ucf_exit
; CHECK-UCFON:       ret void
define void @ucf_if_then_else_test(float addrspace(1)* %inout, i32 %arg) {
entry:
  %gid = call i32 @_Z13get_global_idj(i32 0)
  %dcf_cond = icmp sgt i32 %gid, 16
  br i1 %dcf_cond, label %ucf_entry, label %return

ucf_entry:
  %gep = getelementptr float addrspace(1)* %inout, i32 %gid
  %ucf_cond = icmp sgt i32 %arg, 0
  br i1 %ucf_cond, label %then, label %else

then:
  store float 0.000000e+00, float addrspace(1)* %gep, align 1
  br label %ucf_exit

else:
  store float 1.000000e+00, float addrspace(1)* %gep, align 1
  br label %ucf_exit

ucf_exit:
  br label %return

return:
  ret void
}

; Test loop uniform control flow is preserved
;
; CHECK-UCFOFF:      @ucf_loop_test
; CHECK-UCFOFF-NOT:  br i1 %exit_cond0, label %ucf_loop_exit, label %ucf_loop_second_bb
; CHECK-UCFOFF-NOT:  br i1 %exit_cond1, label %ucf_loop_exit, label %ucf_loop_header
; CHECK-UCFOFF:      br i1 %shouldexit{{.*}}, label %ucf_loop_exit
; CHECK-UCFOFF:      ret void
;
; CHECK-UCFON:       @ucf_loop_test
; CHECK-UCFON:       br i1 %exit_cond0, label %ucf_loop_exit, label %ucf_loop_second_bb
; CHECK-UCFON:       br i1 %exit_cond1, label %ucf_loop_exit, label %ucf_loop_header
; CHECK-UCFON:       ret void
define void @ucf_loop_test(float addrspace(1)* %inout, i32 %arg) {
entry:
  %gid = call i32 @_Z13get_global_idj(i32 0)
  %dcf_cond = icmp sgt i32 %gid, 16
  br i1 %dcf_cond, label %ucf_loop_preheader, label %return

ucf_loop_preheader:
  %gep = getelementptr float addrspace(1)* %inout, i32 %gid
  br label %ucf_loop_header

ucf_loop_header:
  %i = phi i32 [ %arg, %ucf_loop_preheader ], [ %i.next1, %ucf_loop_second_bb ]
  store float 1.000000e+00, float addrspace(1)* %gep, align 1
  %i.next0 = add i32 %i, 1
  %exit_cond0 = icmp sgt i32 %i, 1024
  br i1 %exit_cond0, label %ucf_loop_exit, label %ucf_loop_second_bb

ucf_loop_second_bb:
  store float 2.000000e+00, float addrspace(1)* %gep, align 1
  %i.next1 = add i32 %i.next0, 1
  %exit_cond1 = icmp sgt i32 %i, 1024
  br i1 %exit_cond1, label %ucf_loop_exit, label %ucf_loop_header

ucf_loop_exit:
  br label %return

return:
  ret void
}


; Test uniform control flow inside DCF loop is preserved
;
; CHECK-UCFOFF:      @ucf_inside_dcf_loop_test
; CHECK-UCFOFF-NOT:  br i1 %ucf_if_then_cond, label %then, label %else
; CHECK-UCFOFF:      br label %ucf_exit
; CHECK-UCFOFF-NOT:  br label %ucf_exit
; CHECK-UCFOFF:      ret void
;
; CHECK-UCFON:       @ucf_inside_dcf_loop_test
; CHECK-UCFON:       br i1 %ucf_if_then_cond, label %then, label %else
; CHECK-UCFON:       br label %ucf_exit
; CHECK-UCFON:       br label %ucf_exit
; CHECK-UCFON:       ret void
define void @ucf_inside_dcf_loop_test(float addrspace(1)* %inout, i32 %arg) {
dcf_loop_preheader:
  %gid = call i32 @_Z13get_global_idj(i32 0)
  %gep = getelementptr float addrspace(1)* %inout, i32 %gid
  br label %dcf_loop_header

dcf_loop_header:
  %i = phi i32 [ %gid, %dcf_loop_preheader ], [ %i.next, %ucf_exit ]
  %ucf_if_then_cond = icmp sgt i32 %arg, 0
  br i1 %ucf_if_then_cond, label %then, label %else

then:
  store float 1.000000e+00, float addrspace(1)* %gep, align 1
  br label %ucf_exit

else:
  store float 2.000000e+00, float addrspace(1)* %gep, align 1
  br label %ucf_exit

ucf_exit:
  %i.next = add i32 %i, 1
  %exit_cond = icmp sgt i32 %i.next, 1024
  br i1 %exit_cond, label %dcf_loop_exit, label %dcf_loop_header

dcf_loop_exit:
  ret void
}
