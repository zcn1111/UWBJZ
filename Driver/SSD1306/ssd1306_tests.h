#ifndef __SSD1306_TEST_H__
#define __SSD1306_TEST_H__

//#include <_ansi.h>
#include "BSP.h"
#include "ssd1306.h"
//_BEGIN_STD_C

void ssd1306_TestBorder(void);
void ssd1306_TestFonts1(void);
void ssd1306_TestFonts2(void);
void ssd1306_TestFPS(void);
void ssd1306_TestAll(void);
void ssd1306_TestLine(void);
void ssd1306_TestRectangle(void);
void ssd1306_TestRectangleFill(void);
void ssd1306_TestRectangleInvert(void);
void ssd1306_TestCircle(void);
void ssd1306_TestArc(void);
void ssd1306_TestPolyline(void);
void ssd1306_TestDrawBitmap(void);
void ssd1306_InitDrawBitmap();
void ssd1306_ZhengChang(uint8_t x, uint8_t y, SSD1306_COLOR color,uint8_t flag);
void ssd1306_maininterface(void);
void LedPowerONShow(void);
void ssd1306_CORSBenchmarkface(void);
//_END_STD_C

#endif // __SSD1306_TEST_H__
