//
// Created by wangzhen on 18-6-7.
//

#ifndef DCSS_CONSTANTS_H
#define DCSS_CONSTANTS_H

const short EXCHANGE_OKCOIN = 0x0001;

//const short MSG_TYPE_REQ_QRY_POS        = 0x0101;
//const short MSG_TYPE_RSP_QRY_POS        = 0x0102;
const short MSG_TYPE_REQ_ORDER_INSERT   = 0x0103;
const short MSG_TYPE_RSP_ORDER_INSERT   = 0X0104;
const short MSG_TYPE_RTN_ORDER          = 0x0105;
const short MSG_TYPE_RTN_TRADE          = 0x0106;
const short MSG_TYPE_REQ_ORDER_ACTION   = 0x0107;
const short MSG_TYPE_RSP_ORDER_ACTION   = 0x0108;
const short MSG_TYPE_REQ_QRY_ACCOUNT    = 0x0109;
const short MSG_TYPE_RSP_QRY_ACCOUNT    = 0x010a;
const short MSG_TYPE_REQ_QRY_TICKER     = 0x010b;
const short MSG_TYPE_RSP_QRY_TICKER     = 0x010c;
const short MSG_TYPE_RSP_QRY_ORDER      = 0x010d;
const short MSG_TYPE_REQ_QRY_KLINE      = 0x010e;
const short MSG_TYPE_RSP_QRY_KLINE      = 0x010f;

const short MSG_TYPE_SUB_TICKER         = 0x0201;
const short MSG_TYPE_SUB_KLINE          = 0x0202;
const short MSG_TYPE_SUB_DEPTH          = 0x0203;
const short MSG_TYPE_UNSUB_TICKER       = 0x0204;
const short MSG_TYPE_UNSUB_KLINE        = 0x0205;
const short MSG_TYPE_UNSUB_DEPTH        = 0x0206;
const short MSG_TYPE_RTN_TICKER         = 0x0207;
const short MSG_TYPE_RTN_KLINE          = 0x0208;
const short MSG_TYPE_RTN_DEPTH          = 0x0209;



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
#define ALL_CANCELED        -1
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

#endif //DEMO_CONSTANTS_H
