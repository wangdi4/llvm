//===-- x86_archive.h - OpenCL Archive Interface -----------------===//
//
// Copyright:  (c) 2011 by Apple, Inc., All Rights Reserved.
//
//===----------------------------------------------------------------------===//
//
//
//===----------------------------------------------------------------------===//

#ifndef __X86_ARCHIVE_H
#define __X86_ARCHIVE_H

#include "llvm/Bitcode/Archive.h"
#include "llvm/Support/FormattedStream.h"
#include <utility>
#include <vector>


// FIXME: This should go into llvm archive class so we have derived it
// from there.

// Holds a source that we want to archive.
struct SrcLenStruct { 
  SrcLenStruct(void* _src, unsigned _size) : src(_src), src_size(_size) {}
  SrcLenStruct() : src(NULL), src_size(0) {}
  void* src;          // source
  unsigned src_size;  // source size
};

class CLArchive : public llvm::Archive {
public:
  static void createArchive(std::vector<struct SrcLenStruct> &BCVec,
                            char **log,
                            llvm::formatted_raw_ostream &os);

  static int readArchive(unsigned char *BufPtr, unsigned char *BufEnd,
                         char **log,
                         std::vector<struct SrcLenStruct> &BCVec);

  static bool isArchive(unsigned char *BufPtr, unsigned char *BufEnd);
};



#endif
