//
// Created by wangzhen on 18-6-20.
//

#ifndef DIGITALCURRENCYSTRATEGYSYSTEM_STRATEGYSOCKETHANDLER_H
#define DIGITALCURRENCYSTRATEGYSYSTEM_STRATEGYSOCKETHANDLER_H

#include "PageProvider.h"
#include "IStrategyUtil.h"
#include "PageSocketStruct.h"
#include "Declare.h"
#include <array>
#include "UnitDeclare.h"

UNIT_NAMESPACE_START

class StrategySocketHandler : public IStrategyUtil, public ClientPageProvider
{
public:
    /*constructor with strategy name*/
    explicit StrategySocketHandler(const std::string& strategyName);

    bool RegisterStrategy(int& ridStart, int& ridEnd) override ;

    bool TdConnect(short source) override ;

    bool MdSubscribe(const std::vector<DCSSSymbolField>& tickers, short source) override ;

    bool MdSubscribeKline(const DCSSSymbolField& symbol, KlineTypeType klineType, short source) override ;

    bool MdSubscribeDepth(const DCSSSymbolField& symbol, int depth, short source) override ;
};

DECLARE_PTR(StrategySocketHandler);

UNIT_NAMESPACE_END

#endif //DIGITALCURRENCYSTRATEGYSYSTEM_STRATEGYSOCKETHANDLER_H
