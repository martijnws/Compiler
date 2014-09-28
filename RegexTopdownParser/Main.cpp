#include "stdafx.h"
#include "Parser.h"
#include <sstream>
#include <iostream>

void parse(const std::string& pattern_)
{
	std::stringstream is(pattern_, std::ios_base::in);

	Parser parser(is);
	bool res = parser.parse();

	std::cout << res << std::endl;
}

int _tmain(int argc, _TCHAR* argv[])
{
	parse("");
	parse("aa(a|b)*bb");
	
	return 0;
}