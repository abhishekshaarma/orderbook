#include <iostream>
#include <vector>
#include <cstdint> // for int32_t and uint32_t
#include <list>
#include <algorithm>
#include <unordered_map>
#include <tuple>
#include <set>
#include <map>
#include <memory>
#include <format>    // Added for std::format
#include <stdexcept> // Added for std::logic_error

enum class OrderType
{
    GoodTillCancel,
    FillAndKill
};

enum class Side
{
    Buy,
    Sell
};

using Price = std::int32_t;
using Quantity = std::uint32_t;
using OrderId = std::uint64_t;

struct LevelInfo
{
    Price price_;
    Quantity quantity_;
};

using LevelInfos = std::vector<LevelInfo>;

class OrderBookLevelInfo
{
public:
    OrderBookLevelInfo(const LevelInfos& bids, const LevelInfos& asks)
        : bids_(bids), asks_(asks)
    {}
    
    const LevelInfos& GetBids() const
    {
        return bids_;
    }
    
    const LevelInfos& GetAsks() const
    {
        return asks_;
    }
    
private:
    LevelInfos bids_;
    LevelInfos asks_;
};

class Order
{
public:
    Order(OrderType orderType, OrderId orderId, Side side, Price price, Quantity quantity):
        orderType_(orderType), orderId_(orderId), side_(side), price_(price),
        initialQuantity_(quantity), remainingQuantity_(quantity)  // Fixed variable name
    {}
    
    OrderId GetOrderId() const
    {
        return orderId_;
    }
    
    OrderType GetOrderType() const  // Fixed return type
    {
        return orderType_;
    }
    
    Side GetSide() const
    {
        return side_;
    }
    
    Price GetPrice() const  // Fixed return type and typo
    {
        return price_;
    }
    
    Quantity GetInitialQuantity() const
    {
        return initialQuantity_;
    }
    
    Quantity GetRemainingQuantity() const
    {
        return remainingQuantity_;
    }
    
    Quantity GetFilledQuantity() const
    {
        return GetInitialQuantity() - GetRemainingQuantity();  // Fixed method name
    }
    
    void Fill(Quantity quantity)
    {
        if(quantity > GetRemainingQuantity())
        {
            throw std::logic_error(std::format("Order({}) cannot be filled for more than its remaining quantity", GetOrderId()));  
        }
        remainingQuantity_ -= quantity;
    }

    bool isFilled const()
    {
        return GetRemainingQuantity() == 0;
    }
    
private:
    OrderType orderType_;
    OrderId orderId_;
    Side side_;
    Price price_;
    Quantity initialQuantity_;
    Quantity remainingQuantity_;  // Fixed variable name consistency
};

using OrderPointer = std::shared_ptr<Order>;
using OrderPointers = std::list<OrderPointer>;

class OrderModify
{
public:
    OrderModify(OrderId  orderId, Side side, Price price, Quantity quantity)
        : orderId_(orderId), price_(price), side_(side), quantity_(quantity)
    {}

    OrderId GetOrderId() const {return orderId_;}
    Price GetPrice() const {return price_;}
    Side GetSide() const {return side_;}
    Quantity GetQuantity() const {return quantity_;}

    OrderPointer ToOrderPointer(OrderType type) const
    {
        return std::make_shared<Order>(type, GetOrderId(), GetSide(), GetPrice(), GetQuantity());
        
    }
private:
    OrderId orderId_;
    Price price_;
    Side side_;
    Quantity quantity_;
    
};

struct TradeInfo
{
    OrderId orderId_;
    Price price_;
    Quantity quantity_;
    
};

class Trade
{
public:
    Trade(const TradeInfo& bidTrade, const TradeInfo& askTrade)
        :bidTrade_(bidTrade), askTrade_(askTrade)
       {}
    const TradeInfo & GetBidTrade() const {return bidTrade_;}
    const TradeInfo & GetAskTrade() const {return askTrade_;}

private:
    TradeInfo bidTrade_;
    TradeInfo askTrade_;
    
};

using Trades = std::vector<Trade>;

class Orderbook
{

private:
    struct OrderEntry
    {
        OrderPointer order_{nullptr};
        OrderPointer::iterator location_;
        
    };
    std::map<Price, OrderPointers, std::greater<Price>> bids_;
    std::map<Price, OrderPointers, std::less<Price>> asks_;
    std::unordered_map<OrderId, OrderEntry> orders_;
    
    bool CanMatch(Side side, Price price) const
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
            {
                return false;
                
            }
            const auto& [bestBid, _] = *bids_.begin();
            return price <= BestBid;
        }
        
    }
    Trades MatchOrders()
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
            
                // Create trade record
                trades.push_back(Trade{
                        TradeInfo{bid->GetOrderId(), bid->GetPrice(), quantity},
                        TradeInfo{ask->GetOrderId(), ask->GetPrice(), quantity}
                    });
            
                // Remove filled orders
                if(bid->isFilled())
                {
                    bids.pop_front();
                    orders_.erase(bid->GetOrderId());
                }
                if(ask->isFilled())
                {
                    asks.pop_front();
                    orders_.erase(ask->GetOrderId());
                }
            }
        
            // Clean up empty price levels
            if(bids.empty())
                bids_.erase(bidPrice);
            if(asks.empty())
                asks_.erase(askPrice);
        }
    
        // Handle Fill-and-Kill orders
        if(!bids_.empty())
        {
            auto& [_, bids] = *bids_.begin();
            if(!bids.empty())
            {
                auto& order = bids.front();
                if(order->GetOrderType() == OrderType::FillAndKill)
                    CancelOrder(order->GetOrderId());
            }
        }
    
        if(!asks_.empty())
        {
            auto& [_, asks] = *asks_.begin();
            if(!asks.empty())
            {
                auto& order = asks.front();
                if(order->GetOrderType() == OrderType::FillAndKill)
                    CancelOrder(order->GetOrderId());
            }
        }
    
        return trades;

    }

public:
    Trade AddOrder(OrderPointer order)
    {
        if(!orders_.contains(order->GetOrderId()))
            return { };
        if(order->GetOrderType() == OrderType::FillAndKill && !CanMatch(order->GetSide(), order->GetPrice()))
            return {};

        OrderPointer::iterator iterator;
        if( order->GetSide() == Side::buy)
        {
            auto & orders = bids_.[order->GetPrice()];
            orders.push_back(order);
            iterator = std::next(orders.being(), orders.size() - 1);
            
        }
            orders_.insert({order->GetOrderId(), OrderEntry{ order, iterator} });
        return MatchOrders();
        
    }

    void CancelOrder(OrderId orderId)
    {
        if(!orders_.contains(orderId))
            return;

        const auto & [order, orderIterator] = orders_.at(orderId);
        orders_.erase(orderId);
        if(order->GetSide() == Side::Sell)
        {
            auto price = order->GetPrice();
            auto & orders = asks_->at.Price();
            orders.erase(iterator);
            if(orders_.empty())
            {
                asks_.erase(price);
                
            }
            else
            {
                auto price orders->bids_->GetPrice();
                auto & orders =  bids_->at.Price();
                orders.erase(iterators);
                if(orders.empty())
                    bids_->erase(price);
                
            }
            
        }
    }

    Trades MatchOrder(OrderModify order)
    {
        if(!order_.contains(order.GetOrderId()))
            return { };
        const auto & [existingOrder, _] = orders_.at(order.GetOrderId());
        CancelOrder(order.GetOrderId());

        return AddOrder(order.ToOrderPointer(existingOrder->GetOrderType()));
        
    }

    std::size_t Size() const { return orders_.size();}

    OrderbookLevelInfos GetOrderInfos() const
    {
        LevelInfos bidsInfos, askInfos;
        bidInfos.reserve(orders)
    }
    
    
};  
/*
there is a fill and kill and then there is non fil in kil so we need to add it inthe otderbook and then we need to match it that is whyu we need to make sure it is added and matched and if then we need to fill and then kill it later and if it hasnt ebed
 */
int main()
{
    // Example usage:
    auto order = std::make_shared<Order>(OrderType::GoodTillCancel, 1, Side::Buy, 100, 50);
    
    std::cout << "Order ID: " << order->GetOrderId() << std::endl;
    std::cout << "Price: " << order->GetPrice() << std::endl;
    std::cout << "Initial Quantity: " << order->GetInitialQuantity() << std::endl;
    std::cout << "Remaining Quantity: " << order->GetRemainingQuantity() << std::endl;
    
    // Fill part of the order
    order->Fill(20);
    std::cout << "After filling 20 units:" << std::endl;
    std::cout << "Remaining Quantity: " << order->GetRemainingQuantity() << std::endl;
    std::cout << "Filled Quantity: " << order->GetFilledQuantity() << std::endl;
    
    return 0;
}
