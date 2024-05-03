#include "led_control.h"

uint8_t reply[2];

void try_led_last_time(){
    uint8_t init_KTD2052A[] = {0b11101000};
    uint8_t init_KTD2052C[] = {0b11101010};

    HAL_I2C_Master_Transmit(&hi2c4, init_KTD2052A, init_KTD2052A, 1, 250);
    HAL_I2C_Master_Transmit(&hi2c4, init_KTD2052C, init_KTD2052C, 1, 250);

    HAL_I2C_Master_Transmit(&hi2c4, init_KTD2052A, LED_CONTROL_REGISTER, sizeof(LED_CONTROL_REGISTER), 250);
    HAL_I2C_Master_Transmit(&hi2c4, init_KTD2052C, LED_CONTROL_REGISTER, sizeof(LED_CONTROL_REGISTER), 250);

    HAL_I2C_Master_Transmit(&hi2c4, init_KTD2052A, 0b10011111, 1, 250); //value of the CONTROL_REGISTER
    HAL_I2C_Master_Transmit(&hi2c4, init_KTD2052C, 0b10011111, 1, 250);//value of the CONTROL_REGISTER 



    uint8_t green2[] = {0x07};
    uint8_t green3[] = {0x04};
    uint8_t green_value = {0b11111111};
    HAL_I2C_Master_Transmit(&hi2c4, init_KTD2052A, green2, sizeof(green2), 250);
    HAL_I2C_Master_Transmit(&hi2c4, init_KTD2052A, green_value, sizeof(green_value), 250);
    HAL_I2C_Master_Transmit(&hi2c4, init_KTD2052C, green3, sizeof(green3), 250);
    HAL_I2C_Master_Transmit(&hi2c4, init_KTD2052C, green_value, sizeof(green_value), 250);



 
}

void led_control_init() {
    
    uint8_t init_KTD2052A[] = {0b11101000};
    uint8_t init_KTD2052C[] = {0b11101010};
    uint8_t control[] = {LED_CONTROL_REGISTER, 0b10011111};

    HAL_I2C_Master_Transmit(&hi2c4, init_KTD2052A, 0, 1, 250);
    HAL_I2C_Master_Transmit(&hi2c4, init_KTD2052C, 0, 1, 250);

    HAL_I2C_Master_Transmit(&hi2c4, init_KTD2052A, control, sizeof(control), 250);
    HAL_I2C_Master_Transmit(&hi2c4, init_KTD2052C, control, sizeof(control), 250);
    

    // HAL_I2C_Master_Transmit(&hi2c4, ledaddr1, control, sizeof(control), 250);
    // HAL_I2C_Master_Transmit(&hi2c4, ledaddr2, control, sizeof(control), 250);
}

void led_control_green_test(){
    uint8_t init_KTD2052A[] = {0b11101000};
    uint8_t init_KTD2052C[] = {0b11101010};
    uint8_t green[] = {0x07, 0b11111111};
    HAL_I2C_Master_Transmit(&hi2c4, init_KTD2052A, green, sizeof(green), 250);
}

void led_mem_write(void){
    HAL_I2C_Master_Transmit(&hi2c4, KTD2052A, 0, 1, 250);
    HAL_I2C_Master_Transmit(&hi2c4, KTD2052C, 0, 1, 250);

    uint8_t control_register_value = 0b10011111;
    HAL_I2C_Mem_Write(&hi2c4, KTD2052A, LED_CONTROL_REGISTER, sizeof(LED_CONTROL_REGISTER), control_register_value, sizeof(control_register_value), 100);

    HAL_I2C_Mem_Write(&hi2c4, KTD2052A, 0x07, sizeof(0x07), 0b11111111, sizeof(1), 100);
}

void led_control_read_value(void){
    uint8_t init_KTD2052A[] = {0b11101000};
    uint8_t init_KTD2052C[] = {0b11101010};
    uint8_t control[] = {LED_CONTROL_REGISTER, 0b10011111};

    

    HAL_I2C_Master_Transmit(&hi2c4, init_KTD2052A, LED_CONTROL_REGISTER, 1, 250);
    HAL_I2C_Master_Transmit(&hi2c4, init_KTD2052C, LED_CONTROL_REGISTER, 1, 250);

    HAL_I2C_Master_Transmit(&hi2c4, 0b11101001, 0, 1, 250);
    HAL_I2C_Master_Transmit(&hi2c4, 0b11101011, 0, 1, 250);   

    HAL_I2C_Master_Receive(&hi2c4, init_KTD2052A, reply, sizeof(reply), 250);

    char snprintf_buffer[64u] = {0};

    snprintf(snprintf_buffer, 64u, "%d %d", reply[0], reply[1]);

    update_sensors_extra_value(snprintf_buffer, 2);


}

void led_control_init_reset(){
    uint8_t init_KTD2052A[] = {0b11101000};
    uint8_t init_KTD2052C[] = {0b11101010};
    uint8_t control[] = {LED_CONTROL_REGISTER, 0b11011000};

    HAL_I2C_Master_Transmit(&hi2c4, init_KTD2052A, 0, 1, 250);
    HAL_I2C_Master_Transmit(&hi2c4, init_KTD2052C, 0, 1, 250);


    //if(!HAL_I2C_Slave_Receive(&hi2c4, ledaddr1, control, sizeof(control), 250);

    //HAL_I2C_Master_Transmit(&hi2c4, 0x04, 11111111, 1, 250);
    //HAL_I2C_Master_Transmit(&hi2c4, ledaddr2, control, sizeof(control), 250); 
}

void led_control_set_all(I2C_HandleTypeDef *hi2c4, uint32_t color) {
    led_control_set(hi2c4, (uint32_t[6]){color, color, color, color, color, color});
}

void led_control_set(I2C_HandleTypeDef *hi2c4, uint32_t colors[LED_N]) {
    // left leds
    for (int icolor = 0; icolor < LED_N / 2; ++icolor) {
        HAL_I2C_Master_Transmit(hi2c4, ledaddr1, (uint8_t[2]){RED + icolor * 3, (uint8_t)(colors[icolor] >> 16)}, 2, 100);
        HAL_I2C_Master_Transmit(hi2c4, ledaddr1, (uint8_t[2]){GREEN + icolor * 3, (uint8_t)(colors[icolor] >> 8)}, 2, 100);
        HAL_I2C_Master_Transmit(hi2c4, ledaddr1, (uint8_t[2]){BLUE + icolor * 3, (uint8_t)colors[icolor]}, 2, 100);
    }

    // right leds
    for (int icolor = 0; icolor < LED_N / 2; ++icolor) {
        HAL_I2C_Master_Transmit(hi2c4, ledaddr2, (uint8_t[2]){RED + icolor * 3, (uint8_t)(colors[icolor + 3] >> 16)}, 2, 100);
        HAL_I2C_Master_Transmit(hi2c4, ledaddr2, (uint8_t[2]){GREEN + icolor * 3, (uint8_t)(colors[icolor + 3] >> 8)}, 2, 100);
        HAL_I2C_Master_Transmit(hi2c4, ledaddr2, (uint8_t[2]){BLUE + icolor * 3, (uint8_t)colors[icolor + 3]}, 2, 100);
    }
}

void all_leds_green() {
    led_control_init(&hi2c4);
    led_control_set(&hi2c4, (uint32_t[6]){COLOR_OFF, COLOR_OFF, COLOR_OFF, COLOR_GREEN, COLOR_GREEN, COLOR_GREEN});
}

void all_leds_red() {
    led_control_init(&hi2c4);
    led_control_set(&hi2c4, (uint32_t[6]){COLOR_OFF, COLOR_OFF, COLOR_OFF, COLOR_RED, COLOR_RED, COLOR_RED});
}
