#ifndef MENU_H
#define MENU_H
#include <stdint.h>
#include <stdbool.h>

// C?u hình menu
#define MAX_MENU_ITEMS 10
#define MAX_ITEM_NAME_LENGTH 20
#define MENU_DISPLAY_ROWS 4  // S? dòng hi?n th? trên OLED

// Enum cho lo?i refresh
typedef enum {
    REFRESH_NONE = 0,        // Không refresh
    REFRESH_ALWAYS,          // Refresh liên t?c
    REFRESH_WHEN_SELECTED,   // Ch? refresh khi du?c ch?n
    REFRESH_PERIODIC         // Refresh theo chu k?
} RefreshType;

// Enum cho lo?i menu item
typedef enum {
    MENU_ITEM_NORMAL = 0,    // Item bình thu?ng có th? select
    MENU_ITEM_INFO,          // Item thông tin không th? select
    MENU_ITEM_SEPARATOR      // Item ngan cách (dòng k? ho?c kho?ng tr?ng)
} MenuItemType;

// Ki?u d? li?u cho menu item v?i refresh support
typedef struct {
    char name[MAX_ITEM_NAME_LENGTH];
    void (*callback)(void);              // Hàm du?c g?i khi ch?n item này
    void (*refresh_callback)(char* buffer, uint8_t buffer_size); // Hàm c?p nh?t n?i dung
    bool enabled;                        // Item có du?c kích ho?t không
    MenuItemType item_type;              // Lo?i item
    RefreshType refresh_type;            // Lo?i refresh
    uint32_t refresh_interval_ms;        // Chu k? refresh (ms) - ch? dùng cho REFRESH_PERIODIC
    uint32_t last_refresh_time;          // Th?i gian refresh cu?i cùng
    char display_buffer[MAX_ITEM_NAME_LENGTH]; // Buffer cho n?i dung hi?n th? d?ng
    bool use_dynamic_content;            // S? d?ng n?i dung d?ng hay không
} MenuItem;

// C?u trúc menu
typedef struct {
    MenuItem items[MAX_MENU_ITEMS];
    uint8_t item_count;      // S? lu?ng item trong menu
    uint8_t current_index;   // Item hi?n t?i dang du?c ch?n
    uint8_t display_start;   // Item d?u tiên hi?n th? trên màn hình
    char title[MAX_ITEM_NAME_LENGTH];  // Tiêu d? menu
    bool show_title;         // Có hi?n th? tiêu d? không
    bool auto_refresh_enabled; // B?t/t?t t? d?ng refresh toàn b? menu
    uint32_t global_refresh_interval; // Chu k? refresh toàn b? (ms)
    uint32_t last_global_refresh;     // Th?i gian refresh toàn b? cu?i cùng
} Menu;

// Enum cho các phím di?u khi?n
typedef enum {
    MENU_KEY_NONE,
    MENU_KEY_UP,
    MENU_KEY_DOWN,
    MENU_KEY_SELECT,
    MENU_KEY_BACK
} MenuKey;

// Các hàm API co b?n c?a thu vi?n
void Menu_Init(Menu* menu, const char* title, bool show_title);
void Menu_AddItem(Menu* menu, const char* name, void (*callback)(void), bool enabled);
void Menu_RemoveItem(Menu* menu, uint8_t index);
void Menu_SetItemEnabled(Menu* menu, uint8_t index, bool enabled);
void Menu_HandleKey(Menu* menu, MenuKey key);
void Menu_Display(Menu* menu);
void Menu_UpdatePartial(Menu* menu, uint8_t old_index);
void Menu_UpdateItemLine(Menu* menu, uint8_t index);
void Menu_SetCursor(Menu* menu, uint8_t index);
uint8_t Menu_GetCurrentIndex(Menu* menu);
const char* Menu_GetCurrentItemName(Menu* menu);
void Menu_ExecuteCurrentItem(Menu* menu);

// Các hàm m?i cho refresh functionality
void Menu_AddItemWithRefresh(Menu* menu, const char* name, void (*callback)(void), 
                            void (*refresh_callback)(char* buffer, uint8_t buffer_size),
                            RefreshType refresh_type, uint32_t refresh_interval_ms, bool enabled);
void Menu_SetItemRefresh(Menu* menu, uint8_t index, void (*refresh_callback)(char* buffer, uint8_t buffer_size),
                        RefreshType refresh_type, uint32_t refresh_interval_ms);
void Menu_RefreshItem(Menu* menu, uint8_t index, bool force_refresh);
void Menu_RefreshAll(Menu* menu, bool force_refresh);
void Menu_Update(Menu* menu, uint32_t current_time_ms); // G?i trong main loop d? x? lý refresh

// C?u hình refresh toàn b? menu
void Menu_SetGlobalRefresh(Menu* menu, bool enabled, uint32_t interval_ms);

// ===== HÀM M?I CHO INFO ITEMS =====
// Thêm item thông tin không th? select
void Menu_AddInfoItem(Menu* menu, const char* info_text);

// Thêm item thông tin v?i refresh callback
void Menu_AddInfoItemWithRefresh(Menu* menu, const char* info_text,
                                void (*refresh_callback)(char* buffer, uint8_t buffer_size),
                                RefreshType refresh_type, uint32_t refresh_interval_ms);

// Thêm separator (dòng ngan cách)
void Menu_AddSeparator(Menu* menu, const char* separator_text);

// Ki?m tra xem item có th? select du?c không
bool Menu_IsItemSelectable(Menu* menu, uint8_t index);

// Tìm item selectable ti?p theo/tru?c dó
uint8_t Menu_FindNextSelectableItem(Menu* menu, uint8_t current_index);
uint8_t Menu_FindPrevSelectableItem(Menu* menu, uint8_t current_index);

// C?p nh?t n?i dung info item
void Menu_UpdateInfoItem(Menu* menu, uint8_t index, const char* new_text);

// Utility functions
uint32_t Menu_GetSystemTime(void); // C?n implement theo h? th?ng
const char* Menu_GetItemDisplayText(Menu* menu, uint8_t index);

// Hàm callback m?c d?nh (có th? override)
void Menu_OnItemSelect(Menu* menu, uint8_t index);
void Menu_OnBack(Menu* menu);
void Menu_OnRefresh(Menu* menu); // Callback khi menu du?c refresh

#endif // MENU_H