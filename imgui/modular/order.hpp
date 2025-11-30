#pragma once
#include <string>
#include <chrono>
using namespace std;

enum class OrderStatus {
    Pending,
    InProgress,
    Completed,
    Cancelled,
};

enum class OrderKind { 
    Logo,
    Status,
    Feed,
    Asset,
    Document,
    Other,
};

class Order {
public:
    int orderID;
    string orderName;
    OrderStatus status;
    OrderKind orderKind;  
    chrono::system_clock::time_point deadline;
    string reference;
    string extras;
    string editorAssigned;
    string finalLink;
    int customerID = 0;

    Order(int id, const string& name, OrderKind kind, const chrono::system_clock::time_point& deadline);

    void updateStatus(OrderStatus newStatus);
    void attachLink(const string& link);
    void assignEditor(const string& editorName);
    void unassignEditor();
    void displayOrder() const;
};
