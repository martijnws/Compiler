#pragma once

#include "ToStringVisitor.h"
#include <SyntaxTreeLib\SyntaxNode.h>
#include <FA\NFA.h>
#include <vector>
#include <sstream>

class DrawInfoNode
{
public:
	using EdgeLabelVertexPair = std::pair<std::wstring, const DrawInfoNode*>;

	DrawInfoNode() = default;

	DrawInfoNode(uint32_t depth_, float width_, float xCenter_, float yCenter_)
	:
		m_depth(depth_), m_width(width_), m_xCenter(xCenter_), m_yCenter(yCenter_)
	{

	}

	void init(uint32_t depth_, float width_, float xCenter_, float yCenter_)
	{
		m_depth = depth_; m_width = width_; m_xCenter = xCenter_; m_yCenter = yCenter_;
	}

	auto depth() const
	{
		return m_depth;
	}

	auto width() const
	{
		return m_width;
	}

	auto xCenter() const
	{
		return m_xCenter;
	}

	auto yCenter() const
	{
		return m_yCenter;
	}

	void add(const DrawInfoNode* child_, const std::wstring& label_ = L"")
	{
		m_childVec.push_back(std::make_pair(label_, child_));
	}

	void add(const std::vector<EdgeLabelVertexPair>& childVec_)
	{
		m_childVec.insert(m_childVec.end(), childVec_.begin(), childVec_.end());
	}

	const auto& childVec() const
	{
		return m_childVec;
	}

	virtual const std::wstring label() const = 0;

private:
	uint32_t m_depth = 0;
	float    m_width = 0;
	float    m_xCenter = 0;
	float    m_yCenter = 0;
	std::vector<EdgeLabelVertexPair> m_childVec;
};

class DrawInfoSyntaxNode
:
	public DrawInfoNode
{
public:
	DrawInfoSyntaxNode(const mws::ast::SyntaxNode* n_, uint32_t depth_, float width_, float xCenter_, float yCenter_)
	:
		DrawInfoNode(depth_, width_, xCenter_, yCenter_),
		m_n(n_)
	{

	}

	const std::wstring label() const override
	{
		ToStringVisitor tsv;
		m_n->accept(tsv);
		return tsv.m_result;
	}


private:
	const mws::ast::SyntaxNode* m_n = nullptr;
};

class DrawInfoNFANode
:
	public DrawInfoNode
{
public:
	DrawInfoNFANode(const mws::NFANode* n_, uint32_t label_)
	:
		m_n(n_),
		m_label(label_)
	{

	}

	DrawInfoNFANode(const mws::NFANode* n_, uint32_t label_, uint32_t depth_, float width_, float xCenter_, float yCenter_)
	:
		DrawInfoNode(depth_, width_, xCenter_, yCenter_),
		m_n(n_),
		m_label(label_)
	{

	}

	const std::wstring label() const override
	{
		std::wstringstream s;
		s << m_label;
		return s.str();
	}


private:
	const mws::NFANode* m_n = nullptr;
	uint32_t            m_label;
};
