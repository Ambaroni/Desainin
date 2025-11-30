#pragma once
#include "order.hpp"
#include <vector>
#include <string>
using namespace std;

class OrderManager {
private:
    vector<Order> orders;

public:
    void addOrder(const Order& order);
    void deleteOrder(int orderID);
    Order* findOrder(int OrderId);
    void listOrders() const;
    void displayOrders() const;
    const std::vector<Order>& getOrders() const;

};