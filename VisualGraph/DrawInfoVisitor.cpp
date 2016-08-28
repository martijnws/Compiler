#include "stdafx.h"
#include "DrawInfoVisitor.h"
#include "DrawInfoNode.h"
#include <SyntaxTreeLib\SyntaxNode.h>
#include <FA/NFA.h>
#include <algorithm>

void DrawInfoSyntaxNodeVisitor::visit(const mws::ast::Leaf& n_) 
{
	m_depth = std::max(m_depth, m_curDepth);

	auto xCenter = m_cLeaf + 0.5f;
	auto yCenter = m_curDepth + 0.5f;

	m_subTree = new DrawInfoSyntaxNode(&n_, m_curDepth, 1, xCenter, yCenter);

	++m_cLeaf;
}

void DrawInfoSyntaxNodeVisitor::visit(const mws::ast::BinaryOp& n_) 
{
	++m_curDepth;

	n_.lhs().accept(*this);
	auto lhsTree = m_subTree;
	n_.rhs().accept(*this);
	auto rhsTree = m_subTree;

	--m_curDepth;

	auto xCenter = (lhsTree->xCenter() + rhsTree->xCenter()) / 2.f;
	auto yCenter = m_curDepth + 0.5f;

	m_subTree = new DrawInfoSyntaxNode(&n_, m_curDepth, lhsTree->width() + rhsTree->width(), xCenter, yCenter);
	m_subTree->add(lhsTree);
	m_subTree->add(rhsTree);
}

void DrawInfoSyntaxNodeVisitor::visit(const mws::ast::UnaryOp& n_)
{
	++m_curDepth;

	n_.opr().accept(*this);
	auto subTree = m_subTree;

	--m_curDepth;

	auto xCenter = subTree->xCenter();
	auto yCenter = m_curDepth + 0.5f;

	m_subTree = new DrawInfoSyntaxNode(&n_, m_curDepth, subTree->width(), xCenter, yCenter);
	m_subTree->add(subTree);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void DrawInfoNFANodeVisitor::visit(const mws::NFANode* n_)
{
	auto dn = new DrawInfoNFANode(n_, m_cNode++);
	m_visitMap.insert(std::make_pair(n_, dn));


	auto xL = m_cLeaf + 0.5f; 
	auto xR = xL;
	auto width = 0.f;

	++m_curDepth;

	auto newChildCount = 0;
	for (const auto& pair : n_->_transitionMap)
	{
		auto child = pair.second;
		auto itr = m_visitMap.find(child);
		if (itr == m_visitMap.end())
		{
			visit(child);
			assert(m_subTree);

			++newChildCount;
			
			xL = std::min(xL, m_subTree->xCenter());
			xR = std::max(xR, m_subTree->xCenter());
			width += m_subTree->width();
			
			dn->add(m_subTree);
		}
		else
		{
			dn->add(itr->second);
		}
	}

	--m_curDepth;

	if (newChildCount == 0)
	{
		++m_cLeaf;
		m_depth = std::max(m_depth, m_curDepth);
		width = 1.f;
	}

	auto xCenter = (xL + xR) / 2.f;
	auto yCenter = m_curDepth + 0.5f;

	dn->init(m_curDepth, width, xCenter, yCenter);
	m_subTree = dn;
}
