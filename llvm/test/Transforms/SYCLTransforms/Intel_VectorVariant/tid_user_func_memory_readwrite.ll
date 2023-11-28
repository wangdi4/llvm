; RUN: opt -passes=sycl-kernel-update-call-attrs -S %s | FileCheck %s
; RUN: opt -passes=sycl-kernel-update-call-attrs -S %s -enable-debugify -disable-output 2>&1 | FileCheck %s --check-prefix=DEBUGIFY

; Check that tid builtin's non-kernel user function's memory(none) attribute is
; changed to memory(readwrite).

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; CHECK: define internal fastcc i64 @bar() unnamed_addr [[ATTR0:#[0-9]+]]
; CHECK: define internal fastcc i64 @foo() unnamed_addr [[ATTR1:#[0-9]+]]
; CHECK: define dso_local void @basic({{.*}} [[ATTR2:#[0-9]+]]
; CHECK: [[ATTR0]] = {{.*}} memory(readwrite)
; CHECK: [[ATTR1]] = {{.*}} memory(readwrite)
; CHECK: [[ATTR2]] = {{.*}} memory(argmem: write)

; Function Attrs: convergent mustprogress nofree noinline norecurse nounwind willreturn memory(none)
define internal fastcc i64 @bar() unnamed_addr #0 {
entry:
  %call = tail call i64 @_Z12get_local_idj(i32 noundef 0) #7
  ret i64 %call
}

; Function Attrs: convergent mustprogress nofree nounwind willreturn memory(none)
declare i64 @_Z12get_local_idj(i32 noundef) local_unnamed_addr #1

; Function Attrs: convergent mustprogress nofree noinline norecurse nounwind willreturn memory(none)
define internal fastcc i64 @foo() unnamed_addr #2 {
entry:
  %call = tail call fastcc i64 @bar() #8
  ret i64 %call
}

; Function Attrs: convergent mustprogress nofree norecurse nounwind willreturn memory(argmem: write)
define dso_local void @basic(ptr addrspace(1) noalias nocapture noundef writeonly align 8 %local_id) local_unnamed_addr #3 !arg_type_null_val !2 !no_barrier_path !3 !kernel_has_sub_groups !4 !vectorized_kernel !5 !vectorized_width !6 {
entry:
  %call = tail call fastcc i64 @foo() #8
  %arrayidx = getelementptr inbounds i64, ptr addrspace(1) %local_id, i64 %call
  store i64 %call, ptr addrspace(1) %arrayidx, align 8
  ret void
}

declare i64 @_Z14get_local_sizej(i32)

declare i64 @get_base_global_id.(i32)

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #4

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #4

; Function Attrs: convergent mustprogress nofree noinline norecurse nounwind willreturn memory(readwrite)
define internal fastcc <16 x i64> @_ZGVeN16_bar() unnamed_addr #5 {
entry:
  %vec.retval = alloca <16 x i64>, align 128
  %call = tail call i64 @_Z12get_local_idj(i32 noundef 0) #7
  %0 = trunc i64 %call to i32
  br label %simd.begin.region

simd.begin.region:                                ; preds = %entry
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 16) ]
  br label %simd.loop.preheader

simd.loop.preheader:                              ; preds = %simd.begin.region
  br label %simd.loop.header

simd.loop.header:                                 ; preds = %simd.loop.latch, %simd.loop.preheader
  %index = phi i32 [ 0, %simd.loop.preheader ], [ %indvar, %simd.loop.latch ]
  %add = add nuw i32 %0, %index
  %1 = sext i32 %add to i64
  %vec.retval.gep = getelementptr i64, ptr %vec.retval, i32 %index
  store i64 %1, ptr %vec.retval.gep, align 8
  br label %simd.loop.latch

simd.loop.latch:                                  ; preds = %simd.loop.header
  %indvar = add nuw i32 %index, 1
  %vl.cond = icmp ult i32 %indvar, 16
  br i1 %vl.cond, label %simd.loop.header, label %simd.end.region

simd.end.region:                                  ; preds = %simd.loop.latch
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  br label %return

return:                                           ; preds = %simd.end.region
  %vec.ret = load <16 x i64>, ptr %vec.retval, align 128
  ret <16 x i64> %vec.ret
}

; Function Attrs: convergent mustprogress nofree noinline norecurse nounwind willreturn memory(readwrite)
define internal fastcc <16 x i64> @_ZGVeN16_foo() unnamed_addr #5 {
entry:
  %vec.retval = alloca <16 x i64>, align 128
  br label %simd.begin.region

simd.begin.region:                                ; preds = %entry
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 16) ]
  br label %simd.loop.preheader

simd.loop.preheader:                              ; preds = %simd.begin.region
  br label %simd.loop.header

simd.loop.header:                                 ; preds = %simd.loop.latch, %simd.loop.preheader
  %index = phi i32 [ 0, %simd.loop.preheader ], [ %indvar, %simd.loop.latch ]
  %call = tail call fastcc i64 @bar() #8
  %vec.retval.gep = getelementptr i64, ptr %vec.retval, i32 %index
  store i64 %call, ptr %vec.retval.gep, align 8
  br label %simd.loop.latch

simd.loop.latch:                                  ; preds = %simd.loop.header
  %indvar = add nuw i32 %index, 1
  %vl.cond = icmp ult i32 %indvar, 16
  br i1 %vl.cond, label %simd.loop.header, label %simd.end.region

simd.end.region:                                  ; preds = %simd.loop.latch
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  br label %return

return:                                           ; preds = %simd.end.region
  %vec.ret = load <16 x i64>, ptr %vec.retval, align 128
  ret <16 x i64> %vec.ret
}

; Function Attrs: convergent mustprogress nofree norecurse nounwind willreturn memory(readwrite)
define dso_local void @_ZGVeN16u_basic(ptr addrspace(1) noalias nocapture noundef writeonly align 8 %local_id) local_unnamed_addr #6 !arg_type_null_val !2 !no_barrier_path !3 !kernel_has_sub_groups !4 !vectorized_width !7 !scalar_kernel !1 {
entry:
  %alloca.local_id = alloca ptr addrspace(1), align 8
  store ptr addrspace(1) %local_id, ptr %alloca.local_id, align 8
  br label %simd.begin.region

simd.begin.region:                                ; preds = %entry
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 16), "QUAL.OMP.UNIFORM:TYPED"(ptr %alloca.local_id, ptr addrspace(1) null, i32 1) ]
  br label %simd.loop.preheader

simd.loop.preheader:                              ; preds = %simd.begin.region
  %load.local_id = load ptr addrspace(1), ptr %alloca.local_id, align 8
  br label %simd.loop.header

simd.loop.header:                                 ; preds = %simd.loop.latch, %simd.loop.preheader
  %index = phi i32 [ 0, %simd.loop.preheader ], [ %indvar, %simd.loop.latch ]
  %call = tail call fastcc i64 @foo() #8
  %arrayidx = getelementptr inbounds i64, ptr addrspace(1) %load.local_id, i64 %call
  store i64 %call, ptr addrspace(1) %arrayidx, align 8
  br label %simd.loop.latch

simd.loop.latch:                                  ; preds = %simd.loop.header
  %indvar = add nuw i32 %index, 1
  %vl.cond = icmp ult i32 %indvar, 16
  br i1 %vl.cond, label %simd.loop.header, label %simd.end.region

simd.end.region:                                  ; preds = %simd.loop.latch
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  br label %return

return:                                           ; preds = %simd.end.region
  ret void
}

attributes #0 = { convergent mustprogress nofree noinline norecurse nounwind willreturn memory(none) "vector-variants"="_ZGVeN16_bar" }
attributes #1 = { convergent mustprogress nofree nounwind willreturn memory(none) }
attributes #2 = { convergent mustprogress nofree noinline norecurse nounwind willreturn memory(none) "vector-variants"="_ZGVeN16_foo" }
attributes #3 = { convergent mustprogress nofree norecurse nounwind willreturn memory(argmem: write) "vector-variants"="_ZGVeN16u_basic" }
attributes #4 = { nounwind }
attributes #5 = { convergent mustprogress nofree noinline norecurse nounwind willreturn memory(readwrite) "may-have-openmp-directive"="true" }
attributes #6 = { convergent mustprogress nofree norecurse nounwind willreturn memory(readwrite) "may-have-openmp-directive"="true" }
attributes #7 = { convergent nounwind willreturn memory(none) }
attributes #8 = { convergent nounwind }

!opencl.ocl.version = !{!0}
!sycl.kernels = !{!1}

!0 = !{i32 1, i32 2}
!1 = !{ptr @basic}
!2 = !{ptr addrspace(1) null}
!3 = !{i1 true}
!4 = !{i1 false}
!5 = !{ptr @_ZGVeN16u_basic}
!6 = !{i32 1}
!7 = !{i32 16}

; DEBUGIFY-NOT: WARNING
