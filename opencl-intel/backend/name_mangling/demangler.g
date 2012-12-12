header "pre_include_hpp" {
#include <cstdio>
#include <cstdlib>
#include <stack>
#include "Type.h"
#include "antlr/ParserSharedInputState.hpp"
#include "antlr/TokenBuffer.hpp"
#include "DemangleLexer.hpp"
}

header "post_include_hpp"{
typedef std::stack<reflection::Type*> TypeStack;
namespace namemangling{class DemangleLexer;}
}

options {
	language="Cpp";
  namespace = "namemangling";
	genHashLines = true;		// include line number informatio
}

{
  static reflection::Type* createType(antlr::RefToken t){
    using namespace reflection;
    using namespace reflection::primitives;
    return new Type(static_cast<Primitive>( t->getType()) );
  }
}

class DemangleParser extends Parser;
options {
//	buildAST = true;			// uses CommonAST by default
}

{
private:
TypeStack m_stack;
reflection::TypeCallback* m_cb;
DemangleLexer* m_pLexer;

public:
void setCallback(reflection::TypeCallback* cb){m_cb = cb;}
void setLexer(DemangleLexer* plexer){m_pLexer = plexer;}
}

demangle
	:	(type)* {
    while (!m_stack.empty()){
      reflection::Type* t = m_stack.top();
      m_stack.pop();
      (*m_cb)(t);
      delete t;
    }
  }
;

type
{reflection::Type* pType = NULL;}
  : pType = primitive_type {m_stack.push(pType);}
  | pType = vector_type    {m_stack.push(pType);}
  | pType = pointer_type   {m_stack.push(pType);}
  | pType = dup_type       {m_stack.push(pType);}
  | pType = user_defined_type {m_stack.push(pType);}
;

primitive_type returns [reflection::Type* pType = NULL;]
  : pType = primitive
;

primitive returns [reflection::Type* pType = NULL;]
  : b:BOOL    {pType = createType(b); }
  | uc:UCHAR  {pType = createType(uc);}
  | c:CHAR    {pType = createType(c); }
  | us:USHOR  {pType = createType(us);}
  | s:SHORT   {pType = createType(s); }
  | ui:UINT   {pType = createType(ui);}
  | i:INT     {pType = createType(i); }
  | ul:ULONG  {pType = createType(ul);}
  | l:LONG    {pType = createType(l); }
  | h:HALF    {pType = createType(h); }
  | f:FLOAT   {pType = createType(f); }
  | d:DOUBLE  {pType = createType(d); }
  | v:VOID    {pType = createType(v); }
;

vector_type returns [reflection::Vector* pVector = NULL;]
  {reflection::Type* pPrimitiveType;}
  : VECTOR_PREFIX n:NUMBER UNDERSCORE pPrimitiveType = primitive{
    int vlen = atoi(n->getText().c_str());
    pVector = new reflection::Vector(pPrimitiveType, vlen);
    delete pPrimitiveType;
  }
;

pointer_type returns [reflection::Pointer* ret = NULL;]
  {std::string addressSpace; std::string cvQualifier;}
  :
    POINTER_PREFIX
    (cvQualifier=cv_qualifier)?
    (addressSpace=address_space)?
    type{
      assert(!m_stack.empty() && "stack should not be empty!");
      reflection::Type* t = m_stack.top();
      m_stack.pop();
      ret = new reflection::Pointer(t);
      delete t;
      if ( !addressSpace.empty() )
        ret->addAttribute(addressSpace);
      if ( !cvQualifier.empty() )
        ret->addAttribute(cvQualifier);
    }
;

dup_type returns [reflection::Type* ret = NULL;]
  :
  PARAM_DUP_PREFIX (NUMBER)? UNDERSCORE {
    assert(!m_stack.empty() && "stack should not be empty!");
    ret = m_stack.top()->clone();
  }
;

user_defined_type returns [reflection::Type* ret = NULL;]
  :
  n:NUMBER{
    unsigned int namelen = atoi(n->getText().c_str());
    std::string typeName = m_pLexer->readN(namelen);
    ret = new reflection::UserDefinedTy(typeName);
  }
;


cv_qualifier returns [std::string s]
  : RESTRICT {s = "restrict";}
  | VOLATILE {s = "volatile";}
  | CONST    {s = "const";}
;

address_space returns [std::string attribute]
  : ADDRESS_SPACE_PREFIX n1:NUMBER{
    std::string strSuffixLen = n1->getText();
    char* endString;
    const char* beginString = strSuffixLen.c_str();
    long suffixLen = strtol(beginString, &endString, 10);
    assert (endString != beginString && "address space is not a valid suffix");
    assert(suffixLen>0 && "invalid suffix len");
    std::string strAddressSpace = m_pLexer->readN(suffixLen);
    const std::string AS("AS");
    assert(strAddressSpace.size() > AS.size() && "invalide address space");
    beginString = strAddressSpace.c_str() + AS.size();
    long addressSpace = strtol(beginString, &endString, 10);
    assert (endString != beginString &&"address space is not a valid suffix");
    switch (addressSpace){
      case 0L: attribute = "__private";  break;
      case 1L: attribute = "__global"; break;
      case 2L: attribute = "__constant";  break;
      case 3L: attribute = "__local";  break;
      default: assert (false && "invalid address space");
    }
  }
;

class DemangleLexer extends Lexer;

options{
  k=2;
}

{
//  private: unsigned int m_nSkipChars;

  public: std::string readN(unsigned int n){
    assert(n>0 && "invalid character number");
    resetText();
    while(n-->0)
      consume();
    std::string ret = getText();
    resetText();
    return ret;
  }

//  public:  int LA(unsigned int i){
//    while(m_nSkipChars){
//      inputState->getInput().getChar();
//      m_nSkipChars--;
//    }
//    return CharScanner::LA(i);
//  }
}

protected
DIGIT:
	'0'..'9'
	;

NUMBER:
	(DIGIT)+
	;

//Primitives
BOOL:    "b";
UCHAR:   "h";
CHAR:    "c";
USHOR:   "t";
SHORT:   "s";
UINT:    "j";
INT:     "i";
ULONG:   "m";
LONG:    "l";
HALF:    "Dh";
FLOAT:   "f";
DOUBLE:  "d";
VOID:    "v";
POINTER_PREFIX: "P";
VECTOR_PREFIX:  "Dv";
UNDERSCORE:     "_";
ADDRESS_SPACE_PREFIX: "U";
ADDRESS_SPACE_SUFFIX: "AS";
RESTRICT: "r";
VOLATILE: "V";
CONST:    "K";
PARAM_DUP_PREFIX: "S";

/*
class DemangleTreeWalker extends TreeParser;
expr returns [float r]
{
	float a,b;
	r=0;
}
	:	#(PLUS a=expr b=expr)	{r = a+b;}
	|	#(STAR a=expr b=expr)	{r = a*b;}
	|	i:INT			{r = atof(i->getText().c_str());}
	;
*/
