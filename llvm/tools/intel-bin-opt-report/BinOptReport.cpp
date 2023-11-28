//==- BinOptReport.cpp - Reader tool to parse binary opt-report -*- C++ -*--==//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements a tool that can parse object files containing embedded
// binary optimization report created by Xmain compiler backend for a given
// source file.
//
//===----------------------------------------------------------------------===//

#include "Utils.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/Object/ObjectFile.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/raw_ostream.h"
#include <sstream>

#include "opt_report_proto.pb.h"
#include <google/protobuf/io/gzip_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>

#define DEBUG_TYPE "intel-bor-debug"

using namespace llvm;
using namespace llvm::object;

static const std::string BinOptRptScnName = ".debug_opt_report";

// Mark all our options with this category, everything else (except for -version
// and -help) will be hidden.
static cl::OptionCategory BinOptReportCategory("intel-bin-opt-report options");

static cl::opt<std::string> InputFileName(cl::Positional, cl::desc("<input>"),
                                          cl::init("-"),
                                          cl::cat(BinOptReportCategory));

static cl::opt<std::string> OutputFileName("o", cl::desc("Output file"),
                                           cl::init("-"),
                                           cl::cat(BinOptReportCategory));

// TODO: Implement support for option.
static cl::opt<bool> PrintParsedSection(
    "print-section",
    cl::desc("Print details from the binary opt-report section"),
    cl::init(true), cl::cat(BinOptReportCategory));

namespace llvm {
static void error(Twine Msg) { error(Msg, InputFileName); }
} // namespace llvm

namespace {
// Common structures to represent data format of binary opt-report section's
// data.

// Structure to represent section notification header.
struct NotifyTabHeader {
  char Ident[16];
  uint16_t Version;
  uint16_t HeaderSize;
  uint32_t NumReports;
  uint32_t AnchorIDLen;
  uint32_t AnchorTabOffset;
  uint32_t AnchorTabSize;
  uint32_t PBMsgOffset;
  uint32_t PBMsgSize;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dump(raw_ostream &OS) const {
    OS << "\n==== Header Dump ====\n";
    OS << "Container header:\n";
    OS << "Container version:\t" << ITT_TABLE_VERSION_MAJOR(Version) << "."
       << ITT_TABLE_VERSION_MINOR(Version) << "\n";
    OS << "Header size:\t\t" << HeaderSize << "\n";
    OS << "Number of reports:\t" << NumReports << "\n";
    OS << "Anchor ID length:\t" << AnchorIDLen << "\n";
    OS << "Anchor table offset:\t" << AnchorTabOffset << "\n";
    OS << "Anchor table size:\t" << AnchorTabSize << "\n";
    OS << "Protobuf msg offset:\t" << PBMsgOffset << "\n";
    OS << "Protobuf msg size:\t" << PBMsgSize << "\n";
  }
#endif // !NDEBUG || LLVM_ENABLE_DUMP
};

// Structure to represent anchor table entry.
struct AnchorTabEntry {
  char AnchorID[32] = {0};
  uint64_t AnchorAddr = 0;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dump(raw_ostream &OS) const {
    OS << "Anchor ID:\t" << StringRef(AnchorID, sizeof(AnchorID)) << "\t---->\t"
       << AnchorAddr << "\n";
  }
#endif // !NDEBUG || LLVM_ENABLE_DUMP
};

// Structure to capture overall binary opt-report section's data after parsing.
struct BinOptRptScnData {
  NotifyTabHeader Header;
  StringMap<uint64_t> AnchorTab;
  std::string PBMsgStream;
};

// Multiple .debug_opt_report sections can be sticthed together by linker in
// non-LTO mode. Hence we use a vector to collect parsed data from all these
// sub-sections.
SmallVector<BinOptRptScnData, 4> ParsedSubScns;
} // anonymous namespace

static section_iterator getBinOptReportSection(ObjectFile *Obj) {
  // Loop through all sections in the object file until first binary opt-report
  // section is encountered. TODO: Handle multiple such sections (COMDAT
  // functions?).
  for (const SectionRef &Section : Obj->sections()) {
    Expected<StringRef> NameOrErr = Section.getName();
    if (!NameOrErr)
      error(NameOrErr.takeError(), InputFileName);
    StringRef &ScnName = NameOrErr.get();

    LLVM_DEBUG(dbgs() << "Visiting section " << ScnName << "\n");

    if (ScnName == BinOptRptScnName) {
      LLVM_DEBUG(dbgs() << BinOptRptScnName << " section found\n");
      return {Section};
    }
  }

  return Obj->section_end();
}

static void parseSection(section_iterator ScnIt) {
  ParsedSubScns.clear();
  Expected<StringRef> ContentOrErr = ScnIt->getContents();
  if (!ContentOrErr)
    error(ContentOrErr.takeError(), InputFileName);
  ArrayRef<uint8_t> Bytes = arrayRefFromStringRef(ContentOrErr.get());
  LLVM_DEBUG(dbgs() << "Section size: " << ScnIt->getSize() << "\n");
  LLVM_DEBUG(dbgs() << "Num bytes: " << Bytes.size() << "\n");
  unsigned BinOptRptScnSize = Bytes.size();

  // Iterator to track starting bytes of current sub-section.
  auto CurrSubScn = Bytes.begin();

  while (CurrSubScn != Bytes.end()) {
    LLVM_DEBUG(dbgs() << "\n==== " + BinOptRptScnName +
                             " Section Dump Begin ====\n");
    // Start parsing data collected from .debug_opt_report section.
    BinOptRptScnData ParsedData;

    /**** Parse notify table header ****/

    NotifyTabHeader Header;
    // NOTE: Simple memcpy can be used here since NotifyTabHeaderTy is a simple
    // struct with POD type members.
    std::memcpy(&Header, &*CurrSubScn, sizeof(NotifyTabHeader));

    // Sanity checks for header contents.
    // a. Verify header size field.
    if (Header.HeaderSize != sizeof(NotifyTabHeader))
      error("Bad or corrupted notify table header.");

    // b. Verify anchor table fields.
    if (Header.AnchorTabOffset + Header.AnchorTabSize > BinOptRptScnSize)
      error("Bad or corrupted notify table segment: anchor table goes beyond "
            "section size.");

    // c. Verify Protobuf message fields.
    if (Header.PBMsgOffset + Header.PBMsgSize > BinOptRptScnSize)
      error(
          "Bad or corrupted notify table segment: protobuf message goes beyond "
          "section size.");

    // Print the header.
    LLVM_DEBUG(Header.dump(dbgs()));
    // Populate header in outgoing parsed data.
    ParsedData.Header = Header;

    /**** Parse anchor table entries ****/

    // Sanity checks for anchor table.
    // a. Anchor ID's length should be between 1-32.
    if (Header.AnchorIDLen <= 0 || Header.AnchorIDLen > 32)
      error("Bad or corrupted anchor ID length.");

    // b. Anchor table size should be a mutiple of entry struct size.
    if (Header.AnchorTabSize % sizeof(AnchorTabEntry) != 0)
      error("Bad or corrupted anchor table.");

    // c. Number of entries in anchor table should be same as number of reports.
    if (Header.AnchorTabSize / sizeof(AnchorTabEntry) != Header.NumReports)
      error(
          "Bad or corrupted anchor table: number of entries don't match number "
          "of reports.");

    LLVM_DEBUG(dbgs() << "\n==== Anchor Table Dump ====\n");
    for (unsigned I = 0; I < Header.NumReports; ++I) {
      AnchorTabEntry Entry;
      size_t AnchorTabEntrySize =
          Header.AnchorIDLen + sizeof(AnchorTabEntry::AnchorAddr);
      // Compute offset of each entry in anchor table from top of the table.
      unsigned EntryOffset = Header.AnchorTabOffset + (I * AnchorTabEntrySize);
      // Iterators to point to begin and end of anchor ID for each entry.
      auto EntryAnchorIDBegin = CurrSubScn + EntryOffset;
      auto EntryAnchorIDEnd = EntryAnchorIDBegin + Header.AnchorIDLen;
      std::string AnchorID(EntryAnchorIDBegin, EntryAnchorIDEnd);
      std::strncpy(Entry.AnchorID, AnchorID.c_str(), sizeof(Entry.AnchorID));
      // Compute anchor address offset for current entry.
      unsigned EntryAnchorAddrOffset = EntryOffset + Header.AnchorIDLen;
      std::memcpy(&Entry.AnchorAddr, &*CurrSubScn + EntryAnchorAddrOffset,
                  sizeof(AnchorTabEntry::AnchorAddr));
      // Print the anchor table entry.
      LLVM_DEBUG(Entry.dump(dbgs()));

      // Populate entry in outgoing parsed data.
      assert(ParsedData.AnchorTab.count(
                 StringRef(Entry.AnchorID, sizeof(Entry.AnchorID))) == 0 &&
             "Repeating anchor IDs found in anchor table.");
      ParsedData.AnchorTab[StringRef(Entry.AnchorID, sizeof(Entry.AnchorID))] =
          Entry.AnchorAddr;
    }

    /**** Parse Protobuf message entry ****/
    LLVM_DEBUG(dbgs() << "\n==== Protobuf Message Dump ====\n");
    auto PBMsgBegin = CurrSubScn + Header.PBMsgOffset;
    auto PBMsgEnd = PBMsgBegin + Header.PBMsgSize;
    std::string PBMsg(PBMsgBegin, PBMsgEnd);
    assert(PBMsg.size() == Header.PBMsgSize &&
           "Protobuf message parsed from message has incorrect size.");
    LLVM_DEBUG(dumpRawByteStream(PBMsg, dbgs()));
    ParsedData.PBMsgStream = PBMsg;

    ParsedSubScns.push_back(ParsedData);

    LLVM_DEBUG(dbgs() << "\n==== .debug_opt_report Section Dump End ====\n");

    // Update to start parsing the next sub-section.
    CurrSubScn = PBMsgEnd;
  }
}

static opt_report_proto::BinOptReport deserializeMsg(std::string &PBMsg) {
  // Sanity check if Protobuf is available and versions match.
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  assert(!PBMsg.empty() && "Decoded Protobuf message is empty.");

  LLVM_DEBUG(dbgs() << "Protobuf message (size: " << PBMsg.size()
                    << " bytes):\n"
                    << PBMsg << "\n");

  // Parse binary stream to Protobuf-based binary opt-report object.
  opt_report_proto::BinOptReport BOR;

  std::istringstream MsgStrStream(PBMsg);
  google::protobuf::io::IstreamInputStream ISS(&MsgStrStream);
  google::protobuf::io::GzipInputStream GIS(&ISS);

  if (BOR.ParseFromZeroCopyStream(&GIS)) {
    if (GIS.ZlibErrorCode() <= 0) {
      StringRef ZlibErrorMsg(GIS.ZlibErrorMessage());
      error("Protobuf error: Failed to decompress protobuf message (zlib "
            "error: " +
            ZlibErrorMsg + ")");
    }
  } else {
    error("Protobuf error: Failed to parse binary message stream.");
  }

  return BOR;
}

static void writeReport(const opt_report_proto::BinOptReport &BOR) {
  // Attempt to open output file for writing.
  std::error_code EC;
  llvm::raw_fd_ostream OS(OutputFileName, EC, llvm::sys::fs::OF_TextWithCRLF);
  if (EC)
    error("Can't open file " + OutputFileName + ": " + EC.message() + "\n");

  auto printProtoLoopOptReport =
      [&OS](const opt_report_proto::BinOptReport::LoopOptReport &LOR) {
        OS << "\n=== Loop Begin ===\n";
        OS << "Anchor ID: " << LOR.anchor_id() << "\n";
        OS << "Number of remarks: " << LOR.remarks_size() << "\n";
        for (int I = 0; I < LOR.remarks_size(); ++I) {
          const opt_report_proto::BinOptReport::Remark &R = LOR.remarks(I);
          OS << "  Property: "
             << opt_report_proto::BinOptReport::Property_Name(R.prop_id());
          OS << ", Remark ID: " << R.remark_id();
          OS << ", Remark Args: ";
          for (auto J = 0; J < R.args_size(); ++J) {
            const opt_report_proto::BinOptReport::Arg &Arg = R.args(J);
            if (Arg.has_str_arg())
              OS << Arg.str_arg().value() << " ";
            else if (Arg.has_int32_arg())
              OS << Arg.int32_arg().value() << " ";
          }
          OS << "\n";
        }
        OS << "==== Loop End ====\n";
      };

  OS << "\n--- Start Intel Binary Optimization Report ---\n";
  OS << "Version: " << BOR.major_version() << "." << BOR.minor_version()
     << "\n";
  OS << "Property Message Map:\n";
  for (auto &MapIt : BOR.property_msg_map()) {
    assert(opt_report_proto::BinOptReport::Property_IsValid(MapIt.first) &&
           "Invalid binary opt-report property.");
    OS << "  " << opt_report_proto::BinOptReport::Property_Name(MapIt.first)
       << " --> " << MapIt.second << "\n";
  }
  OS << "Number of reports: " << BOR.opt_reports_size() << "\n";
  for (int I = 0; I < BOR.opt_reports_size(); ++I) {
    const opt_report_proto::BinOptReport::LoopOptReport &LOR =
        BOR.opt_reports(I);
    printProtoLoopOptReport(LOR);
  }

  OS << "\n--- End Intel Binary Optimization Report ---\n\n";
}

int main(int argc, const char **argv) {
  InitLLVM X(argc, argv);

  cl::HideUnrelatedOptions(BinOptReportCategory);
  cl::ParseCommandLineOptions(
      argc, argv,
      "A tool to read binary optimization report embedded in object files "
      "created by Xmain compiler backend.\n");

  // Attempt to open the input binary file for reading.
  Expected<OwningBinary<Binary>> BinaryOrErr = createBinary(InputFileName);
  if (!BinaryOrErr)
    error(BinaryOrErr.takeError(), InputFileName);
  Binary &Bin = *BinaryOrErr.get().getBinary();

  auto *Obj = dyn_cast<ObjectFile>(&Bin);
  if (!Obj)
    error("unsupported object file format");

  section_iterator BORScnIt = getBinOptReportSection(Obj);
  if (BORScnIt == Obj->section_end())
    error(BinOptRptScnName + " section was not found in input");

  parseSection(BORScnIt);
  if (ParsedSubScns.empty())
    error("Unable to parse .debug_opt_report section");

  for (auto &ParsedScn : ParsedSubScns) {
    opt_report_proto::BinOptReport BOR = deserializeMsg(ParsedScn.PBMsgStream);

    writeReport(BOR);
  }

  return 0;
}
