//
// Created by wangzhen on 18-6-15.
//

#include <thread>
#include <fstream>
#include "IMGEngine.h"

int main(int argc, char* argv[])
{
    std::ifstream in(argv[1]);
    std::ostringstream oss;
    oss << in.rdbuf();

    std::shared_ptr<IMGEngine> p(new IMGEngine());
    p->Initialize(oss.str());
    p->Start();
    p->WaitForStop();
}