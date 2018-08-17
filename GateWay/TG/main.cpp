//
// Created by wangzhen on 18-6-7.
//

#include <unistd.h>
#include <thread>
#include <iostream>
#include <fstream>
#include "ITGEngine.h"
#include "Dao.hpp"

int main(int argc, char* argv[])
{
    try
    {
        MySqlDao::Instance().Init("ccts", "192.168.1.98", "root", "root", 3306);
    }
    catch (const std::exception& e)
    {
        std::cerr << "cannot connect to db" << std::endl;
        return 0;
    }

    std::string proxy(argv[1]);
    std::shared_ptr<ITGEngine> p(new ITGEngine());
    p->SetProxy(proxy);
    p->Start();
    p->WaitForStop();
}