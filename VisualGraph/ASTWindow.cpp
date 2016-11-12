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

	hr = wf->CreateTextFormat(
		L"Courier New", nullptr, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 12,
		L"", //locale
		&m_spTextFormatSmall);

	if (FAILED(hr))
	{
		throw Exception(hr);
	}

	RenderTargetGuard rtGuard(m_renderTarget, *this, m_d2dSystem);
	auto rt = m_renderTarget.Get();

	hr = rt->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::CornflowerBlue), &m_spBrushBlue);
	if (FAILED(hr)) throw Exception();
	
	hr = rt->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &m_spBrushWhite);
	if (FAILED(hr)) throw Exception();

    m_spTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	m_spTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    m_spTextFormatSmall->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	m_spTextFormatSmall->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

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

	RenderTargetGuard rtGuard(m_renderTarget, *this, m_d2dSystem);

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

void ASTWindow::drawLabel(Canvas& canvas_, const std::wstring& label_, const D2D1_POINT_2F& ptBeg_, const D2D1_POINT_2F& ptEnd_, ID2D1SolidColorBrush* brush_)
{
	if (!label_.empty())
	{
		auto ptMid = D2D1::Point2F((ptBeg_.x + ptEnd_.x) / 2.f, (ptBeg_.y + ptEnd_.y) / 2.f);
		const auto radius = 0.1f;
		canvas_.drawText(
			label_.c_str(), 
			static_cast<uint32_t>(label_.length()),
			m_spTextFormatSmall.Get(),
			D2D1::RectF(ptMid.x, ptMid.y - 2 * radius, ptMid.x + radius, ptMid.y + 2 * radius),
			brush_);
	}
}

void ASTWindow::drawGraphESet(Canvas& canvas_, const DrawInfoNode& n_, std::set<const DrawInfoNode*>& visitSet_)
{
	assert(&n_);
	visitSet_.insert(&n_);

	auto cx = n_.xCenter();
	auto cy = n_.yCenter();

	for (auto pair : n_.childVec())
	{
		const auto& label = pair.first;
		auto child = pair.second;

		const auto ptBeg = D2D1::Point2F(cx, cy);
		const auto ptEnd = D2D1::Point2F(child->xCenter(), child->yCenter());


		if (visitSet_.find(child) == visitSet_.end())
		{
			drawLabel(canvas_, label, ptBeg, ptEnd, m_spBrushBlue.Get());
			canvas_.drawLine(ptBeg, ptEnd, m_spBrushBlue.Get());

			drawGraphESet(canvas_, *child, visitSet_);
		}
		else
		{
			drawLabel(canvas_, label, ptBeg, ptEnd, m_spBrushBlue.Get());
			canvas_.drawCurveArrow(ptBeg, ptEnd ,m_spBrushBlue.Get(), m_d2dSystem.GetD2D1Factory());
		}
	}
}

void ASTWindow::drawGraphVSet(Canvas& canvas_, const DrawInfoNode& n_, std::set<const DrawInfoNode*>& visitSet_)
{
	assert(&n_);
	visitSet_.insert(&n_);

	auto cx = n_.xCenter();
	auto cy = n_.yCenter();

	for (auto pair : n_.childVec())
	{
		auto child = pair.second;

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
	auto mtxRotate = D2D1::Matrix3x2F::Rotation(-90, D2D1::Point2F(50.f, 50.f));
	auto mtxTransform = D2D1::Matrix3x2F::Translation(40.f, 140.f);
	Canvas canvas(rt, mtxTransform, mtxScale, mtxRotate);

	std::set<const DrawInfoNode*> visitSet;
	drawGraphESet(canvas,  n, visitSet);
	visitSet.clear();
	drawGraphVSet(canvas,  n, visitSet);
}
