#define FLAG 0x7e
#define A 0x03
#define C_UA 0x01

unsigned char g_ua[5];
g_ua[0] = FLAG;
g_ua[1] = A;
g_ua[2] = C_UA;
g_ua[3] = ua[1]^ua[2];



int main(){
	int state = 0;
	unsigned char c;
	while(state != 5){
		switch(state){
			case 0:
			if (c == FLAG){
				state=1;
			}
			break;
			case 1:
			break;
			case 2:
			break;
			case 3:
			break;
			case 4:
			break;
			case 5:
			break;
		
		}
	}


}
