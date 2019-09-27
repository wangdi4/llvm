// INTEL CONFIDENTIAL
//
// Copyright 2006-2018 Intel Corporation.
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

#pragma once

// Uneven/OpenCL partitioner
#include <uneven/blocked_range.h>
#include <uneven/blocked_range2d.h>
#include <uneven/blocked_range3d.h>

#include <tbb/tbb.h>
#include <tbb/task.h>

#include <common_dev_limits.h>

namespace Intel { namespace OpenCL { namespace TaskExecutor {

//
// Represents contiguous range if indices [left,right), including left and excluding right
//
class BlockedRange 
{
public:
    typedef size_t    BlockedRangeValueType;
    typedef size_t    BlockedRangeSizeType;

    BlockedRange() : m_min(0), m_max(0), m_grain(0) {};
        
    BlockedRange( BlockedRangeValueType left, 
                  BlockedRangeValueType right, 
                  BlockedRangeSizeType grain = 1 ) : 
        m_min(left), m_max(right), m_grain(grain) {};

    BlockedRange( const BlockedRange& o ) : 
        m_min(o.m_min), m_max(o.m_max), m_grain(o.m_grain) {};

	BlockedRange(const size_t dims[], size_t grainsize) :
		m_min(0), m_max(dims[0]), m_grain(grainsize) {}

    BlockedRange(const tbb::blocked_range<size_t>& tbb_r) :
        m_min(tbb_r.begin()), m_max(tbb_r.end()), m_grain(tbb_r.grainsize()) {};

    BlockedRange(const tbb::blocked_range<size_t>& tbb_r, BlockedRangeSizeType grain ) :
        m_min(tbb_r.begin()), m_max(tbb_r.end()), m_grain(grain) {}

    BlockedRangeValueType begin() const { return m_min; };
    BlockedRangeValueType end()   const { return m_max; };
    BlockedRangeSizeType  size()  const { return BlockedRangeSizeType( m_max - m_min ); };
    BlockedRangeSizeType  grainsize() const { return m_grain; };

    bool empty()                  const { return !(m_min < m_max); };
    bool is_divisible()           const { return m_grain<size(); };
    
    BlockedRangeValueType left()   const { return m_min; };
    BlockedRangeValueType right()  const { return m_max; };

    // make me the right side of the range, update other to be the left side of the range
    BlockedRange( BlockedRange& o, tbb::split ) : 
        m_min(o.m_min), m_max(o.m_max), m_grain(o.m_grain)
    {
        BlockedRangeValueType middle = m_min + ((m_max - m_min) / 2u);
        m_min = middle;
        o.m_max = middle;
    }

private:
    BlockedRangeValueType m_min;
    BlockedRangeValueType m_max;
    BlockedRangeSizeType  m_grain;
};

//
// Represents contiguous range if 2D indices [left,right), including left and excluding right,
// with priority for one of the for splitting
//
class BlockedRange2d 
{
public:
    BlockedRange2d() : m_coord_split_first(), m_coord_split_second() {};
        
    BlockedRange2d( BlockedRange::BlockedRangeValueType min_coord_split_first, 
                    BlockedRange::BlockedRangeValueType max_coord_split_first, 
                    BlockedRange::BlockedRangeValueType min_coord_split_second, 
                    BlockedRange::BlockedRangeValueType max_coord_split_second, 
                    BlockedRange::BlockedRangeSizeType  grain = 1 ) : 
        m_coord_split_first (min_coord_split_first,max_coord_split_first), 
        m_coord_split_second(min_coord_split_second,max_coord_split_second,grain)  {};

    BlockedRange2d( const BlockedRange2d& o ) : 
        m_coord_split_first(o.m_coord_split_first), m_coord_split_second(o.m_coord_split_second) {};
    
    BlockedRange2d(const tbb::blocked_range<size_t>&      tbb_coord_split_first, 
                   const tbb::blocked_range<size_t>&      tbb_coord_split_second,
                   BlockedRange::BlockedRangeSizeType  grain = 1 ) :
        m_coord_split_first(tbb_coord_split_first, 1), m_coord_split_second(tbb_coord_split_second, grain) {};
    
    // if any is empty - the whole range is empty
    bool empty()                  const { return (m_coord_split_first.empty() || m_coord_split_second.empty()); };
    bool is_divisible()           const { return (m_coord_split_first.is_divisible() || m_coord_split_second.is_divisible()); };

    BlockedRange::BlockedRangeSizeType  grainsize() const { return m_coord_split_second.grainsize(); };

    // make me the right side of the range, update other to be the left side of the range
    // first try to split columns
    BlockedRange2d( BlockedRange2d& o, tbb::split ) : 
        m_coord_split_first(o.m_coord_split_first), m_coord_split_second(o.m_coord_split_second)
    {
        if (m_coord_split_first.size() > 1)
        {
            m_coord_split_first = BlockedRange( o.m_coord_split_first, tbb::split() );
        }
        else
        {
            m_coord_split_second = BlockedRange( o.m_coord_split_second, tbb::split() );
        }
    }
    
protected:
   BlockedRange  m_coord_split_first;
   BlockedRange  m_coord_split_second;
};

//
// Represents contiguous range if 3D indices [left,right), including left and excluding right,
// with priority for one of the for splitting
//
class BlockedRange3d 
{
public:
    BlockedRange3d() : m_coord_split_first(), m_coord_split_second(), m_coord_split_third() {};
        
    BlockedRange3d( BlockedRange::BlockedRangeValueType min_coord_split_first, 
                    BlockedRange::BlockedRangeValueType max_coord_split_first, 
                    BlockedRange::BlockedRangeValueType min_coord_split_second, 
                    BlockedRange::BlockedRangeValueType max_coord_split_second, 
                    BlockedRange::BlockedRangeValueType min_coord_split_third, 
                    BlockedRange::BlockedRangeValueType max_coord_split_third, 
                    BlockedRange::BlockedRangeSizeType  grain = 1 ) : 
        m_coord_split_first (min_coord_split_first,max_coord_split_first), 
        m_coord_split_second(min_coord_split_second,max_coord_split_second), 
        m_coord_split_third (min_coord_split_third,max_coord_split_third,grain)  {};

    BlockedRange3d( const BlockedRange3d& o ) : 
        m_coord_split_first(o.m_coord_split_first), 
        m_coord_split_second(o.m_coord_split_second), 
        m_coord_split_third(o.m_coord_split_third) {};
    
    BlockedRange3d(const tbb::blocked_range<size_t>&      tbb_coord_split_first, 
                   const tbb::blocked_range<size_t>&      tbb_coord_split_second,
                   const tbb::blocked_range<size_t>&      tbb_coord_split_third,
                   BlockedRange::BlockedRangeSizeType  grain = 1 ) :
        m_coord_split_first (tbb_coord_split_first,  1), 
        m_coord_split_second(tbb_coord_split_second, 1),
        m_coord_split_third (tbb_coord_split_third,  grain) {};
    
    // if any is empty - the whole range is empty
    bool empty()                  const 
                { return (m_coord_split_first.empty() || m_coord_split_second.empty() || m_coord_split_third.empty()); };
    bool is_divisible()           const 
                { return (m_coord_split_first.is_divisible() || m_coord_split_second.is_divisible() || m_coord_split_third.is_divisible()); };

    BlockedRange::BlockedRangeSizeType  grainsize() const { return m_coord_split_third.grainsize(); };

    // make me the right side of the range, update other to be the left side of the range
    // first try to split columns
    BlockedRange3d( BlockedRange3d& o, tbb::split ) : 
        m_coord_split_first(o.m_coord_split_first), 
        m_coord_split_second(o.m_coord_split_second), 
        m_coord_split_third(o.m_coord_split_third)
    {
        if (m_coord_split_first.size() > 1)
        {
            m_coord_split_first = BlockedRange( o.m_coord_split_first, tbb::split() );
        } 
        else if (m_coord_split_second.size() > 1)
        {
            m_coord_split_second = BlockedRange( o.m_coord_split_second, tbb::split() );
        }
        else
        {
            m_coord_split_third = BlockedRange( o.m_coord_split_third, tbb::split() );
        }
    }
    
protected:
   BlockedRange  m_coord_split_first;
   BlockedRange  m_coord_split_second;
   BlockedRange  m_coord_split_third;    
};

//
//   Optimize access by Row (split by row)
//

typedef BlockedRange BlockedRangeByRow1d;

class BlockedRangeByRow2d : public BlockedRange2d
{
public:
    BlockedRangeByRow2d() : BlockedRange2d() {};
        
    BlockedRangeByRow2d( BlockedRange::BlockedRangeValueType min_rows, 
                         BlockedRange::BlockedRangeValueType max_rows, 
                         BlockedRange::BlockedRangeValueType min_cols, 
                         BlockedRange::BlockedRangeValueType max_cols, 
                         BlockedRange::BlockedRangeSizeType  grain = 1 ) : 
        BlockedRange2d( min_rows, max_rows, min_cols, max_cols, grain ) {};

    BlockedRangeByRow2d( const BlockedRangeByRow2d& o ) : BlockedRange2d( o ) {}

	BlockedRangeByRow2d(const size_t dims[], size_t grainsize) :
        BlockedRange2d( 0, dims[1], 0, dims[0], grainsize ) {}

    const BlockedRangeByRow1d& rows() const { return m_coord_split_first; };
    const BlockedRangeByRow1d& cols() const { return m_coord_split_second; };

    // make me the right side of the range, update other to be the left side of the range
    // first try to split columns
    BlockedRangeByRow2d( BlockedRangeByRow2d& o, tbb::split ) : BlockedRange2d( o, tbb::split() ) {};
};

class BlockedRangeByRow3d  : public BlockedRange3d
{
public:
    BlockedRangeByRow3d() : BlockedRange3d() {};
        
    BlockedRangeByRow3d( BlockedRange::BlockedRangeValueType min_pages, 
                         BlockedRange::BlockedRangeValueType max_pages, 
                         BlockedRange::BlockedRangeValueType min_rows, 
                         BlockedRange::BlockedRangeValueType max_rows, 
                         BlockedRange::BlockedRangeValueType min_cols, 
                         BlockedRange::BlockedRangeValueType max_cols, 
                         BlockedRange::BlockedRangeSizeType  grain = 1 ) : 
        BlockedRange3d( min_pages,max_pages, min_rows, max_rows, min_cols, max_cols, grain ) {};

    BlockedRangeByRow3d( const BlockedRangeByRow3d& o ) : BlockedRange3d( o ) {};
    
	BlockedRangeByRow3d(const size_t dims[], size_t grainsize) :
        BlockedRange3d( 0, dims[2], 0, dims[1], 0, dims[0], grainsize ) {}

        
    const BlockedRangeByRow1d& pages() const { return m_coord_split_first; };
    const BlockedRangeByRow1d& rows()  const { return m_coord_split_second;  };
    const BlockedRangeByRow1d& cols()  const { return m_coord_split_third;  };

    // make me the right side of the range, update other to be the left side of the range
    // first try to split columns
    BlockedRangeByRow3d( BlockedRangeByRow3d& o, tbb::split ) : BlockedRange3d( o, tbb::split() ) {};
};

//
//   Optimize access by Column (split by column)
//

typedef BlockedRangeByRow1d BlockedRangeByColumn1d;

class BlockedRangeByColumn2d : public BlockedRange2d
{
public:
    BlockedRangeByColumn2d() : BlockedRange2d() {};
        
    BlockedRangeByColumn2d( BlockedRange::BlockedRangeValueType min_rows, 
                            BlockedRange::BlockedRangeValueType max_rows, 
                            BlockedRange::BlockedRangeValueType min_cols, 
                            BlockedRange::BlockedRangeValueType max_cols, 
                            BlockedRange::BlockedRangeSizeType  grain = 1 ) : 
        BlockedRange2d( min_cols, max_cols, min_rows, max_rows, grain ) {};

    BlockedRangeByColumn2d( const BlockedRangeByColumn2d& o ) : BlockedRange2d( o ) {};
    
	BlockedRangeByColumn2d(const size_t dims[], size_t grainsize) :
        BlockedRange2d( 0, dims[0], 0, dims[1], grainsize ) {}

    const BlockedRangeByColumn1d& rows() const { return m_coord_split_second; };
    const BlockedRangeByColumn1d& cols() const { return m_coord_split_first; };

    // make me the right side of the range, update other to be the left side of the range
    // first try to split rows
    BlockedRangeByColumn2d( BlockedRangeByColumn2d& o, tbb::split ) : BlockedRange2d( o, tbb::split() ) {};
};

class BlockedRangeByColumn3d : public BlockedRange3d
{
public:
    BlockedRangeByColumn3d() : BlockedRange3d() {};
        
    BlockedRangeByColumn3d( BlockedRange::BlockedRangeValueType min_pages, 
                            BlockedRange::BlockedRangeValueType max_pages, 
                            BlockedRange::BlockedRangeValueType min_rows, 
                            BlockedRange::BlockedRangeValueType max_rows, 
                            BlockedRange::BlockedRangeValueType min_cols, 
                            BlockedRange::BlockedRangeValueType max_cols, 
                            BlockedRange::BlockedRangeSizeType  grain = 1 ) : 
        BlockedRange3d( min_pages,max_pages, min_cols, max_cols, min_rows, max_rows, grain ) {};

    BlockedRangeByColumn3d( const BlockedRangeByColumn3d& o ) : BlockedRange3d( o ) {};
    
    BlockedRangeByColumn3d(const tbb::blocked_range3d<size_t>& tbb_r) :
        BlockedRange3d( tbb_r.pages(), tbb_r.cols(), tbb_r.rows(), tbb_r.rows().grainsize() ) {};

	BlockedRangeByColumn3d(const size_t dims[], size_t grainsize) :
        BlockedRange3d( 0, dims[2], 0, dims[0], 0, dims[1], grainsize ) {}


    const BlockedRangeByColumn1d& pages() const { return m_coord_split_first;  };
    const BlockedRangeByColumn1d& rows()  const { return m_coord_split_third;  };
    const BlockedRangeByColumn1d& cols()  const { return m_coord_split_second; };

    // make me the right side of the range, update other to be the left side of the range
    // first try to split rows
    BlockedRangeByColumn3d( BlockedRangeByColumn3d& o, tbb::split ) : BlockedRange3d( o, tbb::split() ) {};
};

//
//   Split By Tile
//

typedef BlockedRangeByRow1d BlockedRangeByTile1d;

class BlockedRangeByTile2d 
{
public:
    BlockedRangeByTile2d() : m_rows(), m_cols() {};
        
    BlockedRangeByTile2d( BlockedRange::BlockedRangeValueType min_rows, 
                          BlockedRange::BlockedRangeValueType max_rows, 
                          BlockedRange::BlockedRangeValueType min_cols, 
                          BlockedRange::BlockedRangeValueType max_cols, 
                          BlockedRange::BlockedRangeSizeType  grain = 1 ) : 
        m_rows(min_rows,max_rows,grain), m_cols(min_cols,max_cols,grain)  {};

    BlockedRangeByTile2d( const BlockedRangeByTile2d& o ) : 
        m_rows(o.m_rows), m_cols(o.m_cols) {};
    
	BlockedRangeByTile2d(const size_t dims[], size_t grainsize) :
        m_rows(0, dims[1], grainsize), m_cols(0, dims[0], grainsize) {};

    // if any is empty - the whole range is empty
    bool empty()                  const { return (m_rows.empty() || m_cols.empty()); };
    bool is_divisible()           const { return (m_rows.is_divisible() || m_cols.is_divisible()); };

    const BlockedRangeByRow1d& rows() const { return m_rows; };
    const BlockedRangeByRow1d& cols() const { return m_cols; };

    BlockedRange::BlockedRangeSizeType  grainsize() const { return m_cols.grainsize(); };

    // make me the right side of the range, update other to be the left side of the range
    // first try to split cols
    BlockedRangeByTile2d( BlockedRangeByTile2d& o, tbb::split ) : 
        m_rows(o.m_rows), m_cols(o.m_cols)
    {
        if (m_rows.size() > m_cols.size())
        {
            m_rows = BlockedRange( o.m_rows, tbb::split() );
        }
        else
        {
            m_cols = BlockedRange( o.m_cols, tbb::split() );
        }
    }
    
private:
   BlockedRange  m_rows;
   BlockedRange  m_cols;    
};

class BlockedRangeByTile3d 
{
public:
    BlockedRangeByTile3d() : m_pages(), m_rows(), m_cols() {};
        
    BlockedRangeByTile3d( BlockedRange::BlockedRangeValueType min_pages, 
                          BlockedRange::BlockedRangeValueType max_pages, 
                          BlockedRange::BlockedRangeValueType min_rows, 
                          BlockedRange::BlockedRangeValueType max_rows, 
                          BlockedRange::BlockedRangeValueType min_cols, 
                          BlockedRange::BlockedRangeValueType max_cols, 
                          BlockedRange::BlockedRangeSizeType  grain = 1 ) : 
        m_pages(min_pages,max_pages,grain), m_rows(min_rows,max_rows,grain), m_cols(min_cols,max_cols,grain)  {};

    BlockedRangeByTile3d( const BlockedRangeByTile3d& o ) : 
        m_pages(o.m_pages), m_rows(o.m_rows), m_cols(o.m_cols) {};
    
	BlockedRangeByTile3d(const size_t dims[], size_t grainsize) :
        m_pages(0, dims[2], grainsize), m_rows(0, dims[1], grainsize), m_cols(0, dims[0], grainsize) {};

    // if any is empty - the whole range is empty
    bool empty()                  const { return (m_pages.empty() || m_rows.empty() || m_cols.empty()); };
    bool is_divisible()           const { return (m_pages.is_divisible() || m_rows.is_divisible() || m_cols.is_divisible()); };

    const BlockedRangeByRow1d& pages() const { return m_pages; };
    const BlockedRangeByRow1d& rows()  const { return m_rows;  };
    const BlockedRangeByRow1d& cols()  const { return m_cols;  };

    BlockedRange::BlockedRangeSizeType  grainsize() const { return m_cols.grainsize(); };

    // make me the right side of the range, update other to be the left side of the range
    // first try to split cols
    BlockedRangeByTile3d( BlockedRangeByTile3d& o, tbb::split ) : 
        m_rows(o.m_rows), m_cols(o.m_cols)
    {
        BlockedRange::BlockedRangeSizeType pages_size = m_pages.size();
        BlockedRange::BlockedRangeSizeType rows_size  = m_rows.size();
        BlockedRange::BlockedRangeSizeType cols_size  = m_cols.size();
        
        if ((pages_size >= rows_size) && (pages_size >= cols_size))
        {
            m_pages = BlockedRange( o.m_pages, tbb::split() );
        } 
        else if (rows_size >= cols_size)
        {
            m_rows = BlockedRange( o.m_rows, tbb::split() );
        }
        else
        {
            m_cols = BlockedRange( o.m_cols, tbb::split() );
        }
    }
    
private:
   BlockedRange  m_pages;
   BlockedRange  m_rows;
   BlockedRange  m_cols;    
};

//
//  Just wrapper above default TBB blocked range
//
class BlockedRangeByDefaultTBB1d : public tbb::blocked_range<size_t>
{
public:
    
    BlockedRangeByDefaultTBB1d( size_t left, 
                                size_t right, 
                                size_t grain = 1 ) : 
        tbb::blocked_range<size_t> ( left, right, grain ) {};

    BlockedRangeByDefaultTBB1d( const BlockedRangeByDefaultTBB1d& o ) : 
        tbb::blocked_range<size_t> ( o ) {};
    
	BlockedRangeByDefaultTBB1d(const size_t dims[], size_t grainsize) :
        tbb::blocked_range<size_t> ( 0, dims[0], grainsize ) {};

    // make me the right side of the range, update other to be the left side of the range
    BlockedRangeByDefaultTBB1d( BlockedRangeByDefaultTBB1d& o, tbb::split ) : 
        tbb::blocked_range<size_t> ( o, tbb::split() ) {};
};

class BlockedRangeByDefaultTBB2d : public tbb::blocked_range2d<size_t> 
{
public:
    BlockedRangeByDefaultTBB2d() : tbb::blocked_range2d<size_t> (0,0,0,0) {};
    
    BlockedRangeByDefaultTBB2d( unsigned int min_rows, 
                                unsigned int max_rows, 
                                unsigned int min_cols, 
                                unsigned int max_cols, 
                                size_t grain = 1 ) : 
        tbb::blocked_range2d<size_t> ( min_rows, max_rows, grain, min_cols, max_cols, grain ) {};

    BlockedRangeByDefaultTBB2d( const BlockedRangeByDefaultTBB2d& o ) : 
        tbb::blocked_range2d<size_t> ( o ) {};
    
	BlockedRangeByDefaultTBB2d(const size_t dims[], size_t grainsize) :
        tbb::blocked_range2d<size_t> ( 0, dims[1], grainsize, 0, dims[0], grainsize ) {};

    BlockedRange::BlockedRangeSizeType  grainsize() const { return cols().grainsize(); };

    // make me the right side of the range, update other to be the left side of the range
    BlockedRangeByDefaultTBB2d( BlockedRangeByDefaultTBB2d& o, tbb::split ) : 
        tbb::blocked_range2d<size_t> ( o, tbb::split() ) {};
};

class BlockedRangeByDefaultTBB3d : public tbb::blocked_range3d<size_t> 
{
public:
    BlockedRangeByDefaultTBB3d() : tbb::blocked_range3d<size_t> (0,0,0,0,0,0) {};
    
    BlockedRangeByDefaultTBB3d( unsigned int min_pages, 
                                unsigned int max_pages, 
                                unsigned int min_rows, 
                                unsigned int max_rows, 
                                unsigned int min_cols, 
                                unsigned int max_cols, 
                                size_t grain = 1 ) : 
        tbb::blocked_range3d<size_t> ( min_pages, max_pages, grain, min_rows, max_rows, grain, min_cols, max_cols, grain ) {};

    BlockedRangeByDefaultTBB3d( const BlockedRangeByDefaultTBB3d& o ) : 
        tbb::blocked_range3d<size_t> ( o ) {};
    
	BlockedRangeByDefaultTBB3d(const size_t dims[], size_t grainsize) :
        tbb::blocked_range3d<size_t> ( 0, dims[2], grainsize, 0, dims[1], grainsize, 0, dims[0], grainsize ) {};

    BlockedRange::BlockedRangeSizeType  grainsize() const { return cols().grainsize(); };

    // make me the right side of the range, update other to be the left side of the range
    BlockedRangeByDefaultTBB3d( BlockedRangeByDefaultTBB3d& o, tbb::split ) : 
        tbb::blocked_range3d<size_t> ( o, tbb::split() ) {};
};


//
//  Just wrapper above uneven TBB blocked range
//

class BlockedRangeByUnevenTBB1d : public tbb::uneven::blocked_range<size_t>
{
public:
    BlockedRangeByUnevenTBB1d() : tbb::uneven::blocked_range<size_t> () {};
    
    BlockedRangeByUnevenTBB1d( size_t left, 
                                size_t right, 
                                size_t grain = 1 ) : 
        tbb::uneven::blocked_range<size_t> ( left, right, grain ) {};

    BlockedRangeByUnevenTBB1d( const BlockedRangeByUnevenTBB1d& o ) : 
        tbb::uneven::blocked_range<size_t> ( o ) {};

    BlockedRangeByUnevenTBB1d(const size_t dims[], size_t grainsize) :
        tbb::uneven::blocked_range<size_t> ( 0, dims[0], grainsize ) {};

    // make me the right side of the range, update other to be the left side of the range
    BlockedRangeByUnevenTBB1d( BlockedRangeByUnevenTBB1d& o, const tbb::uneven::split& s) : 
        tbb::uneven::blocked_range<size_t> ( o, s ) {};
};

class BlockedRangeByUnevenTBB2d : public tbb::uneven::blocked_range2d<size_t> 
{
public:
    BlockedRangeByUnevenTBB2d() : tbb::uneven::blocked_range2d<size_t> (0,0,0,0) {};
    
    BlockedRangeByUnevenTBB2d( size_t min_rows, 
                                size_t max_rows, 
                                size_t min_cols, 
                                size_t max_cols, 
                                size_t grain = 1 ) : 
        tbb::uneven::blocked_range2d<size_t> ( min_rows, max_rows, grain, min_cols, max_cols, grain ) {};

    BlockedRangeByUnevenTBB2d( const BlockedRangeByUnevenTBB2d& o ) : 
        tbb::uneven::blocked_range2d<size_t> ( o ) {};
    
    BlockedRangeByUnevenTBB2d(const size_t dims[], size_t grainsize) :
        tbb::uneven::blocked_range2d<size_t> ( 0, dims[1], grainsize, 0, dims[0], grainsize ) {};

    BlockedRange::BlockedRangeSizeType  grainsize() const { return cols().grainsize(); };

    // make me the right side of the range, update other to be the left side of the range
    BlockedRangeByUnevenTBB2d( BlockedRangeByUnevenTBB2d& o, const tbb::uneven::split& s ) : 
        tbb::uneven::blocked_range2d<size_t> ( o, s ) {};
};

class BlockedRangeByUnevenTBB3d : public tbb::uneven::blocked_range3d<size_t> 
{
public:
    BlockedRangeByUnevenTBB3d() : tbb::uneven::blocked_range3d<size_t> (0,0,0,0,0,0) {};
    
    BlockedRangeByUnevenTBB3d( size_t min_pages, 
                                size_t max_pages, 
                                size_t min_rows, 
                                size_t max_rows, 
                                size_t min_cols, 
                                size_t max_cols, 
                                size_t grain = 1 ) : 
        tbb::uneven::blocked_range3d<size_t> ( min_pages, max_pages, grain, min_rows, max_rows, grain, min_cols, max_cols, grain ) {};

    BlockedRangeByUnevenTBB3d( const BlockedRangeByUnevenTBB3d& o ) : 
        tbb::uneven::blocked_range3d<size_t> ( o ) {};
    
    BlockedRangeByUnevenTBB3d(const size_t dims[], size_t grainsize) :
        tbb::uneven::blocked_range3d<size_t> (  0, dims[2], grainsize, 0, dims[1], grainsize, 0, dims[0], grainsize ) {};

    BlockedRange::BlockedRangeSizeType  grainsize() const { return cols().grainsize(); };

    // make me the right side of the range, update other to be the left side of the range
    BlockedRangeByUnevenTBB3d( BlockedRangeByUnevenTBB3d& o, const tbb::uneven::split& s ) : 
        tbb::uneven::blocked_range3d<size_t> ( o, s ) {};
};
}}}

