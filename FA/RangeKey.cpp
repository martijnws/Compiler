#include "RangeKey.h"
#include "NFA.h"
#include <sstream>

namespace mws {

std::wstring RangeKey::toString() const
{
	std::wstringstream label;
	if (_l < _h)
	{
		label << (wchar_t)_l << "-" << (wchar_t)_h;
	}
	else
	if (_l == mws::NFA::E)
	{
		label << L'\u03b5';
	}
	else
	{
		label << (wchar_t)_l;
	}

	return label.str();
}

}
