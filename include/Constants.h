//
// Created by wangzhen on 18-6-7.
//

#ifndef DCSS_CONSTANTS_H
#define DCSS_CONSTANTS_H

// sources
#define EXCHANGE_OKCOIN     1
#define EXCHANGE_BINANCE    2

#define MAX_CURRENCY_NUM 500

enum class Source
{
    Okex,
    Binance
};

enum class GWStatus
{
    Unknown,
    Added,
    Requested,
    Connected,
    Logined,
    Disconnected,
    Loginfailed
};

enum class OrderDirection
{
    Buy,
    Sell
};

enum class OrderType
{
    Limit,
    Market
};

enum class KlineType
{
    Min1,
    Min3,
    Min5,
    Min15,
    Min30,
    Hour1,
    Hour2,
    Hour4,
    Hour6,
    Hour8,
    Hour12,
    Day1,
    Day3,
    Week1,
    Month1
};

enum class OrderStatus
{
    Submitted,
    PartTraded,
    AllTraded,
    Canceling,
    AllCanceled,
    Rejected,
    Expired
};




#endif //DEMO_CONSTANTS_H
