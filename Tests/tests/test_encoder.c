#include <stdio.h>

unsigned long encoded(unsigned long value, int code); 

int main() {

enum eventsId { btn1, btn2, btn3, capTurn, collarDisengage, dispense }; 

printf("%d \n", btn2); 

printf("%lu \n", encoded(123456, btn2)); 


}

// 4 bit encoding 
unsigned long encoded(unsigned long value, int code) { 

long mask = ~0u << 4; 
value &= mask;
value |= code; 

return value; 	

}
