//===-- clang-offload-wrapper/ClangOffloadWrapper.cpp -----------*- C++ -*-===//
// INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2021 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
// end INTEL_CUSTOMIZATION
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// Implementation of the offload wrapper tool. It takes offload target binaries
/// as input and creates wrapper bitcode file containing target binaries
/// packaged as data. Wrapper bitcode also includes initialization code which
/// registers target binaries in offloading runtime at program startup.
///
//===----------------------------------------------------------------------===//

#include "clang/Basic/Version.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/Triple.h"
#include "llvm/BinaryFormat/ELF.h"
#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/Config/dpcpp.version.info.h" // INTEL
#include "llvm/IR/Constants.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Object/ELFObjectFile.h"
#include "llvm/Object/ObjectFile.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/EndianStream.h"
#include "llvm/Support/Errc.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/ErrorOr.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/Program.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/ToolOutputFile.h"
#include "llvm/Support/VCSRevision.h"
#include "llvm/Support/WithColor.h"
#if INTEL_CUSTOMIZATION
#include "llvm/ObjectYAML/ELFYAML.h"
#endif // INTEL_CUSTOMIZATION
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/ModuleUtils.h"
#include <cassert>
#include <cstdint>

#define OPENMP_OFFLOAD_IMAGE_VERSION "1.0"

using namespace llvm;
using namespace llvm::object;

static cl::opt<bool> Help("h", cl::desc("Alias for -help"), cl::Hidden);
#if INTEL_CUSTOMIZATION
static cl::opt<bool> ContainerizeOpenMPImages(
    "containerize-openmp-images", cl::Hidden, cl::init(true),
    cl::desc("Place all OpenMP SPIR-V images into one ELF container"));
#endif // INTEL_CUSTOMIZATION

// Mark all our options with this category, everything else (except for -version
// and -help) will be hidden.
static cl::OptionCategory
    ClangOffloadWrapperCategory("clang-offload-wrapper options");

static cl::opt<std::string> Output("o", cl::Required,
                                   cl::desc("Output filename"),
                                   cl::value_desc("filename"),
                                   cl::cat(ClangOffloadWrapperCategory));

static cl::list<std::string> Inputs(cl::Positional, cl::OneOrMore,
                                    cl::desc("<input files>"),
                                    cl::cat(ClangOffloadWrapperCategory));

static cl::opt<std::string>
    Target("target", cl::Required,
           cl::desc("Target triple for the output module"),
           cl::value_desc("triple"), cl::cat(ClangOffloadWrapperCategory));

static cl::opt<bool> SaveTemps(
    "save-temps",
    cl::desc("Save temporary files that may be produced by the tool. "
             "This option forces print-out of the temporary files' names."),
    cl::Hidden);

static cl::opt<bool> AddOpenMPOffloadNotes(
    "add-omp-offload-notes",
#if INTEL_COLLAB
    cl::init(true),
#endif // INTEL_COLLAB
    cl::desc("Add LLVMOMPOFFLOAD ELF notes to ELF device images."), cl::Hidden);

namespace {

class BinaryWrapper {
<<<<<<< HEAD
public:
  /// Represents a single image to wrap.
  class Image {
  public:
    Image(const llvm::StringRef File_, const llvm::StringRef Manif_,
          const llvm::StringRef Tgt_, BinaryImageFormat Fmt_,
          const llvm::StringRef CompileOpts_, const llvm::StringRef LinkOpts_,
          const llvm::StringRef EntriesFile_, const llvm::StringRef PropsFile_
#if INTEL_CUSTOMIZATION
          ,
          const size_t SubSections_ = 0
#endif // INTEL_CUSTOMIZATION
          )
        : File(File_.str()), Manif(Manif_.str()), Tgt(Tgt_.str()), Fmt(Fmt_),
          CompileOpts(CompileOpts_.str()), LinkOpts(LinkOpts_.str()),
          EntriesFile(EntriesFile_.str()), PropsFile(PropsFile_.str())
#if INTEL_CUSTOMIZATION
          ,
          SubSections(SubSections_)
#endif // INTEL_CUSTOMIZATION
    {
    }

    /// Name of the file with actual contents
    const std::string File;
    /// Name of the manifest file
    const std::string Manif;
    /// Offload target architecture
    const std::string Tgt;
    /// Format
    const BinaryImageFormat Fmt;
    /// Compile options
    const std::string CompileOpts;
    /// Link options
    const std::string LinkOpts;
    /// File listing contained entries
    const std::string EntriesFile;
    /// File with properties
    const std::string PropsFile;

#if INTEL_CUSTOMIZATION
    /// Number of subsections in an application if the image has been read
    /// from a list for the -split-batch option. By default, there is no
    /// subsections and the whole file is a monolith.
    const size_t SubSections;
#endif // INTEL_CUSTOMIZATION

    friend raw_ostream &operator<<(raw_ostream &Out, const Image &Img);
  };

private:
  using SameKindPack = llvm::SmallVector<std::unique_ptr<Image>, 4>;

=======
>>>>>>> b16d2b48b425e4607eda0421689875d9c8504b75
  LLVMContext C;
  Module M;

  StructType *EntryTy = nullptr;
  StructType *ImageTy = nullptr;
  StructType *DescTy = nullptr;

<<<<<<< HEAD
  // SYCL image and binary descriptor types have diverged from libomptarget
  // definitions, but presumably they will converge in future. So, these SYCL
  // specific types should be removed if/when this happens.
  StructType *SyclImageTy = nullptr;
  StructType *SyclDescTy = nullptr;
  StructType *SyclPropSetTy = nullptr;
  StructType *SyclPropTy = nullptr;

  /// Records all added device binary images per offload kind.
  llvm::DenseMap<OffloadKind, std::unique_ptr<SameKindPack>> Packs;
  /// Records all created memory buffers for safe auto-gc
  llvm::SmallVector<std::unique_ptr<MemoryBuffer>, 4> AutoGcBufs;

public:
  void
  addImage(const OffloadKind Kind, llvm::StringRef File, llvm::StringRef Manif,
           llvm::StringRef Tgt, const BinaryImageFormat Fmt,
           llvm::StringRef CompileOpts, llvm::StringRef LinkOpts,
           llvm::StringRef EntriesFile, llvm::StringRef PropsFile
#if INTEL_CUSTOMIZATION
           ,
           const size_t SubSections = 0 // by default, no subsections are there
#endif                                  // INTEL_CUSTOMIZATION
  ) {
    std::unique_ptr<SameKindPack> &Pack = Packs[Kind];
    if (!Pack)
      Pack.reset(new SameKindPack());
    Pack->emplace_back(std::make_unique<Image>(
        File, Manif, Tgt, Fmt, CompileOpts, LinkOpts, EntriesFile, PropsFile
#if INTEL_CUSTOMIZATION
        ,
        SubSections
#endif // INTEL_CUSTOMIZATION
        ));
  }

=======
>>>>>>> b16d2b48b425e4607eda0421689875d9c8504b75
  std::string ToolName;
  std::string ObjcopyPath;
  // Temporary file names that may be created during adding notes
  // to ELF offload images. Use -save-temps to keep them and also
  // see their names. A temporary file's name includes the name
  // of the original input ELF image, so you can easily match
  // them, if you have multiple inputs.
  std::vector<std::string> TempFiles;

#if INTEL_CUSTOMIZATION
  std::string Yaml2ObjPath;
#endif // INTEL_CUSTOMIZATION
private:
  IntegerType *getSizeTTy() {
    switch (M.getDataLayout().getPointerTypeSize(Type::getInt8PtrTy(C))) {
    case 4u:
      return Type::getInt32Ty(C);
    case 8u:
      return Type::getInt64Ty(C);
    }
    llvm_unreachable("unsupported pointer type size");
  }

  // struct __tgt_offload_entry {
  //   void *addr;
  //   char *name;
  //   size_t size;
  //   int32_t flags;
  //   int32_t reserved;
  // };
  StructType *getEntryTy() {
    if (!EntryTy)
      EntryTy = StructType::create("__tgt_offload_entry", Type::getInt8PtrTy(C),
                                   Type::getInt8PtrTy(C), getSizeTTy(),
                                   Type::getInt32Ty(C), Type::getInt32Ty(C));
    return EntryTy;
  }

  PointerType *getEntryPtrTy() { return PointerType::getUnqual(getEntryTy()); }

  // struct __tgt_device_image {
  //   void *ImageStart;
  //   void *ImageEnd;
  //   __tgt_offload_entry *EntriesBegin;
  //   __tgt_offload_entry *EntriesEnd;
  // };
  StructType *getDeviceImageTy() {
    if (!ImageTy)
      ImageTy = StructType::create("__tgt_device_image", Type::getInt8PtrTy(C),
                                   Type::getInt8PtrTy(C), getEntryPtrTy(),
                                   getEntryPtrTy());
    return ImageTy;
  }

  PointerType *getDeviceImagePtrTy() {
    return PointerType::getUnqual(getDeviceImageTy());
  }

  // struct __tgt_bin_desc {
  //   int32_t NumDeviceImages;
  //   __tgt_device_image *DeviceImages;
  //   __tgt_offload_entry *HostEntriesBegin;
  //   __tgt_offload_entry *HostEntriesEnd;
  // };
  StructType *getBinDescTy() {
    if (!DescTy)
      DescTy = StructType::create("__tgt_bin_desc", Type::getInt32Ty(C),
                                  getDeviceImagePtrTy(), getEntryPtrTy(),
                                  getEntryPtrTy());
    return DescTy;
  }

  PointerType *getBinDescPtrTy() {
    return PointerType::getUnqual(getBinDescTy());
  }

  /// Creates binary descriptor for the given device images. Binary descriptor
  /// is an object that is passed to the offloading runtime at program startup
  /// and it describes all device images available in the executable or shared
  /// library. It is defined as follows
  ///
  /// __attribute__((visibility("hidden")))
  /// extern __tgt_offload_entry *__start_omp_offloading_entries;
  /// __attribute__((visibility("hidden")))
  /// extern __tgt_offload_entry *__stop_omp_offloading_entries;
  ///
  /// static const char Image0[] = { <Bufs.front() contents> };
  ///  ...
  /// static const char ImageN[] = { <Bufs.back() contents> };
  ///
  /// static const __tgt_device_image Images[] = {
  ///   {
  ///     Image0,                            /*ImageStart*/
  ///     Image0 + sizeof(Image0),           /*ImageEnd*/
  ///     __start_omp_offloading_entries,    /*EntriesBegin*/
  ///     __stop_omp_offloading_entries      /*EntriesEnd*/
  ///   },
  ///   ...
  ///   {
  ///     ImageN,                            /*ImageStart*/
  ///     ImageN + sizeof(ImageN),           /*ImageEnd*/
  ///     __start_omp_offloading_entries,    /*EntriesBegin*/
  ///     __stop_omp_offloading_entries      /*EntriesEnd*/
  ///   }
  /// };
  ///
  /// static const __tgt_bin_desc BinDesc = {
  ///   sizeof(Images) / sizeof(Images[0]),  /*NumDeviceImages*/
  ///   Images,                              /*DeviceImages*/
  ///   __start_omp_offloading_entries,      /*HostEntriesBegin*/
  ///   __stop_omp_offloading_entries        /*HostEntriesEnd*/
  /// };
  ///
  /// Global variable that represents BinDesc is returned.
<<<<<<< HEAD
  Expected<GlobalVariable *> createBinDesc(OffloadKind Kind,
                                           SameKindPack &Pack) {
    const std::string OffloadKindTag =
        (Twine(".") + offloadKindToString(Kind) + Twine("_offloading.")).str();

    Constant *EntriesB = nullptr, *EntriesE = nullptr;

    if (Kind != OffloadKind::SYCL) {
#if INTEL_COLLAB
      GlobalVariable *EntriesStart = nullptr, *EntriesStop = nullptr;

      if (Triple(M.getTargetTriple()).isWindowsMSVCEnvironment() &&
          Kind == OffloadKind::OpenMP) {
        auto LabelTy = ArrayType::get(Type::getInt8Ty(C), 0);

        EntriesStart = new GlobalVariable(
            M, LabelTy, /*isConstant*/ true, GlobalValue::ExternalLinkage,
            ConstantAggregateZero::get(LabelTy),
            Twine(OffloadKindTag) + Twine("entries_begin"));
        EntriesStart->setAlignment(MaybeAlign(32));
        EntriesStart->setSection("omp_offloading_entries$A");
        EntriesStart->setVisibility(GlobalValue::HiddenVisibility);
        EntriesStart->setUnnamedAddr(GlobalValue::UnnamedAddr::Local);
        EntriesB = ConstantExpr::getBitCast(EntriesStart, getEntryPtrTy());

        EntriesStop = new GlobalVariable(
            M, LabelTy, /*isConstant*/ true, GlobalValue::ExternalLinkage,
            ConstantAggregateZero::get(LabelTy),
            Twine(OffloadKindTag) + Twine("entries_end"));
        // Set 32-byte alignment so that (entries_end - entries_begin) % 32 == 0
        // MSVC incremental linking may introduce zero padding bytes
        // in the middle of the section, and we want to make sure
        // entries_end points to the end of 32-byte aligned chunk,
        // otherwise libomptarget may read past the section.
        EntriesStop->setAlignment(MaybeAlign(32));
        EntriesStop->setSection("omp_offloading_entries$C");
        EntriesStop->setVisibility(GlobalValue::HiddenVisibility);
        EntriesStop->setUnnamedAddr(GlobalValue::UnnamedAddr::Local);
        EntriesE = ConstantExpr::getBitCast(EntriesStop, getEntryPtrTy());

        // LLD will discard empty omp_offloading_entries section,
        // if the compiler does not generate any entries. Then the references
        // to the start/stop symbols will become an error. MSVC linker
        // seems to work even with the empty section.
        // We have to add a dummy entry symbol of *non-zero* size
        // to keep the section alive. We also have to make sure that
        // the dummy symbol is not placed between the start/stop symbols,
        // hence the "$D" suffix below.
        // The worst effect of this is a 32-byte blob per executable.
        auto *DummyInit =
          ConstantAggregateZero::get(ArrayType::get(getEntryTy(), 1u));
        auto *DummyEntry = new GlobalVariable(
            M, DummyInit->getType(), true, GlobalVariable::ExternalLinkage,
            DummyInit, "__dummy.omp_offloading.entry");
        DummyEntry->setAlignment(MaybeAlign(32));
        DummyEntry->setSection("omp_offloading_entries$D");
        DummyEntry->setVisibility(GlobalValue::HiddenVisibility);
      } else {
        // Create external begin/end symbols for the offload entries table.
        EntriesStart = new GlobalVariable(
            M, getEntryTy(), /*isConstant*/ true, GlobalValue::ExternalLinkage,
            /*Initializer*/ nullptr, "__start_omp_offloading_entries");
        EntriesStart->setVisibility(GlobalValue::HiddenVisibility);
        EntriesStop = new GlobalVariable(
            M, getEntryTy(), /*isConstant*/ true, GlobalValue::ExternalLinkage,
            /*Initializer*/ nullptr, "__stop_omp_offloading_entries");
        EntriesStop->setVisibility(GlobalValue::HiddenVisibility);

        // We assume that external begin/end symbols that we have created above
        // will be defined by the linker. But linker will do that only if linker
        // inputs have section with "omp_offloading_entries" name which is not
        // guaranteed. So, we just create dummy zero sized object in the offload
        // entries section to force linker to define those symbols.
        auto *DummyInit =
            ConstantAggregateZero::get(ArrayType::get(getEntryTy(), 0u));
        auto *DummyEntry = new GlobalVariable(
            M, DummyInit->getType(), true, GlobalVariable::ExternalLinkage,
            DummyInit, "__dummy.omp_offloading.entry");
        DummyEntry->setSection("omp_offloading_entries");
        DummyEntry->setVisibility(GlobalValue::HiddenVisibility);

        EntriesB = EntriesStart;
        EntriesE = EntriesStop;
      }
#endif  // INTEL_COLLAB

      if (Verbose) {
        errs() << "  global added: " << EntriesStart->getName() << "\n";
        errs() << "  global added: " << EntriesStop->getName() << "\n";
      }
    } else {
      // Host entry table is not used in SYCL
      EntriesB = Constant::getNullValue(getEntryPtrTy());
      EntriesE = Constant::getNullValue(getEntryPtrTy());
    }
=======
  GlobalVariable *createBinDesc(ArrayRef<ArrayRef<char>> Bufs) {
    // Create external begin/end symbols for the offload entries table.
    auto *EntriesB = new GlobalVariable(
        M, getEntryTy(), /*isConstant*/ true, GlobalValue::ExternalLinkage,
        /*Initializer*/ nullptr, "__start_omp_offloading_entries");
    EntriesB->setVisibility(GlobalValue::HiddenVisibility);
    auto *EntriesE = new GlobalVariable(
        M, getEntryTy(), /*isConstant*/ true, GlobalValue::ExternalLinkage,
        /*Initializer*/ nullptr, "__stop_omp_offloading_entries");
    EntriesE->setVisibility(GlobalValue::HiddenVisibility);

    // We assume that external begin/end symbols that we have created above will
    // be defined by the linker. But linker will do that only if linker inputs
    // have section with "omp_offloading_entries" name which is not guaranteed.
    // So, we just create dummy zero sized object in the offload entries section
    // to force linker to define those symbols.
    auto *DummyInit =
        ConstantAggregateZero::get(ArrayType::get(getEntryTy(), 0u));
    auto *DummyEntry = new GlobalVariable(
        M, DummyInit->getType(), true, GlobalVariable::ExternalLinkage,
        DummyInit, "__dummy.omp_offloading.entry");
    DummyEntry->setSection("omp_offloading_entries");
    DummyEntry->setVisibility(GlobalValue::HiddenVisibility);
>>>>>>> b16d2b48b425e4607eda0421689875d9c8504b75

    auto *Zero = ConstantInt::get(getSizeTTy(), 0u);
    Constant *ZeroZero[] = {Zero, Zero};

    // Create initializer for the images array.
    SmallVector<Constant *, 4u> ImagesInits;
    ImagesInits.reserve(Bufs.size());
    for (ArrayRef<char> Buf : Bufs) {
      auto *Data = ConstantDataArray::get(C, Buf);
      auto *Image = new GlobalVariable(M, Data->getType(), /*isConstant*/ true,
                                       GlobalVariable::InternalLinkage, Data,
                                       ".omp_offloading.device_image");
      Image->setUnnamedAddr(GlobalValue::UnnamedAddr::Global);

      auto *Size = ConstantInt::get(getSizeTTy(), Buf.size());
      Constant *ZeroSize[] = {Zero, Size};

      auto *ImageB = ConstantExpr::getGetElementPtr(Image->getValueType(),
                                                    Image, ZeroZero);
      auto *ImageE = ConstantExpr::getGetElementPtr(Image->getValueType(),
                                                    Image, ZeroSize);

      ImagesInits.push_back(ConstantStruct::get(getDeviceImageTy(), ImageB,
                                                ImageE, EntriesB, EntriesE));
    }

    // Then create images array.
    auto *ImagesData = ConstantArray::get(
        ArrayType::get(getDeviceImageTy(), ImagesInits.size()), ImagesInits);

    auto *Images =
        new GlobalVariable(M, ImagesData->getType(), /*isConstant*/ true,
                           GlobalValue::InternalLinkage, ImagesData,
                           ".omp_offloading.device_images");
    Images->setUnnamedAddr(GlobalValue::UnnamedAddr::Global);

    auto *ImagesB = ConstantExpr::getGetElementPtr(Images->getValueType(),
                                                   Images, ZeroZero);

    // And finally create the binary descriptor object.
    auto *DescInit = ConstantStruct::get(
        getBinDescTy(),
        ConstantInt::get(Type::getInt32Ty(C), ImagesInits.size()), ImagesB,
        EntriesB, EntriesE);

    return new GlobalVariable(M, DescInit->getType(), /*isConstant*/ true,
                              GlobalValue::InternalLinkage, DescInit,
                              ".omp_offloading.descriptor");
  }

  void createRegisterFunction(GlobalVariable *BinDesc) {
    auto *FuncTy = FunctionType::get(Type::getVoidTy(C), /*isVarArg*/ false);
    auto *Func = Function::Create(FuncTy, GlobalValue::InternalLinkage,
                                  ".omp_offloading.descriptor_reg", &M);
    Func->setSection(".text.startup");

    // Get __tgt_register_lib function declaration.
    auto *RegFuncTy = FunctionType::get(Type::getVoidTy(C), getBinDescPtrTy(),
                                        /*isVarArg*/ false);
    FunctionCallee RegFuncC =
        M.getOrInsertFunction("__tgt_register_lib", RegFuncTy);

    // Construct function body
    IRBuilder<> Builder(BasicBlock::Create(C, "entry", Func));
    Builder.CreateCall(RegFuncC, BinDesc);
    Builder.CreateRetVoid();

    // Add this function to constructors.
    // Set priority to 1 so that __tgt_register_lib is executed AFTER
    // __tgt_register_requires (we want to know what requirements have been
    // asked for before we load a libomptarget plugin so that by the time the
    // plugin is loaded it can report how many devices there are which can
    // satisfy these requirements).
    appendToGlobalCtors(M, Func, /*Priority*/ 1);
  }

  void createUnregisterFunction(GlobalVariable *BinDesc) {
    auto *FuncTy = FunctionType::get(Type::getVoidTy(C), /*isVarArg*/ false);
    auto *Func = Function::Create(FuncTy, GlobalValue::InternalLinkage,
                                  ".omp_offloading.descriptor_unreg", &M);
    Func->setSection(".text.startup");

    // Get __tgt_unregister_lib function declaration.
    auto *UnRegFuncTy = FunctionType::get(Type::getVoidTy(C), getBinDescPtrTy(),
                                          /*isVarArg*/ false);
    FunctionCallee UnRegFuncC =
        M.getOrInsertFunction("__tgt_unregister_lib", UnRegFuncTy);

    // Construct function body
    IRBuilder<> Builder(BasicBlock::Create(C, "entry", Func));
    Builder.CreateCall(UnRegFuncC, BinDesc);
    Builder.CreateRetVoid();

    // Add this function to global destructors.
    // Match priority of __tgt_register_lib
    appendToGlobalDtors(M, Func, /*Priority*/ 1);
  }

public:
#if INTEL_CUSTOMIZATION
  void setToolPath(std::string &ToolPath, StringRef ToolName,
                   StringRef FailMessage) {
    // This just needs to be some symbol in the binary; C++ doesn't
    // allow taking the address of ::main however.
    void *P = (void *)(intptr_t)&Help;
    std::string COWPath = sys::fs::getMainExecutable(ToolName.str().c_str(), P);
    if (!COWPath.empty()) {
      auto COWDir = sys::path::parent_path(COWPath);
      ErrorOr<std::string> ToolPathOrErr =
          sys::findProgramByName(ToolName, {COWDir});
      if (ToolPathOrErr) {
        ToolPath = *ToolPathOrErr;
        return;
      }

      // Otherwise, look through PATH environment.
    }

    ErrorOr<std::string> ToolPathOrErr =
        sys::findProgramByName(ToolName);
    if (!ToolPathOrErr) {
      WithColor::warning(errs(), ToolName)
          << "cannot find " << ToolName << "[.exe] in PATH; "
          << FailMessage << ".\n";
      return;
    }

    ToolPath = *ToolPathOrErr;

  }
#endif // INTEL_CUSTOMIZATION
  BinaryWrapper(StringRef Target, StringRef ToolName)
      : M("offload.wrapper.object", C), ToolName(ToolName) {
    M.setTargetTriple(Target);
#if INTEL_CUSTOMIZATION
    setToolPath(Yaml2ObjPath, "yaml2obj", "ELF container cannot be created");
#endif // INTEL_CUSTOMIZATION
    // Look for llvm-objcopy in the same directory, from which
    // clang-offload-wrapper is invoked. This helps OpenMP offload
    // LIT tests.

    // This just needs to be some symbol in the binary; C++ doesn't
    // allow taking the address of ::main however.
    void *P = (void *)(intptr_t)&Help;
    std::string COWPath = sys::fs::getMainExecutable(ToolName.str().c_str(), P);
    if (!COWPath.empty()) {
      auto COWDir = sys::path::parent_path(COWPath);
      ErrorOr<std::string> ObjcopyPathOrErr =
          sys::findProgramByName("llvm-objcopy", {COWDir});
      if (ObjcopyPathOrErr) {
        ObjcopyPath = *ObjcopyPathOrErr;
        return;
      }

      // Otherwise, look through PATH environment.
    }

    ErrorOr<std::string> ObjcopyPathOrErr =
        sys::findProgramByName("llvm-objcopy");
    if (!ObjcopyPathOrErr) {
      WithColor::warning(errs(), ToolName)
          << "cannot find llvm-objcopy[.exe] in PATH; ELF notes cannot be "
             "added.\n";
      return;
    }

    ObjcopyPath = *ObjcopyPathOrErr;
  }

  ~BinaryWrapper() {
    if (TempFiles.empty())
      return;

    StringRef ToolNameRef(ToolName);
    auto warningOS = [ToolNameRef]() -> raw_ostream & {
      return WithColor::warning(errs(), ToolNameRef);
    };

    for (auto &F : TempFiles) {
      if (SaveTemps) {
        warningOS() << "keeping temporary file " << F << "\n";
        continue;
      }

      auto EC = sys::fs::remove(F, false);
      if (EC)
        warningOS() << "cannot remove temporary file " << F << ": "
                    << EC.message().c_str() << "\n";
    }
  }

  const Module &wrapBinaries(ArrayRef<ArrayRef<char>> Binaries) {
    GlobalVariable *Desc = createBinDesc(Binaries);
    assert(Desc && "no binary descriptor");
    createRegisterFunction(Desc);
    createUnregisterFunction(Desc);
    return M;
  }
<<<<<<< HEAD
#if INTEL_CUSTOMIZATION
  // FIXME: move this to llvm/BinaryFormat/ELF.h:
#define NT_INTEL_ONEOMP_OFFLOAD_VERSION 1
#define NT_INTEL_ONEOMP_OFFLOAD_IMAGE_COUNT 2
#define NT_INTEL_ONEOMP_OFFLOAD_IMAGE_AUX 3

  // INTEL_ONEOMP_OFFLOAD_VERSION specifies the offload image
  // structure for Intel oneAPI OpenMP (SPIR-V) offload.
  // We used to use plain SPIR-V images for Intel oneAPI OpenMP offload,
  // and this legacy representation may still be supported by OpenMP
  // offload runtime. Newer toolchain is using ELF container
  // to represent offload image(s) for a single loadable module
  // (i.e. executable or DLL/shared library).
  //
  // Version 1.0:
  //   1. Version 1.0 is identified by ELF note with "INTELONEOMPOFFLOAD"
  //      name and type NT_INTEL_ONEOMP_OFFLOAD_VERSION - the note's descriptor
  //      contains (not null-terminated) string "1.0".
  //   2. The ELF image must have NT_INTEL_ONEOMP_OFFLOAD_IMAGE_COUNT note.
  //      The note's descriptor is a (not null-terminated) string specifying
  //      the integer number of images packed into the ELF image.
  //      The number must be greater than zero.
  //   3. The images may be found in sections named "__openmp_offload_spirv_#",
  //      where '#' is from [0, NT_INTEL_ONEOMP_OFFLOAD_IMAGE_COUNT) interval.
  //   4. The images in these sections are either SPIR-V images or native
  //      images produced by device compilers (e.g. ocloc, etc.).
  //   5. The ELF image may have up to NT_INTEL_ONEOMP_OFFLOAD_IMAGE_COUNT
  //      notes of type NT_INTEL_ONEOMP_OFFLOAD_IMAGE_AUX. Each such note's
  //      descriptor provides auxiliary information for the packed images.
  //      The descriptor has the following structure:
  //        "#\0""<image format>\0""[compile options]\0""[link options]",
  //      where # is from [0, NT_INTEL_ONEOMP_OFFLOAD_IMAGE_COUNT) interval;
  //      <image format> is "0" for "native", and "1" for SPIR-V.
  //      So for each image we can provide two optional strings containing
  //      compilation and linking options for the SPIR-V runtime.
  //      Note that the '\0' is required even when an optional string
  //      is not specified.
  //   6. ELF image for version 1.0 is always a 64-bit little-endian ELF.
  //
  // TODO: we may try to use MsgPack format to have more compact representation
  //       for the auxiliary information. This will require new
  //       NT_INTEL_ONEOMP_OFFLOAD_VERSION identifier (e.g. "1.1" or "2.0").
#define INTEL_ONEOMP_OFFLOAD_VERSION "1.0"

  Expected<StringRef> getTmpFile(Twine NamePattern) {
    Expected<sys::fs::TempFile> TempFile =
        sys::fs::TempFile::create(NamePattern);
    if (!TempFile)
      return TempFile.takeError();

    std::string FileName = TempFile->TmpName;

    if (Error E = TempFile->keep(FileName))
      return std::move(E);

    TempFiles.push_back(std::move(FileName));
    return TempFiles.back();
  }

  // TODO: if this code stays here (vs migrating to the driver), we need to
  //       return Error in case of failure and abort with a fatal error
  //       in the caller.
  void containerizeOpenMPImages() {
    if (Yaml2ObjPath.empty() || ObjcopyPath.empty())
      return;

    // If there are not OpenMP images, there is nothing to do.
    auto OpenMPPackIt = Packs.find(OffloadKind::OpenMP);
    if (OpenMPPackIt == Packs.end())
      return;
    SameKindPack *OpenMPPack = OpenMPPackIt->second.get();

    // Start creating notes for the ELF container.
    std::vector<ELFYAML::NoteEntry> Notes;
    std::string Version = toHex(INTEL_ONEOMP_OFFLOAD_VERSION);
    Notes.emplace_back(ELFYAML::NoteEntry{"INTELONEOMPOFFLOAD",
                                          yaml::BinaryRef(Version),
                                          NT_INTEL_ONEOMP_OFFLOAD_VERSION});

    SmallVector<StringRef, 2> ImageFiles;
    // AuxInfos strings will hold auxiliary information for each image.
    // ELFYAML::NoteEntry structures will hold references to these
    // strings, so we have to make sure the strings are valid up to the point,
    // where YAML is dropped into a file.
    SmallVector<std::string, 2> AuxInfos;
    size_t MaxImagesNum = OpenMPPack->size();
    // To avoid references invalidation, we have to reserve a string
    // for each image. TODO: Should we reserve for ImageFiles and SubSections?
    AuxInfos.reserve(MaxImagesNum);
    // SubSections numbers will hold a number of subsections for each image.
    // They will be used for correct subsection numbering.
    SmallVector<size_t, 2> SubSections;

    // Find SPIR-V images and create notes with auxiliary information
    // for each of them.
    unsigned ImageIdx = 0;
    for (auto *I = OpenMPPack->begin(); I != OpenMPPack->end(); ++I) {
      const BinaryWrapper::Image &Img = *(I->get());
      if (Img.Tgt.find("spir") != 0)
        continue;

      unsigned ImageFmt = 1; // SPIR-V format
      if (Img.Tgt.find("spir64_") == 0 || Img.Tgt.find("spir_") == 0)
        ImageFmt = 0; // native format
      ImageFiles.push_back(Img.File);
      AuxInfos.push_back(toHex((Twine(ImageIdx) + Twine('\0') +
                                Twine(ImageFmt) + Twine('\0') +
                                Img.CompileOpts + Twine('\0') +
                                Img.LinkOpts).str()));
      Notes.emplace_back(ELFYAML::NoteEntry{
          "INTELONEOMPOFFLOAD",
          yaml::BinaryRef(AuxInfos.back()),
          NT_INTEL_ONEOMP_OFFLOAD_IMAGE_AUX});
      SubSections.push_back(Img.SubSections);

      ++ImageIdx;
    }

    // If there are no SPIR-V images, there is nothing to do.
    if (ImageIdx == 0)
      return;

    std::string ImgCount = toHex(Twine(ImageIdx).str());
    Notes.emplace_back(ELFYAML::NoteEntry{"INTELONEOMPOFFLOAD",
                                          yaml::BinaryRef(ImgCount),
                                          NT_INTEL_ONEOMP_OFFLOAD_IMAGE_COUNT});

    StringRef ToolNameRef(ToolName);

    // Helper to emit warnings.
    auto warningOS = [ToolNameRef]() -> raw_ostream & {
      return WithColor::warning(errs(), ToolNameRef);
    };

    // Reserve temporary file for a YAML template that will be used
    // to create an initial ELF image, and a temporary file for
    // the initial ELF image.
    Expected<StringRef> YamlFileName =
        getTmpFile(Output + Twine(".spirv.yamlimage.%%%%%%%.tmp"));
    if (!YamlFileName) {
      logAllUnhandledErrors(YamlFileName.takeError(), warningOS());
      return;
    }
    Expected<StringRef> ImageFileName =
        getTmpFile(Output + Twine(".spirv.elfimage.%%%%%%%.tmp"));
    if (!ImageFileName) {
      logAllUnhandledErrors(ImageFileName.takeError(), warningOS());
      return;
    }

    std::error_code EC;
    raw_fd_stream YamlOS(*YamlFileName, EC);
    if (EC) {
      warningOS() << "cannot open YAML template file " << *YamlFileName << ": "
                  << EC.message() << "\n";
      return;
    }

    // Write YAML template file.
    {
      yaml::Output YamlOut(YamlOS, NULL, 0);

      // We use 64-bit little-endian ELF currently.
      ELFYAML::FileHeader Header{};
      Header.Class = ELF::ELFCLASS64;
      Header.Data = ELF::ELFDATA2LSB;
      Header.Type = ELF::ET_NONE;
      // Do not use any existing machine, otherwise, other plugins
      // may claim this image.
      Header.Machine = ELF::EM_NONE;

      // Create a section with notes.
      ELFYAML::NoteSection Section{};
      Section.Type = ELF::SHT_NOTE;
      Section.Name = ".note.inteloneompoffload";
      Section.Notes.emplace(std::move(Notes));

      ELFYAML::Object Object{};
      Object.Header = Header;
      Object.Chunks.push_back(
          std::make_unique<ELFYAML::NoteSection>(std::move(Section)));

      YamlOut << Object;
      // YamlOut is destructed here causing the YAML write out.
    }
    YamlOS.close();

    if (YamlOS.has_error()) {
      warningOS() << "cannot write YAML template file " << *YamlFileName << ": "
                  << YamlOS.error().message() << "\n";
      YamlOS.clear_error();
      return;
    }

    // Invoke yaml2obj to produce the initial ELF container from the YAML
    // template:
    //   yaml2obj spirv.yamlimage.tmp -o=spirv.elfimage.tmp
    {
      std::vector<StringRef> Args;
      Args.push_back(Yaml2ObjPath);
      Args.push_back(*YamlFileName);
      std::string OutFlag("-o=" + ImageFileName->str());
      Args.push_back(OutFlag);
      bool ExecutionFailed = false;
      std::string ErrMsg;
      (void)sys::ExecuteAndWait(Yaml2ObjPath, Args,
                                /*Env=*/llvm::None, /*Redirects=*/{},
                                /*SecondsToWait=*/0,
                                /*MemoryLimit=*/0, &ErrMsg, &ExecutionFailed);

      if (ExecutionFailed) {
        warningOS() << ErrMsg << "\n";
        return;
      }
    }

    // Invoke objcopy to put each image into its own
    // __openmp_offload_spirv_#[_#] section:
    //   objcopy --add-section=__openmp_offload_spirv_0_0=ImgFile0 \
    //       spirv.elfimage.tmp spirv.elfimage.tmp
    //   objcopy --add-section=__openmp_offload_spirv_0_1=ImgFile1 \
    //       spirv.elfimage.tmp spirv.elfimage.tmp
    //   objcopy --add-section=__openmp_offload_spirv_1=ImgFile2   \
    //       spirv.elfimage.tmp spirv.elfimage.tmp
    //   ...
    unsigned AppIdx = 0;
    unsigned SubImageIdx = 0;
    for (unsigned I = 0; I < ImageIdx; ++I) {
      auto Name = ImageFiles[I];
      auto SubImages = SubSections[I];
      std::vector<StringRef> Args;
      Args.push_back(ObjcopyPath);
      // If there are any subsections in the image, the section name will be
      // in the format of __openmp_offload_spirv_#_#
      std::string Option(("--add-section=__openmp_offload_spirv_" +
                          Twine(AppIdx) +
                          (SubImages > 0 ? "_" + Twine(SubImageIdx) : "") +
                          "=" + Name).str());
      Args.push_back(Option);
      Args.push_back(*ImageFileName);
      Args.push_back(*ImageFileName);
      bool ExecutionFailed = false;
      std::string ErrMsg;
      (void)sys::ExecuteAndWait(ObjcopyPath, Args,
                                /*Env=*/llvm::None, /*Redirects=*/{},
                                /*SecondsToWait=*/0,
                                /*MemoryLimit=*/0, &ErrMsg, &ExecutionFailed);

      if (ExecutionFailed) {
        warningOS() << ErrMsg << "\n";
        return;
      }

      // Every image contains a number of subsection in its parent application,
      // so we should skip this number of images and increment AppIdx just after
      // all the subsections has been processed.
      if (++SubImageIdx >= SubImages) {
        SubImageIdx = 0;
        ++AppIdx;
      }
    }

    // Delete the original Images from the list and replace then
    // with a single combined container ELF.
    for (auto *I = OpenMPPack->begin(); I != OpenMPPack->end();) {
      const BinaryWrapper::Image &Img = *(I->get());
      if (Img.Tgt.find("spir") != 0)
        ++I;
      else
        I = OpenMPPack->erase(I);
    }

    OpenMPPack->emplace_back(std::make_unique<Image>(
        *ImageFileName, /*Manif_=*/"",
        // Use artificial spirv triple to simplify the extraction of the ELF
        // container from the final loadable module. The container will be
        // placed into __CLANG_OFFLOAD_BUNDLE__openmp-spirv section.
        "spirv",
        BinaryImageFormat::none, /*CompileOpts_=*/"", /*LinkOpts_=*/"",
        /*EntriesFile_=*/"", /*PropsFile_=*/""));
  }
#endif // INTEL_CUSTOMIZATION
};
=======
>>>>>>> b16d2b48b425e4607eda0421689875d9c8504b75

  std::unique_ptr<MemoryBuffer> addELFNotes(std::unique_ptr<MemoryBuffer> Buf,
                                            StringRef OriginalFileName) {
    // Cannot add notes, if llvm-objcopy is not available.
    //
    // I did not find a clean way to add a new notes section into an existing
    // ELF file. llvm-objcopy seems to recreate a new ELF from scratch,
    // and we just try to use llvm-objcopy here.
    if (ObjcopyPath.empty())
      return Buf;

    StringRef ToolNameRef(ToolName);

    // Helpers to emit warnings.
    auto warningOS = [ToolNameRef]() -> raw_ostream & {
      return WithColor::warning(errs(), ToolNameRef);
    };
    auto handleErrorAsWarning = [&warningOS](Error E) {
      logAllUnhandledErrors(std::move(E), warningOS());
    };

    Expected<std::unique_ptr<ObjectFile>> BinOrErr =
        ObjectFile::createELFObjectFile(Buf->getMemBufferRef(),
                                        /*InitContent=*/false);
    if (Error E = BinOrErr.takeError()) {
      consumeError(std::move(E));
      // This warning is questionable, but let it be here,
      // assuming that most OpenMP offload models use ELF offload images.
      warningOS() << OriginalFileName
                  << " is not an ELF image, so notes cannot be added to it.\n";
      return Buf;
    }

    // If we fail to add the note section, we just pass through the original
    // ELF image for wrapping. At some point we should enforce the note section
    // and start emitting errors vs warnings.
    support::endianness Endianness;
    if (isa<ELF64LEObjectFile>(BinOrErr->get()) ||
        isa<ELF32LEObjectFile>(BinOrErr->get())) {
      Endianness = support::little;
    } else if (isa<ELF64BEObjectFile>(BinOrErr->get()) ||
               isa<ELF32BEObjectFile>(BinOrErr->get())) {
      Endianness = support::big;
    } else {
      warningOS() << OriginalFileName
                  << " is an ELF image of unrecognized format.\n";
      return Buf;
    }

    // Create temporary file for the data of a new SHT_NOTE section.
    // We fill it in with data and then pass to llvm-objcopy invocation
    // for reading.
    Twine NotesFileModel = OriginalFileName + Twine(".elfnotes.%%%%%%%.tmp");
    Expected<sys::fs::TempFile> NotesTemp =
        sys::fs::TempFile::create(NotesFileModel);
    if (Error E = NotesTemp.takeError()) {
      handleErrorAsWarning(createFileError(NotesFileModel, std::move(E)));
      return Buf;
    }
    TempFiles.push_back(NotesTemp->TmpName);

    // Create temporary file for the updated ELF image.
    // This is an empty file that we pass to llvm-objcopy invocation
    // for writing.
    Twine ELFFileModel = OriginalFileName + Twine(".elfwithnotes.%%%%%%%.tmp");
    Expected<sys::fs::TempFile> ELFTemp =
        sys::fs::TempFile::create(ELFFileModel);
    if (Error E = ELFTemp.takeError()) {
      handleErrorAsWarning(createFileError(ELFFileModel, std::move(E)));
      return Buf;
    }
    TempFiles.push_back(ELFTemp->TmpName);

    // Keep the new ELF image file to reserve the name for the future
    // llvm-objcopy invocation.
    std::string ELFTmpFileName = ELFTemp->TmpName;
    if (Error E = ELFTemp->keep(ELFTmpFileName)) {
      handleErrorAsWarning(createFileError(ELFTmpFileName, std::move(E)));
      return Buf;
    }

    // Write notes to the *elfnotes*.tmp file.
    raw_fd_ostream NotesOS(NotesTemp->FD, false);

    struct NoteTy {
      // Note name is a null-terminated "LLVMOMPOFFLOAD".
      std::string Name;
      // Note type defined in llvm/include/llvm/BinaryFormat/ELF.h.
      uint32_t Type = 0;
      // Each note has type-specific associated data.
      std::string Desc;

      NoteTy(std::string &&Name, uint32_t Type, std::string &&Desc)
          : Name(std::move(Name)), Type(Type), Desc(std::move(Desc)) {}
    };

    // So far we emit just three notes.
    SmallVector<NoteTy, 3> Notes;
    // Version of the offload image identifying the structure of the ELF image.
    // Version 1.0 does not have any specific requirements.
    // We may come up with some structure that has to be honored by all
    // offload implementations in future (e.g. to let libomptarget
    // get some information from the offload image).
    Notes.emplace_back("LLVMOMPOFFLOAD", ELF::NT_LLVM_OPENMP_OFFLOAD_VERSION,
                       OPENMP_OFFLOAD_IMAGE_VERSION);
#if INTEL_CUSTOMIZATION
    Notes.emplace_back("LLVMOMPOFFLOAD", ELF::NT_LLVM_OPENMP_OFFLOAD_PRODUCER,
                       clang::getDPCPPProductName());
    // This is a producer version. Use the same format that is used
    // by clang to report the LLVM version.
    std::string ClangVersion = clang::getClangRevision();
    Notes.emplace_back("LLVMOMPOFFLOAD",
                       ELF::NT_LLVM_OPENMP_OFFLOAD_PRODUCER_VERSION,
                       getDPCPPVersionString() +
                       (ClangVersion.empty() ? "" : (" " + ClangVersion)));
#else // INTEL_CUSTOMIZATION
    // This is a producer identification string. We are LLVM!
    Notes.emplace_back("LLVMOMPOFFLOAD", ELF::NT_LLVM_OPENMP_OFFLOAD_PRODUCER,
                       "LLVM");
    // This is a producer version. Use the same format that is used
    // by clang to report the LLVM version.
    Notes.emplace_back("LLVMOMPOFFLOAD",
                       ELF::NT_LLVM_OPENMP_OFFLOAD_PRODUCER_VERSION,
                       LLVM_VERSION_STRING
#ifdef LLVM_REVISION
                       " " LLVM_REVISION
#endif
    );
#endif // INTEL_CUSTOMIZATION

    // Return the amount of padding required for a blob of N bytes
    // to be aligned to Alignment bytes.
    auto getPadAmount = [](uint32_t N, uint32_t Alignment) -> uint32_t {
      uint32_t Mod = (N % Alignment);
      if (Mod == 0)
        return 0;
      return Alignment - Mod;
    };
    auto emitPadding = [&getPadAmount](raw_ostream &OS, uint32_t Size) {
      for (uint32_t I = 0; I < getPadAmount(Size, 4); ++I)
        OS << '\0';
    };

    // Put notes into the file.
    for (auto &N : Notes) {
      assert(!N.Name.empty() && "We should not create notes with empty names.");
      // Name must be null-terminated.
      if (N.Name.back() != '\0')
        N.Name += '\0';
      uint32_t NameSz = N.Name.size();
      uint32_t DescSz = N.Desc.size();
      // A note starts with three 4-byte values:
      //   NameSz
      //   DescSz
      //   Type
      // These three fields are endian-sensitive.
      support::endian::write<uint32_t>(NotesOS, NameSz, Endianness);
      support::endian::write<uint32_t>(NotesOS, DescSz, Endianness);
      support::endian::write<uint32_t>(NotesOS, N.Type, Endianness);
      // Next, we have a null-terminated Name padded to a 4-byte boundary.
      NotesOS << N.Name;
      emitPadding(NotesOS, NameSz);
      if (DescSz == 0)
        continue;
      // Finally, we have a descriptor, which is an arbitrary flow of bytes.
      NotesOS << N.Desc;
      emitPadding(NotesOS, DescSz);
    }
    NotesOS.flush();

    // Keep the notes file.
    std::string NotesTmpFileName = NotesTemp->TmpName;
    if (Error E = NotesTemp->keep(NotesTmpFileName)) {
      handleErrorAsWarning(createFileError(NotesTmpFileName, std::move(E)));
      return Buf;
    }

    // Run llvm-objcopy like this:
    //   llvm-objcopy --add-section=.note.openmp=<notes-tmp-file-name> \
    //       <orig-file-name> <elf-tmp-file-name>
    //
    // This will add a SHT_NOTE section on top of the original ELF.
    std::vector<StringRef> Args;
    Args.push_back(ObjcopyPath);
    std::string Option("--add-section=.note.openmp=" + NotesTmpFileName);
    Args.push_back(Option);
    Args.push_back(OriginalFileName);
    Args.push_back(ELFTmpFileName);
    bool ExecutionFailed = false;
    std::string ErrMsg;
    (void)sys::ExecuteAndWait(ObjcopyPath, Args,
                              /*Env=*/llvm::None, /*Redirects=*/{},
                              /*SecondsToWait=*/0,
                              /*MemoryLimit=*/0, &ErrMsg, &ExecutionFailed);

    if (ExecutionFailed) {
      warningOS() << ErrMsg << "\n";
      return Buf;
    }

    // Substitute the original ELF with new one.
    ErrorOr<std::unique_ptr<MemoryBuffer>> BufOrErr =
        MemoryBuffer::getFile(ELFTmpFileName);
    if (!BufOrErr) {
      handleErrorAsWarning(
          createFileError(ELFTmpFileName, BufOrErr.getError()));
      return Buf;
    }

    return std::move(*BufOrErr);
  }
};

} // anonymous namespace

int main(int argc, const char **argv) {
  sys::PrintStackTraceOnErrorSignal(argv[0]);

  cl::HideUnrelatedOptions(ClangOffloadWrapperCategory);
  cl::SetVersionPrinter([](raw_ostream &OS) {
    OS << clang::getClangToolFullVersion("clang-offload-wrapper") << '\n';
  });
  cl::ParseCommandLineOptions(
      argc, argv,
      "A tool to create a wrapper bitcode for offload target binaries. Takes "
      "offload\ntarget binaries as input and produces bitcode file containing "
      "target binaries packaged\nas data and initialization code which "
      "registers target binaries in offload runtime.\n");

  if (Help) {
    cl::PrintHelpMessage();
    return 0;
  }

  auto reportError = [argv](Error E) {
    logAllUnhandledErrors(std::move(E), WithColor::error(errs(), argv[0]));
  };
<<<<<<< HEAD
  if (Target.empty()) {
    Target = sys::getProcessTriple();
    if (Verbose)
      errs() << "warning: -" << Target.ArgStr << " option is omitted, using "
             << "host triple '" << Target << "'\n";
  }
=======

>>>>>>> b16d2b48b425e4607eda0421689875d9c8504b75
  if (Triple(Target).getArch() == Triple::UnknownArch) {
    reportError(createStringError(
        errc::invalid_argument, "'" + Target + "': unsupported target triple"));
    return 1;
  }

<<<<<<< HEAD
  // Construct BinaryWrapper::Image instances based on command line args and
  // add them to the wrapper

  BinaryWrapper Wr(Target, argv[0]);
  OffloadKind Knd = OffloadKind::Unknown;
  llvm::StringRef Tgt = "";
  BinaryImageFormat Fmt = BinaryImageFormat::none;
  llvm::StringRef CompileOpts = "";
  llvm::StringRef LinkOpts = "";
  llvm::StringRef EntriesFile = "";
  llvm::StringRef PropsFile = "";
  llvm::SmallVector<llvm::StringRef, 2> CurInputGroup;

  ListArgsSequencer<decltype(Inputs), decltype(Kinds), decltype(Formats),
                    decltype(Targets), decltype(CompileOptions),
                    decltype(LinkOptions), decltype(Entries),
                    decltype(Properties)>
      ArgSeq((size_t)argc, Inputs, Kinds, Formats, Targets, CompileOptions,
             LinkOptions, Entries, Properties);
  int ID = -1;

  do {
    ID = ArgSeq.next();

    // ID != 0 signal that a new image(s) must be added
    if (ID != 0) {
      // create an image instance using current state
      if (CurInputGroup.size() > 2) {
        reportError(
            createStringError(errc::invalid_argument,
                              "too many inputs for a single binary image, "
                              "<binary file> <manifest file>{opt}expected"));
        return 1;
      }
      if (CurInputGroup.size() != 0) {
        if (BatchMode) {
          // transform the batch job (a table of filenames) into a series of
          // 'Wr.addImage' operations for each record in the table
          assert(CurInputGroup.size() == 1 && "1 input in batch mode expected");
          StringRef BatchFile = CurInputGroup[0];
          Expected<std::unique_ptr<util::SimpleTable>> TPtr =
              util::SimpleTable::read(BatchFile);
          if (!TPtr) {
            reportError(TPtr.takeError());
            return 1;
          }
          const util::SimpleTable &T = *TPtr->get();
#if INTEL_CUSTOMIZATION
          StringRef Manif = CurInputGroup.size() > 1 ? CurInputGroup[1] : "";
          const size_t SubSections = T.rows().size();
#endif // INTEL_CUSTOMIZATION

          // iterate via records
          for (const auto &Row : T.rows()) {
#if INTEL_CUSTOMIZATION
            // TODO right numbering for sections (_0_1, _0_2, etc.)
            Wr.addImage(Knd, Row.getCell(COL_CODE),
                        Row.getCell(COL_MANIFEST, Manif), Tgt, Fmt,
                        CompileOpts, LinkOpts,
                        Row.getCell(COL_SYM, EntriesFile),
                        Row.getCell(COL_PROPS, PropsFile),
                        SubSections);
#endif // INTEL_CUSTOMIZATION
          }
        } else {
          if (Knd == OffloadKind::Unknown) {
            reportError(createStringError(errc::invalid_argument,
                                          "offload model not set"));
            return 1;
          }
          StringRef File = CurInputGroup[0];
          StringRef Manif = CurInputGroup.size() > 1 ? CurInputGroup[1] : "";
          Wr.addImage(Knd, File, Manif, Tgt, Fmt, CompileOpts, LinkOpts,
                      EntriesFile, PropsFile);
        }
        CurInputGroup.clear();
      }
=======
  BinaryWrapper Wrapper(Target, argv[0]);

  // Read device binaries.
  SmallVector<std::unique_ptr<MemoryBuffer>, 4u> Buffers;
  SmallVector<ArrayRef<char>, 4u> Images;
  Buffers.reserve(Inputs.size());
  Images.reserve(Inputs.size());
  for (const std::string &File : Inputs) {
    ErrorOr<std::unique_ptr<MemoryBuffer>> BufOrErr =
        MemoryBuffer::getFileOrSTDIN(File);
    if (!BufOrErr) {
      reportError(createFileError(File, BufOrErr.getError()));
      return 1;
>>>>>>> b16d2b48b425e4607eda0421689875d9c8504b75
    }
    std::unique_ptr<MemoryBuffer> Buffer(std::move(*BufOrErr));
    if (File != "-" && AddOpenMPOffloadNotes) {
      // Adding ELF notes for STDIN is not supported yet.
      Buffer = Wrapper.addELFNotes(std::move(Buffer), File);
    }
    const std::unique_ptr<MemoryBuffer> &Buf =
        Buffers.emplace_back(std::move(Buffer));
    Images.emplace_back(Buf->getBufferStart(), Buf->getBufferSize());
  }

  // Create the output file to write the resulting bitcode to.
  std::error_code EC;
  ToolOutputFile Out(Output, EC, sys::fs::OF_None);
  if (EC) {
    reportError(createFileError(Output, EC));
    return 1;
  }

<<<<<<< HEAD
#if INTEL_CUSTOMIZATION
  if (ContainerizeOpenMPImages)
    Wr.containerizeOpenMPImages();
#endif // INTEL_CUSTOMIZATION
  // Create a wrapper for device binaries.
  Expected<const Module *> ModOrErr = Wr.wrap();
  if (!ModOrErr) {
    reportError(ModOrErr.takeError());
    return 1;
  }

#ifndef NDEBUG
  verifyModule(*ModOrErr.get(), &llvm::errs());
#endif

  // And write its bitcode to the file.
  WriteBitcodeToFile(**ModOrErr, Out.os());
=======
  // Create a wrapper for device binaries and write its bitcode to the file.
  WriteBitcodeToFile(
      Wrapper.wrapBinaries(makeArrayRef(Images.data(), Images.size())),
      Out.os());
>>>>>>> b16d2b48b425e4607eda0421689875d9c8504b75
  if (Out.os().has_error()) {
    reportError(createFileError(Output, Out.os().error()));
    return 1;
  }

  // Success.
  Out.keep();
  return 0;
}
