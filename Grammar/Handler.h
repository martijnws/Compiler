#pragma once

#include "TokenStore.h"
#include <cstdint>

namespace mws { namespace grammar {

class Handler
{
public:
	virtual void onNonTerminal(uint8_t type_) = 0;
	virtual void onTerminal(uint8_t type_, const Token& t_) = 0;
    
    inline Token get(uint8_t type_) const
    {
        return _store.get(type_);
    }

    inline void put(uint8_t type_, const Token& t_)
    {
        _store.put(type_, t_);
    }

protected:
    TokenStore _store;
};

}}
