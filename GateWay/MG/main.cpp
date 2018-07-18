//
// Created by wangzhen on 18-6-15.
//

#include <thread>
#include "IMGEngine.h"

std::string jj = "{\"name\":\"ok_mg\",\"source\":1}";

int main()
{
    std::shared_ptr<IMGEngine> p(new IMGEngine());

    p->Initialize(jj);
    p->Start();
    p->WaitForStop();
}