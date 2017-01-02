#include <stdio.h>

int counter = 0;

void test() {
	counter++;
	printf("test %d\n", counter);
}

int main() {
	int i;
	printf("testy test\n");
	for (i = 0; i < 5; i++) {
		test();
	}
	return 0;
}