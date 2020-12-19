#pragma once

template <typename T>
class SFEncrytion
{
public:
	SFEncrytion(void);
	virtual ~SFEncrytion(void);

	static bool Encrypt(unsigned char* pSrc, unsigned int Length);
	static bool Decrypt(unsigned char* pSrc, unsigned int Length);
};

template <typename T>
bool SFEncrytion<T>::Encrypt(unsigned char* pSrc, unsigned int Length)
{
	return T::Encrypt(pSrc, Length);
}

template <typename T>
bool SFEncrytion<T>::Decrypt(unsigned char* pSrc, unsigned int Length)
{
	return T::Decrypt(pSrc, Length);
}