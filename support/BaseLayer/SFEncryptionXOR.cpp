#include "SFEncryptionXOR.h"

#define ENCRYPTION_KEY 0xAFB7E3D9

unsigned int SFEncryptionXOR::m_EncrytionKey = ENCRYPTION_KEY;

SFEncryptionXOR::SFEncryptionXOR()
{
	
}

SFEncryptionXOR::~SFEncryptionXOR(void)
{
}

bool SFEncryptionXOR::Encrypt(unsigned char* pSrc, unsigned int Length)
{
	unsigned int Remain = Length % sizeof(unsigned int);
	Length -= Remain;

	for(unsigned int i = 0; i < Length; i = i + sizeof(unsigned int))
	{
		*(unsigned int*)(pSrc + i) = *(unsigned int*)(pSrc + i) ^ m_EncrytionKey;
	}

	return true;
}

bool SFEncryptionXOR::Decrypt(unsigned char* pSrc, unsigned int Length)
{
	unsigned int Remain = Length % sizeof(unsigned int);
	Length -= Remain;

	for(unsigned int i = 0; i < Length; i = i + sizeof(unsigned int))
	{
		*(unsigned int*)(pSrc + i) = *(unsigned int*)(pSrc + i) ^ m_EncrytionKey;
	}

	return true;
}
