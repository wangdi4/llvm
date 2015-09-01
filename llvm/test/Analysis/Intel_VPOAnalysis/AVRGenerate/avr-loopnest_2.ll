; RUN: opt < %s -simd-function-cloning -avr-generate -analyze | FileCheck %s

; Check sequence AVRs generated for given test.
;CHECK: '_ZGVxM4vv_vec_sum'
;CHECK: AVR_WRN
;CHECK-NEXT: AVR_LABEL:    simd.begin.region
;CHECK-NEXT: AVR_CALL:     call void @llvm.intel.directive(metadata !7)
;CHECK-NEXT: AVR_CALL:     call void @llvm.intel.directive.qual.opnd.i32(metadata !8, i32 4)
;CHECK-NEXT: AVR_CALL:     call void @llvm.intel.directive.qual(metadata !9)
;CHECK-NEXT: AVR_FBRANCH:  br label %simd.loop
;CHECK-NEXT: AVR_LOOP:
;CHECK-NEXT: AVR_LABEL:    simd.loop
;CHECK-NEXT: AVR_PHI:      %index = phi i32 [ 0, %simd.begin.region ], [ %indvar, %simd.loop.exit ]
;CHECK-NEXT: AVR_ASSIGN:   %maskgep = getelementptr i32, i32* %veccast.3, i32 %index
;CHECK-NEXT: AVR_ASSIGN:   %mask5 = load i32, i32* %maskgep
;CHECK-NEXT: AVR_IF:       %maskcond = icmp {{eq i32 %mask5, 1|ne i32 %mask5, 0}}
;CHECK-NEXT: AVR_FBRANCH:  br i1 %maskcond, label %simd.loop.then, label %simd.loop.else
;CHECK-NEXT: AVR_LABEL:    simd.loop.then
;CHECK-NEXT: AVR_ASSIGN:   [[VECGEP1:%vecgep[0-9]*]] = getelementptr i32, i32* %veccast.{{[0-9]+}}, i32 %index
;CHECK-NEXT: AVR_ASSIGN:   %0 = load i32, i32* [[VECGEP1]], align 4
;CHECK-NEXT: AVR_ASSIGN:   [[VECGEP2:%vecgep[0-9]*]] = getelementptr i32, i32* %veccast.{{[0-9]+}}, i32 %index
;CHECK-NEXT: AVR_ASSIGN:   %1 = load i32, i32* [[VECGEP2]], align 4
;CHECK-NEXT: AVR_ASSIGN:   %add = add nsw i32 %0, %1
;CHECK-NEXT: AVR_ASSIGN:   %vec_gep = getelementptr i32, i32* %veccast, i32 %index
;CHECK-NEXT: AVR_ASSIGN:   store i32 %add, i32* %vec_gep
;CHECK-NEXT: AVR_FBRANCH:  br label %simd.loop.exit
;CHECK-NEXT: AVR_LABEL:    simd.loop.exit
;CHECK-NEXT: AVR_ASSIGN:   %indvar = add nuw i32 1, %index
;CHECK-NEXT: AVR_IF:       %vlcond = icmp ult i32 %indvar, 4
;CHECK-NEXT: AVR_FBRANCH:  br i1 %vlcond, label %simd.loop, label %simd.end.region
;CHECK-NEXT: AVR_LABEL:    simd.end.region
;CHECK-NEXT: AVR_CALL:     call void @llvm.intel.directive(metadata !10)
;CHECK-NEXT: AVR_FBRANCH:  br label %return
;CHECK-NEXT: AVR_LABEL:    return
;CHECK-NEXT: AVR_ASSIGN:   %cast = bitcast i32* %veccast to <4 x i32>*
;CHECK-NEXT: AVR_ASSIGN:   %vec_ret = load <4 x i32>, <4 x i32>* %cast
;CHECK-NEXT: AVR_RETURN:   ret <4 x i32> %vec_ret
;CHECK-NEXT: AVR_LABEL:    simd.loop.else
;CHECK-NEXT: AVR_FBRANCH:  br label %simd.loop.exit
;CHECK: '_ZGVxN4vv_vec_sum'
;CHECK: AVR_WRN
;CHECK-NEXT: AVR_LABEL:    simd.begin.region
;CHECK-NEXT: AVR_CALL:     call void @llvm.intel.directive(metadata !7)
;CHECK-NEXT: AVR_CALL:     call void @llvm.intel.directive.qual.opnd.i32(metadata !8, i32 4)
;CHECK-NEXT: AVR_CALL:     call void @llvm.intel.directive.qual(metadata !9)
;CHECK-NEXT: AVR_FBRANCH:  br label %simd.loop
;CHECK-NEXT: AVR_LOOP:
;CHECK-NEXT: AVR_LABEL:    simd.loop
;CHECK-NEXT: AVR_PHI:      %index = phi i32 [ 0, %simd.begin.region ], [ %indvar, %simd.loop.exit ]
;CHECK-NEXT: AVR_ASSIGN:   [[VECGEP3:%vecgep[0-9]*]] = getelementptr i32, i32* %veccast.{{[0-9]+}}, i32 %index
;CHECK-NEXT: AVR_ASSIGN:   %0 = load i32, i32* [[VECGEP3]], align 4
;CHECK-NEXT: AVR_ASSIGN:   [[VECGEP4:%vecgep[0-9]*]] = getelementptr i32, i32* %veccast.{{[0-9]+}}, i32 %index
;CHECK-NEXT: AVR_ASSIGN:   %1 = load i32, i32* [[VECGEP4]], align 4
;CHECK-NEXT: AVR_ASSIGN:   %add = add nsw i32 %0, %1
;CHECK-NEXT: AVR_ASSIGN:   [[VECGEP5:%.*]] = getelementptr i32, i32* %veccast, i32 %index
;CHECK-NEXT: AVR_ASSIGN:   store i32 %add, i32* [[VECGEP5]]
;CHECK-NEXT: AVR_FBRANCH:  br label %simd.loop.exit
;CHECK-NEXT: AVR_LABEL:    simd.loop.exit
;CHECK-NEXT: AVR_ASSIGN:   %indvar = add nuw i32 1, %index
;CHECK-NEXT: AVR_IF:       %vlcond = icmp ult i32 %indvar, 4
;CHECK-NEXT: AVR_FBRANCH:  br i1 %vlcond, label %simd.loop, label %simd.end.region
;CHECK-NEXT: AVR_LABEL:    simd.end.region
;CHECK-NEXT: AVR_CALL:     call void @llvm.intel.directive(metadata !10)
;CHECK-NEXT: AVR_FBRANCH:  br label %return
;CHECK-NEXT: AVR_LABEL:    return
;CHECK-NEXT: AVR_ASSIGN:   %cast = bitcast i32* %veccast to <4 x i32>*
;CHECK-NEXT: AVR_ASSIGN:   %vec_ret = load <4 x i32>, <4 x i32>* %cast
;CHECK-NEXT: AVR_RETURN:   ret <4 x i32> %vec_ret

; ModuleID = 'krtest.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define i32 @vec_sum(i32 %a, i32 %b) #0 {
entry:
  %a.addr = alloca i32, align 4
  %b.addr = alloca i32, align 4
  store i32 %a, i32* %a.addr, align 4
  store i32 %b, i32* %b.addr, align 4
  %0 = load i32, i32* %a.addr, align 4
  %1 = load i32, i32* %b.addr, align 4
  %add = add nsw i32 %0, %1
  ret i32 %add
}

attributes #0 = { nounwind uwtable "_ZGVxM4vv_" "_ZGVxN4vv_" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }

!cilk.functions = !{!0}
!llvm.ident = !{!6}

!0 = !{i32 (i32, i32)* @vec_sum, !1, !2, !3, !4, !5}
!1 = !{!"elemental"}
!2 = !{!"arg_name", !"a", !"b"}
!3 = !{!"arg_step", i32 undef, i32 undef}
!4 = !{!"arg_alig", i32 undef, i32 undef}
!5 = !{!"vec_length", i32 undef, i32 4}
!6 = !{!"clang version 3.7.0 (branches/vpo 1169)"}




