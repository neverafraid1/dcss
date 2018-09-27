//
// Created by wangzhen on 18-6-7.
//

#ifndef DCSS_DATASTRUCT_H
#define DCSS_DATASTRUCT_H

#include "Constants.h"
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
    // 币对如btc_ltc
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
    : Symbol{}, UpdateTime(0), BuyPrice(0.0), SellPrice(0.0), Highest(0.0), Lowest(0.0), LastPrice(0.0), Volume(0.0) {}
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
    : Symbol{0}, Type(KlineType::Min), UpdateTime(0), OpenPrice(0.0), Highest(0.0), Lowest(0.0), ClosePrice(0.0), Volume(0.0), StartTime(0), CloseTime(0) {}
};

struct RspUserLogin
{
	// 登陆成功
    bool Success;

    RspUserLogin()
    : Success(false) {}
};

struct DCSSSignalDepthField
{
	// 价格
	double Price;
	// 报单量
	double Volume;

	DCSSSignalDepthField()
	: Price(0.0), Volume(0.0) {}
};

#define MAX_DEPTH_NUM 100

struct DCSSDepthField
{
	// 币对
	char21 Symbol;
	// 更新时间
	long UpdateTime;

	long SendTime;
	// 卖方深度
	DCSSSignalDepthField AskDepth[MAX_DEPTH_NUM];
	// 买方深度
	DCSSSignalDepthField BidDepth[MAX_DEPTH_NUM];

	DCSSDepthField()
	: Symbol{0}, UpdateTime(0) {}
};

struct DCSSBalanceField
{
	// 币种
    char5 Currency;
    // 可用量
    double Free;
    // 冻结量
    double Freezed;

    DCSSBalanceField()
    : Currency{0}, Free(0.0), Freezed(0.0) {}
};

#define MAX_CURRENCY_NUM 500

struct DCSSTradingAccountField
{
	// 资金信息
    DCSSBalanceField Balance[MAX_CURRENCY_NUM];
};

struct DCSSCurrencyField
{
    // 基准资产
    char10 BaseCurrency;
    // 标价资产
    char10 QuoteCurrecy;

    DCSSCurrencyField()
    : BaseCurrency{0}, QuoteCurrecy{0} {}
};

struct DCSSSymbolField
{
    // 交易所（OK/Bina）
    ExchangeEnum Exchange;
    // 币对
    char21 Symbol;
    // 资产
    DCSSCurrencyField Currency;
    // 下单价格精度
    int PricePrecision;
    // 下单数量精度
    int AmountPrecision;
    // 最小下单量
    double MinAmount;

    DCSSSymbolField()
    : Exchange(ExchangeEnum::Min), Symbol{0}, PricePrecision(0), AmountPrecision(0), MinAmount(0.0) {}
};

struct DCSSReqQrySymbolField
{
    char21 Symbol;

    DCSSReqQrySymbolField()
    : Symbol{0} {}
};

struct DCSSReqQryTickerField
{
	// 币对如ltc_btc
    char21 Symbol;

    DCSSReqQryTickerField()
    : Symbol{0} {}
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
    : Symbol{0}, Type(KlineType::Min), Size(0), Since(0) {}
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
    : Symbol{0}, Direction(OrderDirection::Min), Type(OrderType::Min), Price(0.0), Amount(0.0) {}
};

struct DCSSRspInsertOrderField
{
    // 订单交易成功或失败
    bool Result;
    // 订单ID
    long OrderID;

    DCSSRspInsertOrderField()
    : Result(false), OrderID(0) {}
};

struct DCSSReqCancelOrderField
{
    // 币对如ltc_btc
    char21 Symbol;
    // 订单ID
    long OrderID;

    DCSSReqCancelOrderField()
    : Symbol{0}, OrderID(0) {}
};

struct DCSSRspCancelOrderField
{
    // 订单ID
    long OrderID;

    DCSSRspCancelOrderField()
    : OrderID(0) {}
};

struct DCSSReqQryOrderField
{
    // 币对如ltc_btc
    char21 Symbol;
    // 订单ID -1:未完成订单，否则查询相应订单号的订单
    long OrderID;

    DCSSReqQryOrderField()
    : Symbol{0}, OrderID(0) {}
};

struct DCSSOrderField
{
    // 币对如ltc_btc
    char21 Symbol;
    // 交易所
    ExchangeEnum Exchange;
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
    : Symbol{0}, Exchange(ExchangeEnum::Min), InsertTime(0), OrderID(0), Direction(OrderDirection::Min), Type(OrderType::Min), OriginQuantity(0.0), ExecuteQuantity(0.0), Price(0.0), UpdateTime(0.0), Status(OrderStatus::Min) {}
};

struct DCSSSubTickerField
{
	// 币对如ltc_btc
    char21 Symbol;

    DCSSSubTickerField()
    : Symbol{0} {}
};

struct DCSSSubDepthField
{
	// 币对如ltc_btc
    char21 Symbol;

    DCSSSubDepthField()
    : Symbol{0} {}
};

struct DCSSSubKlineField
{
	// 币对如ltc_btc
    char21 Symbol;
    // K线类型
    KlineType Type;

    DCSSSubKlineField()
    : Symbol{0}, Type(KlineType::Min) {}
};

#endif //DCSS_DATASTRUCT_H
