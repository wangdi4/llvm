#ifndef STRINGUTILS_H
#define STRINGUTILS_H

#include <string>
#include <vector>

namespace Intel { namespace OpenCL { namespace DeviceBackend {
namespace Utils
{

/**
 * Splits the given string using the supplied delimiter
 * populates the given vector of strings
 */
void SplitString( const std::string& s, const char* d, std::vector<std::string>& v );

/**
 * Joins the given strings (as a vector of strings) using
 * the supplied delimiter. 
 * Returns: joined string
 */
std::string JoinStrings( const std::vector<std::string>& vs, const char* d);

} // namespace Utils
}}}

#endif
