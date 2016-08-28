#pragma once

#include <WinLib\Exception.h>
#include <Direct2DLib\RenderTarget.h>
#include <d2d1helper.h>
#include <cmath>
#include <limits>


struct IDWriteTextFormat;

class Canvas
{
public:
	Canvas(ID2D1RenderTarget& rt_, const D2D1::Matrix3x2F& mtxTransform_, const D2D1::Matrix3x2F& mtxScale_)
	:
		_rt(rt_), _mtxScale(mtxScale_)
	{
		// Note: To extract scale matrix from mtxTransform you need to do a Singular Value Decomposition.
		// For now, just pass in scale matrix separately

		_rt.SetTransform(mtxTransform_);
	}

	void drawRectangle(const D2D1_RECT_F& rect_, ID2D1SolidColorBrush* brush_)
	{
		auto lt = _mtxScale.TransformPoint(D2D1::Point2F(rect_.left, rect_.top));
		auto rb = _mtxScale.TransformPoint(D2D1::Point2F(rect_.right, rect_.bottom));
		auto rect = D2D1::RectF(lt.x, lt.y, rb.x, rb.y);	

		_rt.DrawRectangle(rect, brush_);
	}

	void drawEllipse(const D2D1_ELLIPSE& ellipse_, ID2D1SolidColorBrush* brush_)
	{
		auto center = _mtxScale.TransformPoint(ellipse_.point);
		auto radius = _mtxScale.TransformPoint(D2D1::Point2F(ellipse_.radiusX, ellipse_.radiusY));

		auto ellipse = D2D1::Ellipse(center, radius.x, radius.y);
		_rt.DrawEllipse(ellipse, brush_);
	}

	void fillEllipse(const D2D1_ELLIPSE& ellipse_, ID2D1SolidColorBrush* brush_)
	{
		auto center = _mtxScale.TransformPoint(ellipse_.point);
		auto radius = _mtxScale.TransformPoint(D2D1::Point2F(ellipse_.radiusX, ellipse_.radiusY));

		auto ellipse = D2D1::Ellipse(center, radius.x, radius.y);
		_rt.FillEllipse(ellipse, brush_);
	}

	void drawLine(const D2D1_POINT_2F& ptBeg_, const D2D1_POINT_2F& ptEnd_, ID2D1SolidColorBrush* brush_)
	{
		auto ptBeg = _mtxScale.TransformPoint(ptBeg_);
		auto ptEnd = _mtxScale.TransformPoint(ptEnd_);

		_rt.DrawLine(ptBeg, ptEnd, brush_);
	}

	void drawText(const wchar_t* text_, uint32_t len_, IDWriteTextFormat* pTextFormat, const D2D1_RECT_F& rect_, ID2D1SolidColorBrush* brush_)
	{
		auto lt = _mtxScale.TransformPoint(D2D1::Point2F(rect_.left, rect_.top));
		auto rb = _mtxScale.TransformPoint(D2D1::Point2F(rect_.right, rect_.bottom));
		auto rect = D2D1::RectF(lt.x, lt.y, rb.x, rb.y);	

		_rt.DrawText(text_, len_, pTextFormat, rect, brush_);
	}

	void drawCurveArrow(const D2D1_POINT_2F& ptBeg_, const D2D1_POINT_2F& ptEnd_, ID2D1SolidColorBrush* brush_, ID2D1Factory* pD2D1Factory_)
	{
		auto ptBeg = _mtxScale.TransformPoint(ptBeg_);
		auto ptEnd = _mtxScale.TransformPoint(ptEnd_);

		D2D1::Matrix3x2F mtxTransform;
		_rt.GetTransform(&mtxTransform);

		// because y is positive in down direction w have to negate y side
		auto opp = static_cast<double>(ptEnd.y - ptBeg.y);
		auto adj = static_cast<double>(ptEnd.x - ptBeg.x);
		auto hyp = std::sqrt(std::pow(opp, 2) + std::pow(adj, 2));

		if (hyp < std::numeric_limits<double>::epsilon())
		{
			return;
		}

		const double pi = 3.14159265358979323846;

		auto theta = std::asin(opp / hyp) * (180 / pi);
		// Q2, Q3
		if (adj < 0.f)
		{
			theta = 180.0 - theta;	
		}
/*
		auto ptRot = mtxTransform.TransformPoint(ptBeg);
		auto mtxRotate = D2D1::Matrix3x2F::Rotation(static_cast<float>(theta), ptRot);
		_rt.SetTransform(mtxTransform * mtxRotate);
*/
		auto radiusX = 0.5 * hyp;
		auto radiusY = _mtxScale.TransformPoint(D2D1::Point2F(0.f, 0.4f)).y;
		/*
		auto center = D2D1::Point2F((ptBeg.x + radiusX), ptBeg.y);
		auto ellipse = D2D1::Ellipse(center, radiusX, radiusY);
		_rt.DrawEllipse(ellipse, brush_);

		_rt.SetTransform(mtxTransform);
*/

		HRESULT hr = E_FAIL;

		ComPtr<ID2D1PathGeometry> spGeoPath;
		hr = pD2D1Factory_->CreatePathGeometry(&spGeoPath) ;
		if (FAILED(hr)) throw Exception(hr);

		ComPtr<ID2D1GeometrySink> spGeoSink;
		hr = spGeoPath->Open(&spGeoSink);
		if (FAILED(hr)) throw Exception(hr);

        spGeoSink->BeginFigure(ptBeg, D2D1_FIGURE_BEGIN::D2D1_FIGURE_BEGIN_HOLLOW);

        spGeoSink->AddArc(
            D2D1::ArcSegment(ptEnd, D2D1::SizeF(radiusX, radiusY), theta, D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_SMALL)
		);            

        spGeoSink->EndFigure(D2D1_FIGURE_END::D2D1_FIGURE_END_OPEN);
	    hr = spGeoSink->Close();
		if (FAILED(hr)) throw Exception(hr);

		_rt.DrawGeometry(spGeoPath.Get(), brush_);
	}

private:
	ID2D1RenderTarget&      _rt;
	const D2D1::Matrix3x2F& _mtxScale;
};


