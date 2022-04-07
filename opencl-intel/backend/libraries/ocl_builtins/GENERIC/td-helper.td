// Helper func to get a linear strided list of ints.
// e.g.
// Range<0, 4>.Tout == [0, 1, 2, 3]
// Range<1, 4, 2>.Tout == [1, 3]
class Range<int start, int end, int step = 1> {
  list<int> _zeros = !listsplat(0, !sub(end, start));
  list<int> _indices = !foldl([]<int>, _zeros, acc, z, acc # [!add(step, !sub(!size(acc), 1))]);
  list<int> _range = !foreach(x, _indices, !add(x, start));
  list<int> Tout = !filter(x, _range, !lt(x, end));
}

// Helper func to generate shuffle index IR
// e.g.
// JoinIndices<[0, 1, 2]>.Tout == "<i32 0, i32 1, i32 2>"
class JoinIndices<list<int> indices, string index_type = "i32"> {
  string Tout = "<" # !interleave(!foreach(i, indices, index_type # " " # i), ", ") # ">";
}
