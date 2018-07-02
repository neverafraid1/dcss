//
// Created by wangzhen on 18-6-19.
//

#ifndef DIGITALCURRENCYSTRATEGYSYSTEM_PAGE_H
#define DIGITALCURRENCYSTRATEGYSYSTEM_PAGE_H

#include "FrameHeader.h"
#include "Frame.h"
#include "constants.h"
#include <memory>

class Page;
DECLARE_PTR(Page);

class Page
{
public:
    inline void* GetBuffer() const { return buffer; }

    inline short GetPageNum() const { return pageNum; }

    bool IsAtPageEnd() const;

    void* LocateWritableFrame();

    void* LocateReadableFrame();

    void SkipFrame();

    void SkipWrittenFrame();

    void SkipToTime(long time);

    void FinishPage();

    static PagePtr Load(const std::string& dir, const std::string& uname, short pageNum, bool isWriting, bool quickMode);
private:
    Page(void* buffer);

    inline FH_STATUS_TYPE GetCurStatus() const
    {
        return frame.GetStatus();
    }
private:
    Frame frame;
    void* const buffer;
    int position;
    int frameNum;
    short pageNum;
};

inline bool Page::IsAtPageEnd() const
{
    return GetCurStatus() == UNIT_FRAME_STATUS_PAGE_END;
}

inline void Page::SkipFrame()
{
    position += frame.next();
    frameNum += 1;
}

inline void Page::SkipWrittenFrame()
{
    while (GetCurStatus() == UNIT_FRAME_STATUS_WRITTEN)
        SkipFrame();
}

inline void Page::SkipToTime(long time)
{
    while (GetCurStatus() == UNIT_FRAME_STATUS_WRITTEN
            && frame.GetNano() < time)
        SkipFrame();
}

inline void* Page::LocateWritableFrame()
{
    SkipWrittenFrame();
    return (GetCurStatus() == UNIT_FRAME_STATUS_RAW
    && (position + PAGE_MIN_HEADROOM < UNIT_PAGE_SIZE) ?
    frame.GetAddress() : nullptr);
}

inline void* Page::LocateReadableFrame()
{
    return (GetCurStatus() == UNIT_FRAME_STATUS_WRITTEN ? frame.GetAddress() : nullptr);
}

#endif //DIGITALCURRENCYSTRATEGYSYSTEM_PAGE_H
