#ifndef RC4_HPP
#define RC4_HPP

class RC4 {
private:
	unsigned char state[256];
	int x, y;

public:
	RC4(){}

	void init(unsigned char key[], int length) {
		int i;
		int j = 0;
		for (i=0; i < 256; i++) 
			state[i] = i;	

		for (i=0; i < 256; i++) {
			j = (j + state[i] + key[i % length]) & 0xff;
			swap(i,j);
		}
	}

	unsigned char nextByte(void) {
		x = (x + 1) & 0xff;
		y = (y + state[x]) & 0xff;
		swap(x, y);
		int idx = (state[x] + state[y]) & 0xff;
		return state[idx];
	}

	long nextLong(void) {
		unsigned char n0 = nextByte();
		unsigned char n1 = nextByte();
		unsigned char n2 = nextByte();
		unsigned char n3 = nextByte();	
		unsigned long r = n0 + (n1 << 8) + (n2 << 16) + ((n3 <<24) & 0xffffffff);
		return ((r ^ 0x80000000) - 0x80000000);
	}

private:
	void swap(int i, int j) {
		unsigned char t = state[i];
		state[i] = state[j];
		state[j] = t;	
	}
};

#endif
