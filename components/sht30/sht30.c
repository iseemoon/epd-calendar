/*
* @file         sht30.c 
* @brief        ESP32操作SHT30-I2C
* @details      用户应用程序的入口文件,用户所有要实现的功能逻辑均是从该文件开始或者处理
* @author       hx-zsj 
* @par Copyright (c):  
*               红旭无线开发团队，QQ群：671139854
* @par History:          
*               Ver0.0.1:
                     hx-zsj, 2018/08/06, 初始化版本\n 
*/

/* 
=============
头文件包含
=============
*/
#include <stdio.h>
#include "string.h"
#include "esp_system.h"
// #include "esp_event.h"
#include "esp_log.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "sht30.h"


#define TAG   "SHT30"

//I2C 
#define I2C_SCL_IO          GPIO_NUM_22         //SCL->IO22
#define I2C_SDA_IO          GPIO_NUM_21         //SDA->IO21
#define I2C_MASTER_NUM      I2C_NUM_1           //I2C_1
#define WRITE_BIT           I2C_MASTER_WRITE    //写:0
#define READ_BIT            I2C_MASTER_READ     //读:1
#define ACK_CHECK_EN        0x1                 //主机检查从机的ACK
#define ACK_CHECK_DIS       0x0                 //主机不检查从机的ACK
#define ACK_VAL             0x0                 //应答
#define NACK_VAL            0x1                 //不应答

//SHT30
#define SHT30_WRITE_ADDR    0x44                //I2C地址，ADDR管脚接低电平时为0x44,接高电平为0x45
#define CMD_FETCH_DATA_H    0x2c                //循环采样，参考sht30 datasheet
#define CMD_FETCH_DATA_L    0x06

/*
* IIC初始化
* @param[in]   void  		       :无
* @retval      void                :无
*/
bool i2c_init(void)
{
    esp_err_t err = ESP_OK;
	//i2c配置结构体
    i2c_config_t conf={
        .mode = I2C_MODE_MASTER,                    //I2C模式
        .sda_io_num = I2C_SDA_IO,                   //SDA IO映射
        .scl_io_num = I2C_SCL_IO,                   //SCL IO映射
        .sda_pullup_en = GPIO_PULLUP_ENABLE,        //SDA IO模式
        .scl_pullup_en = GPIO_PULLUP_ENABLE,        //SCL IO模式
        .master.clk_speed = 400000,                 //I2C CLK频率
	};

    if (i2c_param_config(I2C_MASTER_NUM, &conf) == ESP_OK)
    {
        return i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0) == ESP_OK;
    }

    return false;
}

/*
* sht30初始化
* @param[in]   void  		        :无
* @retval      bool                  :0成功，其他失败
*/
bool sht30_init(void)
{
    if (i2c_init())
    {
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();                                   //新建操作I2C句柄
        i2c_master_start(cmd);                                                          //启动I2C
        i2c_master_write_byte(cmd, SHT30_WRITE_ADDR << 1 | WRITE_BIT, ACK_CHECK_EN);    //发地址+写+检查ack
        i2c_master_write_byte(cmd, CMD_FETCH_DATA_H, ACK_CHECK_EN);                     //发数据高8位+检查ack
        i2c_master_write_byte(cmd, CMD_FETCH_DATA_L, ACK_CHECK_EN);                     //发数据低8位+检查ack
        i2c_master_stop(cmd);                                                           //停止I2C
        esp_err_t err = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(100));  //I2C发送
        i2c_cmd_link_delete(cmd);                                                       //删除I2C句柄

        if (err == ESP_OK)
        {
            return true;
        }

        i2c_driver_delete(I2C_MASTER_NUM);
        ESP_LOGE(TAG, "i2c_master_cmd_begin error: %d", err);
    }
    else
    {
        ESP_LOGE(TAG, "I2C initialize error");
    }

    return false;
}

/*
* sht30校验算法
* @param[in]   pdata  		        :需要校验的数据
* @param[in]   nbrOfBytes  		    :需要校验的数据长度
* @retval      int                  :校验值
* @note        修改日志 
*               Ver0.0.1:
                    hx-zsj, 2018/08/06, 初始化版本\n 
*/
unsigned char SHT3X_CalcCrc(unsigned char *data, unsigned char nbrOfBytes)
{
	unsigned char bit;        // bit mask
    unsigned char crc = 0xFF; // calculated checksum
    unsigned char byteCtr;    // byte counter
    unsigned int POLYNOMIAL =  0x131;           // P(x) = x^8 + x^5 + x^4 + 1 = 100110001

    // calculates 8-Bit checksum with given polynomial
    for(byteCtr = 0; byteCtr < nbrOfBytes; byteCtr++) {
        crc ^= (data[byteCtr]);
        for(bit = 8; bit > 0; --bit) {
            if(crc & 0x80) {
                crc = (crc << 1) ^ POLYNOMIAL;
            }  else {
                crc = (crc << 1);
            }
        }
    }
	return crc;
}
/*
* sht30数据校验
* @param[in]   pdata  		        :需要校验的数据
* @param[in]   nbrOfBytes  		    :需要校验的数据长度
* @param[in]   checksum  		    :校验的结果
* @retval      int                  :0成功，其他失败
*/
bool SHT3X_CheckCrc(unsigned char *pdata, unsigned char nbrOfBytes, unsigned char checksum)
{
	unsigned char crc = SHT3X_CalcCrc(pdata, nbrOfBytes);// calculates 8-Bit checksum
    return (crc == checksum);
}
/*
* 获取sht30温湿度
* @param[in]   void  		       :无
* @retval      void                :无
* @note        修改日志 
*               Ver0.0.1:
                    hx-zsj, 2018/08/06, 初始化版本\n 
*/
bool sht30_get_value(uint8_t * temp, uint8_t * humi)
{
    unsigned char sht30_buf[6]={0}; 
    uint32_t data;
    esp_err_t err;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();                                   //新建操作I2C句柄
    i2c_master_start(cmd);                                                          //启动I2C
    i2c_master_write_byte(cmd, SHT30_WRITE_ADDR << 1 | READ_BIT, ACK_CHECK_EN);     //发地址+读+检查ack
    i2c_master_read_byte(cmd, &sht30_buf[0], ACK_VAL);                              //读取数据+回复ack
    i2c_master_read_byte(cmd, &sht30_buf[1], ACK_VAL);                              //读取数据+回复ack
    i2c_master_read_byte(cmd, &sht30_buf[2], ACK_VAL);                              //读取数据+回复ack
    i2c_master_read_byte(cmd, &sht30_buf[3], ACK_VAL);                              //读取数据+回复ack
    i2c_master_read_byte(cmd, &sht30_buf[4], ACK_VAL);                              //读取数据+回复ack
    i2c_master_read_byte(cmd, &sht30_buf[5], NACK_VAL);                             //读取数据+不回复ack
    i2c_master_stop(cmd);                                                           //停止I2C
    err = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(100));            //I2C发送
    i2c_cmd_link_delete(cmd);                                                       //删除I2C句柄
    if(err == ESP_OK)
    {
        //校验读出来的数据，算法参考sht30 datasheet
        if( (SHT3X_CheckCrc(sht30_buf,2,sht30_buf[2])) && (SHT3X_CheckCrc(sht30_buf+3, 2, sht30_buf[5])) )
        {
            data = (sht30_buf[0]<<8 | sht30_buf[1]);
            *temp = (uint8_t) ( ((float)data *175) / 65535 - 50 );
            *humi = (uint8_t)( (sht30_buf[3] * 256 + sht30_buf[4]) * 100 / 65535.0) ;
        }
        else
        {
            err = ESP_ERR_INVALID_CRC;
            ESP_LOGE(TAG, "data crc check error");
        }
    }
    else
    {
        ESP_LOGE(TAG, "i2c read data error: %d", err);
    }

    i2c_driver_delete(I2C_MASTER_NUM);

    return err == ESP_OK;
}
    