header "pre_include_hpp" {
#include <cstdio>
#include <cstdlib>
#include <stack>
#include "Type.h"
#include "antlr/ParserSharedInputState.hpp"
#include "antlr/TokenBuffer.hpp"
}

header "post_include_hpp"{
typedef std::stack<reflection::Type*> TypeStack;
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

public:
void setCallback(reflection::TypeCallback* cb){m_cb = cb;};
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

cv_qualifier returns [std::string s]
  : RESTRICT {s = "restrict";}
  | VOLATILE {s = "volatile";}
  | CONST    {s = "const";}
;

address_space returns [std::string attribute]
  : ADDRESS_SPACE_PREFIX n1:NUMBER ADDRESS_SPACE_SUFFIX n2:NUMBER{
    std::string strSuffixLen = n1->getText();
    std::string strAddressSpace = n2->getText();
    unsigned suffixLen = atoi(strSuffixLen.c_str());
    unsigned addressSpace = atoi(strAddressSpace.c_str());
    if (suffixLen -2 != strAddressSpace.length())
      throw ("internal bug! we need a custom lexer..");
    switch (addressSpace){
      case 0: attribute = "__private";  break;
      case 1: attribute = "__global"; break;
      case 2: attribute = "__local";  break;
      case 3: attribute = "__const";  break;
      default: assert (false && "unreachable code");
    }
  }
;

class DemangleLexer extends Lexer;

options{
  k=2;
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
HALF:    "f";
FLOAT:   "d";
DOUBLE:  "Dh";
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
