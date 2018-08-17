//
// Created by wangzhen on 18-7-4.
//

#include <iostream>
#include <deque>
#include <ta-lib/ta_func.h>
#include <iomanip>

#include "IDCSSStrategy.h"
#include "json.hpp"

std::string symbol("btc_usdt");

TA_Integer fastPeriod(12);
TA_Integer slowPeriod(26);
TA_Integer signalPeriod(9);

class Strategy : public IDCSSStrategy
{
public:
    Strategy(const std::string& name);

    void OnRspQryKline(const DCSSKlineHeaderField* header, const std::vector<const DCSSKlineField*>& kline,
            int requestId, int errorId, const char* errorMsg, uint8_t source, long recvTime) final;


    void OnRtnKline(const DCSSKlineHeaderField* header, const std::vector<const DCSSKlineField* >& kline,
            uint8_t source, long recvTime) final;
private:
    std::deque<double> closePirces;

};

Strategy::Strategy(const std::string& name)
: IDCSSStrategy(name)
{ }

void Strategy::OnRspQryKline(const DCSSKlineHeaderField* header, const std::vector<const DCSSKlineField*>& kline,
        int requestId, int errorId, const char* errorMsg, uint8_t source, long recvTime)
{
    if (errorId == 0)
    {
        for (const auto& item : kline)
        {
            std::cout << item->ClosePrice << std::endl;
            closePirces.push_back(item->ClosePrice);
        }
    }


}

void Strategy::OnRtnKline(const DCSSKlineHeaderField* header, const std::vector<const DCSSKlineField*>& kline,
        uint8_t source, long recvTime)
{
    for (auto& item : kline)
    {
        closePirces.pop_front();
        closePirces.push_back(item->ClosePrice);
    }

    std::cout << kline.size() << "\ttime:" << kline.front()->Time << "\tMillisec:" << kline.front()->Millisec << "\topen:" << kline.front()->OpenPrice <<
    "\thigh:" << kline.front()->Highest << "\tlow:" << kline.front()->Lowest << "\tclose:" << kline.front()->ClosePrice << "\tvolume:" << kline.front()->Volume << std::endl;

    int beginIndex = closePirces.size() - slowPeriod - 1;
    int endIndex = closePirces.size();
    int arraySize = endIndex - beginIndex + 1;

    TA_Real data[closePirces.size()];
    for (int index = 0; index < closePirces.size(); ++index)
        data[index] = closePirces.at(index);

    TA_Real macd[1];
    TA_Real signal[1];
    TA_Real his[1];

    TA_Integer outbegin(0), outele(0);

    TA_RetCode retCode = TA_MACD(0, 34, data,
            fastPeriod, slowPeriod, signalPeriod,
            &outbegin, &outele,
            macd, signal, his);

    if (retCode == TA_SUCCESS)
    {
        for (auto& item : macd)
            std::cout << std::setiosflags(std::ios::fixed) << *macd << std::endl;
    }


}

Strategy strategy("cpp_test");

int main(int argc, char* argv[])
{
    strategy.SetConfigPath(argv[1]);

    strategy.Init({EXCHANGE_OKCOIN}, {});

    strategy.Start();

    strategy.QryKline(EXCHANGE_OKCOIN, symbol, KLINE_1MIN, 35);

    strategy.SubscribeKline(EXCHANGE_OKCOIN, symbol, KLINE_1MIN);

    strategy.Block();

    return 0;
}