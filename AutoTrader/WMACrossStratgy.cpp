#include "stdafx.h"
#include "WMACrossStratgy.h"
#include "ThostFtdcDepthMDFieldWrapper.h"
#include "TechUtils.h"

WMACrossStratgy::WMACrossStratgy(size_t short_ma, size_t long_ma)
: MACrossStratgy(short_ma, long_ma)
{
}

WMACrossStratgy::~WMACrossStratgy()
{
}

MACrossTech* WMACrossStratgy::generateTechVec(const CThostFtdcDepthMDFieldWrapper& info) const{
	return (new MACrossTech(CrossStratgyType::WMA, m_shortMA, m_longMA, info.UUID(), info.InstrumentId(), info.Time(), info.LastPrice()));
}

// WMA
/*
y=wma(x,a)��y=(n*x0+(n-1)*x1+(n- 2)*x2)+...+1*xn)/(n+(n-1)+(n-2)+...+1)
*/
double WMACrossStratgy::calculateK(const std::list<CThostFtdcDepthMDFieldWrapper>& data, const CThostFtdcDepthMDFieldWrapper& current, int seconds) const
{
	return TechUtils::CalulateWMA(data, current, seconds);
}