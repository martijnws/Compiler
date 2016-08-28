#pragma once

#include <SyntaxTreeLib\Visitor.h>
#include <map>

class DrawInfoNode;

namespace mws
{
	class NFANode;
}

class GraphDrawInfo
{
public:
	const auto* detach()
	{
		const auto* subTree = m_subTree;
		m_subTree = nullptr;
		return subTree;
	}

	auto depth() const
	{
		return m_depth;
	}

protected:
	DrawInfoNode* m_subTree = nullptr;	
	uint32_t      m_curDepth = 0;
	uint32_t      m_depth = 0;
	uint32_t      m_cLeaf = 0;
	float         m_xCenter = 0;
	float         m_yCenter = 0;
};

class DrawInfoSyntaxNodeVisitor
: 
	public mws::ast::Visitor,
	public GraphDrawInfo
{
public:
	void visit(const mws::ast::Leaf& n_) override; 
	void visit(const mws::ast::BinaryOp& n_) override;
	void visit(const mws::ast::UnaryOp& n_) override;
};

class DrawInfoNFANodeVisitor
: 
	public GraphDrawInfo
{
public:
	void visit(const mws::NFANode* n_);

private:
	std::map<const mws::NFANode*, DrawInfoNode*> m_visitMap;
	uint32_t                                     m_cNode = 0;
};
