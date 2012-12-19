#ifndef __MACH_O_TRIE__
#define __MACH_O_TRIE__

namespace mach_o {
namespace trie {

struct Node;
  
struct Edge {
  Edge(const std::string &s, const Node &n) : fSubString(s) {
    fChild.push_back(n);
  }
  std::string fSubString;
  std::vector<Node> fChild;
};

struct Node {
  Node(const std::string &s) : fCumulativeString(s), fAddress(0), fFlags(0),
    fOrdered(false), fHaveExportInfo(false), fTrieOffset(0) {}
  std::string fCumulativeString;
	std::vector<Edge>	fChildren;
	uint64_t			fAddress;
	uint32_t			fFlags;
	bool				fOrdered;
	bool				fHaveExportInfo;
	uint32_t			fTrieOffset;
	
	void addSymbol(const char* fullStr, uint64_t address, uint32_t flags) {
		const char* partialStr = fullStr + fCumulativeString.size();
		for (std::vector<Edge>::iterator it = fChildren.begin(); it != fChildren.end(); ++it) {
			Edge& e = *it;
			int subStringLen = e.fSubString.size();
			if ( strncmp(e.fSubString.c_str(), partialStr, subStringLen) == 0 ) {
				// already have matching edge, go down that path
				e.fChild.back().addSymbol(fullStr, address, flags);
				return;
			}
			else {
				for (int i=subStringLen-1; i > 0; --i) {
					if ( strncmp(e.fSubString.c_str(), partialStr, i) == 0 ) {
						// found a common substring, splice in new node
						//  was A -> C,  now A -> B -> C
            std::string bNodeCummStr = e.fChild.back().fCumulativeString;
            bNodeCummStr.resize(bNodeCummStr.size() + i - subStringLen);
						//node* aNode = this;
						Node bNode(bNodeCummStr);
            //Node cNode(e.fChild().back());
            std::string abEdgeStr(e.fSubString);
            abEdgeStr.resize(i);
            std::string bcEdgeStr(e.fSubString.c_str() + i);

            Edge bcEdge(bcEdgeStr, e.fChild.back());
						bNode.fChildren.push_back(bcEdge);
						bNode.addSymbol(fullStr, address, flags);

						Edge& abEdge = e;
						abEdge.fSubString = abEdgeStr;
						abEdge.fChild[0] = bNode;
						
						return;
					}
				}
			}
		}
		// no commonality with any existing child, make a new edge that is this whole string
		Node newNode(fullStr);
		newNode.fAddress = address;
		newNode.fFlags = flags;
		newNode.fHaveExportInfo = true;

		Edge newEdge(partialStr, newNode);
		fChildren.push_back(newEdge);
	}
	
	void addOrderedNodes(const char* name, std::vector<Node*>& orderedNodes) {
		if ( !fOrdered ) {
			orderedNodes.push_back(this);
			//fprintf(stderr, "ordered %p %s\n", this, fCummulativeString);
			fOrdered = true;
		}
		const char* partialStr = name + fCumulativeString.size();
		for (std::vector<Edge>::iterator it = fChildren.begin(); it != fChildren.end(); ++it) {
			Edge& e = *it;
			int subStringLen = e.fSubString.size();
			if ( strncmp(e.fSubString.c_str(), partialStr, subStringLen) == 0 ) {
				// already have matching edge, go down that path
				e.fChild.back().addOrderedNodes(name, orderedNodes);
				return;
			}
		}
	}

	// byte for terminal node size in bytes, or 0x00 if not terminal node
	// teminal node (uleb128 flags, uleb128 addr)
	// byte for child node count
	//  each child: zero terminated substring, uleb128 node offset
	bool updateOffset(uint32_t& offset) {
		uint32_t nodeSize = 1; // byte for length of export info
		if ( fHaveExportInfo ) 
			nodeSize += uleb128_size(fFlags) + uleb128_size(fAddress);

		// add children
		++nodeSize; // byte for count of chidren
		for (std::vector<Edge>::iterator it = fChildren.begin(); it != fChildren.end(); ++it) {
			Edge& e = *it;
			nodeSize += e.fSubString.size() + 1 + uleb128_size(e.fChild.back().fTrieOffset);
		}
		bool result = (fTrieOffset != offset);
		fTrieOffset = offset;
		//fprintf(stderr, "updateOffset %p %05d %s\n", this, fTrieOffset, fCummulativeString);
		offset += nodeSize;
		// return true if fTrieOffset was changed
		return result;
	}

	void appendToStream(std::vector<uint8_t>& out) {
		if ( fHaveExportInfo ) {
			// nodes with export info: size, flags, address
			out.push_back(uleb128_size(fFlags) + uleb128_size(fAddress));
			append_uleb128(fFlags, out);
			append_uleb128(fAddress, out);
		}
		else {
			// no export info
			out.push_back(0);
		}
		// write number of children
		out.push_back(fChildren.size());
		// write each child
		for (std::vector<Edge>::iterator it = fChildren.begin(); it != fChildren.end(); ++it) {
			Edge& e = *it;
			append_string(e.fSubString.c_str(), out);
			append_uleb128(e.fChild.back().fTrieOffset, out);
		}
	}
	
private:
	static void append_uleb128(uint64_t value, std::vector<uint8_t>& out) {
		uint8_t byte;
		do {
			byte = value & 0x7F;
			value &= ~0x7F;
			if ( value != 0 )
				byte |= 0x80;
			out.push_back(byte);
			value = value >> 7;
		} while( byte >= 0x80 );
	}
	
	static void append_string(const char* str, std::vector<uint8_t>& out) {
		for (const char* s = str; *s != '\0'; ++s)
			out.push_back(*s);
		out.push_back('\0');
	}
	
	static unsigned int	uleb128_size(uint64_t value) {
		uint32_t result = 0;
		do {
			value = value >> 7;
			++result;
		} while ( value != 0 );
		return result;
	}
	

};

inline uint64_t read_uleb128(const uint8_t*& p, const uint8_t* end) {
	uint64_t result = 0;
	int		 bit = 0;
	do {
		uint64_t slice = *p & 0x7f;
		result |= (slice << bit);
		bit += 7;
	} 
	while (*p++ & 0x80);
	return result;
}

struct Entry
{
	const char*		name;
	uint64_t		address;
	uint64_t		flags;
};


inline void makeTrie(const std::vector<Entry>& input, std::vector<uint8_t>& output)
{
	Node start("");
	
	// make nodes for all exported symbols
	for (std::vector<Entry>::const_iterator it = input.begin(); it != input.end(); ++it) {
		start.addSymbol(it->name, it->address, it->flags);
	}

	// create vector of nodes
	std::vector<Node*> orderedNodes;
	orderedNodes.reserve(input.size()*2);
	for (std::vector<Entry>::const_iterator it = input.begin(); it != input.end(); ++it) {
		start.addOrderedNodes(it->name, orderedNodes);
	}
	
	// assign each node in the vector an offset in the trie stream, iterating until all uleb128 sizes have stabilized
	bool more;
	do {
		uint32_t offset = 0;
		more = false;
		for (std::vector<Node*>::iterator it = orderedNodes.begin(); it != orderedNodes.end(); ++it) {
			if ( (*it)->updateOffset(offset) )
				more = true;
		}
	} while ( more );
	
	// create trie stream
	for (std::vector<Node*>::iterator it = orderedNodes.begin(); it != orderedNodes.end(); ++it) {
		(*it)->appendToStream(output);
	}
}

struct EntryWithOffset
{
	uintptr_t		nodeOffset;
	Entry			entry;
	
	bool operator<(const EntryWithOffset& other) const { return ( nodeOffset < other.nodeOffset ); }
};
}
}

#endif
