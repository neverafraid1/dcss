//
// Created by wangzhen on 18-6-7.
//

#ifndef DCSS_CONSTANTS_H
#define DCSS_CONSTANTS_H

enum ExchangeEnum : unsigned char
{
    Min,

    Okex,
    Binance,

    Max
};

enum class GWStatus
{
    Min,

    Unknown,
    Added,
    Requested,
    Connected,
    Logined,
    Disconnected,
    Loginfailed,

    Max
};

enum class OrderDirection
{
    Min,

    Buy,
    Sell,

    Max
};

enum class OrderType
{
    Min,

    Limit,
    Market,

    Max
};

enum class KlineType
{
    Min,

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
    Month1,

    Max
};

enum class OrderStatus
{
    Min,

    Submitted,
    PartTraded,
    AllTraded,
    Canceling,
    AllCanceled,
    Rejected,
    Expired,

    Max
};

#endif //DEMO_CONSTANTS_H
