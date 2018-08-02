//
// Created by wangzhen on 18-6-7.
//

#include <unistd.h>
#include <thread>
#include <iostream>
#include <fstream>
#include "ITGEngine.h"

int main(int argc, char* argv[])
{
    std::ifstream in(argv[1]);
    std::ostringstream oss;
    oss << in.rdbuf();

    std::shared_ptr<ITGEngine> p(new ITGEngine);
    p->Initialize(oss.str());
    p->Start();
    p->WaitForStop();
}