//RUN: %clang_cc1 -triple spir64-unknown-unknown-intelfpga -O0 -cl-std=CL2.0 -emit-llvm -o - %s | FileCheck %s
//RUN: %clang_cc1 -triple spir64-unknown-unknown-intelfpga -O0 -cl-std=CL1.2 -emit-llvm -o - %s | FileCheck %s
//RUN: %clang_cc1 -triple x86_64-unknown-unknown-intelfpga -O0 -cl-std=CL2.0 -emit-llvm -o - %s | FileCheck %s
//RUN: %clang_cc1 -triple x86_64-unknown-unknown-intelfpga -O0 -cl-std=CL1.2 -emit-llvm -o - %s | FileCheck %s

// This test file is a copy of CodeGenIntelHLS/pragmas.cpp adopted for OpenCL C
// Main changes:
//   * declarations of SV, SV2 and Sv2p variables moved into foo_ivdep function
//     to workaround OpenCL C 1.2 restriction: program scope variables must
//     reside in constant address space
//   * udpated some CHECK-lines to take into account address spaces

void bar(int i);
int ibar(int i);

//CHECK-LABEL: foo_unroll
void foo_unroll()
{
  //CHECK: br{{.*}}!llvm.loop [[UNROLL1:![0-9]+]]
  #pragma unroll
  for (int i=0;i<32;++i) { bar(i); }

  //CHECK: br{{.*}}!llvm.loop [[UNROLL2:![0-9]+]]
  #pragma unroll 4
  for (int i=0;i<32;++i) { bar(i); }
}

//CHECK-LABEL: foo_coalesce
void foo_coalesce()
{
  //CHECK: br{{.*}}!llvm.loop [[COAL1:![0-9]+]]
  #pragma loop_coalesce
  for (int i=0;i<32;++i) { bar(i); }

  //CHECK: br{{.*}}!llvm.loop [[COAL2:![0-9]+]]
  #pragma loop_coalesce 4
  for (int i=0;i<32;++i) { bar(i); }
}

//CHECK-LABEL: foo_ii
void foo_ii()
{
  //CHECK: br{{.*}}!llvm.loop [[II1:![0-9]+]]
  #pragma ii 4
  for (int i=0;i<32;++i) { bar(i); }
}

//CHECK-LABEL: foo_max_concurrency
void foo_max_concurrency()
{
  //CHECK: br{{.*}}!llvm.loop [[MAXC1:![0-9]+]]
  #pragma max_concurrency 4
  for (int i=0;i<32;++i) { bar(i); }
}

//CHECK-LABEL: foo_max_interleaving
void foo_max_interleaving()
{
  //CHECK: br{{.*}}!llvm.loop [[MAXI1:![0-9]+]]
  for (int j=0;j<32;++j) {
    #pragma max_interleaving 1
    for (int i=0;i<32;++i) { bar(i); }
  }
}

//CHECK-LABEL: foo_ivdep
void foo_ivdep(int select)
{
  struct SIVDep {
    int A[32];
  } SV;

  struct SIVDep2 {
    struct SIVDep X[8][16];
  } SV2, *Sv2p = &SV2;

  //CHECK: br{{.*}}!llvm.loop [[IVDEP1:![0-9]+]]
  #pragma ivdep
  for (int i=0;i<32;++i) { bar(i); }

  //CHECK: br{{.*}}!llvm.loop [[IVDEP2:![0-9]+]]
  #pragma ivdep safelen(4)
  for (int i=0;i<32;++i) { bar(i); }

  int myArray[32];

  //CHECK: [[IVD_TOK1:%[0-9]+]] = call token{{.*}}region.entry() [ "DIR.PRAGMA.IVDEP"(), "QUAL.PRAGMA.ARRAY"([32 x i32]* %myArray, i32 -1) ]
  //CHECK: region.exit(token [[IVD_TOK1]]) [ "DIR.PRAGMA.END.IVDEP"() ]
  #pragma ivdep array(myArray)
  for (int i=0;i<32;++i) { myArray[i] = ibar(i); }

  //CHECK: [[IVD_TOK2:%[0-9]+]] = call token{{.*}}region.entry() [ "DIR.PRAGMA.IVDEP"(), "QUAL.PRAGMA.ARRAY"([32 x i32]* %myArray, i32 8) ]
  //CHECK: region.exit(token [[IVD_TOK2]]) [ "DIR.PRAGMA.END.IVDEP"() ]
  #pragma ivdep safelen(8) array(myArray)
  for (int i=0;i<32;++i) { myArray[i] = ibar(i); }

  //CHECK: [[IVD_TOK3:%[0-9]+]] = call token{{.*}}region.entry() [ "DIR.PRAGMA.IVDEP"(), "QUAL.PRAGMA.ARRAY"([32 x i32]* %myArray, i32 8) ]
  //CHECK: br{{.*}}!llvm.loop [[IVDEP4:![0-9]+]]
  //CHECK: region.exit(token [[IVD_TOK3]]) [ "DIR.PRAGMA.END.IVDEP"() ]
  #pragma unroll 4
  #pragma ivdep safelen(8) array(myArray)
  for (int i=0;i<32;++i) { myArray[i] = ibar(i); }

  //CHECK: [[IVD_TOK4:%[0-9]+]] = call token{{.*}}region.entry() [ "DIR.PRAGMA.IVDEP"(), "QUAL.PRAGMA.ARRAY"([32 x i32]* %myArray, i32 8) ]
  //CHECK: br{{.*}}!llvm.loop [[IVDEP5:![0-9]+]]
  //CHECK: region.exit(token [[IVD_TOK4]]) [ "DIR.PRAGMA.END.IVDEP"() ]
  #pragma ivdep safelen(8) array(myArray)
  #pragma unroll 4
  for (int i=0;i<32;++i) { myArray[i] = ibar(i); }

  //CHECK: [[SVA:%A[0-9]*]] = getelementptr inbounds %struct.SIVDep, %struct.SIVDep* %SV, i32 0, i32 0
  //CHECK: [[IVD_TOK5:%[0-9]+]] = call token{{.*}}region.entry() [ "DIR.PRAGMA.IVDEP"(), "QUAL.PRAGMA.ARRAY"([32 x i32]* [[SVA]], i32 -1) ]
  //CHECK: region.exit(token [[IVD_TOK5]]) [ "DIR.PRAGMA.END.IVDEP"() ]
  #pragma ivdep array(SV.A)
  for (int i=0;i<32;++i) { SV.A[i] = ibar(i); }

  //CHECK: load{{.*}}Sv2p
  //CHECK: [[LSV2P:%A[0-9]*]] = getelementptr{{.*}}%struct.SIVDep, %struct.SIVDep{{.*}}*
  //CHECK: [[IVD_TOK6:%[0-9]+]] = call token{{.*}}region.entry() [ "DIR.PRAGMA.IVDEP"(), "QUAL.PRAGMA.ARRAY"([32 x i32]{{.*}}* [[LSV2P]], i32 -1) ]
  //CHECK: region.exit(token [[IVD_TOK6]]) [ "DIR.PRAGMA.END.IVDEP"() ]
  #pragma ivdep array(Sv2p->X[2][3].A)
  for (int i=0;i<32;++i) { Sv2p->X[2][3].A[i] = ibar(i); }

  int myArray2[32];
  int *ptr = select ? myArray : myArray2;
  //CHECK: [[IVD_TOK7:%[0-9]+]] = call token{{.*}}region.entry() [ "DIR.PRAGMA.IVDEP"(), "QUAL.PRAGMA.ARRAY"(i32{{.*}}** %ptr{{.*}}, i32 -1) ]
  //CHECK: region.exit(token [[IVD_TOK7]]) [ "DIR.PRAGMA.END.IVDEP"() ]
  #pragma ivdep array(ptr)
  for (int i=0;i<32;++i) { ptr[i] = ibar(i); }

  ptr = &myArray[16];
  //CHECK: [[IVD_TOK8:%[0-9]+]] = call token{{.*}}region.entry() [ "DIR.PRAGMA.IVDEP"(), "QUAL.PRAGMA.ARRAY"(i32{{.*}}** %ptr{{.*}}, i32 -1) ]
  //CHECK: region.exit(token [[IVD_TOK8]]) [ "DIR.PRAGMA.END.IVDEP"() ]
  #pragma ivdep array(ptr)
  for (int i=0;i<32;++i) { ptr[i] = ibar(i); }
}

//CHECK: [[UNROLL1]] = distinct !{[[UNROLL1]], [[UNROLL1A:![0-9]+]]}
//CHECK: [[UNROLL1A]] = !{!"llvm.loop.unroll.enable"}
//CHECK: [[UNROLL2]] = distinct !{[[UNROLL2]], [[UNROLL2A:![0-9]+]]}
//CHECK: [[UNROLL2A]] = !{!"llvm.loop.unroll.count", i32 4}
//CHECK: [[COAL1]] = distinct !{[[COAL1]], [[COAL1A:![0-9]+]]}
//CHECK: [[COAL1A]] = !{!"llvm.loop.coalesce.enable"}
//CHECK: [[COAL2]] = distinct !{[[COAL2]], [[COAL2A:![0-9]+]]}
//CHECK: [[COAL2A]] = !{!"llvm.loop.coalesce.count", i32 4}
//CHECK: [[II1]] = distinct !{[[II1]], [[II1A:![0-9]+]]}
//CHECK: [[II1A]] = !{!"llvm.loop.ii.count", i32 4}
//CHECK: [[MAXC1]] = distinct !{[[MAXC1]], [[MAXC1A:![0-9]+]]}
//CHECK: [[MAXC1A]] = !{!"llvm.loop.max_concurrency.count", i32 4}
//CHECK: [[MAXI1]] = distinct !{[[MAXI1]], [[MAXI1A:![0-9]+]]}
//CHECK: [[MAXI1A]] = !{!"llvm.loop.max_interleaving.count", i32 1}
//CHECK: [[IVDEP1]] = distinct !{[[IVDEP1]], [[IVDEP1A:![0-9]+]]}
//CHECK: [[IVDEP1A]] = !{!"llvm.loop.ivdep.enable"}
//CHECK: [[IVDEP2]] = distinct !{[[IVDEP2]], [[IVDEP2A:![0-9]+]]}
//CHECK: [[IVDEP2A]] = !{!"llvm.loop.ivdep.safelen", i32 4}
//CHECK: [[IVDEP4]] = distinct !{[[IVDEP4]], [[UNROLL2A]]}
//CHECK: [[IVDEP5]] = distinct !{[[IVDEP5]], [[UNROLL2A]]}
