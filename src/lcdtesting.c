/**
  ******************************************************************************
  * @file    main.c
  * @author  Ac6
  * @version V1.0
  * @date    01-December-2013
  * @brief   Default main function.
  ******************************************************************************
*/

#include "stm32f0xx.h"
#include "stm32f0_discovery.h"
#include "Sudoku_problem.h"
#include "Sudoku_solution.h"
#include "sinewave.h"
#include "message.h"


#define DISPLAY_ON 0x3f
#define DISPLAY_OFF 0x3e
#define GPIOA_ODR_RS 0x1<<10
#define CHIP_SELECT_1 0x1<<11
#define CHIP_SELECT_2 0x1<<12
#define GPIOA_ODR_RW 0x1<<9
#define GPIOA_ODR_E 0x1<<11
#define GPIOA_ODR_DB7 0x1<<7
#define GPIOA_ODR_DB6 0x1<<6
#define GPIOA_ODR_DB5 0x1<<5
#define GPIOA_ODR_DB4 0x1<<4 //PA4 no longer correspond to this bit as PA4 is now used for the DAC, this bit is now controlled by PA8
#define GPIOA_ODR_DB3 0x1<<3
#define GPIOA_ODR_DB2 0x1<<2
#define GPIOA_ODR_DB1 0x1<<1
#define GPIOA_ODR_DB0 0x1<<0
#define PIXEL_LENGTH 7
#define PIXEL_WIDTH 5
#define BLANK -1
#define CURSOR 10
#define RES 11
#define INPUT_LENGTH 7
#define MES_WIDTH 13
#define MES_HEIGHT 5
int compare_solution(void);
int compare_cheat(void);
void nanowait(int t);
void transfer_data(void);
int character_bit(int data, int col);
void dac_init(void);
void tim6_init(void);
void dma3_init(void);
void tim3_init(void);//
void unable_dac (int duration);
void message2mem(void);
int xaxis;
int yaxis;
int prob_num = 0;
int i=0;

int user_input_index = 0;
char user_input[INPUT_LENGTH] = {0};
char display_mem[64][64] = {0};
int sudoku_board[9][9] = {0};
int prob_num_max = (sizeof(sudoku_prob) / sizeof(sudoku_prob[0])) / 81;
int message[MES_HEIGHT][MES_WIDTH] = {0};
void message_init(void)
{
	for (int i = 0; i < MES_HEIGHT; i++)
	{
		for (int j = 0; j < MES_WIDTH; j ++)
		{
			message[i][j] = message_def[i][j];
		}
	}
}
void sudoku2mem()
{
	for (int i = 0; i < 64 ;i++)
	{
		for (int j = 0; j< 64; j ++)
		{
			display_mem[i][j] = 0;
		}
	}//clear the board
	for (int i = 0; i < 64; i ++)
	{
		for (int j = 0; j < 64; j ++)
		{
			if ((character_bit(sudoku_board[i/PIXEL_LENGTH][j/PIXEL_WIDTH], j % PIXEL_WIDTH) & (1 << (i % PIXEL_LENGTH)) ) && (i <= (3*PIXEL_LENGTH)*3 && j <= (3 * PIXEL_WIDTH)*3))
			{
				display_mem[i][j] = 1;
			}
			else
			{
				display_mem[i][j] = 0;
			} //Filling the numbers, dont ask how, I don't even know...
			if (((i % (3 * PIXEL_LENGTH) == 0) || (j % (3 * PIXEL_WIDTH) == 0)) && (i <= (3*PIXEL_LENGTH)*3 && j <= (3 * PIXEL_WIDTH)*3))
			{
				display_mem[i][j] = 1;
			} //Drawing the board border
		}
	}

	for (int i = 0; i < PIXEL_LENGTH; i++)
	{
		for (int j = 0; j < PIXEL_WIDTH; j++)
		{
			display_mem[yaxis * PIXEL_LENGTH + i][xaxis * PIXEL_WIDTH + j] ^= 1;
		}
	}//Display the Cursor
} //This function convert the sudoku_board array into display_mem for the LCD to use

void message2mem(void)
{
	for (int i = 0; i < 64; i ++)
	{
		for (int j = 0; j < 64; j++)
		{
			display_mem[i][j] = 0;
		}
	}
	message[1][12] = prob_num;
	switch(prob_num)
	{
		case 0:
		case 1:
		case 2:
			message[2][11] = 'E';
			break;
		case 3:
			message[2][11] = 'M';
			break;
		case 4:
			message[2][11] = 'H';
			break;
		case 5:
			message[2][11] = 'V';
			message[2][12] = 'H';
			break;
	}
	for (int i = 0; i < (64 / 8) * MES_HEIGHT - (MES_HEIGHT - 1); i ++) //idk??
		{
			for (int j = 0; j < 64; j ++)
			{
				if ((character_bit(message[i/PIXEL_LENGTH][j/PIXEL_WIDTH], j % PIXEL_WIDTH) & (1 << (i % PIXEL_LENGTH)) ) )
				{
					display_mem[i][j] = 1;
				}
				else
				{
					display_mem[i][j] = 0;
				} //Filling the numbers, dont ask how, I don't even know...
			}
		}

}

void chip_init(int chip_num)
{
	//selecting chip
	GPIOB->ODR &= ~CHIP_SELECT_1;
	GPIOB->ODR &= ~CHIP_SELECT_2;
	if (chip_num % 2 == 1)
	{
		GPIOB->ODR |= CHIP_SELECT_1;
	}
	else
	{
		GPIOB->ODR |= CHIP_SELECT_2;
	}
	//clear RS and RW
	GPIOA->ODR &= ~(GPIOA_ODR_RS);
	GPIOA->ODR &= ~(GPIOA_ODR_RW);
	//Set data bit for display to turn on
	GPIOA->ODR &= ~0xff; //clear the data bits
	GPIOA->ODR |= DISPLAY_ON | (((1 << 4) & DISPLAY_ON) << 4);
	//Transfer data
	transfer_data();

}

void chip1_off(void)
{
	//clear RS and RW
	GPIOA->ODR &= ~(GPIOA_ODR_RS);
	GPIOA->ODR &= ~(GPIOA_ODR_RW);
	//Set data bit for display to turn on
	GPIOA->ODR &= ~0xff; //clear the data bits
	GPIOA->ODR |= DISPLAY_OFF| (((1 << 4) & DISPLAY_OFF) << 4);
	//Transfer data
	transfer_data();
}
void transfer_data(void)
{
	nanowait(8000);
	GPIOA->ODR |= GPIOA_ODR_E;
	nanowait(8000);
	GPIOA->ODR &= ~GPIOA_ODR_E;
	nanowait(8000);

}

void set_X(int X)
{
	if (X >= 8)
	{
		X = 7;
	}
	else if (X < 0)
	{
		X = 0;
	}
	GPIOA->ODR &= ~(GPIOA_ODR_RS);
	GPIOA->ODR &= ~(GPIOA_ODR_RW);

	GPIOA->ODR &= ~0xff; //clear the data bits
	GPIOA->ODR |= GPIOA_ODR_DB7 | GPIOA_ODR_DB5 | GPIOA_ODR_DB4 | GPIOA_ODR_DB3;
	GPIOA->ODR |= X | (((1 << 4) & GPIOA_ODR_DB4) << 4);
	transfer_data();
}

void set_Y(int Y)
{
	if (Y >= 63)
	{
		Y = 63;
	}
	else if (Y < 0)
	{
		Y = 0;
	}
	GPIOA->ODR &= ~(GPIOA_ODR_RS);
	GPIOA->ODR &= ~(GPIOA_ODR_RW);

	GPIOA->ODR &= ~0xff; //clear the data bits
	GPIOA->ODR |= GPIOA_ODR_DB6;
	GPIOA->ODR |= Y | (((1 << 4) & Y) << 4);

	transfer_data();
}
void display_startline(int line_num)
{
	GPIOA->ODR &= ~(GPIOA_ODR_RS);
	GPIOA->ODR &= ~(GPIOA_ODR_RW);
	GPIOA->ODR &= ~0xff; // clear the data bits
	GPIOA->ODR |= GPIOA_ODR_DB7 | GPIOA_ODR_DB6;
	//Set data bit for the startline to be all 0s
	//GPIOA->ODR &= ~(GPIOA_ODR_DB5 | GPIOA_ODR_DB4 | GPIOA_ODR_DB3 | GPIOA_ODR_DB2 | GPIOA_ODR_DB1 | GPIOA_ODR_DB0);
	GPIOA->ODR |= line_num | (((1 << 4) & line_num) << 4);

	transfer_data();
}


void GPIOA_GPIOB_init(void)
{
	//enable GPIOA
	 RCC-> AHBENR |= RCC_AHBENR_GPIOAEN;
		//enable GPIOB
	 RCC-> AHBENR |= RCC_AHBENR_GPIOBEN;
		//PA0-12 are outputs
	 for(int i = 0; i <= 12; i++)
	 {
		 GPIOA->MODER &= ~(0x03<<(2*i));
		 GPIOA->MODER |= (0x01<<(2*i));
	 }
		//PB11/12 are outputs
	 GPIOB->MODER &= ~(0x03<<(2*11));
	 GPIOB->MODER &= ~(0x03<<(2*12));
	 GPIOB->MODER |= (0x01<<(2*11));
	 GPIOB->MODER |= (0x01<<(2*12));
}

void write_data(int data)
{
	GPIOA->ODR |= (GPIOA_ODR_RS); //set RS to 1
	GPIOA->ODR &= ~(GPIOA_ODR_RW); //set RW to 0
	GPIOA->ODR &= ~(0xff);
	GPIOA->ODR |= data  | (((1 << 4) & data) << 4);

	transfer_data();

}

void nanowait(int t) {
    asm("       mov r0,%0\n"
        "repeat:\n"
        "       sub r0,#83\n"
        "       bgt repeat\n"
        : : "r"(t) : "r0", "cc");
}
/*    4-11. Data pins - PA0-7,except for data pin 8 (data bit 4), data pin 8 correspond to PA8
      12. CS1 PB11
      13. CS2 PB12
      14. reset PA8
      15. R/W PA9
      16. D/I? PA10 RS
      17. Enable PA11
      18. Vee PA12*/

void display_sudoku(void)
{
	int data = 0;
	sudoku2mem();
	set_Y(0);
	for (int i = 0; i < 8; i++)
	{
		set_X(i);
		for (int j = 0; j < 64; j ++)
		{
			data = 0;
			for (int h = 0; h < 8; h++)
			{
				data |= display_mem[i * 8 + h][j] << h; //Idk man, my head hurts writing this
			}
			write_data(data);
		}
	}
} //Display the sudoku to the LCD

void display_message(void)
{
	int data = 0;
	message2mem();
	set_Y(0);
	for (int i = 0; i < 8; i++)
	{
		set_X(i);
		for (int j = 0; j < 64; j ++)
		{
			data = 0;
			for (int h = 0; h < 8; h++)
			{
				data |= display_mem[i * 8 + h][j] << h; //Idk man, my head hurts writing this
			}
			write_data(data);
		}
	}
}
void init_grid(int prob_num)
{
	int off_set = prob_num * 81;
	for (int i = 0; i < 9; i++)
	{
		for (int j = 0; j < 9; j++)
		{
			if (sudoku_prob[off_set + i * 9 + j] != 0)
			{
				sudoku_board[i][j] = sudoku_prob[off_set + i * 9 + j];
			}
			else
			{
				sudoku_board[i][j] = BLANK;
			}
		}
	}
	return;
} //This function fill the sudoku_board with the problem given in the sudoku_problem.h file depending on the problem number

void clear_display(void)
{
	for (int x = 0; x < 8; x ++)
	{
		set_X(x);
		for (int y = 0; y <64; y ++ )
		{
			write_data(0);
		}
	}
	set_X(0);
	set_Y(0);
} //Clear the display, set the cursor back to 0,0

void adc_init (void){
	RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
	GPIOB->MODER |= 0xF;
	RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
	RCC->CR2 |= RCC_CR2_HSI14ON;
	ADC1->CFGR1 &= ~ADC_CFGR1_RES; //12 bit resolution
	while(!(RCC->CR2 & RCC_CR2_HSI14RDY));
	ADC1->CR |= ADC_CR_ADEN;
	while(!(ADC1->ISR & ADC_ISR_ADRDY));
	while((ADC1->CR & ADC_CR_ADSTART));
}

void adc_cursor_read(void)
{
	ADC1->CHSELR = 0;
	ADC1->CHSELR |= 1 << 8;
	while(!(ADC1->ISR & ADC_ISR_ADRDY));
	ADC1->CR |= ADC_CR_ADSTART;
	while(!(ADC1->ISR & ADC_ISR_EOC));

	xaxis = (int) ADC1->DR * 8.5 / (3103.0);


	ADC1->CHSELR = 0;
	ADC1->CHSELR |= 1 << 9;
	while(!(ADC1->ISR & ADC_ISR_ADRDY));
	ADC1->CR |= ADC_CR_ADSTART;
	while(!(ADC1->ISR & ADC_ISR_EOC));

	yaxis = (int) ADC1 ->DR * 8.5 / (3103.0);
	//Maximum voltage level is only 3.2 because of the voltage divider (otherwise 4.6 wouldn't work, exceed 4.0 max ADC input voltage
	//Hence the interpretation of the ADC->DR need to change to scale properly
}

static int row = 0;
void scan_keypad(void)
{
	for (row = 1; row < 5; row ++)
	{
		GPIOB->BSRR = 1 << (row +3);
		nanowait(10000000);
		GPIOB->BRR = 0xf0;
		nanowait(1000000);
	}
}

void init_keypad()
{
	RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
	RCC->AHBENR |= RCC_AHBENR_GPIOCEN;

	GPIOB->MODER &= ~(0x3 << (2 * 4));
	GPIOB->MODER |= 0x1 << (2 * 4);
	GPIOB->MODER &= ~(0x3 << (2 * 5));
	GPIOB->MODER |= 0x1 << (2 * 5);
	GPIOB->MODER &= ~(0x3 << (2 * 6));
	GPIOB->MODER |= 0x1 << (2 * 6);
	GPIOB->MODER &= ~(0x3 << (2 * 7));
	GPIOB->MODER |= 0x1 << (2 * 7);

	GPIOB->ODR &= ~0x000000f0;

	GPIOC->MODER &= ~(3<<(2*7));
	GPIOC->MODER |= 2<<(2*7);

	GPIOC->MODER &= ~(3<<(2*8));
	GPIOC->MODER |= 2<<(2*8);

	GPIOC->MODER &= ~(3<<(2*9));
	GPIOC->MODER |= 2<<(2*9);

	GPIOC->PUPDR &= ~GPIO_PUPDR_PUPDR7;
	GPIOC->PUPDR |= GPIO_PUPDR_PUPDR7_1;

	GPIOC->PUPDR &= ~GPIO_PUPDR_PUPDR8;
	GPIOC->PUPDR |= GPIO_PUPDR_PUPDR8_1;

	GPIOC->PUPDR &= ~GPIO_PUPDR_PUPDR9;
	GPIOC->PUPDR |= GPIO_PUPDR_PUPDR9_1;

	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
	TIM3->PSC = 1 - 1;
	TIM3->ARR = 0xffffffff;

	//2
	TIM3->CCMR1 &= ~TIM_CCMR1_CC2S;
	TIM3->CCMR1 |= TIM_CCMR1_CC2S_0;
	TIM3->CCMR1 |= TIM_CCMR1_IC2F_3 | TIM_CCMR1_IC2F_2 | TIM_CCMR1_IC2F_1 | TIM_CCMR1_IC2F_0;
	TIM3->CCMR1 &= ~TIM_CCMR1_IC2PSC;

	TIM3->CCER &= ~(TIM_CCER_CC2P|TIM_CCER_CC2NP);
	TIM3->CCER |= TIM_CCER_CC2E;
	TIM3->DIER |= TIM_DIER_CC2IE;

	//3
	TIM3->CCMR2 &= ~TIM_CCMR2_CC3S;
	TIM3->CCMR2 |= TIM_CCMR2_CC3S_0;
	TIM3->CCMR2 |= TIM_CCMR2_IC3F_3 | TIM_CCMR2_IC3F_2 | TIM_CCMR2_IC3F_1 | TIM_CCMR2_IC3F_0;
	TIM3->CCMR2 &= ~TIM_CCMR2_IC3PSC;
	TIM3->CCER |= TIM_CCER_CC3E;
	TIM3->DIER |= TIM_DIER_CC3IE;
	//4
	TIM3->CCMR2 &= ~TIM_CCMR2_CC4S;
	TIM3->CCMR2 |= TIM_CCMR2_CC4S_0;
	TIM3->CCMR2 |= TIM_CCMR2_IC4F_3 | TIM_CCMR2_IC4F_2 | TIM_CCMR2_IC4F_1 | TIM_CCMR2_IC4F_0;
	TIM3->CCMR2 &= ~TIM_CCMR2_IC4PSC;
	TIM3->CCER |= TIM_CCER_CC4E;
	TIM3->DIER |= TIM_DIER_CC4IE;

	TIM3->CR1 |= TIM_CR1_CEN;

	NVIC->ISER[0] = 1 << TIM3_IRQn;

}

void input(int input)
{
	int offset = 81 * prob_num;
	if (sudoku_prob[offset + yaxis * 9 + xaxis] == 0)
	{
		sudoku_board[yaxis][xaxis] = input;
		if (input == 3 || (input == RES))
		{
			user_input_index = 0;
		}
		user_input[user_input_index % INPUT_LENGTH] = input;
		user_input_index ++;
	}
}

void TIM3_IRQHandler()
{
	if ((TIM3->SR & TIM_SR_UIF) != 0)
	{
		TIM3->SR &= ~TIM_SR_UIF;
		return;
	}
	if (TIM3->SR & TIM_SR_CC2IF)
	{
		switch(row)
		{
			case 1:
				input(1);
				break;
			case 2:
				input(4);
				break;
			case 3:
				input(7);
				break;
			case 4:
				input(RES);
				break;
		}
	}
	if (TIM3->SR & TIM_SR_CC3IF)
	{
		switch(row)
		{
			case 1:
				input(2);
				break;
			case 2:
				input(5);
				break;
			case 3:
				input(8);
				break;
			case 4:
				input(0);
				break;
		}
	}
	if (TIM3->SR & TIM_SR_CC4IF)
	{
		switch(row)
		{
			case 1:
				input(3);
				break;
			case 2:
				input(6);
				break;
			case 3:
				input(9);
				break;
			case 4:
				input(BLANK);
				break;
		}
	}


	nanowait(10 * 1000 * 1000);
	while ((GPIOC->IDR & (0x380)) != 0);
	nanowait(10 * 1000 * 1000);
	int __attribute((unused)) useless;
	useless = TIM3->CCR2;
	useless = TIM3->CCR3;
	useless = TIM3->CCR4;
	return;
}

int compare_solution()
{
	int off_set = prob_num * 81;
	for (int i = 0; i < 9; i++)
	{
		for (int j = 0; j < 9; j++)
		{
			if (sudoku_sol[off_set + i * 9 + j] != sudoku_board[i][j])
			{
				return 0;
			}
		}
	}
	return 1;
}

void check_cheat()
{
	int off_set = 81 * prob_num;
	if (compare_cheat())
	{
		for (int i = 0; i < 9; i ++)
		{
			for (int j = 0; j < 9; j++)
			{
				sudoku_board[i][j] = sudoku_sol[off_set + i * 9 + j];
				display_sudoku();
				nanowait(100000000);
			}
		}
	}
}

int compare_cheat(void)
{
	const int cheat_code[INPUT_LENGTH] = {3,6,2,2,0,1,8};
	for (int i = 0; i < INPUT_LENGTH; i++)
	{
		if (user_input[i] != cheat_code[i])
		{
			return 0;
		}
	}
	return 1;
}

int compare_reset(void)
{

	for (int i = 0; i < INPUT_LENGTH - 1; i++)
	{
		if (user_input[i] == RES)
		{
			for (int j = 0; j < INPUT_LENGTH; j++)
			{
				user_input[j] = 0;
			}
			prob_num = (prob_num + 1) % prob_num_max;
			return 1;
		}
	}
	return 0;
}

void check_win()
{
	if (compare_solution() == 1)
	{
		dac_init();
		tim6_init();
		dma3_init();
	    tim3_init();//
	    unable_dac(10000000);
		chip_init(2);
		set_X(0);
		set_Y(0);
		clear_display();
		message_init();
	    for (int i = 0; i < MES_WIDTH; i++)
	    {
	    	message[3][i] = message_win[i];
	    }
	    display_message();
	    chip_init(1);
	}
}
void dac_init(void) {
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;// dac is in channel 1, channel 1 is in pa4
	GPIOA->MODER &= ~(3<< 2 * 4);
	GPIOA->MODER |= 3<< 2 * 4;
    RCC -> APB1ENR |= RCC_APB1ENR_DACEN;

    DAC -> CR |= DAC_CR_DMAEN1;//dac channel1 DMA enabled
	DAC -> CR |= DAC_CR_EN1;//dcan channel1 enable
	DAC -> CR |= DAC_CR_TEN1;//trigger enable
}

void tim6_init(void) {
	RCC->APB1ENR |= RCC_APB1ENR_TIM6EN;
	TIM6->PSC = 1 - 1;
	TIM6->ARR = 202 - 1;
	TIM6->CR2 |= TIM_CR2_MMS_1;
	TIM6->CR1 |= TIM_CR1_ARPE;
	TIM6 -> CR1  |= TIM_CR1_CEN ;
	 return;
}
int t6_dir = 1;
int t6_offset = 0;
int t6_max = 20;
int t6_min = -20;

void TIM2_IRQHandler(void) {
	if(TIM6->ARR == 201) {
		nanowait(100000000);
		TIM6->ARR = 302;
	}
	else if (TIM6->ARR == 302){
		nanowait(100000000);
		TIM6->ARR =  169;
	}
	else {
		nanowait(100000000);
		TIM6->ARR =  201;
	}
	if ((TIM2->SR & TIM_SR_UIF) != 0) { // Check for overflow.
		// Clear the UIF to prevent endless IRQ.
		TIM2->SR &= ~TIM_SR_UIF;
	}
	return;
}

void tim3_init(void) {
	 RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
	 // Set prescaler output to 6MHz (48MHz/48)
	 TIM2->PSC = 48 - 1;

	 TIM2->ARR = 2000 - 1;


	 TIM2 -> DIER |= TIM_DIER_UIE;
	 TIM2 -> CR1  |= TIM_CR1_CEN ;
	// TIM3->CR2|=TIM_CR1_CKD_0;
	 NVIC->ISER[0] = 1 << TIM2_IRQn;
	 return;
}

void unable_dac (int duration){

	nanowait(duration);
	DAC -> CR &=~ DAC_CR_EN1;
}


void dma3_init(void) {
	RCC->AHBENR |= RCC_AHBENR_DMA1EN; /* (1) */
	//DMA1_Channel3 -> CCR &= ~DMA_CCR_EN;
	DMA1_Channel3->CMAR = (uint32_t) sine_table;
	DMA1_Channel3->CPAR = (uint32_t) (&(DAC->DHR12R1)); /* (3) */


	DMA1_Channel3->CNDTR = sizeof(sine_table)/sizeof(sine_table[0]); /* (5) */

	DMA1_Channel3->CCR |= DMA_CCR_DIR;
	DMA1_Channel3->CCR |= DMA_CCR_MSIZE_0;
	DMA1_Channel3->CCR |= DMA_CCR_PSIZE_0;
	DMA1_Channel3->CCR |= DMA_CCR_MINC;
	DMA1_Channel3->CCR &= ~DMA_CCR_PL;
	DMA1_Channel3->CCR |= DMA_CCR_CIRC;
	DMA1_Channel3 ->CCR |= DMA_CCR_EN;
}



int main(void) {
	LOOP:GPIOA_GPIOB_init();

	chip_init(1);
	display_startline(0);
	set_X(0);
	set_Y(0);
	clear_display();

	chip_init(2);
	set_X(0);
	set_Y(0);
	clear_display();
	message_init();
	display_message();
	chip_init(1);

	init_grid(prob_num);

	init_keypad();
	adc_init();


	while(1)
	{
		adc_cursor_read();
		scan_keypad();
		display_sudoku();
		check_win();
		check_cheat();


		if (compare_reset())
		{
			goto LOOP;
		}
	}

}



