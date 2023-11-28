// INTEL CONFIDENTIAL
//
// Copyright 2012 Intel Corporation.
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

#include "ConversionParser.h"
#include <cstdlib>
#include <sstream>

namespace reflection {

//
// Parser automaton
//

class ParserState {
protected:
  std::string m_capture;

public:
  virtual void process(std::istream &cstream);

  virtual State next() const = 0;

  std::string capture() const;

  virtual ~ParserState() = 0;
}; // End ParserState

void ParserState::process(std::istream &cstream) {
  char c;
  cstream.get(c);
  if (cstream.good())
    m_capture += c;
}

ParserState::~ParserState() {}

std::string ParserState::capture() const { return m_capture; }

class Prefix : public ParserState {
  bool m_endPattern;

public:
  Prefix() : m_endPattern(false) {}

  void process(std::istream &cstream) override {
    char c = cstream.peek();
    if ('_' != c)
      ParserState::process(cstream);
    else {
      m_endPattern = true;
      cstream.get();
    }
  }

  State next() const override { return m_endPattern ? TYPENAME : PREFIX; }
};

class TypeName : public ParserState {
  State m_next;

public:
  TypeName() : m_next(TYPENAME) {}

  void process(std::istream &cstream) override {
    char c = cstream.peek();
    if (isdigit(c))
      m_next = TYPELEN;
    else if ('_' == c) {
      cstream.get();
      m_next = POSTFIX;
    } else
      ParserState::process(cstream);
  }

  State next() const override { return m_next; }
};

class TypeLen : public ParserState {
  State m_next;

public:
  TypeLen() : m_next(TYPELEN) {}

  void process(std::istream &cstream) override {
    char c = cstream.peek();
    if (!cstream.good())
      return;
    if (isdigit(c))
      ParserState::process(cstream);
    else {
      assert('_' == c && "invalid conversion format");
      cstream.get();
      m_next = POSTFIX;
    }
  }

  State next() const override { return m_next; }
};

class Postfix : public ParserState {
public:
  State next() const override { return POSTFIX; }
};

ConversionDescriptor::ConversionDescriptor(const std::string &s) : m_width(1) {
  parse(s);
}

int ConversionDescriptor::compare(const ConversionDescriptor &that) const {
  int cmp = m_postfix.compare(that.m_postfix);
  if (0 != cmp)
    return cmp;
  cmp = m_conversionTy.compare(that.m_conversionTy);
  if (0 != cmp)
    return cmp;
  return m_width - that.m_width;
}

void ConversionDescriptor::parse(const std::string &s) {
  ParserState *states[STATES_NUM];
  Prefix prefix;
  TypeName typeName;
  TypeLen typeLen;
  Postfix postfix;
  states[PREFIX] = &prefix;
  states[TYPENAME] = &typeName;
  states[TYPELEN] = &typeLen;
  states[POSTFIX] = &postfix;

  State state = PREFIX;
  std::istringstream cstream(s);
  while (cstream.good()) {
    states[state]->process(cstream);
    state = states[state]->next();
  }
  m_conversionTy = states[TYPENAME]->capture();
  std::string strWidth = states[TYPELEN]->capture();
  m_width = strWidth.empty() ? 1 : atoi(strWidth.c_str());
  m_postfix = states[POSTFIX]->capture();
}

} // end namespace reflection
