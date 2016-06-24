#ifndef TRADE_SPI_H_
#define TRADE_SPI_H_

#include "ThostFtdcTraderApi.h"
#include <mutex>
#include <condition_variable>
#include "crossplatform.h"
#include <functional>
#include "AccountMgr.h"

class IPositionControl;
class Order;

namespace PP{
	class PositionProfitMgr;
}

typedef std::function<void()>															InitedAccountCallback;
typedef std::function<void(CThostFtdcOrderField*)>										RtnOrderCallback;
typedef std::function<void(CThostFtdcTradeField*)>										RtnTradeCallback;
typedef std::function<void(CThostFtdcInputOrderActionField*, CThostFtdcRspInfoField*)>	CancelOrderCallback;
typedef PP::PositionProfitMgr PPMgr;

class CtpTradeSpi : public CThostFtdcTraderSpi
{

	class TradeThreadStateChangedHandler{
		//Sleep(1000) is for avoiding query frequency limit, otherwise the queryRequest will return -3
	public:
		TradeThreadStateChangedHandler(CtpTradeSpi* pTradeUserSpi)
			:m_TradeUserSpiPtr(pTradeUserSpi)
		{
		}

		//step 1
		void OnFrontConnected(){
			m_TradeUserSpiPtr->ReqUserLogin();
		}

		//step 2
		void OnLogined(){
			m_TradeUserSpiPtr->ReqSettlementInfoConfirm();
		}

		//step 3 
		void OnConfirmedSettlementInfo(){
			sleep(1000);
			m_TradeUserSpiPtr->ReqQryOrder();
		}

		//step 4 : get the order list 
		void OnLastRspQryOrder(bool successful = true){
			sleep(1000);
			m_TradeUserSpiPtr->ReqQryTrade();
		}

		//step 5 : get the trade list
		//请求查询成交响应,要区分程序启动时第一次查询跟之后的查询
		//并且只能查询当天的成交，昨仓不能，所以还要查询持仓明细OnRspQryInvestorPositionDetail()
		void OnLastRspQryTrade(bool successful = true){
			sleep(1000);
			m_TradeUserSpiPtr->ReqQryInvestorPositionDetail();
		}

		//step 6: query account
		void OnLastRspQryInvestorPositionDetail(bool successful = true){
			sleep(1000);
			m_TradeUserSpiPtr->ReqQryTradingAccount();
		}

		//step 7:
		void OnLastRspQryTradingAccount(bool successful = true){
			//查询所有合约的持仓
			sleep(1000);
			m_TradeUserSpiPtr->ReqQryInvestorPosition_all();
		}

		//step 8
		void OnLastRspQryInvestorPosition(bool successful = true){
			sleep(1000);
			m_TradeUserSpiPtr->ReqQryInstrument_all();
		}

		void OnLastRspQryInstrument(bool successful = true){
			m_TradeUserSpiPtr->NotifyQueryEnd();
		}

	private:
		CtpTradeSpi* m_TradeUserSpiPtr;
	};

public:
	CtpTradeSpi(CThostFtdcTraderApi* pUserApi,
		const char * brokerID, const char* userID, const char* password, const char* prodName,
		PPMgr& ppmgr,
		RtnOrderCallback onRtnOrderCallback,
		RtnTradeCallback onRtnTradeCallback,
		CancelOrderCallback OnRtnCancellOrderCallback);
        virtual	~CtpTradeSpi();

		void ReqOrderInsert(Order ord);

		void ReqOrderAction(const CThostFtdcOrderField& order);// TThostFtdcSequenceNoType orderSeq, TThostFtdcExchangeIDType exchangeId, TThostFtdcOrderSysIDType orderSysId);

		//理解：在触发了新的交易之前，把已提交未成交的报单给撤销
		void CancelOrder(long long MDtime, int aliveDuration = 6, const std::string& instrumentId = "");

		void ReqAllRateParameters(const std::vector<std::string>& instruments);

		void ForceClose();

		void WaitQueryEnd(){
			std::unique_lock<std::mutex> lk(m_mtx);
			m_con.wait(lk, [this]() -> bool { return !m_querying.load(); });
		}

private:

	virtual void OnFrontConnected();

	virtual void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	virtual void OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///请求查询报单响应
	virtual void OnRspQryOrder(CThostFtdcOrderField *pOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///请求查询成交响应
	virtual void OnRspQryTrade(CThostFtdcTradeField *pTrade, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///请求查询投资者持仓明细响应
	virtual void OnRspQryInvestorPositionDetail(CThostFtdcInvestorPositionDetailField *pInvestorPositionDetail, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	virtual void OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	virtual void OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	virtual void OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	virtual void OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	virtual void OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	virtual void OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	virtual void OnFrontDisconnected(int nReason);

	virtual void OnHeartBeatWarning(int nTimeLapse);

	virtual void OnRtnOrder(CThostFtdcOrderField *pOrder);

	virtual void OnRtnTrade(CThostFtdcTradeField *pTrade);

	//响应查询合约保证金率
	void OnRspQryInstrumentMarginRate(CThostFtdcInstrumentMarginRateField *pInstrumentMarginRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///响应查询合约手续费率
	void OnRspQryInstrumentCommissionRate(CThostFtdcInstrumentCommissionRateField *pInstrumentCommissionRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///响应查询期权交易成本
	void OnRspQryOptionInstrTradeCost(CThostFtdcOptionInstrTradeCostField *pOptionInstrTradeCost, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///响应查询期权合约手续费
	void OnRspQryOptionInstrCommRate(CThostFtdcOptionInstrCommRateField *pOptionInstrCommRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

private:
	void ReqUserLogin();

	void ReqSettlementInfoConfirm();

	void ReqQryOrder();

	void ReqQryTrade();

	void ReqQryInvestorPositionDetail();

	void ReqQryInstrument(TThostFtdcInstrumentIDType instId);

	void ReqQryTradingAccount();

	void ReqQryInvestorPosition_all();

	void ReqQryInvestorPosition();

	void ReqQryInstrument_all();

	///请求查询合约保证金率
	void ReqQryInstrumentMarginRate(const char* instId);

	///请求查询合约手续费率
	void ReqQryInstrumentCommissionRate(const char* instId);

	///请求查询期权交易成本
	void ReqQryOptionInstrTradeCost(const char* instId);

	///请求查询期权合约手续费
	void ReqQryOptionInstrCommRate(const char* instId);

	bool IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo);

	void NotifyQueryEnd(){
		m_querying.store(false);
		m_con.notify_all();
	}

private:
	TThostFtdcBrokerIDType								m_brokerID;
	TThostFtdcUserIDType								m_userID;
	TThostFtdcPasswordType								m_password;
	TThostFtdcProductInfoType							m_productName;
	TThostFtdcFrontIDType								m_frontID;
	TThostFtdcSessionIDType								m_sessionID;
	char												m_orderRef[13];
	CThostFtdcTradingAccountField						m_accountInfo;

private:
	std::condition_variable								m_con;
	std::atomic<bool>									m_querying;
	std::mutex											m_mtx;
	RtnOrderCallback									m_OnRtnOrder_callback;
	RtnTradeCallback									m_OnRtnTrade_callback;
	CancelOrderCallback									m_OnCancelOrder_callback;
	PPMgr&												m_ppmgr;
	CThostFtdcTraderApi*								pUserApi;
	TradeThreadStateChangedHandler						m_stateChangeHandler;
	int              									m_requestId;
};

#endif 
