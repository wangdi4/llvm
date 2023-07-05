; RUN: opt -passes=auto-cpu-clone -generate-vector-variants < %s -S | FileCheck %s


; CHECK: @__intel_cpu_feature_indicator = external global [2 x i64]
; CHECK: @_ZGVZN16vv_func = dso_local ifunc <16 x i32> (<16 x i32>, <16 x ptr>), ptr @_ZGVZN16vv_func.resolver

; CHECK:      define dso_local ptr @_ZGVZN16vv_func.resolver() #4 {
; CHECK-NEXT: resolver_entry:
; CHECK-NEXT:   call void @__intel_cpu_features_init()
; CHECK-NEXT:   %cpu_feature_indicator = load i64, ptr @__intel_cpu_feature_indicator, align 8
; CHECK-NEXT:   %cpu_feature_join = and i64 %cpu_feature_indicator, 4816840611962862
; CHECK-NEXT:   %cpu_feature_check = icmp eq i64 %cpu_feature_join, 4816840611962862
; CHECK-NEXT:   br i1 %cpu_feature_check, label %resolver_return, label %resolver_else
; CHECK-EMPTY:
; CHECK-NEXT: resolver_return:                                  ; preds = %resolver_entry
; CHECK-NEXT:   ret ptr @_ZGVZN16vv_func.l
; CHECK-EMPTY:
; CHECK-NEXT: resolver_else:                                    ; preds = %resolver_entry
; CHECK-NEXT:   %cpu_feature_indicator1 = load i64, ptr @__intel_cpu_feature_indicator, align 8
; CHECK-NEXT:   %cpu_feature_join2 = and i64 %cpu_feature_indicator1, 429926490094
; CHECK-NEXT:   %cpu_feature_check3 = icmp eq i64 %cpu_feature_join2, 429926490094
; CHECK-NEXT:   br i1 %cpu_feature_check3, label %resolver_return4, label %resolver_else5
; CHECK-EMPTY:
; CHECK-NEXT: resolver_return4:                                 ; preds = %resolver_else
; CHECK-NEXT:   ret ptr @_ZGVZN16vv_func.a
; CHECK-EMPTY:
; CHECK-NEXT: resolver_else5:                                   ; preds = %resolver_else
; CHECK-NEXT:   ret ptr null
; CHECK-NEXT: }


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress nofree nosync nounwind willreturn memory(readwrite) uwtable

define dso_local x86_regcallcc <16 x i32> @_ZGVZN16vv_func(<16 x i32> noundef %c, <16 x ptr> noundef %x) local_unnamed_addr #0 !llvm.vec.auto.cpu.dispatch !0 {
entry:
  %vec.c = alloca <16 x i32>, align 64
  %vec.x = alloca <16 x ptr>, align 128
  %vec.retval = alloca <16 x i32>, align 64
  store <16 x i32> %c, ptr %vec.c, align 64
  store <16 x ptr> %x, ptr %vec.x, align 128
  br label %simd.begin.region

simd.begin.region:                                ; preds = %entry
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 16) ]
  br label %simd.loop.preheader

simd.loop.preheader:                              ; preds = %simd.begin.region
  br label %simd.loop.header

simd.loop.header:                                 ; preds = %simd.loop.latch, %simd.loop.preheader
  %index = phi i32 [ 0, %simd.loop.preheader ], [ %indvar, %simd.loop.latch ]
  %vec.x.gep = getelementptr ptr, ptr %vec.x, i32 %index
  %vec.x.elem = load ptr, ptr %vec.x.gep, align 8
  %0 = load i32, ptr %vec.x.elem, align 4
  %vec.c.gep = getelementptr i32, ptr %vec.c, i32 %index
  %vec.c.elem = load i32, ptr %vec.c.gep, align 4
  %add = add nsw i32 %0, %vec.c.elem
  %vec.retval.gep = getelementptr i32, ptr %vec.retval, i32 %index
  store i32 %add, ptr %vec.retval.gep, align 4
  br label %simd.loop.latch

simd.loop.latch:                                  ; preds = %simd.loop.header
  %indvar = add nuw i32 %index, 1
  %vl.cond = icmp ult i32 %indvar, 16
  br i1 %vl.cond, label %simd.loop.header, label %simd.end.region

simd.end.region:                                  ; preds = %simd.loop.latch
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  br label %return

return:                                           ; preds = %simd.end.region
  %vec.ret = load <16 x i32>, ptr %vec.retval, align 64
  ret <16 x i32> %vec.ret
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { mustprogress nofree nosync nounwind willreturn memory(readwrite) uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }

!0 = !{!"skylake-avx512", !"tigerlake"}
