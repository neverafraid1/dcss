//
// Created by wangzhen on 18-6-7.
//

#ifndef DCSS_CONSTANTS_H
#define DCSS_CONSTANTS_H

// sources
#define EXCHANGE_OKCOIN     1

///////////////////////////////////
// KlineTypeType: K线时间周期类型
///////////////////////////////////
//1min
#define KLINE_1MIN          '0'
//3min
#define KLINE_3MIN          '1'
//5min
#define KLINE_5MIN          '2'
//15min
#define KLINE_15MIN         '3'
//30min
#define KLINE_30MIN         '4'
//1hour
#define KLINE_1HOUR         '5'
//2hour
#define KLINE_2HOUR         '6'
//4hour
#define KLINE_4HOUR         '7'
//6hour
#define KLINE_6HOUR         '8'
//12hour
#define KLINE_12HOUR        '9'
//day
#define KLINE_1DAY          'a'
//3day
#define KLINE_3DAY          'b'
//week
#define KLINE_1WEEK         'c'

typedef char KlineTypeType;

///////////////////////////////////
// TradeTypeType: 交易类型类型
///////////////////////////////////
//买入
#define BUY                 '0'
//卖出
#define SELL                '1'
//按市价买入
#define BUY_MARKET          '2'
//按市价卖出
#define SELL_MARKET         '3'

typedef char TradeTypeType;

///////////////////////////////////
// OrderStatusType: 报单状态类型
///////////////////////////////////
//已撤销
#define ALL_CANCELED        (-1)
//等待成交
#define SUBMITTED           0
//部分成交
#define PART_TRADED         1
//完全成交
#define ALL_TRADED          2
//撤单处理中
#define CANCELING           4

typedef int OrderStatusType;

#define MAX_CURRENCY_NUM 500


///////////////////////////////////
// GateWayStatusType: 网关状态类型
///////////////////////////////////
// 未知（初始状态）
#define TD_STATUS_UNKNOWN       '0'
// 已添加
#define TD_STATUS_ADDED         '1'
// 请求连接
#define TD_STATUS_REQUESTED     '2'
// 已连接
#define TD_STATUS_CONNECTED     '3'
// 已登录
#define TD_STATUS_LOGINED       '4'
// 连接断开
#define TD_STATUS_DISCONNECTED  '5'
// 登陆失败
#define TD_STATUS_LOGINFAILED   '6'

typedef unsigned char GateWayStatusType;

#endif //DEMO_CONSTANTS_H
