#include "UserManager.hpp"

UserManager::UserManager() : nextUserID(1001) {}

bool UserManager::registerUser(const std::string& username, const std::string& password, user::Role role) {
    if (usernameExists(username)) {
        return false;
    }
    
    users.emplace_back(nextUserID, username, password, role);
    nextUserID++;
    return true;
}

user* UserManager::loginUser(const std::string& username, const std::string& password) {
    for (auto& u : users) {
        if (u.getUsername() == username && u.verifyPassword(password)) {
            return &u;
        }
    }
    return nullptr;
}

bool UserManager::usernameExists(const std::string& username) const {
    for (const auto& u : users) {
        if (u.getUsername() == username) {
            return true;
        }
    }
    return false;
}

user* UserManager::getUserByID(int userID) {
    for (auto& u : users) {
        if (u.getUserID() == userID) {
            return &u;
        }
    }
    return nullptr;
}

const std::vector<user>& UserManager::getAllUsers() const {
    return users;
}
