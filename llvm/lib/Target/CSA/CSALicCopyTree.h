//===-- CSALicCopyTree.h - CSA helper methods for generating copies. ------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//
// This file defines a data structure to represent a tree of copy
// statements.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_CSA_CSALICCOPYTREE_H
#define LLVM_LIB_TARGET_CSA_CSALICCOPYTREE_H


// No-op for the printf for now.
//
#define CSA_DEBUG_PRINTF(...)
#define CSA_DEBUG_COPY_TREE 0

// Structure to represent a degree-D copy statement.
// 
// The source is in "src".
// The outputs are in "out".
// -1 == %ign for an output.
// 
template <int D>
struct CSALicCopyStmt {
  int src;
  int out[D];

  const static int IGN = -1;

  // Default constructor: create a node with specified src, and all
  // IGN outputs.
  CSALicCopyStmt(int src_) : src(src_) {
    for (int z = 0; z < D; ++z) {
      out[z] = IGN;
    }
  }

#if CSA_DEBUG_COPY_TREE
  void print() {
    std::printf("copy64 ");
    for (int z = 0; z < D; ++z) {
      if (out[z] > 0) {
        std::printf("%d, ",  out[z]);
      }
      else {
        std::printf("%%ign, ");
      }
    }
    std::printf("%d\n", src);
  }
#endif // CSA_DEBUG_COPY_TREE
  
};


template <int D>
struct CSALicCopyTree {
  int m_num_copies;
  int m_leaf_start;
  int m_leaf_stop;
  std::vector<CSALicCopyStmt<D> > m_copy_stmts;

  // This constructor creates a list of copy statements (organized in
  // a complete D-ary tree)
  //
  //  1. num_copies leaves, numbered in the interval
  //      [ leaf_start(), leaf_stop() )
  //
  //  2. leaf_start() internal nodes, numbered 0 to leaf_start()-1.
  //  3. Root at 0.
  //
  // If num_copies < 2, the tree is empty.
  CSALicCopyTree(int num_copies)
    : m_num_copies(num_copies)
    , m_leaf_start(0)
    , m_leaf_stop(0) {

    assert(D>=2);
    generate_copy_tree(m_copy_stmts,
                       m_num_copies,
                       &m_leaf_start,
                       &m_leaf_stop);
  }

  int leaf_start() const {
    return m_leaf_start;
  }
  int leaf_stop() const {
    return m_leaf_stop;
  }

  int num_copy_stmts() const {
    return m_copy_stmts.size();
  }

  const CSALicCopyStmt<D>& get_copy_stmt(int i) {
    assert((i >= 0) && (i < m_copy_stmts.size()));
    return m_copy_stmts[i];
  }
  
  
#if CSA_DEBUG_COPY_TREE
  void print() {
    std::printf("Copy tree: %d copies\n",
                m_num_copies);
    std::printf("Leaves: [%d, %d)\n",
                m_leaf_start,
                m_leaf_stop);
    for (int z = 0; z < m_copy_stmts.size(); ++z) {
      m_copy_stmts[z].print();
    }
    std::printf("\n");
  }
#endif // CSA_DEBUG_COPY_TREE
  
  // Check invariants on the copy tree.
  bool validate() {
    // Count the number of times each number is used as a source and
    // destination in the copy tree.
    std::vector<int> sources;
    std::vector<int> destinations;

    // Check that we have 1 root and the correct number of leaves.
    int num_roots = 0;
    int num_leaves = 0;
    int ign_count = 0;
    
    for (int z = 0; z < m_leaf_stop; ++z) {
      sources.push_back(0);
      destinations.push_back(0);
    }


    for (int i = 0; i < m_copy_stmts.size(); ++i) {
      // Count the source of this copy stmt.
      int src = m_copy_stmts[i].src;
      if ((src < 0) || (src >= m_leaf_stop)) {
        std::printf("ERROR: found src of %d. m_leaf_stop=%d\n",
                    src, m_leaf_stop);
        goto error;
      }
      sources[src]++;

      for (int j = 0; j < D; ++j) {
        int dest = m_copy_stmts[i].out[j];
        if (dest == CSALicCopyStmt<D>::IGN) {
          ign_count++;
        }
        else {
          if ((dest < 0) || (dest > m_leaf_stop)) {
            std::printf("ERROR: found dest of %d. m_leaf_stop=%d\n",
                        dest, m_leaf_stop);
            goto error;
          }
          destinations[dest]++;
        }
      }
    }

    for (int z = 0; z < m_leaf_stop; ++z) {
      int src_count = sources[z];
      int dest_count = destinations[z];

      // No node in a tree should be a source or a destination more
      // than once.
      if ((src_count >= 2) ||(dest_count >=2)) {
        std::printf("ERROR: z = %d, has src_count=%d, dest_count=%d\n",
                    z, src_count, dest_count);
        goto error;
      }

      if (src_count == 0) {
        num_leaves++;
        if (num_leaves > m_num_copies) {
          std::printf("ERROR: Found extra leave %d\n", z);
          goto error;
        }
      }

      if (dest_count == 0) {
        num_roots++;
        if (num_roots > 1) {
          std::printf("ERROR: Found additional root %d\n", z);
          goto error;
        }
      }
    }

    for (int z = m_leaf_start; z < m_leaf_stop; ++z) {
      int src_count = sources[z];
      if (src_count != 0) {
        std::printf("ERROR: Node %d is not a leaf in the tree\n", z);
        goto error;
      }
    }

    // We should never have more than D-1 %ign statements, because (a)
    // only the last internal node in the tree should be incomplete,
    // and (b) it must have at least two outputs, because otherwise it
    // wouldn't a copy.
    if (ign_count >= D-1) {
      std::printf("ERROR: ign_count is %d\n", ign_count);
      goto error;
    }

    std::printf("Validated copy tree with D=%d, %d copies. ign_count=%d, Leaves = [%d, %d)\n",
                D,
                m_num_copies,
                ign_count, 
                m_leaf_start,
                m_leaf_stop);
    return true;
    
  error:
    std::printf("ERROR in tree\n");
#if CSA_DEBUG_COPY_TREE    
    this->print();
#endif    
    return false;
  }


  static
  void generate_copy_tree(std::vector<CSALicCopyStmt<D> >& cstmts,
                          int num_copies,
                          int* leaf_start_ptr,
                          int* leaf_stop_ptr) {
    
    if (num_copies <= 1) {
      cstmts.clear();
      return;
    }

    int num_levels = 0;
    int complete_level_size = 1;
    int num_internal_nodes = 0;

    // Find the number of complete levels in the tree.
    while (complete_level_size * D < num_copies) {
      num_levels++;
      num_internal_nodes += complete_level_size;
      complete_level_size = complete_level_size * D;
    }


    // At this point, complete_level_size is the number of leaves in the last
    // complete level of the tree.
    assert(complete_level_size < num_copies);
    assert(num_copies <= complete_level_size * D);

    CSA_DEBUG_PRINTF("Generating %d copies: num_levels = %d, complete_level_size=%d, num_internal_nodes=%d\n",
                     num_copies,
                     num_levels,
                     complete_level_size,
                     num_internal_nodes);

    // First, generate the largest complete tree that has fewer than
    // "num_copies" leaves.  We will use this tree as the basis for
    // generating the remaining copies.
    //
    // Level x begins with node (D^x-1)/(D-1), and has D^x nodes.
    //
    //  Level 0:     0
    //  Level 1:     1, 2, ... D
    //  Level 2:     D+1, D+2, ... D + D^2
    //   etc.

    int Dpow = 1;  // Stores D^x
    for (int x = 0; x < num_levels; ++x) {
      int start_node = (Dpow-1) / (D-1);
      //      CSA_DEBUG_PRINTF("x = %d, start_node = %d\n", x, start_node);

      // Generate copy statement from source z,
      // to D*z+1, D*z +2, .. D*z + D
      for (int z = start_node; z < start_node + Dpow; ++z) {
        CSALicCopyStmt<D> cstmt(z);
        for (int y = 0; y < D; ++y) {
          cstmt.out[y] = D*z + y + 1;
        }
        cstmts.push_back(cstmt);
      }
      Dpow = Dpow * D;
    }

    // The starting value of the nodes in the last complete level.
    int complete_level_start = (Dpow-1) / (D-1);
    // The starting value of the nodes in the (only) incomplete level.
    int incomplete_level_start = (Dpow*D - 1) / (D-1);

    //  assert(complete_level_size * D >= num_copies);
    CSA_DEBUG_PRINTF("Incomplete tree level: starts at %d. Complete level starts at %d,  complete_level_size=%d\n",
                 incomplete_level_start,
                 complete_level_start,
                 complete_level_size);


    // Let x track the number of nodes on the last complete level of
    // the tree that will feed copy statements.  The remaining nodes
    // on this last complete level will be directly consumed.
    //
    // If there are x nodes feeding copy statements, then the total
    // number of copies available for consumption is at most
    //
    //  D*x + (complete_level_size - x)
    //
    // We will generate all the complete copy statements first,
    // with potentially the last one having ign statements.
    //
    // Add copy statements to the last level, one at a time, until we
    // have enough copies.
    //
    int x = 0;
    int copies_generated = complete_level_size;

    int last_level_counter = incomplete_level_start;
    while (copies_generated < num_copies) {
      // Adding a new copy statement removes a copy from the previous
      // level.
      copies_generated--;

      // Now add in (up to) D children.
      CSALicCopyStmt<D> cstmt(complete_level_start + x);
      int z = 0;
      while ((z < D) && (copies_generated < num_copies)) {
        cstmt.out[z] = last_level_counter;
        // TBD: Technically, we could probably do the math to calculate
        // exactly how many of each copy we need, and eliminate all
        // of these matching counters.
        //
        // But given that we have to build the list of statements anyway,
        // it probably doesn't hurt too badly...
        last_level_counter++;
        copies_generated++;
        z++;
      }

      x++;
      cstmts.push_back(cstmt);
    }

    (*leaf_start_ptr) = complete_level_start + x;
    (*leaf_stop_ptr) = last_level_counter;
    
    CSA_DEBUG_PRINTF("Range of statements: [%d, %d).  Available copies = %d\n",
                     (*leaf_start_ptr),
                     (*leaf_stop_ptr),
                     (*leaf_stop_ptr) - (*leaf_start_ptr));
    assert(((*leaf_stop_ptr) - (*leaf_start_ptr)) == num_copies);
  }
};


#endif // LLVM_LIB_TARGET_CSA_CSALICCOPYTREE_H
