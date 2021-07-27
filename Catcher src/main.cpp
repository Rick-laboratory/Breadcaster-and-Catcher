#include <iostream>
#include "UDP_Receiver.h"
#include "Message_Interpreter.h"

int main(void) {
	::ShowWindow(::GetConsoleWindow(), SW_HIDE);

	MI MI_Object;
	UDP_Receiver UDP(&MI_Object);
	if (UDP.UDP_Receiver_INIT() != 0)
		return EXIT_FAILURE;
	if (UDP.UDP_Receiver_BIND() != 0)
		return EXIT_FAILURE;
	UDP.UDP_Receiver_EXEC();
	UDP.~UDP_Receiver();
	return 0;
}

//Fun Fact
//elaborating on @theatrus, a notable efficiency is that modulo operator can be replaced by bitwise and with a mask
//(e.g.x % 1024 == x & 1023), and 
//integer division can be replaced by a shift right operation(e.g.x / 1024 == x / 2 ^ 10 == x >> 10)
