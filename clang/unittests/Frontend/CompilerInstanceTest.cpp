//===- unittests/Frontend/CompilerInstanceTest.cpp - CI tests -------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "clang/Frontend/CompilerInstance.h"
#include "clang/Basic/FileManager.h"
#include "clang/Frontend/CompilerInvocation.h"
#if INTEL_CUSTOMIZATION
#include "llvm/Config/config.h"
#endif // INTEL_CUSTOMIZATION
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Format.h"
#if INTEL_CUSTOMIZATION
#include "llvm/Support/Path.h"
#endif // INTEL_CUSTOMIZATION
#include "llvm/Support/ToolOutputFile.h"
#include "gtest/gtest.h"

using namespace llvm;
using namespace clang;

namespace {

TEST(CompilerInstance, DefaultVFSOverlayFromInvocation) {
  // Create a temporary VFS overlay yaml file.
  int FD;
  SmallString<256> FileName;
  ASSERT_FALSE(sys::fs::createTemporaryFile("vfs", "yaml", FD, FileName));
  ToolOutputFile File(FileName, FD);

  SmallString<256> CurrentPath;
  sys::fs::current_path(CurrentPath);
  sys::fs::make_absolute(CurrentPath, FileName);

  // Mount the VFS file itself on the path 'virtual.file'. Makes this test
  // a bit shorter than creating a new dummy file just for this purpose.
  const std::string CurrentPathStr = std::string(CurrentPath.str());
  const std::string FileNameStr = std::string(FileName.str());
  const char *VFSYaml = "{ 'version': 0, 'roots': [\n"
                        "  { 'name': '%s',\n"
                        "    'type': 'directory',\n"
                        "    'contents': [\n"
                        "      { 'name': 'vfs-virtual.file', 'type': 'file',\n"
                        "        'external-contents': '%s'\n"
                        "      }\n"
                        "    ]\n"
                        "  }\n"
                        "]}\n";
  File.os() << format(VFSYaml, CurrentPathStr.c_str(), FileName.c_str());
  File.os().flush();

  // Create a CompilerInvocation that uses this overlay file.
  const std::string VFSArg = "-ivfsoverlay" + FileNameStr;
  const char *Args[] = {"clang", VFSArg.c_str(), "-xc++", "-"};

  IntrusiveRefCntPtr<DiagnosticsEngine> Diags =
      CompilerInstance::createDiagnostics(new DiagnosticOptions());

  CreateInvocationOptions CIOpts;
  CIOpts.Diags = Diags;
  std::shared_ptr<CompilerInvocation> CInvok =
      createInvocation(Args, std::move(CIOpts));

  if (!CInvok)
    FAIL() << "could not create compiler invocation";
  // Create a minimal CompilerInstance which should use the VFS we specified
  // in the CompilerInvocation (as we don't explicitly set our own).
  CompilerInstance Instance;
  Instance.setDiagnostics(Diags.get());
  Instance.setInvocation(CInvok);
  Instance.createFileManager();

  // Check if the virtual file exists which means that our VFS is used by the
  // CompilerInstance.
  ASSERT_TRUE(Instance.getFileManager().getFile("vfs-virtual.file"));
}

#if INTEL_CUSTOMIZATION
#if !WINDOWS_ONECORE
static std::string LibPath(const std::string Name = "VfsTestLib") {
  const std::vector<testing::internal::string> &Argvs =
      testing::internal::GetArgvs();
  StringRef Argv0 = Argvs.size() > 0 ? Argvs[0] : "FrontendTests";
  llvm::SmallString<256> Buf(llvm::sys::path::parent_path(Argv0));
  llvm::sys::path::append(Buf, (Name + LTDL_SHLIB_EXT));
  return Buf.c_str();
}

TEST(CompilerInstance, VFSOverlayLibrary) {
  // Create a CompilerInvocation that uses this overlay library.
  const std::string VFSArg = "-ivfsoverlay-lib" + LibPath();
  const char *Args[] = {"clang", VFSArg.c_str(), "-xc", "-"};

  IntrusiveRefCntPtr<DiagnosticsEngine> Diags =
      CompilerInstance::createDiagnostics(new DiagnosticOptions());

  std::shared_ptr<CompilerInvocation> CInvok =
      createInvocationFromCommandLine(Args, Diags);

  if (!CInvok)
    FAIL() << "could not create compiler invocation";

  CompilerInstance Instance;
  Instance.setDiagnostics(Diags.get());
  Instance.setInvocation(CInvok);
  Instance.createFileManager();

  // Check if the virtual file exists which means that our VFS is used by the
  // CompilerInstance.
  ASSERT_TRUE(Instance.getFileManager().getFile("virtual.file"));
}
#endif // !WINDOWS_ONECORE
#endif // INTEL_CUSTOMIZATION
TEST(CompilerInstance, AllowDiagnosticLogWithUnownedDiagnosticConsumer) {
  auto DiagOpts = new DiagnosticOptions();
  // Tell the diagnostics engine to emit the diagnostic log to STDERR. This
  // ensures that a chained diagnostic consumer is created so that the test can
  // exercise the unowned diagnostic consumer in a chained consumer.
  DiagOpts->DiagnosticLogFile = "-";

  // Create the diagnostic engine with unowned consumer.
  std::string DiagnosticOutput;
  llvm::raw_string_ostream DiagnosticsOS(DiagnosticOutput);
  auto DiagPrinter = std::make_unique<TextDiagnosticPrinter>(
      DiagnosticsOS, new DiagnosticOptions());
  CompilerInstance Instance;
  IntrusiveRefCntPtr<DiagnosticsEngine> Diags = Instance.createDiagnostics(
      DiagOpts, DiagPrinter.get(), /*ShouldOwnClient=*/false);

  Diags->Report(diag::err_expected) << "no crash";
  ASSERT_EQ(DiagnosticsOS.str(), "error: expected no crash\n");
}

} // anonymous namespace
