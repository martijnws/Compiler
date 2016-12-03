#include "RangeKey.h"
#include "NFA.h"
#include <CommonLib/Unicode.H>
#include <sstream>

namespace mws {

String RangeKey::toString() const
{
	Char bufL[8];
	const auto sizeL = common::utf::Encoder<Char>::encode(_l, bufL);
	bufL[sizeL] = _C('\0');

	Char bufH[8];
	const auto sizeH = common::utf::Encoder<Char>::encode(_h, bufH);
	bufH[sizeH] = _C('\0');

	StringStream label;

	if (_l < _h)
	{
		label << bufL << _C("-") << bufH;
	}
	else
	if (_l == mws::NFA::E)
	{
		label << _C('\u03b5');
	}
	else
	{
		label << bufL;
	}

	return label.str();
}

}
