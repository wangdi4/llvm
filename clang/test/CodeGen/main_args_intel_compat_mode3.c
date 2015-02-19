// CQ#364268
// RUN: %clang_cc1 -emit-llvm -fintel-compatibility -o - %s | FileCheck %s

typedef struct SC {
    unsigned char a;
    unsigned char b;
    unsigned short c;
} SC;

typedef struct SA {
    SC p[1];
} SA;

typedef struct SB {
    SC p[1];
} SB;


// CHECK: define i32 @main(%struct.SA* %sa, %struct.SB* %sb)
int main(SA *sa, SB *sb) {
}
