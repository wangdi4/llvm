//RUN: %clang_cc1 -fhls -emit-llvm -o - %s | FileCheck %s
//RUN: %clang_cc1 -fhls -debug-info-kind=limited -emit-llvm -o %t %s

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

struct SIVDep {
  int A[32];
}SV;

struct SIVDep2 {
  SIVDep X[8][16];
}SV2, *Sv2p = &SV2;

//CHECK-LABEL: foo_ivdep
void foo_ivdep(int select)
{
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

  //CHECK: [[IVD_TOK5:%[0-9]+]] = call token{{.*}}region.entry() [ "DIR.PRAGMA.IVDEP"(), "QUAL.PRAGMA.ARRAY"([32 x i32]* getelementptr inbounds (%struct.SIVDep, %struct.SIVDep* {{.*}}SV{{.*}}, i32 0, i32 0), i32 -1) ]
  //CHECK: region.exit(token [[IVD_TOK5]]) [ "DIR.PRAGMA.END.IVDEP"() ]
  #pragma ivdep array(SV.A)
  for (int i=0;i<32;++i) { SV.A[i] = ibar(i); }

  //CHECK: load{{.*}}Sv2p
  //CHECK: [[LSV2P:%A[0-9]*]] = getelementptr{{.*}}%struct.SIVDep, %struct.SIVDep*
  //CHECK: [[IVD_TOK6:%[0-9]+]] = call token{{.*}}region.entry() [ "DIR.PRAGMA.IVDEP"(), "QUAL.PRAGMA.ARRAY"([32 x i32]* [[LSV2P]], i32 -1) ]
  //CHECK: region.exit(token [[IVD_TOK6]]) [ "DIR.PRAGMA.END.IVDEP"() ]
  #pragma ivdep array(Sv2p->X[2][3].A)
  for (int i=0;i<32;++i) { Sv2p->X[2][3].A[i] = ibar(i); }

  int myArray2[32];
  int *ptr = select ? myArray : myArray2;
  //CHECK: [[IVD_TOK7:%[0-9]+]] = call token{{.*}}region.entry() [ "DIR.PRAGMA.IVDEP"(), "QUAL.PRAGMA.ARRAY"(i32** %ptr{{.*}}) ]
  //CHECK: region.exit(token [[IVD_TOK7]]) [ "DIR.PRAGMA.END.IVDEP"() ]
  #pragma ivdep array(ptr)
  for (int i=0;i<32;++i) { ptr[i] = ibar(i); }

  ptr = &myArray[16];
  //CHECK: [[IVD_TOK8:%[0-9]+]] = call token{{.*}}region.entry() [ "DIR.PRAGMA.IVDEP"(), "QUAL.PRAGMA.ARRAY"(i32** %ptr{{.*}}) ]
  //CHECK: region.exit(token [[IVD_TOK8]]) [ "DIR.PRAGMA.END.IVDEP"() ]
  #pragma ivdep array(ptr)
  for (int i=0;i<32;++i) { ptr[i] = ibar(i); }
  //
  //CHECK: [[IVD_TOK9:%[0-9]+]] = call token{{.*}}region.entry()
  //CHECK-SAME: [ "DIR.PRAGMA.IVDEP"(),
  //CHECK-SAME: "QUAL.PRAGMA.ARRAY"([32 x i32]* %myArray2, i32 -1,
  //CHECK-SAME: [32 x i32]* %myArray, i32 -1) ]
  //CHECK: br{{.*}}!llvm.loop [[IVDEP3:![0-9]+]]
  //CHECK: region.exit(token [[IVD_TOK9]]) [ "DIR.PRAGMA.END.IVDEP"() ]
  #pragma ivdep array(myArray2)
  #pragma ivdep array(myArray)
  #pragma ivdep safelen(8)
  for (int i=0;i<32;++i) { myArray[i] = ibar(i); }
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
//CHECK: [[IVDEP1]] = distinct !{[[IVDEP1]], [[IVDEP1A:![0-9]+]]}
//CHECK: [[IVDEP1A]] = !{!"llvm.loop.ivdep.enable"}
//CHECK: [[IVDEP2]] = distinct !{[[IVDEP2]], [[IVDEP2A:![0-9]+]]}
//CHECK: [[IVDEP2A]] = !{!"llvm.loop.ivdep.safelen", i32 4}
//CHECK: [[IVDEP4]] = distinct !{[[IVDEP4]], [[UNROLL2A]]}
//CHECK: [[IVDEP5]] = distinct !{[[IVDEP5]], [[UNROLL2A]]}
//CHECK: [[IVDEP3]] = distinct !{[[IVDEP3]], [[IVDEP3A:![0-9]+]]}
//CHECK: [[IVDEP3A]] = !{!"llvm.loop.ivdep.safelen", i32 8}
