; RUN: opt < %s -vec-clone -vpo-cfg-restructuring -avr-generate -analyze | FileCheck %s

;
; Check the correctness of generated Abstract Layer for switch
;

;CHECK: Printing analysis 'AVR Generate' for function '_ZGVbM4vvvv_switcher':
;CHECK-NEXT: WRN

;CHECK: simd.begin.region:
;CHECK-NEXT: call void @llvm.intel.directive
;CHECK-NEXT: call void @llvm.intel.directive.qual.opnd.i32
;CHECK-NEXT: call void @llvm.intel.directive
;CHECK-NEXT: br label %simd.loop

;CHECK-NEXT: LOOP( IV )
;CHECK: simd.loop:
;CHECK-NEXT: %index = phi [0, simd.begin.region], [%indvar, simd.loop.exit]
;TEMP-DO-NOT-CHECK: br i1 %mask.cond, label %simd.loop.then, label %simd.loop.else

;CHECK: simd.loop.then:
;CHECK-NEXT: %vec.n.cast.gep = %vec.n.cast getelementptr %index
;CHECK-NEXT: %0 = load %vec.n.cast.gep
;CHECK-NEXT: switch(%0){

;CHECK-NEXT: case 1:
;CHECK: sw.bb:
;CHECK: %vec.a.cast.gep{{[0-9]*}} = %vec.a.cast getelementptr %index
;CHECK: %ret.cast.gep{{[0-9]*}} = store %add
;CHECK: br label %return
;CHECK: break;

;CHECK: case 2:
;CHECK: sw.bb.1:
;CHECK: %vec.a.cast.gep{{[0-9]*}} = %vec.a.cast getelementptr %index
;CHECK: %ret.cast.gep{{[0-9]*}} = store %sub
;CHECK: br label %return
;CHECK: break;

;CHECK: case 3:
;CHECK: sw.bb.2:
;CHECK: %vec.a.cast.gep{{[0-9]*}} = %vec.a.cast getelementptr %index
;CHECK: %ret.cast.gep{{[0-9]*}} = store %sub4
;CHECK: br label %return
;CHECK: break;

;CHECK: case 4:
;CHECK: sw.bb.5:
;CHECK: %vec.a.cast.gep{{[0-9]*}} = %vec.a.cast getelementptr %index
;CHECK: %8 = load %vec.a.cast.gep{{[0-9]*}}
;CHECK: %ret.cast.gep{{[0-9]*}} = store %add7
;CHECK: br label %return
;CHECK: break;

;CHECK: case 5:
;CHECK: sw.bb.8:
;CHECK: %vec.a.cast.gep{{[0-9]*}} = %vec.a.cast getelementptr %index
;CHECK: %ret.cast.gep{{[0-9]*}} = store %sub9
;CHECK: br label %return
;CHECK: break;

;CHECK: default:
;CHECK: sw.default:
;CHECK: %vec.a.cast.gep{{[0-9]*}} = %vec.a.cast getelementptr %index
;CHECK: %ret.cast.gep{{[0-9]+}} = store %mul
;CHECK: br label %return

;CHECK: simd.loop.else:
;CHECK: br label %simd.loop.exit

;CHECK: simd.loop.exit:
;CHECK: %indvar = %index add 1
;CHECK: br i1 %vl.cond, label %simd.loop, label %simd.end.region, !llvm.loop

;CHECK: simd.end.region:
;CHECK: call void @llvm.intel.directive
;CHECK: call void @llvm.intel.directive

;CHECK: br label %return

;CHECK: return:
;CHECK: %vec.ret.cast = bitcast %ret.cast
;CHECK: %vec.ret = load %vec.ret.cast
;CHECK: ret <4 x i32> %vec.ret

;ModuleID = 'switch.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define i32 @switcher(i32 %n, i32 %a, i32 %b, i32 %c) #0 {
entry:
  %retval = alloca i32, align 4
  %n.addr = alloca i32, align 4
  %a.addr = alloca i32, align 4
  %b.addr = alloca i32, align 4
  %c.addr = alloca i32, align 4
  store i32 %n, i32* %n.addr, align 4
  store i32 %a, i32* %a.addr, align 4
  store i32 %b, i32* %b.addr, align 4
  store i32 %c, i32* %c.addr, align 4
  %0 = load i32, i32* %n.addr, align 4
  switch i32 %0, label %sw.default [
    i32 0, label %sw.bb
    i32 1, label %sw.bb.1
    i32 2, label %sw.bb.2
    i32 3, label %sw.bb.5
    i32 4, label %sw.bb.8
  ]

sw.bb:                                            ; preds = %entry
  %1 = load i32, i32* %a.addr, align 4
  %2 = load i32, i32* %b.addr, align 4
  %add = add nsw i32 %1, %2
  store i32 %add, i32* %retval, align 4
  br label %return

sw.bb.1:                                          ; preds = %entry
  %3 = load i32, i32* %a.addr, align 4
  %4 = load i32, i32* %b.addr, align 4
  %sub = sub nsw i32 %3, %4
  store i32 %sub, i32* %retval, align 4
  br label %return

sw.bb.2:                                          ; preds = %entry
  %5 = load i32, i32* %a.addr, align 4
  %6 = load i32, i32* %b.addr, align 4
  %add3 = add nsw i32 %5, %6
  %7 = load i32, i32* %c.addr, align 4
  %sub4 = sub nsw i32 %add3, %7
  store i32 %sub4, i32* %retval, align 4
  br label %return

sw.bb.5:                                          ; preds = %entry
  %8 = load i32, i32* %a.addr, align 4
  %9 = load i32, i32* %b.addr, align 4
  %add6 = add nsw i32 %8, %9
  %10 = load i32, i32* %c.addr, align 4
  %add7 = add nsw i32 %add6, %10
  store i32 %add7, i32* %retval, align 4
  br label %return

sw.bb.8:                                          ; preds = %entry
  %11 = load i32, i32* %a.addr, align 4
  %12 = load i32, i32* %c.addr, align 4
  %sub9 = sub nsw i32 %11, %12
  store i32 %sub9, i32* %retval, align 4
  br label %return

sw.default:                                       ; preds = %entry
  %13 = load i32, i32* %a.addr, align 4
  %14 = load i32, i32* %b.addr, align 4
  %mul = mul nsw i32 %13, %14
  store i32 %mul, i32* %retval, align 4
  br label %return

return:                                           ; preds = %sw.default, %sw.bb.8, %sw.bb.5, %sw.bb.2, %sw.bb.1, %sw.bb
  %15 = load i32, i32* %retval, align 4
  ret i32 %15
}

attributes #0 = { nounwind uwtable "vector-variants"="_ZGVbM4vvvv_,_ZGVbN4vvvv_" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!cilk.functions = !{!0}
!llvm.ident = !{!6}

!0 = !{i32 (i32, i32, i32, i32)* @switcher, !1, !2, !3, !4, !5}
!1 = !{!"elemental"}
!2 = !{!"arg_name", !"n", !"a", !"b", !"c"}
!3 = !{!"arg_step", i32 undef, i32 undef, i32 undef, i32 undef}
!4 = !{!"arg_alig", i32 undef, i32 undef, i32 undef, i32 undef}
!5 = !{!"vec_length", i32 undef, i32 4}
!6 = !{!"clang version 3.8.0 (branches/vpo 2061)"}


; Source Code: switch.c
;_declspec(vector)
;int switcher(int n, int a, int b, int c)
;{
;  switch(n) {
;  case 0:
;    return a + b;
;    break;
;  case 1:
;    return a - b;
;    break;
;  case 2:
;    return a + b -c;
;    break;
;  case 3:
;    return a + b +c;
;    break;
;  case 4:
;    return a -c;
;    break;
;  default:
;   return a * b;
;  }
;}
