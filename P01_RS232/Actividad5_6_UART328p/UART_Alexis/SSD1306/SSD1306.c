#include "SSD1306.h"

//----- Auxiliary data ------//
uint8_t __GLCD_Buffer[__GLCD_Screen_Width * __GLCD_Screen_Lines];

GLCD_t __GLCD;

#define __I2C_SLA_W(Address)		(Address<<1)
#define __I2C_SLA_R(Address)		((Address<<1) | (1<<0))
#define __GLCD_GetLine(Y)		(Y / __GLCD_Screen_Line_Height)
#define __GLCD_Min(X, Y)		((X < Y) ? X : Y)
#define __GLCD_AbsDiff(X, Y)		((X > Y) ? (X - Y) : (Y - X))
#define __GLCD_Swap(X, Y)		do { typeof(X) t = X; X = Y; Y = t; } while (0)
#define __GLCD_Byte2ASCII(Value)	(Value = Value + '0')
#define __GLCD_Pointer(X, Y)		(X + ((Y / __GLCD_Screen_Line_Height) *__GLCD_Screen_Width))
//---------------------------//

//----- Prototypes ----------------------------//
static void GLCD_Send(const uint8_t Control, uint8_t *Data, const uint8_t Length);
static void GLCD_BufferWrite(const uint8_t X, const uint8_t Y, const uint8_t Data);
static uint8_t GLCD_BufferRead(const uint8_t X, const uint8_t Y);
//---------------------------------------------//

//----- Functions -------------//
void GLCD_SendCommand(uint8_t Command)
{
	GLCD_Send(0<<__GLCD_DC, &Command, 1);
}

void GLCD_SendData(const uint8_t Data)
{
	GLCD_BufferWrite(__GLCD.X++, __GLCD.Y, Data);
}

void GLCD_Setup(void)
{
	//Reset if needed
	#ifdef defined(GLCD_RST) 
		PinMode(GLCD_RST, Output);
		GLCD_Reset();
	#endif

	//Setup I2C hardware
	__I2C_Setup();

	//Commands needed for initialization
	//-------------------------------------------------------------
	GLCD_SendCommand(__GLCD_Command_Display_Off);				//0xAE
	
	GLCD_SendCommand(__GLCD_Command_Display_Clock_Div_Ratio_Set);		//0xD5
	GLCD_SendCommand(0xF0);							//Suggest ratio
	
	GLCD_SendCommand(__GLCD_Command_Multiplex_Radio_Set);			//0xA8
	GLCD_SendCommand(__GLCD_Screen_Height - 1);
	
	GLCD_SendCommand(__GLCD_Command_Display_Offset_Set);			//0xD3
	GLCD_SendCommand(0x00);							//No offset

	GLCD_SendCommand(__GLCD_Command_Charge_Pump_Set);			//0x8D
	GLCD_SendCommand(0x14);							//Enable charge pump

	GLCD_SendCommand(__GLCD_Command_Display_Start_Line_Set | 0x00);		//0x40 | Start line
	
	GLCD_SendCommand(__GLCD_Command_Memory_Addressing_Set);			//0x20
	GLCD_SendCommand(0x00);							//Horizontal Addressing - Operate like KS0108
	
	GLCD_SendCommand(__GLCD_Command_Segment_Remap_Set | 0x01);		//0xA0 - Left towards Right

	GLCD_SendCommand(__GLCD_Command_Com_Output_Scan_Dec);			//0xC8 - Up towards Down

	GLCD_SendCommand(__GLCD_Command_Com_Pins_Set);				//0xDA
	
	#if (GLCD_Size == GLCD_128_64)
		GLCD_SendCommand(0x12);						//Sequential COM pin configuration
	#elif (GLCD_Size == GLCD_128_32)
		GLCD_SendCommand(0x02);						//Alternative COM pin configuration
	#elif (GLCD_Size == GLCD_96_16)
		GLCD_SendCommand(0x02);						//Alternative COM pin configuration
	#endif
	
	GLCD_SendCommand(__GLCD_Command_Constrast_Set);				//0x81
	GLCD_SendCommand(0xFF);

	GLCD_SendCommand(__GLCD_Command_Precharge_Period_Set);			//0xD9
	GLCD_SendCommand(0xF1);

	GLCD_SendCommand(__GLCD_Command_VCOMH_Deselect_Level_Set);		//0xDB
	GLCD_SendCommand(0x20);

	GLCD_SendCommand(__GLCD_Command_Display_All_On_Resume);			//0xA4
	GLCD_SendCommand(__GLCD_Command_Display_Normal);			//0xA6
	GLCD_SendCommand(__GLCD_Command_Scroll_Deactivate);			//0x2E
	GLCD_SendCommand(__GLCD_Command_Display_On);				//0xAF
	//-------------------------------------------------------------

	//Go to 0,0
	GLCD_GotoXY(0, 0);
	
	//Reset GLCD structure
	__GLCD.Mode = GLCD_Non_Inverted;
	__GLCD.X = __GLCD.Y = __GLCD.Font.Width = __GLCD.Font.Height = __GLCD.Font.Lines = 0;
}

#if #define (GLCD_RST)
	void GLCD_Reset(void)
	{
		DigitalWrite(GLCD_RST, High);
		_delay_ms(_GLCD_Delay_1);
		DigitalWrite(GLCD_RST, Low);
		_delay_ms(_GLCD_Delay_2);
		DigitalWrite(GLCD_RST, High);
	}
#endif

#if defined(GLCD_Error_Checking)
	enum GLCD_Status_t GLCD_GetStatus(void)
	{
		return (__GLCD.Status);
	}
#endif

void GLCD_Render(void)
{
	//We have to send buffer as 16-byte packets
	//Buffer Size:				  Width * Height / Line_Height
	//Packet Size:				  16
	//Loop Counter:				  Buffer size / Packet Size		=
	//							= ((Width * Height) / 8) / 16	=
	//							= (Width / 16) * (Height / 8)	=
	//							= (Width >> 4) * (Height >> 3)
	uint8_t i, loop;
	loop = (__GLCD_Screen_Width>>4) * (__GLCD_Screen_Height>>3);

	//Set columns
	GLCD_SendCommand(__GLCD_Command_Column_Address_Set);			//0x21
	GLCD_SendCommand(0x00);									//Start
	GLCD_SendCommand(__GLCD_Screen_Width - 1);				//End

	//Set rows
	GLCD_SendCommand(__GLCD_Command_Page_Address_Set);			//0x22
	GLCD_SendCommand(0x00);									//Start
	GLCD_SendCommand(__GLCD_Screen_Lines - 1);				//End

	//Send buffer
	for (i = 0 ; i < loop ; i++)
		GLCD_Send(1<<__GLCD_DC, &__GLCD_Buffer[i<<4], 16);
}

void GLCD_SetDisplay(const uint8_t On)
{
	GLCD_SendCommand(On ? __GLCD_Command_Display_On : __GLCD_Command_Display_Off);
}

void GLCD_SetContrast(const uint8_t Contrast)
{
	GLCD_SendCommand(__GLCD_Command_Constrast_Set);
	GLCD_SendCommand(Contrast);
}

void GLCD_Clear(void)
{
	GLCD_FillScreen(GLCD_White);
}

void GLCD_GotoX(const uint8_t X)
{
	if (X < __GLCD_Screen_Width)
		__GLCD.X = X;
}

void GLCD_GotoY(const uint8_t Y)
{
	if (Y < __GLCD_Screen_Height)
		__GLCD.Y = Y;
}

void GLCD_GotoXY(const uint8_t X, const uint8_t Y)
{
	GLCD_GotoX(X);
	GLCD_GotoY(Y);
}


uint8_t GLCD_GetX(void)
{
	return __GLCD.X;
}

uint8_t GLCD_GetY(void)
{
	return __GLCD.Y;
}

uint8_t GLCD_GetLine(void)
{
	return (__GLCD_GetLine(__GLCD.Y));
}

void GLCD_SetPixel(const uint8_t X, const uint8_t Y , enum Color_t Color)
{
	uint8_t data = 0;
	
	//Goto to point
	GLCD_GotoXY(X, Y);

	//Read data
	data = GLCD_BufferRead(__GLCD.X, __GLCD.Y);
	
	//Set pixel
	if (Color == GLCD_Black)
		BitSet(data, Y % 8);
	else
		BitClear(data, Y % 8);
	
	//Sent data
	GLCD_BufferWrite(__GLCD.X, __GLCD.Y, data);
}


void GLCD_FillScreen(enum Color_t Color)
{
	uint8_t i, j;

	for (j = 0 ; j < __GLCD_Screen_Height ; j += __GLCD_Screen_Line_Height)
		for (i = 0 ; i < __GLCD_Screen_Width ; i++)
			GLCD_BufferWrite(i, j, Color);
}


void GLCD_InvertScreen(void)
{
	if (__GLCD.Mode == GLCD_Inverted)
		__GLCD.Mode = GLCD_Non_Inverted;
	else
		__GLCD.Mode = GLCD_Inverted;

	GLCD_SendCommand(__GLCD.Mode);
}

void GLCD_InvertRect(uint8_t X1, uint8_t Y1, uint8_t X2, uint8_t Y2)
{
	uint8_t width, height, offset, mask, h, i, data;

	width = X2 - X1 + 1;
	height = Y2 - Y1 + 1;
	offset = Y1 % __GLCD_Screen_Line_Height;
	Y1 -= offset;
	mask = 0xFF;
	data = 0;

	//Calculate mask for top fractioned region
	if (height < (__GLCD_Screen_Line_Height - offset))
	{
		mask >>= (__GLCD_Screen_Line_Height - height);
		h = height;
	}
	else
	{
		h = __GLCD_Screen_Line_Height - offset;
	}
	mask <<= offset;
	
	//Draw fractional rows at the top of the region
	GLCD_GotoXY(X1, Y1);
	for (i = 0 ; i < width ; i++)
	{
		data = GLCD_BufferRead(__GLCD.X, __GLCD.Y);
		data = ((~data) & mask) | (data & (~mask));
		GLCD_BufferWrite(__GLCD.X++, __GLCD.Y, data);
	}

	//Full rows
	while ((h + __GLCD_Screen_Line_Height) <= height)
	{
		h += __GLCD_Screen_Line_Height;
		Y1 += __GLCD_Screen_Line_Height;
		GLCD_GotoXY(X1, Y1);
		
		for (i=0 ; i < width ; i++)
		{
			data = ~GLCD_BufferRead(__GLCD.X, __GLCD.Y);
			GLCD_BufferWrite(__GLCD.X++, __GLCD.Y, data);
		}
	}

	//Fractional rows at the bottom of the region
	if (h < height)
	{
		mask = ~(0xFF<<(height - h));
		GLCD_GotoXY(X1, (Y1 + __GLCD_Screen_Line_Height));

		for (i = 0 ; i < width ; i++)
		{
			data = GLCD_BufferRead(__GLCD.X, __GLCD.Y);
			data = ((~data) & mask) | (data & (~mask));
			GLCD_BufferWrite(__GLCD.X++, __GLCD.Y, data);
		}
	}
}

void GLCD_SetFont(const uint8_t *Name, const uint8_t Width, const uint8_t Height, enum PrintMode_t Mode)
{
	if ((Width < __GLCD_Screen_Width) && (Height < __GLCD_Screen_Height) && ((Mode == GLCD_Overwrite) || (Mode == GLCD_Merge)))
	{
		//Change font pointer to new font
		__GLCD.Font.Name = (uint8_t *)(Name);
		
		//Update font's size
		__GLCD.Font.Width = Width;
		__GLCD.Font.Height = Height;
		
		//Update lines required for a character to be fully displayed
		__GLCD.Font.Lines = (Height - 1) / __GLCD_Screen_Line_Height + 1;
		
		//Update blending mode
		__GLCD.Font.Mode = Mode;
	}
}

uint8_t GLCD_GetWidthChar(const char Character)
{
	//+1 for space after each character
	return (pgm_read_byte(&(__GLCD.Font.Name[(Character - 32) * (__GLCD.Font.Width * __GLCD.Font.Lines + 1)])) + 1);
}

uint16_t GLCD_GetWidthString(const char *Text)
{
	uint16_t width = 0;

	while (*Text)
		width += GLCD_GetWidthChar(*Text++);

	return width;
}

uint16_t GLCD_GetWidthString_P(const char *Text)
{
	uint16_t width = 0;
	char r = pgm_read_byte(Text++);

	while (r)
	{
		width += GLCD_GetWidthChar(r);
		r = pgm_read_byte(Text++);
	}
	
	return width;
}

void GLCD_PrintChar(char Character)
{
	//If it doesn't work, replace pgm_read_byte with pgm_read_word
	uint16_t fontStart, fontRead, fontReadPrev;
	uint8_t x, y, y2, i, j, width, overflow, data, dataPrev;
	fontStart = fontRead = fontReadPrev = x = y = y2 = i = j = width = overflow = data = dataPrev = 0;
	
	//#1 - Save current position
	x = __GLCD.X;
	y = y2 = __GLCD.Y;
	
	//#2 - Remove leading empty characters
	Character -= 32;														//32 is the ASCII of the first printable character
	
	//#3 - Find the start of the character in the font array
	fontStart = Character * (__GLCD.Font.Width * __GLCD.Font.Lines + 1);		//+1 due to first byte of each array line being the width
	
	//#4 - Update width - First byte of each line is the width of the character
	width = pgm_read_byte(&(__GLCD.Font.Name[fontStart++]));
	
	
	//#5 - Calculate overflowing bits
	overflow = __GLCD.Y % __GLCD_Screen_Line_Height;
	
	//#6 - Print the character
	//Scan the lines needed
	for (j = 0 ; j < __GLCD.Font.Lines ; j++)
	{
		//Go to the start of the line
		GLCD_GotoXY(x, y);
		
		//Update the indices for reading the line
		fontRead = fontStart + j;
		fontReadPrev = fontRead - 1;

		//Scan bytes of selected line
		for (i = 0 ; i < width ; i++)
		{
			//Read byte
			data = pgm_read_byte(&(__GLCD.Font.Name[fontRead]));
			
			//Shift byte
			data <<= overflow;
			
			//Merge byte with previous one
			if (j > 0)
			{
				dataPrev = pgm_read_byte(&(__GLCD.Font.Name[fontReadPrev]));
				dataPrev >>= __GLCD_Screen_Line_Height - overflow;
				data |= dataPrev;
				fontReadPrev += __GLCD.Font.Lines;
			}

			//Edit byte depending on the mode
			if (__GLCD.Font.Mode == GLCD_Merge)
			data |= GLCD_BufferRead(__GLCD.X, __GLCD.Y);
			
			//Send byte
			GLCD_BufferWrite(__GLCD.X++, __GLCD.Y, data);
			
			//Increase index
			fontRead += __GLCD.Font.Lines;
		}

		//Send an empty column of 1px in the end
		if (__GLCD.Font.Mode == GLCD_Overwrite)
			GLCD_BufferWrite(__GLCD.X, __GLCD.Y, GLCD_White);
		
		//Increase line counter
		y += __GLCD_Screen_Line_Height;
	}

	//#7 - Update last line, if needed
	//If (LINE_STARTING != LINE_ENDING)
	if (__GLCD_GetLine(y2) != __GLCD_GetLine((y2 + __GLCD.Font.Height - 1)) && y < __GLCD_Screen_Height)
	{
		//Go to the start of the line
		GLCD_GotoXY(x, y);
		
		//Update the index for reading the last printed line
		fontReadPrev = fontStart + j - 1;

		//Scan bytes of selected line
		for (i = 0 ; i < width ; i++)
		{
			//Read byte
			data = GLCD_BufferRead(__GLCD.X, __GLCD.Y);
			
			//Merge byte with previous one
			dataPrev = pgm_read_byte(&(__GLCD.Font.Name[fontReadPrev]));
			dataPrev >>= __GLCD_Screen_Line_Height - overflow;
			data |= dataPrev;
			
			//Edit byte depending on the mode
			if (__GLCD.Font.Mode == GLCD_Merge)
				data |= GLCD_BufferRead(__GLCD.X, __GLCD.Y);
			
			//Send byte
			GLCD_BufferWrite(__GLCD.X++, __GLCD.Y, data);

			//Increase index
			fontReadPrev += __GLCD.Font.Lines;
		}

		//Send an empty column of 1px in the end
		if (__GLCD.Font.Mode == GLCD_Overwrite)
			GLCD_BufferWrite(__GLCD.X, __GLCD.Y, GLCD_White);
	}
	
	//Move cursor to the end of the printed character
	GLCD_GotoXY(x + width + 1, y2);
}

void GLCD_PrintString(const char *Text)
{
	while(*Text)
	{
		if ((__GLCD.X + __GLCD.Font.Width) >= __GLCD_Screen_Width)
			break;

		GLCD_PrintChar(*Text++);
	}
}


void GLCD_PrintInteger(const int32_t Value)
{
	if (Value == 0)
	{
		GLCD_PrintChar('0');
	}
	else if ((Value > INT32_MIN) && (Value <= INT32_MAX))
	{
		//int32_max_bytes + sign + null = 12 bytes
		char bcd[12] = { '\0' };
		
		//Convert integer to array
		Int2bcd(Value, bcd);
		
		//Print from first non-zero digit
		GLCD_PrintString(bcd);
	}
}


static void GLCD_Send(const uint8_t Control, uint8_t *Data, const uint8_t Length)
{
	uint8_t i;
	#if defined(GLCD_Error_Checking)
		uint8_t status;
	#endif

	do
	{
		//Transmit START signal
		__I2C_Start();

		#if defined(GLCD_Error_Checking)
			status = __I2C_Status();
			if ((status != MT_START_TRANSMITTED) && (status != MT_REP_START_TRANSMITTED))
			{
				__GLCD.Status = GLCD_Error;
				break;
		}
		#endif
		

		//Transmit SLA+W
		__I2C_Transmit(__I2C_SLA_W(__GLCD_I2C_Address));
		#if defined(GLCD_Error_Checking)
			status = __I2C_Status();
			if ((status != MT_SLA_W_TRANSMITTED_ACK) && (status != MT_SLA_W_TRANSMITTED_NACK))
			{
				__GLCD.Status = GLCD_Error;
				break;
			}
		#endif
		

		//Transmit control byte
		__I2C_Transmit(Control);
		#if defined(GLCD_Error_Checking)
			status = __I2C_Status();
			if ((status != MT_DATA_TRANSMITTED_ACK) && (status != MT_DATA_TRANSMITTED_NACK))
			{
				__GLCD.Status = GLCD_Error;
				break;
			}
		#endif
		

		for (i = 0 ; i < Length ; i++)
		{
			//Transmit data
			__I2C_Transmit(Data[i]);
			#if defined(GLCD_Error_Checking)
				status = __I2C_Status();
				if ((status != MT_DATA_TRANSMITTED_ACK) && (status != MT_DATA_TRANSMITTED_NACK))
				{
					__GLCD.Status = GLCD_Error;
					break;
				}
			#endif
			
		}

		#if defined(GLCD_Error_Checking)
			__GLCD.Status = GLCD_Ok;
		#endif
		
	}
	while (0);
	
	//Transmit STOP signal
	__I2C_Stop();
}

static void GLCD_BufferWrite(const uint8_t X, const uint8_t Y, const uint8_t Data)
{
	__GLCD_Buffer[__GLCD_Pointer(X, Y)] = Data;
}

static uint8_t GLCD_BufferRead(const uint8_t X, const uint8_t Y)
{
	//y>>3 = y / 8
	return (__GLCD_Buffer[__GLCD_Pointer(X, Y)]);
}

//-----------------------------//
