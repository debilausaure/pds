#include <stdio.h>

unsigned long arrondi512(unsigned long n) {
	unsigned long arrondi = (n >> 9) << 9;
	if (n != arrondi) {
		arrondi+=512;
	}
	return arrondi;
}

int main(void) {
	printf("Arrondi 1025 = %d\n", arrondi512(1025));
	return 0;
}
