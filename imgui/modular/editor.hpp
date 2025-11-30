#pragma once
#include "user.hpp"
#include "order.hpp"
#include "OrderManager.hpp"
#include <string>
using namespace std;

class Editor : public user {
    private:
    int editorID;

    public:
    Editor(int id, const string& name);

    void assignOrder(OrderManager& manager, int orderID);
    void completeOrder(OrderManager& manager, int orderID) const;
    void attachLink(OrderManager& manager, int orderID, const string& link);

    void displayInfo() const override;
};