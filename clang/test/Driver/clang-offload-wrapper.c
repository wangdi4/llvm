// INTEL UNSUPPORTED: intel_opencl && i686-pc-windows
// REQUIRES: x86-registered-target

//
// Check help message.
//
// RUN: clang-offload-wrapper --help | FileCheck %s --check-prefix CHECK-HELP
// CHECK-HELP: {{.*}}OVERVIEW: A tool to create a wrapper bitcode for offload target binaries. Takes offload
// CHECK-HELP: {{.*}}target binaries as input and produces bitcode file containing target binaries packaged
// CHECK-HELP: {{.*}}as data and initialization code which registers target binaries in offload runtime.
// CHECK-HELP: {{.*}}USAGE: clang-offload-wrapper [options] <input files>
// CHECK-HELP: {{.*}}  -o <filename>               - Output filename
// CHECK-HELP: {{.*}}  --target=<triple>           - Target triple for the output module

//
// Generate a file to wrap.
//
// RUN: echo 'Content of device file' > %t.tgt

//
// Check bitcode produced by the wrapper tool.
//
// RUN: clang-offload-wrapper -add-omp-offload-notes -target=x86_64-pc-linux-gnu -o %t.wrapper.bc %t.tgt 2>&1 | FileCheck %s --check-prefix ELF-WARNING
// RUN: llvm-dis %t.wrapper.bc -o - | FileCheck %s --check-prefix CHECK-IR

// ELF-WARNING: is not an ELF image, so notes cannot be added to it.
// CHECK-IR: target triple = "x86_64-pc-linux-gnu"

// CHECK-IR-DAG: [[ENTTY:%.+]] = type { ptr, ptr, i{{32|64}}, i32, i32 }
// CHECK-IR-DAG: [[IMAGETY:%.+]] = type { ptr, ptr, ptr, ptr }
// CHECK-IR-DAG: [[DESCTY:%.+]] = type { i32, ptr, ptr, ptr }

// CHECK-IR: [[ENTBEGIN:@.+]] = external hidden constant [[ENTTY]]
// CHECK-IR: [[ENTEND:@.+]] = external hidden constant [[ENTTY]]

// CHECK-IR: [[DUMMY:@.+]] = hidden constant [0 x [[ENTTY]]] zeroinitializer, section "omp_offloading_entries"

// CHECK-IR: [[BIN:@.+]] = internal unnamed_addr constant [[BINTY:\[[0-9]+ x i8\]]] c"Content of device file{{.+}}"

// CHECK-IR: [[IMAGES:@.+]] = internal unnamed_addr constant [1 x [[IMAGETY]]] [{{.+}} { ptr [[BIN]], ptr getelementptr inbounds ([[BINTY]], ptr [[BIN]], i64 1, i64 0), ptr [[ENTBEGIN]], ptr [[ENTEND]] }]

// CHECK-IR: [[DESC:@.+]] = internal constant [[DESCTY]] { i32 1, ptr [[IMAGES]], ptr [[ENTBEGIN]], ptr [[ENTEND]] }

// CHECK-IR: @llvm.global_ctors = appending global [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 1, ptr [[REGFN:@.+]], ptr null }]
// CHECK-IR: @llvm.global_dtors = appending global [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 1, ptr [[UNREGFN:@.+]], ptr null }]

// CHECK-IR: define internal void [[REGFN]]()
// CHECK-IR:   call void @__tgt_register_lib(ptr [[DESC]])
// CHECK-IR:   ret void

// CHECK-IR: declare void @__tgt_register_lib(ptr)

// CHECK-IR: define internal void [[UNREGFN]]()
// CHECK-IR:   call void @__tgt_unregister_lib(ptr [[DESC]])
// CHECK-IR:   ret void

<<<<<<< HEAD
// CHECK-IR: declare void @__tgt_unregister_lib([[DESCTY]]*)

// CHECK-IR: define internal void [[SYCL_REGFN]]()
// CHECK-IR:   call void @__sycl_register_lib([[SYCL_DESCTY]]* [[SYCL_DESC]])
// CHECK-IR:   ret void

// CHECK-IR: declare void @__sycl_register_lib([[SYCL_DESCTY]]*)

// CHECK-IR: define internal void [[SYCL_UNREGFN]]()
// CHECK-IR:   call void @__sycl_unregister_lib([[SYCL_DESCTY]]* [[SYCL_DESC]])
// CHECK-IR:   ret void

// CHECK-IR: declare void @__sycl_unregister_lib([[SYCL_DESCTY]]*)

// -------
// Check options' effects: -emit-reg-funcs, -desc-name
//
// RUN: echo 'Content of device file' > %t.tgt
//
// RUN: clang-offload-wrapper -kind sycl -host=x86_64-pc-linux-gnu -emit-reg-funcs=0 -desc-name=lalala -o - %t.tgt | llvm-dis | FileCheck %s --check-prefix CHECK-IR1
// CHECK-IR1: source_filename = "offload.wrapper.object"
// CHECK-IR1: [[IMAGETY:%.+]] = type { i16, i8, i8, i8*, i8*, i8*, i8*, i8*, i8*, i8*, %__tgt_offload_entry*, %__tgt_offload_entry*, %_pi_device_binary_property_set_struct*, %_pi_device_binary_property_set_struct* }
// CHECK-IR1: [[ENTTY:%.+]] = type { i8*, i8*, i64, i32, i32 }
// CHECK-IR1: [[DESCTY:%.+]] = type { i16, i16, [[IMAGETY]]*, [[ENTTY]]*, [[ENTTY]]* }
// CHECK-IR1-NOT: @llvm.global_ctors
// CHECK-IR1-NOT: @llvm.global_dtors
// CHECK-IR1-NOT: section ".tgtimg"
// CHECK-IR1: @.sycl_offloading.lalala = constant [[DESCTY]] { i16 {{[0-9]+}}, i16 1, [[IMAGETY]]* getelementptr inbounds ([1 x [[IMAGETY]]], [1 x [[IMAGETY]]]* @.sycl_offloading.device_images, i64 0, i64 0), [[ENTTY]]* null, [[ENTTY]]* null }

// -------
// Check option's effects: -entries
//
// RUN: echo 'Content of device file' > %t.tgt
// RUN: echo -e 'entryA\nentryB' > %t.txt
// RUN: clang-offload-wrapper -host=x86_64-pc-linux-gnu -kind=sycl -entries=%t.txt %t.tgt -o - | llvm-dis | FileCheck %s --check-prefix CHECK-IR3
// CHECK-IR3: source_filename = "offload.wrapper.object"
// CHECK-IR3: @__sycl_offload_entry_name = internal unnamed_addr constant [7 x i8] c"entryA\00"
// CHECK-IR3: @__sycl_offload_entry_name.1 = internal unnamed_addr constant [7 x i8] c"entryB\00"
// CHECK-IR3: @__sycl_offload_entries_arr = internal constant [2 x %__tgt_offload_entry] [%__tgt_offload_entry { i8* null, i8* getelementptr inbounds ([7 x i8], [7 x i8]* @__sycl_offload_entry_name, i64 0, i64 0), i64 0, i32 0, i32 0 }, %__tgt_offload_entry { i8* null, i8* getelementptr inbounds ([7 x i8], [7 x i8]* @__sycl_offload_entry_name.1, i64 0, i64 0), i64 0, i32 0, i32 0 }]
// CHECK-IR3: @.sycl_offloading.device_images = internal unnamed_addr constant [1 x %__tgt_device_image] [%__tgt_device_image { {{.*}}, %__tgt_offload_entry* getelementptr inbounds ([2 x %__tgt_offload_entry], [2 x %__tgt_offload_entry]* @__sycl_offload_entries_arr, i64 0, i64 0), %__tgt_offload_entry* getelementptr inbounds ([2 x %__tgt_offload_entry], [2 x %__tgt_offload_entry]* @__sycl_offload_entries_arr, i64 1, i64 0), %_pi_device_binary_property_set_struct* null, %_pi_device_binary_property_set_struct* null }]

// -------
// Check that device image can be extracted from the wrapper object by the clang-offload-bundler tool.
//
// RUN: clang-offload-wrapper -o %t.wrapper.bc -host=x86_64-pc-linux-gnu -kind=sycl -target=spir64-unknown-linux %t1.tgt
// RUN: %clang -target x86_64-pc-linux-gnu -c %t.wrapper.bc -o %t.wrapper.o
// RUN: clang-offload-bundler --type=o --input=%t.wrapper.o --targets=sycl-spir64-unknown-linux --output=%t1.out --unbundle
// RUN: diff %t1.out %t1.tgt
=======
// CHECK-IR: declare void @__tgt_unregister_lib(ptr)
>>>>>>> b16d2b48b425e4607eda0421689875d9c8504b75

// Check that clang-offload-wrapper adds LLVMOMPOFFLOAD notes
// into the ELF offload images:
// RUN: yaml2obj %S/Inputs/empty-elf-template.yaml -o %t.64le -DBITS=64 -DENCODING=LSB
// RUN: clang-offload-wrapper -add-omp-offload-notes -target=x86_64-pc-linux-gnu -o %t.wrapper.elf64le.bc %t.64le
// RUN: llvm-dis %t.wrapper.elf64le.bc -o - | FileCheck %s --check-prefix OMPNOTES
// RUN: yaml2obj %S/Inputs/empty-elf-template.yaml -o %t.64be -DBITS=64 -DENCODING=MSB
// RUN: clang-offload-wrapper -add-omp-offload-notes -target=x86_64-pc-linux-gnu -o %t.wrapper.elf64be.bc %t.64be
// RUN: llvm-dis %t.wrapper.elf64be.bc -o - | FileCheck %s --check-prefix OMPNOTES
// RUN: yaml2obj %S/Inputs/empty-elf-template.yaml -o %t.32le -DBITS=32 -DENCODING=LSB
// RUN: clang-offload-wrapper -add-omp-offload-notes -target=x86_64-pc-linux-gnu -o %t.wrapper.elf32le.bc %t.32le
// RUN: llvm-dis %t.wrapper.elf32le.bc -o - | FileCheck %s --check-prefix OMPNOTES
// RUN: yaml2obj %S/Inputs/empty-elf-template.yaml -o %t.32be -DBITS=32 -DENCODING=MSB
// RUN: clang-offload-wrapper -add-omp-offload-notes -target=x86_64-pc-linux-gnu -o %t.wrapper.elf32be.bc %t.32be
// RUN: llvm-dis %t.wrapper.elf32be.bc -o - | FileCheck %s --check-prefix OMPNOTES

// There is no clean way for extracting the offload image
// from the object file currently, so try to find
// the inserted ELF notes in the device image variable's
// initializer:
// OMPNOTES: @{{.+}} = internal unnamed_addr constant [{{[0-9]+}} x i8] c"{{.*}}LLVMOMPOFFLOAD{{.*}}LLVMOMPOFFLOAD{{.*}}LLVMOMPOFFLOAD{{.*}}"
// INTEL_CUSTOMIZATION
// Check that SPIR-V image is automatically containerized, and the notes
// can be added into the container ELF:

// RUN: clang-offload-wrapper -add-omp-offload-notes                           \
// RUN:   -host=x86_64-pc-linux-gnu -containerize-openmp-images=true           \
// RUN:     -kind=openmp -target=spir64             -format=native %t3.tgt     \
// RUN:   -o %t2.wrapper.bc 2>&1 |                                             \
// RUN: FileCheck %s --allow-empty --check-prefix ELF-CONTAINER-WARNING
// RUN: llvm-dis %t2.wrapper.bc -o - | FileCheck %s --check-prefix ELF-CONTAINER

// ELF-CONTAINER-WARNING-NOT: notes cannot be added
// ELF-CONTAINER: @{{.+}} = internal unnamed_addr constant [{{[0-9]+}} x i8] c"{{.*}}INTELONEOMPOFFLOAD{{.*}}INTELONEOMPOFFLOAD{{.*}}INTELONEOMPOFFLOAD
// ELF-CONTAINER-SAME: LLVMOMPOFFLOAD\00\001.0{{.*}}LLVMOMPOFFLOAD\00\00Intel(R) oneAPI DPC++/C++ Compiler{{.*}}LLVMOMPOFFLOAD\00\00202{{[2-9]}}.{{[0-9]}}.0{{.*}}",
// ELF-CONTAINER-SAME: section "__CLANG_OFFLOAD_BUNDLE__openmp-spirv"
// end INTEL_CUSTOMIZATION
