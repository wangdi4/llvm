; ModuleID = 'vfclone_private.cpp'
; source_filename = "vfclone_private.cpp"
;
; int foo2(int index, bool *valid);
;
; #pragma omp declare simd uniform(arr)
; void foo1(int tindex, int *arr) {
;   bool valid = false;
;   int index = foo2(tindex, &valid);
;
;   if (valid) {
;     arr[index] = -1;
;   }
; }
;
; RUN: opt -vec-clone -S < %s | FileCheck %s
; RUN: opt -passes="vec-clone" -S < %s | FileCheck %s

; CHECK-LABEL: void @_ZGVbN4vu_4foo1iPi
; CHECK: entry:
; CHECK: %valid = alloca i8, align 1

; CHECK: simd.begin.region:
; CHECK-NEXT: %entry.region = call token @llvm.directive.region.entry()
; CHECK-SAME: DIR.OMP.SIMD
; CHECK-SAME: QUAL.OMP.SIMDLEN
; CHECK-SAME: i32 4
; CHECK-NEXT: br label %simd.loop

; CHECK: simd.end.region:
; CHECK-NEXT: call void @llvm.directive.region.exit(token %entry.region)
; CHECK-SAME: DIR.OMP.END.SIMD
; CHECK-NEXT: br label %return
; CHECK: ret void
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: uwtable
define void @_Z4foo1iPi(i32 %tindex, i32* nocapture %arr) #0 {
entry:
  %valid = alloca i8, align 1
  call void @llvm.lifetime.start(i64 1, i8* nonnull %valid) #3
  store i8 0, i8* %valid, align 1, !tbaa !1
  %call = call i32 @_Z4foo2iPb(i32 %tindex, i8* nonnull %valid)
  %0 = load i8, i8* %valid, align 1, !tbaa !1, !range !5
  %tobool = icmp eq i8 %0, 0
  br i1 %tobool, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  %idxprom = sext i32 %call to i64
  %arrayidx = getelementptr inbounds i32, i32* %arr, i64 %idxprom
  store i32 -1, i32* %arrayidx, align 4, !tbaa !6
  br label %if.end

if.end:                                           ; preds = %entry, %if.then
  call void @llvm.lifetime.end(i64 1, i8* nonnull %valid) #3
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

declare i32 @_Z4foo2iPb(i32, i8*) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #0 = { uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x8664" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" "vector-variants"="_ZGVbN4vu__Z4foo1iPi" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (cfe/trunk)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"bool", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C++ TBAA"}
!5 = !{i8 0, i8 2}
!6 = !{!7, !7, i64 0}
!7 = !{!"int", !3, i64 0}
