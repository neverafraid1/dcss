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

	const std::string KLINE("/api/v1/klines");

	const std::string PRICE("/api/v3/ticker/price");

	const std::string ORDER("/api/v3/order");

	const std::string ACCOUNT_INFO("/api/v3/account");

	const std::string USER_DATA_STREAM("/api/v1/userDataStream");

	const std::string X_MBX_APIKEY("X-MBX-APIKEY");

}  // namespace BinanceConstant

#endif /* GATEWAY_MG_BINANCE_BINANCECONSTANT_H_ */
