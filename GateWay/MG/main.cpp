//
// Created by wangzhen on 18-6-15.
//

#include <thread>
#include "IMGEngine.h"

using namespace DCSS::MG;
using namespace DCSS;

int main()
{
    std::shared_ptr<IMGEngine> p = IMGEngine::CreateMGEngine();

    p->Connect();

    while (!p->IsConnected())
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    char10 s = "bch_btc";

    p->ReqSubDepth(s, 5);

    std::this_thread::sleep_for(std::chrono::seconds(10));

    p->ReqUnSubDepth(s, 5);

    while(1)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}