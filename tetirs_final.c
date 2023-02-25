#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "system.h"
#include "altera_avalon_spi.h"
#include "altera_avalon_pio_regs.h"
#include "altera_avalon_spi_regs.h"

const int REFRESH_RATE = 350;
const int GAME_SPEED   = 500;

int timer, points, btnValue, gameOver, randSeed, isSquare, btnDelay, numOfFixedLeds; //Global variables

struct led {
  int col;
  int row;
};

struct figure {
  struct led led1;
  struct led led2;
  struct led led3;
  struct led led4;
};

struct led fixedLeds[96];
struct figure movingShape;

struct figure shapes[7] = {
		{{4,2},  {4,1}, {5,2}, {5,1}}, 	// O
		{{3,1 }, {4,1}, {5,1}, {6,1}}, 	// I
		{{3,2 }, {4,2}, {4,1}, {5,2}}, 	// T
		{{3,2 }, {4,2}, {4,1}, {5,1}}, 	// Z
		{{3,1 }, {4,1}, {4,2}, {5,2}}, 	// Z2
		{{3,2 }, {4,2}, {5,1}, {4,1}}, 	// L
		{{3,2 }, {4,2}, {5,2}, {5,1}}  	// L2
};

struct figure generateNewRandomShape(){
	isSquare=0;
	int randomFigure = rand() % 7;
	if (randomFigure == 0)
			isSquare=1;

	return shapes[randomFigure];
}

void clearMatrix(){
	IOWR_ALTERA_AVALON_PIO_DATA(PIO_0_BASE, 0x00);
	IOWR_ALTERA_AVALON_PIO_DATA(PIO_1_BASE, 0x000);
	usleep(15);
}

void lightLed(struct led position){
	IOWR_ALTERA_AVALON_PIO_DATA(PIO_0_BASE, 1 << (position.col - 1)); // Shift bits to light corresponding column
    IOWR_ALTERA_AVALON_PIO_DATA(PIO_1_BASE, 1 << (position.row - 1)); // Shift bits to light corresponding row
    usleep(REFRESH_RATE);
	clearMatrix();
}

void drawFigure(struct figure shape) {
		lightLed(shape.led1);
		lightLed(shape.led2);
		lightLed(shape.led3);
		lightLed(shape.led4);
}

void drawFixedLeds(){
		for(int i=0;i<numOfFixedLeds;i++)
			lightLed(fixedLeds[i]);
}

void writeToDisplay(char *text){
	// initialize display
	unsigned char init_DOGM163_3V[9] = {0x39, 0x15, 0x55, 0x6E, 0x70, 0x38, 0x0C, 0x01, 0x06};
	for(int i=0; i<9;i++){
		IOWR_ALTERA_AVALON_PIO_DATA(PIO_3_BASE, 0);
		IOWR_ALTERA_AVALON_SPI_TXDATA(SPI_0_BASE, init_DOGM163_3V[i]);
		usleep(30);
	}

	unsigned char cursor = 0x08;
	IOWR_ALTERA_AVALON_PIO_DATA(PIO_3_BASE, 0);
	IOWR_ALTERA_AVALON_SPI_TXDATA(SPI_0_BASE, cursor);
	usleep(30);

	IOWR_ALTERA_AVALON_PIO_DATA(PIO_3_BASE, 0);
	IOWR_ALTERA_AVALON_SPI_TXDATA(SPI_0_BASE, 0x80);
	usleep(30);
	IOWR_ALTERA_AVALON_PIO_DATA(PIO_3_BASE, 0);
	IOWR_ALTERA_AVALON_SPI_TXDATA(SPI_0_BASE, 0x80);
	usleep(30);

	// Send text to display
	IOWR_ALTERA_AVALON_PIO_DATA(PIO_3_BASE, 1);
	for(int i=0; i<strlen(text); i++){
		IOWR_ALTERA_AVALON_SPI_TXDATA(SPI_0_BASE, text[i]);
		usleep(30);
	}
}

int checkCollision(){
	for(int i=0;i<numOfFixedLeds;i++)
		if((fixedLeds[i].col == movingShape.led1.col && fixedLeds[i].row - 1 == movingShape.led1.row) ||
		   (fixedLeds[i].col == movingShape.led2.col && fixedLeds[i].row - 1 == movingShape.led2.row) ||
		   (fixedLeds[i].col == movingShape.led3.col && fixedLeds[i].row - 1 == movingShape.led3.row) ||
		   (fixedLeds[i].col == movingShape.led4.col && fixedLeds[i].row - 1 == movingShape.led4.row))
			return 1;

	return 0;
}

int invalidMove(){
	int touchesBorder = 0;
	if(!((movingShape.led1.col <=8 && movingShape.led1.col >= 1 && movingShape.led1.row <=12 && movingShape.led1.row >= 1) &&
		(movingShape.led2.col <=8 && movingShape.led2.col >= 1 && movingShape.led2.row <=12 && movingShape.led2.row >= 1) &&
		(movingShape.led3.col <=8 && movingShape.led3.col >= 1 && movingShape.led3.row <=12 && movingShape.led3.row >= 1) &&
		(movingShape.led4.col <=8 && movingShape.led4.col >= 1 && movingShape.led4.row <=12 && movingShape.led4.row >= 1)))
			touchesBorder=1;

	int touchesFixedLeds = 0;
	for(int i=0;i<numOfFixedLeds;i++)
		if((fixedLeds[i].col == movingShape.led1.col && fixedLeds[i].row == movingShape.led1.row) ||
		   (fixedLeds[i].col == movingShape.led2.col && fixedLeds[i].row == movingShape.led2.row) ||
		   (fixedLeds[i].col == movingShape.led3.col && fixedLeds[i].row == movingShape.led3.row) ||
		   (fixedLeds[i].col == movingShape.led4.col && fixedLeds[i].row == movingShape.led4.row)){
			   touchesFixedLeds = 1;
			   break;
		   }

	if (touchesBorder == 1 || touchesFixedLeds == 1)
		return 1;

	return 0;
}

void rotateFigure(){
	if(isSquare == 0){ // Square should not be rotated
		struct figure shapeBeforeRotation = movingShape;
		struct led center = movingShape.led2;
		int swapRow;

		swapRow = movingShape.led1.row;
		movingShape.led1.row = center.row - (center.col - movingShape.led1.col);
		movingShape.led1.col = center.col + (center.row - swapRow);

		swapRow = movingShape.led3.row;
		movingShape.led3.row = center.row - (center.col - movingShape.led3.col);
		movingShape.led3.col = center.col + (center.row - swapRow);

		swapRow = movingShape.led4.row;
		movingShape.led4.row = center.row - (center.col - movingShape.led4.col);
		movingShape.led4.col = center.col + (center.row - swapRow);

		if(invalidMove() == 1)
			movingShape = shapeBeforeRotation;
	}
}

void moveLeft(){
	   struct figure shapeBeforeMovement = movingShape;

		movingShape.led1.col--;
		movingShape.led2.col--;
		movingShape.led3.col--;
		movingShape.led4.col--;

		if(invalidMove() == 1)
			movingShape = shapeBeforeMovement;
}

void moveRight(){
	   struct figure shapeBeforeMovement = movingShape;

		movingShape.led1.col++;
		movingShape.led2.col++;
		movingShape.led3.col++;
		movingShape.led4.col++;

		if(invalidMove() == 1)
			movingShape = shapeBeforeMovement;
}

void fallDown(){
	   struct figure shapeBeforeFalling = movingShape;

		movingShape.led1.row++;
		movingShape.led2.row++;
		movingShape.led3.row++;
		movingShape.led4.row++;

		if(invalidMove() == 1)
			movingShape = shapeBeforeFalling;
}

void touchDown(){
	  fixedLeds[numOfFixedLeds++] = movingShape.led1;
	  fixedLeds[numOfFixedLeds++] = movingShape.led2;
	  fixedLeds[numOfFixedLeds++] = movingShape.led3;
	  fixedLeds[numOfFixedLeds++] = movingShape.led4;
}

void ifBtnPressedDoAction(){
	btnValue = IORD_ALTERA_AVALON_PIO_DATA(PIO_2_BASE);
		if (btnDelay % 11 == 0)
			switch (btnValue)
			{
			    case 0b1110: // UP
			    	rotateFigure();
			    	break;
			    case 0b1101: // LEFT
			    	moveLeft();
			    	break;
			    case 0b1011: // RIGHT
			    	moveRight();
			    	break;
			    case 0b0111: // DOWN
			    	fallDown();
			    	break;
			}
		btnDelay++;
}

void eliminateCompleteLines(){
	int numOfLedsInSameRow = 0;
	int bonusMultiplier = 0;
	for(int currentRow=1; currentRow<=12; currentRow++){
		numOfLedsInSameRow = 0;
		for(int i=0;i<numOfFixedLeds;i++){
			if(fixedLeds[i].row == currentRow)
				numOfLedsInSameRow++;
		}
		if(numOfLedsInSameRow == 8){
			bonusMultiplier++;
				for(int i=0;i<numOfFixedLeds;i++){
					if(fixedLeds[i].row == currentRow){
						fixedLeds[i].col = 13; // Mark completed LED to be removed
						fixedLeds[i].row = 13;
					} else if(fixedLeds[i].row < currentRow)
						fixedLeds[i].row++;
			}
		}
	}
	points+=(bonusMultiplier*10) * bonusMultiplier;
	int counter = 0;
	for(int i=0; i<numOfFixedLeds; i++){
		if(fixedLeds[i + counter].col == 13)
			counter++;
		fixedLeds[i] = fixedLeds[i + counter]; // Remove marked completed LEDs
	}
	numOfFixedLeds-=counter;
}

int main()
{
  printf("GAME STARTED\n");
  writeToDisplay("    START GAME");

  char strPoints[16];
  points = 0;
  numOfFixedLeds = 0;
  timer = 1;
  gameOver = 0;
  randSeed = 0;
  btnValue = IORD_ALTERA_AVALON_PIO_DATA(PIO_2_BASE);
    while(btnValue == 0b1111){  // No button pressed
  	  btnValue = IORD_ALTERA_AVALON_PIO_DATA(PIO_2_BASE);
  	  randSeed++;
    }
    srand(randSeed); // Seed the random figure generator based on elapsed time

    writeToDisplay("        3");
    usleep(1000000);
    writeToDisplay("        2");
    usleep(1000000);
    writeToDisplay("        1");
    usleep(1000000);

  while(gameOver == 0){
	  sprintf(strPoints, "    POINTS: %d", points);
	  writeToDisplay(strPoints);
	  movingShape = generateNewRandomShape();

	  if(checkCollision() == 1)
		  gameOver = 1;

	  while(movingShape.led1.row < 12 && movingShape.led2.row < 12 && movingShape.led3.row < 12 && movingShape.led4.row < 12){
		    btnDelay = 0;
			for(int fallTimer = 0; fallTimer < GAME_SPEED; fallTimer+=timer){
				ifBtnPressedDoAction();
				drawFigure(movingShape);
				drawFixedLeds();
			}

			if(checkCollision() == 1)
				break;

			fallDown();
		}

	  touchDown();

	  eliminateCompleteLines();

	  if(numOfFixedLeds/2 == 0){ // Dynamically adjust speed based on how full the matrix is
		  timer = 1;
	  } else {
		  timer = numOfFixedLeds/2;
	  }
  }
  clearMatrix();
  writeToDisplay("    GAME OVER");
  printf("GAME OVER");
  printf("\nPOINTS: %d", points);
  return 0;
}
