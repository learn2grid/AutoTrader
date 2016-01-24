#ifndef ORDER_H
#define ORDER_H
#include "stdafx.h"
#include <string>
#include "ThostFtdcUserApiStruct.h"

//enum ExchangeDirection{
//	Buy = '0',
//	Sell = '1',
//};

//enum ExchangePriceType{
//	AnyPrice = 0x0,
//	LimitPrice = 0x1,
//	LastPrice = 0x2,
//};
//
//enum TimeCondition{
//	///������ɣ�������
//	IOC = '1',
//	///������Ч
//	GFS = '2',
//	///������Ч
//    GFD = '3',
//	///ָ������ǰ��Ч
//	GTD = '4',
//	///����ǰ��Ч
//	GTC = '5',
//	///���Ͼ�����Ч
//	GFA = '6',
//};
//
//enum VolumeCondition{
//	///�κ�����
//	AnyVolume = '1',
//	///��С����
//	MinVolume = '2',
//	///ȫ������
//	WholeVolume = '3',
//};
//
//enum ContingentCondition{
//	Immediately = '1',
//	///ֹ��
//	Touch = '2',
//	///ֹӮ
//	TouchProfit = '3',
//	///Ԥ��
//	ParkedOrder = '4',
//	///���¼۴���������
//	LastPriceGreaterThanStopPrice = '5',
//	///���¼۴��ڵ���������
//	LastPriceGreaterEqualStopPrice = '6',
//	///���¼�С��������
//	LastPriceLesserThanStopPrice = '7',
//	///���¼�С�ڵ���������
//	LastPriceLesserEqualStopPrice = '8',
//	///��һ�۴���������
//	AskPriceGreaterThanStopPrice = '9',
//	///��һ�۴��ڵ���������
//	AskPriceGreaterEqualStopPrice = 'A',
//	///��һ��С��������
//	AskPriceLesserThanStopPrice = 'B',
//	///��һ��С�ڵ���������
//	AskPriceLesserEqualStopPrice = 'C',
//	///��һ�۴���������
//	BidPriceGreaterThanStopPrice = 'D',
//	///��һ�۴��ڵ���������
//	BidPriceGreaterEqualStopPrice = 'E',
//	///��һ��С��������
//	BidPriceLesserThanStopPrice = 'F',
//	///��һ��С�ڵ���������
//	BidPriceLesserEqualStopPrice = 'H',
//};
//


class STRATEGY_API Order
{
public:
	enum OrderType{
		LimitPriceFOKOrder = 0x0, //FOK: trade partly successfully while the others backout;
		LimitPriceFAKOrder = 0x1, //FAK: trade total successfully or total failed ; 
		AnyPriceOrder = 0x2,
		TriggerOrder = 0x3,
		NoType,
	};

	Order();
	Order(const std::string& instrument, TThostFtdcPriceType refprice, \
		TThostFtdcVolumeType voltype, \
		TThostFtdcDirectionType direction, \
		TThostFtdcCombOffsetFlagType kpp);
	//Order(const std::string& instrument, double refprice, ExchangeDirection direction, OrderType type);
	~Order();

	void SetTriggerTick(const std::string& ticktime)  { m_triggerTickTime = ticktime; }

	std::string GetTriggerTick() { return m_triggerTickTime; }

	std::string GetInstrumentId() const{
		return m_innerStruct.InstrumentID;
	}

	void SetInstrumentId(const std::string& in);

	void SetRefExchangePrice(double price);

	double GetRefExchangePrice() const{
		//todo : distinguish the m_type
		return m_innerStruct.LimitPrice;
	}

	TThostFtdcDirectionType GetExchangeDirection() const {
		return m_innerStruct.Direction;
	}

	void SetExchangeDirection(TThostFtdcDirectionType in){
		m_innerStruct.Direction = in;
	}

	void SetCombOffsetFlagType(char flag){
		m_innerStruct.CombOffsetFlag[0] = flag;
		for (int i = 1; i < 5; i++){
			m_innerStruct.CombOffsetFlag[i] = 0;
		}
	}

	void SetCombHedgeFlag(char flag){
		m_innerStruct.CombHedgeFlag[0] = flag;
		for (int i = 1; i < 5; i++){
			m_innerStruct.CombHedgeFlag[i] = 0;
		}
	}

	char GetCombOffsetFlagType() const {
		return m_innerStruct.CombOffsetFlag[0];
	}

	void SetVolume(int shou){
		m_innerStruct.VolumeTotalOriginal = shou;
	}

	int GetVolume() const{
		return m_innerStruct.VolumeTotalOriginal;
	}

	void SetOrderType(OrderType type);

	OrderType GetOrderType() const {
		return m_type;
	}

	void SetIdentityInfo(const std::string& brokerId, const std::string& userId, const std::string& investorId, const std::string& ordRef);

	bool IsValid();

	bool GetOrderOriginStruct(CThostFtdcInputOrderField& ord){
		if (IsValid()){
			ord = m_innerStruct;
			return true;
		}
		return false;
	}
private:
	CThostFtdcInputOrderField m_innerStruct;
	OrderType m_type;
	std::string m_triggerTickTime;
};

#endif