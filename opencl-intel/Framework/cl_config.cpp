
///////////////////////////////////////////////////////////
//  cl_config.cpp
//  Implementation of the configuration class
//  Created on:      10-Dec-2008 8:45:02 AM
//  Original author: ulevy
///////////////////////////////////////////////////////////

#include "cl_config.h"

using std::string;
using namespace Intel::OpenCL::Framework;

ConfigFile::ConfigFile(string filename, string delimiter, string comment, string sentry)
{
	// Construct a ConfigFile, getting keys and values from given file
	m_sDelimiter = delimiter;
	m_sComment = comment;
	m_sSentry = sentry;
	
	cl_err_code clErrRet = ReadFile(filename, (*this));
	if (CL_FAILED(clErrRet))
	{
		Add<string>(CL_CONFIG_LOG_FILE, "C:\\cl.cfg");
		Add<bool>(CL_CONFIG_USE_LOGGER, false);
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


cl_err_code ConfigFile::ReadFile(string fileName, ConfigFile& cf)
{
	// Load a ConfigFile from is
	// Read in keys and values, keeping internal whitespace

	std::fstream is(fileName.c_str());
	if (!is)
	{
		return CL_ERR_FILE_NOT_EXISTS;
	}
	
	typedef string::size_type pos;
	const string& delim  = cf.m_sDelimiter;  // separator
	const string& comm   = cf.m_sComment;    // comment
	const string& sentry = cf.m_sSentry;     // end of file sentry
	const pos skip = delim.length();        // length of separator

	string nextline = "";  // might need to read ahead to see where value ends
	
	while (is || nextline.length() > 0)
	{
		// Read an entire line at a time
		string line;
		if (nextline.length() > 0)
		{
			line = nextline;  // we read ahead; use it now
			nextline = "";
		}
		else
		{
			std::getline(is, line);
		}
		
		// Ignore comments
		line = line.substr(0, line.find(comm));
		
		// Check for end of file sentry
		if (sentry != "" && line.find(sentry) != string::npos)
		{
			return CL_SUCCESS;
		}
		
		// Parse the line if it contains a delimiter
		pos delimPos = line.find( delim );
		if (delimPos < string::npos)
		{
			// Extract the key
			string key = line.substr(0, delimPos);
			line.replace(0, delimPos+skip, "");
			
			// See if value continues on the next line
			// Stop at blank line, next line with a key, end of stream,
			// or end of file sentry
			bool terminate = false;
			while (!terminate && is)
			{
				std::getline(is, nextline);
				terminate = true;
				
				string nlcopy = nextline;
				ConfigFile::trim(nlcopy);
				if(nlcopy == "")
				{
					continue;
				}
				
				nextline = nextline.substr(0, nextline.find(comm));
				if (nextline.find(delim) != string::npos)
				{
					continue;
				}
				if (sentry != "" && nextline.find(sentry) != string::npos)
				{
					continue;
				}
				
				nlcopy = nextline;
				ConfigFile::trim(nlcopy);
				if (nlcopy != "")
				{
					line += "\n";
				}
				line += nextline;
				terminate = false;
			}
			
			// Store key and value
			ConfigFile::trim(key);
			ConfigFile::trim(line);
			cf.m_mapContents[key] = line;  // overwrites if key is repeated
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
