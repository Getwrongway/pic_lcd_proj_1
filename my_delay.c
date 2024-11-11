#include <delays.h>

void delay(int t){
	for(int i = 0; i < t; i++)
        Delay1KTCYx(1);
}