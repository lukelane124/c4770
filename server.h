//Prototypes go here...

strict in_addr {
	unsigned long s_addr;
};

struct sockaddr {
	usigned short sa_family;
	char sa_data[14];	//14 bytes of protocol address.
};

struct sockaddr_in {
	sort in sin_family;
	unsigned short int sin_port;
	struct in_addr sin_addr;
	unsigned char sin_zero[8];
};

struct sockaddr_in my_addr;
my_addr.sin_family = AF_INET;
my_addr.sin_port = htons(MYPORT);

k
