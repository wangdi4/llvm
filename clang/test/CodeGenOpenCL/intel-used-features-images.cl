// RUN: %clang_cc1 %s -cl-std=CL2.0 -triple spir -DACCESS_QUALIFIER=read_only  -DIMAGE_TYPE=image1d_t -O0 -emit-llvm -o - | FileCheck %s
// RUN: %clang_cc1 %s -cl-std=CL2.0 -triple spir -DACCESS_QUALIFIER=write_only -DIMAGE_TYPE=image1d_t -O0 -emit-llvm -o - | FileCheck %s
// RUN: %clang_cc1 %s -cl-std=CL2.0 -triple spir -DACCESS_QUALIFIER=read_write -DIMAGE_TYPE=image1d_t -O0 -emit-llvm -o - | FileCheck %s
//
// RUN: %clang_cc1 %s -cl-std=CL2.0 -triple spir -DACCESS_QUALIFIER=read_only  -DIMAGE_TYPE=image1d_array_t -O0 -emit-llvm -o - | FileCheck %s
// RUN: %clang_cc1 %s -cl-std=CL2.0 -triple spir -DACCESS_QUALIFIER=write_only -DIMAGE_TYPE=image1d_array_t -O0 -emit-llvm -o - | FileCheck %s
// RUN: %clang_cc1 %s -cl-std=CL2.0 -triple spir -DACCESS_QUALIFIER=read_write -DIMAGE_TYPE=image1d_array_t -O0 -emit-llvm -o - | FileCheck %s
//
// RUN: %clang_cc1 %s -cl-std=CL2.0 -triple spir -DACCESS_QUALIFIER=read_only  -DIMAGE_TYPE=image1d_buffer_t -O0 -emit-llvm -o - | FileCheck %s
// RUN: %clang_cc1 %s -cl-std=CL2.0 -triple spir -DACCESS_QUALIFIER=write_only -DIMAGE_TYPE=image1d_buffer_t -O0 -emit-llvm -o - | FileCheck %s
// RUN: %clang_cc1 %s -cl-std=CL2.0 -triple spir -DACCESS_QUALIFIER=read_write -DIMAGE_TYPE=image1d_buffer_t -O0 -emit-llvm -o - | FileCheck %s
//
// RUN: %clang_cc1 %s -cl-std=CL2.0 -triple spir -DACCESS_QUALIFIER=read_only  -DIMAGE_TYPE=image2d_t -O0 -emit-llvm -o - | FileCheck %s
// RUN: %clang_cc1 %s -cl-std=CL2.0 -triple spir -DACCESS_QUALIFIER=write_only -DIMAGE_TYPE=image2d_t -O0 -emit-llvm -o - | FileCheck %s
// RUN: %clang_cc1 %s -cl-std=CL2.0 -triple spir -DACCESS_QUALIFIER=read_write -DIMAGE_TYPE=image2d_t -O0 -emit-llvm -o - | FileCheck %s
//
// RUN: %clang_cc1 %s -cl-std=CL2.0 -triple spir -DACCESS_QUALIFIER=read_only  -DIMAGE_TYPE=image2d_array_t -O0 -emit-llvm -o - | FileCheck %s
// RUN: %clang_cc1 %s -cl-std=CL2.0 -triple spir -DACCESS_QUALIFIER=write_only -DIMAGE_TYPE=image2d_array_t -O0 -emit-llvm -o - | FileCheck %s
// RUN: %clang_cc1 %s -cl-std=CL2.0 -triple spir -DACCESS_QUALIFIER=read_write -DIMAGE_TYPE=image2d_array_t -O0 -emit-llvm -o - | FileCheck %s
//
// RUN: %clang_cc1 %s -cl-std=CL2.0 -triple spir -DACCESS_QUALIFIER=read_only  -DIMAGE_TYPE=image2d_depth_t -O0 -emit-llvm -o - | FileCheck --check-prefix=CHECK --check-prefix=CHECK-NON-OPT-DEPTH %s
// RUN: %clang_cc1 %s -cl-std=CL2.0 -triple spir -DACCESS_QUALIFIER=write_only -DIMAGE_TYPE=image2d_depth_t -O0 -emit-llvm -o - | FileCheck --check-prefix=CHECK --check-prefix=CHECK-NON-OPT-DEPTH %s
// RUN: %clang_cc1 %s -cl-std=CL2.0 -triple spir -DACCESS_QUALIFIER=read_write -DIMAGE_TYPE=image2d_depth_t -O0 -emit-llvm -o - | FileCheck --check-prefix=CHECK --check-prefix=CHECK-NON-OPT-DEPTH %s
//
// RUN: %clang_cc1 %s -cl-std=CL2.0 -triple spir -DACCESS_QUALIFIER=read_only  -DIMAGE_TYPE=image2d_array_depth_t -O0 -emit-llvm -o - | FileCheck --check-prefix=CHECK --check-prefix=CHECK-NON-OPT-DEPTH %s
// RUN: %clang_cc1 %s -cl-std=CL2.0 -triple spir -DACCESS_QUALIFIER=write_only -DIMAGE_TYPE=image2d_array_depth_t -O0 -emit-llvm -o - | FileCheck --check-prefix=CHECK --check-prefix=CHECK-NON-OPT-DEPTH %s
// RUN: %clang_cc1 %s -cl-std=CL2.0 -triple spir -DACCESS_QUALIFIER=read_write -DIMAGE_TYPE=image2d_array_depth_t -O0 -emit-llvm -o - | FileCheck --check-prefix=CHECK --check-prefix=CHECK-NON-OPT-DEPTH %s
//
// RUN: %clang_cc1 %s -cl-std=CL2.0 -triple spir -DACCESS_QUALIFIER=read_only  -DIMAGE_TYPE=image2d_msaa_t -O0 -emit-llvm -o - | FileCheck --check-prefix=CHECK --check-prefix=CHECK-NON-OPT-MSAA %s
// RUN: %clang_cc1 %s -cl-std=CL2.0 -triple spir -DACCESS_QUALIFIER=write_only -DIMAGE_TYPE=image2d_msaa_t -O0 -emit-llvm -o - | FileCheck --check-prefix=CHECK --check-prefix=CHECK-NON-OPT-MSAA %s
// RUN: %clang_cc1 %s -cl-std=CL2.0 -triple spir -DACCESS_QUALIFIER=read_write -DIMAGE_TYPE=image2d_msaa_t -O0 -emit-llvm -o - | FileCheck --check-prefix=CHECK --check-prefix=CHECK-NON-OPT-MSAA %s
//
// RUN: %clang_cc1 %s -cl-std=CL2.0 -triple spir -DACCESS_QUALIFIER=read_only  -DIMAGE_TYPE=image2d_array_msaa_t -O0 -emit-llvm -o - | FileCheck --check-prefix=CHECK --check-prefix=CHECK-NON-OPT-MSAA %s
// RUN: %clang_cc1 %s -cl-std=CL2.0 -triple spir -DACCESS_QUALIFIER=write_only -DIMAGE_TYPE=image2d_array_msaa_t -O0 -emit-llvm -o - | FileCheck --check-prefix=CHECK --check-prefix=CHECK-NON-OPT-MSAA %s
// RUN: %clang_cc1 %s -cl-std=CL2.0 -triple spir -DACCESS_QUALIFIER=read_write -DIMAGE_TYPE=image2d_array_msaa_t -O0 -emit-llvm -o - | FileCheck --check-prefix=CHECK --check-prefix=CHECK-NON-OPT-MSAA %s
//
// RUN: %clang_cc1 %s -cl-std=CL2.0 -triple spir -DACCESS_QUALIFIER=read_only  -DIMAGE_TYPE=image2d_msaa_depth_t -O0 -emit-llvm -o - | FileCheck --check-prefix=CHECK --check-prefix=CHECK-NON-OPT-MSAA %s
// RUN: %clang_cc1 %s -cl-std=CL2.0 -triple spir -DACCESS_QUALIFIER=write_only -DIMAGE_TYPE=image2d_msaa_depth_t -O0 -emit-llvm -o - | FileCheck --check-prefix=CHECK --check-prefix=CHECK-NON-OPT-MSAA %s
// RUN: %clang_cc1 %s -cl-std=CL2.0 -triple spir -DACCESS_QUALIFIER=read_write -DIMAGE_TYPE=image2d_msaa_depth_t -O0 -emit-llvm -o - | FileCheck --check-prefix=CHECK --check-prefix=CHECK-NON-OPT-MSAA %s
//
// RUN: %clang_cc1 %s -cl-std=CL2.0 -triple spir -DACCESS_QUALIFIER=read_only  -DIMAGE_TYPE=image2d_array_msaa_depth_t -O0 -emit-llvm -o - | FileCheck --check-prefix=CHECK --check-prefix=CHECK-NON-OPT-MSAA %s
// RUN: %clang_cc1 %s -cl-std=CL2.0 -triple spir -DACCESS_QUALIFIER=write_only -DIMAGE_TYPE=image2d_array_msaa_depth_t -O0 -emit-llvm -o - | FileCheck --check-prefix=CHECK --check-prefix=CHECK-NON-OPT-MSAA %s
// RUN: %clang_cc1 %s -cl-std=CL2.0 -triple spir -DACCESS_QUALIFIER=read_write -DIMAGE_TYPE=image2d_array_msaa_depth_t -O0 -emit-llvm -o - | FileCheck --check-prefix=CHECK --check-prefix=CHECK-NON-OPT-MSAA %s
//
// RUN: %clang_cc1 %s -cl-std=CL2.0 -triple spir -DACCESS_QUALIFIER=read_only  -DIMAGE_TYPE=image3d_t -O0 -emit-llvm -o - | FileCheck %s
// RUN: %clang_cc1 %s -cl-std=CL2.0 -triple spir -DACCESS_QUALIFIER=write_only -DIMAGE_TYPE=image3d_t -O0 -emit-llvm -o - | FileCheck %s
// RUN: %clang_cc1 %s -cl-std=CL2.0 -triple spir -DACCESS_QUALIFIER=read_write -DIMAGE_TYPE=image3d_t -O0 -emit-llvm -o - | FileCheck %s

#pragma OPENCL EXTENSION cl_khr_depth_images : enable
#pragma OPENCL EXTENSION cl_khr_gl_msaa_sharing : enable

kernel void test_read_image(ACCESS_QUALIFIER IMAGE_TYPE img) {}

// CHECK-DAG: !opencl.used.optional.core.features = !{![[MD:[0-9]+]]}
// CHECK-DAG: ![[MD]] = !{!"cl_images"}
//
// CHECK-NON-OPT-DEPTH-DAG: !opencl.used.extensions = !{![[MD:[0-9]+]]}
// CHECK-NON-OPT-DEPTH-DAG: ![[MD]] = !{!"cl_khr_depth_images"}
//
// CHECK-NON-OPT-MSAA-DAG: !opencl.used.extensions = !{![[MD:[0-9]+]]}
// CHECK-NON-OPT-MSAA-DAG: ![[MD]] = !{!"cl_khr_gl_msaa_sharing"}
