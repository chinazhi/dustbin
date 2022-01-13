#ifndef _DS18B20_H
#define _DS18B20_H

/**
 * @brief DS18B20 BSP init
 * 
 * @return int 0 error 1 success
 */
int ds18b20_init(void);

/**
 * @brief read temp
 * 
 * @return float 
 */
float ds18b20_read_temp(void);

#endif


