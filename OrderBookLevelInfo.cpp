#include "OrderBookLevelInfo.h"

OrderBookLevelInfo::OrderBookLevelInfo(const LevelInfos& bids, const LevelInfos& asks)
    : bids_(bids), asks_(asks)
{
}

const LevelInfos& OrderBookLevelInfo::GetBids() const
{
    return bids_;
}

const LevelInfos& OrderBookLevelInfo::GetAsks() const
{
    return asks_;
}
