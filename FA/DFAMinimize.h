#pragma once

#include "RangeKey.h"

namespace mws {

class DFANode;

void minimize(DFANode* d_, const std::set<RangeKey, RangeKey::Less>& rkSet_);

}