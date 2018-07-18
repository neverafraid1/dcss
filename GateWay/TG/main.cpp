//
// Created by wangzhen on 18-6-7.
//

#include <unistd.h>
#include <thread>
#include <iostream>
#include "ITGEngine.h"

std::string jj = "{\"name\":\"cpp_test\",\"folder\":\"/home/wangzhen/dcss/unit/strategy\",\"accounts\":[{\"source\":1,\"api_key\":\"1\",\"secret_key\":\"2\"},{\"source\":2,\"api_key\":\"3\",\"secret_key\":\"4\"}]}";

int main()
{
    std::shared_ptr<ITGEngine> p(new ITGEngine);
    p->Initialize(jj);
    p->Start();
    p->WaitForStop();
}