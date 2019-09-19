; Verify call with no side effect can be serialized as serialized once per all lanes.
; RUN: opt -S -VPlanDriver -vplan-force-vf=8 < %s | FileCheck %s
; RUN: opt -S -VPlanDriver -enable-vp-value-codegen=true -vplan-force-vf=8 < %s | FileCheck %s

; Function Attrs: nounwind readonly
declare i32 @foo_i32(i32 addrspace(1)*, i32) local_unnamed_addr readonly #1

; Function Attrs: nounwind
define void @testfun_i32(i32 addrspace(1)* noalias %_arg_) local_unnamed_addr {
entry:
  %alloca._arg_ = alloca i32 addrspace(1)*
  store i32 addrspace(1)* %_arg_, i32 addrspace(1)** %alloca._arg_
  br label %simd.begin.region

simd.begin.region:                                ; preds = %entry
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 8), "QUAL.OMP.UNIFORM"(i32 addrspace(1)** %alloca._arg_) ]
  br label %simd.loop.preheader

simd.loop.preheader:                              ; preds = %simd.begin.region
  %load._arg_ = load i32 addrspace(1)*, i32 addrspace(1)** %alloca._arg_
  br label %simd.loop

; CHECK:       vector.body:
; CHECK:       {{.*}} = call i32 @foo_i32({{.*}})
; CHECK-NOT:   {{.*}} = call i32 @foo_i32({{.*}})
; CHECK:       VPlannedBB:

simd.loop:                                        ; preds = %simd.loop.exit, %simd.loop.preheader
  %index = phi i32 [ 0, %simd.loop.preheader ], [ %indvar, %simd.loop.exit ]
  %call2.i.i.i.i.i = call i32 @foo_i32(i32 addrspace(1)* %load._arg_, i32 1) #1
  br label %simd.loop.exit

simd.loop.exit:                                   ; preds = %simd.loop
  %indvar = add nuw i32 %index, 1
  %vl.cond = icmp ult i32 %indvar, 8
  br i1 %vl.cond, label %simd.loop, label %simd.end.region

simd.end.region:                                  ; preds = %simd.loop.exit
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  br label %return

return:                                           ; preds = %simd.end.region
  ret void
}

; same as before but function's return type is void.
; Function Attrs: nounwind readonly
declare void @foo_void(i32 addrspace(1)*, i32) local_unnamed_addr readonly #1

; Function Attrs: nounwind
define void @testfun_void(i32 addrspace(1)* noalias %_arg_) local_unnamed_addr {
entry:
  %alloca._arg_ = alloca i32 addrspace(1)*
  store i32 addrspace(1)* %_arg_, i32 addrspace(1)** %alloca._arg_
  br label %simd.begin.region

simd.begin.region:                                ; preds = %entry
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 8), "QUAL.OMP.UNIFORM"(i32 addrspace(1)** %alloca._arg_) ]
  br label %simd.loop.preheader

simd.loop.preheader:                              ; preds = %simd.begin.region
  %load._arg_ = load i32 addrspace(1)*, i32 addrspace(1)** %alloca._arg_
  br label %simd.loop

; CHECK:       vector.body:
; CHECK:       call void @foo_void({{.*}})
; CHECK-NOT:   call void @foo_void({{.*}})
; CHECK:       VPlannedBB:

simd.loop:                                        ; preds = %simd.loop.exit, %simd.loop.preheader
  %index = phi i32 [ 0, %simd.loop.preheader ], [ %indvar, %simd.loop.exit ]
  call void @foo_void(i32 addrspace(1)* %load._arg_, i32 1) #1
  br label %simd.loop.exit

simd.loop.exit:                                   ; preds = %simd.loop
  %indvar = add nuw i32 %index, 1
  %vl.cond = icmp ult i32 %indvar, 8
  br i1 %vl.cond, label %simd.loop, label %simd.end.region

simd.end.region:                                  ; preds = %simd.loop.exit
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  br label %return

return:                                           ; preds = %simd.end.region
  ret void
}


; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #1 = { nounwind }

