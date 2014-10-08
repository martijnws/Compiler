#pragma once

#include "Token.h"
#include <initializer_list> 
#include <algorithm>

namespace mws { namespace td { namespace LL1 {

bool isIn(const Token& t_, const std::initializer_list<Token::Type>& list_)
{
	return std::any_of(list_.begin(), list_.end(), [t_](const Token::Type& tt_){ return t_._type == tt_; });
}

}}}
