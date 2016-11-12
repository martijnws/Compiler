// ProgWin5.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "Resource.h"
#include "ASTWindow.h"
#include <ComLib\CoInit.h>

#include <WinLib\WindowImpl.h>
#include <WinLib\Module.h>
#include <WinLib\basic_oarraybuf.h>
#include <WinLib\DebugOutputStream.h>
#include <WinLib\Exception.h>

#include <Direct2DLib\System.h>

#include <RegexLL1ParserLib\Parser.h>
#include <FA\AlphabetVisitor.h>
#include <FA\NFABuilderVisitor.h>
#include <SyntaxTreeLib\Visitor.h>

#include <sstream>

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	DebugOutputStream dbgOS;
	mtl::basic_oarraybuf<TCHAR> dbgOSBuf(dbgOS);
	std::wcout.set_rdbuf(&dbgOSBuf);

	CoInit coInit;
	Module module(hInstance);

	int retval = 0;
	try
	{
		//const auto* expr = "[1-9][0-9]*.?[0-9]*";
		//const auto* expr = "a";
		const auto* expr = "a|b";
		std::stringstream is(expr, std::ios::in);
		// build the regex syntax tree
		mws::td::LL1::Parser parser(is);
		if (!parser.parse())
		{
			std::wcout << "parser error" << std::endl;
			return -1;
		}

		// root of the regex syntax tree
		auto astRoot = parser._astBuilder.detach();

		// collect ranges of characters. Each range represents a sinlge element in the alphabet of the FA we're building
        mws::AlphabetVisitor alphabetVisitor;
        astRoot->accept(alphabetVisitor);
		// split the ranges into disjoint ranges. This ensures each character can fall into exactly 1 range.
        auto rkSet = mws::getDisjointRangeSet(alphabetVisitor._rkVec);

		// build NFA with set of ranges as input alphabet
        mws::NFABuilderVisitor nfaBuilderVisitor(rkSet);
		// the NFA accepts any string represented by astRoot
		astRoot->accept(nfaBuilderVisitor);

		d2d::System d2dSystem;
		HRESULT hr = E_FAIL;

		WindowImpl<ASTWindow> wnd(d2dSystem, astRoot);
		wnd.Create(module);
		wnd.Show(nCmdShow);
		wnd.Update();

		retval = Window::Run(IDC_VISUALGRAPH);
	}
	catch(const Exception& e)
	{
		std::wcout << e.Message() << std::endl;
		retval = -1;
	}

	return retval;
}

