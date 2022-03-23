; RUN: opt -vpo-paropt -vpo-paropt-use-mapper-api=false -S %s | FileCheck %s
; RUN: opt -passes='vpo-paropt' -vpo-paropt-use-mapper-api=false -S %s | FileCheck %s

; This file tests the implementation to support omp target exit data.
; void device_side_scan(int arg) {
;   #pragma omp target exit data map(from: arg) if(arg) device(4)
;   {++arg;}
; }

target triple = "x86_64-unknown-linux-gnu"
target device_triples = "x86_64-pc-lux-gnu"

@"@tid.addr" = external global i32

; Function Attrs: nounwind uwtable
define void @_Z16device_side_scani(i32 %arg) #0 {
entry:
  %arg.addr = alloca i32, align 4
  store i32 %arg, i32* %arg.addr, align 4, !tbaa !2
  %tobool = icmp ne i32 %arg, 0
  %frombool = zext i1 %tobool to i8
  br label %DIR.OMP.TARGET.EXIT.DATA.1

DIR.OMP.TARGET.EXIT.DATA.1:                       ; preds = %entry
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.EXIT.DATA"(), "QUAL.OMP.MAP.FROM"(i32* %arg.addr, i32* %arg.addr, i64 4, i64 2, i8* null, i8* null), "QUAL.OMP.IF"(i1 %tobool), "QUAL.OMP.DEVICE"(i32 4) ]
  br label %DIR.OMP.TARGET.EXIT.DATA.2

DIR.OMP.TARGET.EXIT.DATA.2:                       ; preds = %DIR.OMP.TARGET.EXIT.DATA.1
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET.EXIT.DATA"() ]
  br label %DIR.OMP.END.TARGET.EXIT.DATA.1

DIR.OMP.END.TARGET.EXIT.DATA.1:                   ; preds = %DIR.OMP.TARGET.EXIT.DATA.2
  %1 = load i32, i32* %arg.addr, align 4, !tbaa !2
  %inc = add nsw i32 %1, 1
  store i32 %inc, i32* %arg.addr, align 4, !tbaa !2
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #2

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { argmemonly nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 6.0.0"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C++ TBAA"}

; CHECK:  call void @__tgt_target_data_end({{.*}})
