//===--- utils/unittest/UnitTestMain/TestMain.cpp - unittest driver -------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "gtest/gtest.h"

uint64_t seedForValidation;

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);

  seedForValidation = 0;

  for( int count = 0; count < argc; count++ )
  {
      std::string strIn(argv[count]); 
      std::string key("--validationSeed=");
      size_t found;

      found = strIn.rfind(key);
      if (found != std::string::npos) {
          size_t length;
          const unsigned int buffLen = 80;
          char buffer[buffLen];

          if(strIn.size() > key.size()) 
          {
              if(buffLen > strIn.size() - key.size())
              {
                  length = strIn.copy(buffer,(strIn.size() - key.size()),key.size());
                  buffer[length]='\0';
                  seedForValidation = atol(buffer);
              }
          }
      }
  }

  return RUN_ALL_TESTS();
}
