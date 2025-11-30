#pragma once
#include "order.hpp"
#include "user.hpp"
#include "OrderManager.hpp"
#include <string>
#include <iostream>
#include <chrono>
using namespace std;

class Customer : public user {
    private:
    int CustomerID;

    public:
    Customer(int id, const string& name);

    void createOrder(OrderManager& manager,
        int orderID,
        const string& orderName,
        OrderKind kind,
        chrono::system_clock::time_point deadline,
        const string& reference,
        const string& extras); 
    
    void modifyOrder(OrderManager& manager,
        int orderID,
        const string& newName,
        OrderKind newKind,
        chrono::system_clock::time_point newDeadline,
        const string& reference,
        const string& extras);
    
    void displayInfo() const override;
};
