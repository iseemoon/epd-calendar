#include "sdkconfig.h"
#include "rtc.h"
#include <stdio.h>


#define  tmYearToY2k(Y)       ((Y) - 30)    // offset is from 2000
#define  y2kYearToTm(Y)       ((Y) + 30)

#define RX8025T_I2C_ADDRESS    0x32

#define I2C_SLAVE_SCL_IO      CONFIG_I2C_SLAVE_SCL            /*!< gpio number for i2c slave clock */
#define I2C_SLAVE_SDA_IO      CONFIG_I2C_SLAVE_SDA            /*!< gpio number for i2c slave data */
#define I2C_SLAVE_NUM         I2C_NUMBER(CONFIG_I2C_SLAVE_PORT_NUM)   /*!< I2C port number for slave dev */
#define I2C_SLAVE_TX_BUF_LEN (2 * DATA_LENGTH)                /*!< I2C slave tx buffer size */
#define I2C_SLAVE_RX_BUF_LEN (2 * DATA_LENGTH)                /*!< I2C slave rx buffer size */

#define I2C_MASTER_SCL_IO     16                              /*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO     4                               /*!< gpio number for I2C master data  */
#define I2C_MASTER_NUM        I2C_NUMBER(0)                   /*!< I2C port number for master dev */
#define I2C_MASTER_FREQ_HZ    400000L                         /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE 0                           /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0                           /*!< I2C master doesn't need buffer */

#define ESP_SLAVE_ADDR        RX8025T_I2C_ADDRESS     /*!< ESP32 slave address, you can set any 7bit value */
#define WRITE_BIT             I2C_MASTER_WRITE                /*!< I2C master write */
#define READ_BIT              I2C_MASTER_READ                 /*!< I2C master read */
#define ACK_CHECK_EN          0x1                             /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS         0x0                             /*!< I2C master will not check ack from slave */
#define ACK_VAL               I2C_MASTER_ACK                  /*!< I2C ack value */
#define NACK_VAL              I2C_MASTER_NACK                 /*!< I2C nack value */



#define TIME_REGISTER_SIZE    7


// Convert Decimal to Binary Coded Decimal (BCD)
static uint8_t dec2bcd(uint8_t dec)
{
  return ((dec/10) << 4) + (dec % 10);
}

// Convert Binary Coded Decimal (BCD) to Decimal
static uint8_t bcd2dec(uint8_t bcd)
{
  return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

/**
 * @brief i2c master initialization
 */
static esp_err_t i2c_init(i2c_port_t i2c_port, gpio_num_t sda_io_num, gpio_num_t scl_io_num)
{
    i2c_config_t conf;
    conf.mode             = I2C_MODE_MASTER;
    conf.sda_io_num       = sda_io_num;
    conf.sda_pullup_en    = GPIO_PULLUP_ENABLE;
    conf.scl_io_num       = scl_io_num;
    conf.scl_pullup_en    = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
    i2c_param_config(i2c_port, &conf);
    return i2c_driver_install(i2c_port, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
}

static esp_err_t i2c_read_byte(i2c_port_t i2c_port, uint8_t addr, uint8_t * data)
{
    esp_err_t ret;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (ESP_SLAVE_ADDR << 1), ACK_CHECK_EN);
    i2c_master_write_byte(cmd, (8 + addr), ACK_CHECK_EN);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(i2c_port, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    if (ret != ESP_OK)
        return ret;

    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd,  (ESP_SLAVE_ADDR << 1) | READ_BIT, ACK_CHECK_EN);
    i2c_master_read_byte(cmd, data, ACK_VAL);
    ret = i2c_master_cmd_begin(i2c_port, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

static esp_err_t i2c_write_byte(i2c_port_t i2c_port, uint8_t addr, uint8_t data)
{
    esp_err_t ret;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (ESP_SLAVE_ADDR << 1), ACK_CHECK_EN);
    i2c_master_write_byte(cmd, addr, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, data, ACK_CHECK_EN);
    ret = i2c_master_cmd_begin(i2c_port, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

/**
 * @brief test code to read esp-i2c-slave
 *        We need to fill the buffer of esp slave device, then master can read them out.
 * _______________________________________________________________________________________
 * | start | slave_addr + rd_bit +ack | read n-1 bytes + ack | read 1 byte + nack | stop |
 * --------|--------------------------|----------------------|--------------------|------|
 */
static esp_err_t i2c_read_multi(i2c_port_t i2c_port, uint8_t *data_rd, size_t size)
{
    esp_err_t ret = ESP_OK;
    if (size > 0)
    {
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (ESP_SLAVE_ADDR << 1), ACK_CHECK_EN);
        i2c_master_write_byte(cmd, 0, ACK_CHECK_EN);
        i2c_master_stop(cmd);
        ret = i2c_master_cmd_begin(i2c_port, cmd, 1000 / portTICK_RATE_MS);
        i2c_cmd_link_delete(cmd);

        cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (ESP_SLAVE_ADDR << 1) | READ_BIT, ACK_CHECK_EN);
        if (size > 1)
        {
            i2c_master_read(cmd, data_rd, size - 1, ACK_VAL);
        }
        i2c_master_read_byte(cmd, data_rd + size - 1, NACK_VAL);
        i2c_master_stop(cmd);
        ret = i2c_master_cmd_begin(i2c_port, cmd, 1000 / portTICK_RATE_MS);
        i2c_cmd_link_delete(cmd);
    }

    return ret;
}

/**
 * @brief Test code to write esp-i2c-slave
 *        Master device write data to slave(both esp32),
 *        the data will be stored in slave buffer.
 *        We can read them out from slave buffer.
 * ___________________________________________________________________
 * | start | slave_addr + wr_bit + ack | write n bytes + ack  | stop |
 * --------|---------------------------|----------------------|------|*/
static esp_err_t i2c_write_multi(i2c_port_t i2c_num, uint8_t *data_wr, size_t size)
{
    esp_err_t ret = ESP_OK;
    if (size > 0)
    {
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (ESP_SLAVE_ADDR << 1), ACK_CHECK_EN);
        i2c_master_write(cmd, data_wr, size, ACK_CHECK_EN);
        i2c_master_stop(cmd);
        ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_RATE_MS);
        i2c_cmd_link_delete(cmd);
    }

    return ret;
}

RTC::RTC()
{
    i2c_port = I2C_NUMBER(0);
}

bool RTC::Init(i2c_port_t _i2c_port, gpio_num_t sda_io_num, gpio_num_t scl_io_num)
{
    i2c_port = _i2c_port;
    if (i2c_init(i2c_port, sda_io_num, scl_io_num) == ESP_OK)
    {
        uint8_t data = 0xFF;
        if (i2c_read_byte(i2c_port, 0x0E, &data) == ESP_OK)
        {
            if (data & 1)
            {
                Adjust(946684800);
                return Adjust(946684800);
            }
            return true;
        }
    }
    return false;
}


bool RTC::Read(DateTime * pDatetime)
{
    uint8_t data_rd[TIME_REGISTER_SIZE] = { 0 };
    if (pDatetime && i2c_read_multi(i2c_port, data_rd, sizeof(data_rd)) == ESP_OK)
    {
        uint8_t sec   = bcd2dec(data_rd[0] & 0x7F);
        uint8_t min   = bcd2dec(data_rd[1]);
        uint8_t hour  = bcd2dec(data_rd[2] & 0x3f);
        uint8_t day   = bcd2dec(data_rd[4]);
        uint8_t month = bcd2dec(data_rd[5]);
        uint8_t year  = bcd2dec(data_rd[6]);

        *pDatetime = DateTime(year + 2000, month, day, hour, min, sec);

        return true;
    }

    return false;
}

bool RTC::Read(time_t * pTime)
{
    DateTime datetime;
    if (pTime && Read(&datetime))
    {
        *pTime = datetime.unixtime();
        return true;
    }
    return false;
}

bool RTC::Adjust(DateTime * pDatetime)
{
    return i2c_write_byte(i2c_port, 0, dec2bcd(pDatetime->second())) == ESP_OK
        && i2c_write_byte(i2c_port, 1, dec2bcd(pDatetime->minute())) == ESP_OK
        && i2c_write_byte(i2c_port, 2, dec2bcd(pDatetime->hour())) == ESP_OK
        && i2c_write_byte(i2c_port, 3, 1 << pDatetime->dayOfWeek()) == ESP_OK
        && i2c_write_byte(i2c_port, 4, dec2bcd(pDatetime->day())) == ESP_OK
        && i2c_write_byte(i2c_port, 5, dec2bcd(pDatetime->month())) == ESP_OK
        && i2c_write_byte(i2c_port, 6, dec2bcd(pDatetime->year()-2000)) == ESP_OK;
}

bool RTC::Adjust(time_t when)
{
    DateTime datetime((uint32_t)when);
    return Adjust(&datetime);
}

bool RTC::IsRunning()
{
    uint8_t data = 0xFF;
    return (i2c_read_byte(i2c_port, 0, &data) == ESP_OK);
}

/*
void RTC_DS1307::setCalibration(char calValue)
{
  unsigned char calReg = abs(calValue) & 0x1f;
  if (calValue >= 0) calReg |= 0x20; // S bit is positive to speed up the clock

  beginTransmission(DS1307_CTRL_ID);

  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0x07, ACK_CHECK_EN)); // Point to calibration register
  ESP_ERROR_CHECK(i2c_master_write_byte(cmd, calReg, ACK_CHECK_EN));

  endTransmission(DS1307_CTRL_ID, cmd);
}

char RTC_DS1307::getCalibration()
{
  beginTransmission(DS1307_CTRL_ID);

  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0x07, ACK_CHECK_EN)); // Point to calibration register

  endTransmission(DS1307_CTRL_ID, cmd);

  uint8_t data_rd[1];
  ESP_ERROR_CHECK(i2c_master_read_slave(I2C_MASTER_NUM, data_rd, 1));

  unsigned char calReg = data_rd[0];
  char out = calReg & 0x1f;
  if (!(calReg & 0x20)) out = -out; // S bit clear means a negative value
  return out;
}

bool RTC_DS1307::exists = false;

RTC_DS1307 RTC = RTC_DS1307(); // create an instance for the user

static esp_err_t beginTransmission(uint16_t address)
{
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (address << 1) | READ_BIT, ACK_CHECK_EN);
  i2c_master_write_byte(cmd, RTC_DS1307_CMD_START, ACK_CHECK_EN);
  i2c_master_stop(cmd);
  esp_err_t ret;
  ret = i2c_master_cmd_begin((i2c_port_t)I2C_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
  i2c_cmd_link_delete(cmd);
  return ret;
}

static esp_err_t endTransmission(i2c_port_t port, i2c_cmd_handle_t cmd)
{
  esp_err_t ret = i2c_master_cmd_begin(port, cmd, 1000 / portTICK_RATE_MS);
  i2c_cmd_link_delete(cmd);
  return ret;
}*/

