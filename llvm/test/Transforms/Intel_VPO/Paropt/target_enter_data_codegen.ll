; RUN: opt -bugpoint-enable-legacy-pm -vpo-paropt -vpo-paropt-use-mapper-api=false -S %s | FileCheck %s
; RUN: opt -passes='vpo-paropt' -vpo-paropt-use-mapper-api=false -S %s | FileCheck %s

; This file tests the implementation to support omp target enter data.
; void device_side_scan(int arg) {
;   #pragma omp target enter data map(to: arg) if(arg) device(4)
;   {++arg;}
; }

; CHECK:  call void @__tgt_target_data_begin({{.*}})

target triple = "x86_64-unknown-linux-gnu"
target device_triples = "x86_64-pc-lux-gnu"

@"@tid.addr" = external global i32

define void @_Z16device_side_scani(i32 %arg) {
entry:
  %arg.addr = alloca i32, align 4
  store i32 %arg, ptr %arg.addr, align 4
  %tobool = icmp ne i32 %arg, 0
  %frombool = zext i1 %tobool to i8
  br label %DIR.OMP.TARGET.ENTER.DATA.1

DIR.OMP.TARGET.ENTER.DATA.1:                      ; preds = %entry
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.ENTER.DATA"(),
    "QUAL.OMP.MAP.TO"(ptr %arg.addr, ptr %arg.addr, i64 4, i64 1, ptr null, ptr null),
    "QUAL.OMP.IF"(i1 %tobool),
    "QUAL.OMP.DEVICE"(i32 4) ]
  br label %DIR.OMP.TARGET.ENTER.DATA.2

DIR.OMP.TARGET.ENTER.DATA.2:                      ; preds = %DIR.OMP.TARGET.ENTER.DATA.1
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET.ENTER.DATA"() ]
  br label %DIR.OMP.END.TARGET.ENTER.DATA.1

DIR.OMP.END.TARGET.ENTER.DATA.1:                  ; preds = %DIR.OMP.TARGET.ENTER.DATA.2
  %1 = load i32, ptr %arg.addr, align 4
  %inc = add nsw i32 %1, 1
  store i32 %inc, ptr %arg.addr, align 4
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
