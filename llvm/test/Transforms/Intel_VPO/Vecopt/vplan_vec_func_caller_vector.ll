; RUN: opt -passes=vplan-vec -disable-vplan-predicator -S < %s  | FileCheck %s

; CHECK-LABEL: vector.body
; CHECK: call <4 x i32> @_ZGVbN4vv_foo

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1

define i32 @foo(i32 %x, i32 %y) #0 {
entry:
  %x.addr = alloca i32, align 4
  %y.addr = alloca i32, align 4
  store i32 %x, ptr %x.addr, align 4
  store i32 %y, ptr %y.addr, align 4
  %0 = load i32, ptr %x.addr, align 4
  %1 = load i32, ptr %y.addr, align 4
  %add = add nsw i32 %0, %1
  ret i32 %add
}

; Function Attrs: noinline norecurse nounwind readnone uwtable
define <4 x i32> @_ZGVbN4vv_foo(<4 x i32> %x, <4 x i32> %y) local_unnamed_addr #1 {
entry:
  %vec.x = alloca <4 x i32>, align 16
  %vec.y = alloca <4 x i32>, align 16
  %vec.retval = alloca <4 x i32>, align 16
  store <4 x i32> %x, ptr %vec.x, align 16
  store <4 x i32> %y, ptr %vec.y, align 16
  br label %vector.body

vector.body:                                      ; preds = %vector.body, %entry
  %lsr.iv = phi i64 [ %lsr.iv.next, %vector.body ], [ 0, %entry ]
  %0 = shl i64 %lsr.iv, 2
  %uglygep9 = getelementptr i8, ptr %vec.x, i64 %0
  %wide.load = load <4 x i32>, ptr %uglygep9, align 16
  %1 = shl i64 %lsr.iv, 2
  %uglygep6 = getelementptr i8, ptr %vec.y, i64 %1
  %wide.load2 = load <4 x i32>, ptr %uglygep6, align 16
  %2 = add nsw <4 x i32> %wide.load2, %wide.load
  %3 = shl i64 %lsr.iv, 2
  %uglygep = getelementptr i8, ptr %vec.retval, i64 %3
  store <4 x i32> %2, ptr %uglygep, align 16
  %lsr.iv.next = add nuw nsw i64 %lsr.iv, 4
  %tmp = trunc i64 %lsr.iv.next to i32
  %4 = icmp eq i32 %tmp, 4
  br i1 %4, label %return, label %vector.body

return:                                           ; preds = %vector.body
  %vec.ret = load <4 x i32>, ptr %vec.retval, align 16
  ret <4 x i32> %vec.ret
}

; Function Attrs: noinline nounwind uwtable
define i32 @main() local_unnamed_addr #1 {
entry:
  %a = alloca [1000 x i32], align 16
  %b = alloca [1000 x i32], align 16
  %c = alloca [1000 x i32], align 16
  call void @llvm.lifetime.start(i64 4000, ptr nonnull %a) #4
  call void @llvm.lifetime.start(i64 4000, ptr nonnull %b) #4
  call void @llvm.lifetime.start(i64 4000, ptr nonnull %c) #4
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv27 = phi i64 [ 0, %entry ], [ %indvars.iv.next28, %for.body ]
  %arrayidx = getelementptr inbounds [1000 x i32], ptr %a, i64 0, i64 %indvars.iv27
  %0 = trunc i64 %indvars.iv27 to i32
  store i32 %0, ptr %arrayidx, align 4
  %arrayidx2 = getelementptr inbounds [1000 x i32], ptr %b, i64 0, i64 %indvars.iv27
  store i32 %0, ptr %arrayidx2, align 4
  %indvars.iv.next28 = add nuw nsw i64 %indvars.iv27, 1
  %exitcond29 = icmp eq i64 %indvars.iv.next28, 1000
  br i1 %exitcond29, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4) ]
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %for.end
  %indvars.iv = phi i64 [ 0, %for.end ], [ %indvars.iv.next, %omp.inner.for.body ]
  %arrayidx5 = getelementptr inbounds [1000 x i32], ptr %a, i64 0, i64 %indvars.iv
  %1 = load i32, ptr %arrayidx5, align 4
  %arrayidx7 = getelementptr inbounds [1000 x i32], ptr %b, i64 0, i64 %indvars.iv
  %2 = load i32, ptr %arrayidx7, align 4
  %call = tail call i32 @foo(i32 %1, i32 %2)
  %arrayidx9 = getelementptr inbounds [1000 x i32], ptr %c, i64 0, i64 %indvars.iv
  store i32 %call, ptr %arrayidx9, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1000
  br i1 %exitcond, label %omp.loop.exit, label %omp.inner.for.body

omp.loop.exit:                                    ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.QUAL.LIST.END.1

DIR.QUAL.LIST.END.1:                              ; preds = %omp.loop.exit
  %3 = load i32, ptr %c, align 16
  %call12 = tail call i32 (ptr, ...) @printf(ptr @.str, i32 %3)
  call void @llvm.lifetime.end(i64 4000, ptr nonnull %c) #4
  call void @llvm.lifetime.end(i64 4000, ptr nonnull %b) #4
  call void @llvm.lifetime.end(i64 4000, ptr nonnull %a) #4
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

attributes #0 = { noinline norecurse nounwind readnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" "vector-variants"="_ZGVbN4vv_foo,_ZGVcN8vv_foo,_ZGVdN8vv_foo,_ZGVeN16vv_foo,_ZGVbM4vv_foo,_ZGVcM8vv_foo,_ZGVdM8vv_foo,_ZGVeM16vv_foo" }
attributes #1 = { noinline norecurse nounwind readnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { argmemonly nounwind }
attributes #4 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
