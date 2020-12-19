#pragma once

class SFEncryptionXOR
{
public:
	SFEncryptionXOR();
	virtual ~SFEncryptionXOR();

	static bool Encrypt(unsigned char* pSrc, unsigned int Length);
	static bool Decrypt(unsigned char* pSrc, unsigned int Length);

protected:

private:
	static unsigned int m_EncrytionKey;
};
