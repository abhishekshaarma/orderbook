#include "Orderbook.h"
#include <algorithm>
#include <numeric>


Orderbook::~Orderbook
{
}

bool Orderbook::CanMatch(Side side, Price price) const
{
    if(side == Side::Buy)
    {
        if(asks_.empty())
            return false;

        const auto& [bestAsk, _] = *asks_.begin();
        return price >= bestAsk;
    }
    else
    {
        if(bids_.empty())
            return false;
            
        const auto& [bestBid, _] = *bids_.begin();
        return price <= bestBid;
    }
}

Trades Orderbook::MatchOrders()
{
    Trades trades;
    trades.reserve(orders_.size());

    while(true)
    {
        if(bids_.empty() || asks_.empty())
            break;
        
        auto& [bidPrice, bids] = *bids_.begin();
        auto& [askPrice, asks] = *asks_.begin();
    
        if(bidPrice < askPrice)
            break;
        
        while(!bids.empty() && !asks.empty())
        {
            auto& bid = bids.front();
            auto& ask = asks.front();
        
            Quantity quantity = std::min(bid->GetRemainingQuantity(), ask->GetRemainingQuantity());
            bid->Fill(quantity);
            ask->Fill(quantity);
        
            trades.push_back(Trade{
                TradeInfo{bid->GetOrderId(), bid->GetPrice(), quantity},
                TradeInfo{ask->GetOrderId(), ask->GetPrice(), quantity}
            });
        
            if(bid->IsFilled())
            {
                orders_.erase(bid->GetOrderId());
                bids.pop_front();
            }
            if(ask->IsFilled())
            {
                orders_.erase(ask->GetOrderId());
                asks.pop_front();
            }
        }
    
        if(bids.empty())
            bids_.erase(bidPrice);
        if(asks.empty())
            asks_.erase(askPrice);
    }

    // Handle Fill-and-Kill orders that couldn't be fully filled
    std::vector<OrderId> ordersToCancel;
    
    for(auto& [price, orders] : bids_)
    {
        for(auto& order : orders)
        {
            if(order->GetOrderType() == OrderType::FillAndKill)
            {
                ordersToCancel.push_back(order->GetOrderId());
            }
        }
    }
    
    for(auto& [price, orders] : asks_)
    {
        for(auto& order : orders)
        {
            if(order->GetOrderType() == OrderType::FillAndKill)
            {
                ordersToCancel.push_back(order->GetOrderId());
            }
        }
    }
    
    for(OrderId orderId : ordersToCancel)
    {
        CancelOrder(orderId);
    }

    return trades;
}

Trades Orderbook::AddOrder(OrderPointer order)
{
    if(orders_.contains(order->GetOrderId()))
        return {};
        
    if(order->GetOrderType() == OrderType::FillAndKill && !CanMatch(order->GetSide(), order->GetPrice()))
        return {};

    OrderPointers::iterator iterator;
    
    if(order->GetSide() == Side::Buy)
    {
        auto& orders = bids_[order->GetPrice()];
        orders.push_back(order);
        iterator = std::prev(orders.end());
    }
    else
    {
        auto& orders = asks_[order->GetPrice()];
        orders.push_back(order);
        iterator = std::prev(orders.end());
    }
    
    orders_.insert({order->GetOrderId(), OrderEntry{order, iterator}});
    return MatchOrders();
}

void Orderbook::CancelOrder(OrderId orderId)
{
    if(!orders_.contains(orderId))
        return;

    const auto& [order, orderIterator] = orders_.at(orderId);
    orders_.erase(orderId);
    
    if(order->GetSide() == Side::Sell)
    {
        auto price = order->GetPrice();
        auto& orders = asks_.at(price);
        orders.erase(orderIterator);
        if(orders.empty())
        {
            asks_.erase(price);
        }
    }
    else
    {
        auto price = order->GetPrice();
        auto& orders = bids_.at(price);
        orders.erase(orderIterator);
        if(orders.empty())
        {
            bids_.erase(price);
        }
    }
}

Trades Orderbook::ModifyOrder(OrderModify order)
{
    if(!orders_.contains(order.GetOrderId()))
        return {};
        
    const auto& [existingOrder, _] = orders_.at(order.GetOrderId());
    CancelOrder(order.GetOrderId());
    return AddOrder(order.ToOrderPointer(existingOrder->GetOrderType()));
}

std::size_t Orderbook::Size() const 
{ 
    return orders_.size();
}

OrderBookLevelInfo Orderbook::GetOrderInfos() const
{
    LevelInfos bidsInfos, asksInfos;
    bidsInfos.reserve(bids_.size());
    asksInfos.reserve(asks_.size());

    auto CreateLevelInfo = [](Price price, const OrderPointers& orders)
    {
        return LevelInfo{price, std::accumulate(orders.begin(), orders.end(),
                                               (Quantity)0,
            [](Quantity runningSum, const OrderPointer& order)
            {
                return runningSum + order->GetRemainingQuantity();
            })};
    };
    
    for(const auto& [price, orders] : bids_)
        bidsInfos.push_back(CreateLevelInfo(price, orders));

    for(const auto& [price, orders] : asks_)
        asksInfos.push_back(CreateLevelInfo(price, orders));
        
    return OrderBookLevelInfo{bidsInfos, asksInfos};
}
