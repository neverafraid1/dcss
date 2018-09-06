//
// Created by wangzhen on 18-6-25.
//

#include "IDCSSStrategy.h"
#include "DCSSStrategyImpl.h"

IDCSSStrategy::IDCSSStrategy(const std::string& name)
{
	pImpl.reset(new DCSSStrategyImpl(name, this));
}

bool IDCSSStrategy::Init(const std::string& path)
{
	return pImpl->Init(path);
}

void IDCSSStrategy::Start()
{
	pImpl->Start();
}

void IDCSSStrategy::Terminate()
{
	pImpl->Terminate();
}

void IDCSSStrategy::Stop()
{
	pImpl->Stop();
}

void IDCSSStrategy::Run()
{
	pImpl->Run();
}

void IDCSSStrategy::Block()
{
	pImpl->Block();
}

int IDCSSStrategy::InsertOrder(uint8_t source, const std::string& symbol, double price, double volume, OrderDirection direction, OrderType type)
{
    return pImpl->InsertOrder(source, symbol, price, volume, direction, type);
}

int IDCSSStrategy::CancelOrder(uint8_t source, const std::string& symbol, long orderId)
{
    return pImpl->CancelOrder(source, symbol, orderId);
}

int IDCSSStrategy::QryOrder(uint8_t source, const std::string& symbol, long orderId)
{
	return pImpl->QryOrder(source, symbol, orderId);
}

int IDCSSStrategy::QryTicker(uint8_t source, const std::string& symbol)
{
    return pImpl->QryTicker(source, symbol);
}

int IDCSSStrategy::QryKline(uint8_t source, const std::string& symbol, KlineType klineType, int size, long since)
{
    return pImpl->QryKline(source, symbol, klineType, size, since);
}

void IDCSSStrategy::SubscribeTicker(uint8_t source, const std::string& symbol)
{
    pImpl->SubscribeTicker(source, symbol);
}

void IDCSSStrategy::SubscribeKline(uint8_t source, const std::string& symbol, KlineType klineType)
{
    pImpl->SubscribeKline(source, symbol, klineType);
}

void IDCSSStrategy::SubscribeDepth(uint8_t source, const std::string& symbol, int depth)
{
    pImpl->SubscribeDepth(source, symbol, depth);
}

void IDCSSStrategy::OnRtnTicker(const DCSSTickerField* ticker, uint8_t source, long recvTime)
{
    DCSS_LOG_DEBUG(pImpl->GetLogger(), "(recv time)" << recvTime << " (source)" << (int)source << " (tick time)" << ticker->UpdateTime << " (symbol)" << ticker->Symbol);
}

void IDCSSStrategy::OnRtnKline(const DCSSKlineField* kline, uint8_t source, long recvTime)
{
    DCSS_LOG_DEBUG(pImpl->GetLogger(), "(recv time)" << recvTime << " (source)" << (int)source << " (symbol)" << kline->Symbol);
}

void IDCSSStrategy::OnRtnDepth(const DCSSDepthField* depth, uint8_t source, long recvTime)
{
    DCSS_LOG_DEBUG(pImpl->GetLogger(), "(recv time)" << recvTime << " (source)" << (int)source << " (symbol)" << depth->Symbol);
}

void IDCSSStrategy::OnRtnOrder(const DCSSOrderField* order, uint8_t source, long recvTime)
{
    DCSS_LOG_DEBUG(pImpl->GetLogger(), "(recv time)" << recvTime << " (source)" << (int)source << " (symbol)" << order->Symbol
                                          << " (status)" << (int)order->Status);
}

void IDCSSStrategy::OnRspOrderInsert(const DCSSRspInsertOrderField* rsp, int requestID, int errorId, const char* errorMsg, uint8_t source, long recvTime)
{
    std::stringstream ss;
    ss << " (recv time)" << recvTime << "(source)" << source;
    if (errorId != 0)
        ss << " (error id)" << errorId;
    if (errorMsg != nullptr)
        ss << " (error msg)" << errorMsg;
    DCSS_LOG_DEBUG(pImpl->GetLogger(), ss.str().c_str());
}

void IDCSSStrategy::OnRspQryTicker(const DCSSTickerField* rsp, int requestId, int errorId, const char* errorMsg,
        uint8_t source, long recvTime)
{
    std::stringstream ss;
    ss << " (recv time)" << recvTime << "(source)" << (int)source;
    if (errorId != 0)
        ss << " (error id)" << errorId;
    if (errorMsg != nullptr)
        ss << " (error msg)" << errorMsg;
    DCSS_LOG_DEBUG(pImpl->GetLogger(), ss.str().c_str());
}

void IDCSSStrategy::OnRspQryKline(const DCSSKlineField* kline, int requestId, int errorId, const char* errorMsg, uint8_t source, long recvTime)
{
    std::stringstream ss;
    ss << " (recv time)" << recvTime << "(source)" << (int)source;
    if (errorId != 0)
        ss << " (error id)" << errorId;
    if (errorMsg != nullptr)
        ss << " (error msg)" << errorMsg;
    DCSS_LOG_DEBUG(pImpl->GetLogger(), ss.str().c_str());
}

void IDCSSStrategy::OnRspQryOrder(const DCSSOrderField* rsp, int requestId, int errorId, const char* errorMsg,
        uint8_t source, long recvTime)
{
    std::stringstream ss;
    ss << " (recv time)" << recvTime << "(source)" << (int)source;
    if (errorId != 0)
        ss << " (error id)" << errorId;
    if (errorMsg != nullptr)
        ss << " (error msg)" << errorMsg;
    DCSS_LOG_DEBUG(pImpl->GetLogger(), ss.str().c_str());
}

void IDCSSStrategy::Debug(const char* msg)
{
    DCSS_LOG_DEBUG(pImpl->GetLogger(), msg);
}

void IDCSSStrategy::Info(const char* msg)
{
	DCSS_LOG_INFO(pImpl->GetLogger(), msg);
}

void IDCSSStrategy::Error(const char* msg)
{
	DCSS_LOG_ERROR(pImpl->GetLogger(), msg);
}
