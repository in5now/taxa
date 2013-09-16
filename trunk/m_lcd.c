/*
 * m_lcd.c
 *
 *  Created on: 08.03.2013
 *      Author: admin
 */

#include "TCPIP Stack/TCPIP.h"
#include "m_lcd.h"
#include "config.h"

decode_t decode_cb = NULL;
//BYTE (*decode_cb)(BYTE);

static DWORD lcd_tout, lcd_timestamp;


static rom BYTE cp1251[] = {/*�*/65, /*�*/160, /*�*/66, /*�*/161, /*�*/224, /*�*/
71, /*�*/163, /*�*/164, /*�*/165, /*�*/166, /*�*/75, /*�*/167, /*�*/77, /*�*/
72, /*�*/79, /*�*/168, /*�*/80, /*�*/67, /*�*/84, /*�*/169, /*�*/170, /*�*/88, /*�*/
225, /*�*/171, /*�*/172, /*�*/226, /*�*/173, /*�*/174, /*�*/196, /*�*/175, /*�*/
176, /*�*/177, /*�*/97, /*�*/178, /*�*/179, /*�*/180, /*�*/227, /*�*/101, /*�*/
182, /*�*/183, /*�*/184, /*�*/185, /*�*/186, /*�*/187, /*�*/188, /*�*/189, /*�*/
111, /*�*/190, /*�*/112, /*�*/99, /*�*/191, /*�*/121, /*�*/228, /*�*/120, /*�*/
229, /*�*/192, /*�*/193, /*�*/230, /*�*/194, /*�*/195, /*�*/196, /*�*/197,
/*�*/198, /*�*/199 };


void LCDTest(WORD offset)
{
	BYTE ch;

	uitoa(offset, LCDText);
	for (ch = 0; ch < 16; ch++) {
		LCDText[ch + 16] = offset + ch;
	}
	LCDUpdate();
}

static BYTE _LCD_cp1251(BYTE in)
{
	if (in < 0xC0)
		return in;
	return cp1251[in - 0xC0];
}

void LCD_init(void)
{
	decode_cb = _LCD_cp1251;
}

BYTE * LCD_decode(BYTE *str)
{
	BYTE *s = str;
	for(; *s; s++) {
		*s = decode_cb ? (*decode_cb)(*s) : *s;
	}

	return str;
}

void LCD_set_tout(DWORD tout)
{
	 lcd_tout = tout;
	 lcd_timestamp = TickGet();
}

void LCD_serve_tout_prompt(void)
{
	 static DWORD t;

	 if(lcd_tout && ((TickGet() - lcd_timestamp) > lcd_tout)) {
		 LCD_prompt();
	 	lcd_tout = 0;
	 }
}

void LCD_prompt(void)
{
	strcpy(LCD_STRING_0, AppConfig.acc_prompt_msg);
	strcpypgm2ram(LCD_STRING_1, "                ");
	LCD_decode(LCD_ALL);
	LCDUpdate();
}

void LCD_show_addresses(void)
{
	BYTE *p_str = LCD_STRING_1;
	BYTE i;

	cvrt_ip_out(&AppConfig.MyIPAddr, LCD_STRING_0);

	for (i = 0; i < 6; i++) {
		*p_str ++ = btohexa_high(AppConfig.MyMACAddr.v[i]);
		*p_str ++ = btohexa_low(AppConfig.MyMACAddr.v[i]);
	}
	*p_str = '\0';
	LCD_decode(LCD_ALL);
	LCDUpdate();
}

void LCD_BackLight(BYTE light)
{
	// LCD light on/off
	LATGbits.LATG5 = light ? 1 : 0;
	TRISGbits.TRISG5 = 0;
} 