//
// Created by wangzhen on 18-6-27.
//

#ifndef DIGITALCURRENCYSTRATEGYSYSTEM_SYSMESSAGES_H
#define DIGITALCURRENCYSTRATEGYSYSTEM_SYSMESSAGES_H

/*10-19 for strategy*/
const short MSG_TYPE_STRATEGY_START = 10;
const short MSG_TYPE_STRATEGY_END = 11;
const short MSG_TYPE_TRADE_ENGINE_LOGIN = 12;
const short MSG_TYPE_TRADE_ENGINE_ACK = 13;

/*20-29 for service*/
const short MSG_TYPE_PAGED_START = 20;
const short MSG_TYPE_PAGED_END = 21;

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
const short MSG_TYPE_RTN_BALANCE        = 0x0110;
const short MSG_TYPE_RTN_TD_STATUS      = 0x0111;

/*market data*/
const short MSG_TYPE_SUB_TICKER         = 0x0201;
const short MSG_TYPE_SUB_KLINE          = 0x0202;
const short MSG_TYPE_SUB_DEPTH          = 0x0203;
const short MSG_TYPE_UNSUB_TICKER       = 0x0204;
const short MSG_TYPE_UNSUB_KLINE        = 0x0205;
const short MSG_TYPE_UNSUB_DEPTH        = 0x0206;
const short MSG_TYPE_RTN_TICKER         = 0x0207;
const short MSG_TYPE_RTN_KLINE          = 0x0208;
const short MSG_TYPE_RTN_DEPTH          = 0x0209;

/*30-39 for control*/
const short MSG_TYPE_TRADE_ENGINE_OPEN = 30;
const short MSG_TYPE_TRADE_ENGINE_CLOSE = 31;
const short MSG_TYPE_MD_ENGINE_OPEN = 32;
const short MSG_TYPE_MD_ENGINE_CLOSE = 33;
const short MSG_TYPE_SWITCH_TRADING_DAY = 34;
const short MSG_TYPE_STRING_COMMAND = 35;

// 50 - 89 utilities
const short MSG_TYPE_TIME_TICK = 50;
const short MSG_TYPE_SUBSCRIBE_MARKET_DATA = 51;
const short MSG_TYPE_SUBSCRIBE_L2_MD = 52;
const short MSG_TYPE_SUBSCRIBE_INDEX = 53;
const short MSG_TYPE_SUBSCRIBE_ORDER_TRADE = 54;
const short MSG_TYPE_ENGINE_STATUS = 60;
// 90 - 99 memory alert
const short MSG_TYPE_MEMORY_FROZEN = 90; // UNLESS SOME MEMORY UNLOCK, NO MORE LOCKING
#endif //DIGITALCURRENCYSTRATEGYSYSTEM_SYSMESSAGES_H
