#pragma once
#include <string>
#include <vector>
#include "OrderManager.hpp"
#include "UserManager.hpp"

class SaveManager {
public:
    // Save all data to CSV file
    static bool saveToFile(const std::string& filename, const OrderManager& manager, const UserManager& userManager);
    
    // Load all data from CSV file
    static bool loadFromFile(const std::string& filename, OrderManager& manager, UserManager& userManager);
    
private:
    // Helper functions for CSV operations
    static std::string escapeCSV(const std::string& field);
    static std::string unescapeCSV(const std::string& field);
    static std::vector<std::string> parseCSVLine(const std::string& line);
};
