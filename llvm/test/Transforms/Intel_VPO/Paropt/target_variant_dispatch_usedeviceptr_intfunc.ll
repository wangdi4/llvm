; RUN: opt -vpo-paropt-prepare -S -vpo-paropt-use-raw-dev-ptr=false -vpo-paropt-use-interop=false < %s | FileCheck %s -check-prefix=BUFFPTR
; RUN: opt < %s -passes='function(vpo-paropt-prepare)' -S -vpo-paropt-use-raw-dev-ptr=false -vpo-paropt-use-interop=false | FileCheck %s -check-prefix=BUFFPTR
; RUN: opt -vpo-paropt-prepare -vpo-paropt-use-interop=false -S < %s | FileCheck %s -check-prefix=RAWPTR
; RUN: opt < %s -passes='function(vpo-paropt-prepare)' -vpo-paropt-use-interop=false -S | FileCheck %s -check-prefix=RAWPTR

; Test for TARGET VARIANT DISPATCH construct with a USE_DEVICE_PTR clause

; C Source
; int __attribute__((nothrow)) foo_gpu(void* p1, void* p2, int dummy) {return 456;}
;
; #pragma omp declare variant(foo_gpu) match(construct={target variant dispatch}, device={arch(gen)})
; int __attribute__((nothrow)) foo(void* p1, void* p2, int dummy) {return 123;}
;
; void *a_cpu; float *floatPtr;
; int main() {
;   void *b_cpu = (void *) floatPtr;
;   int rrr;
;   #pragma omp target variant dispatch use_device_ptr(a_cpu, b_cpu)
;      rrr = foo(a_cpu, b_cpu, 77777);
;   return rrr;
; }
;
; Expected IR: when (default)  -vpo-paropt-use-raw-dev-ptr=false
;
;   %call1 = call i32 @__tgt_is_device_available(i64 -1, i8* null)
;   %available = icmp ne i32 %call1, 0
;   ...
;   br i1 %available, label %if.device.available.create.buffers,
;                     label %end.if.device.available.create.buffers
;
; if.device.available.create.buffers:
;   ...
;   %buffer4 = call i8* @__tgt_create_buffer(i64 -1, i8* %hostPtr3)
;   ...
;   %isNull5 = icmp eq i8* %buffer4, null
;   br i1 %isNull5, label %begin.check.buffer, label %if.ptr.not.null6
;
; if.ptr.not.null6:
;   ...
;   %buffer = call i8* @__tgt_create_buffer(i64 -1, i8* %hostPtr)
;   ...
;
; begin.check.buffer:
;   %dispatch = load i1, i1* %dispatch.flag
;   %notDispatch = xor i1 %dispatch, true
;   br i1 %notDispatch, label %check.unused.buffer, label %end.check.buffer
;
; check.unused.buffer:
;   %buffer7 = load i8*, i8** %tgt.buffer
;   %notNull = icmp ne i8* %buffer7, null
;   br i1 %notNull, label %free.unused.buffer, label %check.unused.buffer8
;
; free.unused.buffer:
;   %4 = call i32 @__tgt_release_buffer(i64 -1, i8* %buffer7)
;   br label %check.unused.buffer8
;
; check.unused.buffer8:
;   %buffer9 = load i8*, i8** %tgt.buffer2
;   %notNull10 = icmp ne i8* %buffer9, null
;   br i1 %notNull10, label %free.unused.buffer11, label %end.check.unused.buffer
;
; free.unused.buffer11:
;   %5 = call i32 @__tgt_release_buffer(i64 -1, i8* %buffer9)
;   br label %end.check.unused.buffer
;
; end.check.unused.buffer:
;   br label %end.check.buffer
;
; end.check.buffer:
;   br label %end.if.device.available.create.buffers
;
; end.if.device.available.create.buffers:
;   %dispatch12 = load i1, i1* %dispatch.flag
;   br i1 %dispatch12, label %variant.call, label %base.call
;
; variant.call:
;   %buffer13 = load i8*, i8** %tgt.buffer
;   %buffer14 = load i8*, i8** %tgt.buffer2
;   %variant = call i32 @foo_gpu(i8* %buffer13, i8* %buffer14, i32 77777)
;   %buffer15 = load i8*, i8** %tgt.buffer
;   %6 = call i32 @__tgt_release_buffer(i64 -1, i8* %buffer15)
;   %buffer16 = load i8*, i8** %tgt.buffer2
;   %7 = call i32 @__tgt_release_buffer(i64 -1, i8* %buffer16)
;   br label %if.end
;
; base.call:
;   %call = call i32 @foo(i8* %2, i8* %3, i32 77777) #3
;   br label %if.end
;
; if.end:
;   %callphi = phi i32 [ %variant, %variant.call ], [ %call, %base.call ]
; ...
;
; Expected IR: when -vpo-paropt-use-raw-dev-ptr=true
;  %call1 = call i32 @__tgt_is_device_available(i64 -1, i8* null)
;  %dispatch = icmp ne i32 %call1, 0
;  br i1 %dispatch, label %variant.call, label %base.call
;
;variant.call:                                     ; preds = %DIR.OMP.TARGET.VARIANT.DISPATCH.2
;  %a_cpu.load = load i8*, i8** @a_cpu, align 8
;  %b_cpu.load = load i8*, i8** %b_cpu, align 8
;...
;  call void @__tgt_target_data_begin(i64 -1, i32 2, i8** %10, i8** %11, i64* getelementptr inbounds ([2 x i64], [2 x i64]* @.offload_sizes, i32 0, i32 0), i64* getelementptr inbounds ([2 x i64], [2 x i64]* @.offload_maptypes, i32 0, i32 0))
;  %a_cpu.buffer.cast = load i8*, i8** %6, align 8
;  %b_cpu.buffer.cast = load i8*, i8** %8, align 8
;  %variant = call i32 @foo_gpu(i8* %a_cpu.buffer.cast, i8* %b_cpu.buffer.cast, i32 77777)
;  call void @__tgt_target_data_end(i64 -1, i32 2, i8** %10, i8** %11, i64* getelementptr inbounds ([2 x i64], [2 x i64]* @.offload_sizes, i32 0, i32 0), i64* getelementptr inbounds ([2 x i64], [2 x i64]* @.offload_maptypes, i32 0, i32 0))
;  br label %if.end
;
;base.call:                                        ; preds = %DIR.OMP.TARGET.VARIANT.DISPATCH.2
;  %call = call i32 @foo(i8* %4, i8* %5, i32 77777) #4
;  br label %if.end
;
;if.end:                                           ; preds = %base.call, %variant.call
;  %callphi = phi i32 [ %variant, %variant.call ], [ %call, %base.call ]
;  store i32 %callphi, i32* %rrr, align 4, !tbaa !8
;  br label %DIR.OMP.END.TARGET.VARIANT.DISPATCH.3
;
;DIR.OMP.END.TARGET.VARIANT.DISPATCH.3:            ; preds = %if.end
;  br label %DIR.OMP.END.TARGET.VARIANT.DISPATCH.4
;....

;Is device ready?
; BUFFPTR: call i32 @__tgt_is_device_available(i64 -1
;
;Create target buffers for both host pointers
; Check that loads from @a_cpu and %b_cpu are used in tgt_create_buffer call,
; BUFFPTR-DAG: [[HOSTPTR1:%hostPtr[^ ]*]] = load i8*, i8** %b_cpu
; BUFFPTR: %{{[^ ]+}} = call i8* @__tgt_create_buffer(i64 -1, i8* [[HOSTPTR1]])
; BUFFPTR-DAG: [[HOSTPTR2:%hostPtr[^ ]*]] = load i8*, i8** @a_cpu
; BUFFPTR: %{{[^ ]+}} = call i8* @__tgt_create_buffer(i64 -1, i8* [[HOSTPTR2]])
;
;Code to clean up target buffers in case some are not created
; BUFFPTR: call i32 @__tgt_release_buffer(i64 -1, i8*
; BUFFPTR: call i32 @__tgt_release_buffer(i64 -1, i8*
;
;Load and check the dispatch flag
; BUFFPTR: [[DISPATCH:%[a-zA-Z._0-9]+]] = load i1
; BUFFPTR-NEXT: br i1 [[DISPATCH]], label %[[VARIANTLBL:[a-zA-Z._0-9]+]], label %[[BASELBL:[a-zA-Z._0-9]+]]
;
;Variant Call: load 2 tgt buffers before call, and release them afterwards
; BUFFPTR: [[VARIANTLBL]]:
; BUFFPTR-DAG: [[BUFF1:%[a-zA-Z._0-9]+]] = load i8*
; BUFFPTR-DAG: [[BUFF2:%[a-zA-Z._0-9]+]] = load i8*
; BUFFPTR: [[VARIANT:%[a-zA-Z._0-9]+]] = call i32 @foo_gpu(i8* [[BUFF1]], i8* [[BUFF2]]
; BUFFPTR: call i32 @__tgt_release_buffer(i64 -1, i8*
; BUFFPTR: call i32 @__tgt_release_buffer(i64 -1, i8*
;
;Base Call:
; BUFFPTR-DAG: [[BASELBL]]:
; BUFFPTR-NEXT: [[BASE:%[a-zA-Z._0-9]+]] = call i32 @foo(i8*
;
;Check phi:
; BUFFPTR: phi i32 [ [[VARIANT]], %[[VARIANTLBL]] ], [ [[BASE]], %[[BASELBL]] ]
;
; when -vpo-paropt-use-raw-dev-ptr=true
;
;Is device ready?
; RAWPTR: call i32 @__tgt_is_device_available(i64 -1
; RAWPTR-NEXT: [[DISPATCH:%[a-zA-Z._0-9]+]] = icmp ne
; RAWPTR-NEXT: br i1 [[DISPATCH]], label %[[VARIANTLBL:[a-zA-Z._0-9]+]], label %[[BASELBL:[a-zA-Z._0-9]+]]
;
;Variant Call
; RAWPTR: [[VARIANTLBL]]:
; RAWPTR: call void @__tgt_target_data_begin({{.+}})
; RAWPTR-DAG: [[ARG1:%[a-zA-Z._0-9]+]] = load i8*, i8**
; RAWPTR-DAG: [[ARG2:%[a-zA-Z._0-9]+]] = load i8*, i8**
; RAWPTR: [[VARIANT:%[a-zA-Z._0-9]+]] = call i32 @foo_gpu(i8* [[ARG1]], i8* [[ARG2]]
; RAWPTR: call void @__tgt_target_data_end({{.+}})
;
;Base Call:
; RAWPTR: [[BASELBL]]:
; RAWPTR: [[BASE:%[a-zA-Z._0-9]+]] = call i32 @foo(i8*
;
;Check phi:
; RAWPTR: phi i32 [ [[VARIANT]], %[[VARIANTLBL]] ], [ [[BASE]], %[[BASELBL]] ]
;
; ModuleID = 'target_variant_dispatch_usedeviceptr_intfunc.c'
source_filename = "target_variant_dispatch_usedeviceptr_intfunc.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

@floatPtr = common dso_local global float* null, align 8
@a_cpu = common dso_local global i8* null, align 8

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @foo_gpu(i8* %p1, i8* %p2, i32 %dummy) #0 {
entry:
  %p1.addr = alloca i8*, align 8
  %p2.addr = alloca i8*, align 8
  %dummy.addr = alloca i32, align 4
  store i8* %p1, i8** %p1.addr, align 8
  store i8* %p2, i8** %p2.addr, align 8
  store i32 %dummy, i32* %dummy.addr, align 4
  ret i32 456
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @foo(i8* %p1, i8* %p2, i32 %dummy) #1 {
entry:
  %p1.addr = alloca i8*, align 8
  %p2.addr = alloca i8*, align 8
  %dummy.addr = alloca i32, align 4
  store i8* %p1, i8** %p1.addr, align 8
  store i8* %p2, i8** %p2.addr, align 8
  store i32 %dummy, i32* %dummy.addr, align 4
  ret i32 123
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #2 {
entry:
  %retval = alloca i32, align 4
  %b_cpu = alloca i8*, align 8
  %rrr = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  %0 = load float*, float** @floatPtr, align 8
  %1 = bitcast float* %0 to i8*
  store i8* %1, i8** %b_cpu, align 8
  br label %DIR.OMP.TARGET.VARIANT.DISPATCH.1

DIR.OMP.TARGET.VARIANT.DISPATCH.1:                ; preds = %entry
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.VARIANT.DISPATCH"(), "QUAL.OMP.USE_DEVICE_PTR:PTR_TO_PTR"(i8** @a_cpu, i8** %b_cpu) ]
  br label %DIR.OMP.TARGET.VARIANT.DISPATCH.2

DIR.OMP.TARGET.VARIANT.DISPATCH.2:                ; preds = %DIR.OMP.TARGET.VARIANT.DISPATCH.1
  %3 = load i8*, i8** @a_cpu, align 8
  %4 = load i8*, i8** %b_cpu, align 8
  %call = call i32 @foo(i8* %3, i8* %4, i32 77777) #3
  store i32 %call, i32* %rrr, align 4
  br label %DIR.OMP.END.TARGET.VARIANT.DISPATCH.3

DIR.OMP.END.TARGET.VARIANT.DISPATCH.3:            ; preds = %DIR.OMP.TARGET.VARIANT.DISPATCH.2
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TARGET.VARIANT.DISPATCH"() ]
  br label %DIR.OMP.END.TARGET.VARIANT.DISPATCH.4

DIR.OMP.END.TARGET.VARIANT.DISPATCH.4:            ; preds = %DIR.OMP.END.TARGET.VARIANT.DISPATCH.3
  %5 = load i32, i32* %rrr, align 4
  ret i32 %5
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #3

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #3


attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "openmp-variant"="name:foo_gpu;construct:target_variant_dispatch;arch:gen" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 9.0.0"}
