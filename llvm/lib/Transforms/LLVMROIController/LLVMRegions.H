/*BEGIN_LEGAL 
Intel Open Source License 

Copyright (c) 2016-2017 Intel Corporation. All rights reserved.
 
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.  Redistributions
in binary form must reproduce the above copyright notice, this list of
conditions and the following disclaimer in the documentation and/or
other materials provided with the distribution.  Neither the name of
the Intel Corporation nor the names of its contributors may be used to
endorse or promote products derived from this software without
specific prior written permission.
 
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE INTEL OR
ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
END_LEGAL */
#ifndef LLVM_REGIONS_H
#define LLVM_REGIONS_H

/*
   LLVM-regions are specified using a text file with the records of the form:
    comment,thread-id,region-id,start-bbl,start-bbl-count,end-bbl,end-bbl-count,
        region-weight
*/
#include <algorithm>
#include <sstream>
#include <string.h>
#include <cctype>
#include <set>
#include "roidefs.H"

using namespace std;

extern set<UINT32> ROIbbset;
extern struct ROIinfo ROIinfoArray[]; 
extern struct ROIbbinfo ROIbbinfoArray[];
extern map<UINT32,UINT32> bbinfoIndex;
extern UINT32 ROIcount;
extern UINT32 ROIbbcount;
extern UINT32 firstBbId;
extern string firstBbDesc;

namespace CONTROLLER{
class BBREGION
{
    public:
        UINT32 _bbidStart; // read in
        UINT32 _bbidEnd; // read in
        UINT64 _bbcountStart; // read in
        UINT64 _bbcountEnd; // read in
        double _weight; // read in
        string _comment; // read in
        UINT32 _rid; // read in
        UINT32 _tid; // read in
        BBREGION()
        {
            _bbidStart = 0;
            _bbidEnd = 0;
            _bbcountStart = 0;
            _bbcountEnd = 0;
            _weight = 0;
            _rid = 0;
            _tid = 0 ;
        }
        string GetComment() const { return _comment;}
        UINT32 GetRegionId() const { return _rid;}
        UINT64 GetRegionStartBbId() const { return _bbidStart; }
        UINT64 GetRegionEndBbId() const { return _bbidEnd; }
        UINT64 GetRegionStartBbCount() const { return _bbcountStart; }
        UINT64 GetRegionEndBbCount() const { return _bbcountEnd; }
        double GetWeight() const { return _weight;}
};

typedef vector<BBREGION> BBREGION_VECTOR;

class CONTROL_BBREGIONS
{
    private:
    static const UINT32 BUFSIZE=2000;

    public:
    CONTROL_BBREGIONS(string roiFilename, CONTROL_MANAGER* cm)
    {
        _cm = cm;
        _valid = true;
        _maxThreads = MAX_THREADS;
        _regions = new BBREGION_VECTOR[_maxThreads];
        _roiFileName = roiFilename;
    }
    private:
        bool _valid;
        string _roiFileName;
        CONTROLLER::CONTROL_MANAGER* _cm;
    UINT32 StringToUINT32(string &s, const char * name)
    {
       char* end = 0 ;
       INT32 retval = strtoul(s.c_str(), &end, 10); // decimal expected 
       ASSERT((*end == 0), "ERROR reading " + name + " from " + s);
       ASSERT((retval >=0 ), name + " (" + s  + ") must be positive " );
       return (UINT32)retval;
    }

    UINT64 StringToUINT64(string &s, const char * name)
    {
       char* end = 0 ;
       INT64 retval = strtoull(s.c_str(), &end, 10); // decimal expected 
       ASSERT((*end == 0), "ERROR reading " + name + " from " + s);
       ASSERT((retval >=0 ), name + " (" + s  + ") must be positive " );
       return (UINT64)retval;
    }

    double StringToDouble(string &s, const char * name )
    {
       char* end = 0 ;
       double retval = strtod(s.c_str(), &end); 
       ASSERT((*end == 0), "ERROR reading " + name + " from " + s);
       return retval;
    }

    public:
    VOID ReadRegionsFile()
    {
        string filename = _roiFileName;

        ifstream rfile(filename.c_str());

        if (!rfile.is_open())
        {
            cerr << "Could not open regions file " << 
                 _roiFileName << endl;
            exit(-1);
        }

        UINT32 lineNum = 0;
        UINT32 recordNum = 0;
        BBREGION * region = 0;
        while(true)
        {
            if( rfile.eof() )
            {
                break;
            }

            CHAR record[BUFSIZE+1];
            CHAR urecord[BUFSIZE+1];
            string field;

            double t_weight;
            string t_comment;
            INT32 t_rid;
            INT32 t_tid;
            UINT64 t_bbcountStart;
            UINT64 t_bbcountEnd;
            UINT32 t_bbidStart; 
            UINT32 t_bbidEnd; 

            rfile.getline(record, BUFSIZE);
            lineNum++;

            if(strlen(record)==0) continue;

            // Create a temporary record with lower case letters
            for(UINT32 i=0; i <= strlen(record); i++)
                urecord[i] = tolower(record[i]); 

            // first word "comment" : this is the header
            if(strncmp(urecord,"comment",7)==0) continue;

            // first letter '#' : this is a comment 
            if(record[0]=='#') // cheking 'record' not 'urecord'
            {
                if( strncmp(record,"# First Block",13) == 0)
                {
                    //# First Block, 1, main:1:entry
                    istringstream s(record);
                    field.clear();
                    getline(s, field, ','); // '# First Block '
                    field.clear();
                    getline(s, field, ','); // 'bbid'
                    firstBbId = StringToUINT32(field,"firstBbId");
                    cerr << "firstBbId " << firstBbId << endl;
                    field.clear();
                    getline(s, field); // block name
                    firstBbDesc = field;
                    cerr << "firstBbDesc " << firstBbDesc << endl;
                }
            continue;
            }

            istringstream s(record);
            recordNum++;

// comment,thread-id,region-id,start-bbl,start-bbl-count,end-bbl,end-bbl-count,region-weight

            // cerr << "Record # " << recordNum << endl;
            field.clear();
            getline(s, field, ',');
            ASSERT(!field.empty(), "Empty comment field.");
            t_comment = field;
            // cerr << "Comment " << t_comment << endl;

            field.clear();
            getline(s, field, ',');
            ASSERT(!field.empty(), "Empty thread-id field.");
            t_tid = StringToUINT32(field, "thread-id");
            // cerr << "thread-id " << t_tid << endl;

            field.clear();
            getline(s, field, ',');
            ASSERT(!field.empty(), "Empty region-id field.");
            t_rid = StringToUINT32(field, "region-id");
            //cerr << "region-id " << t_rid << endl;

            field.clear();
            getline(s, field, ',');
            ASSERT(!field.empty(), "Empty start-bbl field.");
            t_bbidStart  = StringToUINT32(field, 
                                            "start-bbl");
            ROIbbset.insert(t_bbidStart);

            field.clear();
            getline(s, field, ',');
            ASSERT(!field.empty(), "Empty start-bbl-count field.");
            t_bbcountStart  = StringToUINT64(field, 
                                            "start-bbl-count");
            //cerr << "start-icount " << t_bbcountStart << endl;

            field.clear();
            getline(s, field, ',');
            ASSERT(!field.empty(), "Empty end-bbl field.");
            t_bbidEnd  = StringToUINT32(field, 
                                            "end-bbl");
            ROIbbset.insert(t_bbidEnd);

            field.clear();
            getline(s, field, ',');
            ASSERT(!field.empty(), "Empty end-bbl-count field.");
            t_bbcountEnd  = StringToUINT64(field, 
                                          "end-bblcount");
            //cerr << "end-bbcount " << t_bbcountEnd << endl;

            field.clear();
            getline(s, field, ',');
            ASSERT(!field.empty(), "Empty region-weight field.");
            t_weight  = StringToDouble(field, "region-weight");
            ASSERT((t_weight >= 0), 
                    "region-weight (" + field + ") must be positive" );
            ASSERT((t_weight <= 1), 
                    "region-weight (" + field + ") must be between 0 and 1" );
            //cerr << "region-weight" << t_weight << endl;

            string tail;

            s >> tail;

            if(!tail.empty())
                cerr << "WARNING: regions:in file '" << filename << 
                    "' line number " << dec << lineNum << 
                    ": ignoring fields : " << tail  << endl;

            _regions[t_tid].push_back(BBREGION());
            region = & _regions[t_tid].back();
            region->_comment = t_comment;
            region->_rid = t_rid;
            region->_tid = t_tid;
            region->_weight = t_weight;
            region->_bbidStart = t_bbidStart;
            region->_bbidEnd = t_bbidEnd;
            region->_bbcountStart = t_bbcountStart;
            region->_bbcountEnd = t_bbcountEnd;
        }
        rfile.close();
    }

    VOID PrintRegions()
    {
        cerr << "ROIbbset: " << endl;
        for (set<UINT32>::iterator it=ROIbbset.begin(); it!=ROIbbset.end(); ++it)
            cerr << ' ' << *it;
        cerr << endl;

        for(UINT32 tid=0; tid < _maxThreads; tid++)
        {
            for ( UINT32 i = 0; i < _regions[tid].size(); i++ )
            {
                BBREGION * region = & _regions[tid][i];
                cerr << " comment '" << region->_comment << "'"
                << " rid " << region->_rid
                << " tid " << region->_tid
                << " weight " << region->_weight
                << " bbidStart " << region->_bbidStart
                << " bbcountStart " << region->_bbcountStart
                << " bbidEnd " << region->_bbidEnd
                << " bbcountEnd " << region->_bbcountEnd
                << endl;
            }
        }

        cerr << "ROIinfoArray:" << endl;
        for ( UINT32 i = 1; i <= ROIcount; i++ )
        {
            struct ROIinfo * region = & ROIinfoArray[i];
            cerr << " rid " << i
            << " startBbid " << region->startBbid
            << " startBbCount " << region->startBbCount
            << " endBbid " << region->endBbid
            << " endBbCount " << region->endBbCount
            << " weight " << region->weight
            << endl;
        }

        cerr << "ROIbbinfoArray:" << endl;
        for ( UINT32 i = 1; i <= ROIbbcount; i++ )
        {
            struct ROIbbinfo * bbinfo = & ROIbbinfoArray[i];
            cerr << " bbid " <<  bbinfo->bbid
            << " execcount " << bbinfo->execcount
            << " nextStartingROI " << bbinfo->nextStartingROI
            << " nextEndingROI " << bbinfo->nextEndingROI
            << endl;
        }
    }

    VOID ProcessRegions()
    {
        UINT32 index=1;
        for (set<UINT32>::iterator it=ROIbbset.begin(); it!=ROIbbset.end(); ++it)
        {
            bbinfoIndex[*it] = index;
            ROIbbinfoArray[index] = {0,0,0,0};
            index++;
        }
        ASSERTX((index-1) == ROIbbset.size());
        ROIbbcount = ROIbbset.size();
        ROIcount = 0;
        for(UINT32 tid=0; tid < _maxThreads; tid++)
        {
            for ( UINT32 i = 0; i < _regions[tid].size(); i++ )
            {
                BBREGION * region = & _regions[tid][i];
                UINT32 rid = region->_rid;
                UINT32  bbidStart = region->_bbidStart;
                UINT64  bbcountStart = region->_bbcountStart;
                UINT32  bbidEnd = region->_bbidEnd;
                UINT64  bbcountEnd = region->_bbcountEnd;
                double  weight = region->_weight;
                UINT32 startBbIndex = bbinfoIndex[bbidStart];
                UINT32 endBbIndex = bbinfoIndex[bbidEnd];

                ROIcount++;
                ASSERTX(ROIcount < MAX_ROICOUNT );

                ROIinfoArray[rid].startBbid = bbidStart;
                ROIinfoArray[rid].endBbid = bbidEnd;
                ROIinfoArray[rid].startBbCount = bbcountStart;
                ROIinfoArray[rid].endBbCount = bbcountEnd;
                ROIinfoArray[rid].weight = weight;

                if(ROIbbinfoArray[startBbIndex].bbid == 0)
                    ROIbbinfoArray[startBbIndex].bbid = bbidStart;
                else
                    ASSERTX(ROIbbinfoArray[startBbIndex].bbid == bbidStart);

                if(ROIbbinfoArray[endBbIndex].bbid == 0)
                    ROIbbinfoArray[endBbIndex].bbid = bbidEnd;
                else
                    ASSERTX(ROIbbinfoArray[endBbIndex].bbid == bbidEnd);

                if(ROIbbinfoArray[startBbIndex].nextStartingROI == 0)
                    ROIbbinfoArray[startBbIndex].nextStartingROI = rid;
                else
                {
                    UINT32 otherROI =  ROIbbinfoArray[startBbIndex].nextStartingROI;
                    ASSERTX(ROIinfoArray[otherROI].startBbid == bbidStart); 
                    UINT64 otherROIbbcountStart = ROIinfoArray[otherROI].startBbCount; 
                    if(otherROIbbcountStart > bbcountStart)
                    {
                        // This ROI start fires earlier
                        ROIbbinfoArray[startBbIndex].nextStartingROI = rid;
                    }
                }

                if(ROIbbinfoArray[endBbIndex].nextEndingROI == 0)
                    ROIbbinfoArray[endBbIndex].nextEndingROI = rid;
                else
                {
                    UINT32 otherROI =  ROIbbinfoArray[endBbIndex].nextEndingROI;
                    ASSERTX(ROIinfoArray[otherROI].endBbid == bbidEnd); 
                    UINT64 otherROIbbcountEnd = ROIinfoArray[otherROI].endBbCount; 
                    if(otherROIbbcountEnd > bbcountEnd)
                    {
                        // This ROI end fires earlier
                        ROIbbinfoArray[endBbIndex].nextEndingROI = rid;
                    }
                }
            }
        }
    }

    BBREGION_VECTOR *_regions; // per thread vector containing region info
    bool _active;
    THREADID _maxThreads;
};
}
#endif
