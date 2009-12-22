/////////////////////////////////////////////////////////////////////////
// cl_utils.cpp:
/////////////////////////////////////////////////////////////////////////
// INTEL CONFIDENTIAL
// Copyright 2007-2008 Intel Corporation All Rights Reserved.
//
// The source code contained or described herein and all documents related 
// to the source code ("Material") are owned by Intel Corporation or its 
// suppliers or licensors. Title to the Material remains with Intel Corporation
// or its suppliers and licensors. The Material may contain trade secrets and 
// proprietary and confidential information of Intel Corporation and its 
// suppliers and licensors, and is protected by worldwide copyright and trade 
// secret laws and treaty provisions. No part of the Material may be used, copied, 
// reproduced, modified, published, uploaded, posted, transmitted, distributed, 
// or disclosed in any way without Intel’s prior express written permission. 
//
// No license under any patent, copyright, trade secret or other intellectual
// property right is granted to or conferred upon you by disclosure or delivery 
// of the Materials, either expressly, by implication, inducement, estoppel or 
// otherwise. Any license under such intellectual property rights must be express
// and approved by Intel in writing.
//
// Unless otherwise agreed by Intel in writing, you may not remove or alter this notice 
// or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors 
// in any way.
/////////////////////////////////////////////////////////////////////////

#include "cl_config.h"

using namespace Intel::OpenCL::Utils;
using std::string;

ConfigFile::ConfigFile(string filename, string delimiter, string comment, string sentry)
{
	// Construct a ConfigFile, getting keys and values from given file
	m_sDelimiter = delimiter;
	m_sComment = comment;
	m_sSentry = sentry;

	cl_err_code clErrRet = ReadFile(filename, (*this));
	if (CL_FAILED(clErrRet))
	{
		// set defaults
	}
}


ConfigFile::ConfigFile()
{
	// Construct a ConfigFile without a file; empty
	m_sDelimiter = string(1,'=');
	m_sComment = string(1,'#');
}


void ConfigFile::Remove(const string& key)
{
	// Remove key and its value
	m_mapContents.erase(m_mapContents.find(key));
	return;
}


bool ConfigFile::KeyExists(const string& key) const
{
	// Indicate whether key is found
	mapci p = m_mapContents.find(key);
	return (p != m_mapContents.end());
}

/* static */
int ConfigFile::tokenize(const string & sin, std::vector<string> & tokens)
{
	string s = sin;
	s += char(0);    // add a 0 char for getting end-of-string parsing
	string seps = ",;|";
	seps += char(0);    
	int pos1 = 0;
	int pos2 = 0;
	while ((pos2 = s.find_first_of(seps, pos1)) != string::npos) 
	{
		if (pos2 > pos1)
		{
			string sub = s.substr(pos1, pos2-pos1);
			trim(sub);
			tokens.push_back(sub);
		}
		pos1 = pos2+1;   // don't forget that or you'll get an infinite loop
	}
	return tokens.size();
}


/* static */
void ConfigFile::trim( string& s )
{
	// Remove leading and trailing whitespace
	static const char whitespace[] = " \n\t\v\r\f";
	s.erase(0, s.find_first_not_of(whitespace));
	s.erase(s.find_last_not_of(whitespace) + 1U);
}


cl_err_code ConfigFile::ReadFile(string fileName, ConfigFile& cfg)
{
	std::fstream fsInputStream(fileName.c_str());
	if (!fsInputStream)
	{
		return CL_ERR_FILE_NOT_EXISTS;
	}
	
	string strNextLine = "";
	string line;

	const string& strDelimeter  = cfg.m_sDelimiter;
	
	// get the length of the eparator
	const string::size_type szSepLength = strDelimeter.length();

	const string& strComment = cfg.m_sComment;
	const string& strEOF = cfg.m_sSentry;

	while ( (NULL != fsInputStream) || (strNextLine.length() > 0) )
	{
		// if the next line is not empty, get its content and mark as empty line
		if (strNextLine.length() <= 0)
		{
			std::getline(fsInputStream, line);
		}
		else
		{
			line = strNextLine;
			strNextLine = "";
		}

		// find the first location of the commnet and get the string before it
		string::size_type szCommentPos = line.find(strComment);
		line = line.substr(0, szCommentPos);

		if ( (line.find(strEOF) != string::npos) && (strEOF != "") )
		{
			return CL_SUCCESS;
		}

		bool bFinish = false;
		// getting the first position of the delimeter
		string::size_type szDelimeterPos = line.find(strDelimeter);
		
		// continue only if the position is not the end of line
		if (string::npos > szDelimeterPos)
		{
			string strKey = line.substr(0, szDelimeterPos);
			ConfigFile::trim(strKey);

			string::size_type szPos = szDelimeterPos + szSepLength;
			line.replace(0, szPos, "");

			while ( (false == bFinish) && fsInputStream )
			{
				// get new line
				std::getline(fsInputStream, strNextLine);

				// save the next line
				string strCopy = strNextLine;
				ConfigFile::trim(strCopy);
				
				if(strCopy == "")
				{
					continue;
				}

				bFinish = true;

				strNextLine = strNextLine.substr(0, strNextLine.find(strComment));
				if ( (string::npos != strNextLine.find(strDelimeter)) ||
					 (strEOF != "") && (string::npos != strNextLine.find(strEOF)))
				{
					continue;
				}

				strCopy = strNextLine;
				ConfigFile::trim(strCopy);
				
				if (strCopy != "")
				{
					line += "\n";
				}
				line += strNextLine;
				bFinish = false;
			}

			ConfigFile::trim(line);
			// add the new config param to the container
			cfg.m_mapContents[strKey] = line;
		}
	}
	return CL_SUCCESS;
}

cl_err_code ConfigFile::WriteFile(string fileName, ConfigFile& cf)
{
	std::fstream os(fileName.c_str(), std::ios::out);
	for (ConfigFile::mapci p = cf.m_mapContents.begin(); p != cf.m_mapContents.end(); ++p)
	{
		os << p->first << " " << cf.m_sDelimiter << " ";
		os << p->second << std::endl;
	}
	return CL_SUCCESS;

}


