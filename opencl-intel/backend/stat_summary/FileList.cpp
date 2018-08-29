// INTEL CONFIDENTIAL
//
// Copyright 2014-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#include <iostream>
#include <string>
#include <vector>

using namespace std;

static bool isIRFileName(const string &name)
{
  if (name.size() < 4)
    return false;

  if (name.compare(name.size()-3, 3, ".ll") == 0)
    return true;

  return false;
}


#if defined (WIN32)
#include <windows.h>

namespace Intel {

bool getIRFileNames(const char *dirName, vector<string> &fileList)
{
   LPWIN32_FIND_DATAA FindFileData = 0;

   HANDLE hFind = FindFirstFileA(dirName, FindFileData);
   if (hFind == INVALID_HANDLE_VALUE) {
     cout << "Error: Can't find directory " << dirName << "\n";
     return false;
   }

   do {
     if (isIRFileName(FindFileData->cFileName) &&
         GetFileAttributesA(FindFileData->cFileName) == FILE_ATTRIBUTE_ARCHIVE)
       fileList.push_back(FindFileData->cFileName);
   } while (FindNextFileA(hFind, FindFileData));

   FindClose(hFind);
   // report if this directory doesn't contain any IR file
   if (fileList.size() == 0) {
     cout << "No IR files found in directory " << dirName << "\n";
     return false;
   }

   return true;
}
}
#else // WIN32


#include <dirent.h>
#include <sys/stat.h>

namespace Intel {

bool getIRFileNames(const char *dirName, vector<string> &fileList)
{
  DIR *dirp = opendir(dirName);
  // report if the given directory is not found
  if (dirp == nullptr) {
    cout << "Error: Can't find directory " << dirName <<"\n";
    return false;
  }

  struct dirent *dp;
  struct stat ir_fstat;
  string fileName(dirName);
  if (*fileName.rbegin() != '/')
    fileName.append("/");
  int size = fileName.size();
  // traverse all files in a directory.
  // if a file ends with .ll and this is a regular file
  // keep it in the file list
  while ((dp = readdir(dirp)) != nullptr) {
    fileName.resize(size);
    fileName.append(dp->d_name);
    if (isIRFileName(fileName.c_str()) &&
      stat(fileName.c_str(), &ir_fstat) == 0 &&
      ir_fstat.st_mode & S_IFREG)
      fileList.push_back(fileName);
  }
  closedir(dirp);

  // report if this directory doesn't contain any IR file
  if (fileList.size() == 0) {
    cout << "No IR files found in directory " << dirName << "\n";
    return false;
  }

  return true;
}
}
#endif // WIN32
