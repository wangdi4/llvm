// REQUIRES: intel_feature_sw_dtrans
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple i386-pc-windows-msvc -emit-dtrans-info -fexceptions -fcxx-exceptions -emit-llvm -opaque-pointers %s -o - | FileCheck %s 

// Test DTrans generation for inalloca parameters and return values, which are
// just pointers to the actual type, but only seem to appear on 32 bit windows,
// and are seemingly difficult to get to happen, as just about every line here
// is required.
struct Repr {
  union ReprUnion {
    ~ReprUnion() noexcept;
    char *Ptr;
  };
};

struct String {
  String();
  String &append(const String&);
  Repr Rep;
};

struct ErrCode {
  ErrCode() noexcept;
  String message() const;
};

struct SystemErr {
  static String MakeStr(ErrCode E, String Message) {
    Message.append(E.message());
    return Message;
  }
  SystemErr(ErrCode E, const String &Message) {
    MakeStr(E, Message);
  }
};

void use() {
  throw SystemErr(ErrCode{}, String{});
}

// CHECK: declare !intel.dtrans.func.type ![[STR_CTOR:[0-9]+]] {{.*}}"intel_dtrans_func_index"="1" ptr @"??0String@@QAE@XZ"(ptr{{.*}} "intel_dtrans_func_index"="2") 
// CHECK: declare !intel.dtrans.func.type ![[ERR_CODE_CTOR:[0-9]+]] {{.*}}"intel_dtrans_func_index"="1" ptr @"??0ErrCode@@QAE@XZ"(ptr{{.*}} "intel_dtrans_func_index"="2")

// CHECK: define{{.*}} "intel_dtrans_func_index"="1" ptr @"??0SystemErr@@QAE@UErrCode@@ABUString@@@Z"(ptr{{.*}} "intel_dtrans_func_index"="2" %{{.+}}, ptr{{.*}} byval(%"struct..?AUErrCode@@.ErrCode"){{.*}} "intel_dtrans_func_index"="3" %{{.+}}, ptr{{.*}} "intel_dtrans_func_index"="4" %{{.+}}){{.*}} !intel.dtrans.func.type ![[SYSERR_FUNC:[0-9]+]]

// CHECK: declare !intel.dtrans.func.type ![[THROW_EXC:[0-9]+]] {{.*}}@_CxxThrowException(ptr "intel_dtrans_func_index"="1", ptr "intel_dtrans_func_index"="2")

// CHECK: define {{.*}} void @"?MakeStr@SystemErr@@SA?AUString@@UErrCode@@U2@@Z"(ptr {{.*}}sret(%"struct..?AUString@@.String"){{.*}} "intel_dtrans_func_index"="1" %{{.+}}, ptr {{.*}} byval(%"struct..?AUErrCode@@.ErrCode") {{.*}}"intel_dtrans_func_index"="2" %{{.+}}, ptr {{.*}}byval(%"struct..?AUString@@.String"){{.*}} "intel_dtrans_func_index"="3" %{{.+}}){{.*}} !intel.dtrans.func.type ![[MAKE_STR:[0-9]+]]

// CHECK: declare !intel.dtrans.func.type ![[APPEND:[0-9]+]] {{.*}}"intel_dtrans_func_index"="1" ptr @"?append@String@@QAEAAU1@ABU1@@Z"(ptr{{.*}} "intel_dtrans_func_index"="2", ptr{{.*}} "intel_dtrans_func_index"="3")

// CHECK: declare !intel.dtrans.func.type ![[MESSAGE:[0-9]+]] {{.*}}void @"?message@ErrCode@@QBE?AUString@@XZ"(ptr{{.*}} "intel_dtrans_func_index"="1", ptr sret(%"struct..?AUString@@.String"){{.*}} "intel_dtrans_func_index"="2")

// CHECK: !intel.dtrans.types = !{![[SYSERR:[0-9]+]], ![[STRING:[0-9]+]], ![[REPR:[0-9]+]], ![[ERRCODE:[0-9]+]], ![[RTTI:[0-9]+]], ![[THROWINFO:[0-9]+]]}

// CHECK: ![[SYSERR]] = !{!"S", %"struct..?AUSystemErr@@.SystemErr" zeroinitializer, i32 1, ![[CHAR:[0-9]+]]}
// CHECK: ![[CHAR]] = !{i8 0, i32 0}

// CHECK: ![[STRING]] = !{!"S", %"struct..?AUString@@.String" zeroinitializer, i32 1, ![[REPR_REF:[0-9]+]]}
// CHECK: ![[REPR_REF]] = !{%"struct..?AURepr@@.Repr" zeroinitializer, i32 0}

// CHECK: ![[REPR]] = !{!"S", %"struct..?AURepr@@.Repr" zeroinitializer, i32 1, ![[CHAR]]}
// CHECK: ![[ERRCODE]] = !{!"S", %"struct..?AUErrCode@@.ErrCode" zeroinitializer, i32 1, ![[CHAR]]}
// CHECK: ![[RTTI]] = !{!"S", %rtti.TypeDescriptor15 zeroinitializer, i32 3, ![[CHAR_PTRPTR:[0-9]+]], ![[CHAR_PTR:[0-9]+]], ![[CHAR_ARR:[0-9]+]]}
// CHECK: ![[CHAR_PTRPTR]] = !{i8 0, i32 2}
// CHECK: ![[CHAR_PTR]] = !{i8 0, i32 1}
// CHECK: ![[CHAR_ARR]] = !{!"A", i32 16, ![[CHAR]]}

// CHECK: ![[THROWINFO]] = !{!"S", %eh.ThrowInfo zeroinitializer, i32 4, ![[INT:[0-9]+]], ![[CHAR_PTR]], ![[CHAR_PTR]], ![[CHAR_PTR]]}

// CHECK: ![[STR_CTOR]] = distinct !{![[STR_PTR:[0-9]+]], ![[STR_PTR]]}
// CHECK: ![[STR_PTR]] = !{%"struct..?AUString@@.String" zeroinitializer, i32 1}

// CHECK: ![[ERR_CODE_CTOR]] = distinct !{![[ERR_CODE_PTR:[0-9]+]], ![[ERR_CODE_PTR]]}
// CHECK: ![[ERR_CODE_PTR]] = !{%"struct..?AUErrCode@@.ErrCode" zeroinitializer, i32 1}

// CHECK: ![[SYSERR_FUNC]] = distinct !{![[SYSERR_PTR:[0-9]+]], ![[SYSERR_PTR]], ![[ERR_CODE_PTR]], ![[STR_PTR]]}
// CHECK: ![[SYSERR_PTR]] = !{%"struct..?AUSystemErr@@.SystemErr" zeroinitializer, i32 1}

// CHECK: ![[THROW_EXC]] = distinct !{![[CHAR_PTR]], ![[THROW_PTR:[0-9]+]]
// CHECK: ![[THROW_PTR]] = !{%eh.ThrowInfo zeroinitializer, i32 1}

// CHECK: ![[MAKE_STR]] = distinct !{![[STR_PTR]], ![[ERR_CODE_PTR]], ![[STR_PTR]]}
// CHECK: ![[APPEND]] = distinct !{![[STR_PTR]], ![[STR_PTR]], ![[STR_PTR]]}
// CHECK: ![[MESSAGE]] = distinct !{![[ERR_CODE_PTR]], ![[STR_PTR]]}
