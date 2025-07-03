#pragma once

#include "Types.h"
#include "Order.h"

class OrderModify
{
public:
    OrderModify(OrderId orderId, Side side, Price price, Quantity quantity);

    OrderId GetOrderId() const;
    Price GetPrice() const;
    Side GetSide() const;
    Quantity GetQuantity() const;

    OrderPointer ToOrderPointer(OrderType type) const;
    
private:
    OrderId orderId_;
    Price price_;
    Side side_;
    Quantity quantity_;
};
