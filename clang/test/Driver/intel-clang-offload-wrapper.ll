// REQUIRES: x86-registered-target

//
// Generate a file to wrap.
//
// RUN: echo 'Content of device file' > %t.tgt

//
// Check bitcode produced by the wrapper tool.
//
// RUN: clang-offload-wrapper -kind=openmp -host=x86_64-pc-windows-msvc -o - %t.tgt | llvm-dis | FileCheck %s --check-prefix CHECK-IR

// CHECK-IR: target triple = "x86_64-pc-windows-msvc"

// CHECK-IR-DAG: [[ENTTY:%.+]] = type { i8*, i8*, i{{32|64}}, i32, i32 }
// CHECK-IR-DAG: [[IMAGETY:%.+]] = type { i8*, i8*, [[ENTTY]]*, [[ENTTY]]* }
// CHECK-IR-DAG: [[DESCTY:%.+]] = type { i32, [[IMAGETY]]*, [[ENTTY]]*, [[ENTTY]]* }

// CHECK-IR: [[ENTBEGIN:@.+]] = hidden local_unnamed_addr constant [0 x i8] zeroinitializer, section ".omp_offloading.entries$A", align 32
// CHECK-IR: [[ENTEND:@.+]] = hidden local_unnamed_addr constant [0 x i8] zeroinitializer, section ".omp_offloading.entries$C", align 32

// CHECK-IR: [[BIN:@.+]] = internal unnamed_addr constant [[BINTY:\[[0-9]+ x i8\]]] c"Content of device file{{.+}}"

// CHECK-IR: [[IMAGES:@.+]] = internal unnamed_addr constant [1 x [[IMAGETY]]] [{{.+}} { i8* getelementptr inbounds ([[BINTY]], [[BINTY]]* [[BIN]], i64 0, i64 0), i8* getelementptr inbounds ([[BINTY]], [[BINTY]]* [[BIN]], i64 1, i64 0), [[ENTTY]]* bitcast ([0 x i8]* [[ENTBEGIN]] to [[ENTTY]]*), [[ENTTY]]* bitcast ([0 x i8]* [[ENTEND]] to [[ENTTY]]*) }]

// CHECK-IR: [[DESC:@.+]] = internal constant [[DESCTY]] { i32 1, [[IMAGETY]]* getelementptr inbounds ([1 x [[IMAGETY]]], [1 x [[IMAGETY]]]* [[IMAGES]], i64 0, i64 0), [[ENTTY]]* bitcast ([0 x i8]* [[ENTBEGIN]] to [[ENTTY]]*), [[ENTTY]]* bitcast ([0 x i8]* [[ENTEND]] to [[ENTTY]]*) }

// CHECK-IR: @llvm.global_ctors = appending global [1 x { i32, void ()*, i8* }] [{ i32, void ()*, i8* } { i32 0, void ()* [[REGFN:@.+]], i8* null }]
// CHECK-IR: @llvm.global_dtors = appending global [1 x { i32, void ()*, i8* }] [{ i32, void ()*, i8* } { i32 0, void ()* [[UNREGFN:@.+]], i8* null }]

// CHECK-IR: define internal void [[REGFN]]()
// CHECK-IR:   call void @__tgt_register_lib([[DESCTY]]* [[DESC]])
// CHECK-IR:   ret void

// CHECK-IR: declare void @__tgt_register_lib([[DESCTY]]*)

// CHECK-IR: define internal void [[UNREGFN]]()
// CHECK-IR:   call void @__tgt_unregister_lib([[DESCTY]]* [[DESC]])
// CHECK-IR:   ret void

// CHECK-IR: declare void @__tgt_unregister_lib([[DESCTY]]*)
