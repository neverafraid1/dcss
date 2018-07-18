//
// Created by wangzhen on 18-7-4.
//

#include "IDCSSStrategy.h"

DCSSSymbolField symbol;

class Strategy : public IDCSSStrategy
{
public:
    Strategy(const std::string& name);
    void Init()
    {
        mData->AddRegisterTd(EXCHANGE_OKCOIN);
        mData->AddMarketData(EXCHANGE_OKCOIN);


        mUtil->MdSubscribe({symbol}, EXCHANGE_OKCOIN);
    }
};

Strategy::Strategy(const std::string& name)
: IDCSSStrategy(name)
{ }

int main()
{
    strcpy(symbol.Base, "bch");
    strcpy(symbol.Quote, "btc");
    Strategy str("cpp_test");
    str.Init();

    str.Start();
    str.Block();

    return 0;
}