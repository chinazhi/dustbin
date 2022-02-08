#ifndef _DS18B20_H
#define _DS18B20_H

/**
 * @brief DS18B20 BSP init
 * 
 * @return int 0 error 1 success
 */
uint8_t DS18B20_Init(void);
//int ds18b20_init(void);

/**
 * @brief read temp
 * 
 * @return float 
 */
float DS18B20_GetTemp_SkipRom(void);
//float ds18b20_read_temp(void);

#endif


