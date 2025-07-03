#pragma once

#include "Types.h"

class OrderBookLevelInfo
{
public:
    OrderBookLevelInfo(const LevelInfos& bids, const LevelInfos& asks);
    
    const LevelInfos& GetBids() const;
    const LevelInfos& GetAsks() const;
    
private:
    LevelInfos bids_;
    LevelInfos asks_;
};
