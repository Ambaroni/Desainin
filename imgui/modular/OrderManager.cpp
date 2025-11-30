#include "OrderManager.hpp"
#include <iostream>
#include <algorithm>

void OrderManager::addOrder(const Order& order) {
    orders.push_back(order);
}

Order* OrderManager::findOrder(int OrderId) {
    for (auto &o : orders) {
        if (o.orderID == OrderId)
            return &o;
    }
    return nullptr;
}

void OrderManager::listOrders() const {
    std::cout << "Orders (count=" << orders.size() << "):\n";
    for (const auto &o : orders) {
        std::cout << " - [" << o.orderID << "] " << o.orderName << "\n";
    }
}

void OrderManager::displayOrders() const {
    for (const auto &o : orders) {
        o.displayOrder();
    }
}

void OrderManager::deleteOrder(int OrderId) {
    auto oldSize = orders.size();
    orders.erase(std::remove_if(orders.begin(), orders.end(), [OrderId](const Order &o) {
        return o.orderID == OrderId;
    }), orders.end());
    if (orders.size() < oldSize) {
        std::cout << "Order " << OrderId << " removed.\n";
    } else {
        std::cout << "Order " << OrderId << " not found.\n";
    }
}

const std::vector<Order>& OrderManager::getOrders() const {
    return orders;
}
