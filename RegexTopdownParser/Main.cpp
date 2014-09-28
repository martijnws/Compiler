#include "stdafx.h"
#include "Parser.h"
#include <sstream>
#include <iostream>

int _tmain(int argc, _TCHAR* argv[])
{
	const std::string pattern("aa(a|b)*bb");
	std::stringstream is(pattern, std::ios_base::in);

	Parser parser(is);
	bool res = parser.parse();

	std::cout << res << std::endl;
	return 0;
}