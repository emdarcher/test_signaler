//send out a pulse and then compture it with another timer.
//display the response result on LEDs

#include <stm32f10x.h>

#define GREEN_LED (1<<9)
#define BLUE_LED (1<<8)

#define TEST_PASS 0
#define TEST_FAIL 1
#define TEST_DONE 2
#define TEST_ERROR 3

void Delay(uint32_t nTime);

void init_timers(void);
void init_tim3(void);
void init_LEDs(void);

void set_pwm_G_LED(uint8_t pulse_width);
void set_pwm_B_LED(uint8_t pulse_width);

void send_pulse_tim2(uint8_t pulse_ms);
void get_pulse_ms_tim4(void);

void judge_pulse(void);
void set_LEDs(void);

volatile uint16_t pulse_ms_tim4;
volatile uint16_t pulse_store;

volatile uint8_t test_result;

int main(void)
{
    
    init_timers();
    //init_tim3();
    init_LEDs();
    init_tim3();
    
    //init_SPI1();
    //init_digit_pins();
    
    // Configure SysTick Timer
    if (SysTick_Config(SystemCoreClock / 1000))
        while (1);
    
    while (1)   {
        //int i;
       
        //while((TIM2->SR & TIM_SR_UIF) == 0); //wait till done sending pulse
        
        //set_pwm_G_LED(128);
        
        //pulse_store = TIM4->CCR2;
        //get_pulse_ms_tim4();
        
        //judge_pulse();
        //set_LEDs();
    }
}

// Timer code
/*(5)*/

void init_timers(void) {
    
    
    //-----------CLOCK SETUPS------------
    
    //setup clock for GPIOB with alternate function IO allowed
    RCC->APB2ENR |= (   RCC_APB2ENR_IOPBEN  |
                        RCC_APB2ENR_AFIOEN  
                        );
                        
    //setup timer 2 clock
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    
    //setup GPIOA
    RCC->APB2ENR |= (   RCC_APB2ENR_IOPAEN  );
    
    //enable TIM4 clock!!!!
    RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;
    
    
    //------------setup GPIO pins------------ 
    
    //PB6-7 input floating, 
    GPIOB->CRL |= ( GPIO_CRL_CNF6_0 );//| GPIO_CRL_CNF7_0 );
    GPIOB->CRL &= ~(GPIO_CRL_CNF6_1 | //GPIO_CRL_CNF7_1 |
                    GPIO_CRL_MODE6  );//| GPIO_CRL_MODE7  );
                    
    //PB8 output push pull, alternate function, 2MHz
    GPIOB->CRH |= ( GPIO_CRH_CNF8_1 |
                    GPIO_CRH_MODE8_1);
                    
    GPIOB->CRH &= ~(GPIO_CRH_CNF8_0 |
                    GPIO_CRH_MODE8_0);
                    
    //PA2 Tim2_ch3
    //PUSH-PULL, AFIO, 2MHz
    //GPIOA->CRL |= ( GPIO_CRL_CNF2_1 |
    //                GPIO_CRL_MODE2_1); //50MHz?
    //GPIOA->CRL &= ~(GPIO_CRL_CNF2_0 |
    //                GPIO_CRL_MODE2_0);
    
    //PA1 tim2_ch2
    //PUSH-PULL, AFIO, 2MHz
    GPIOA->CRL |= ( GPIO_CRL_CNF1_1 |
                    GPIO_CRL_MODE1_1);
    GPIOA->CRL &= ~(GPIO_CRL_CNF1_0 |
                    GPIO_CRL_MODE1_0);
    
    //PA0 input floating for trigger
    GPIOA->CRL |= ( GPIO_CRL_CNF0_0);
    GPIOA->CRL &= ~(GPIO_CRL_CNF0_1 |
                    GPIO_CRL_MODE0  );
    
    
    //PA3 TIM15_CH2 for input trigger
    //GPIOA->CRL |= ( GPIO_CRL_CNF3_0);
    //GPIOA->CRL &= ~(GPIO_CRL_CNF3_1 |
    //                GPIO_CRL_MODE3  );
    
    
    //-----------setup TIM2--------------------
    
    //timer 2 clock 24,000,000/1,000 = 24,000 so prescaler 24,000
    //to get 1,000Hz timer clock
    TIM2->PSC = (uint16_t)(23999); //23999 + 1 = 24000
    TIM2->ARR = (uint16_t)(255); //0 to 255 ms
    
    //setup PWM mode two, OCIM 111, OC2 is high when CNT < CCR3
    TIM2->CCMR1 |= TIM_CCMR1_OC2M;
    
    //enable CCR2 output OC2
    TIM2->CCER |= ( TIM_CCER_CC2E );
    
    //setup CC1 input channel to IC1 mapped to TI1
    TIM2->CCMR1 |= ( TIM_CCMR1_CC1S_0 );
    //trigger selection
    TIM2->SMCR |= (TIM_SMCR_TS_2 | TIM_SMCR_TS_0 );
    
    //trigger slave mode
    TIM2->SMCR |= (TIM_SMCR_SMS_2 | TIM_SMCR_SMS_1);
    
    //one pulse mode
    TIM2->CR1 |= ( TIM_CR1_OPM );
    
    //setup CCR2 so (ARR-CCR2)= desired pulse
    TIM2->CCR2=(uint16_t)(245);

    //---------setup TIM4 stuff----------------
    
    //using PWM input mode from RM0041 page 292
    
    TIM4->PSC = (uint16_t)(23999); //to get 1,000Hz timer clock
    
    //setup CCR1 and CCR2 to go to TI1
    TIM4->CCMR1 |= (TIM_CCMR1_CC1S_0 |
                    TIM_CCMR1_CC2S_1 );
    
    //setup CCR2 to activate on falling edge
    TIM4->CCER |= ( TIM_CCER_CC2P   );
    
    //setup the timer input triggering for TI1
    TIM4->SMCR |= ( TIM_SMCR_TS_2 | TIM_SMCR_TS_0 );
    TIM4->SMCR &= ~(TIM_SMCR_TS_1 );
    
    //slave mode selection to reset mode
    TIM4->SMCR |= ( TIM_SMCR_SMS_2 );
    TIM4->SMCR &= ~(TIM_SMCR_SMS_1 | TIM_SMCR_SMS_0);
    
    //enable the capture inputs
    TIM4->CCER |= ( TIM_CCER_CC1E | TIM_CCER_CC2E );
    
    //enable TIM4
    TIM4->CR1 |= (  TIM_CR1_CEN );
}

void get_pulse_ms_tim4(void){
    
    while(((TIM4->SR) & TIM_SR_CC2IF) == 0); //wait till capture done
    
    pulse_ms_tim4 = TIM4->CCR2;
}

void send_pulse_tim2(uint8_t pulse_ms){
    
    uint16_t arr_val = TIM2->ARR;
    TIM2->CCR3 = (uint16_t)(arr_val - pulse_ms);
    
    TIM2->CR1 |= TIM_CR1_CEN;
    
}

void judge_pulse(void){
    
    if((pulse_ms_tim4 > 7) && (pulse_ms_tim4 < 13)){
        test_result = TEST_PASS;
    }
    else if((pulse_ms_tim4 > 97) && (pulse_ms_tim4 < 103)){
        test_result = TEST_FAIL;
    }
    else if((pulse_ms_tim4 > 22) && (pulse_ms_tim4 < 28)){
        test_result = TEST_DONE;
    }
    else {
        test_result = TEST_ERROR;
    }
}

void set_LEDs(void){
    
    switch (test_result){
        
        case TEST_PASS:
            set_pwm_G_LED(128); //set green led blinking
            //GPIOC->BRR |= BLUE_LED; //off blue led
            set_pwm_B_LED(0);
            break;
        
        case TEST_FAIL:
            set_pwm_G_LED(0);//set green led off
            //GPIOC->BSRR |= BLUE_LED; //on blue led
            set_pwm_B_LED(255);
            break;
        
        case TEST_DONE:
            set_pwm_G_LED(255); //put green led steady on
            set_pwm_B_LED(0); //put blue led off
            break;
        
        case TEST_ERROR:
            set_pwm_G_LED(0); //put green led off
            set_pwm_B_LED(128); //blink blue led
            break;
            
    }
    
}

void set_pwm_G_LED(uint8_t pulse_width){
    
    TIM3->CCR4 = (uint16_t)(pulse_width);
    
}

void set_pwm_B_LED(uint8_t pulse_width){
    
    TIM3->CCR3 = (uint16_t)(pulse_width);
    
}


void init_LEDs(void){

    //setup clock for GPIOC
    RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;
    //RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
    //----setup GPIOC config----
    
    //PC9 output push-pull 2MHz
    //GPIOC->CRH |= ( GPIO_CRH_MODE9 );
    //with AFIO for TIM3
    GPIOC->CRH |= ( GPIO_CRH_CNF9_1 | GPIO_CRH_MODE9_1 );
    //GPIOB->CRL |= ( GPIO_CRL_CNF1_1 | GPIO_CRL_MODE1_1);
    
    //PC8 output push-pull 2MHz
    //GPIOC->CRH |= ( GPIO_CRH_MODE8_1 );
    //for AFIO
    GPIOC->CRH |= ( GPIO_CRH_CNF8_1 | GPIO_CRH_MODE8_1 );
    //GPIOB->CRL |= ( GPIO_CRL_CNF0_1 | GPIO_CRL_MODE0_1 );
    
    
}

void init_tim3(void){
    
    //setup clock for TIM3
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN; 
    
    //afio enable
    RCC->APB2ENR |= ( RCC_APB2ENR_AFIOEN  );
                        
                        
    //remap TIM3 fully to the GPIOC pins
    AFIO->MAPR |= AFIO_MAPR_TIM3_REMAP;
    
    TIM3->PSC = (uint16_t)(23999); //get 1000Hz
    TIM3->ARR = (uint16_t)(255); //0-255 PWM value
    
    set_pwm_G_LED(132);
    TIM3->CCR3 = 128;
    
    //setup PWM Mode 1 with bits 110 for OC4
    TIM3->CCMR2 |= ( TIM_CCMR2_OC4M_2 | TIM_CCMR2_OC4M_1 );
    //enable CC4 output 
    TIM3->CCER |= TIM_CCER_CC4E;
    
    //setup PWM Mode 1 with bits 110 for OC3
    TIM3->CCMR2 |= ( TIM_CCMR2_OC3M_2 | TIM_CCMR2_OC3M_1 );
    //enable CC3 output 
    TIM3->CCER |= TIM_CCER_CC3E;
    
    //enable TIM3
    TIM3->CR1 |= TIM_CR1_CEN;
    
    //set_pwm_G_LED(132);
    //TIM3->CCR3 = 128;
}

static __IO uint32_t TimingDelay;

void Delay(uint32_t nTime) {
    TimingDelay = nTime;
    while(TimingDelay != 0);
}

void SysTick_Handler(void) {
    if (TimingDelay != 0x00)
        TimingDelay--;
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t* file, uint32_t line)
{

    /* Infinite loop */
    /* Use GDB to find out why we're here */
    while (1);
}
#endif

