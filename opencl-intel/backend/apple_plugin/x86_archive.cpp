//===-- x86_archive.cpp - OpenCL Archive Interface -----------------===//
//
// Copyright:  (c) 2011 by Apple, Inc., All Rights Reserved.
//
//===----------------------------------------------------------------------===//
//
//
//===----------------------------------------------------------------------===//

#error Override this file from \\cvcc-ubu-01.iil.intel.com\cvcc\Installations\Apple\ExternalSources\Plugin\
#include "x86_archive.h"

void CLArchive::createArchive(std::vector<struct SrcLenStruct> &BCVec,
                            char **log,
                            llvm::formatted_raw_ostream &os)
{
}

int CLArchive::readArchive(unsigned char *BufPtr, unsigned char *BufEnd,
                         char **log,
                         std::vector<struct SrcLenStruct> &BCVec)
{
    return 1;
}

bool CLArchive::isArchive(unsigned char *BufPtr, unsigned char *BufEnd)
{
    return false;
}
