#pragma once

#include "DFANode.h"
#include "NFA.h"
#include <CommonLib/Buffer.h>
#include <CommonLib/String.h>

namespace mws {

class Token
{
public:
	static const std::size_t Invalid = -1;

    std::size_t _type;
    String      _lexeme;
};

class Lexer
{
public:
    Lexer(IStreamExt& is_, const std::vector<StringExt>& regexCol_);
   
    Token next();

private:
    common::BufferExt _buf;
    DFANode*          _dfa;
    bool              _eof;
};

}
