//blinking lights tutorial/walkthrough in book

#include <stm32f10x.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>

void Delay(uint32_t nTime);

void send_pulse(void);

int main(void)
{
//		GPIO_InitTypeDef	GPIO_InitStructure;

	// Enable Peripheral Clocks
	
	/*(1)*/

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);	
	//manual
	//RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;

	//Oh THIS CLOCK!
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	//manual
	//RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
	
	// Configure Pins
	/*(2)*/
/*		
GPIO_StructInit(&GPIO_InitStructure);
GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 + GPIO_Pin_8; //adding both GPIOs!
GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_2MHz;
	*/
	//manually
	
	//GPIOC_CRH |= (1<<1); 	
  //GPIOC->CRH |= (1<<1);
	GPIOC->CRH = GPIO_CRH_MODE8_1;	
	
	//GPIO_Init(GPIOC,	&GPIO_InitStructure);
	//
	
	//button config here I think with a new GPIO_InitStructure thingy
	GPIO_StructInit(&GPIO_InitStructure);
	//make sure to do clock init first?
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	//	GPIOA_CRL |= (1<<2);
	//GPIOA->CRL |= (1<<2);
	//GPIOA->CRL	|= GPIO_CRL_CNF0_0;	

	// Configure SysTick Timer
	/*(3)*/
	if (SysTick_Config(SystemCoreClock / 1000))
			while (1);

	int Pressed = 0;
    int Pressed_Confidence_Level = 0; //Measure button press cofidence
    int Released_Confidence_Level = 0; //Measure button release confidence

    while (1)
    {
        if (bit_is_clear(PINB, 1))
        {
            Pressed_Confidence_Level ++; //Increase Pressed Conficence
            Released_Confidence_Level = 0; //Reset released button confidence since there is a button press
            if (Pressed_Confidence_Level >500)   //Indicator of good button press
            {
                if (Pressed == 0)
                {
                    PORTB ^= 1 << PINB0;
                    PORTB ^= 1 << PINB2;
                    Pressed = 1;
                }
//Zero it so a new pressed condition can be evaluated
                Pressed_Confidence_Level = 0;
            }
        }
        else
        {
            Released_Confidence_Level ++; //This works just like the pressed
            Pressed_Confidence_Level = 0; //Reset pressed button confidence since the button is released
            if (Released_Confidence_Level >500)
            {
                Pressed = 0;
                Released_Confidence_Level = 0;
            }
        }
    }
}

// Timer code
/*(5)*/

static __IO uint32_t TimingDelay;

void Delay(uint32_t nTime){
	TimingDelay = nTime;
	while(TimingDelay != 0);
}

void SysTick_Handler(void){
	if (TimingDelay != 0x00)
		TimingDelay--;
}

void send_pulse(void){
    //send 10ms pulse on PC6
    

}


#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t* file, uint32_t line)
{

	/* Infinite loop */
	/* Use GDB to find out why we're here */
	while (1);
}
#endif

