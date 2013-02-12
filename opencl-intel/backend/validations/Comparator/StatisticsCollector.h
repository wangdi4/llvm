/*****************************************************************************\

Copyright (c) Intel Corporation (2011).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  IStatisticsCollector.h

\*****************************************************************************/


#ifndef __STATISTICSCOLLECTOR_H__
#define __STATISTICSCOLLECTOR_H__

#include "llvm/Support/DataTypes.h"
#include <string>
#include <sstream>
#include <map>
#include "Exception.h"

namespace Validation
{

    class StatisticsCollector
    {
    public:

        /// Type of statistics to gather
        /// COUNT - Count specified string
        /// SUM - compute sum of values
        /// AVG - compute average value
        enum STAT_TYPE {COUNT, SUM, AVG};

        struct StatValue
        {
            double value;
            double res;
            uint64_t count;
            STAT_TYPE type;
        };

        StatisticsCollector() {}
        void UpdateStatistics(STAT_TYPE statType, std::string stat_name, double in_val);
        void CountStatistics(std::string stat_name);

        double GetResult(const std::string& stat_name);

        std::string ToString();

    private:
        void Finalize();
        typedef std::map<std::string, StatValue> StatMap;
        StatMap m_values;
    };

}
#endif // __STATISTICSCOLLECTOR_H__