; RUN: opt < %s -passes=vplan-vec -vplan-force-vf=2 -S 2>&1 | FileCheck %s
; RUN: opt -passes='hir-ssa-deconstruction,hir-vplan-vec,print<hir>' -vplan-force-vf=2 -disable-output < %s 2>&1 | FileCheck %s --check-prefixes=HIR-CG

define void @foo(ptr %p, i1 %uniform) #0 {
; CHECK-LABEL: @foo(
; CHECK:         [[TMP5:%.*]] = bitcast <2 x i1> [[TMP1:%.*]] to i2
; CHECK-NEXT:    [[CTTZ:%.*]] = call i2 @llvm.cttz.i2(i2 [[TMP5]], i1 false)
; CHECK-NEXT:    [[TMP6:%.*]] = zext i2 [[CTTZ]] to i32
; CHECK-NEXT:    [[TMP7:%.*]] = mul i32 [[TMP6]], 2
; CHECK-NEXT:    [[TMP8:%.*]] = add i32 [[TMP7]], 0
; CHECK-NEXT:    [[TMP9:%.*]] = extractelement <4 x i64> [[PREDBLEND:%.*]], i32 [[TMP8]]
; CHECK-NEXT:    [[TMP10:%.*]] = insertelement <2 x i64> undef, i64 [[TMP9]], i64 0
; CHECK-NEXT:    [[TMP11:%.*]] = add i32 [[TMP7]], 1
; CHECK-NEXT:    [[TMP12:%.*]] = extractelement <4 x i64> [[PREDBLEND]], i32 [[TMP11]]
; CHECK-NEXT:    [[TMP13:%.*]] = insertelement <2 x i64> [[TMP10]], i64 [[TMP12]], i64 1
; CHECK-NEXT:    [[TMP14:%.*]] = add <2 x i64> [[TMP13]], <i64 1, i64 1>
;
; HIR-CG-LABEL: Function: foo
; HIR-CG:         [[BSFINTMASK:%.*]] = bitcast.<2 x i1>.i2([[MASK:%.*]]);
; HIR-CG-NEXT:    [[BSF:%.*]] = @llvm.cttz.i2([[BSFINTMASK]],  0);
; HIR-CG-NEXT:    [[ZEXT:%.*]] = zext.i2.i32([[BSF]]);
; HIR-CG-NEXT:    [[RESULT:%.*]] = undef;
; HIR-CG-NEXT:    [[MUL:%.*]] = [[ZEXT]]  *  2;
; HIR-CG-NEXT:    [[ADD0:%.*]] = [[MUL]]  +  0;
; HIR-CG-NEXT:    [[EXTRACT0:%.*]] = extractelement [[VAL:%.*]],  [[ADD0]];
; HIR-CG-NEXT:    [[RESULT]] = insertelement [[RESULT]],  [[EXTRACT0]],  0;
; HIR-CG-NEXT:    [[ADD1:%.*]] = [[MUL]]  +  1;
; HIR-CG-NEXT:    [[EXTRACT1:%.*]] = extractelement [[VAL]],  [[ADD1]];
; HIR-CG-NEXT:    [[RESULT]] = insertelement [[RESULT]],  [[EXTRACT1]],  1;
; HIR-CG-NEXT:    [[REPLICATED:%.*]] = shufflevector [[RESULT]],  undef,  <i32 0, i32 1, i32 0, i32 1>;
; HIR-CG-NEXT:    [[USE:%.*]] = <i64 1, i64 1> + [[RESULT]];
;
entry:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %header

header:
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %latch ]
  %cond = icmp sgt i64 %iv, 0
  br i1 %cond, label %uni.start, label %latch

uni.start:
  br i1 %uniform, label %if.then, label %if.else

if.then:
  br label %if.end

if.else:
  br label %if.end

if.end:
  %blend = phi <2 x i64> [ <i64 1, i64 3>, %if.then ], [ <i64 2, i64 4>, %if.else]
  %val = add nsw nuw <2 x i64> %blend, <i64 1, i64 1>
  br label %uni.end

uni.end:
  br label %latch

latch:
  %st = phi <2 x i64> [ <i64 -1, i64 -1>, %header ], [ %val, %uni.end]
  store <2 x i64> %st, ptr %p
  %iv.next = add nsw nuw i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, 4
  br i1 %exitcond, label %exit, label %header

exit:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
