#pragma once

#include "DFANode.h"
#include "NFA.h"
#include <CommonLib/Buffer.h>
#include <string>

namespace mws {

class Token
{
public:
    std::size_t _type;
    std::string _lexeme;
};

class Lexer
{
public:
    Lexer(std::istream& is_);
   
    const char* regex(std::size_t type) const;

    Token next();

private:
    common::Buffer<256> _buf;
    DFANode*            _dfa;
    bool                _eof;
};

}
