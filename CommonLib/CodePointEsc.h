#pragma once

#include "CodePoint.h"

namespace mws {

inline bool CodePointFromEsc(CodePoint cpEsc_, CodePoint& cp_)
{
	CodePoint cp = -1;

	switch(cpEsc_)
	{
	case CP('\''):
	case CP('\"'):
	case CP('\\'):
		cp = cpEsc_;	break;

	case CP('?'):
		cp = CP('\?'); break;
	case CP('a'):
		cp = CP('\a'); break;
	case CP('b'):
		cp = CP('\b'); break;
	case CP('f'):
		cp = CP('\f'); break;
	case CP('n'):
		cp = CP('\n'); break;
	case CP('r'):
		cp = CP('\r'); break;
	case CP('t'):
		cp = CP('\t'); break;
	case CP('v'):
		cp = CP('\v'); break;
	case CP('0'):
		cp = CP('\0'); break;

	};

	const auto res = cp != -1;
	if (res)
	{
		cp_ = cp;
	}

	return res;
}

}

