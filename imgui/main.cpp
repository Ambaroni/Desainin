#include <d3d9.h>
#include <tchar.h>
#include <string>
#include <chrono>

#include "imgui.h"
#include "imgui_impl_dx9.h"
#include "imgui_impl_win32.h"

#include "modular/OrderManager.hpp"
#include "modular/customer.hpp"
#include "modular/editor.hpp"
#include "modular/UserManager.hpp"
#include "modular/SaveManager.hpp"

// DirectX9 globals
static LPDIRECT3D9              g_pD3D = nullptr;
static LPDIRECT3DDEVICE9        g_pd3dDevice = nullptr;
static D3DPRESENT_PARAMETERS    g_d3dpp = {};

// Forward declarations
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void ResetDevice();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// App state
struct AppState {
    UserManager userManager;
    OrderManager manager;
    
    // UI state
    enum Screen { 
        LoginChoice, Register, Login, 
        CustomerMenu, NewOrder, OrderDetails,
        EditorMenu, EditorOrderDetails
    };
    Screen currentScreen = LoginChoice;
    
    // Login form buffers
    char bufUsername[64] = "";
    char bufPassword[64] = "";
    int regRoleIndex = 0; // 0=Customer, 1=Editor
    char regErrorMsg[128] = "";
    char loginErrorMsg[128] = "";
    
    // Logged-in state
    int loggedUserID = 0;
    std::string loggedUsername = "";
    user::Role loggedRole = user::Role::Customer;
    int selectedOrderID = 0;
    
    // Order form buffers
    char bufOrderName[128] = "My Order";
    char bufReference[128] = "";
    char bufExtras[128] = "";
    char bufFinalLink[256] = "";
    char bufEditorAssign[64] = "";
    int nextOrderID = 1001;
    int kindIndex = 0;
    int deadlineDays = 7;
    int statusIndex = 0; // For editor status changes
    
    // Detail prefill flag
    bool detailsPrefilled = false;
};

// Helper function to calculate days until deadline
int calculateDaysUntilDeadline(const std::chrono::system_clock::time_point& deadline) {
    using namespace std::chrono;
    auto now = system_clock::now();
    auto diff = duration_cast<hours>(deadline - now);
    return static_cast<int>(diff.count() / 24);
}

int main(int, char**)
{
    // Create window
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"ImGui Example", nullptr };
    ::RegisterClassExW(&wc);
    HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"Desainin Order Manager", WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, nullptr, nullptr, wc.hInstance, nullptr);

    if (!CreateDeviceD3D(hwnd)) {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // Setup ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui::StyleColorsDark();
    
    // Custom Professional Dark Theme
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 5.0f;
    style.FrameRounding = 5.0f;
    style.PopupRounding = 5.0f;
    style.ScrollbarRounding = 5.0f;
    style.GrabRounding = 5.0f;
    style.TabRounding = 5.0f;
    
    // Color palette
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.15f, 0.15f, 0.17f, 1.00f);  // Darker bg
    colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.20f, 0.22f, 1.00f);    // Frame bg
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.25f, 0.25f, 0.27f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.30f, 0.30f, 0.32f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.20f, 0.20f, 0.22f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.25f, 0.35f, 0.50f, 1.00f);     // Blue buttons
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.30f, 0.40f, 0.55f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.35f, 0.45f, 0.60f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.25f, 0.35f, 0.50f, 0.50f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.30f, 0.40f, 0.55f, 0.75f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.35f, 0.45f, 0.60f, 1.00f);
    
    // Try to load Arial font
    if (io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\arial.ttf", 16.0f)) {
        // Font loaded successfully
    }

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX9_Init(g_pd3dDevice);

    AppState app;
    
    // Load saved data at startup
    SaveManager::loadFromFile("savedata.txt", app.manager, app.userManager);

    // Main loop
    bool done = false;
    while (!done) {
        MSG msg;
        while (::PeekMessageW(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done) break;

        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // Get main viewport for centering windows
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImVec2 center = viewport->GetCenter();

        // ========== LOGIN CHOICE WINDOW ==========
        if (app.currentScreen == AppState::LoginChoice) {
            ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
            ImGui::SetNextWindowSize(ImVec2(400, 200), ImGuiCond_FirstUseEver);
            ImGui::Begin("Desainin - Welcome", nullptr, ImGuiWindowFlags_NoCollapse);
            
            ImGui::Text("Welcome to Desainin!");
            ImGui::Text("What would you like to do?");
            ImGui::Separator();
            
            ImGui::SetCursorPosX((ImGui::GetWindowWidth() - 310) * 0.5f);
            if (ImGui::Button("Register##choice", ImVec2(150, 0))) {
                strcpy(app.bufUsername, "");
                strcpy(app.bufPassword, "");
                strcpy(app.regErrorMsg, "");
                app.regRoleIndex = 0;
                app.currentScreen = AppState::Register;
            }
            
            ImGui::SameLine();
            
            if (ImGui::Button("Login##choice", ImVec2(150, 0))) {
                strcpy(app.bufUsername, "");
                strcpy(app.bufPassword, "");
                strcpy(app.loginErrorMsg, "");
                app.currentScreen = AppState::Login;
            }
            
            ImGui::End();
        }

        // ========== REGISTER WINDOW ==========
        if (app.currentScreen == AppState::Register) {
            ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
            ImGui::SetNextWindowSize(ImVec2(450, 300), ImGuiCond_FirstUseEver);
            ImGui::Begin("Create New Account", nullptr, ImGuiWindowFlags_NoCollapse);
            
            ImGui::Text("Register a new account");
            ImGui::Separator();
            
            ImGui::InputText("Username##reg", app.bufUsername, IM_ARRAYSIZE(app.bufUsername));
            ImGui::InputText("Password##reg", app.bufPassword, IM_ARRAYSIZE(app.bufPassword), ImGuiInputTextFlags_Password);
            
            const char* roles[] = { "Customer", "Editor" };
            ImGui::Combo("Role##reg", &app.regRoleIndex, roles, IM_ARRAYSIZE(roles));
            
            if (strlen(app.regErrorMsg) > 0) {
                ImGui::TextColored(ImVec4(1, 0.3f, 0.3f, 1), "%s", app.regErrorMsg);
            }
            
            ImGui::Separator();
            
            if (ImGui::Button("Create Account##btn", ImVec2(150, 0))) {
                if (strlen(app.bufUsername) == 0 || strlen(app.bufPassword) == 0) {
                    strcpy(app.regErrorMsg, "Username and password cannot be empty!");
                } else if (strlen(app.bufUsername) < 3) {
                    strcpy(app.regErrorMsg, "Username must be at least 3 characters!");
                } else if (strlen(app.bufPassword) < 4) {
                    strcpy(app.regErrorMsg, "Password must be at least 4 characters!");
                } else if (app.userManager.usernameExists(app.bufUsername)) {
                    strcpy(app.regErrorMsg, "Username already exists!");
                } else {
                    user::Role role = (app.regRoleIndex == 0) ? user::Role::Customer : user::Role::Editor;
                    bool success = app.userManager.registerUser(app.bufUsername, app.bufPassword, role);
                    if (success) {
                        strcpy(app.regErrorMsg, "");
                        strcpy(app.bufUsername, "");
                        strcpy(app.bufPassword, "");
                        app.currentScreen = AppState::LoginChoice;
                    } else {
                        strcpy(app.regErrorMsg, "Registration failed!");
                    }
                }
            }
            
            ImGui::SameLine();
            
            if (ImGui::Button("Back##reg", ImVec2(150, 0))) {
                app.currentScreen = AppState::LoginChoice;
                strcpy(app.regErrorMsg, "");
            }
            
            ImGui::End();
        }

        // ========== LOGIN WINDOW ==========
        if (app.currentScreen == AppState::Login) {
            ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
            ImGui::SetNextWindowSize(ImVec2(450, 280), ImGuiCond_FirstUseEver);
            ImGui::Begin("Login", nullptr, ImGuiWindowFlags_NoCollapse);
            
            ImGui::Text("Login to your account");
            ImGui::Separator();
            
            ImGui::InputText("Username##login", app.bufUsername, IM_ARRAYSIZE(app.bufUsername));
            ImGui::InputText("Password##login", app.bufPassword, IM_ARRAYSIZE(app.bufPassword), ImGuiInputTextFlags_Password);
            
            if (strlen(app.loginErrorMsg) > 0) {
                ImGui::TextColored(ImVec4(1, 0.3f, 0.3f, 1), "%s", app.loginErrorMsg);
            }
            
            ImGui::Separator();
            
            if (ImGui::Button("Login##btn", ImVec2(150, 0))) {
                user* loggedInUser = app.userManager.loginUser(app.bufUsername, app.bufPassword);
                if (loggedInUser) {
                    app.loggedUserID = loggedInUser->getUserID();
                    app.loggedUsername = loggedInUser->getUsername();
                    app.loggedRole = loggedInUser->getRole();
                    strcpy(app.loginErrorMsg, "");
                    
                    if (app.loggedRole == user::Role::Customer) {
                        app.currentScreen = AppState::CustomerMenu;
                    } else {
                        // Editor login successful
                        app.currentScreen = AppState::EditorMenu;
                    }
                } else {
                    strcpy(app.loginErrorMsg, "Invalid username or password!");
                }
            }
            
            ImGui::SameLine();
            
            if (ImGui::Button("Back##login", ImVec2(150, 0))) {
                app.currentScreen = AppState::LoginChoice;
                strcpy(app.loginErrorMsg, "");
            }
            
            ImGui::End();
        }

        // ========== CUSTOMER MENU WINDOW ==========
        if (app.currentScreen == AppState::CustomerMenu) {
            ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
            ImGui::SetNextWindowSize(ImVec2(700, 500), ImGuiCond_FirstUseEver);
            ImGui::Begin("Customer Menu", nullptr, ImGuiWindowFlags_NoCollapse);
            
            ImGui::Text("Logged in as: %s (ID: %d)", app.loggedUsername.c_str(), app.loggedUserID);
            ImGui::SameLine(ImGui::GetWindowWidth() - 100);
            if (ImGui::Button("Logout##custmenu")) {
                app.currentScreen = AppState::LoginChoice;
                app.loggedUserID = 0;
                app.loggedUsername = "";
                app.selectedOrderID = 0;
            }
            
            ImGui::Separator();
            ImGui::Text("Your Orders:");
            
            ImGui::BeginChild("orders_list", ImVec2(0, 300), true);
            int orderCount = 0;
            for (const auto& order : app.manager.getOrders()) {
                if (order.customerID != app.loggedUserID) continue;
                
                orderCount++;
                const char* statusStr = "Pending";
                switch(order.status) {
                    case OrderStatus::Pending: statusStr = "Pending"; break;
                    case OrderStatus::InProgress: statusStr = "In Progress"; break;
                    case OrderStatus::Completed: statusStr = "Completed"; break;
                    case OrderStatus::Cancelled: statusStr = "Cancelled"; break;
                }
                
                char label[256];
                snprintf(label, sizeof(label), "[ID:%d] %s - %s", 
                         order.orderID, order.orderName.c_str(), statusStr);
                
                if (ImGui::Selectable(label, app.selectedOrderID == order.orderID)) {
                    app.selectedOrderID = order.orderID;
                    app.detailsPrefilled = false;
                    app.currentScreen = AppState::OrderDetails;
                }
            }
            if (orderCount == 0) {
                ImGui::TextDisabled("No orders yet. Create your first order!");
            }
            ImGui::EndChild();
            
            ImGui::Separator();
            
            if (ImGui::Button("New Order##btn", ImVec2(150, 0))) {
                app.nextOrderID = 1001 + (int)app.manager.getOrders().size();
                strcpy(app.bufOrderName, "");
                strcpy(app.bufReference, "");
                strcpy(app.bufExtras, "");
                app.kindIndex = 0;
                app.deadlineDays = 7;
                app.currentScreen = AppState::NewOrder;
            }
            
            ImGui::End();
        }

        // ========== NEW ORDER WINDOW ==========
        if (app.currentScreen == AppState::NewOrder) {
            ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
            ImGui::SetNextWindowSize(ImVec2(550, 450), ImGuiCond_FirstUseEver);
            ImGui::Begin("New Order", nullptr, ImGuiWindowFlags_NoCollapse);
            
            ImGui::Text("Create a new order (Customer ID: %d)", app.loggedUserID);
            ImGui::Separator();
            ImGui::Text("Order ID will be auto-assigned: %d", app.nextOrderID);
            ImGui::InputText("Order Name##neworder", app.bufOrderName, IM_ARRAYSIZE(app.bufOrderName));
            
            const char* kinds[] = { "Logo", "Status", "Feed", "Asset", "Document", "Other" };
            ImGui::Combo("Order Kind##neworder", &app.kindIndex, kinds, IM_ARRAYSIZE(kinds));
            
            ImGui::InputInt("Deadline (days)##neworder", &app.deadlineDays);
            if (app.deadlineDays < 1) app.deadlineDays = 1;
            if (app.deadlineDays > 365) app.deadlineDays = 365;
            
            ImGui::InputText("Reference##neworder", app.bufReference, IM_ARRAYSIZE(app.bufReference));
            ImGui::InputTextMultiline("Extras##neworder", app.bufExtras, IM_ARRAYSIZE(app.bufExtras), ImVec2(0, 60));
            
            ImGui::Separator();
            
            if (ImGui::Button("Create Order##btn", ImVec2(150, 0))) {
                if (strlen(app.bufOrderName) == 0) {
                    ImGui::OpenPopup("validation_error");
                } else {
                    using namespace std::chrono;
                    auto deadline = system_clock::now() + hours(24 * app.deadlineDays);
                    Customer cust(app.loggedUserID, "User");
                    cust.createOrder(app.manager, app.nextOrderID, 
                                    std::string(app.bufOrderName), 
                                    static_cast<OrderKind>(app.kindIndex),
                                    deadline, 
                                    std::string(app.bufReference), 
                                    std::string(app.bufExtras));
                    app.currentScreen = AppState::CustomerMenu;
                }
            }
            
            ImGui::SameLine();
            
            if (ImGui::Button("Cancel##neworder", ImVec2(150, 0))) {
                app.currentScreen = AppState::CustomerMenu;
            }
            
            // Validation error popup
            if (ImGui::BeginPopupModal("validation_error", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::Text("Order name cannot be empty!");
                if (ImGui::Button("OK", ImVec2(120, 0))) {
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }
            
            ImGui::End();
        }

        // ========== ORDER DETAILS WINDOW ==========
        if (app.currentScreen == AppState::OrderDetails) {
            ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
            ImGui::SetNextWindowSize(ImVec2(600, 550), ImGuiCond_FirstUseEver);
            ImGui::Begin("Order Details", nullptr, ImGuiWindowFlags_NoCollapse);
            
            if (app.selectedOrderID == 0) {
                ImGui::Text("No order selected.");
                if (ImGui::Button("Back##noselect")) {
                    app.currentScreen = AppState::CustomerMenu;
                }
                ImGui::End();
            } else {
                Order* order = app.manager.findOrder(app.selectedOrderID);
                
                if (!order) {
                    ImGui::Text("Order not found (may have been deleted).");
                    if (ImGui::Button("Back##notfound")) {
                        app.currentScreen = AppState::CustomerMenu;
                        app.selectedOrderID = 0;
                    }
                    ImGui::End();
                } else {
                    // Prefill form once
                    if (!app.detailsPrefilled) {
                        strncpy(app.bufOrderName, order->orderName.c_str(), sizeof(app.bufOrderName) - 1);
                        app.bufOrderName[sizeof(app.bufOrderName) - 1] = '\0';
                        app.kindIndex = static_cast<int>(order->orderKind);
                        app.deadlineDays = calculateDaysUntilDeadline(order->deadline);
                        if (app.deadlineDays < 1) app.deadlineDays = 1;
                        strncpy(app.bufReference, order->reference.c_str(), sizeof(app.bufReference) - 1);
                        app.bufReference[sizeof(app.bufReference) - 1] = '\0';
                        strncpy(app.bufExtras, order->extras.c_str(), sizeof(app.bufExtras) - 1);
                        app.bufExtras[sizeof(app.bufExtras) - 1] = '\0';
                        app.detailsPrefilled = true;
                    }
                    
                    ImGui::Text("Order ID: %d (auto-assigned)", order->orderID);
                    ImGui::Text("Customer ID: %d", order->customerID);
                    
                    const char* statusStr = "Pending";
                    switch(order->status) {
                        case OrderStatus::Pending: statusStr = "Pending"; break;
                        case OrderStatus::InProgress: statusStr = "In Progress"; break;
                        case OrderStatus::Completed: statusStr = "Completed"; break;
                        case OrderStatus::Cancelled: statusStr = "Cancelled"; break;
                    }
                    ImGui::Text("Status: %s", statusStr);
                    
                    if (!order->finalLink.empty()) {
                        ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.8f, 1.0f), "Final Link: %s", order->finalLink.c_str());
                    }
                    
                    if (!order->editorAssigned.empty()) {
                        ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.2f, 1.0f), "Assigned Editor: %s", order->editorAssigned.c_str());
                    } else {
                        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Assigned Editor: (None)");
                    }
                    
                    ImGui::Separator();
                    
                    // Check if order is completed
                    bool isCompletedCustomer = (order->status == OrderStatus::Completed);
                    
                    if (isCompletedCustomer) {
                        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "ORDER COMPLETED - LOCKED");
                        ImGui::Text("This order cannot be modified.");
                    } else {
                        ImGui::InputText("Order Name##details", app.bufOrderName, IM_ARRAYSIZE(app.bufOrderName));
                        
                        const char* kinds[] = { "Logo", "Status", "Feed", "Asset", "Document", "Other" };
                        ImGui::Combo("Order Kind##details", &app.kindIndex, kinds, IM_ARRAYSIZE(kinds));
                        
                        ImGui::InputInt("Deadline (days)##details", &app.deadlineDays);
                        if (app.deadlineDays < 1) app.deadlineDays = 1;
                        if (app.deadlineDays > 365) app.deadlineDays = 365;
                        
                        ImGui::InputText("Reference##details", app.bufReference, IM_ARRAYSIZE(app.bufReference));
                        ImGui::InputTextMultiline("Extras##details", app.bufExtras, IM_ARRAYSIZE(app.bufExtras), ImVec2(0, 80));
                    }
                    
                    ImGui::Separator();
                    
                    if (isCompletedCustomer) {
                        ImGui::BeginDisabled();
                    }
                    
                    if (ImGui::Button("Update Order##btn", ImVec2(150, 0))) {
                        if (strlen(app.bufOrderName) == 0) {
                            ImGui::OpenPopup("update_validation_error");
                        } else {
                            using namespace std::chrono;
                            auto deadline = system_clock::now() + hours(24 * app.deadlineDays);
                            Customer cust(app.loggedUserID, "User");
                            cust.modifyOrder(app.manager, order->orderID,
                                            std::string(app.bufOrderName),
                                            static_cast<OrderKind>(app.kindIndex),
                                            deadline,
                                            std::string(app.bufReference),
                                            std::string(app.bufExtras));
                            app.currentScreen = AppState::CustomerMenu;
                            app.selectedOrderID = 0;
                            app.detailsPrefilled = false;
                        }
                    }
                    
                    ImGui::SameLine();
                    
                    if (ImGui::Button("Delete Order##btn", ImVec2(150, 0))) {
                        ImGui::OpenPopup("delete_confirm");
                    }
                    
                    if (isCompletedCustomer) {
                        ImGui::EndDisabled();
                    }
                    
                    ImGui::SameLine();
                    
                    if (ImGui::Button("Back##details", ImVec2(150, 0))) {
                        app.currentScreen = AppState::CustomerMenu;
                        app.detailsPrefilled = false;
                    }
                    
                    // Delete confirmation popup
                    if (ImGui::BeginPopupModal("delete_confirm", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                        ImGui::Text("Are you sure you want to delete this order?");
                        ImGui::Text("This action cannot be undone.");
                        ImGui::Separator();
                        if (ImGui::Button("Yes, Delete", ImVec2(120, 0))) {
                            app.manager.deleteOrder(app.selectedOrderID);
                            app.currentScreen = AppState::CustomerMenu;
                            app.selectedOrderID = 0;
                            app.detailsPrefilled = false;
                            ImGui::CloseCurrentPopup();
                        }
                        ImGui::SameLine();
                        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                            ImGui::CloseCurrentPopup();
                        }
                        ImGui::EndPopup();
                    }
                    
                    // Validation error popup
                    if (ImGui::BeginPopupModal("update_validation_error", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                        ImGui::Text("Order name cannot be empty!");
                        if (ImGui::Button("OK", ImVec2(120, 0))) {
                            ImGui::CloseCurrentPopup();
                        }
                        ImGui::EndPopup();
                    }
                    
                    ImGui::End();
                }
            }
        }

        // ========== EDITOR MENU WINDOW ==========
        if (app.currentScreen == AppState::EditorMenu) {
            ImGui::SetNextWindowSize(ImVec2(700, 450), ImGuiCond_FirstUseEver);
            ImGui::Begin("Editor Menu", nullptr);
            
            ImGui::Text("Logged in as: %s (Editor, ID: %d)", app.loggedUsername.c_str(), app.loggedUserID);
            if (ImGui::Button("Logout##editormenu")) {
                app.currentScreen = AppState::LoginChoice;
                app.loggedUserID = 0;
                app.loggedUsername = "";
                app.selectedOrderID = 0;
            }
            
            ImGui::Separator();
            ImGui::Text("All Orders:");
            
            ImGui::BeginChild("editor_orders_list", ImVec2(0, 250), true);
            for (const auto& order : app.manager.getOrders()) {
                char label[256];
                const char* statusStr = "";
                switch (order.status) {
                    case OrderStatus::Pending: statusStr = "Pending"; break;
                    case OrderStatus::InProgress: statusStr = "In Progress"; break;
                    case OrderStatus::Completed: statusStr = "Completed"; break;
                }
                snprintf(label, sizeof(label), "[ID:%d] %s (Customer: %d) [%s]", 
                         order.orderID, order.orderName.c_str(), order.customerID, statusStr);
                
                if (ImGui::Selectable(label, app.selectedOrderID == order.orderID)) {
                    app.selectedOrderID = order.orderID;
                    app.statusIndex = static_cast<int>(order.status);
                    app.currentScreen = AppState::EditorOrderDetails;
                }
            }
            ImGui::EndChild();
            
            ImGui::Separator();
            
            if (ImGui::Button("Refresh##editor", ImVec2(150, 0))) {
                // Refresh is automatic (list is dynamic)
            }
            
            ImGui::End();
        }

        // ========== EDITOR ORDER DETAILS WINDOW ==========
        if (app.currentScreen == AppState::EditorOrderDetails) {
            ImGui::SetNextWindowSize(ImVec2(600, 550), ImGuiCond_FirstUseEver);
            ImGui::Begin("Editor - Order Details", nullptr);
            
            if (app.selectedOrderID == 0) {
                ImGui::Text("No order selected.");
                if (ImGui::Button("Back##editor_noselect")) {
                    app.currentScreen = AppState::EditorMenu;
                }
                ImGui::End();
            } else {
                Order* order = app.manager.findOrder(app.selectedOrderID);
                
                if (!order) {
                    ImGui::Text("Order not found (may have been deleted).");
                    if (ImGui::Button("Back##editor_notfound")) {
                        app.currentScreen = AppState::EditorMenu;
                        app.selectedOrderID = 0;
                    }
                    ImGui::End();
                } else {
                    // Prefill on first open
                    if (!app.detailsPrefilled) {
                        strncpy(app.bufOrderName, order->orderName.c_str(), sizeof(app.bufOrderName) - 1);
                        app.bufOrderName[sizeof(app.bufOrderName) - 1] = '\0';
                        strncpy(app.bufReference, order->reference.c_str(), sizeof(app.bufReference) - 1);
                        app.bufReference[sizeof(app.bufReference) - 1] = '\0';
                        strncpy(app.bufFinalLink, order->finalLink.c_str(), sizeof(app.bufFinalLink) - 1);
                        app.bufFinalLink[sizeof(app.bufFinalLink) - 1] = '\0';
                        strncpy(app.bufEditorAssign, order->editorAssigned.c_str(), sizeof(app.bufEditorAssign) - 1);
                        app.bufEditorAssign[sizeof(app.bufEditorAssign) - 1] = '\0';
                        app.kindIndex = static_cast<int>(order->orderKind);
                        app.statusIndex = static_cast<int>(order->status);
                        app.detailsPrefilled = true;
                    }
                    
                    ImGui::Text("Order ID: %d", order->orderID);
                    ImGui::Text("Customer ID: %d", order->customerID);
                    ImGui::Separator();
                    
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
                    
                    ImGui::Text("Order Name: (Read-only)");
                    ImGui::InputText("##order_name_display", app.bufOrderName, sizeof(app.bufOrderName), ImGuiInputTextFlags_ReadOnly);
                    
                    ImGui::Text("Order Kind: (Read-only)");
                    const char* kinds[] = { "Logo", "Status", "Feed", "Asset", "Document", "Other" };
                    ImGui::Text("%s", kinds[app.kindIndex]);
                    
                    ImGui::Text("Reference: (Read-only)");
                    ImGui::InputText("##reference_display", app.bufReference, sizeof(app.bufReference), ImGuiInputTextFlags_ReadOnly);
                    
                    ImGui::PopStyleColor(2);
                    
                    // If order is completed, lock all editing
                    bool isCompleted = (order->status == OrderStatus::Completed);
                    
                    if (isCompleted) {
                        ImGui::Separator();
                        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "ORDER COMPLETED - LOCKED");
                        ImGui::Text("Status: Completed");
                        ImGui::Text("Final Link: %s", order->finalLink.c_str());
                    } else {
                        ImGui::Separator();
                        ImGui::TextColored(ImVec4(0.2f, 0.8f, 1.0f, 1.0f), "Editable Fields:");
                        ImGui::Text("Order Status:");
                        const char* statuses[] = { "Pending", "In Progress", "Completed" };
                        ImGui::Combo("##editor_status", &app.statusIndex, statuses, IM_ARRAYSIZE(statuses));
                        
                        ImGui::Text("Final Link:");
                        ImGui::InputText("##final_link_editor", app.bufFinalLink, sizeof(app.bufFinalLink));
                        
                        ImGui::Separator();
                        ImGui::TextColored(ImVec4(0.2f, 0.8f, 1.0f, 1.0f), "Assignment:");
                        
                        // Display current assigned editor
                        if (!order->editorAssigned.empty()) {
                            ImGui::Text("Assigned to: %s", order->editorAssigned.c_str());
                            
                            // Show "Unassign" button only if assigned to current editor
                            if (order->editorAssigned == app.loggedUsername) {
                                if (ImGui::Button("Unassign from Me##editor_unassign", ImVec2(150, 0))) {
                                    order->unassignEditor();
                                    ImGui::OpenPopup("editor_unassign_success");
                                }
                            }
                        } else {
                            ImGui::Text("Assigned to: (None)");
                            if (ImGui::Button("Assign to Me##editor_assign", ImVec2(150, 0))) {
                                order->assignEditor(app.loggedUsername);
                                ImGui::OpenPopup("editor_assign_success");
                            }
                        }
                    }
                    
                    ImGui::Separator();
                    
                    // Save changes button (disabled if completed)
                    if (isCompleted) {
                        ImGui::BeginDisabled();
                    }
                    if (ImGui::Button("Save Changes##editor", ImVec2(150, 0))) {
                        order->status = static_cast<OrderStatus>(app.statusIndex);
                        order->finalLink = std::string(app.bufFinalLink);
                        ImGui::OpenPopup("editor_save_success");
                    }
                    
                    if (isCompleted) {
                        ImGui::EndDisabled();
                    }
                    
                    ImGui::SameLine();
                    
                    // Delete order button (disabled if completed)
                    if (isCompleted) {
                        ImGui::BeginDisabled();
                    }
                    if (ImGui::Button("Delete Order##editor", ImVec2(150, 0))) {
                        ImGui::OpenPopup("editor_delete_confirm");
                    }
                    
                    if (isCompleted) {
                        ImGui::EndDisabled();
                    }
                    
                    ImGui::SameLine();
                    
                    // Back button
                    if (ImGui::Button("Back##editor_details", ImVec2(150, 0))) {
                        app.currentScreen = AppState::EditorMenu;
                        app.detailsPrefilled = false;
                    }
                    
                    // Save success popup
                    if (ImGui::BeginPopupModal("editor_save_success", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                        ImGui::Text("Order updated successfully!");
                        if (ImGui::Button("OK##save_success", ImVec2(120, 0))) {
                            ImGui::CloseCurrentPopup();
                        }
                        ImGui::EndPopup();
                    }
                    
                    // Delete confirmation popup
                    if (ImGui::BeginPopupModal("editor_delete_confirm", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                        ImGui::Text("Are you sure you want to delete this order?");
                        ImGui::Text("This action cannot be undone.");
                        ImGui::Separator();
                        if (ImGui::Button("Yes, Delete##editor", ImVec2(120, 0))) {
                            app.manager.deleteOrder(app.selectedOrderID);
                            app.currentScreen = AppState::EditorMenu;
                            app.selectedOrderID = 0;
                            app.detailsPrefilled = false;
                            ImGui::CloseCurrentPopup();
                        }
                        ImGui::SameLine();
                        if (ImGui::Button("Cancel##delete", ImVec2(120, 0))) {
                            ImGui::CloseCurrentPopup();
                        }
                        ImGui::EndPopup();
                    }
                    
                    // Assignment success popup
                    if (ImGui::BeginPopupModal("editor_assign_success", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                        ImGui::Text("You have been assigned to this order!");
                        if (ImGui::Button("OK##assign_success", ImVec2(120, 0))) {
                            ImGui::CloseCurrentPopup();
                        }
                        ImGui::EndPopup();
                    }
                    
                    // Unassignment success popup
                    if (ImGui::BeginPopupModal("editor_unassign_success", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                        ImGui::Text("You have been unassigned from this order!");
                        if (ImGui::Button("OK##unassign_success", ImVec2(120, 0))) {
                            ImGui::CloseCurrentPopup();
                        }
                        ImGui::EndPopup();
                    }
                    
                    ImGui::End();
                }
            }
        }

        // Rendering
        ImGui::EndFrame();
        g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
        D3DCOLOR clear_col_dx = D3DCOLOR_RGBA(70, 100, 140, 255);
        g_pd3dDevice->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clear_col_dx, 1.0f, 0);
        if (g_pd3dDevice->BeginScene() >= 0) {
            ImGui::Render();
            ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
            g_pd3dDevice->EndScene();
        }

        HRESULT hr = g_pd3dDevice->Present(nullptr, nullptr, nullptr, nullptr);
        if (hr == D3DERR_DEVICELOST && g_pd3dDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
            ResetDevice();
    }

    // Save data before shutdown
    SaveManager::saveToFile("savedata.txt", app.manager, app.userManager);

    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
    return 0;
}

bool CreateDeviceD3D(HWND hWnd) {
    if ((g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == nullptr)
        return false;

    ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
    g_d3dpp.Windowed = TRUE;
    g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    g_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
    g_d3dpp.EnableAutoDepthStencil = TRUE;
    g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;

    if (g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dpp, &g_pd3dDevice) < 0)
        return false;

    return true;
}

void CleanupDeviceD3D() {
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
    if (g_pD3D) { g_pD3D->Release(); g_pD3D = nullptr; }
}

void ResetDevice() {
    ImGui_ImplDX9_InvalidateDeviceObjects();
    HRESULT hr = g_pd3dDevice->Reset(&g_d3dpp);
    if (hr == D3DERR_INVALIDCALL)
        IM_ASSERT(0);
    ImGui_ImplDX9_CreateDeviceObjects();
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg) {
    case WM_SIZE:
        if (g_pd3dDevice != nullptr && wParam != SIZE_MINIMIZED) {
            g_d3dpp.BackBufferWidth = LOWORD(lParam);
            g_d3dpp.BackBufferHeight = HIWORD(lParam);
            ResetDevice();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU)
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}