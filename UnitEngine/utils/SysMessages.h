//
// Created by wangzhen on 18-6-27.
//

#ifndef DIGITALCURRENCYSTRATEGYSYSTEM_SYSMESSAGES_H
#define DIGITALCURRENCYSTRATEGYSYSTEM_SYSMESSAGES_H

/*10-1f for strategy*/
const short MSG_TYPE_STRATEGY_START         = 0x0010;
const short MSG_TYPE_STRATEGY_END           = 0x0011;
const short MSG_TYPE_TRADE_ENGINE_LOGIN     = 0x0012;
const short MSG_TYPE_TRADE_ENGINE_ACK       = 0x0013;

/*20-2f for service*/
const short MSG_TYPE_PAGED_START            = 0x0020;
const short MSG_TYPE_PAGED_END              = 0x0021;

/*30-3f for control*/
const short MSG_TYPE_TRADE_ENGINE_OPEN      = 0x0030;
const short MSG_TYPE_TRADE_ENGINE_CLOSE     = 0x0031;
const short MSG_TYPE_MD_ENGINE_OPEN         = 0x0032;
const short MSG_TYPE_MD_ENGINE_CLOSE        = 0x0033;
const short MSG_TYPE_SWITCH_TRADING_DAY     = 0x0034;
const short MSG_TYPE_STRING_COMMAND         = 0x0035;

const short MSG_TYPE_REQ_ORDER_INSERT       = 0x0103;
const short MSG_TYPE_RSP_ORDER_INSERT       = 0X0104;
const short MSG_TYPE_RTN_ORDER              = 0x0105;
const short MSG_TYPE_RTN_TRADE              = 0x0106;
const short MSG_TYPE_REQ_ORDER_ACTION       = 0x0107;
const short MSG_TYPE_RSP_ORDER_ACTION       = 0x0108;
const short MSG_TYPE_REQ_QRY_ACCOUNT        = 0x0109;
const short MSG_TYPE_RSP_QRY_ACCOUNT        = 0x010a;
const short MSG_TYPE_REQ_QRY_TICKER         = 0x010b;
const short MSG_TYPE_RSP_QRY_TICKER         = 0x010c;
const short MSG_TYPE_REQ_QRY_KLINE          = 0x010d;
const short MSG_TYPE_RSP_QRY_KLINE          = 0x010e;
const short MSG_TYPE_RTN_BALANCE            = 0x010f;
const short MSG_TYPE_RTN_TD_STATUS          = 0x0110;
const short MSG_TYPE_REQ_QRY_SIGNAL_ORDER   = 0x0111;
const short MSG_TYPE_REQ_QRY_OPEN_ORDER     = 0x0112;
const short MSG_TYPE_RSP_QRY_SIGNAL_ORDER   = 0x0113;
const short MSG_TYPE_RSP_QRY_OPEN_ORDER     = 0x0114;
const short MSG_TYPE_REQ_QRY_SYMBOL         = 0X0115;
const short MSG_TYPE_RSP_QRY_SYMBOL         = 0X0116;

/*market data*/
const short MSG_TYPE_SUB_TICKER             = 0x0201;
const short MSG_TYPE_SUB_KLINE              = 0x0202;
const short MSG_TYPE_SUB_DEPTH              = 0x0203;
const short MSG_TYPE_UNSUB_TICKER           = 0x0204;
const short MSG_TYPE_UNSUB_KLINE            = 0x0205;
const short MSG_TYPE_UNSUB_DEPTH            = 0x0206;
const short MSG_TYPE_RTN_TICKER             = 0x0207;
const short MSG_TYPE_RTN_KLINE              = 0x0208;
const short MSG_TYPE_RTN_DEPTH              = 0x0209;

// 50 - 89 utilities
const short MSG_TYPE_TIME_TICK              = 50;
const short MSG_TYPE_SUBSCRIBE_MARKET_DATA  = 51;
const short MSG_TYPE_SUBSCRIBE_L2_MD        = 52;
const short MSG_TYPE_SUBSCRIBE_INDEX        = 53;
const short MSG_TYPE_SUBSCRIBE_ORDER_TRADE  = 54;
const short MSG_TYPE_ENGINE_STATUS          = 60;
// 90 - 99 memory alert
const short MSG_TYPE_MEMORY_FROZEN          = 90; // UNLESS SOME MEMORY UNLOCK, NO MORE LOCKING
#endif //DIGITALCURRENCYSTRATEGYSYSTEM_SYSMESSAGES_H
