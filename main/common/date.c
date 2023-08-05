/**
 * @file date.c
 *
 */
/*********************
 *      INCLUDES
 *********************/
#include "date.h"
#include <stdio.h>
#include <string.h>

/**********************
 *      TYPEDEFS
 **********************/

#define  MIN_YEAR  1900
#define  MAX_YEAR  2099

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/


/**
 * 用来表示1900年到2099年间农历年份的相关信息，共24位bit的16进制表示，其中：
 * 1. 前4位表示该年闰哪个月；
 * 2. 5-17位表示农历年份13个月的大小月分布，0表示小，1表示大；
 * 3. 最后7位表示农历年首（正月初一）对应的公历日期。
 *
 * 以2014年的数据0x955ABF为例说明：
 * 1001 0101 0101 1010 1011 1111
 * 闰九月  农历正月初一对应公历1月31号
 */
// const static uint32_t LUNAR_INFO[] = {
//     0x84B6BF, 0x04AE53, 0x0A5748, 0x5526BD, 0x0D2650, 0x0D9544, 0x46AAB9, 0x056A4D, 0x09AD42, 0x24AEB6, /*1900-1909*/
//     0x04AE4A, 0x6A4DBE, 0x0A4D52, 0x0D2546, 0x5D52BA, 0x0B544E, 0x0D6A43, 0x296D37, 0x095B4B, 0x749BC1, /*1910-1919*/
//     0x049754, 0x0A4B48, 0x5B25BC, 0x06A550, 0x06D445, 0x4ADAB8, 0x02B64D, 0x095742, 0x2497B7, 0x04974A, /*1920-1929*/
//     0x664B3E, 0x0D4A51, 0x0EA546, 0x56D4BA, 0x05AD4E, 0x02B644, 0x393738, 0x092E4B, 0x7C96BF, 0x0C9553, /*1930-1939*/
//     0x0D4A48, 0x6DA53B, 0x0B554F, 0x056A45, 0x4AADB9, 0x025D4D, 0x092D42, 0x2C95B6, 0x0A954A, 0x7B4ABD, /*1940-1949*/
//     0x06CA51, 0x0B5546, 0x555ABB, 0x04DA4E, 0x0A5B43, 0x352BB8, 0x052B4C, 0x8A953F, 0x0E9552, 0x06AA48, /*1950-1959*/
//     0x6AD53C, 0x0AB54F, 0x04B645, 0x4A5739, 0x0A574D, 0x052642, 0x3E9335, 0x0D9549, 0x75AABE, 0x056A51, /*1960-1969*/
//     0x096D46, 0x54AEBB, 0x04AD4F, 0x0A4D43, 0x4D26B7, 0x0D254B, 0x8D52BF, 0x0B5452, 0x0B6A47, 0x696D3C, /*1970-1979*/
//     0x095B50, 0x049B45, 0x4A4BB9, 0x0A4B4D, 0xAB25C2, 0x06A554, 0x06D449, 0x6ADA3D, 0x0AB651, 0x095746, /*1980-1989*/
//     0x5497BB, 0x04974F, 0x064B44, 0x36A537, 0x0EA54A, 0x86B2BF, 0x05AC53, 0x0AB647, 0x5936BC, 0x092E50, /*1990-1999*/
//     0x0C9645, 0x4D4AB8, 0x0D4A4C, 0x0DA541, 0x25AAB6, 0x056A49, 0x7AADBD, 0x025D52, 0x092D47, 0x5C95BA, /*2000-2009*/
//     0x0A954E, 0x0B4A43, 0x4B5537, 0x0AD54A, 0x955ABF, 0x04BA53, 0x0A5B48, 0x652BBC, 0x052B50, 0x0A9345, /*2010-2019*/
//     0x474AB9, 0x06AA4C, 0x0AD541, 0x24DAB6, 0x04B64A, 0x6a573D, 0x0A4E51, 0x0D2646, 0x5E933A, 0x0D534D, /*2020-2029*/
//     0x05AA43, 0x36B537, 0x096D4B, 0xB4AEBF, 0x04AD53, 0x0A4D48, 0x6D25BC, 0x0D254F, 0x0D5244, 0x5DAA38, /*2030-2039*/
//     0x0B5A4C, 0x056D41, 0x24ADB6, 0x049B4A, 0x7A4BBE, 0x0A4B51, 0x0AA546, 0x5B52BA, 0x06D24E, 0x0ADA42, /*2040-2049*/
//     0x355B37, 0x09374B, 0x8497C1, 0x049753, 0x064B48, 0x66A53C, 0x0EA54F, 0x06AA44, 0x4AB638, 0x0AAE4C, /*2050-2059*/
//     0x092E42, 0x3C9735, 0x0C9649, 0x7D4ABD, 0x0D4A51, 0x0DA545, 0x55AABA, 0x056A4E, 0x0A6D43, 0x452EB7, /*2060-2069*/
//     0x052D4B, 0x8A95BF, 0x0A9553, 0x0B4A47, 0x6B553B, 0x0AD54F, 0x055A45, 0x4A5D38, 0x0A5B4C, 0x052B42, /*2070-2079*/
//     0x3A93B6, 0x069349, 0x7729BD, 0x06AA51, 0x0AD546, 0x54DABA, 0x04B64E, 0x0A5743, 0x452738, 0x0D264A, /*2080-2089*/
//     0x8E933E, 0x0D5252, 0x0DAA47, 0x66B53B, 0x056D4F, 0x04AE45, 0x4A4EB9, 0x0A4D4C, 0x0D1541, 0x2D92B5, /*2090-2099*/
// };

const static uint32_t  solar_1_1[] = {
       1887,                                                                         0xec04c,  0xec23f, /*1888-1889*/
    0xec435,   0xec649,  0xec83e,  0xeca51,  0xecc46,  0xece3a,  0xed04d,  0xed242,  0xed436,  0xed64a, /*1890-1899*/
    0xed83f,   0xeda53,  0xedc48,  0xede3d,  0xee050,  0xee244,  0xee439,  0xee64d,  0xee842,  0xeea36, /*1900-1909*/
    0xeec4a,   0xeee3e,  0xef052,  0xef246,  0xef43a,  0xef64e,  0xef843,  0xefa37,  0xefc4b,  0xefe41,
    0xf0054,   0xf0248,  0xf043c,  0xf0650,  0xf0845,  0xf0a38,  0xf0c4d,  0xf0e42,  0xf1037,  0xf124a,
    0xf143e,   0xf1651,  0xf1846,  0xf1a3a,  0xf1c4e,  0xf1e44,  0xf2038,  0xf224b,  0xf243f,  0xf2653,
    0xf2848,   0xf2a3b,  0xf2c4f,  0xf2e45,  0xf3039,  0xf324d,  0xf3442,  0xf3636,  0xf384a,  0xf3a3d,
    0xf3c51,   0xf3e46,  0xf403b,  0xf424e,  0xf4443,  0xf4638,  0xf484c,  0xf4a3f,  0xf4c52,  0xf4e48,
    0xf503c,   0xf524f,  0xf5445,  0xf5639,  0xf584d,  0xf5a42,  0xf5c35,  0xf5e49,  0xf603e,  0xf6251,
    0xf6446,   0xf663b,  0xf684f,  0xf6a43,  0xf6c37,  0xf6e4b,  0xf703f,  0xf7252,  0xf7447,  0xf763c,
    0xf7850,   0xf7a45,  0xf7c39,  0xf7e4d,  0xf8042,  0xf8254,  0xf8449,  0xf863d,  0xf8851,  0xf8a46,
    0xf8c3b,   0xf8e4f,  0xf9044,  0xf9237,  0xf944a,  0xf963f,  0xf9853,  0xf9a47,  0xf9c3c,  0xf9e50,
    0xfa045,   0xfa238,  0xfa44c,  0xfa641,  0xfa836,  0xfaa49,  0xfac3d,  0xfae52,  0xfb047,  0xfb23a, /*2000-2009*/
    0xfb44e,   0xfb643,  0xfb837,  0xfba4a,  0xfbc3f,  0xfbe53,  0xfc048,  0xfc23c,  0xfc450,  0xfc645, /*2010-2019*/
    0xfc839,   0xfca4c,  0xfcc41,  0xfce36,  0xfd04a,  0xfd23d,  0xfd451,  0xfd646,  0xfd83a,  0xfda4d, /*2020-2029*/
    0xfdc43,   0xfde37,  0xfe04b,  0xfe23f,  0xfe453,  0xfe648,  0xfe83c,  0xfea4f,  0xfec44,  0xfee38, /*2030-2039*/
    0xff04c,   0xff241,  0xff436,  0xff64a,  0xff83e,  0xffa51,  0xffc46,  0xffe3a, 0x10004e, 0x100242, /*2040-2049*/
    0x100437, 0x10064b, 0x100841, 0x100a53, 0x100c48, 0x100e3c, 0x10104f, 0x101244, 0x101438, 0x10164c, /*2050-2059*/
    0x101842, 0x101a35, 0x101c49, 0x101e3d, 0x102051, 0x102245, 0x10243a, 0x10264e, 0x102843, 0x102a37, /*2060-2069*/
    0x102c4b, 0x102e3f, 0x103053, 0x103247, 0x10343b, 0x10364f, 0x103845, 0x103a38, 0x103c4c, 0x103e42, /*2070-2079*/
    0x104036, 0x104249, 0x10443d, 0x104651, 0x104846, 0x104a3a, 0x104c4e, 0x104e43, 0x105038, 0x10524a, /*2080-2089*/
    0x10543e, 0x105652, 0x105847, 0x105a3b, 0x105c4f, 0x105e45, 0x106039, 0x10624c, 0x106441, 0x106635, /*2090-2099*/
    0x106849, 0x106a3d, 0x106c51, 0x106e47, 0x10703c, 0x10724f, 0x107444, 0x107638, 0x10784c, 0x107a3f, /*2100-2109*/
    0x107c53, 0x107e48,                                                                                 /*2110-2111*/
};

const static uint32_t lunar_month_days[] = {
     1887,                                                          0x1694, 0x16aa,     /*1888-1889*/
    0x4ad5, 0xab6,  0xc4b7, 0x4ae,  0xa56,  0xb52a, 0x1d2a, 0xd54,  0x75aa, 0x156a,     /*1890-1899*/
    0x1096d,0x95c,  0x14ae, 0xaa4d, 0x1a4c, 0x1b2a, 0x8d55, 0xad4,  0x135a, 0x495d,     /*1900-1909*/
    0x95c,  0xd49b, 0x149a, 0x1a4a, 0xbaa5, 0x16a8, 0x1ad4, 0x52da, 0x12b6, 0xe937,     /*1910-1919*/
    0x92e,  0x1496, 0xb64b, 0xd4a,  0xda8,  0x95b5, 0x56c,  0x12ae, 0x492f, 0x92e,      /*1920-1929*/
    0xcc96, 0x1a94, 0x1d4a, 0xada9, 0xb5a,  0x56c,  0x726e, 0x125c, 0xf92d, 0x192a,     /*1930-1939*/
    0x1a94, 0xdb4a, 0x16aa, 0xad4,  0x955b, 0x4ba,  0x125a, 0x592b, 0x152a, 0xf695,     /*1940-1949*/
    0xd94,  0x16aa, 0xaab5, 0x9b4,  0x14b6, 0x6a57, 0xa56,  0x1152a,0x1d2a, 0xd54,      /*1950-1959*/
    0xd5aa, 0x156a, 0x96c,  0x94ae, 0x14ae, 0xa4c,  0x7d26, 0x1b2a, 0xeb55, 0xad4,      /*1960-1969*/
    0x12da, 0xa95d, 0x95a,  0x149a, 0x9a4d, 0x1a4a, 0x11aa5,0x16a8, 0x16d4, 0xd2da,     /*1970-1979*/
    0x12b6, 0x936,  0x9497, 0x1496, 0x1564b, 0xd4a, 0xda8,  0xd5b4, 0x156c, 0x12ae,     /*1980-1989*/
    0xa92f, 0x92e,  0xc96,  0x6d4a, 0x1d4a, 0x10d65, 0xb58, 0x156c, 0xb26d, 0x125c,     /*1990-1999*/
    0x192c, 0x9a95, 0x1a94, 0x1b4a, 0x4b55, 0xad4,  0xf55b, 0x4ba,  0x125a, 0xb92b,     /*2000-2009*/
    0x152a, 0x1694, 0x96aa, 0x15aa, 0x12ab5, 0x974, 0x14b6, 0xca57, 0xa56,  0x1526,     /*2010-2019*/
    0x8e95, 0xd54,  0x15aa, 0x49b5,  0x96c, 0xd4ae, 0x149c, 0x1a4c, 0xbd26, 0x1aa6,     /*2020-2029*/
    0xb54,  0x6d6a, 0x12da, 0x1695d, 0x95a, 0x149a, 0xda4b, 0x1a4a, 0x1aa4, 0xbb54,     /*2030-2039*/
    0x16b4, 0xada,  0x495b, 0x936,  0xf497, 0x1496, 0x154a, 0xb6a5, 0xda4,  0x15b4,     /*2040-2049*/
    0x6ab6, 0x126e, 0x1092f, 0x92e, 0xc96, 0xcd4a,  0x1d4a, 0xd64,  0x956c, 0x155c,     /*2050-2059*/
    0x125c, 0x792e, 0x192c, 0xfa95, 0x1a94, 0x1b4a, 0xab55, 0xad4,  0x14da, 0x8a5d,     /*2060-2069*/
    0xa5a,  0x1152b, 0x152a, 0x1694, 0xd6aa, 0x15aa, 0xab4, 0x94ba, 0x14b6, 0xa56,      /*2070-2079*/
    0x7527, 0xd26,  0xee53, 0xd54,  0x15aa, 0xa9b5, 0x96c,  0x14ae, 0x8a4e, 0x1a4c,     /*2080-2089*/
    0x11d26,0x1aa4, 0x1b54, 0xcd6a, 0xada,   0x95c, 0x949d, 0x149a, 0x1a2a, 0x5b25,     /*2090-2099*/
    0x1aa4, 0xfb52, 0x16b4, 0xaba,  0xa95b, 0x936,  0x1496, 0x9a4b, 0x154a, 0x136a5,    /*2100-2109*/
    0xda4,  0x15ac,                                                                     /*2110-2111*/
};

static const uint8_t DAY_OF_MONTH[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
static const uint32_t DAY_OF_YEAR[] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };


// 农历月的名称
const char * MONTH_CN[]       = { "月", "正", "二", "三", "四", "五", "六", "七", "八", "九", "十", "冬", "腊" };
// const char * DAY_TENFOLD[]    = { "十", "初" ,"二", "三" };
const char * DAY_TENS_DIGIT[] = { "初" ,"十", "廿", "卅" };
const char * DAY_UNIT_DIGIT[] = { "日", "一", "二", "三", "四", "五", "六", "七", "八", "九" };

/*
每年的正小寒点到各节期正节期点（即十五度倍数点）的分钟数。   
地球公转每年都一样。由于公转轨道是椭圆，故这个数列并不是准确的等差数列
*/
const uint32_t SOLAR_TERM_OFFSET[] = { // 各节气与小寒偏移的分钟数
	     0,  21208,  42467,  63836,  85337, 107014, 128867, 150921,
    173149, 195551, 218072, 240693, 263343, 285989, 308563, 331033,
    353350, 375494, 397447, 419210, 440795, 462224, 483532, 504758,
};

// 天干
const char * TIANGAN[]= { "甲", "乙", "丙", "丁", "戊", "己", "庚", "辛", "壬", "癸", };

// 地支
const char * DIZHI[] = { "子", "丑", "寅", "卯", "辰", "巳", "午", "未", "申", "酉", "戌", "亥", };
// 生肖
const char * ANIMALS[] = { "鼠", "牛", "虎", "兔", "龙", "蛇", "马", "羊", "猴", "鸡", "狗", "猪" };


// month为月份，取值范围: 1~12;
// date_type表示日期类型，0: 公历日期, 1: 农历日期, 3: 星期, 4: 完整星期
//  对于公历和农历日期，day为日期，取值范围: 0~31，0表示当月的最后一天;
//  对于星期类型的日期，day的个位数表示星期几，0:星期日,1:星期一,以此类推;
//    day的十位数表示当月的第几周，1~4表示正数，5～8表示倒数。如：2表示当月的第二周，5表示当月的最后一周，6表示当月的倒数第二周
//    date_type为3时，表示以完整的星期计数，即月首从第一个星期日开始计算周数，周末从最后一个星期六开始计算。
typedef struct _FESTIVAL {
    uint8_t month;
    uint8_t day;
    struct {
        uint8_t date_type:  2;      /*!< 0: 公历日期, 1: 农历日期, 2: 星期, 3: 完整星期 */
        uint8_t is_holiday: 1;      /*!< 是否节假日 */
    } flags;
    char * festival;
} FESTIVAL;


const FESTIVAL festivals[] = {
    { .month = 1,  .day = 1,  .flags = { .is_holiday = 1, .date_type = 0 }, "元旦" },
    { .month = 2,  .day = 14, .flags = { .is_holiday = 0, .date_type = 0 }, "情人节" },
    { .month = 3,  .day = 8,  .flags = { .is_holiday = 0, .date_type = 0 }, "妇女节" },
    { .month = 3,  .day = 12, .flags = { .is_holiday = 0, .date_type = 0 }, "植树节" },
    { .month = 5,  .day = 1,  .flags = { .is_holiday = 1, .date_type = 0 }, "劳动节" },
    { .month = 5,  .day = 4,  .flags = { .is_holiday = 0, .date_type = 0 }, "青年节" },
    { .month = 6,  .day = 1,  .flags = { .is_holiday = 0, .date_type = 0 }, "儿童节" }, 
    { .month = 7,  .day = 1,  .flags = { .is_holiday = 0, .date_type = 0 }, "建党节" },
    { .month = 8,  .day = 1,  .flags = { .is_holiday = 0, .date_type = 0 }, "建军节" },
    { .month = 9,  .day = 10, .flags = { .is_holiday = 0, .date_type = 0 }, "教师节" },
    { .month = 10, .day = 1,  .flags = { .is_holiday = 1, .date_type = 0 }, "国庆节" },
    { .month = 12, .day = 24, .flags = { .is_holiday = 0, .date_type = 0 }, "平安夜" },
    { .month = 12, .day = 25, .flags = { .is_holiday = 0, .date_type = 0 }, "圣诞节" },

    { .month = 1,  .day = 1,  .flags = { .is_holiday = 1, .date_type = 1 }, "春节" },
    { .month = 1,  .day = 15, .flags = { .is_holiday = 0, .date_type = 1 }, "元宵节" },
    { .month = 1,  .day = 25, .flags = { .is_holiday = 0, .date_type = 1 }, "填仓节" },
    { .month = 5,  .day = 5,  .flags = { .is_holiday = 0, .date_type = 1 }, "端午节" },
    { .month = 7,  .day = 7,  .flags = { .is_holiday = 0, .date_type = 1 }, "七夕节" },
    { .month = 7,  .day = 15, .flags = { .is_holiday = 0, .date_type = 1 }, "中元节" },
    { .month = 8,  .day = 15, .flags = { .is_holiday = 1, .date_type = 1 }, "中秋节" },
    { .month = 9,  .day = 9,  .flags = { .is_holiday = 0, .date_type = 1 }, "重阳节" },
    { .month = 12, .day = 8,  .flags = { .is_holiday = 0, .date_type = 1 }, "腊八节" },
    { .month = 12, .day = 23, .flags = { .is_holiday = 0, .date_type = 1 }, "北方小年" },
    { .month = 12, .day = 24, .flags = { .is_holiday = 0, .date_type = 1 }, "南方小年" },
    { .month = 12, .day = 0,  .flags = { .is_holiday = 1, .date_type = 1 }, "除夕" },

    { .month = 5,  .day = 20, .flags = { .is_holiday = 0, .date_type = 2 }, "母亲节" },
    { .month = 6,  .day = 30, .flags = { .is_holiday = 0, .date_type = 2 }, "父亲节" },
    { .month = 11, .day = 44, .flags = { .is_holiday = 0, .date_type = 2 }, "感恩节" },
};

const FESTIVAL COMMEMORATION_DAY[] = {
    { .month = 1,  .day = 8,  .flags = { .is_holiday = 1, .date_type = 0 }, "周恩来逝世纪念日[1976]" },
    { .month = 5,  .day = 17, .flags = { .is_holiday = 0, .date_type = 0 }, "世界电信日[1969]" },
    { .month = 9,  .day = 9, .flags =  { .is_holiday = 0, .date_type = 0 }, "毛泽东逝世纪念日[1976]" },
    { .month = 12, .day = 26, .flags = { .is_holiday = 0, .date_type = 0 }, "毛泽东诞辰纪念日[1893]" },

    { .month = 2,  .day = 2,  .flags = { .is_holiday = 0, .date_type = 1 }, "龙抬头" },
    { .month = 9,  .day = 9,  .flags = { .is_holiday = 0, .date_type = 1 }, "中国老年节[1989]" },

    { .month = 4,  .day = 53, .flags = { .is_holiday = 0, .date_type = 3 }, "秘书节[1952]" },   // 4月最后一个完整星期的周三
};

// commemoration day
// static const char * GRE_DAYS[] = { //公历节日 *表示放假日
//     "0101*新年元旦",
//     "0106 中国13亿人口日[2005]",
//     "0121 列宁逝世纪念日[1924]",
//     "0127 宋庆龄诞辰纪念日[1893]",
//     "0202 世界湿地日[1996]", 
//     "0207 国际声援南非日[1964]", 
//     "0210 世界气象日[1960]", 
//     "0214 西方情人节", 
//     "0215 中国12亿人口日[1995]", 
//     "0219 邓小平逝世纪念日[1997]", 
//     "0221 反对殖民制度斗争日[1949]", 
//     "0224 第三世界青年日", 
//     "0228 世界居住条件调查日", 
//     //-
//     "0301 国际海豹日[1983]", 
//     "0303 全国爱耳日[2000]", 
//     "0305 中国青年志愿者服务日[2000] 毛泽东题词“向雷锋同志学习”[1963] 周恩来诞辰纪念日[1898] 斯大林逝世纪念日[1953]", 
//     "0308 国际妇女节[1910]", 
//     "0312 植树节[1979] 孙中山逝世纪念日", 
//     "0314 国际警察日(节) 马克思逝世纪念日[1883]", 
//     "0315 国际消费者权益日[1983]", 
//     "0316 手拉手情系贫困小伙伴全国统一行动日", 
//     "0317 中国国医节[1929] 国际航海日", 
//     "0317 中国国医节[1929] 国际航海日", 
//     "0318 全国科技人才活动日 巴黎公社纪念日[1871]", 
//     "0321 世界睡眠日[2001] 世界儿歌日 世界森林日(林业节)[1972] 消除种族歧视国际日[1976]", 
//     //"0321 世界儿歌日", 
//     "0322 世界水日[1993] 中国水周(3月22日至3月28日)[1988设/1994改]", 
//     "0323 世界气象日[1950]", 
//     "0324 世界防治结核病日[1996]", 
//     //"0325 全国中小学生安全教育日", 
//     "0330 巴勒斯坦国土日", 
//     //-
//     "0401 愚人节 全国爱国卫生运动月(四月) 税收宣传月(四月)", 
//     "0402 国际儿童图书日", 
//     "0407 世界卫生日[1950] 1994年卢旺达境内灭绝种族罪行国际反思日[2004]", 
//     "0421 全国企业家活动日[1994]", 
//     "0422 世界地球日[1970] 列宁诞辰纪念日[1870]", 
//     "0423 世界图书和版权日", 
//     "0424 世界青年反对殖民主义日[1957] 亚非新闻工作者日", 
//     "0425 全国预防接种宣传日[1986]", 
//     "0426 世界知识产权日[2001]", 
//     "0427 联谊城日", 
//     "0430 全国交通安全反思日", 
//     //-
//     "0501 国际劳动节[1889] 国际示威游行日", 
//     "0503 世界哮喘日", 
//     "0504 中国五四青年节[1939] 五四运动纪念日[1919] 科技传播日", 
//     "0505 碘缺乏病防治日[1994] 马克思诞辰纪念日[1818]", 
//     "0508 世界红十字日[1948] 世界微笑日", 
//     "0512 国际护士节[1912]", 
//     "0515 国际家庭(咨询)日[1994]", 
//     "0517 世界电信日[1969]", 
//     "0518 国际博物馆日", 
//     "0520 全国学生营养日[1990] 全国学生营养日[1990]", 
//     "0522 国际生物多样性日[1994设/2001改]", 
//     //"0523 国际牛奶日", 
//     "0526 世界向人体条件挑战日[1993]", 
//     "0529 宋庆龄逝世纪念日[1981]", 
//     "0530 “五卅”反对帝国主义运动纪念日[1925]", 
//     "0531 世界无烟日[1988]",  
//     //-
//     "0601 国际儿童节[1949]", 
//     "0605 世界环境日[1974]", 
//     "0606 全国爱眼日[1996]", 
//     "0611 中国人口日", 
//     "0617 防治荒漠化和干旱日[1995]", 
//     "0620 世界难民日[2001]", 
//     "0622 中国儿童慈善活动日", 
//     "0623 国际奥林匹克日[1894] 世界手球日", 
//     "0625 全国土地日[1991]", 
//     "0626 国际反毒品日[1987] 国际宪章日(联合国宪章日)", 
//     "0630 世界青年联欢节", 
//     //-
//     "0701 中国共产党建党日[1921] 香港回归纪念日[1997] 国际建筑日[1985] 亚洲“三十亿人口日”[1988]", 
//     "0702 国际体育记者日", 
//     "0706 朱德逝世纪念日[1976]", 
//     "0707 中国人民抗日战争纪念日[1937]", 
//     "0711 中国航海日[2005] 世界(50亿)人口日[1987]", 
//     "0720 人类首次成功登月[1969]", 
//     "0726 世界语(言)创立日", 
//     "0728 第一次世界大战爆发[1914]", 
//     "0730 非洲妇女日", 
//     //-
//     "0801 中国人民解放军建军节[1927]", 
//     "0805 恩格斯逝世纪念日[1895]", 
//     "0806 国际电影节[1932]", 
//     "0808 中国男子节(爸爸节)[1988]", 
//     "0809 世界土著人民国际日[1994]", 
//     "0812 国际青年人日[1999]", 
//     "0813 国际左撇子日[1975设/1976]", 
//     "0815 日本正式宣布无条件投降日[1945] 世界反法西斯战争胜利纪念日[1945]", 
//     "0826 全国律师咨询日[1993]", 
//     //-
//     "0903 中国抗日战争胜利纪念日[1945]", 
//     "0908 世界扫盲日[1966] 国际新闻工作者(团结)日[1958]", 
//     "0910 教师节[1985]", 
//     "0914 世界清洁地球日", 
//     "0916 国际臭氧层保护日[1987]", 
//     "0918 九·一八事变纪念日(中国国耻日)[1931]", 
//     "0920 全国公民道德宣传日[2003] 全国爱牙日[1989]", 
//     "0921 国际和平日(全球停火和非暴力日,2002年以后)[2002]", 
//     "0925 鲁迅诞辰纪念日[1881]", 
//     "0926 (曲阜国际)孔子文化节[1989]", 
//     "0927 世界旅游日[1979]", 
//     "0929 世界心脏日[2000]",
//     //-
//     "1001*国庆节[1949] 国际音乐日[1980] 国际敬老日(老人节)[1991]", 
//     "1002 国际和平(与民主自由)斗争日[1949]", 
//     "1004 世界动物日[1949]", 
//     "1008 全国高血压日[1998] 狮子会世界视觉日[1998]", 
//     "1009 世界邮政日(万国邮联日)[1969]", 
//     "1010 辛亥革命纪念日[1911] 世界精神卫生日[1992] 世界居室卫生日", 
//     "1011 声援南非政治犯日", 
//     "1012 世界(60亿)人口日[1999]", 
//     "1013 中国少年先锋队建队纪念日[1949] 世界保健日 国际教师节 采用格林威治时间为国际标准时间日[1884]", 
//     "1014 世界标准日[1969]", 
//     "1015 国际盲人节(白手杖节)[1984]", 
//     "1016 世界粮食日[1979]", 
//     "1017 世界消除贫困日[1992]", 
//     "1019 鲁迅逝世纪念日[1936]", 
//     "1022 世界传统医药日[1992]", 
//     "1024 联合国日[1945] 世界发展信息日", 
//     "1028 世界“男性健康日”[2000]", 
//     "1031 世界勤俭日", 
//     //-
//     "1107 十月社会主义革命纪念日(现俄“和谐和解日”)[1917]", 
//     "1108 中国记者日[2000]", 
//     "1109 全国消防安全宣传教育日[1992]", 
//     "1110 世界青年节[1946]", 
//     "1111 光棍节 国际科学与和平周(本日所属的一周)", 
//     "1112 孙中山诞辰纪念日[1866, 1926定] 刘少奇逝世纪念日[1969]", 
//     "1114 世界糖尿病日[1991]", 
//     "1117 国际大学生节(世界学生节)[1946]", 
//     "1120 世界儿童日[1990]", 
//     "1121 世界问候日[1973] 世界电视日[1996]", 
//     "1124 刘少奇诞辰纪念日[1893]", 
//     "1128 恩格斯诞辰纪念日[1820]", 
//     "1129 国际声援巴勒斯坦人民国际日[1977]", 
//     //-
//     "1201 世界艾滋病日[1988] 朱德诞辰纪念日[1886]", 
//     "1202 废除一切形式奴役世界日[1986]", 
//     "1203 世界残疾人日[1992]", 
//     "1204 中国法制宣传日[2001]", 
//     "1205 国际经济和社会发展志愿人员日[1985] 世界弱能人士日", 
//     "1207 国际民航日[纪念1994, 1996定]", 
//     //"1208 国际儿童电视日", 
//     "1209 一二·九运动纪念日[1935] 世界足球日[1995]", 
//     "1210 世界人权日(诺贝尔日)[1950]", 
//     "1211 世界防治哮喘日[1998]", 
//     "1212 西安事变纪念日[1936]", 
//     "1213 南京大屠杀纪念日·勿忘国耻,紧记血泪史![1937]", 
//     "1215 世界强化免疫日", 
//     "1220 澳门回归纪念日[1999]", 
//     "1221 国际篮球日 斯大林诞辰纪念日[1879]", 
//     "1224 平安夜", 
//     "1225 圣诞节", 
//     //"1229 国际生物多样性日")
// };

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Tells whether a year is leap year or not
 * @param year a year
 * @return 0: not leap year; 1: leap year
 */
uint8_t is_leap_year(int32_t year)
{
    return (year % 4) || ((year % 100 == 0) && (year % 400)) ? 0 : 1;
}

/**
 * Get the number of days in a month
 * @param year a year
 * @param month a month. The range is basically [1..12]
 * @return [28..31]
 */
uint8_t get_month_days(int32_t year, uint8_t month)
{
    // assert(month > 0 && month <= 12); //, "set month between 1 and 12");

    return DAY_OF_MONTH[month-1] + (month == 2 && is_leap_year(year) ? 1 : 0) ;
}

/**
 * Get the day of the week
 * @param year a year
 * @param month a  month [1..12]
 * @param day a day [1..31]
 * @return [0..6] which means [Sun..Sat] or [Mon..Sun] depending on LV_CALENDAR_WEEK_STARTS_MONDAY
 */
uint8_t get_day_of_week(int32_t year, uint8_t month, uint8_t day)
{
#if 0
    uint32_t a = month < 3 ? 1 : 0;
    uint32_t b = year - a;

#if LV_CALENDAR_WEEK_STARTS_MONDAY
    uint32_t day_of_week = (day + (31 * (month - 2 + 12 * a) / 12) + b + (b / 4) - (b / 100) + (b / 400) - 1) % 7;
#else
    uint32_t day_of_week = (day + (31 * (month - 2 + 12 * a) / 12) + b + (b / 4) - (b / 100) + (b / 400)) % 7;
#endif

    return day_of_week;
#else
    if (month < 3)
    {
	    --year;
	    month += 12;
	}
	return (day + (13*month+8)/5 + year + (year/4) - (year/100) + (year/400)) % 7;
#endif
}

/**
 * Get days elapsed from 1900/01/01
 */
int32_t days_from_1900(int32_t year, uint8_t month, uint8_t day)
{
    if (year < 1900 || month > 12) return -1;

    uint32_t days = (year - 1900) * 365;
    if (year > 1904)
    {
        days += (year - 1901) / 4;
        if (year > 2100)
        {
            int n = year - 2001;
            days -= (n / 100);
            days += (n / 400);
        }
    }

    days += (DAY_OF_YEAR[month - 1] + day);
    if (!(month > 2 && is_leap_year(year)))
    {
        days -= 1;
    }
    return days;
}

/**
 * Get seconds elapsed from 1900/01/01 00:00:00
 */
int64_t seconds_from_1900(int32_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second)
{
    if (year < 1900 || month > 12) return -1;
    return days_from_1900(year, month, day) * 86400 + hour * 3600 + minute * 60 + second;
}

/**
 * Get days elapsed from 00/03/01
 * y: a year greater than or equal to 0
 * m: month, must between 1 and 12
 * d: day, must between 1 and 31
 */
uint32_t solar_date_to_days(int32_t y, int m, int d)
{
    m = (m + 9) % 12;
    y = y - m / 10;
    return 365 * y + y / 4 - y / 100 + y / 400 + (m * 306 + 5) / 10 + (d - 1);
}

/**
 * Convert days which elapsed from 0000/03/01 to a solar date
 * g: days elapsed from 00/03/01
 */
void days_to_solar_date(uint64_t g, DATE * solar_date)
{
    uint64_t y = (g * 10000 + 14780) / 3652425;
    int32_t ddd = g - (365 * y + y / 4 - y / 100 + y / 400);
    if (ddd < 0)
    {
        y--;
        ddd = g - (365 * y + y / 4 - y / 100 + y / 400);
    }

    int32_t mi = (100 * ddd + 52) / 3060;
    solar_date->year  = (int)(y + (mi + 2) / 12);
    solar_date->month = (int)((mi + 2) % 12 + 1);
    solar_date->day   = (int)(ddd - (mi * 306 + 5) / 10 + 1);
}

/**
 * 返回农历 year年闰哪个月 1-12 , 没闰返回 0
 *
 * @param lunar_year 将要计算的年份，取值范围：1900 ～ 2099
 * @return 返回农历 year年闰哪个月(1-12), 没闰返回 0
 */
uint8_t get_leap_month(int32_t lunar_year)
{
    uint32_t monthInfo = lunar_month_days[lunar_year - lunar_month_days[0]];
    return (uint8_t)(monthInfo >> 13) & 0x0F;
    // return (uint8_t)(LUNAR_INFO[lunar_year - MIN_YEAR] >> 20) & 0x0F;
}

/**
 * 传回农历 year年的总天数
 *
 * @param year 将要计算的年份
 * @return 返回传入年份的总天数
 */
uint32_t get_lunar_year_days(int32_t year)
{
    uint32_t sum = get_leap_month(year) > 0 ? 377 : 348;

    uint32_t monthInfo = lunar_month_days[year - lunar_month_days[0]];
    // uint32_t monthInfo = LUNAR_INFO[year - MIN_YEAR] & 0x0FFF80;
    for (int32_t i = 0x1000; i > 0; i >>= 1)
    {
        if ((monthInfo & i))
            ++sum;
    }
    return sum;
}

/**
 * 返回农历年某个月份的总天数
 *
 * @param year  将要计算的年份，取值范围: 1900 ~ 2099
 * @param month 将要计算的月份，取值范围: 1 ～ 13。如果该年包含闰月，则闰月及其之后的月份索引比其实际月份大1
 * @return 返回农历该月的总天数
 */
static uint8_t _get_lunar_month_days(int32_t year, uint8_t month)
{
    return (lunar_month_days[year - lunar_month_days[0]] & (0x2000 >> month)) ? 30 : 29;
}

/**
 * 返回农历年某个月份的总天数
 *
 * @param year    将要计算的年份，取值范围: 1900 ~ 2099
 * @param month   将要计算的月份，取值范围: 1 ～ 12
 * @param is_leap 1表示该月为闰月，0表示普通月份
 * @return 返回农历该月的总天数
 */
uint8_t get_lunar_month_days(int32_t year, uint8_t month, uint8_t is_leap)
{
    uint8_t leap_month = get_leap_month(year);
    if (leap_month && (month > leap_month || (is_leap && month == leap_month)))
    {
        month++;
    }

    return _get_lunar_month_days(year, month);
}

int getBitInt(int data, int length, int shift)
{
    return (data & (((1 << length) - 1) << shift)) >> shift;
}

/**
 * 公历转农历 Solar To Lunar
 * @param year  公历年
 * @param month 公历月
 * @param day   公历日
 * @return [0]农历年 [1]农历月 [2]农历日 [3]是否闰月 0 false : 1 true
 */
int get_lunar_date(int32_t year, uint8_t month, uint8_t day, DATE * lunar_date)
{
    int index = year - solar_1_1[0];
    int data = (year << 9) | (month << 5) | day;
    int solar11;
    if (solar_1_1[index] > data)
    {
        index--;
    }
    solar11 = solar_1_1[index];
    int y = getBitInt(solar11, 12, 9);
    int m = getBitInt(solar11, 4, 5);
    int d = getBitInt(solar11, 5, 0);
    long offset = solar_date_to_days(year, month, day) - solar_date_to_days(y, m, d);

    int days = lunar_month_days[index];
    int leap = getBitInt(days, 4, 13);

    int lunarY = index + solar_1_1[0];
    int lunarM = 1;
    int lunarD;
    offset += 1;

    for (int i = 0; i < 13; i++)
    {
        int dm = getBitInt(days, 1, 12 - i) == 1 ? 30 : 29;
        if (offset > dm) {
            lunarM++;
            offset -= dm;
        } else {
            break;
        }
    }
    lunarD = (int) (offset);
    lunar_date->year = lunarY;
    lunar_date->month = lunarM;
    lunar_date->day = lunarD;

    lunar_date->ext.is_leap_month = 0;
    if (leap != 0 && lunarM > leap)
    {
        lunar_date->month -= 1;
        if (lunar_date->month == leap)
        {
            lunar_date->ext.is_leap_month = 1;
        }
    }
    return 0;
}

/**
 * 农历转公历
 * @param lunarYear  农历年
 * @param lunarMonth 农历月
 * @param lunarDay   农历天
 * @param isLeap     是否是闰年 0 false : 1 true
 * @return [0]新历年 [1]新历月 [2]新历日 [3]是否闰月 0 false : 1 true
 */
int lunarToSolar(int lunarYear, int lunarMonth, int lunarDay, uint8_t isLeap)
{
    int days = lunar_month_days[lunarYear - lunar_month_days[0]];
    int leap = getBitInt(days, 4, 13);
    int offset = 0;
    int loop = leap;
    if (!isLeap) {
        if (lunarMonth <= leap || leap == 0) {
            loop = lunarMonth - 1;
        } else {
            loop = lunarMonth;
        }
    }
    for (int i = 0; i < loop; i++) {
        offset += getBitInt(days, 1, 12 - i) == 1 ? 30 : 29;
    }
    offset += lunarDay;

    int solar11 = solar_1_1[lunarYear - solar_1_1[0]];

    int y = getBitInt(solar11, 12, 9);
    int m = getBitInt(solar11, 4, 5);
    int d = getBitInt(solar11, 5, 0);

    DATE solar_date;
    days_to_solar_date(solar_date_to_days(y, m, d) + offset - 1, &solar_date);

    return 0;
}

// month: 公历月份
// day:   公历日期
const char * get_gre_festival(uint8_t month, uint8_t day)
{
    for (int i=0; i < sizeof(festivals)/sizeof(FESTIVAL); ++i)
    {
        const FESTIVAL * fes = &festivals[i];
        if (fes->flags.date_type == 0 && fes->month == month)
        {
            if (fes->day == day) // || (fes->day == 0 && day == DAY_OF_MONTH[month-1]))
            {
                return fes->festival;
            }
        }
    }

    return NULL;
}

// lunar_date: 农历日期
const char * get_lunar_festival(DATE * lunar_date)
{
    for (int i=0; i < sizeof(festivals)/sizeof(FESTIVAL); ++i)
    {
        const FESTIVAL * fes = &festivals[i];
        if (fes->flags.date_type == 1 && fes->month == lunar_date->month)
        {
            if (lunar_date->day == fes->day)
            {
                return fes->festival;
            }
            else if (fes->day == 0)
            {
                if (lunar_date->day == 30)
                {
                    return fes->festival;
                }
                else if (lunar_date->day == 29)
                {
                    if (get_lunar_month_days(lunar_date->year, lunar_date->month, lunar_date->ext.is_leap_month) == 29)
                    {
                        return fes->festival;
                    }
                }
            }
        }
    }
    return NULL;
}

// days_of_month = get_month_days(year, month);
// week_of_day1  = get_day_of_week(year, month, 1);
// week_last_day = (days_of_month - 1 + week_of_day1) % 7;
const char * get_week_festival(uint8_t month, uint8_t day, uint8_t days_of_month, uint8_t week_of_day1)
{
#if 0
    char date[16];
	uint8_t advance = 0;
	uint8_t reverse = 0;
    uint8_t week = (day - 1 + week_of_day1) % 7;
    snprintf(date, sizeof(date), "%02d%d", month, week);
    for (int i=0; i < sizeof(WEEK_FESTIVAL)/sizeof(char *); ++i)
    {
        int n = strncmp(WEEK_FESTIVAL[i], date, 3);
        if (n == 0)
        {
            uint8_t no = WEEK_FESTIVAL[i][3] - '0';
			if (WEEK_FESTIVAL[i][4] == '+')
			{
				uint8_t first_sunday = week_of_day1 > 0 ? 8 - week_of_day1 : 1;
				uint8_t last_saturday = days_of_month - (days_of_month + week_of_day1) % 7;
				if (day <= last_saturday)
				{
					reverse = (last_saturday - day) / 7 + 1;
				}
				if (day >= first_sunday)
				{
					advance = (day - first_sunday) / 7 + 1;
				}
			}
			else
			{
				advance = (day + 6) / 7;                    // 正数第几周，从1开始，取值1、2、3、4
    			reverse = (days_of_month - day) / 7 + 1;   // 倒数第几周，从1开始，取值1、2、3、4
			}

			if (no == advance || no == (reverse + 4))
            {
                return WEEK_FESTIVAL[i];
            }
        }
        else if (n > 0)
        {
            break;
        }
    }
#else
    uint8_t advance = 0;
	uint8_t reverse = 0;
    uint8_t week = (day - 1 + week_of_day1) % 7;
    for (int i=0; i < sizeof(festivals)/sizeof(FESTIVAL); ++i)
    {
        const FESTIVAL * fes = &festivals[i];
        if (fes->flags.date_type > 1 && fes->month == month)       // 此处为公历月份
        {
            if (fes->day % 10 == week) // 比较星期
            {
                uint8_t no = fes->day / 10;
                if (fes->flags.date_type == 3)
                {
                    uint8_t first_sunday = week_of_day1 > 0 ? 8 - week_of_day1 : 1;
                    uint8_t last_saturday = days_of_month - (days_of_month + week_of_day1) % 7;
                    if (day <= last_saturday)
                    {
                        reverse = (last_saturday - day) / 7 + 1;
                    }
                    if (day >= first_sunday)
                    {
                        advance = (day - first_sunday) / 7 + 1;
                    }
                }
                else
                {
                    advance = (day + 6) / 7;                   // 正数第几周，从1开始，取值1、2、3、4
                    reverse = (days_of_month - day) / 7 + 1;   // 倒数第几周，从1开始，取值1、2、3、4
                }

                if (no == advance || no == (reverse + 4))
                {
                    return fes->festival;
                }
            }
        }
    }
#endif

    return NULL;
}

const char * get_animal_str(int32_t lunar_year)
{
    return ANIMALS[(lunar_year - 4) % 12];
}

const char * get_gan_str(int32_t lunar_year)
{
    return TIANGAN[(lunar_year - 4) % 10];
}

const char * get_zhi_str(int32_t lunar_year)
{
    return DIZHI[(lunar_year - 4) % 12];
}

const char * get_year_ganzhi_str(int32_t lunar_year)
{
    static char ganzhi[10];
    snprintf(ganzhi, sizeof(ganzhi), "%s%s", get_gan_str(lunar_year), get_zhi_str(lunar_year));
    return ganzhi;
}

// 凡甲年己年，一月天干为丙，二月天干为丁，其余顺推；
// 凡乙年庚年，一月天干为戊，二月天干为己，其余顺推；
// 凡丙年辛年，一月天干为庚，二月天干为辛，其余顺推；
// 凡丁年壬年，一月天干为壬，二月天干为癸，其余顺推；
// 凡戊年癸年，一月天干为甲，二月天干为乙，其余顺推；
// 每月的地支是固定的，正月为寅，二月为卯，三月为辰，四月为巳，五月为午，六月为未，七月为申，八月为酉，九月为戌，十月为亥，十一月为子，十二月为丑
// lunar_month: 农历月份，取值：1 ～ 12
const char * get_month_ganzhi_str(int32_t lunar_year, uint8_t lunar_month)
{
    static char ganzhi[10];
    // const char * gan;
    // switch ((lunar_year - 4) % 10)
    // {
    //     case 0: case 5:     // 甲年己年，一月天干为丙
    //         gan = TIANGAN[(lunar_month + 1) % 10];
    //         break;
    //     case 1: case 6:     // 乙年庚年，一月天干为戊
    //         gan = TIANGAN[(lunar_month + 3) % 10];
    //         break;
    //     case 2: case 7:     // 丙年辛年，一月天干为庚
    //         gan = TIANGAN[(lunar_month + 5) % 10];
    //         break;
    //     case 3: case 8:     // 丁年壬年，一月天干为壬
    //         gan = TIANGAN[(lunar_month + 7) % 10];
    //         break;
    //     case 4: case 9:     // 戊年癸年
    //         gan = TIANGAN[(lunar_month + 9) % 10];
    //         break;
    // }

    uint8_t shift = ((lunar_year - 4) % 5) * 2 + 1;
    const char * gan = TIANGAN[(lunar_month + shift) % 10];
    const char * zhi = DIZHI[(lunar_month + 1) %12];
    snprintf(ganzhi, sizeof(ganzhi), "%s%s", gan, zhi);
    return ganzhi;
}

const char * get_day_ganzhi_str(int32_t gre_year, uint8_t month, uint8_t day)
{
    static char ganzhi[10];

    int32_t days = days_from_1900(gre_year, month, day);
    if (days >= 0)
    {
        days += 10;     // 1900/1/1为甲戌日(60进制10)
        snprintf(ganzhi, sizeof(ganzhi), "%s%s", TIANGAN[days%10], DIZHI[days%12]);
        return ganzhi;
    }
    return "";
}

const char * get_festival(int32_t year, uint8_t month, uint8_t day)
{
    uint8_t advance = 0;
	uint8_t reverse = 0;
    uint8_t days_of_month  = get_month_days(year, month);
    uint8_t week_of_day1 = get_day_of_week(year, month, 1);
    uint8_t week = (day - 1 + week_of_day1) % 7;
    for (int i=0; i < sizeof(festivals)/sizeof(FESTIVAL); ++i)
    {
        const FESTIVAL * fes = &festivals[i];
        if (fes->flags.date_type == 1)      // 比较农历月份和农历日期
        {
            if (fes->month == month)        // 此处为农历月份
            {
                if (fes->day == day || (fes->day == 0 && day == days_of_month))
                {
                    return fes->festival;
                }
            }
        }
        else if (fes->month == month)       // 此处为公历月份
        {
            if (fes->flags.date_type == 0)
            {
                if (fes->day == day || (fes->day == 0 && day == days_of_month))
                {
                    return fes->festival;
                }
            }
            else if (fes->day % 10 == week) // 比较星期
            {
                uint8_t no = fes->day / 10;
                if (fes->flags.date_type == 3)
                {
                    uint8_t first_sunday = week_of_day1 > 0 ? 8 - week_of_day1 : 1;
                    uint8_t last_saturday = days_of_month - (days_of_month + week_of_day1) % 7;
                    if (day <= last_saturday)
                    {
                        reverse = (last_saturday - day) / 7 + 1;
                    }
                    if (day >= first_sunday)
                    {
                        advance = (day - first_sunday) / 7 + 1;
                    }
                }
                else
                {
                    advance = (day + 6) / 7;                   // 正数第几周，从1开始，取值1、2、3、4
                    reverse = (days_of_month - day) / 7 + 1;   // 倒数第几周，从1开始，取值1、2、3、4
                }

                if (no == advance || no == (reverse + 4))
                {
                    return fes->festival;
                }
            }
        }
    }
    return NULL;
}

const char * get_lunar_month_str(uint8_t lunar_month, uint8_t is_leap_month)
{
    static char str[10];
    snprintf(str, sizeof(str), is_leap_month ? "闰%s%s" : "%s%s", MONTH_CN[lunar_month], MONTH_CN[0]);
    return str;
}

const char * get_lunar_day_str(uint8_t lunar_day)
{
    static char str[10];
    switch (lunar_day)
    {
        case 10:
            return "初十";
        case 20:
            return "二十";
        case 30:
            return "三十";
        default:
            snprintf(str, sizeof(str), "%s%s", DAY_TENS_DIGIT[lunar_day / 10], DAY_UNIT_DIGIT[lunar_day % 10]);
            break;
    }    
    return str;
}

const char * get_week_str(uint8_t week)
{
    return DAY_UNIT_DIGIT[week];
}

// 计算某年的第n个节气为几日(从小寒开始，n取值范围：0～23)
// 地球公转周期31556925974.7毫秒
// 基准小寒点 1900年1月6日2时5分
// const char * get_day_of_solar_term(uint32_t y, uint8_t n, DATE * solar_date)
// {
//     // const uint32_t base_days = solar_date_to_days(1900, 1, 6);
//     // printf("\n===> base_days: %u\n", base_days);

//     uint64_t ms = ((uint64_t)y - 1900) * 31556925974;// + ((uint64_t)SOLAR_TERM_OFFSET[n]) * 60000;
//     uint32_t days = (ms / 1000) / 86400;
//     uint32_t remain = (ms / 1000 + 7500) % 86400;
//     printf("\n===> ms: %llu, days: %u, remain: %u\n", ms, days, remain);

//     days_to_solar_date(days + 693906, solar_date);
//     return SOLAR_TERM_NAME[n];
// }

// const constellation = new Array(
//   {s: "0120", e: "0218", c: "水瓶座"},
//   {s: "0219", e: "0320", c: "双鱼座"},
//   {s: "0321", e: "0419", c: "白羊座"},
//   {s: "0420", e: "0520", c: "金牛座"},
//   {s: "0521", e: "0621", c: "双子座"},
//   {s: "0622", e: "0722", c: "巨蟹座"},
//   {s: "0723", e: "0822", c: "狮子座"},
//   {s: "0823", e: "0922", c: "处女座"},
//   {s: "0923", e: "1023", c: "天秤座"},
//   {s: "1024", e: "1122", c: "天蝎座"},
//   {s: "1123", e: "1221", c: "射手座"},
//   {s: "1222", e: "0119", c: "摩羯座"}
// );
// // 计算星座
// // sy sm sd 为传入年月日
// let constellationStr = null;
// let c = sm * 100 + sd; //把传入日期数据转化为与数组中可对比的数字
// constellation.forEach((item, index)=>{
//   if(!constellationStr){
//     if(c >= Number(item.s) && c <= Number(item.e)){
//       constellationStr = item.c;
//     }
//     else if(c >= Number(item.s) || c <= Number(item.e)){
//       constellationStr = item.c;
//     }
//   }
// })