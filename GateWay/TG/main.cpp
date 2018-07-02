//
// Created by wangzhen on 18-6-7.
//

#include <unistd.h>
#include <thread>
#include "ITGEngine.h"

int main()
{
    int requestID = 1;

    std::shared_ptr<DCSS::ITGEngine> p = DCSS::ITGEngine::CreateTGEngine();
    p->Connect();
    p->Login();
//    DCSS::char10 symbol;
//    bzero(symbol, 10);
//    memcpy(symbol, "ltc_btc", 7);

    while (!p->IsLogged())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    DCSS::DCSSReqInsertOrderField req;
    memcpy(req.Symbol, "ltc_btc", 7);
    req.TradeType = BUY;
    req.Price = 1.1;
    req.Aumount = 0.1;

    p->ReqQryUserInfo(requestID++);

    while(1)
    {
        sleep(1);
    }

}