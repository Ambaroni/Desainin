#include "order.hpp"
#include <iostream>
#include <ctime>
using namespace std;

Order::Order(int id, const string& name, OrderKind kind, const chrono::system_clock::time_point& deadline)
    : orderID(id), orderName(name), orderKind(kind), deadline(deadline), status(OrderStatus::Pending), editorAssigned("") {};

void Order::updateStatus(OrderStatus newStatus) {
    status = newStatus;
}

void Order::attachLink(const string& link) {
    reference = link;
}

void Order::assignEditor(const string& editorName) {
    editorAssigned = editorName;
}

void Order::unassignEditor() {
    editorAssigned = "";
}

void Order::displayOrder() const {
    time_t deadline_time = chrono::system_clock::to_time_t(deadline);
    cout << "Order ID: " << orderID << "\n"
         << "Customer ID: " << customerID << "\n"
         << "Order Name: " << orderName << "\n"
         << "Order Kind: " << static_cast<int>(orderKind) << "\n"
         << "Status: " << static_cast<int>(status) << "\n"
         << "Deadline: " << ctime(&deadline_time)
         << "Reference: " << reference << "\n"
         << "Extras: " << extras << "\n"
         << "Editor Assigned: " << editorAssigned << "\n";
}