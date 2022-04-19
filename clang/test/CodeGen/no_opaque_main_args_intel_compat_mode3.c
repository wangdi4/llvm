// CQ#364268
// RUN: %clang_cc1 -emit-llvm -no-opaque-pointers -fintel-compatibility -triple x86_64-unknown-linux-gnu -o - %s | FileCheck %s

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


// CHECK: define{{.*}}i32 @main(%struct.SA* noundef %sa, %struct.SB* noundef %sb)
int main(SA *sa, SB *sb) {
}
