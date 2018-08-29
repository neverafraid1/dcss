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
	MySqlDao::Instance().Init("ccts", "192.168.1.98", "root", "root", 3306);
    std::shared_ptr<ITGEngine> p(new ITGEngine());
    p->Start();
    p->WaitForStop();
}
