; RUN: opt < %s -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt  -S | FileCheck %s
; RUN: opt < %s -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt'  -S | FileCheck %s

; This file tests the implementation of omp target with map clause for
; two different target devices. Please note that the device information is
; represented as module level attribute in the form of
; target device_triples = "x86_64-mic,i386-pc-linux-gnu"
;
;  int x = 1, y;
;  int foo()
;  {
;    int i;
;    for (i=0;i<1000;i++)
;    #pragma omp target map(to:x) map(from:y)
;    y = x + 1; // The copy of y on the device has a value of 2.
;    printf("After the target region is executed, y = %d\n", y);
;    return 0;
;  }

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "x86_64-mic,i386-pc-linux-gnu"

@x = common global i32 0, align 4
@y = common global i32 0, align 4
@.str = private unnamed_addr constant [45 x i8] c"After the target region is executed, x = %d\0A\00", align 1

; Function Attrs: nounwind uwtable
define i32 @foo() #0 {
entry:
  %i = alloca i32, align 4
  %0 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #2
  store i32 0, i32* %i, align 4, !tbaa !1
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %1 = load i32, i32* %i, align 4, !tbaa !1
  %cmp = icmp slt i32 %1, 1000
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.MAP.TO"(i32* @x), "QUAL.OMP.MAP.FROM"(i32* @y),"QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0) ]
  %3 = load i32, i32* @x, align 4, !tbaa !1
  %add = add nsw i32 %3, 1
  store i32 %add, i32* @y, align 4, !tbaa !1
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TARGET"() ]
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %4 = load i32, i32* %i, align 4, !tbaa !1
  %inc = add nsw i32 %4, 1
  store i32 %inc, i32* %i, align 4, !tbaa !1
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %5 = load i32, i32* @y, align 4, !tbaa !1
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([45 x i8], [45 x i8]* @.str, i32 0, i32 0), i32 %5)
  %6 = bitcast i32* %i to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %6) #2
  ret i32 0
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

declare i32 @printf(i8*, ...) #3

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind }
attributes #3 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!omp_offload.info = !{!5}
!llvm.ident = !{!0}

!0 = !{!"clang version 5.0.0 (cfe/trunk)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
!5 = !{i32 0, i32 54, i32 -698850821, !"foo", i32 42, i32 0, i32 0}

; CHECK:  @.offload_maptypes = private unnamed_addr constant [2 x i64] [i64 33, i64 34]
; CHECK:  call i32 @__tgt_target({{.*}})
