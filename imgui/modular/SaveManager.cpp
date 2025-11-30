#include "SaveManager.hpp"
#include <fstream>
#include <sstream>
#include <chrono>
#include <iostream>
#include <ctime>

using namespace std;


string SaveManager::escapeCSV(const string& field) {
    string result = field;
  
    bool needsQuotes = result.find(',') != string::npos || 
                       result.find('\n') != string::npos || 
                       result.find('"') != string::npos;
    
    if (needsQuotes || result.empty()) {
  
        size_t pos = 0;
        while ((pos = result.find('"', pos)) != string::npos) {
            result.replace(pos, 1, "\"\"");
            pos += 2;
        }
        return "\"" + result + "\"";
    }
    return result;
}


string SaveManager::unescapeCSV(const string& field) {
    string result = field;
    
 
    if (result.length() >= 2 && result.front() == '"' && result.back() == '"') {
        result = result.substr(1, result.length() - 2);
        

        size_t pos = 0;
        while ((pos = result.find("\"\"", pos)) != string::npos) {
            result.replace(pos, 2, "\"");
            pos += 1;
        }
    }
    
    return result;
}

vector<string> SaveManager::parseCSVLine(const string& line) {
    vector<string> fields;
    string field;
    bool inQuotes = false;
    
    for (size_t i = 0; i < line.length(); ++i) {
        char c = line[i];
        
        if (c == '"') {
            if (inQuotes && i + 1 < line.length() && line[i + 1] == '"') {
             
                field += '"';
                ++i;
            } else {
          
                inQuotes = !inQuotes;
            }
        } else if (c == ',' && !inQuotes) {
         
            fields.push_back(field);
            field.clear();
        } else {
            field += c;
        }
    }
    
    fields.push_back(field);
    return fields;
}


bool SaveManager::saveToFile(const string& filename, const OrderManager& manager, const UserManager& userManager) {
    try {
        ofstream file(filename);
        if (!file.is_open()) {
            cerr << "Error: Could not open file for writing: " << filename << endl;
            return false;
        }
        
      
        file << "# Users\n";
        file << "TYPE,ID,USERNAME,PASSWORD,ROLE\n";
        
       
        const auto& users = userManager.getAllUsers();
        for (const auto& user : users) {
            string roleStr = (user.getRole() == user::Role::Customer) ? "Customer" : "Editor";
            file << "USER," << user.getUserID() << "," 
                 << escapeCSV(user.getUsername()) << "," 
                 << escapeCSV(user.getPassword()) << "," 
                 << roleStr << "\n";
        }
        
    
        file << "\n# Orders\n";
        file << "TYPE,ORDERID,ORDERNAME,STATUS,ORDERTYPE,DEADLINE,REFERENCE,EXTRAS,EDITOR_ASSIGNED,FINALLINK,CUSTOMERID\n";
        
     
        const auto& orders = manager.getOrders();
        for (const auto& order : orders) {
            string statusStr;
            switch (order.status) {
                case OrderStatus::Pending: statusStr = "Pending"; break;
                case OrderStatus::InProgress: statusStr = "InProgress"; break;
                case OrderStatus::Completed: statusStr = "Completed"; break;
                case OrderStatus::Cancelled: statusStr = "Cancelled"; break;
                default: statusStr = "Pending";
            }
            
            string kindStr;
            switch (order.orderKind) {
                case OrderKind::Logo: kindStr = "Logo"; break;
                case OrderKind::Status: kindStr = "Status"; break;
                case OrderKind::Feed: kindStr = "Feed"; break;
                case OrderKind::Asset: kindStr = "Asset"; break;
                case OrderKind::Document: kindStr = "Document"; break;
                case OrderKind::Other: kindStr = "Other"; break;
                default: kindStr = "Other";
            }
            
         
            auto deadline_time = chrono::system_clock::to_time_t(order.deadline);
            char deadline_str[20];
            strftime(deadline_str, sizeof(deadline_str), "%Y-%m-%d", localtime(&deadline_time));
            
            file << "ORDER," << order.orderID << "," 
                 << escapeCSV(order.orderName) << "," 
                 << statusStr << "," 
                 << kindStr << "," 
                 << deadline_str << "," 
                 << escapeCSV(order.reference) << "," 
                 << escapeCSV(order.extras) << "," 
                 << escapeCSV(order.editorAssigned) << "," 
                 << escapeCSV(order.finalLink) << "," 
                 << order.customerID << "\n";
        }
        
        file.close();
        cout << "Data saved successfully to " << filename << endl;
        return true;
        
    } catch (const exception& e) {
        cerr << "Error while saving: " << e.what() << endl;
        return false;
    }
}


bool SaveManager::loadFromFile(const string& filename, OrderManager& manager, UserManager& userManager) {
    try {
        ifstream file(filename);
        if (!file.is_open()) {
            cerr << "Info: Save file not found, starting fresh: " << filename << endl;
            return false;
        }
        
        string line;
        bool readingUsers = false;
        bool readingOrders = false;
        
        while (getline(file, line)) {
           
            if (line.empty() || line[0] == '#') continue;
            
      
            if (line.find("Users") != string::npos) {
                readingUsers = true;
                readingOrders = false;
                continue;
            }
            if (line.find("Orders") != string::npos) {
                readingOrders = true;
                readingUsers = false;
                continue;
            }
            
     
            if (line.find("TYPE,") == 0) continue;
            
            vector<string> fields = parseCSVLine(line);
            
            if (fields.empty()) continue;
            
            if (fields[0] == "USER" && fields.size() >= 5) {
                int userID = stoi(fields[1]);
                string username = unescapeCSV(fields[2]);
                string password = unescapeCSV(fields[3]);
                string roleStr = fields[4];
                user::Role role = (roleStr == "Editor") ? user::Role::Editor : user::Role::Customer;
                
        
                userManager.registerUser(username, password, role);
            } 
            else if (fields[0] == "ORDER" && fields.size() >= 11) {
                int orderID = stoi(fields[1]);
                string orderName = unescapeCSV(fields[2]);
                string statusStr = fields[3];
                string kindStr = fields[4];
                string deadlineStr = fields[5];
                string reference = unescapeCSV(fields[6]);
                string extras = unescapeCSV(fields[7]);
                string editorAssigned = unescapeCSV(fields[8]);
                string finalLink = unescapeCSV(fields[9]);
                int customerID = stoi(fields[10]);
                
        
                OrderStatus status = OrderStatus::Pending;
                if (statusStr == "InProgress") status = OrderStatus::InProgress;
                else if (statusStr == "Completed") status = OrderStatus::Completed;
                else if (statusStr == "Cancelled") status = OrderStatus::Cancelled;
                
       
                OrderKind kind = OrderKind::Other;
                if (kindStr == "Logo") kind = OrderKind::Logo;
                else if (kindStr == "Status") kind = OrderKind::Status;
                else if (kindStr == "Feed") kind = OrderKind::Feed;
                else if (kindStr == "Asset") kind = OrderKind::Asset;
                else if (kindStr == "Document") kind = OrderKind::Document;
                
           
                int year, month, day;
                if (sscanf(deadlineStr.c_str(), "%d-%d-%d", &year, &month, &day) == 3) {
                    struct tm deadline_tm = {};
                    deadline_tm.tm_year = year - 1900;
                    deadline_tm.tm_mon = month - 1;
                    deadline_tm.tm_mday = day;
                    deadline_tm.tm_hour = 0;
                    deadline_tm.tm_min = 0;
                    deadline_tm.tm_sec = 0;
                    time_t deadline_time = mktime(&deadline_tm);
                    auto deadline = chrono::system_clock::from_time_t(deadline_time);
                    
               
                    Order newOrder(orderID, orderName, kind, deadline);
                    newOrder.status = status;
                    newOrder.reference = reference;
                    newOrder.extras = extras;
                    newOrder.editorAssigned = editorAssigned;
                    newOrder.finalLink = finalLink;
                    newOrder.customerID = customerID;
                    
                    manager.addOrder(newOrder);
                }
            }
        }
        
        file.close();
        cout << "Data loaded successfully from " << filename << endl;
        return true;
        
    } catch (const exception& e) {
        cerr << "Error while loading: " << e.what() << endl;
        return false;
    }
}
