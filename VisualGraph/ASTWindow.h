#pragma once

#include <WinLib\Window.h>
#include <WinLib\ForwardDecl.h>
#include <Direct2DLib\RenderTarget.h>
#include <RegexSyntaxTreeLib\SyntaxNode.h>
#include <vector>
#include <set>

class DrawInfoNode;
class Canvas;

namespace mws 
{
	class NFANode;
}

class ASTWindow
	: public Window
{
public:
	using RenderTarget      = d2d::DeviceContextRenderTarget;
	using RenderTargetGuard = d2d::RenderTargetGuard<RenderTarget>;

    ASTWindow(d2d::System& system_, mws::regex::SyntaxNode* astRoot_)
	:
		m_d2dSystem(system_), m_astRoot(astRoot_)
    {
    
    }

    void Create(const Module& module);

protected:
	void Init(MsgMap& msgMap);

private:
	LRESULT OnMsg(const WmCreate& msg);
	LRESULT OnMsg(const WmPaint& msg);
    LRESULT OnMsg(const WmSize& msg);
	LRESULT OnMsg(const WmCommand& msg);
   
	void Draw(ID2D1RenderTarget& rt);
	void drawLabel(Canvas& canvas_, const std::wstring& label_, const D2D1_POINT_2F& ptBeg_, const D2D1_POINT_2F& ptEnd_, ID2D1SolidColorBrush* brush_);
	void drawGraphESet(Canvas& canvas_, const DrawInfoNode& n_, std::set<const DrawInfoNode*>& visitSet_);
	void drawGraphVSet(Canvas& canvas_, const DrawInfoNode& n_, std::set<const DrawInfoNode*>& visitSet_);


private:
	d2d::System&                 m_d2dSystem;
	RenderTarget                 m_renderTarget;
	ComPtr<IDWriteTextFormat>    m_spTextFormat;
	ComPtr<IDWriteTextFormat>    m_spTextFormatSmall;
	ComPtr<ID2D1SolidColorBrush> m_spBrushBlue;
	ComPtr<ID2D1SolidColorBrush> m_spBrushWhite;
	mws::regex::SyntaxNode::Ptr  m_astRoot;
	mws::NFANode*                m_nfaStart = nullptr;
};


