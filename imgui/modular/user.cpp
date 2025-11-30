#include "user.hpp"
#include <iostream>
using namespace std;

user::user(int id, const string& name, user::Role role)
    : userID(id), username(name), role(role), password("") {}

user::user(int id, const string& name, const string& pwd, user::Role role)
    : userID(id), username(name), password(pwd), role(role) {}

int user::getUserID() const {
    return userID;
}
string user::getUsername() const {
    return username;
}
string user::getPassword() const {
    return password;
}
user::Role user::getRole() const {
    return role;
}

bool user::verifyPassword(const string& pwd) const {
    return password == pwd;
}

string user::roleToString(user::Role r) {
    switch (r) {
        case Role::Customer: return string("Customer");
        case Role::Editor: return string("Editor");
    }
    return string("Unknown");
}

void user::displayInfo() const {
    cout << "User ID: " << userID << "\n"
         << "Username: " << username << "\n"
         << "Role: " << roleToString(role) << "\n";
}