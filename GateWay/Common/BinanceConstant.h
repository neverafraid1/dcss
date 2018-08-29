/*
 * BinanceConstant.h
 *
 *  Created on: 2018年8月24日
 *      Author: wangzhen
 */

#ifndef GATEWAY_MG_BINANCE_BINANCECONSTANT_H_
#define GATEWAY_MG_BINANCE_BINANCECONSTANT_H_

#include <string>

namespace BinanceConstant
{
	const std::string API_BASE_URL("https://api.binance.com");

	const std::string WS_API_BASE_URL("wss://stream.binance.com:9443/ws/");

	const std::string GET_KLINE("/api/v1/klines");

	const std::string GET_PRICE("/api/v3/ticker/price");

	const std::string POST_INSERT_ORDER("/api/v3/order");

	const std::string GET_QRY_ORDER("/api/v3/order");

	const std::string DEL_CANCEL_ORDER("/api/v3/order");

	const std::string GET_ACCOUNT_INFO("/api/v3/account");

	const std::string POST_USER_DATA_STREAM("/api/v1/userDataStream");

	const std::string PUT_USER_DATA_STREAM("/api/v1/userDataStream");

	const std::string DEL_USER_DATA_STREAM("/api/v1/userDataStream");

};



#endif /* GATEWAY_MG_BINANCE_BINANCECONSTANT_H_ */
