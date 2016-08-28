#include "stdafx.h"
#include "ASTWindow.h"
#include "DrawInfoVisitor.h"
#include "DrawInfoNode.h"
#include "ToStringVisitor.h"
#include "Canvas.h"
#include "resource.h"
#include <WinLib\String.h>
#include <WinLib\Msg.h>
#include <WinLib\MsgHandler.h>
#include <WinLib\PaintDC.h>
#include <WinLib\WindowClass.h>
#include <Direct2DLib\BitmapSource.h>
#include <Direct2DLib\System.h>
#include <FA\AlphabetVisitor.h>
#include <FA\NFABuilderVisitor.h>
#include <cassert>

void ASTWindow::Init(MsgMap& msgMap)
{
	Window::Init(msgMap);

	using Wnd = ASTWindow;
	RegisterMsgHandler<WmCreate >(&Wnd::OnMsg, msgMap);
	RegisterMsgHandler<WmPaint  >(&Wnd::OnMsg, msgMap);
    RegisterMsgHandler<WmSize   >(&Wnd::OnMsg, msgMap);
	RegisterMsgHandler<WmCommand>(&Wnd::OnMsg, msgMap);
}

void ASTWindow::Create(const Module& module)
{
    Icon iconLg(module, IDI_VISUALGRAPH); 
    Icon iconSm(module, IDI_SMALL);
    WindowClass wndClass = WindowClass::GetBasicWindowClass(IDC_VISUALGRAPH, iconLg, iconSm, 0, module);

	FixedString<128> title(module, IDS_APP_TITLE);
    Window::Create(wndClass, title);
}

LRESULT ASTWindow::OnMsg(const WmCreate& msg)
{
	HRESULT hr = E_FAIL;

	auto wf = m_d2dSystem.GetDWriteFactory();
	// Create a DirectWrite text format object.
	hr = wf->CreateTextFormat(
		L"Courier New", nullptr, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 24,
		L"", //locale
		&m_spTextFormat);

	if (FAILED(hr))
	{
		throw Exception(hr);
	}

	RenderTargetGuard rtGuard(m_renderTarget, *this);
	auto rt = m_renderTarget.Get();

	hr = rt->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::CornflowerBlue), &m_spBrushBlue);
	if (FAILED(hr)) throw Exception();
	
	hr = rt->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &m_spBrushWhite);
	if (FAILED(hr)) throw Exception();

    m_spTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	m_spTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

	// Build NFA from AST
	mws::AlphabetVisitor alphabetVisitor;
	m_astRoot->accept(alphabetVisitor);
	auto rkSet = mws::getDisjointRangeSet(alphabetVisitor._rkVec);

	mws::NFABuilderVisitor visitor(rkSet);
	m_astRoot->accept(visitor);
	m_nfaStart = visitor.startState();

	return 0;
}

LRESULT ASTWindow::OnMsg(const WmPaint& msg)
{
    PaintDC dc(m_hwnd);

	RenderTargetGuard rtGuard(m_renderTarget, *this);

	Draw(*m_renderTarget.Get());

    return 0;
}

LRESULT ASTWindow::OnMsg(const WmSize& msg)
{
	m_renderTarget.Resize(Size(msg.GetWidth(), msg.GetHeight()));
	return 0;
}

LRESULT ASTWindow::OnMsg(const WmCommand& msg)
{
	return 0;
}

void ASTWindow::drawGraphESet(Canvas& canvas_, const DrawInfoNode& n_, std::set<const DrawInfoNode*>& visitSet_)
{
	assert(&n_);
	visitSet_.insert(&n_);

	auto cx = n_.xCenter();
	auto cy = n_.yCenter();

	for (auto child : n_.childVec())
	{
		auto cxChild = child->xCenter();
		auto cyChild = child->yCenter();

		if (visitSet_.find(child) == visitSet_.end())
		{
			canvas_.drawLine(D2D1::Point2F(cx, cy), D2D1::Point2F(cxChild, cyChild), m_spBrushBlue.Get());
			drawGraphESet(canvas_, *child, visitSet_);
		}
		else
		{
			canvas_.drawCurveArrow(D2D1::Point2F(cx, cy), D2D1::Point2F(cxChild, cyChild), m_spBrushBlue.Get(), m_d2dSystem.GetD2D1Factory());
		}
	}
}

void ASTWindow::drawGraphVSet(Canvas& canvas_, const DrawInfoNode& n_, std::set<const DrawInfoNode*>& visitSet_)
{
	assert(&n_);
	visitSet_.insert(&n_);

	auto cx = n_.xCenter();
	auto cy = n_.yCenter();

	for (auto child : n_.childVec())
	{
		if (visitSet_.find(child) == visitSet_.end())
		{
			drawGraphVSet(canvas_, *child, visitSet_);
		}
	}

	auto radius = 0.2f;

	auto circle = D2D1::Ellipse(D2D1::Point2F(cx, cy), radius, radius);
	canvas_.drawEllipse(circle, m_spBrushBlue.Get());
	canvas_.fillEllipse(circle, m_spBrushWhite.Get());

	const auto label = n_.label();
	canvas_.drawText(
		label.c_str(), 
		static_cast<uint32_t>(label.length()), 
		m_spTextFormat.Get(),
		D2D1::RectF(cx - radius, cy - radius, cx + radius, cy + radius),
		m_spBrushBlue.Get());
}
void ASTWindow::Draw(ID2D1RenderTarget& rt)
{
	rt.Clear(D2D1::ColorF(D2D1::ColorF::White));

	HRESULT hr = E_FAIL;

	//DrawInfoSyntaxNodeVisitor visitor;
	//m_astRoot->accept(visitor);
	DrawInfoNFANodeVisitor visitor;
	visitor.visit(m_nfaStart);
	const auto& n = *visitor.detach();

	auto mtxScale = D2D1::Matrix3x2F::Scale(100.f, 100.f);
	auto mtxRotate = D2D1::Matrix3x2F::Rotation(90.f, D2D1::Point2F(40.f, 140.f));
	auto mtxTransform = D2D1::Matrix3x2F::Translation(40.f, 40.f);
	Canvas canvas(rt, mtxTransform, mtxScale);

	std::set<const DrawInfoNode*> visitSet;
	drawGraphESet(canvas,  n, visitSet);
	visitSet.clear();
	drawGraphVSet(canvas,  n, visitSet);
}
