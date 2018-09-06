/*
 * OkexConstant.h
 *
 *  Created on: 2018年9月3日
 *      Author: wangzhen
 */

#ifndef GATEWAY_COMMON_OKEXCONSTANT_H_
#define GATEWAY_COMMON_OKEXCONSTANT_H_

#include <string>

namespace OkexConstant {

	const std::string WS_API_BASE_URL("wss://real.okex.com:10441/websocket");

	const std::string API_BASE_URL("https://www.okex.com/api/v1");

	const std::string TICKER("ticker.do");

	const std::string DEPTH("depth.do");

	const std::string KLINE("kline.do");

	const std::string USER_INFO("userinfo.do");

	const std::string TRADE("trade.do");

	const std::string CANCEL_ORDER("cancel_order.do");

	const std::string ORDER_INFO("order_info.do");

	const std::string EVENT("event");

	const std::string ERROR_CODE("error_code");

}  // namespace OkexConstant



#endif /* GATEWAY_COMMON_OKEXCONSTANT_H_ */
