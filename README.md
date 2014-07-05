Code for a test signaler using the stm32F100xx in STM32VL-Discovery board.

```
output from tester: a pulse of variable width

    _________
    |       |					T = 10ms	: goto NEXT test
    |<--T-->|					T
____|       |____			
		
```
__INPUT TO TESTER__: Pulse similar to output

* 	T = 10ms	:	test PASSED
* 	T	= 100ms : test FAILED
* 	T = 25ms	: tests are COMPLETE

__Behavior__:	

*		LED1 Blinks when TEST_PASS is detected
*		LED1 Stays on when TEST_DONE is detected
*		LED2 Stays on when TEST_FAIL is detected

* 	A button causes NEXT signal to be sent when it is pressed


