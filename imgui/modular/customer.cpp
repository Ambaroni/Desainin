#include "Customer.hpp"
#include <iostream>
using namespace std;

Customer::Customer(int id, const string& name)
    : user(id, name, user::Role::Customer), CustomerID(id) {}

void Customer::createOrder(OrderManager& manager,
                           int orderID,
                           const string& orderName,
                           OrderKind kind,
                           chrono::system_clock::time_point deadline,
                           const string& reference,
                           const string& extras) {
    Order newOrder(orderID, orderName, kind, deadline);
    newOrder.reference = reference;
    newOrder.extras = extras;
    newOrder.customerID = CustomerID;
    manager.addOrder(newOrder);

    cout << "Customer " << username << " created Order " << orderID
         << " (" << orderName << ") with reference: " << reference
         << " and extras: " << extras << endl;
}

void Customer::modifyOrder(OrderManager& manager,
                           int orderID,
                           const string& newName,
                           OrderKind newKind,
                           chrono::system_clock::time_point newDeadline,
                           const string& reference,
                           const string& extras) {
    Order* order = manager.findOrder(orderID);
    if (order) {
        order->orderName = newName;
        order->orderKind = newKind;
        order->deadline = newDeadline;
        order->reference = reference;
        order->extras = extras;

        cout << "Customer " << username << " modified Order " << orderID
             << " → Name: " << newName
             << ", Kind updated, Deadline adjusted, Reference: " << reference
             << ", Extras: " << extras << endl;
    } else {
        cout << "Order " << orderID << " not found.\n";
    }
}

void Customer::displayInfo() const {
    cout << "Customer ID: " << CustomerID
         << " | Name: " << username
         << " | Role: " << user::roleToString(role) << endl;
}
