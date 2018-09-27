//
// Created by wangzhen on 18-6-20.
//

#ifndef DIGITALCURRENCYSTRATEGYSYSTEM_STRATEGYSOCKETHANDLER_H
#define DIGITALCURRENCYSTRATEGYSYSTEM_STRATEGYSOCKETHANDLER_H

#include <array>
#include <unordered_set>
#include <unordered_map>

#include "PageProvider.h"
#include "IStrategyUtil.h"
#include "PageSocketStruct.h"
#include "UnitDeclare.h"

UNIT_NAMESPACE_START

class StrategySocketHandler : public IStrategyUtil, public ClientPageProvider
{
public:
    /*constructor with strategy name*/
    explicit StrategySocketHandler(const std::string& strategyName);

    ~StrategySocketHandler() override ;

    bool RegisterStrategy(int& ridStart, int& ridEnd) override ;

    bool TdConnect(const std::string& config) override ;

    bool MdSubscribeTicker(const std::string& tickers, uint8_t source) override ;

    bool MdSubscribeKline(const std::string& symbol, int klineType, uint8_t source) override ;

    bool MdSubscribeDepth(const std::string& symbol, uint8_t source) override ;

private:
    void UnSubAll();

    std::unordered_map<uint8_t, std::unordered_set<std::string> > mSubedTicker;
    std::unordered_map<uint8_t, std::unordered_map<std::string, std::unordered_set<int> > > mSubedKline;
    std::unordered_map<uint8_t, std::unordered_set<std::string> > mSubedDepth;
};

DECLARE_PTR(StrategySocketHandler);

UNIT_NAMESPACE_END

#endif //DIGITALCURRENCYSTRATEGYSYSTEM_STRATEGYSOCKETHANDLER_H
