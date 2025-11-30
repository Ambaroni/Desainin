#pragma once
#include <string>
using namespace std;

class user {
    public:
    enum class Role { Customer, Editor };

    protected:
    int userID;
    string username;
    string password;
    Role role;

    public:
    user(int id, const string& name, Role role);
    user(int id, const string& name, const string& pwd, Role role);

    int getUserID() const;
    string getUsername() const;
    string getPassword() const;
    Role getRole() const;
    static string roleToString(Role r);
    bool verifyPassword(const string& pwd) const;

    virtual void displayInfo() const;
};