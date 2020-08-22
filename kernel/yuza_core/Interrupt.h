#pragma once

#pragma pack( push, 1 )

struct InterruptFrame {
	int edi;
	int esi;
	int ebp;
	int esp;
	int ebx;
	int edx;
	int ecx;
	int eax;
	int vector;
	int errorCode;
	int eip;
	int cs;
	int flags;
	int user_esp;
	int user_ss;
	
	void Print() const;
};



#pragma pack( pop )
