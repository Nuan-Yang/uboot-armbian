/*
 * AMLOGIC lcd external driver.
 *
 * Communication protocol:
 * MIPI 
 *
 */

#include <common.h>
#include <asm/arch/io.h>
#include <asm/arch/gpio.h>
#include <amlogic/aml_lcd_extern.h>

//#define LCD_EXT_DEBUG_INFO
#ifdef LCD_EXT_DEBUG_INFO
#define DBG_PRINT(...)		printf(__VA_ARGS__)
#else
#define DBG_PRINT(...)
#endif

#define LCD_EXTERN_DEVICE_NODE    "/lcd_extern_mipi_N070ICN"
#define LCD_EXTERN_NAME			"lcd_mipi_N070ICN"
#define LCD_EXTERN_TYPE			LCD_EXTERN_MIPI

static struct lcd_extern_config_t lcd_ext_config ={
	.name = LCD_EXTERN_NAME,
	.type = LCD_EXTERN_TYPE,
};

//******************** mipi command ********************//
//format:  data_type, num, data....
//special: data_type=0xff, num<0xff means delay ms, num=0xff means ending.
//******************************************************//
static unsigned char mipi_init_on_table[] = {
    0x39,5,0xFF,0xAA,0x55,0xA5,0x80, //========== Internal setting ==========

    0x39,3,0x6F,0x11,0x00, //MIPI related Timing Setting
    0x39,3,0xF7,0x20,0x00,

    0x15,2,0x6F,0x06,      //Improve ESD option
    0x15,2,0xF7,0xA0,
    0x15,2,0x6F,0x19,
    0x15,2,0xF7,0x12,

    0x15,2,0x6F,0x08,     //Vcom floating
    0x15,2,0xFA,0x40,
    0x15,2,0x6F,0x11,
    0x15,2,0xF3,0x01,

    0x39,6,0xF0,0x55,0xAA,0x52,0x08,0x00, //========== page0 relative ==========
    0x15,2,0xC8,0x80,

    0x39,3,0xB1,0x6C,0x01, //Set WXGA resolution

    0x15,2,0xB6,0x08,      //Set source output hold time

    0x15,2,0x6F,0x02,      //EQ control function
    0x15,2,0xB8,0x08,

    0x39,3,0xBB,0x54,0x54, //Set bias current for GOP and SOP

    0x39,3,0xBC,0x05,0x05, //Inversion setting

    0x15,2,0xC7,0x01,      //zigzag setting

    0x39,6,0xBD,0x02,0xB0,0x0C,0x0A,0x00, //DSP Timing Settings update for BIST

    0x39,6,0xF0,0x55,0xAA,0x52,0x08,0x01, //========== page1 relative ==========

    0x39,3,0xB0,0x05,0x05,                // Setting AVDD, AVEE clamp
    0x39,3,0xB1,0x05,0x05,

    0x39,3,0xBC,0x3A,0x01,                // VGMP, VGMN, VGSP, VGSN setting
    0x39,3,0xBD,0x3E,0x01,

    0x15,2,0xCA,0x00,                    // gate signal control

    0x15,2,0xC0,0x04,                    // power IC control
    0x15,2,0xB2,0x00,
    0x15,2,0xBE,0x80,      //vcom    -1.88V

    0x39,3,0xB3,0x19,0x19, // Setting VGH=15V, VGL=-11V
    0x39,3,0xB4,0x12,0x12,

    0x39,3,0xB9,0x24,0x24, // power control for VGH, VGL
    0x39,3,0xBA,0x14,0x14,

    0x39,6,0xF0,0x55,0xAA,0x52,0x08,0x02, //========== page2 relative ==========

    0x15,2,0xEE,0x01,                     //Gamma setting
    0x39,5,0xEF,0x09,0x06,0x15,0x18,      //Gradient Control for Gamma Voltage

    0x39,7,0xB0,0x00,0x00,0x00,0x08,0x00,0x17, //========== GOA relative ==========
    0x15,2,0x6F,0x06,
    0x39,7,0xB0,0x00,0x25,0x00,0x30,0x00,0x45,
    0x15,2,0x6F,0x0C,
    0x39,5,0xB0,0x00,0x56,0x00,0x7A,
    0x39,7,0xB1,0x00,0xA3,0x00,0xE7,0x01,0x20, ////////////////////////////
    0x15,2,0x6F,0x06,
    0x39,7,0xB1,0x01,0x7A,0x01,0xC2,0x01,0xC5,
    0x15,2,0x6F,0x0C,
    0x39,5,0xB1,0x02,0x06,0x02,0x5F,
    0x39,7,0xB2,0x02,0x92,0x02,0xD0,0x02,0xFC,
    0x15,2,0x6F,0x06,
    0x39,7,0xB2,0x03,0x35,0x03,0x5D,0x03,0x8B,
    0x15,2,0x6F,0x0C,
    0x39,5,0xB2,0x03,0xA2,0x03,0xBF,
    0x39,5,0xB3,0x03,0xD2,0x03,0xFF,

    0x39,6,0xF0,0x55,0xAA,0x52,0x08,0x06,      //PAGE6 : GOUT Mapping, VGLO select
    0x39,3,0xB0,0x00,0x17,
    0x39,3,0xB1,0x16,0x15,
    0x39,3,0xB2,0x14,0x13,
    0x39,3,0xB3,0x12,0x11,
    0x39,3,0xB4,0x10,0x2D,
    0x39,3,0xB5,0x01,0x08,
    0x39,3,0xB6,0x09,0x31,
    0x39,3,0xB7,0x31,0x31,
    0x39,3,0xB8,0x31,0x31,
    0x39,3,0xB9,0x31,0x31,
    0x39,3,0xBA,0x31,0x31,
    0x39,3,0xBB,0x31,0x31,
    0x39,3,0xBC,0x31,0x31,
    0x39,3,0xBD,0x31,0x09,
    0x39,3,0xBE,0x08,0x01,
    0x39,3,0xBF,0x2D,0x10,
    0x39,3,0xC0,0x11,0x12,
    0x39,3,0xC1,0x13,0x14,
    0x39,3,0xC2,0x15,0x16,
    0x39,3,0xC3,0x17,0x00,
    0x39,3,0xE5,0x31,0x31,
    0x39,3,0xC4,0x00,0x17,
    0x39,3,0xC5,0x16,0x15,
    0x39,3,0xC6,0x14,0x13,
    0x39,3,0xC7,0x12,0x11,
    0x39,3,0xC8,0x10,0x2D,
    0x39,3,0xC9,0x01,0x08,
    0x39,3,0xCA,0x09,0x31,
    0x39,3,0xCB,0x31,0x31,
    0x39,3,0xCC,0x31,0x31,
    0x39,3,0xCD,0x31,0x31,
    0x39,3,0xCE,0x31,0x31,
    0x39,3,0xCF,0x31,0x31,
    0x39,3,0xD0,0x31,0x31,
    0x39,3,0xD1,0x31,0x09,
    0x39,3,0xD2,0x08,0x01,
    0x39,3,0xD3,0x2D,0x10,
    0x39,3,0xD4,0x11,0x12,
    0x39,3,0xD5,0x13,0x14,
    0x39,3,0xD6,0x15,0x16,
    0x39,3,0xD7,0x17,0x00,
    0x39,3,0xE6,0x31,0x31,
    0x39,6,0xD8,0x00,0x00,0x00,0x00,0x00, //VGL level select;
    0x39,6,0xD9,0x00,0x00,0x00,0x00,0x00,
    0x15,2,0xE7,0x00,

    0x39,6,0xF0,0x55,0xAA,0x52,0x08,0x03, //===page 3====//gate timing control
    0x39,3,0xB0,0x20,0x00,
    0x39,3,0xB1,0x20,0x00,
    0x39,6,0xB2,0x05,0x00,0x42,0x00,0x00,
    0x39,6,0xB6,0x05,0x00,0x42,0x00,0x00,
    0x39,6,0xBA,0x53,0x00,0x42,0x00,0x00,
    0x39,6,0xBB,0x53,0x00,0x42,0x00,0x00,
    0x15,2,0xC4,0x40,

    0x39,6,0xF0,0x55,0xAA,0x52,0x08,0x05, //===page 5====//
    0x39,3,0xB0,0x17,0x06,
    0x15,2,0xB8,0x00,
    0x39,6,0xBD,0x03,0x01,0x01,0x00,0x01,
    0x39,3,0xB1,0x17,0x06,
    0x39,3,0xB9,0x00,0x01,
    0x39,3,0xB2,0x17,0x06,
    0x39,3,0xBA,0x00,0x01,
    0x39,3,0xB3,0x17,0x06,
    0x39,3,0xBB,0x0A,0x00,
    0x39,3,0xB4,0x17,0x06,
    0x39,3,0xB5,0x17,0x06,
    0x39,3,0xB6,0x14,0x03,
    0x39,3,0xB7,0x00,0x00,
    0x39,3,0xBC,0x02,0x01,
    0x15,2,0xC0,0x05,
    0x15,2,0xC4,0xA5,
    0x39,3,0xC8,0x03,0x30,
    0x39,3,0xC9,0x03,0x51,
    0x39,6,0xD1,0x00,0x05,0x03,0x00,0x00,
    0x39,6,0xD2,0x00,0x05,0x09,0x00,0x00,
    0x15,2,0xE5,0x02,
    0x15,2,0xE6,0x02,
    0x15,2,0xE7,0x02,
    0x15,2,0xE9,0x02,
    0x15,2,0xED,0x33,

    0x05,1,0x11, //sleep out
    0xff,30,     //delay 30ms
    0x05,1,0x29, //display on
    0xff,30,     //delay 30ms
    0xff,0xff,   //ending flag
};

static unsigned char mipi_init_off_table[] = {
    0x05,1,0x28, //display off
    0xff,10,     //delay 10ms
    0x05,1,0x10, //sleep in
    0xff,10,     //delay 10ms
    0xff,0xff,   //ending flag
};

static int get_lcd_extern_config(char *dt_addr)
{
#ifdef CONFIG_OF_LIBFDT
	char *of_node = LCD_EXTERN_DEVICE_NODE;
	struct lcd_extern_config_t *pdata = &lcd_ext_config;
	
	if (get_lcd_extern_dt_data(dt_addr, of_node, pdata) != 0){
		printf("[error] %s probe: failed to get dt data\n", LCD_EXTERN_NAME);
		return -1;
	}
#endif
	return 0;
}

static struct aml_lcd_extern_driver_t lcd_ext_driver = {
    .name = LCD_EXTERN_NAME,
    .type = LCD_EXTERN_TYPE,
    .reg_read = NULL,
    .reg_write = NULL,
    .power_on = NULL,
    .power_off = NULL,
    .init_on_cmd_8  = &mipi_init_on_table[0],
    .init_off_cmd_8 = &mipi_init_off_table[0],
    .get_lcd_ext_config = get_lcd_extern_config,
};

struct aml_lcd_extern_driver_t* aml_lcd_extern_get_driver(void)
{
    return &lcd_ext_driver;
}
