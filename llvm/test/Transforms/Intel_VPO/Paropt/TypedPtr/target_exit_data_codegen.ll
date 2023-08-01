; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -vpo-paropt -vpo-paropt-use-mapper-api=false -S %s | FileCheck %s
; RUN: opt -opaque-pointers=0 -passes='vpo-paropt' -vpo-paropt-use-mapper-api=false -S %s | FileCheck %s

; This file tests the implementation to support omp target exit data.
; void device_side_scan(int arg) {
;   #pragma omp target exit data map(from: arg) if(arg) device(4)
;   {++arg;}
; }

; CHECK:  call void @__tgt_target_data_end({{.*}})

target triple = "x86_64-unknown-linux-gnu"
target device_triples = "x86_64-pc-lux-gnu"

@"@tid.addr" = external global i32

define void @_Z16device_side_scani(i32 %arg) {
entry:
  %arg.addr = alloca i32, align 4
  store i32 %arg, i32* %arg.addr, align 4
  %tobool = icmp ne i32 %arg, 0
  %frombool = zext i1 %tobool to i8
  br label %DIR.OMP.TARGET.EXIT.DATA.1

DIR.OMP.TARGET.EXIT.DATA.1:                       ; preds = %entry
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.EXIT.DATA"(),
    "QUAL.OMP.MAP.FROM"(i32* %arg.addr, i32* %arg.addr, i64 4, i64 2, i8* null, i8* null),
    "QUAL.OMP.IF"(i1 %tobool),
    "QUAL.OMP.DEVICE"(i32 4) ]
  br label %DIR.OMP.TARGET.EXIT.DATA.2

DIR.OMP.TARGET.EXIT.DATA.2:                       ; preds = %DIR.OMP.TARGET.EXIT.DATA.1
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET.EXIT.DATA"() ]
  br label %DIR.OMP.END.TARGET.EXIT.DATA.1

DIR.OMP.END.TARGET.EXIT.DATA.1:                   ; preds = %DIR.OMP.TARGET.EXIT.DATA.2
  %1 = load i32, i32* %arg.addr, align 4
  %inc = add nsw i32 %1, 1
  store i32 %inc, i32* %arg.addr, align 4
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
