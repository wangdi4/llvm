; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s
;

declare void @llvm.x86.mic.mask.packstorel.ps(i8*, i16, <16 x float>, i32, i32) nounwind

declare void @llvm.x86.mic.mask.packstoreh.ps(i8*, i16, <16 x float>, i32, i32) nounwind

define i64 @test(i16 addrspace(1)* %a1, i64 %num.vector.wi.i, i64 %a15, i64 %a17, i1 %a18, <16 x i16> %a22, <16 x float> %vector, float addrspace(1)* %array) {
; CHECK: test
; CHECK: subq      $768, %rsp
; CHECK: ..B1.2
entry:
  %a5 = alloca [768 x i8], align 128
  %a12 = bitcast [768 x i8]* %a5 to [384 x i16] addrspace(3)*

  %max.vector.gid.i = add i64 %a15, %a17

  br i1 %a18, label %__loop_body, label %__edge_case

__edge_case:                               
  %a23 = getelementptr [384 x i16] addrspace(3)* %a12, i64 0, i64 0
  %ptrTypeCast16vector_func.i = bitcast i16 addrspace(3)* %a23 to <16 x i16> addrspace(3)*
  store <16 x i16> %a22, <16 x i16> addrspace(3)* %ptrTypeCast16vector_func.i, align 2
  br label %__exit

__loop_body:                            
  %dim_0_ind_var.i = phi i64 [ %dim_0_inc_ind_var.i, %__inc_loop ], [ 0, %entry ]
  %dim_0_tid.i = phi i64 [ %dim_0_inc_tid.i, %__inc_loop ], [ %max.vector.gid.i, %entry ]

  %arrayidx.i = getelementptr inbounds i16 addrspace(1)* %a1, i64 %dim_0_tid.i
  %a71 = load i16 addrspace(1)* %arrayidx.i, align 2

  %arrayidx1.i = getelementptr inbounds [384 x i16] addrspace(3)* %a12, i64 0, i64 %dim_0_tid.i
  store i16 6, i16 addrspace(3)* %arrayidx1.i, align 2

  %array_ptr = getelementptr float addrspace(1)* %array, i64 %dim_0_tid.i
  %ptr_packstore_l = bitcast float addrspace(1)* %array_ptr to i8*
  call void @llvm.x86.mic.mask.packstorel.ps(i8* %ptr_packstore_l, i16 7, <16 x float> %vector, i32 0, i32 0) nounwind

  %ptr_packstore_h = getelementptr i8* %ptr_packstore_l, i64 64
  call void @llvm.x86.mic.mask.packstoreh.ps(i8* %ptr_packstore_h, i16 7, <16 x float> %vector, i32 0, i32 0) nounwind
  br label %__inc_loop

__inc_loop:
  %dim_0_inc_ind_var.i = add i64 %dim_0_ind_var.i, 1
  %dim_0_inc_tid.i = add i64 %dim_0_tid.i, 1

  %dim_0_cmp.to.max.i = icmp eq i64 %dim_0_inc_ind_var.i, %a17
  br i1 %dim_0_cmp.to.max.i, label %__exit, label %__loop_body

__exit:                       
  ret i64 0
}


