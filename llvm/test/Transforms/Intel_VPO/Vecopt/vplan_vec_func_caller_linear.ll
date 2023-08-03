; RUN: opt -passes=vplan-vec -disable-vplan-predicator -S < %s  | FileCheck %s

; CHECK-LABEL: vector.body
; CHECK: call <2 x i64> @_ZGVbN2l_foo

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1

; Function Attrs: noinline nounwind uwtable
define i64 @foo(i64 %i) #0 {
entry:
  %i.addr = alloca i64, align 4
  store i64 %i, ptr %i.addr, align 4
  %0 = load i64, ptr %i.addr, align 4
  %add = add nsw i64 %0, 3
  ret i64 %add
}

; Function Attrs: noinline norecurse nounwind readnone uwtable
define <2 x i64> @_ZGVbN2l_foo(i64 %i) local_unnamed_addr #3 {
entry:
  %vec.retval = alloca <2 x i64>, align 16
  %broadcast.splatinsert = insertelement <2 x i64> undef, i64 %i, i64 0
  %broadcast.splat = shufflevector <2 x i64> %broadcast.splatinsert, <2 x i64> undef, <2 x i32> zeroinitializer
  br label %vector.body

vector.body:                                      ; preds = %vector.body, %entry
  %lsr.iv = phi i64 [ %lsr.iv.next, %vector.body ], [ 0, %entry ]
  %vec.ind = phi <2 x i64> [ <i64 0, i64 1>, %entry ], [ %vec.ind.next, %vector.body ]
  %0 = add <2 x i64> %broadcast.splat, %vec.ind
  %1 = add nsw <2 x i64> %0, <i64 3, i64 3>
  %2 = shl i64 %lsr.iv, 2
  %uglygep = getelementptr i8, ptr %vec.retval, i64 %2
  store <2 x i64> %1, ptr %uglygep, align 16
  %vec.ind.next = add <2 x i64> %vec.ind, <i64 4, i64 4>
  %lsr.iv.next = add nuw nsw i64 %lsr.iv, 2
  %tmp = trunc i64 %lsr.iv.next to i32
  %3 = icmp eq i32 %tmp, 2
  br i1 %3, label %return, label %vector.body

return:                                           ; preds = %vector.body
  %vec.ret = load <2 x i64>, ptr %vec.retval, align 16
  ret <2 x i64> %vec.ret
}

; Function Attrs: noinline nounwind uwtable
define i32 @main() local_unnamed_addr #1 {
entry:
  %a = alloca [1000 x i64], align 16
  call void @llvm.lifetime.start(i64 4000, ptr nonnull %a) #5
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %entry
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 2) ]
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %DIR.OMP.SIMD.1
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.1 ], [ %indvars.iv.next, %omp.inner.for.body ]
  %call = tail call i64 @foo(i64 %indvars.iv)
  %arrayidx = getelementptr inbounds [1000 x i64], ptr %a, i64 0, i64 %indvars.iv
  store i64 %call, ptr %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1000
  br i1 %exitcond, label %omp.loop.exit, label %omp.inner.for.body

omp.loop.exit:                                    ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.QUAL.LIST.END.2

DIR.QUAL.LIST.END.2:                              ; preds = %omp.loop.exit
  %0 = load i64, ptr %a, align 16
  %call3 = tail call i32 (ptr, ...) @printf(ptr @.str, i64 %0)
  call void @llvm.lifetime.end(i64 4000, ptr nonnull %a) #5
  ret i32 0
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, ptr nocapture) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, ptr nocapture) #2

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

declare i32 @printf(ptr, ...) #4

attributes #0 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" "vector-variants"="_ZGVbN2l_foo" }
attributes #1 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { argmemonly nounwind }
attributes #3 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
