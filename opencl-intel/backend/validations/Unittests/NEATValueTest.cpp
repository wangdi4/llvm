#include <gtest/gtest.h>
#include "NEATValue.h"
#include <iostream>
#include <fstream>

using namespace Validation;
using namespace std;

/// this test checks serialization of NEATValue structure into standard streams
/// variable of type NEATValue is created and filled with reference data 
/// then it is written to output stream. 
/// Next it is read from the same stream into new NEATValue variable
/// Then checking we read the same as we written before
TEST(NEATValue, SerializationCheck) {
    
    /// ACCURATE CHECK
    {
        stringstream ref_str(stringstream::in | stringstream::out);
        NEATValue ref;
        ref.SetAccurateVal<float> (1.0f);
        ref_str << ref;                    

        NEATValue val;       
        ref_str >> val;
        EXPECT_EQ(val.IsAcc(), true);
        EXPECT_EQ(*val.GetAcc<float>(), 1.0f);
    }
 
    /// UNKNOWN CHECK
    {
        stringstream ref_str(stringstream::in | stringstream::out);
        NEATValue ref;
        ref.SetStatus(NEATValue::UNKNOWN);
        ref_str << ref;                    

        NEATValue val;       
        ref_str >> val;
        EXPECT_EQ(val.IsUnknown(), true);
    }

    /// ANY CHECK
    {
        stringstream ref_str(stringstream::in | stringstream::out);
        NEATValue ref;
        ref.SetStatus(NEATValue::ANY);
        ref_str << ref;                    

        NEATValue val;       
        ref_str >> val;
        EXPECT_EQ(val.IsAny(), true);
    }
    /// UNWRITTEN CHECK
    {
        stringstream ref_str(stringstream::in | stringstream::out);
        NEATValue ref;
        ref.SetStatus(NEATValue::UNWRITTEN);
        ref_str << ref;                    

        NEATValue val;       
        ref_str >> val;
        EXPECT_EQ(val.IsUnwritten(), true);
    }

    /// INTERVAL CHECK
    {
        stringstream ref_str(stringstream::in | stringstream::out);
        NEATValue ref;
        ref.SetIntervalVal<double>(1.0, 88.34);
        ref_str << ref;                    

        NEATValue val;       
        ref_str >> val;
        EXPECT_EQ(val.IsInterval(), true);
        EXPECT_EQ(*val.GetMin<double>(), 1.0);
        EXPECT_EQ(*val.GetMax<double>(), 88.34);
    }


}

