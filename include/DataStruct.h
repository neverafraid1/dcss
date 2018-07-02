//
// Created by wangzhen on 18-6-7.
//

#ifndef DCSS_PagePtrUCT_H
#define DCSS_DATASTRUCT_H

#include "Constants.h"
#include <string>
#include <cstring>

typedef char char9[9];
typedef char char10[10];
typedef char char11[11];
typedef char char18[18];
typedef char char5[5];
typedef long long3[3];

struct DCSSSymbolField
{
    char5 Base;

    char5 Quote;

    char10 Exchange;
};

struct DCSSTickerField
{
    // 日期 YYYYMMDD
    char10 Date;
    // 时间 hh:mm:ss
    char10 Time;
    // 毫秒
    int MilliSec;
    // 币对
    char10 Symbol;
    // 买一价
    double BuyPrice;
    // 卖一价
    double SellPrice;
    // 最高价
    double Highest;
    // 最低价
    double Lowest;
    // 最新成交价
    double LastPrice;
    // 成交量(最近的24小时)
    double Volume;

    DCSSTickerField()
    {
        memset(this, 0, sizeof(DCSSTickerField));
    }
};


struct DCSSKlineHeaderField
{
    // 币对
    char10 Symbol;
    // K线时间周期
    KlineTypeType KlineType;
    // K线条数
    int Size;

    DCSSKlineHeaderField()
    {
        memset(this, 0, sizeof(DCSSKlineHeaderField));
    }
};

struct DCSSKlineField
{
    // 日期 YYYYMMDD
    char10 Date;
    // 时间 hh:mm:ss
    char10 Time;
    // 毫秒
    int Millisec;
    // 开盘价
    double OpenPrice;
    // 最高价
    double Highest;
    // 最低价
    double Lowest;
    // 收盘价
    double ClosePrice;
    // 成交量
    double Volume;

    DCSSKlineField()
    {
        memset(this, 0, sizeof(DCSSKlineField));
    }
};

struct RspUserLogin
{
    bool Success;
};

struct BalanceField
{

};

struct DCSSDepthHeaderField
{
    char10 Symbol;
    // 日期 YYYYMMDD
    char10 Date;
    // 时间 hh:mm:ss
    char10 Time;
    // 毫秒
    int Millisec;

    int AskNum;

    int BidNum;

    DCSSDepthHeaderField()
    {
        memset(this, 0, sizeof(DCSSDepthHeaderField));
    }
};

struct DCSSDepthField
{
    double Price;

    double Volume;

    DCSSDepthField()
    {
        memset(this, 0, sizeof(DCSSDepthField));
    }
};

struct DCSSUserInfoField
{
    struct Fund
    {
        char5 Currency;
        double Balance;
    };

    Fund Free[MAX_CURRENCY_NUM];
    Fund Freezed[MAX_CURRENCY_NUM];

};

struct DCSSReqQryTickerField
{
    char10 Symbol;

    DCSSReqQryTickerField()
    {
        memset(this, 0, sizeof(DCSSReqQryTickerField));
    }
};

struct DCSSReqQryKlineField
{
    // 币对如ltc_btc
    char10 Symbol;
    // K线时间周期类型
    KlineTypeType KlineType;
    // 指定获取数据的条数 非必填(默认全部获取)
    int Size;
    // 时间戳 非必填(默认全部获取)
    long Since;

    DCSSReqQryKlineField()
    {
        memset(this, 0, sizeof(DCSSReqQryKlineField));
    }
};

struct DCSSReqInsertOrderField
{
    // 币对如ltc_btc
    DCSSSymbolField Symbol;
    // 买卖类型
    TradeTypeType TradeType;
    // 下单价格 市价单不传price
    double Price;
    // 交易数量
    double Amount;

    DCSSReqInsertOrderField()
    {
        memset(this, 0, sizeof(DCSSReqInsertOrderField));
    }
};

struct DCSSRspInsertOrderField
{
    // 订单交易成功或失败
    bool Result;
    // 订单ID
    long OrderID;

    DCSSRspInsertOrderField()
            :Result(false), OrderID(0)
    { }
};

struct DCSSReqCancelOrderField
{
    // 币对如ltc_btc
    char10 Symbol;
    // 订单ID(一次最多取消三笔订单)
    long3 OrderID;

    DCSSReqCancelOrderField()
    {
        memset(this, 0, sizeof(DCSSReqCancelOrderField));
    }
};

struct DCSSRspCancelOrderField
{
    // 撤单请求成功的订单ID，等待系统执行撤单
    long3 SuccessID;
    // 撤单请求失败的订单ID
    long3 ErrorID;

    DCSSRspCancelOrderField()
    {
        memset(this, 0, sizeof(DCSSRspCancelOrderField));
    }
};

struct DCSSReqQryOrderField
{
    // 币对如ltc_btc
    char10 Symbol;
    // 订单ID -1:未完成订单，否则查询相应订单号的订单
    long OrderID;

    DCSSReqQryOrderField()
    {
        memset(this, 0, sizeof(DCSSReqQryOrderField));
    }
};

struct DCSSRspQryOrderHeaderField
{
    char10 Symbol;

    int Size;

    DCSSRspQryOrderHeaderField()
    {
        memset(this, 0, sizeof(DCSSRspQryOrderHeaderField));
    }
};

struct DCSSRspQryOrderField
{
    // 币对如ltc_btc
    char10 Symbol;
    // 委托数量
    double Amount;
    // 委托日期
    char10 CreateDate;
    // 委托时间
    char10 CreateTime;
    // 毫秒
    int MilliSec;
    // 平均成交价
    double AvgPrice;
    // 成交数量
    double DealAmount;
    // 订单ID
    long OrderID;
    // 订单ID(不建议使用)
    long OrdersID;
    // 委托价格
    double Price;
    // 报单状态
    OrderStatusType OrderStatus;
    // 交易类型
    TradeTypeType TradeType;

    DCSSRspQryOrderField()
    {
        memset(this, 0, sizeof(DCSSRspQryOrderField));
    }
};

struct DCSSOrderField
{
    // 币对如ltc_btc
    char10 Symbol;
    // 委托日期
    char10 CreateDate;
    // 委托时间
    char10 CreateTime;
    // 毫秒
    int Millisec;
    // 订单id
    long OrderID;
    // 交易类型
    TradeTypeType TradeType;
    // 单笔成交数量
    double SigTradeAmout;
    // 单笔成交价格
    double SigTradePrice;
    // 委托数量（市价卖代表要卖总数量；限价单代表委托数量）
    double TradeAmount;
    // 委托价格（市价买单代表购买总金额； 限价单代表委托价格）
    double TradeUnitPrice;
    // 已完成成交量
    double CompletedTradeAmount;
    // 成交金额
    double TradePrice;
    // 平均成交价
    double AveragePrice;
    // 当按市场价买币时表示剩余金额，其他情况表示此笔交易剩余买/卖币的数量
    double UnTrade;
    // 报单状态
    OrderStatusType OrderStatus;

    DCSSOrderField()
    {
        memset(this, 0, sizeof(DCSSOrderField));
    }
};

struct DCSSBalanceField
{

};

struct DCSSSubTickerField
{
    char10 Symbol;

    DCSSSubTickerField()
    {
        memset(this, 0, sizeof(DCSSSubTickerField));
    }
};

struct DCSSSubDepthField
{
    char10 Symbol;

    int Depth;

    DCSSSubDepthField()
    {
        memset(this, 0, sizeof(DCSSSubDepthField));
    }
};

struct DCSSSubKlineField
{
    char10 Symbol;

    KlineTypeType KlineType;

    DCSSSubKlineField()
    {
        memset(this, 0, sizeof(DCSSSubKlineField));
    }
};

#endif //DEMO_DATASTRUCT_H
