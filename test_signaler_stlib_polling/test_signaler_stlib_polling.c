//blinking lights tutorial/walkthrough in book

#include <stm32f10x.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>

#define TEST_PASS 0
#define TEST_DONE 1
#define TEST_FAIL 2

uint8_t test_result;

uint8_t state_num;
uint8_t pulse_state;
uint16_t in_pulse_len;

void Delay(uint32_t nTime);

void send_pulse(void);
void judge_pulse(void);
void measure_in_pulse(void);
void led_out_setup(void);
void blink_led(void);


int main(void)
{
    GPIO_InitTypeDef	GPIO_InitStructure;

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

    GPIO_StructInit(&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 + GPIO_Pin_8 + GPIO_Pin_6; //adding both GPIOs!
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_2MHz;
    GPIO_Init(GPIOC,	&GPIO_InitStructure);
	//manually
	
	//GPIOC_CRH |= (1<<1); 	
    //GPIOC->CRH |= (1<<1);
	//GPIOC->CRH = GPIO_CRH_MODE8_1;	
	
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
    
    //setup input pulse pin
    GPIO_StructInit(&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    
	// Configure SysTick Timer
	/*(3)*/
	if (SysTick_Config(SystemCoreClock / 1000))
			while (1);

	int Pressed = 0;
    int Pressed_Confidence_Level = 0; //Measure button press cofidence
    int Released_Confidence_Level = 0; //Measure button release confidence

while (1)
{
    if(state_num == 0){
        if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0 ) == 1)
        {
            Pressed_Confidence_Level ++; //Increase Pressed Conficence
            Released_Confidence_Level = 0; //Reset released button confidence since there is a button press
            if (Pressed_Confidence_Level >500)   //Indicator of good button press
            {
                if (Pressed == 0)
                {
                    //PORTB ^= 1 << PINB0;
                    //PORTB ^= 1 << PINB2;
                    send_pulse();
                    state_num = 1;
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
    else if(state_num == 1){
        in_pulse_len = 0;
        pulse_state = 1;
        measure_in_pulse();
    }
    else if(state_num == 2){
        judge_pulse();
    }
    else if(state_num == 3){
        led_out_setup();
    }
    else if(state_num == 4){
        blink_led();
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
    GPIO_WriteBit(GPIOC, GPIO_Pin_6, Bit_SET);
    
    Delay(10);
    
    GPIO_WriteBit(GPIOC, GPIO_Pin_6, Bit_RESET);

}

void measure_in_pulse(void){
    if(pulse_state == 1){
    while(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_7) == 1){
        
        in_pulse_len++;
        Delay(1);
    }
    pulse_state = 0; 
    } else if (pulse_state == 0){
        state_num = 2;
    }
}

void judge_pulse(void){
    
    if ((in_pulse_len >8) && (in_pulse_len < 12)){
        test_result = TEST_PASS;
    }
    else if ((in_pulse_len >98) && (in_pulse_len < 102)){
        test_result = TEST_FAIL;
    }
    else if ((in_pulse_len >23) && (in_pulse_len < 27)){
        test_result = TEST_DONE;
    }
    state_num = 3;
}

void led_out_setup(void){

uint8_t to_next_state = 0;

    switch (test_result){
        case TEST_PASS:
            GPIO_WriteBit(GPIOC, GPIO_Pin_9, Bit_SET);
            GPIO_WriteBit(GPIOC, GPIO_Pin_8, Bit_RESET);
            break;
        case TEST_FAIL:
            GPIO_WriteBit(GPIOC, GPIO_Pin_8, Bit_SET);
            GPIO_WriteBit(GPIOC, GPIO_Pin_9, Bit_RESET);
            break;
        case TEST_DONE:
            to_next_state = 1;
            break;
    
    }
    state_num = (to_next_state) ? 4 : 1;

}

void blink_led(void){
    GPIO_WriteBit(GPIOC, GPIO_Pin_9, Bit_SET);
    Delay(250);
    GPIO_WriteBit(GPIOC, GPIO_Pin_9, Bit_RESET);
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t* file, uint32_t line)
{

	/* Infinite loop */
	/* Use GDB to find out why we're here */
	while (1);
}
#endif

