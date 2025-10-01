#include "menu.h"
#include "oled_128x64.h"
#include "button.h"
#include "delay.h"
#include <stdio.h>
//Bien global cho button
Button button1, button2, button3;

// Bien global cho menu
Menu main_menu;
Menu settings_menu;
Menu* current_menu = &main_menu;

// Khai báo các hàm callback
void menu_settings(void);
void menu_about(void);
void menu_brightness(void);
void menu_contrast(char* buffer, uint8_t buffer_size);
void menu_back_to_main(void);

// Hàm doc phim (Cann implement theo hardware cua mình)
MenuKey read_key(void);

int main(void) {
    // Khoi tao I2C và OLED
    I2C_Config(I2C1, I2C_REMAP, 400000);
    OLED_Init(&I2C1Handle);
    OLED_Fill(0x00, 2);
		Button_Init(&button1, GPIOB, GPIO_Pin_3);
		Button_Init(&button2, GPIOB, GPIO_Pin_4);
		Button_Init(&button3, GPIOB, GPIO_Pin_5);
		delay_Init();
    
    // Khoi tao main menu
    Menu_Init(&main_menu, "Main Menu", true);
    Menu_AddItem(&main_menu, "Settings", menu_settings, true);
    Menu_AddItem(&main_menu, "About", menu_about, true);
    
    // Khoi tao settings menu
    Menu_Init(&settings_menu, "Settings", true);
    Menu_AddItem(&settings_menu, "Brightness", menu_brightness, true);
		Menu_AddInfoItemWithRefresh(&settings_menu, "Contrast: ", menu_contrast, REFRESH_PERIODIC, 1000);
    Menu_AddItem(&settings_menu, "Back", menu_back_to_main, true);
    
    // Hien thi menu ban dau
    Menu_Display(current_menu);
    uint32_t last_update_time = 0;
    while (1)
		{
			uint32_t current_time = millis();
			MenuKey key = read_key();
        if (key != MENU_KEY_NONE) {  // Neu có phím duoc bam
            Menu_HandleKey(current_menu, key);
        }
				if (current_time - last_update_time >= 100) { // update m?i 100ms
            Menu_Update(current_menu, current_time);
            last_update_time = current_time;
        }
		}
}

void menu_settings(void) {
    current_menu = &settings_menu;
    Menu_Display(current_menu);
}

void menu_about(void) {
    OLED_Fill(0x00, 2);
    OLED_SetCursor(0, 0, 2);
    OLED_WriteString("About");
    OLED_SetCursor(0, 1, 2);
    OLED_WriteString("STM32 Menu");
    OLED_SetCursor(0, 2, 2);
		char ver[3] = "1.0";
    OLED_Printf("Version %s", ver);
		delay_ms(3000);
    Menu_Display(current_menu);  // Quay ve menu
}

void menu_brightness(void) {
    OLED_Fill(0x00, 2);
    OLED_SetCursor(0, 0, 2);
    OLED_WriteString("Brightness");
    OLED_SetCursor(0, 2, 2);
    OLED_WriteString("Adjusting...");
    
    // Code dieu chinh brightness OLED
		delay_ms(3000);
    Menu_Display(current_menu);
}

void menu_contrast(char* buffer, uint8_t buffer_size) {
	static int current_contrast_value = 0;
	current_contrast_value ++;
	snprintf(buffer, buffer_size, "Contrast: %d", current_contrast_value);
}

void menu_back_to_main(void) {
    current_menu = &main_menu;
    Menu_Display(current_menu);
}

MenuKey read_key(void)
{
	if(Button_IsPressed(&button1))
	{
		return MENU_KEY_DOWN;
	}
	if(Button_IsPressed(&button2))
	{
		return MENU_KEY_UP;
	}
	if(Button_IsPressed(&button3))
	{
		return MENU_KEY_SELECT;
	}
	return MENU_KEY_NONE;
}

// Override các callback neu can
void Menu_OnItemSelect(Menu* menu, uint8_t index) {
    // Xu lý dac biet khi chon item
    // Ví du: play sound, log, etc.
}

void Menu_OnBack(Menu* menu) {
    // Xu lý khi bam back
    if (menu == &settings_menu) {
        current_menu = &main_menu;
        Menu_Display(current_menu);
    }
}

