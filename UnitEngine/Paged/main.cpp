//
// Created by wangzhen on 18-7-11.
//

#include "PageEngine.h"

USING_UNIT_NAMESPACE

int main()
{
    std::shared_ptr<PageEngine> engine(new PageEngine());
    PstTempPagePtr tempPageTask(new PstTempPage(engine.get()));
    engine->AddTask(tempPageTask);
    engine->SetFreq(0.01);
    engine->Start();

    while (1)
    {
        sleep(1);
    }
}