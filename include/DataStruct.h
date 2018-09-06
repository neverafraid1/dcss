//
// Created by wangzhen on 18-6-7.
//

#ifndef DCSS_DATASTRUCT_H
#define DCSS_DATASTRUCT_H

#include "Constants.h"
#include <string>
#include <cstring>

typedef char char9[9];
typedef char char10[10];
typedef char char11[11];
typedef char char18[18];
typedef char char21[21];
typedef char char5[5];
typedef long long3[3];

/**
 * attention:
 * if you have a c++ composite object (etc : std::string, std::vector) in a struct
 * or it's a class with virtual function table, DONOT use memset(0) in construction function!!!
 */

struct DCSSTickerField
{
    // 币对   btc_ltc
    char21 Symbol;
	// 更新时间
	long UpdateTime;
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

struct DCSSKlineField
{
    // 币对
    char21 Symbol;
    // K线时间周期
    KlineType Type;
    // 时间 timestamp * 1000 + millisec
    long UpdateTime;
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
    // K线开始时间
    long StartTime;
    // K线结束时间
    long CloseTime;

    DCSSKlineField()
    {
        memset(this, 0, sizeof(DCSSKlineField));
    }
};

struct RspUserLogin
{
	// 登陆成功
    bool Success;
};

struct DCSSSignalDepthField
{
	// 价格
	double Price;
	// 报单量
	double Volume;
};

#define MAX_DEPTH_NUM 100

struct DCSSDepthField
{
	// 币对
	char21 Symbol;
	// 更新时间
	long UpdateTime;
	// 卖方深度
	DCSSSignalDepthField AskDepth[MAX_DEPTH_NUM];
	// 买方深度
	DCSSSignalDepthField BidDepth[MAX_DEPTH_NUM];

    DCSSDepthField()
    {
        memset(this, 0, sizeof(DCSSDepthField));
    }
};

struct DCSSBalanceField
{
	// 币种
    char5 Currency;
    // 可用量
    double Free;
    // 冻结量
    double Freezed;
};

#define MAX_CURRENCY_NUM 500

struct DCSSTradingAccountField
{
	// 资金信息
    DCSSBalanceField Balance[MAX_CURRENCY_NUM];

    DCSSTradingAccountField()
    {
        memset(this, 0, sizeof(DCSSTradingAccountField));
    }

};

struct DCSSReqQryTickerField
{
	// 币对如ltc_btc
    char21 Symbol;

    DCSSReqQryTickerField()
    {
        memset(this, 0, sizeof(DCSSReqQryTickerField));
    }
};

struct DCSSReqQryKlineField
{
    // 币对如ltc_btc
    char21 Symbol;
    // K线时间周期类型
    KlineType Type;
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
    char21 Symbol;
    // 方向
    OrderDirection Direction;
    // 买卖类型
    OrderType Type;
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
    char21 Symbol;
    // 订单ID
    long OrderID;

    DCSSReqCancelOrderField()
    {
        memset(this, 0, sizeof(DCSSReqCancelOrderField));
    }
};

struct DCSSRspCancelOrderField
{
    // 订单ID
    long OrderID;

    DCSSRspCancelOrderField()
    {
        memset(this, 0, sizeof(DCSSRspCancelOrderField));
    }
};

struct DCSSReqQryOrderField
{
    // 币对如ltc_btc
    char21 Symbol;
    // 订单ID -1:未完成订单，否则查询相应订单号的订单
    long OrderID;

    DCSSReqQryOrderField()
    {
        memset(this, 0, sizeof(DCSSReqQryOrderField));
    }
};

struct DCSSOrderField
{
    // 币对如ltc_btc
    char21 Symbol;
    // 插入时间
    long InsertTime;
    // 订单id
    long OrderID;
    // 买卖方向
    OrderDirection Direction;
    // 报单类型
    OrderType Type;
    // 初始报单数量
    double OriginQuantity;
    // 已完成成交量
    double ExecuteQuantity;
    // 报单价格
    double Price;
    // 更新时间
    long UpdateTime;
    // 报单状态
    OrderStatus Status;

    DCSSOrderField()
    {
        memset(this, 0, sizeof(DCSSOrderField));
    }
};

struct DCSSSubTickerField
{
	// 币对如ltc_btc
    char21 Symbol;

    DCSSSubTickerField()
    {
        memset(this, 0, sizeof(DCSSSubTickerField));
    }
};

struct DCSSSubDepthField
{
	// 币对如ltc_btc
    char21 Symbol;
    // 深度
    int Depth;

    DCSSSubDepthField()
    {
        memset(this, 0, sizeof(DCSSSubDepthField));
    }
};

struct DCSSSubKlineField
{
	// 币对如ltc_btc
    char21 Symbol;
    // K线类型
    KlineType Type;

    DCSSSubKlineField()
    {
        memset(this, 0, sizeof(DCSSSubKlineField));
    }
};

#endif //DCSS_DATASTRUCT_H
