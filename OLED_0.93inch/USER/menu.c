#include "menu.h"
#include "oled_128x64.h" // Include thu vien OLED
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include "delay.h"
// Bien global de luu handle I2C
extern I2C_Handle_t I2C1Handle;

static uint32_t system_time_ms = 0;

/**
 * Khoi tao menu
 */
void Menu_Init(Menu* menu, const char* title, bool show_title) {
    if (!menu) return;
    
    menu->item_count = 0;
    menu->current_index = 0;
    menu->display_start = 0;
    menu->show_title = show_title;
    menu->auto_refresh_enabled = false;
    menu->global_refresh_interval = 1000;
    menu->last_global_refresh = 0;
    
    if (title && strlen(title) < MAX_ITEM_NAME_LENGTH) {
        strcpy(menu->title, title);
    } else {
        strcpy(menu->title, "Menu");
    }
    
    // Xóa tat ca items
    for (int i = 0; i < MAX_MENU_ITEMS; i++) {
        menu->items[i].name[0] = '\0';
        menu->items[i].callback = NULL;
        menu->items[i].enabled = false;
        menu->items[i].item_type = MENU_ITEM_NORMAL;
        menu->items[i].refresh_callback = NULL;
        menu->items[i].refresh_type = REFRESH_NONE;
        menu->items[i].use_dynamic_content = false;
    }
}

/*
 * Thêm item vào menu voi tính nang refresh
 */
void Menu_AddItemWithRefresh(Menu* menu, const char* name, void (*callback)(void), 
                            void (*refresh_callback)(char* buffer, uint8_t buffer_size),
                            RefreshType refresh_type, uint32_t refresh_interval_ms, bool enabled) {
    if (!menu || !name || menu->item_count >= MAX_MENU_ITEMS) return;
    
    // Copy tên item
    strncpy(menu->items[menu->item_count].name, name, MAX_ITEM_NAME_LENGTH - 1);
    menu->items[menu->item_count].name[MAX_ITEM_NAME_LENGTH - 1] = '\0';
    
    // Thiet lap callback chính
    menu->items[menu->item_count].callback = callback;
    menu->items[menu->item_count].enabled = enabled;
    menu->items[menu->item_count].item_type = MENU_ITEM_NORMAL;
    
    // Thiet lap refresh callback và cau hình
    menu->items[menu->item_count].refresh_callback = refresh_callback;
    menu->items[menu->item_count].refresh_type = refresh_type;
    menu->items[menu->item_count].refresh_interval_ms = refresh_interval_ms;
    menu->items[menu->item_count].last_refresh_time = 0;
    
    // Thiet lap noi dung dong
    menu->items[menu->item_count].use_dynamic_content = (refresh_callback != NULL);
    
    // Khoi tao display buffer voi noi dung ban dau
    if (menu->items[menu->item_count].use_dynamic_content) {
        // Neu có refresh callback, goi ngay lan dau de khoi tao
        refresh_callback(menu->items[menu->item_count].display_buffer, MAX_ITEM_NAME_LENGTH);
    } else {
        // Neu không có refresh, copy tên goc vào display buffer
        strncpy(menu->items[menu->item_count].display_buffer, name, MAX_ITEM_NAME_LENGTH - 1);
        menu->items[menu->item_count].display_buffer[MAX_ITEM_NAME_LENGTH - 1] = '\0';
    }
    
    menu->item_count++;
    
    // T? d?ng di chuy?n cursor d?n item selectable d?u tiên n?u c?n
    if (menu->item_count == 1 && Menu_IsItemSelectable(menu, 0)) {
        menu->current_index = 0;
    }
}

/**
 * Thêm item vào menu
 */
void Menu_AddItem(Menu* menu, const char* name, void (*callback)(void), bool enabled) {
    // Goi AddItemWithRefresh v?i refresh_type = REFRESH_NONE
    Menu_AddItemWithRefresh(menu, name, callback, NULL, REFRESH_NONE, 0, enabled);
}

// ===== IMPLEMENTATION CÁC HÀM INFO ITEMS MOI =====

/**
 * Thêm item thông tin không th? select
 */
void Menu_AddInfoItem(Menu* menu, const char* info_text) {
    if (!menu || !info_text || menu->item_count >= MAX_MENU_ITEMS) return;
    
    // Copy text info
    strncpy(menu->items[menu->item_count].name, info_text, MAX_ITEM_NAME_LENGTH - 1);
    menu->items[menu->item_count].name[MAX_ITEM_NAME_LENGTH - 1] = '\0';
    
    // Thi?t l?p nhu info item
    menu->items[menu->item_count].callback = NULL;
    menu->items[menu->item_count].enabled = false;  // Info items luôn disabled
    menu->items[menu->item_count].item_type = MENU_ITEM_INFO;
    menu->items[menu->item_count].refresh_callback = NULL;
    menu->items[menu->item_count].refresh_type = REFRESH_NONE;
    menu->items[menu->item_count].refresh_interval_ms = 0;
    menu->items[menu->item_count].last_refresh_time = 0;
    menu->items[menu->item_count].use_dynamic_content = false;
    
    // Copy vào display buffer
    strncpy(menu->items[menu->item_count].display_buffer, info_text, MAX_ITEM_NAME_LENGTH - 1);
    menu->items[menu->item_count].display_buffer[MAX_ITEM_NAME_LENGTH - 1] = '\0';
    
    menu->item_count++;
}

/**
 * Thêm item thông tin v?i refresh callback
 */
void Menu_AddInfoItemWithRefresh(Menu* menu, const char* info_text,
                                void (*refresh_callback)(char* buffer, uint8_t buffer_size),
                                RefreshType refresh_type, uint32_t refresh_interval_ms) {
    if (!menu || !info_text || menu->item_count >= MAX_MENU_ITEMS) return;
    
    // Copy text info
    strncpy(menu->items[menu->item_count].name, info_text, MAX_ITEM_NAME_LENGTH - 1);
    menu->items[menu->item_count].name[MAX_ITEM_NAME_LENGTH - 1] = '\0';
    
    // Thi?t l?p nhu info item v?i refresh
    menu->items[menu->item_count].callback = NULL;
    menu->items[menu->item_count].enabled = false;  // Info items luôn disabled
    menu->items[menu->item_count].item_type = MENU_ITEM_INFO;
    menu->items[menu->item_count].refresh_callback = refresh_callback;
    menu->items[menu->item_count].refresh_type = refresh_type;
    menu->items[menu->item_count].refresh_interval_ms = refresh_interval_ms;
    menu->items[menu->item_count].last_refresh_time = 0;
    menu->items[menu->item_count].use_dynamic_content = (refresh_callback != NULL);
    
    // Kh?i t?o display buffer
    if (refresh_callback != NULL) {
        refresh_callback(menu->items[menu->item_count].display_buffer, MAX_ITEM_NAME_LENGTH);
    } else {
        strncpy(menu->items[menu->item_count].display_buffer, info_text, MAX_ITEM_NAME_LENGTH - 1);
        menu->items[menu->item_count].display_buffer[MAX_ITEM_NAME_LENGTH - 1] = '\0';
    }
    
    menu->item_count++;
}

/**
 * Thêm separator (dòng ngan cách)
 */
void Menu_AddSeparator(Menu* menu, const char* separator_text) {
    if (!menu || menu->item_count >= MAX_MENU_ITEMS) return;
    
    const char* text = separator_text ? separator_text : "---------------";
    
    // Copy text separator
    strncpy(menu->items[menu->item_count].name, text, MAX_ITEM_NAME_LENGTH - 1);
    menu->items[menu->item_count].name[MAX_ITEM_NAME_LENGTH - 1] = '\0';
    
    // Thi?t l?p nhu separator item
    menu->items[menu->item_count].callback = NULL;
    menu->items[menu->item_count].enabled = false;
    menu->items[menu->item_count].item_type = MENU_ITEM_SEPARATOR;
    menu->items[menu->item_count].refresh_callback = NULL;
    menu->items[menu->item_count].refresh_type = REFRESH_NONE;
    menu->items[menu->item_count].refresh_interval_ms = 0;
    menu->items[menu->item_count].last_refresh_time = 0;
    menu->items[menu->item_count].use_dynamic_content = false;
    
    // Copy vào display buffer
    strncpy(menu->items[menu->item_count].display_buffer, text, MAX_ITEM_NAME_LENGTH - 1);
    menu->items[menu->item_count].display_buffer[MAX_ITEM_NAME_LENGTH - 1] = '\0';
    
    menu->item_count++;
}

/**
 * Ki?m tra xem item có th? select du?c không
 */
bool Menu_IsItemSelectable(Menu* menu, uint8_t index) {
    if (!menu || index >= menu->item_count) return false;
    
    return (menu->items[index].item_type == MENU_ITEM_NORMAL && menu->items[index].enabled);
}

/**
 * Tìm item selectable ti?p theo
 */
uint8_t Menu_FindNextSelectableItem(Menu* menu, uint8_t current_index) {
    if (!menu || menu->item_count == 0) return current_index;
    
    uint8_t start_index = current_index;
    uint8_t index = current_index;
    
    do {
        index = (index + 1) % menu->item_count;
        if (Menu_IsItemSelectable(menu, index)) {
            return index;
        }
    } while (index != start_index);
    
    // N?u không tìm th?y item selectable nào, gi? nguyên index hi?n t?i
    return current_index;
}

/**
 * Tìm item selectable tru?c dó
 */
uint8_t Menu_FindPrevSelectableItem(Menu* menu, uint8_t current_index) {
    if (!menu || menu->item_count == 0) return current_index;
    
    uint8_t start_index = current_index;
    uint8_t index = current_index;
    
    do {
        index = (index == 0) ? menu->item_count - 1 : index - 1;
        if (Menu_IsItemSelectable(menu, index)) {
            return index;
        }
    } while (index != start_index);
    
    // N?u không tìm th?y item selectable nào, gi? nguyên index hi?n t?i
    return current_index;
}

/**
 * C?p nh?t n?i dung info item
 */
void Menu_UpdateInfoItem(Menu* menu, uint8_t index, const char* new_text) {
    if (!menu || index >= menu->item_count || !new_text) return;
    
    MenuItem* item = &menu->items[index];
    if (item->item_type != MENU_ITEM_INFO && item->item_type != MENU_ITEM_SEPARATOR) return;
    
    // C?p nh?t n?i dung
    strncpy(item->name, new_text, MAX_ITEM_NAME_LENGTH - 1);
    item->name[MAX_ITEM_NAME_LENGTH - 1] = '\0';
    
    strncpy(item->display_buffer, new_text, MAX_ITEM_NAME_LENGTH - 1);
    item->display_buffer[MAX_ITEM_NAME_LENGTH - 1] = '\0';
    
    // C?p nh?t hi?n th? n?u item dang visible
    Menu_UpdateItemLine(menu, index);
}

/**
 * Xóa item khoi menu
 */
void Menu_RemoveItem(Menu* menu, uint8_t index) {
    if (!menu || index >= menu->item_count) return;
    
    // Dich chuyen các item ve phía truoc
    for (uint8_t i = index; i < menu->item_count - 1; i++) {
        menu->items[i] = menu->items[i + 1];
    }
    menu->item_count--;
    
    // Ðieu chinh cursor neu can - d?m b?o cursor luôn ? item selectable
    if (menu->item_count > 0) {
        if (menu->current_index >= menu->item_count) {
            menu->current_index = menu->item_count - 1;
        }
        
        // Ð?m b?o cursor ? item selectable
        if (!Menu_IsItemSelectable(menu, menu->current_index)) {
            uint8_t next_selectable = Menu_FindNextSelectableItem(menu, menu->current_index);
            if (next_selectable == menu->current_index) {
                // Không có item selectable nào, th? tìm ngu?c l?i
                next_selectable = Menu_FindPrevSelectableItem(menu, menu->current_index);
            }
            menu->current_index = next_selectable;
        }
    } else {
        menu->current_index = 0;
    }
}

/**
 * Bat/tat item
 */
void Menu_SetItemEnabled(Menu* menu, uint8_t index, bool enabled) {
    if (!menu || index >= menu->item_count) return;
    
    // Ch? normal items m?i có th? enable/disable
    if (menu->items[index].item_type == MENU_ITEM_NORMAL) {
        menu->items[index].enabled = enabled;
        
        // N?u item hi?n t?i b? disable, tìm item selectable khác
        if (!enabled && menu->current_index == index) {
            uint8_t next_selectable = Menu_FindNextSelectableItem(menu, index);
            if (next_selectable == index) {
                next_selectable = Menu_FindPrevSelectableItem(menu, index);
            }
            menu->current_index = next_selectable;
        }
    }
}

/**
 * C?u hình refresh cho item dã có san
 */
void Menu_SetItemRefresh(Menu* menu, uint8_t index, void (*refresh_callback)(char* buffer, uint8_t buffer_size),
                        RefreshType refresh_type, uint32_t refresh_interval_ms) {
    if (!menu || index >= menu->item_count) return;
    
    menu->items[index].refresh_callback = refresh_callback;
    menu->items[index].refresh_type = refresh_type;
    menu->items[index].refresh_interval_ms = refresh_interval_ms;
    menu->items[index].last_refresh_time = 0;
    menu->items[index].use_dynamic_content = (refresh_callback != NULL);
    
    // Neu có refresh callback, goi ngay de cap nhat
    if (refresh_callback != NULL) {
        refresh_callback(menu->items[index].display_buffer, MAX_ITEM_NAME_LENGTH);
    }
}
                                                
/*
 * Refresh m?t item c? th?
 */
void Menu_RefreshItem(Menu* menu, uint8_t index, bool force_refresh) {
    if (!menu || index >= menu->item_count) return;
    
    MenuItem* item = &menu->items[index];
    
    // Ki?m tra n?u item không có refresh callback
    if (!item->refresh_callback || !item->use_dynamic_content) return;
    
    uint32_t current_time = Menu_GetSystemTime();
    bool should_refresh = force_refresh;
    
    // Ki?m tra di?u ki?n refresh theo lo?i
    switch (item->refresh_type) {
        case REFRESH_NONE:
            return;
            
        case REFRESH_ALWAYS:
            should_refresh = true;
            break;
            
        case REFRESH_WHEN_SELECTED:
            should_refresh = (menu->current_index == index);
            break;
            
        case REFRESH_PERIODIC:
            should_refresh = should_refresh || 
                           (current_time - item->last_refresh_time >= item->refresh_interval_ms);
            break;
    }
    
    // Th?c hi?n refresh n?u c?n
    if (should_refresh) {
        item->refresh_callback(item->display_buffer, MAX_ITEM_NAME_LENGTH);
        item->last_refresh_time = current_time;
        Menu_UpdateItemLine(menu, index);
    }
}

/**
 * Refresh t?t c? các item trong menu
 */
void Menu_RefreshAll(Menu* menu, bool force_refresh) {
    if (!menu) return;
    
    for (uint8_t i = 0; i < menu->item_count; i++) {
        Menu_RefreshItem(menu, i, force_refresh);
    }
}

/**
 * C?p nh?t menu - g?i trong main loop
 */
void Menu_Update(Menu* menu, uint32_t current_time_ms) {
    if (!menu) return;
    
    system_time_ms = current_time_ms;
    
    if (menu->auto_refresh_enabled) {
        if (current_time_ms - menu->last_global_refresh >= menu->global_refresh_interval) {
            Menu_RefreshAll(menu, false);
            menu->last_global_refresh = current_time_ms;
        }
    } else {
        // CH? refresh items có REFRESH_PERIODIC ho?c REFRESH_WHEN_SELECTED
        for (uint8_t i = 0; i < menu->item_count; i++) {
            if (menu->items[i].refresh_type == REFRESH_PERIODIC || 
                (menu->items[i].refresh_type == REFRESH_WHEN_SELECTED && menu->current_index == i)) {
                Menu_RefreshItem(menu, i, false);
            }
        }
    }
}

/**
 * C?u hình global refresh cho toàn b? menu
 */
void Menu_SetGlobalRefresh(Menu* menu, bool enabled, uint32_t interval_ms) {
    if (!menu) return;
    
    menu->auto_refresh_enabled = enabled;
    menu->global_refresh_interval = interval_ms;
    menu->last_global_refresh = Menu_GetSystemTime();
}

/**
 * L?y th?i gian h? th?ng (milliseconds)
 */
uint32_t Menu_GetSystemTime(void) {
    return millis();          // Arduino style
}

/**
 * L?y text hi?n th? c?a item (uu tiên display_buffer n?u có dynamic content)
 */
const char* Menu_GetItemDisplayText(Menu* menu, uint8_t index) {
    if (!menu || index >= menu->item_count) return "";
    
    if (menu->items[index].use_dynamic_content) {
        return menu->items[index].display_buffer;
    } else {
        return menu->items[index].name;
    }
}

/**
 * Xu lý phím bam - C?P NH?T Ð? SKIP INFO ITEMS
 */
void Menu_HandleKey(Menu* menu, MenuKey key) {
    if (!menu || menu->item_count == 0) return;
    uint8_t oldIndex = menu->current_index;
    
    switch (key) {
        case MENU_KEY_NONE:
            return;
            
        case MENU_KEY_UP:
            menu->current_index = Menu_FindPrevSelectableItem(menu, menu->current_index);
            
            // Ðieu chinh display window
            if (menu->current_index < menu->display_start) {
                menu->display_start = menu->current_index;
            }
            // Cap nhat hien thi
            Menu_UpdatePartial(menu, oldIndex);
            break;
            
        case MENU_KEY_DOWN:
            menu->current_index = Menu_FindNextSelectableItem(menu, menu->current_index);
            
            // Ði?u ch?nh display window
            uint8_t max_display_items = menu->show_title ? MENU_DISPLAY_ROWS - 1 : MENU_DISPLAY_ROWS;
            if (menu->current_index >= menu->display_start + max_display_items) {
                menu->display_start = menu->current_index - max_display_items + 1;
            }
            if (menu->current_index < menu->display_start) {
                menu->display_start = menu->current_index;
            }
            // Cap nhat hien thi
            Menu_UpdatePartial(menu, oldIndex);
            break;
            
        case MENU_KEY_SELECT:
            Menu_ExecuteCurrentItem(menu);
            break;
            
        case MENU_KEY_BACK:
            Menu_OnBack(menu);
            break;
    }
}

/**
 * Hien thi menu lên OLED - C?P NH?T Ð? X? LÝ INFO ITEMS
 */
void Menu_Display(Menu* menu) {
    if (!menu) return;
    
    // Xóa màn hình
    OLED_Fill(0x00, 2);
    
    uint8_t row = 0;
    uint8_t max_display_items = MENU_DISPLAY_ROWS;
    
    // Hien thi tiêu de neu có
    if (menu->show_title) {
        OLED_SetCursor(0, 0, 2);
        OLED_WriteString(menu->title);
        row = 1;
        max_display_items = MENU_DISPLAY_ROWS - 1;
    }
    
    // Hi?n th? các menu items
    for (uint8_t i = 0; i < max_display_items && (menu->display_start + i) < menu->item_count; i++) {
        uint8_t item_index = menu->display_start + i;
        OLED_SetCursor(0, row + i, 2);
        
        char display_line[32] = {0};
        const char* display_text = Menu_GetItemDisplayText(menu, item_index);
        MenuItem* item = &menu->items[item_index];
        
        // X? lý hi?n th? theo lo?i item
        if (item->item_type == MENU_ITEM_INFO) {
            // Info item - không có cursor, có th? có prefix d?c bi?t
            snprintf(display_line, sizeof(display_line), "i %.27s", display_text);
        } else if (item->item_type == MENU_ITEM_SEPARATOR) {
            // Separator - hi?n th? nguyên dòng không có prefix
            snprintf(display_line, sizeof(display_line), "%.29s", display_text);
        } else {
            // Normal item
            if (item_index == menu->current_index) {
                snprintf(display_line, sizeof(display_line), "> %.26s", display_text);
            } else {
                snprintf(display_line, sizeof(display_line), "  %.26s", display_text);
            }
            
            // Thêm ký hi?u disable n?u c?n
            if (!item->enabled) {
                size_t current_len = strlen(display_line);
                if (current_len < sizeof(display_line) - 5) {
                    strcat(display_line, " (-)");
                }
            }
        }
        
        OLED_WriteString(display_line);
    }
    
    // Hi?n th? scroll indicator n?u có nhi?u item hon màn hình
    if (menu->item_count > max_display_items) {
        // Hi?n th? d?u scroll up
        if (menu->display_start > 0) {
            OLED_SetCursor(120, menu->show_title ? 1 : 0, 2);
            OLED_WriteString("^");
        }
        
        // Hi?n th? d?u scroll down
        if (menu->display_start + max_display_items < menu->item_count) {
            OLED_SetCursor(120, MENU_DISPLAY_ROWS - 1, 2);
            OLED_WriteString("v");
        }
    }
}

void Menu_UpdatePartial(Menu* menu, uint8_t old_index) {
    if (!menu) return;

    uint8_t max_display_items = MENU_DISPLAY_ROWS;
    uint8_t row_offset = menu->show_title ? 1 : 0;
    if (menu->show_title) {
        max_display_items -= 1;
    }

    // C?p nh?t c? old index và current index
    for (uint8_t i = 0; i < 2; i++) {
        uint8_t index = (i == 0) ? old_index : menu->current_index;

        if (index < menu->display_start || index >= menu->display_start + max_display_items) {
            continue;
        }

        uint8_t row = row_offset + (index - menu->display_start);
        OLED_SetCursor(0, row, 2);

        char display_line[32] = {0};
        const char* display_text = Menu_GetItemDisplayText(menu, index);
        MenuItem* item = &menu->items[index];

        // X? lý hi?n th? theo lo?i item
        if (item->item_type == MENU_ITEM_INFO) {
            snprintf(display_line, sizeof(display_line), "i %.27s", display_text);
        } else if (item->item_type == MENU_ITEM_SEPARATOR) {
            snprintf(display_line, sizeof(display_line), "%.29s", display_text);
        } else {
            if (index == menu->current_index) {
                snprintf(display_line, sizeof(display_line), "> %.26s", display_text);
            } else {
                snprintf(display_line, sizeof(display_line), "  %.26s", display_text);
            }

            if (!item->enabled) {
                size_t current_len = strlen(display_line);
                if (current_len < sizeof(display_line) - 5) {
                    strcat(display_line, " (-)");
                }
            }
        }

        OLED_WriteString(display_line);
    }
}

void Menu_UpdateItemLine(Menu* menu, uint8_t index) {
    if (!menu || index >= menu->item_count) return;

    uint8_t max_display_items = MENU_DISPLAY_ROWS;
    uint8_t row_offset = menu->show_title ? 1 : 0;
    if (menu->show_title) {
        max_display_items -= 1;
    }

    // Ki?m tra xem item có n?m trong vùng hi?n th? không
    if (index < menu->display_start || index >= menu->display_start + max_display_items) {
        return;
    }

    uint8_t row = row_offset + (index - menu->display_start);
    OLED_SetCursor(0, row, 2);

    char display_line[32] = {0};
    const char* display_text = Menu_GetItemDisplayText(menu, index);
    MenuItem* item = &menu->items[index];

    // X? lý hi?n th? theo lo?i item
    if (item->item_type == MENU_ITEM_INFO) {
        snprintf(display_line, sizeof(display_line), "i %.27s", display_text);
    } else if (item->item_type == MENU_ITEM_SEPARATOR) {
        snprintf(display_line, sizeof(display_line), "%.29s", display_text);
    } else {
        if (index == menu->current_index) {
            snprintf(display_line, sizeof(display_line), "> %.26s", display_text);
        } else {
            snprintf(display_line, sizeof(display_line), "  %.26s", display_text);
        }

        if (!item->enabled) {
            size_t current_len = strlen(display_line);
            if (current_len < sizeof(display_line) - 5) {
                strcat(display_line, " (-)");
            }
        }
    }

    OLED_WriteString(display_line);
}

/**
 * Set cursor d?n item c? th? - C?P NH?T Ð? CH? CH?N SELECTABLE ITEMS
 */
void Menu_SetCursor(Menu* menu, uint8_t index) {
    if (!menu || index >= menu->item_count) return;
    
    // Ch? cho phép set cursor t?i selectable items
    if (!Menu_IsItemSelectable(menu, index)) {
        index = Menu_FindNextSelectableItem(menu, index);
    }
    
    menu->current_index = index;
    
    // Ði?u ch?nh display window
    uint8_t max_display_items = menu->show_title ? MENU_DISPLAY_ROWS - 1 : MENU_DISPLAY_ROWS;
    if (index < menu->display_start) {
        menu->display_start = index;
    } else if (index >= menu->display_start + max_display_items) {
        menu->display_start = index - max_display_items + 1;
    }
    
    Menu_Display(menu);
}

/**
 * L?y index c?a item hi?n t?i
 */
uint8_t Menu_GetCurrentIndex(Menu* menu) {
    return menu ? menu->current_index : 0;
}

/**
 * L?y tên c?a item hi?n t?i
 */
const char* Menu_GetCurrentItemName(Menu* menu) {
    if (!menu || menu->current_index >= menu->item_count) return "";
    return menu->items[menu->current_index].name;
}

/**
 * Th?c thi callback c?a item hi?n t?i
 */
void Menu_ExecuteCurrentItem(Menu* menu) {
    if (!menu || menu->current_index >= menu->item_count) return;
    
    MenuItem* current_item = &menu->items[menu->current_index];
    
    if (current_item->enabled && current_item->callback) {
        current_item->callback();
    }
    
    // Gui callback tùy chinh
    Menu_OnItemSelect(menu, menu->current_index);
}

/**
 * Callback khi ch?n item (có th? override)
 */
__weak void Menu_OnItemSelect(Menu* menu, uint8_t index) {
    // Override function này trong main.c n?u c?n x? lý d?c bi?t
}

/**
 * Callback khi b?m Back (có th? override)  
 */
__weak void Menu_OnBack(Menu* menu) {
    // Override function này trong main.c n?u c?n x? lý d?c bi?t
    // Ví d?: quay v? menu cha, thoát menu, etc.
}

/**
 * Callback m?c d?nh khi menu refresh (có th? override)
 */
__attribute__((weak)) void Menu_OnRefresh(Menu* menu) {
    // Implementation m?c d?nh - có th? override trong user code
    (void)menu;   // Suppress unused parameter warning
}