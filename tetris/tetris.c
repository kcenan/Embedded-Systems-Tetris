/*
 * tetris.c
 *
 * Created: 1/5/2017 11:22:58 PM
 *  Author: CIT
 */ 


#include<avr/io.h>
/*Includes io.h header file where all the Input/Output Registers and its Bits are defined for all AVR microcontrollers*/

#define    F_CPU    1000000
/*Defines a macro for the delay.h header file. F_CPU is the microcontroller frequency value for the delay.h header file. Default value of F_CPU in delay.h header file is 1000000(1MHz)*/

#include<util/delay.h>
/*Includes delay.h header file which defines two functions, _delay_ms (millisecond delay) and _delay_us (microsecond delay)*/
// LCD Commands.
#define START_LINE        0b11000000
#define SET_ADDRESS        0b01000000
#define SET_PAGE        0b10111000
#define DISP_ON            0b00111111
#define DISP_OFF        0b00111110

#define        GLCD_DATA_DDR        DDRC
#define        GLCD_DATA_PIN        PINC
#define        GLCD_DATA_PORT        PORTC
/*Defines a macro for the glcd.h header file. GLCD_DATA_PORT macro defines the PORT Register to which the data pins of the Graphical LCD are connected. Default PORT Resistor for data pins in glcd.h file is PORTA*/

#define     GLCD_CS_PORT         PORTB
/*Defines a macro for the glcd.h header file. GLCD_CONT_PORT macro defines the PORT Register to which the control pins of the Graphical LCD are connected. Default PORT Resistor for control pins in glcd.h file is PORTB*/

#define     GLCD_RSRW_PORT         PORTA
/*Defines a macro for the glcd.h header file. GLCD_CONT_PORT macro defines the PORT Register to which the control pins of the Graphical LCD are connected. Default PORT Resistor for control pins in glcd.h file is PORTB*/

#define     GLCD_ERST_PORT         PORTD
/*Defines a macro for the glcd.h header file. GLCD_CONT_PORT macro defines the PORT Register to which the control pins of the Graphical LCD are connected. Default PORT Resistor for control pins in glcd.h file is PORTB*/

#define     GLCD_RS     2
/*Defines a macro for the glcd.h header file. GLCD_RS macro defines the microcontroller Port pin to which the RS pin of the Graphical LCD is connected. Default Port pin for RS pin in glcd.h file is PB0*/

#define     GLCD_RW     3
/*Defines a macro for the glcd.h header file. GLCD_RW macro defines the microcontroller Port pin to which the RW pin of the Graphical LCD is connected. Default Port pin for RW pin in glcd.h file is PB1*/

#define     GLCD_EN     6
/*Defines a macro for the glcd.h header file. GLCD_EN macro defines the microcontroller Port pin to which the EN pin of the Graphical LCD is connected. Default Port pin for EN pin in glcd.h file is PB2*/

#define     GLCD_CS1     0
/*Defines a macro for the glcd.h header file. GLCD_CS1 macro defines the microcontroller Port pin to which the CS1 pin of the Graphical LCD is connected. Default Port pin for CS1 pin in glcd.h file is PB3*/

#define     GLCD_CS2     1
/*Defines a macro for the glcd.h header file. GLCD_CS1 macro defines the microcontroller Port pin to which the CS2 pin of the Graphical LCD is connected. Default Port pin for CS2 pin in glcd.h file is PB4*/

#define     GLCD_RST     7
/*Defines a macro for the glcd.h header file. GLCD_RST macro defines the microcontroller Port pin to which the RST pin of the Graphical LCD is connected. Default Port pin for RST pin in glcd.h file is PB5*/

#define MOSI 5
#define SCK 7
#define SS 4

#include<avr/io.h>
#include<util/delay.h>
#include<avr/interrupt.h>

static int array[8][64];
int column;
int page;
int currentrow;
int currentcolumn;
int currentshape;
int pauseState;
//current shape: 0dikline, 1kere, 2yukar?t,3uzun çubuk sa?da a?a?? L 180degree rotation, 4 uzun çubuk yan , 5 sa?aT , 6uzunçubukaltta k?sa çubuk sa?daL
int  static gameOn;
int static score;
int counter;

int getScore(){
	return score;
}

void setScore(int sc){
	 score=sc;
}

int getPauseState(){
	return pauseState;
}

void setPauseState(int bb){
	 pauseState=bb;
}


int getGameOn(){
	return gameOn;
}

void setGameOn(int aa){
	gameOn =aa;
}

void setArray(int pagenum,int colonindex,int Num){
    array[pagenum][colonindex]=Num;
}

void resetArray(int pagenum,int colonindex){
    array[pagenum][colonindex]=0;
}

int getArray(int pagenum,int colonindex){
    return array[pagenum][colonindex];
}

int getArray64(int row,int colonindex){
    int page=row/8;
    int index=row%8;
    int curr= getArray(page,colonindex);
    curr = checkDigit(curr,index);
    return curr;
}

void setArray64(int row,int colonindex){
    int page=row/8;
    int index=row%8;
    int curr= getArray(page,colonindex);
    curr |=(1<<index);
    setArray(page,colonindex,curr);
}

void resetArray64(int row,int colonindex){
    int page=row/8;
    int index=row%8;
    int curr= getArray(page,colonindex);
    curr &=~ (1<<index);
    setArray(page,colonindex,curr);
}

void fillArray(int row,int column,int height){
    for (int i=0;i<height;i++)
    {
        setArray64(row+i,column);
    }
}

void refillArray(int row,int column,int height){
    for ( int i=0;i<height;i++)
    {
        resetArray64(row+i,column);
    }
}

int clearArray(){
    for (int i=0;i<60;i++)
    {
        if (checkLineFull(i))
        {
				setScore(getScore()+1);
                for(int j=4;j<20;j++){
                resetArray64(i,j*3);
                resetArray64(i,j*3+1);
                resetArray64(i,j*3+2);
            }
        }
    }
}

int ClearScreen(){
    for (int i=0;i<64;i++)
    {
     
                for(int j=0;j<20;j++){
                resetArray64(i,j*3);
                resetArray64(i,j*3+1);
                resetArray64(i,j*3+2);
            }
        
    }
}

int FillScreen(){
    for (int i=0;i<64;i++)
    {
                for(int j=4;j<20;j++){
                setArray64(i,j*3);
                setArray64(i,j*3+1);
                setArray64(i,j*3+2);
            }
        
    }
}

int checkLineFull(int row){
    int res=1;
    for(int i=4;i<20;i++){
        if (getArray64(row,i*3)==0)
        {
            res=0;
        }
    }
    return res;
}

int checkLineEmpty(int row){
    int res=1;
    for(int i=4;i<20;i++){
        if (getArray64(row,i*3))
        {
            res=0;
        }
    }
    return res;
}

void moveToAnotherRow(int from,int to){
    for(int k=4;k<20;k++){
        if (getArray64(from,k*3)){
            setArray64(to,k*3);
            resetArray64(from,k*3);
            setArray64(to,k*3+1);
            resetArray64(from,k*3+1);
            setArray64(to,k*3+2);
            resetArray64(from,k*3+2);
        }
    }
}

void updateArray(){
    
	clearArray();
    for(int k=59;k>=0;k--){
        if (checkLineEmpty(k)==0)
        {
            int carryTo=firstEmptyLineBefore(k);
            if (carryTo!=99)
            {
                    moveToAnotherRow(k,carryTo);
            }
            
        }
    }
}

int anyEmptyLineBefore(int row){
    int bool=0;    
        for(int k=0;k<row;k++){
        if (checkLineEmpty(k))
        {
            bool=1;
        }
    }
    return bool;
}

int anyFullLineBefore(int row){
    int bool=0;    
        for(int k=0;k<row;k++){
        if (checkLineFull(k))
        {
            bool=1;
        }
    }
    return bool;
}

int firstEmptyLineBefore(int row){
    int bool=99;    
        for(int k=row+1;k<61;k++){
        if (checkLineEmpty(k))
        {
            bool=k;
        }
    }
    return bool;
}

void deleteArray(int row,int column,int height){
    for (int i=0;i<height;i++)
    {
        resetArray64(row+i,column);
    }
}

void makeLine(int row,int column){
	for (int i=0;i<3;i++)
	{
    	fillArray(row,column+i,12);
	}
}

void deleteLine(int row,int column){
	for (int i=0;i<3;i++)
	{
    	deleteArray(row,column+i,12);
	}
}
//
void makeHorizontalLine(int row,int column){
	for (int i=0;i<12;i++)
	{
		fillArray(row,column+i,3);
	}
}
//
void deleteHorizontalLine(int row,int column){
	for (int i=0;i<12;i++)
	{
		deleteArray(row,column+i,3);
	}
}

void makeSquare(int row,int column){
	for (int i=0;i<6;i++)
	{
    	fillArray(row,column+i,6);
	}
}

void deleteSquare(int row,int column){
	for (int i=0;i<6;i++)
	{
    	deleteArray(row,column+i,6);
	}
}


void makeShapeT(int row,int column){
	for (int i=0;i<9;i++)
	{
    	fillArray(row+3,column+i,3);
	}
	for (int i=3;i<6;i++)
	{
    	fillArray(row,column+i,3);
	}
}

void deleteShapeT(int row,int column){
    	for (int i=0;i<9;i++){
        	deleteArray(row+3,column+i,3);
    	}
    	for (int i=3;i<6;i++){
    		deleteArray(row,column+i,3);
    	}
}
//DENE
void makeShapeL(int row,int column){
	for (int i=0;i<3;i++)
	{
		fillArray(row,column+i,3);
	}
	for (int i=3;i<6;i++)
	{
		fillArray(row ,column+i,9);
	}
}
//DENE
void deleteShapeL(int row,int column){
		for (int i=0;i<3;i++)
		{
			deleteArray(row,column+i,3);
		}
		for (int i=3;i<6;i++)
		{
			deleteArray(row ,column+i,9);
		}
}
void makeRightT(int row,int column){
	for (int i=0;i<3;i++)
	{
		fillArray(row,column+i,9);
		fillArray(row+3 ,column+3+i ,3);
	}
		
}
//DENE
void deleteRightT(int row,int column){
	for (int i=0;i<3;i++)
	{
		deleteArray(row,column+i,9);
		deleteArray(row+3 ,column+3+i ,3);
	}
	
}
void makeDownL(int row,int column){
	for (int i=0;i<6;i++)
	{
		fillArray(row + 3,column+i,3);
	}
	for (int i=6;i<9;i++)
	{
		fillArray(row ,column+i,6);
	}
}
//DENE
void deleteDownL(int row,int column){
	for (int i=0;i<6;i++)
	{
		deleteArray(row + 3,column+i,3);
	}
	for (int i=6;i<9;i++)
	{
		deleteArray(row ,column+i,6);
	}
}



int power(int mainnum, int pow){
    int result=1;
    if(pow!=0){
    for(int i=0;i<pow;i++){
        result=result*mainnum;
    }
    }
return result;
}

int checkDigit(int num,int index){
    int res= power(2,index);
    res=res & num;
    return res;
}
 

void goNext(){
	if (moveCheck())
        {
            move();
        }
        else{
			if (anyFullLineBefore(60))
			{
				
				updateArray();
			}
            createRandomShape();
        }
}
 int moveCheck(){
	int filled = 0;
    
	if (currentshape == 0){
   	 if (currentrow == 48){
   	 filled = 1;
   	 }
   	 else{
    	filled = getArray64(currentrow + 12,currentcolumn) || filled;
   	 }
    }
	else if (currentshape == 1){
   	 if (currentrow == 54){
   	 filled = 1;
   	 }
   	 else{
    	filled = getArray64(currentrow + 7,currentcolumn) || getArray64(currentrow + 7,currentcolumn+3) ;
   	 }
   	 
    }
	else if (currentshape == 2){
   	 if (currentrow == 54){
   	 filled = 1;
   	 }
   	 else{
    	filled = getArray64(currentrow + 6,currentcolumn) || getArray64(currentrow + 6,currentcolumn+3) || getArray64(currentrow + 6,currentcolumn+6);
   	 }
    }
    else if (currentshape == 3){
	    if (currentrow == 51){
		    filled = 1;
	    }
	    else{
		    filled = getArray64(currentrow + 3,currentcolumn) || getArray64(currentrow + 9,currentcolumn+3) ;
	    }
    }
	else if (currentshape == 4){
		if (currentrow == 57){
			filled = 1;
		}
		else{
			for (int i = 0 ; i<4; i++){
				filled = getArray64(currentrow + 3,currentcolumn + (i*3)) || filled;
			}	
		}
	}
	 else if (currentshape == 5){
		 if (currentrow == 51){
			 filled = 1;
		 }
		 else{
			 filled = getArray64(currentrow + 9,currentcolumn) || getArray64(currentrow + 6,currentcolumn+3) ;
		 }
	 }
	 else if (currentshape == 6){
		 if (currentrow == 54){
			 filled = 1;
		 }
		 else{
			for (int i = 0 ; i<3; i++){
				filled = getArray64(currentrow + 6,currentcolumn + (i*3)) || filled;
			}
		 }
	 }
    filled =! filled;
  	return filled;
	}
    
void move(){
	if (currentshape == 0){
    	deleteLine(currentrow,currentcolumn);
    	currentrow = currentrow + 3;
    	makeLine(currentrow,currentcolumn);
	}
	else if (currentshape == 1){
    	deleteSquare(currentrow,currentcolumn);
    	currentrow = currentrow + 3;
    	makeSquare(currentrow,currentcolumn);
	}
	else if (currentshape == 2){
    	deleteShapeT(currentrow,currentcolumn);
    	currentrow = currentrow + 3;
    	makeShapeT(currentrow,currentcolumn);
	}
	else if (currentshape == 3){
		deleteShapeL(currentrow,currentcolumn);
		currentrow = currentrow + 3;
		makeShapeL(currentrow,currentcolumn);
	}
	else if (currentshape == 4){
		deleteHorizontalLine(currentrow,currentcolumn);
		currentrow = currentrow + 3;
		makeHorizontalLine(currentrow,currentcolumn);
	}
	else if (currentshape == 5){
		deleteRightT(currentrow,currentcolumn);
		currentrow = currentrow + 3;
		makeRightT(currentrow,currentcolumn);
	}
	else if (currentshape == 6){
		deleteDownL(currentrow,currentcolumn);
		currentrow = currentrow + 3;
		makeDownL(currentrow,currentcolumn);
	}
}
 
void createRandomShape(){
    
    currentshape = rand() % 7 ;
    //currentshape = 2;
   	 if (currentshape == 0){
   		 currentrow = 0;
   		 currentcolumn = 33;
   		 checkGenerate();
   		 makeLine(currentrow,currentcolumn);
   	 }
   	 else if (currentshape == 1){
   		 currentrow = 0;
   		 currentcolumn = 33;
   		 checkGenerate();
   		 makeSquare(currentrow,currentcolumn);
   	 }
   	 else if (currentshape == 2){
   		 currentrow = 0;
   		 currentcolumn = 33;
   		 checkGenerate();
   		 makeShapeT(currentrow,currentcolumn);   	 
   	 }
	else if (currentshape == 3){
		currentrow = 0;
		currentcolumn = 33;
		checkGenerate();
		makeShapeL(currentrow,currentcolumn);
	}
	else if (currentshape == 4){
		currentrow = 0;
		currentcolumn = 30;
		checkGenerate();
		makeHorizontalLine(currentrow,currentcolumn);
	}
	else if (currentshape == 5){
		currentrow = 0;
		currentcolumn = 33;
		checkGenerate();
		makeRightT(currentrow,currentcolumn);
	}
	else if (currentshape == 6){
		currentrow = 0;
		currentcolumn = 33;
		checkGenerate();
		makeDownL(currentrow,currentcolumn);
	}
    
}

void checkGenerate(){
	int filled = 0;
    
    if (currentshape == 0){
   	 for (int i=0;i<4;i++){
   		 filled = filled || getArray64(currentrow + (i*3),currentcolumn);
   	  }
    }
    else if (currentshape == 1){
   		 filled = getArray64(currentrow ,currentcolumn ) || getArray64(currentrow ,currentcolumn + 3) || getArray64(currentrow + 3,currentcolumn)
   		 || getArray64(currentrow + 3,currentcolumn+3);
    }
    else if(currentshape == 2){
   	 for (int i=0;i<3;i++){
   		 filled = getArray64(currentrow + 3 ,currentcolumn + i*3 );
   	 }
   		 filled = filled || getArray64(currentrow ,currentcolumn+3);
    }
     else if(currentshape == 3){  
		 for (int i=0;i<3;i++){
			 filled = getArray64(currentrow + i*3 ,currentcolumn +3 );
		 }
		 filled = filled || getArray64(currentrow ,currentcolumn);
	 }
	 else if(currentshape == 4){
		 
		 for (int i=0;i<4;i++)
		 {
			 filled = filled || getArray64(currentrow+i*3,currentcolumn);
		 }
	 }
	 
	 else if(currentshape == 5){
		 
		 for (int i=0;i<3;i++)
		 {
			filled = filled || getArray64(currentrow+i*3,currentcolumn);
		 }
		 filled = filled || getArray64(currentrow+3,currentcolumn+3);
	 }
	  else if(currentshape == 6){
		 for (int i=0;i<3;i++)
		 {
			 filled = filled || getArray64(currentrow+3,currentcolumn+i*3);
		 }
		 filled = filled || getArray64(currentrow,currentcolumn+6);
	  }
	  
    if (filled == 1){
   	 gameOver();
    }
    
}

int moveAroundCheck(){
	int filled = 0;
	if (currentshape == 0){
   		for (int i=2;i<4;i++){
   		filled |= getArray64(currentrow + 3 ,currentcolumn - 3 + (i*3));
   		}
		filled |= getArray64(currentrow + 3 ,currentcolumn - 3 );
   	}
	//shape 1 is free.	
	else if (currentshape == 2){ //yukar?T
		filled |=  getArray64(currentrow + 6,currentcolumn+3);
	}	
	else if (currentshape == 3){ //3uzun çubuk sa?da a?a?? L 180degree rotation
		for (int i=0;i<2;i++){
   		filled |= getArray64(currentrow  + (i*3) ,currentcolumn + 6);
   		}
		filled |=  getArray64(currentrow+3 ,currentcolumn);
	}
	
	else if (currentshape == 4){
		for (int i=0;i<2;i++){
   		filled |=getArray64(currentrow + 3+ (i*3),currentcolumn+3);
   		}
		filled |=getArray64(currentrow-3,currentcolumn+3);
	}
	else if (currentshape == 5){
		filled |=  getArray64(currentrow + 3,currentcolumn-3);
	}
	else if (currentshape == 6){ //6uzunçubukaltta k?sa çubuk sa?daL
		for (int i=0;i<2;i++){
   		filled |= getArray64(currentrow-3  ,currentcolumn + (i*3) );
   		}
		filled |=  getArray64(currentrow+6 ,currentcolumn+3);
	}
	
    return filled;
		
}

void moveAround(){
	if (currentshape == 0){
		deleteLine(currentrow,currentcolumn);
		currentshape = 4;
		currentrow = currentrow + 3;
		currentcolumn = currentcolumn - 3;
		makeHorizontalLine(currentrow,currentcolumn);
	}
	else if (currentshape == 2){
		deleteShapeT(currentrow,currentcolumn);
		currentshape = 5;
		currentcolumn = currentcolumn + 3;
		makeRightT(currentrow,currentcolumn);
	}
	
	else if (currentshape == 3){
		deleteShapeL(currentrow,currentcolumn);
		currentshape = 6;
		makeDownL(currentrow,currentcolumn);
	}
	
	else if (currentshape == 4){
		deleteHorizontalLine(currentrow,currentcolumn);
		currentshape = 0;
		currentrow = currentrow - 3;
		currentcolumn = currentcolumn + 3;
		makeLine(currentrow,currentcolumn);
	}
	else if (currentshape == 5){
		deleteRightT(currentrow,currentcolumn);
		currentshape = 2;
		currentcolumn = currentcolumn - 3;
		makeShapeT(currentrow,currentcolumn);
	}
	else if (currentshape == 6){
		deleteDownL(currentrow,currentcolumn);
		currentshape = 3;
		makeShapeL(currentrow,currentcolumn);
	}
}

void turn(){
	if (moveAroundCheck() == 0){
        	moveAround();
    	}
}

void gameOver(){
	FillScreen();
	setGameOn(0);
	setScore(0);
}

 void rightMove(){
    if (rightShiftCheck())
    {rightShift();
    }
}

 int rightShiftCheck(){
    int filled = 0;
    if (currentshape == 0){
   	 if (currentcolumn==57){
   		 filled = 1;
   	 }
   	 else{
   		 for (int i=0;i<4;i++){
   		 filled = filled || getArray64(currentrow + (i*3),currentcolumn+3);
   		 }
   	 }
     }
   	 
     else if (currentshape == 1){
   	  if (currentcolumn==54){
   		 filled = 1;
   	 }
   	  else{
   		 filled = getArray64(currentrow ,currentcolumn + 6 ) || getArray64(currentrow + 3,currentcolumn+6) ;
   	 }
     }
    
     else if (currentshape == 2){
   	 if (currentcolumn==51){
   		 filled = 1;
   	 }
   	  else{
   		 filled = getArray64(currentrow + 3 ,currentcolumn + 9) || getArray64(currentrow ,currentcolumn+ 6);
   	  }
     }
	 else if (currentshape == 3){
		 if (currentcolumn==54){
			 filled = 1;
		 }
		 else{
			 filled = getArray64(currentrow  ,currentcolumn + 6) || getArray64(currentrow +3 ,currentcolumn+ 6) || getArray64(currentrow +6 ,currentcolumn+ 6);
		 }
	 }
	 else if (currentshape == 4){
		 if (currentcolumn==48){
			 filled = 1;
		 }
		 else{
			 filled = getArray64(currentrow  ,currentcolumn + 12) || filled;
		 }
	 }
     
	 else if (currentshape == 5){
		 if (currentcolumn==54){
			 filled = 1;
		 }
		 else{
			 filled = getArray64(currentrow  ,currentcolumn +3) || getArray64(currentrow +3  ,currentcolumn +6) || getArray64(currentrow+6  ,currentcolumn +3);
		 }
	 }
	  else if (currentshape == 6){
		  if (currentcolumn==51){
			  filled = 1;
		  }
		  else{
			  filled = getArray64(currentrow  ,currentcolumn +9) || getArray64(currentrow +3  ,currentcolumn +9) ;
		  }
	  }
     filled =! filled;
    return filled;
 }

 void rightShift(){
	if (currentshape == 0){
    	deleteLine(currentrow,currentcolumn);
    	currentcolumn = currentcolumn + 3;
    	makeLine(currentrow,currentcolumn);
	}
	else if (currentshape == 1){
    	deleteSquare(currentrow,currentcolumn);
    	currentcolumn = currentcolumn + 3;
    	makeSquare(currentrow,currentcolumn);
	}
	else if (currentshape == 2){
    	deleteShapeT(currentrow,currentcolumn);
    	currentcolumn = currentcolumn + 3;
    	makeShapeT(currentrow,currentcolumn);
	}
	else if (currentshape == 3){
		deleteShapeL(currentrow,currentcolumn);
		currentcolumn = currentcolumn + 3;
		makeShapeL(currentrow,currentcolumn);
	}
	else if (currentshape == 4){
		deleteHorizontalLine(currentrow,currentcolumn);
		currentcolumn = currentcolumn + 3;
		makeHorizontalLine(currentrow,currentcolumn);
	}
	else if (currentshape == 5){
		deleteRightT(currentrow,currentcolumn);
		currentcolumn = currentcolumn + 3;
		makeRightT(currentrow,currentcolumn);
	}
	else if (currentshape == 6){
		deleteDownL(currentrow,currentcolumn);
		currentcolumn = currentcolumn + 3;
		makeDownL(currentrow,currentcolumn);
	}
}

void leftMove(){
    if (leftShiftCheck())
    {leftShift();
    }
}

 int leftShiftCheck(){
     int filled = 0;
     if (currentcolumn == 0)
     {
   	  filled = 1;
     }
     else{
     if (currentshape == 0){
   	  for (int i=0;i<4;i++){
   		   filled = filled || getArray64(currentrow + (i*3),currentcolumn-1);
   	  }
   	 
     }
     else if (currentshape == 1){
   	  filled = getArray64(currentrow ,currentcolumn - 1 ) || getArray64(currentrow + 3,currentcolumn - 1) ;
     }
     else if (currentshape == 2){
   	  filled = getArray64(currentrow ,currentcolumn + 2 ) || getArray64(currentrow + 3,currentcolumn - 1 );
     }
	 else if (currentshape == 3){
		for (int i=1;i<3;i++){
		  filled = filled || getArray64(currentrow + (i*3),currentcolumn+2);
		 }
		  filled = getArray64(currentrow ,currentcolumn -1) || filled;
	 }
	else if (currentshape == 4){
		 filled = getArray64(currentrow ,currentcolumn -1) || filled;
	 }
	else if (currentshape == 5){
		 for (int i=0;i<3;i++){
			 filled = filled || getArray64(currentrow + (i*3),currentcolumn-1);
		 }
	 }
	 else if (currentshape == 6){
			 filled = getArray64(currentrow ,currentcolumn + 5) || getArray64(currentrow + 3,currentcolumn - 1);
	 }
	}
 
     filled =! filled;
     return filled;
 }

  void leftShift(){
 	if (currentshape == 0){
    	deleteLine(currentrow,currentcolumn);
    	currentcolumn = currentcolumn - 3;
    	makeLine(currentrow,currentcolumn);
	}
	else if (currentshape == 1){
    	deleteSquare(currentrow,currentcolumn);
    	currentcolumn = currentcolumn - 3;
    	makeSquare(currentrow,currentcolumn);
	}
	else if (currentshape == 2){
    	deleteShapeT(currentrow,currentcolumn);
    	currentcolumn = currentcolumn - 3;
    	makeShapeT(currentrow,currentcolumn);
	}
	else if (currentshape == 3){
		deleteShapeL(currentrow,currentcolumn);
		currentcolumn = currentcolumn - 3;
		makeShapeL(currentrow,currentcolumn);
	}
	else if (currentshape == 4){
		deleteHorizontalLine(currentrow,currentcolumn);
		currentcolumn = currentcolumn - 3;
		makeHorizontalLine(currentrow,currentcolumn);
	}
	else if (currentshape == 5){
		deleteRightT(currentrow,currentcolumn);
		currentcolumn = currentcolumn - 3;
		makeRightT(currentrow,currentcolumn);
	}
	else if (currentshape == 6){
		deleteDownL(currentrow,currentcolumn);
		currentcolumn = currentcolumn - 3;
		makeDownL(currentrow,currentcolumn);
	}
}


void DataMode (void)
{   
    GLCD_RSRW_PORT  |= (1<<GLCD_RS);
    GLCD_RSRW_PORT   &= ~ (1<<GLCD_RW);
  
}

void WriteData(uint8_t data) {
    DataMode();
    GLCD_DATA_PORT = data;                 
    LocknLoad();
}

void WriteCommand(uint8_t cmd) {
        
        GLCD_CS_PORT  |= (1<<GLCD_CS1);       
        
        GLCD_RSRW_PORT   &= ~ (1<<GLCD_RW);// Clear RW & RS for Command Mode.   
        GLCD_RSRW_PORT   &= ~ (1<<GLCD_RS);         
        
        GLCD_DATA_PORT = cmd;                   // Send the command.
        LocknLoad();
}

void LocknLoad (void)
{
    GLCD_ERST_PORT   |=   (1<<GLCD_EN);     
    
    _delay_us(4);               // Small delay.
    
    GLCD_ERST_PORT  &= ~(1<<GLCD_EN);

    _delay_us(4);
}

void clearl (void){
    
    unsigned int i;
    
    // Reset global variables.
    column = 0;
    page = 0;
    
    // Reset LCD registers.
    WriteCommand(SET_ADDRESS);
    WriteCommand(SET_PAGE);
    
    
    for(i = 0; i < 1024 ;i++)
    {
        
        if (column == 128)
        {
            column = 0;
            page++;
            setNewPage(page);
        }

        cSelect(column);
        
        DataMode();
        GLCD_DATA_PORT = 0b00000000;                 // This clears the pixels.
        LocknLoad();

        column++;
    }
    
    // Reset global variables.
    column = 0;
    page = 0;
    
    // Reset LCD registers.
    WriteCommand(SET_ADDRESS);
    WriteCommand(SET_PAGE);
    
}

void setXY(uint8_t x, uint8_t y) {      
    //x column
    //y row page
    uint8_t cmd;
    
    if(x > 127) x = 0;                // Validate the x and y values.
    if(y > 63)  y = 0;
    page = y;
    column = x;
    cSelect(column);                 // Select the chip based on the column number.
    // The column variable is global and also used
    // by the ASCII driver function lcd_putch().
    
    
    cmd = SET_ADDRESS | x;            //Select coulmn
    WriteCommand(cmd);
    
    cmd = SET_PAGE | page;            // Set the Page (y address) on the selected chip.
    WriteCommand(cmd);

}

void cSelect(uint8_t col)
{
        GLCD_CS_PORT  |= (1<<GLCD_CS1);       
        GLCD_CS_PORT  &= ~(1<<GLCD_CS2);       

}

void setNewPage (unsigned char PageData)
{
    PageData = PageData + SET_PAGE;
    WriteCommand(PageData);
    
    WriteCommand(SET_ADDRESS);         // This line resets the column address to the
}

void showArray(){
    for (int i=0;i<8;i++)
    {
    for (int j=0;j<64;j++)
    {
    setXY(j,i);
    WriteData(getArray(i,j));
    }
    }
		
	showScore();
    
}

void fast(){
	while (moveCheck())
	{
		goNext();
	}
	counter=5;	
}

void initializePorts(){
      DDRC|=0xff;
      DDRA|=0x0C;
      DDRB|=0x03;
      DDRD|=0xC0;
      
      GLCD_DATA_PORT    =0x00;
      GLCD_ERST_PORT    =0x00;
      GLCD_RSRW_PORT    =0x00;
      GLCD_CS_PORT        =0x00;
      
      GLCD_ERST_PORT |= (1<<GLCD_RST);

      WriteCommand(DISP_OFF);      // According to the data sheet this is needed.
      WriteCommand(START_LINE);
      WriteCommand(DISP_ON);
      // Clear every pixel and reset the column and page to 0.
      clearl();
	  setScore(0);
	  setGameOn(0);
	  setPauseState(0);
}

ISR(TIMER0_OVF_vect){
	_delay_ms(500);
	if (getGameOn())
		{
	counter++;
		}
		
	int valA=PINA;
	int valB= PINB;
    if(checkDigit(valB,7)) //left
    {
		if (getGameOn())
		{
		leftMove();
		}
    }else if (checkDigit(valB,5))//rotate
    {
		if (getGameOn())
		{
		rightMove();
		}
    }else if (checkDigit(valB,6))//right
    {		
		if (getGameOn())
		{
			fast();
		}
		
			
	}else if (checkDigit(valB,4))//rotate
    {
		if (getGameOn())
		{
			turn();
		}
	}
	
	if (checkDigit(valA,7))//new game
	{		
			ClearScreen();
			initializeScreen();
			createRandomShape();
			setGameOn(1);
			setScore(0);
	}else if (checkDigit(valA,6))//pause
    {
			setGameOn(0);
			
	}else if (checkDigit(valA,5))//cont
	{
		setGameOn(1);
		
		}

	
	if (counter==5 )
	{	

				goNext();
				counter=0;
		
	
		
		
	}
		showArray();
}

void initializeInterupt(){

	DDRB &= 0x0F;  //PORT B INPUT
	DDRA &=0x3f;
	
 	TCNT0=-32;
 	TCCR0=0x01;
 	TIMSK=(1<<TOIE0);

 	sei();
}

void initializeScreen(){

	fillArray(0,60,64);
	fillArray(0,11,64);
	
for (int i=0;i<11;i++)
{
	refillArray(0,i,64);
}

for (int i=0;i<6;i++)
{
	setArray64(i*10,61);
}
	
for (int i=7;i<64;i++)
{
		setArray64(60,i);
		setArray64(61,i);
		setArray64(62,i);
		setArray64(63,i);
}

}

void showScore(){
	refillArray(0,62,60);
	refillArray(0,63,60);
	
	if (getScore()<181)
	{
		fillArray(60-(getScore()/3),63,(getScore()/3));
	}else if (getScore()>180 && getScore() <363)
	{
		fillArray(0,63,64);
		fillArray(4-(getScore()/3),62,(getScore()/3)-64);
	}
}

int main(void)
{

	initializePorts();
	 initializeScreen();
	initializeInterupt();
 
	while(1){
	}

}

