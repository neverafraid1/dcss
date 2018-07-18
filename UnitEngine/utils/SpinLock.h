//
// Created by wangzhen on 18-7-5.
//

#ifndef DIGITALCURRENCYSTRATEGYSYSTEM_SPINLOCK_H
#define DIGITALCURRENCYSTRATEGYSYSTEM_SPINLOCK_H

#include <atomic>

class SpinLock
{
public:
    SpinLock() = default;
    SpinLock(const SpinLock&) = delete;
    SpinLock& operator=(const SpinLock) = delete;

    void Lock()
    {
        bool expected = false;
        while (!mFlag.compare_exchange_strong(expected, true))
            expected = false;
    }

    void UnLock()
    {
        mFlag.store(false);
    }

private:
    std::atomic<bool> mFlag = ATOMIC_VAR_INIT(false);
};

#endif //DIGITALCURRENCYSTRATEGYSYSTEM_SPINLOCK_H
