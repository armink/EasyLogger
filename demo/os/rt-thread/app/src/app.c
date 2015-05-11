#include "app_task.h"

int main(void){
	/* disable interrupt first */
	rt_hw_interrupt_disable();

	/* startup RT-Thread RTOS */
	rtthread_startup();

	return 0;
}


