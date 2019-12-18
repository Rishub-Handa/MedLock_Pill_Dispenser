#include <stdio.h>
#include <stdint.h>

void print_uint64_t(uint64_t num); 

int main(int argc, char **argv) {
    print_uint64_t(1572210519468); 
}

void print_uint64_t(uint64_t num) {

    char rev[128]; 
    char *p = rev+1;
    char str[18]; 


    while (num > 0) {
        *p++ = '0' + ( num % 10);
        num/= 10;
    }
    p--;
    int i = 0; 
    while (p > rev) {
        str[i] = *p--; 
        i++; 
    }
    printf("%s", str); 
}