#pragma once

#include "Types.h"
#include "Constants.h"
#include <memory>
#include <list>

class Order
{
public:
    Order(OrderType orderType, OrderId orderId, Side side, Price price, Quantity quantity);

    Order(OrderId orderId, Side side, Quantity quantity)
        : Order(OrderType::Market, orderId, side, Constants::InvalidPrice, quantity)
    {}
    
    OrderId GetOrderId() const;
    OrderType GetOrderType() const;
    Side GetSide() const;
    Price GetPrice() const;
    Quantity GetInitialQuantity() const;
    Quantity GetRemainingQuantity() const;
    Quantity GetFilledQuantity() const;
    
    void Fill(Quantity quantity);
    bool IsFilled() const;
    
private:
    OrderType orderType_;
    OrderId orderId_;
    Side side_;
    Price price_;
    Quantity initialQuantity_;
    Quantity remainingQuantity_;
};

using OrderPointer = std::shared_ptr<Order>;
using OrderPointers = std::list<OrderPointer>;
