// Tetris .c
// Runs on TM4C123
// Haakon Mongstad


// Last Modified: 4/27/2022 


// ******* Possible Hardware I/O connections*******************
// Slide pot pin 1 connected to ground
// Slide pot pin 2 connected to PD2/AIN5
// Slide pot pin 3 connected to +3.3V 
// buttons connected to PE0-PE3
// 32*R resistor DAC bit 0 on PB0 (least significant bit)
// 16*R resistor DAC bit 1 on PB1
// 8*R resistor DAC bit 2 on PB2 
// 4*R resistor DAC bit 3 on PB3
// 2*R resistor DAC bit 4 on PB4
// 1*R resistor DAC bit 5 on PB5 (most significant bit)
// LED on PD1
// LED on PD0


#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "ST7735.h"
#include "Print.h"
#include "Random.h"
#include "TExaS.h"
#include "ADC.h"
#include "Images.h"
#include "Sound.h"
#include "Timer1.h"
#include "Buttons.h"
#include "Timer0A.h"


#define T 0x54
#define E 0x45
#define R 0x52
#define I 0x49
#define S 0x53
#define Z 0x5A

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void Delay100ms(uint32_t count); // time delay in 0.1 seconds

uint8_t col;
uint32_t coldata = 0;
uint8_t row;
uint8_t flag = 0;
uint8_t flag2 = 0;
uint32_t lastCol;
uint8_t *rowpt = &row;
extern uint8_t flag3;
extern uint8_t flag4;

typedef struct {
	int8_t x;
	int8_t y;
} coords;

const coords shapes [7] [4] [4] = {
	{ // T
		{{0, 8}, {8, 0}, {8, 8}, {16, 8}}, //Original
		{{0, 8}, {8, 0}, {8, 8}, {8, 16}}, //90
		{{0, 8}, {16, 8}, {8, 8}, {8, 16}}, //180
		{{8, 0}, {16, 8}, {8, 8}, {8, 16}}, //270
	},
	{ // Square
		{{0, 0}, {8, 0}, {0, 8}, {8, 8}}, //Original
		{{0, 0}, {8, 0}, {0, 8}, {8, 8}}, //90
		{{0, 0}, {8, 0}, {0, 8}, {8, 8}}, //180
		{{0, 0}, {8, 0}, {0, 8}, {8, 8}}, //270
	},
	{ // Line
		{{0, 0}, {8, 0}, {16, 0}, {24, 0}}, //Original
		{{0, 0}, {0, -8}, {0, -16}, {0, -24}}, //90
		{{0, 0}, {8, 0}, {16, 0}, {24, 0}}, //180
		{{0, 0}, {0, -8}, {0, -16}, {0, -24}}, //270
	},
	{ // Left L
		{{0, 0}, {0, 8}, {0, 16}, {8, 16}}, //Original
		{{8, 8}, {0, 8}, {16, 8}, {16, 0}}, //90
		{{0, 0}, {8, 0}, {8, 8}, {8, 16}}, //180
		{{0, 0}, {8, 0}, {0, 8}, {16, 0}}, //270
	},
	{ // Right L
		{{0, 16}, {8, 0}, {8, 8}, {8, 16}}, //Original
		{{0, 0}, {8, 0}, {16, 0}, {16, 8}}, //90
		{{0, 0}, {0, 8}, {0, 16}, {8, 0}}, //180
		{{0, 0}, {0, 8}, {8, 8}, {16, 8}}, //270
	},
	{ // Z
		{{0, 0}, {8, 0}, {8, 8}, {16, 8}}, //Original
		{{0, 8}, {8, 0}, {8, 8}, {0, 16}}, //90
		{{0, 0}, {8, 0}, {8, 8}, {16, 8}}, //180
		{{0, 8}, {8, 0}, {8, 8}, {0, 16}}, //270
	},
	{ // S
		{{8, 0}, {0, 8}, {8, 8}, {16, 0}}, //Original
		{{0, 0}, {0, 8}, {8, 8}, {8, 16}}, //90
		{{8, 0}, {0, 8}, {8, 8}, {16, 0}}, //180
		{{0, 0}, {0, 8}, {8, 8}, {8, 16}}, //270
	}
};

typedef struct {
	int16_t sqr1Row;
	int16_t sqr1Col;
	int16_t sqr2Row;
	int16_t sqr2Col;
	int16_t sqr3Row;
	int16_t sqr3Col;
	int16_t sqr4Row;
	int16_t sqr4Col;
}sqrLocs;

sqrLocs shapeCoords;

int8_t board [16][10] = {
	{0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0}
};
int8_t topRow[10] = {15, 15, 15 ,15, 15, 15, 15, 15, 15, 15};
int8_t shapeLocY[4] = {0,0,0,0};
int8_t shapeLocX[4] = {0,0,0,0};


uint16_t lightBlue; 
uint16_t darkBlue;
uint16_t orange;

sqrLocs shape_Init(const coords coordinates[4], uint8_t column){
	sqrLocs shapeLocations;
	int16_t highRow = 0;
	int16_t highCol = 0;
	
	for (uint8_t i = 0; i<4; i++){
		if (-coordinates[i].y < highRow){
			highRow = -coordinates[i].y;
		}
		if (coordinates[i].x > highCol){
			highCol = coordinates[i].x;
		}
	}
	
	
	int8_t rowDiff = -(highRow/8);
	if (column + (highCol/8) > 9){
	shapeLocations.sqr1Col = (9-(highCol/8) + (coordinates[0].x/8));
	shapeLocations.sqr1Row = -(coordinates[0].y/8) + rowDiff;
	shapeLocations.sqr2Col = (9-(highCol/8) + (coordinates[1].x/8));
	shapeLocations.sqr2Row = -(coordinates[1].y/8) + rowDiff;
	shapeLocations.sqr3Col = (9-(highCol/8) + (coordinates[2].x/8));
	shapeLocations.sqr3Row = -(coordinates[2].y/8) + rowDiff;
	shapeLocations.sqr4Col = (9-(highCol/8) + (coordinates[3].x/8));
	shapeLocations.sqr4Row = -(coordinates[3].y/8) + rowDiff;
	
	}
	
	else{
	
	shapeLocations.sqr1Col = column + (coordinates[0].x/8);
	shapeLocations.sqr1Row = -(coordinates[0].y/8) + rowDiff;
	shapeLocations.sqr2Col = column + (coordinates[1].x/8);
	shapeLocations.sqr2Row = -(coordinates[1].y/8) + rowDiff;
	shapeLocations.sqr3Col = column + (coordinates[2].x/8);
	shapeLocations.sqr3Row = -(coordinates[2].y/8) + rowDiff;
	shapeLocations.sqr4Col = column + (coordinates[3].x/8);
	shapeLocations.sqr4Row = -(coordinates[3].y/8) + rowDiff;
	
	}
	return shapeLocations;
}

uint8_t board_Init(sqrLocs shapeLocations){
	
	if (board[shapeLocations.sqr1Row][shapeLocations.sqr1Col] != 0){
		return 0;
	}
	else if (board[shapeLocations.sqr2Row][shapeLocations.sqr2Col] != 0){
		return 0;
	}
	else if (board[shapeLocations.sqr3Row][shapeLocations.sqr3Col] != 0){
		return 0;
	}
	else if (board[shapeLocations.sqr4Row][shapeLocations.sqr4Col] != 0){
		return 0;
	}
	else {
	board[shapeLocations.sqr1Row][shapeLocations.sqr1Col] = 1;
	board[shapeLocations.sqr2Row][shapeLocations.sqr2Col] = 1;	
	board[shapeLocations.sqr3Row][shapeLocations.sqr3Col] = 1;	
	board[shapeLocations.sqr4Row][shapeLocations.sqr4Col] = 1;
	return 1;
	}
	
}

uint8_t sideShiftBoard(int8_t shapeRow, int8_t shapeCol, int8_t lastShapeCol){
	int8_t colDiff = shapeCol - lastCol;
	int8_t highCol = 0;
	int8_t highColIdx = 0;
	int8_t edge1 = -1;
	int8_t edge2 = -1;
	int8_t edge3 = -1;
	int8_t edge4 = -1;
	int8_t edgeCount = 0;
	int8_t overlap = 0;
	
	int8_t lastColList[4] = {shapeCoords.sqr1Col, shapeCoords.sqr2Col, shapeCoords.sqr3Col, shapeCoords.sqr4Col};
	
	int8_t colList[4] = {shapeCoords.sqr1Col + colDiff, shapeCoords.sqr2Col + colDiff, 
											 shapeCoords.sqr3Col + colDiff, shapeCoords.sqr4Col + colDiff};
	
	int8_t rowList[4] = {shapeCoords.sqr1Row, shapeCoords.sqr2Row, 
											 shapeCoords.sqr3Row, shapeCoords.sqr4Row};
	

	
	for (uint8_t i; i < 4; i++){
		if (colList[i] > highCol){
			highCol = colList[i];
			highColIdx = i;
		}
	}
	
	if (highCol > 9){
		for(uint8_t i = 0; i < 4; i++){
			if (colList[i] == highCol) {
				for (uint8_t j= 0; j < 4; j ++){
					if (i != j) {
					if (colDiff < 0){ //moving left
					if((colList[j] - colList[i]) >= colDiff){ 
						overlap = 1;
					}
					}
				}
				}
				if (overlap == 0) {
					if (board[rowList[i]][9] != 0){
						return 0;
				}
			}
			}
			else { overlap = 0;
				//int8_t colIdx = 9- (highCol-colList[i]);		
				for (uint8_t j= 0; j < 4; j ++){
					if (colDiff < 0){ //moving left
						if ((colList[j] - colList[i]) >= colDiff){
							overlap = 1;
						}
					}
					else{ //moving right
						if ((colList[i] - colList[j] <= colDiff)){
							overlap = 1;
						}
					}
				}
			if (overlap == 0){
				if (board[rowList[i]][9- (highCol-colList[i])] != 0){
					return 0;
				}
			}
			}
			}
			board[shapeCoords.sqr1Row][shapeCoords.sqr1Col] = 0;
			board[shapeCoords.sqr2Row][shapeCoords.sqr2Col] = 0;
			board[shapeCoords.sqr3Row][shapeCoords.sqr3Col] = 0;
			board[shapeCoords.sqr4Row][shapeCoords.sqr4Col] = 0;

			shapeCoords.sqr1Col = (0 == highColIdx)?9:9 - (highCol - colList[0]);
			shapeCoords.sqr2Col = (1 == highColIdx)?9:9 - (highCol - colList[1]);
			shapeCoords.sqr3Col = (2 == highColIdx)?9:9 - (highCol - colList[2]);
			shapeCoords.sqr4Col = (3 == highColIdx)?9:9 - (highCol - colList[3]);
			
			board[shapeCoords.sqr1Row][shapeCoords.sqr1Col] = 1;
			board[shapeCoords.sqr2Row][shapeCoords.sqr2Col] = 1;
			board[shapeCoords.sqr3Row][shapeCoords.sqr3Col] = 1;
			board[shapeCoords.sqr4Row][shapeCoords.sqr4Col] = 1;
			
			return 1;
			
	}
	
	else{
		for(uint8_t i = 0; i < 4; i++){ overlap = 0;
			for(uint8_t j = 0; j < 4; j++){
				if (colDiff < 0) { //moving left
					if ((colList[j] - colList[i]) >= colDiff){
					overlap = 1;
					}
				}
				else{ //moving right
					if ((colList[i] - colList[j]) <= colDiff){
						overlap = 1;
					}
				}
			}
			if (overlap == 0){
			if (board[rowList[i]][colList[i]] != 0){
				return 0;
			}
		}
		}
		board[shapeCoords.sqr1Row][shapeCoords.sqr1Col] = 0;
		board[shapeCoords.sqr2Row][shapeCoords.sqr2Col] = 0;
		board[shapeCoords.sqr3Row][shapeCoords.sqr3Col] = 0;
		board[shapeCoords.sqr4Row][shapeCoords.sqr4Col] = 0;
		
		shapeCoords.sqr1Col = colList[0];
		shapeCoords.sqr2Col = colList[1];
		shapeCoords.sqr3Col = colList[2];
		shapeCoords.sqr4Col = colList[3];
		
		
		board[shapeCoords.sqr1Row][shapeCoords.sqr1Col] = 1;
		board[shapeCoords.sqr2Row][shapeCoords.sqr2Col] = 1;
		board[shapeCoords.sqr3Row][shapeCoords.sqr3Col] = 1;
		board[shapeCoords.sqr4Row][shapeCoords.sqr4Col] = 1;
		
		return 1;
		

		}
		
	}

int8_t verticalShiftBoard(){
	int8_t rowList[4] = {shapeCoords.sqr1Row, shapeCoords.sqr2Row, 
											shapeCoords.sqr3Row, shapeCoords.sqr4Row};
	int8_t colList[4] = {shapeCoords.sqr1Col, shapeCoords.sqr2Col, 
											shapeCoords.sqr3Col, shapeCoords.sqr4Col};
	int8_t stacked = 1;
	
	for (uint8_t i = 0; i < 4; i ++){
		if(rowList[i] + 1 > 15){
			return 0;
		}
		else{
		for (uint8_t j = 0; j < 4; j ++){
			if (j != i) {
				if (((rowList[i] +1) != rowList[j]) || ((colList[i] != colList[j]) && ((rowList[i] +1) == rowList[j]))){
					stacked = 0;
				}
				else{
					stacked = 1;
					break;
				}
			}
		}
		if (stacked == 0){
			if (board[rowList[i] + 1][colList[i]] != 0) {
				return 0;
			}
			stacked = 1;
		}
		}

	}
	
	board[shapeCoords.sqr1Row][shapeCoords.sqr1Col] = 0;
	board[shapeCoords.sqr2Row][shapeCoords.sqr2Col] = 0;
	board[shapeCoords.sqr3Row][shapeCoords.sqr3Col] = 0;
	board[shapeCoords.sqr4Row][shapeCoords.sqr4Col] = 0;
	
	shapeCoords.sqr1Row = shapeCoords.sqr1Row + 1;
	shapeCoords.sqr2Row = shapeCoords.sqr2Row + 1;
	shapeCoords.sqr3Row = shapeCoords.sqr3Row + 1;
	shapeCoords.sqr4Row = shapeCoords.sqr4Row + 1;
	
	board[shapeCoords.sqr1Row][shapeCoords.sqr1Col] = 1;
	board[shapeCoords.sqr2Row][shapeCoords.sqr2Col] = 1;
	board[shapeCoords.sqr3Row][shapeCoords.sqr3Col] = 1;
	board[shapeCoords.sqr4Row][shapeCoords.sqr4Col] = 1;

	return 1;
}

uint8_t rotateUpdateBoard(const coords oldCoordinates[4], const coords newCoordinates[4]){
	int8_t newRow[4];
	int8_t newCol[4];
	int8_t oldRow[4] = {shapeCoords.sqr1Row, shapeCoords.sqr2Row, 
											shapeCoords.sqr3Row, shapeCoords.sqr4Row};
	int8_t oldCol[4] = {shapeCoords.sqr1Col, shapeCoords.sqr2Col, 
											shapeCoords.sqr3Col, shapeCoords.sqr4Col};
	
	for (uint8_t i = 0; i < 4; i ++){
		if ((((newCoordinates[i].x/8) - (oldCoordinates[i].x/8)) + oldCol[i]) > 9 || (((newCoordinates[i].x/8) - (oldCoordinates[i].x/8)) + oldCol[i]) < 0){
			return 0;
	}
		else {
			newCol[i] = ((newCoordinates[i].x/8) - (oldCoordinates[i].x/8)) + oldCol[i];
		}
		if ((((newCoordinates[i].y/8) - (oldCoordinates[i].y/8)) + oldRow[i]) > 15 || (((newCoordinates[i].y/8) - (oldCoordinates[i].y/8)) + oldRow[i]) < 0){
			return 0;
		}
		else{
			newRow[i] = ((newCoordinates[i].y/8) - (oldCoordinates[i].y/8)) + oldRow[i];
		}
	}

	
	board[shapeCoords.sqr1Row][shapeCoords.sqr1Col] = 0;
	board[shapeCoords.sqr2Row][shapeCoords.sqr2Col] = 0;
	board[shapeCoords.sqr3Row][shapeCoords.sqr3Col] = 0;
	board[shapeCoords.sqr4Row][shapeCoords.sqr4Col] = 0;
	
	
		for (uint8_t i = 0; i < 4; i ++){
		if (board[newRow[i]][newCol[i]] != 0){
			board[shapeCoords.sqr1Row][shapeCoords.sqr1Col] = 1;
			board[shapeCoords.sqr2Row][shapeCoords.sqr2Col] = 1;
			board[shapeCoords.sqr3Row][shapeCoords.sqr3Col] = 1;
			board[shapeCoords.sqr4Row][shapeCoords.sqr4Col] = 1;
			return 0;
		}
	}
	
	shapeCoords.sqr1Row = newRow[0];
	shapeCoords.sqr2Row = newRow[1];
	shapeCoords.sqr3Row = newRow[2];
	shapeCoords.sqr4Row = newRow[3];
	
	shapeCoords.sqr1Col = newCol[0];
	shapeCoords.sqr2Col = newCol[1];
	shapeCoords.sqr3Col = newCol[2];
	shapeCoords.sqr4Col = newCol[3];
	
	board[shapeCoords.sqr1Row][shapeCoords.sqr1Col] = 1;
	board[shapeCoords.sqr2Row][shapeCoords.sqr2Col] = 1;
	board[shapeCoords.sqr3Row][shapeCoords.sqr3Col] = 1;
	board[shapeCoords.sqr4Row][shapeCoords.sqr4Col] = 1;
	
	return 1;
	
	
	
}

	
uint8_t checkRowClear(void){  // returns index of cleared row, or -1 if none
	for (uint8_t i = 15; i >= 0; i --){
		for (uint8_t j = 0; j < 10; j ++){
			if (board[i][j] == 0){
				break;
			}
			else if ((j == 9) && board[i][j] == 1){
				return i;
			}
		}
	}
	return -1;
}

uint16_t calculateScore(void){
	uint16_t count = 0;
	for (uint8_t i = 0; i < 16; i++){
		for (uint8_t j = 0; j < 10; j ++){
			if (board[i][j] == 1){
				count++;
			}
		}
	}
	return count;
}

	
	




void Drop_Init(void){
	NVIC_ST_CTRL_R = 0;               // disable SysTick during setup
  NVIC_ST_CTRL_R = 0x07;      // enable SysTick with core clock
  
}

void Drop_Speed(uint32_t period){
		NVIC_ST_CTRL_R = 0;               // disable SysTick during setup
		NVIC_ST_RELOAD_R = period-1;
		NVIC_ST_CTRL_R = 0x07;      // enable SysTick with core clock

}

void SysTick_Handler(void){
	flag2 = 1;
}



void Timer1A_Handler(void){ // can be used to perform tasks in background
  TIMER1_ICR_R = TIMER_ICR_TATOCINT;// acknowledge TIMER1A timeout
   // execute user task
	uint32_t newdata = ADC_In();
	if (newdata != coldata) {
		lastCol = coldata;
		coldata = newdata;
		flag = 1;
	}
}

uint32_t convertCol(uint32_t data){
	return ((data * 10)/ 4096);
}

int main1(void){
  DisableInterrupts();
  TExaS_Init(NONE);       // Bus clock is 80 MHz 
  Random_Init(1);

  Output_Init();
  ST7735_FillScreen(0x0000);            // set screen to black
  
  ST7735_DrawBitmap(22, 159, PlayerShip0, 18,8); // player ship bottom
  ST7735_DrawBitmap(53, 151, Bunker0, 18,5);
  ST7735_DrawBitmap(42, 159, PlayerShip1, 18,8); // player ship bottom
  ST7735_DrawBitmap(62, 159, PlayerShip2, 18,8); // player ship bottom
  ST7735_DrawBitmap(82, 159, PlayerShip3, 18,8); // player ship bottom

  ST7735_DrawBitmap(0, 9, SmallEnemy10pointA, 16,10);
  ST7735_DrawBitmap(20,9, SmallEnemy10pointB, 16,10);
  ST7735_DrawBitmap(40, 9, SmallEnemy20pointA, 16,10);
  ST7735_DrawBitmap(60, 9, SmallEnemy20pointB, 16,10);
  ST7735_DrawBitmap(80, 9, SmallEnemy30pointA, 16,10);
  ST7735_DrawBitmap(100, 9, SmallEnemy30pointB, 16,10);

  Delay100ms(50);              // delay 5 sec at 80 MHz

  ST7735_FillScreen(0x0000);   // set screen to black
  ST7735_SetCursor(1, 1);
  ST7735_OutString("GAME OVER");
  ST7735_SetCursor(1, 2);
  ST7735_OutString("Nice try,");
  ST7735_SetCursor(1, 3);
  ST7735_OutString("Earthling!");
  ST7735_SetCursor(2, 4);
  LCD_OutDec(1234);
  while(1){
  }

}


// You can't use this timer, it is here for starter code only 
// you must use interrupts to perform delays
void Delay100ms(uint32_t count){uint32_t volatile time;
  while(count>0){
    time = 727240;  // 0.1sec at 80 MHz
    while(time){
      time--;
    }
    count--;
  }
}
typedef enum {English, Spanish, Portuguese, French} Language_t;
Language_t myLanguage=English;
typedef enum {HELLO, GOODBYE, LANGUAGE} phrase_t;
const char GameOver_English[] = "Game Over";
const char GameOver_Spanish[] = "Fin Del Juego";
const char Score_English[] = "Score:";
	const char Score_Spanish[] = "Puntos:";
const char Hold_English[] = "HOLD";
const char Hold_Spanish[] = "GUARDAR";
const char Start_English[] = "Start";
const char Start_Spanish[] = "Comienzo";
const char Hello_English[] ="Hello";
const char Hello_Spanish[] ="\xADHola!";
const char Hello_Portuguese[] = "Ol\xA0";
const char Hello_French[] ="All\x83";
const char Goodbye_English[]="Goodbye";
const char Goodbye_Spanish[]="Adi\xA2s";
const char Goodbye_Portuguese[] = "Tchau";
const char Goodbye_French[] = "Au revoir";
const char Language_English[]="English";
const char Language_Spanish[]="Espa\xA4ol";
const char Language_Portuguese[]="Portugu\x88s";
const char Language_French[]="Fran\x87" "ais";

const char *Phrases[3][4]={
  {Hello_English,Hello_Spanish,Hello_Portuguese,Hello_French},
  {Goodbye_English,Goodbye_Spanish,Goodbye_Portuguese,Goodbye_French},
  {Language_English,Language_Spanish,Language_Portuguese,Language_French}
};

const char *welcomePhrases[5][2]={
	{Start_English, Start_Spanish},
	{Language_English, Language_Spanish},
	{Hold_English, Hold_Spanish},
	{GameOver_English, GameOver_Spanish},
	{Score_English, Score_Spanish}
};

int16_t drawShape(uint8_t drawcol, uint8_t drawrow, uint8_t shape, uint8_t dir, uint16_t color){  //col is 8x8 pixels, row is 1 pixel
/*
				uint8_t maxcol = 0;
				int16_t maxrow = 24;
			for (uint8_t i = 0; i < 4; i++){
			if (((-(shapes[shape][0][i].y) + 24) + (8*drawrow)) <=  maxrow){
				maxrow = ((shapes[shape][0][i].y + 24) + (8*drawrow));
			}
			if (((shapes[shape][0][i].x + 8) + (8*drawcol)) >=  maxcol){
				maxcol = ((shapes[shape][0][i].x + 8) + (8*drawcol));
			}
		}
	*/
			uint8_t maxcol = 0;
			int16_t maxpix = -8;
			int16_t minpix = 24;
			int16_t offset;
			int16_t minrow;
			int16_t maxrow;
			
			for (uint8_t i = 0; i < 4; i++){
				if (-(shapes[shape][dir][i].y) >=  maxpix){
					maxpix = -shapes[shape][dir][i].y;
				}
				if (-shapes[shape][dir][i].y <= minpix){
					minpix = -shapes[shape][dir][i].y;
				}
				if (((shapes[shape][dir][i].x + 8) + (8*drawcol)) >=  maxcol){
					maxcol = ((shapes[shape][dir][i].x + 8) + (8*drawcol));
				}
			}
			offset = 24 - minpix;
			maxrow = (maxpix + offset) + (8*drawrow);
			minrow = (minpix + offset) + (8*drawrow);
			
		if ((maxcol > 80) && (maxrow > 144)){
			for (uint8_t i = 0; i < 4; i++){
			ST7735_FillRect((((shapes[shape][dir][i].x + 8) + (8*drawcol)) == maxcol)?80:(80) - (maxcol - ((shapes[shape][dir][i].x + 8) + (8*drawcol))),
			(((-shapes[shape][dir][i].y + offset) + (8*drawrow)) == maxrow)?144:(144) - (maxrow - ((-shapes[shape][dir][i].y + offset) + (8*drawrow))), 
				8, 8, color);
			}
			return minrow;
		}
		else if (maxcol > 80){
					for (uint8_t i = 0; i < 4; i++){
			ST7735_FillRect((((shapes[shape][dir][i].x + 8) + (8*drawcol)) == maxcol)?80:(80) - (maxcol - ((shapes[shape][dir][i].x + 8) + (8*drawcol))),
			(-shapes[shape][dir][i].y + offset) + (8*drawrow), 8, 8, color);
	}
}
		
		else if (maxrow > 144){
				for (uint8_t i = 0; i < 4; i++){
			ST7735_FillRect((shapes[shape][dir][i].x + 8) + (8*drawcol),
					(((-shapes[shape][dir][i].y + offset) + (8*drawrow)) == maxrow)?144:(144) - (maxrow - ((-shapes[shape][dir][i].y + offset) + (8*drawrow))),
					8, 8, color);
	}
			
		}
		else{
		for (uint8_t i = 0; i < 4; i++){
			ST7735_FillRect((shapes[shape][dir][i].x + 8) + (8*drawcol),
			(-shapes[shape][dir][i].y + offset) + (8*drawrow), 8, 8, color);
	}
}

		
	
}



void eraseShape(uint8_t drawcol, uint8_t drawrow, uint8_t shape, uint8_t dir){
			uint8_t maxcol = 0;
			int16_t maxpix = -8;
			int16_t minpix = 24;
			int16_t offset;
			int16_t minrow;
			int16_t maxrow;
			
			for (uint8_t i = 0; i < 4; i++){
				if (-(shapes[shape][dir][i].y) >=  maxpix){
					maxpix = -shapes[shape][dir][i].y;
				}
				if (-shapes[shape][dir][i].y <= minpix){
					minpix = -shapes[shape][dir][i].y;
				}
				if (((shapes[shape][dir][i].x + 8) + (8*drawcol)) >=  maxcol){
					maxcol = ((shapes[shape][dir][i].x + 8) + (8*drawcol));
				}
			}
			offset = 24 - minpix;
			maxrow = (maxpix + offset) + (8*drawrow);
			minrow = (minpix + offset) + (8*drawrow);
			
			
		if ((maxcol > 80) && (maxrow > 144)){
			for (uint8_t i = 0; i < 4; i++){
			ST7735_FillRect((((shapes[shape][0][dir].x + 8) + (8*drawcol)) == maxcol)?80:(80) - (maxcol - ((shapes[shape][dir][i].x + 8) + (8*drawcol))),
			(((-shapes[shape][dir][i].y + offset) + (8*drawrow)) == maxrow)?144:(144) - (maxrow - ((-shapes[shape][dir][i].y + offset) + (8*drawrow))), 
				8, 8, ST7735_BLACK);
			}
		}
		else if (maxcol > 80){
					for (uint8_t i = 0; i < 4; i++){
			ST7735_FillRect((((shapes[shape][dir][i].x + 8) + (8*drawcol)) == maxcol)?80:(80) - (maxcol- ((shapes[shape][dir][i].x + 8) + (8*drawcol))),
			(-shapes[shape][dir][i].y + offset) + (8*drawrow), 8, 8, ST7735_BLACK);
	}
}
		
		else if (maxrow > 144){
				for (uint8_t i = 0; i < 4; i++){
			ST7735_FillRect((shapes[shape][dir][i].x + 8) + (8*drawcol),
					(((-shapes[shape][dir][i].y + offset) + (8*drawrow)) == maxrow)?144:(144) - (maxrow - ((-shapes[shape][dir][i].y + offset) + (8*drawrow))),
					8, 8, ST7735_BLACK);
	}
			
		}
		else{
		for (uint8_t i = 0; i < 4; i++){
			ST7735_FillRect((shapes[shape][dir][i].x + 8) + (8*drawcol),
			(-shapes[shape][dir][i].y + offset) + (8*drawrow), 8, 8, ST7735_BLACK);
	}
}
}

uint8_t language;

void welcomeScreen(void){
	language = 0;
	
  ST7735_FillScreen(0x0000);            // set screen to black
	ST7735_DrawCharS(4, 20, T, ST7735_RED, ST7735_BLACK, 4);
	ST7735_DrawCharS(24, 20, E, ST7735_Color565(255, 165, 0), ST7735_BLACK, 4);
	ST7735_DrawCharS(44, 20, T, ST7735_YELLOW, ST7735_BLACK, 4);
	ST7735_DrawCharS(64, 20, R, ST7735_GREEN, ST7735_BLACK, 4);
	ST7735_DrawCharS(84, 20, I, ST7735_Color565(0, 204, 204), ST7735_BLACK, 4);
	ST7735_DrawCharS(104, 20, Z, ST7735_MAGENTA, ST7735_BLACK, 4);
	ST7735_DrawString(8, 7, (char *) welcomePhrases[0][language],ST7735_WHITE);
	ST7735_DrawString(7, 9, (char *) welcomePhrases[1][0], ST7735_WHITE);
	ST7735_DrawString(7, 11, (char *) welcomePhrases[1][1], ST7735_WHITE);
	for (uint8_t i = 0; i < 4; i++){
		ST7735_FillRect(shapes[0][0][i].x + 10, (shapes[0][0][i].y) + 70, 8, 8, ST7735_MAGENTA);
	}
	for (uint8_t i = 0; i < 4; i++){
		ST7735_FillRect(shapes[1][0][i].x + 10, (shapes[1][0][i].y) + 100, 8, 8, ST7735_YELLOW);
	}
	for (uint8_t i = 0; i < 4; i++){
		ST7735_FillRect(shapes[2][0][i].x + 10, (shapes[2][0][i].y) + 140, 8, 8, ST7735_Color565(0, 204, 204));
	}
	for (uint8_t i = 0; i < 4; i++){
		ST7735_FillRect(shapes[3][0][i].x + 64, (shapes[3][0][i].y) + 130, 8, 8, ST7735_Color565(255, 165, 0));
	}
	for (uint8_t i = 0; i < 4; i++){
		ST7735_FillRect(shapes[4][0][i].x + 100, (shapes[4][0][i].y) + 130, 8, 8, ST7735_Color565(51, 153, 255));
	}
	for (uint8_t i = 0; i < 4; i++){
		ST7735_FillRect(shapes[5][0][i].x + 100, (shapes[5][0][i].y) + 100, 8, 8, ST7735_RED);
	}
	for (uint8_t i = 0; i < 4; i++){
		ST7735_FillRect(shapes[6][0][i].x + 100, (shapes[6][0][i].y) + 70, 8, 8, ST7735_GREEN);
	}
	
	while ((GPIO_PORTE_DATA_R & 0x01) == 0){
		
		while (language == 0){				
			if ((GPIO_PORTE_DATA_R & 0x01) == 1){ break;}
			else if ((GPIO_PORTE_DATA_R & 0x04) != 0) {
		
				language = 1;
				ST7735_FillRect(60, 56, 40, 25, ST7735_BLACK);
				ST7735_DrawString(7, 7, (char *) welcomePhrases[0][language],ST7735_WHITE);
			}
			}
		
		while (language == 1){
				if ((GPIO_PORTE_DATA_R & 0x01) == 1){ break;}
				else if ((GPIO_PORTE_DATA_R & 0x02) != 0) {
				language = 0;
				ST7735_FillRect(42, 56, 50, 25, ST7735_BLACK);
				ST7735_DrawString(8, 7, (char *) welcomePhrases[0][language],ST7735_WHITE);
		
				}
	}
}


}


int main(void){ char l;
  DisableInterrupts();
  TExaS_Init(NONE);       // Bus clock is 80 MHz 
  Output_Init();
	buttonInit();
	ADC_Init();
	Random_Init(ADC_In());
	Sound_Init();
	EdgeCounter_Init();
	EnableInterrupts();
	
  ST7735_FillScreen(0x0000);            // set screen to black
	welcomeScreen();
  ST7735_FillScreen(ST7735_Color565(153, 51, 255));            // set screen to purple
	ST7735_FillRect(8, 24, 80, 128, ST7735_BLACK);
	ST7735_DrawCharS(18, 8, T, ST7735_RED, ST7735_Color565(153, 51, 255), 2);
	ST7735_DrawCharS(28, 8, E, ST7735_Color565(255, 165, 0), ST7735_Color565(153, 51, 255), 2);
	ST7735_DrawCharS(38, 8, T, ST7735_YELLOW, ST7735_Color565(153, 51, 255), 2);
	ST7735_DrawCharS(48, 8, R, ST7735_GREEN, ST7735_Color565(153, 51, 255), 2);
	ST7735_DrawCharS(58, 8, I, ST7735_Color565(0, 204, 204), ST7735_Color565(153, 51, 255), 2);
	ST7735_DrawCharS(68, 8, Z, ST7735_MAGENTA, ST7735_Color565(153, 51, 255), 2);
	if (language == 0) {
		ST7735_DrawString(15, 4, (char *) welcomePhrases[2][language], ST7735_WHITE);
		ST7735_FillRect(96, 54, 24,32, ST7735_BLACK);
	}
	else {
		ST7735_DrawString(15, 4, (char *) welcomePhrases[2][language], ST7735_WHITE);
		ST7735_FillRect(96, 54, 24,32, ST7735_BLACK);

	}
	Timer1_Init(8000000, 2);
	//ableInterrupts();
	
	lightBlue = ST7735_Color565(0, 204, 204);
	darkBlue = ST7735_Color565(255, 165, 0);
	orange = ST7735_Color565(51, 153, 255);

	uint16_t shapeColor[7] = {ST7735_MAGENTA, ST7735_YELLOW, lightBlue, darkBlue,
	orange, ST7735_RED, ST7735_GREEN};
	col = 0;
	int8_t clearedRow;
	flag3 = 0;
	flag4 = 0;
	uint8_t holdShape;
	uint8_t tempHoldShape;
	uint8_t justStoppedHolding = 0;
	uint8_t holding = 0; // 1 if already holding somehing, 0 if not. 
	//Drop_Init();
	Drop_Speed(999999999);
	//Drop_Speed(4000000000);
	uint8_t shape = (Random()) % 7;
	while(1){
	row = 0;
	if (justStoppedHolding == 0){
		shape = (Random()) % 7;
	}
	justStoppedHolding = 0;
	uint8_t dir = 0;
	shapeCoords = shape_Init(shapes[shape][dir], col);
	if (board_Init(shapeCoords) == 0){
		Sound_Drop();
		break;
	}
	drawShape(col, row, shape, dir, shapeColor[shape]);
	
		


		while (1)
		
		{
			
		if (flag2 == 1){ //move down
			//if (row < 15){
			if (verticalShiftBoard() == 0){
				flag2 = 0;
				break;
			}
			row ++;
			eraseShape(col, row-1, shape, dir);
			drawShape(col, row, shape, dir, shapeColor[shape]);
			//}
			flag2 = 0;
	}
		
		
		if (flag == 1) { //move left or right
			col = convertCol(coldata);
			lastCol = convertCol(lastCol);
			if (col != lastCol) {
				
			if (sideShiftBoard(row, col, lastCol) == 1) {
				eraseShape(lastCol, row, shape, dir);
				drawShape(col, row, shape, dir, shapeColor[shape]);
			}
			}
			flag = 0;
		}
		
		if (flag3 == 1){ // rotate shape

			if ((rotateUpdateBoard(shapes[shape][dir], shapes[shape][(dir+1)%4])) == 1){
				eraseShape(lastCol, row, shape, dir);
				dir = (dir + 1) % 4;
				drawShape(col, row, shape, dir, shapeColor[shape]);
			}
			
			flag3 = 0;
		}
		
		if (flag4 == 1) {
			if (holding == 0){
				holdShape = shape;
				board[shapeCoords.sqr1Row][shapeCoords.sqr1Col] = 0;
				board[shapeCoords.sqr2Row][shapeCoords.sqr2Col] = 0;
				board[shapeCoords.sqr3Row][shapeCoords.sqr3Col] = 0;
				board[shapeCoords.sqr4Row][shapeCoords.sqr4Col] = 0;
				eraseShape(lastCol, row, shape, dir);
			if (shape == 0){
				for (uint8_t i = 0; i < 4; i++){
					ST7735_FillRect(shapes[shape][0][i].x + 96, (shapes[shape][0][i].y) + 62, 8, 8, ST7735_MAGENTA);
				}
			}
			else if (shape == 1) {
				for (uint8_t i = 0; i < 4; i++){
					ST7735_FillRect(shapes[shape][0][i].x + 100, (shapes[shape][0][i].y) + 62, 8, 8, ST7735_YELLOW);
				}
			}
			else if (shape == 2) {
				for (uint8_t i = 0; i < 4; i++){
					ST7735_FillRect(shapes[shape][1][i].x + 104, (shapes[shape][1][i].y) + 78, 8, 8, ST7735_Color565(0, 204, 204));
				}
			}
			else if (shape == 3) {
				for (uint8_t i = 0; i < 4; i++){
					ST7735_FillRect(shapes[shape][0][i].x + 100, (shapes[shape][0][i].y) + 62, 8, 8, ST7735_Color565(255, 165, 0));
				}
			}
			else if (shape == 4) {
				for (uint8_t i = 0; i < 4; i++){
					ST7735_FillRect(shapes[shape][0][i].x + 100, (shapes[shape][0][i].y) + 62, 8, 8, ST7735_Color565(51, 153, 255));
				}
			}
			else if (shape == 5) {
				for (uint8_t i = 0; i < 4; i++){
					ST7735_FillRect(shapes[shape][0][i].x + 96, (shapes[shape][0][i].y) + 62, 8, 8, ST7735_RED);
				}	
			}
			else{
				for (uint8_t i = 0; i < 4; i++){
					ST7735_FillRect(shapes[shape][0][i].x + 96, (shapes[shape][0][i].y) + 62, 8, 8, ST7735_GREEN);
				}	
			}
			holding = 1;
			flag4 = 0;
			break;
				
		}
		else{
			board[shapeCoords.sqr1Row][shapeCoords.sqr1Col] = 0;
			board[shapeCoords.sqr2Row][shapeCoords.sqr2Col] = 0;
			board[shapeCoords.sqr3Row][shapeCoords.sqr3Col] = 0;
			board[shapeCoords.sqr4Row][shapeCoords.sqr4Col] = 0;
			eraseShape(lastCol, row, shape, dir);
			ST7735_FillRect(96, 54, 24,32, ST7735_BLACK);

			if (shape == 0){
				for (uint8_t i = 0; i < 4; i++){
					ST7735_FillRect(shapes[shape][0][i].x + 96, (shapes[shape][0][i].y) + 62, 8, 8, ST7735_MAGENTA);
				}
			}
			else if (shape == 1) {
				for (uint8_t i = 0; i < 4; i++){
					ST7735_FillRect(shapes[shape][0][i].x + 100, (shapes[shape][0][i].y) + 62, 8, 8, ST7735_YELLOW);
				}
			}
			else if (shape == 2) {
				for (uint8_t i = 0; i < 4; i++){
					ST7735_FillRect(shapes[shape][1][i].x + 104, (shapes[shape][1][i].y) + 78, 8, 8, ST7735_Color565(0, 204, 204));
				}
			}
			else if (shape == 3) {
				for (uint8_t i = 0; i < 4; i++){
					ST7735_FillRect(shapes[shape][0][i].x + 100, (shapes[shape][0][i].y) + 62, 8, 8, ST7735_Color565(255, 165, 0));
				}
			}
			else if (shape == 4) {
				for (uint8_t i = 0; i < 4; i++){
					ST7735_FillRect(shapes[shape][0][i].x + 100, (shapes[shape][0][i].y) + 62, 8, 8, ST7735_Color565(51, 153, 255));
				}
			}
			else if (shape == 5) {
				for (uint8_t i = 0; i < 4; i++){
					ST7735_FillRect(shapes[shape][0][i].x + 96, (shapes[shape][0][i].y) + 62, 8, 8, ST7735_RED);
				}	
			}
			else{
				for (uint8_t i = 0; i < 4; i++){
					ST7735_FillRect(shapes[shape][0][i].x + 96, (shapes[shape][0][i].y) + 62, 8, 8, ST7735_GREEN);
				}	
			}
			tempHoldShape = holdShape;
			holdShape = shape;
			shape = tempHoldShape;
			justStoppedHolding = 1;
			flag4 = 0;
			break;
			
			
			
			
		}

		
}


}
}	
ST7735_FillScreen(ST7735_Color565(153, 51, 255));
uint8_t x = (language == 0)?6:4;
uint8_t x2 = (language == 0)?6:6;
ST7735_DrawString(x, 4, (char *) welcomePhrases[3][language], ST7735_WHITE);
ST7735_SetCursor(x2, 8);
ST7735_OutString((char *) welcomePhrases[4][language]);
uint16_t score = calculateScore();
LCD_OutDec(score);






while(1){}
		

}

