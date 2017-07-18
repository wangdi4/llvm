; RUN: opt -VPlanDriver -use-i1-mask-for-simd-funcs -disable-vplan-subregions -S < %s  | FileCheck %s

; CHECK-LABEL: vector.body
; CHECK-NOT: zext <4 x i1>
; CHECK: call <4 x i32> @_ZGVbM4vv_vec_sum

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [12 x i8] c"a[%d] = %d\0A\00", align 1
@.str.1 = private unnamed_addr constant [12 x i8] c"b[%d] = %d\0A\00", align 1
@.str.2 = private unnamed_addr constant [12 x i8] c"c[%d] = %d\0A\00", align 1

define i32 @vec_sum(i32 %i, i32 %j) #0 {
entry:
  %i.addr = alloca i32, align 4
  %j.addr = alloca i32, align 4
  store i32 %i, i32* %i.addr, align 4
  store i32 %j, i32* %j.addr, align 4
  %0 = load i32, i32* %i.addr, align 4
  %1 = load i32, i32* %j.addr, align 4
  %add = add nsw i32 %0, %1
  ret i32 %add
}

define i32 @main() local_unnamed_addr #1 {
entry:
  %a = alloca [200 x i32], align 16
  %b = alloca [200 x i32], align 16
  %c = alloca [200 x i32], align 16
  %0 = bitcast [200 x i32]* %c to i8*
  %t = alloca i32, align 4
  %1 = bitcast [200 x i32]* %a to i8*
  call void @llvm.lifetime.start(i64 800, i8* nonnull %1) #4
  %2 = bitcast [200 x i32]* %b to i8*
  call void @llvm.lifetime.start(i64 800, i8* nonnull %2) #4
  call void @llvm.lifetime.start(i64 800, i8* nonnull %0) #4
  call void @llvm.memset.p0i8.i64(i8* nonnull %0, i8 0, i64 800, i32 16, i1 false)
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv56 = phi i64 [ 0, %entry ], [ %indvars.iv.next57, %for.body ]
  %arrayidx = getelementptr inbounds [200 x i32], [200 x i32]* %a, i64 0, i64 %indvars.iv56
  %3 = trunc i64 %indvars.iv56 to i32
  store i32 %3, i32* %arrayidx, align 4
  %arrayidx2 = getelementptr inbounds [200 x i32], [200 x i32]* %b, i64 0, i64 %indvars.iv56
  store i32 %3, i32* %arrayidx2, align 4
  %indvars.iv.next57 = add nuw nsw i64 %indvars.iv56, 1
  %exitcond58 = icmp eq i64 %indvars.iv.next57, 200
  br i1 %exitcond58, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  tail call void @llvm.intel.directive(metadata !"DIR.OMP.SIMD")
  tail call void @llvm.intel.directive.qual.opnd.i32(metadata !"QUAL.OMP.SIMDLEN", i32 4)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.PRIVATE", i32* nonnull %t)
  call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %DIR.QUAL.LIST.END.1

DIR.QUAL.LIST.END.1:                              ; preds = %for.end
  %4 = bitcast i32* %t to i8*
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %if.end, %DIR.QUAL.LIST.END.1
  %indvars.iv53 = phi i64 [ 0, %DIR.QUAL.LIST.END.1 ], [ %indvars.iv.next54, %if.end ]
  call void @llvm.lifetime.start(i64 4, i8* nonnull %4) #4
  store i32 7, i32* %t, align 4
  %rem59 = and i64 %indvars.iv53, 1
  %tobool = icmp eq i64 %rem59, 0
  br i1 %tobool, label %if.end, label %if.then

if.then:                                          ; preds = %omp.inner.for.body
  %arrayidx7 = getelementptr inbounds [200 x i32], [200 x i32]* %a, i64 0, i64 %indvars.iv53
  %5 = load i32, i32* %arrayidx7, align 4
  %arrayidx9 = getelementptr inbounds [200 x i32], [200 x i32]* %b, i64 0, i64 %indvars.iv53
  %6 = load i32, i32* %arrayidx9, align 4
  %call = call i32 @vec_sum(i32 %5, i32 %6)
  store i32 %call, i32* %t, align 4
  br label %if.end

if.end:                                           ; preds = %omp.inner.for.body, %if.then
  %7 = phi i32 [ 7, %omp.inner.for.body ], [ %call, %if.then ]
  %arrayidx11 = getelementptr inbounds [200 x i32], [200 x i32]* %c, i64 0, i64 %indvars.iv53
  store i32 %7, i32* %arrayidx11, align 4
  call void @llvm.lifetime.end(i64 4, i8* nonnull %4) #4
  %indvars.iv.next54 = add nuw nsw i64 %indvars.iv53, 1
  %exitcond55 = icmp eq i64 %indvars.iv.next54, 200
  br i1 %exitcond55, label %omp.loop.exit, label %omp.inner.for.body

omp.loop.exit:                                    ; preds = %if.end
  call void @llvm.intel.directive(metadata !"DIR.OMP.END.SIMD")
  call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %for.body15

for.body15:                                       ; preds = %for.body15, %omp.loop.exit
  %indvars.iv = phi i64 [ 0, %omp.loop.exit ], [ %indvars.iv.next, %for.body15 ]
  %arrayidx17 = getelementptr inbounds [200 x i32], [200 x i32]* %a, i64 0, i64 %indvars.iv
  %8 = load i32, i32* %arrayidx17, align 4
  %9 = trunc i64 %indvars.iv to i32
  %call18 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([12 x i8], [12 x i8]* @.str, i64 0, i64 0), i32 %9, i32 %8)
  %arrayidx20 = getelementptr inbounds [200 x i32], [200 x i32]* %b, i64 0, i64 %indvars.iv
  %10 = load i32, i32* %arrayidx20, align 4
  %call21 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([12 x i8], [12 x i8]* @.str.1, i64 0, i64 0), i32 %9, i32 %10)
  %arrayidx23 = getelementptr inbounds [200 x i32], [200 x i32]* %c, i64 0, i64 %indvars.iv
  %11 = load i32, i32* %arrayidx23, align 4
  %call24 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([12 x i8], [12 x i8]* @.str.2, i64 0, i64 0), i32 %9, i32 %11)
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 200
  br i1 %exitcond, label %for.end27, label %for.body15

for.end27:                                        ; preds = %for.body15
  call void @llvm.lifetime.end(i64 800, i8* nonnull %0) #4
  call void @llvm.lifetime.end(i64 800, i8* nonnull %2) #4
  call void @llvm.lifetime.end(i64 800, i8* nonnull %1) #4
  ret i32 0
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.intel.directive(metadata) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.intel.directive.qual.opnd.i32(metadata, i32) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.intel.directive.qual.opndlist(metadata, ...) #2

declare i32 @printf(i8*, ...) #4

declare <4 x float> @llvm.masked.load.v4f32.p0v4f32(<4 x float>*, i32, <4 x i1>, <4 x float>) #3

declare void @llvm.masked.store.v4f32.p0v4f32(<4 x float>, <4 x float>*, i32, <4 x i1>) #2

declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i32, i1) #2

attributes #0 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" "vector-variants"="_ZGVbN4vv_vec_sum,_ZGVcN4vv_vec_sum,_ZGVdN4vv_vec_sum,_ZGVeN4vv_vec_sum,_ZGVbM4vv_vec_sum,_ZGVcM4vv_vec_sum,_ZGVdM4vv_vec_sum,_ZGVeM4vv_vec_sum" }
attributes #1 = { noinline norecurse nounwind readnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { argmemonly nounwind }
attributes #3 = { argmemonly nounwind readonly }
attributes #4 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
