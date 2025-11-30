#include "editor.hpp"
#include <iostream>
using namespace std;

Editor::Editor(int id, const string& name)
    : user(id, name, user::Role::Editor), editorID(id) {}

void Editor::assignOrder(OrderManager& manager, int orderID) {
    Order* order = manager.findOrder(orderID);
    if (order) {
        order->editorAssigned = username;
        order->updateStatus(OrderStatus::InProgress);
        cout << "Editor " << username << " assigned to order " << orderID << "\n";
    } else {
        cout << "Order " << orderID << " not found.\n";
    }
}

void Editor::completeOrder(OrderManager& manager, int orderID) const {
    Order* order = manager.findOrder(orderID);
    if (order) {
        order->updateStatus(OrderStatus::Completed);
        cout << "Editor " << username << " completed order " << orderID << "\n";
    } else {
        cout << "Order " << orderID << " not found.\n";
    }
}

void Editor::attachLink(OrderManager& manager, int orderID, const string& link) {
    Order* order = manager.findOrder(orderID);
    if (order) {
        order->attachLink(link);
        cout << "Editor " << username << " attached link to order " << orderID << "\n";
    } else {
        cout << "Order " << orderID << " not found.\n";
    }
}

void Editor::displayInfo() const {
    cout << "Editor ID: " << editorID << "\n"
         << "Username: " << username << "\n"
         << "Role: " << user::roleToString(role) << "\n";
}
    