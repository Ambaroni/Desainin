#pragma once
#include "user.hpp"
#include <vector>

class UserManager {
private:
    std::vector<user> users;
    int nextUserID = 1;

public:
    UserManager();
    bool registerUser(const std::string& username, const std::string& password, user::Role role);
    user* loginUser(const std::string& username, const std::string& password);
    bool usernameExists(const std::string& username) const;
    user* getUserByID(int userID);
    const std::vector<user>& getAllUsers() const;
};
