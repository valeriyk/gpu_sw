#include <stdio.h>
#include <stdint.h>


int64_t edge (uint16_t ax, uint16_t ay, uint16_t bx, uint16_t by, uint16_t cx, uint16_t cy) {
	
	int32_t bxax = (int32_t) (bx-ax);
	int32_t cyay = (int32_t) (cy-ay);
	int32_t byay = (int32_t) (by-ay);
	int32_t cxax = (int32_t) (cx-ax);
	int64_t m0 = (int64_t) bxax * (int64_t) cyay; 
	int64_t m1 = (int64_t) byay * (int64_t) cxax;
	return  m0 - m1;
}

int main (void) {

	int64_t max = 0, min = 0;
	uint8_t max_num = 0, min_num = 0;

	for (int i = 0; i < 64; i++) {
		uint16_t ax = (i & 0x01) ? 2047 : 0;
		uint16_t ay = (i & 0x02) ? 2047 : 0;
		uint16_t bx = (i & 0x04) ? 2047 : 0;
		uint16_t by = (i & 0x08) ? 2047 : 0;
		uint16_t cx = (i & 0x10) ? 2047 : 0;
		uint16_t cy = (i & 0x20) ? 2047 : 0;
		
		int64_t res = edge (ax, ay, bx, by, cx, cy);
		if (res > max) {
			max = res;
			max_num = i;
		}
		if (res < min) {
			min = res;
			min_num = i;
		}
	}
	printf ("min=%d (N%d), max=%d (N%d)\n", min, min_num, max, max_num);

	return 0;
}
