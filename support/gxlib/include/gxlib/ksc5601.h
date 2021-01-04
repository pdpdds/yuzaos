#ifdef CPLUSPLUS
extern "C"  {
#endif
extern int  convertMultibyteToUnicodeString( unsigned char* charstring 
					,unsigned int charlen 
					,unsigned short* unistring 
					 ,unsigned int unilen );
extern unsigned short convert_char_ksc5601_to_ucs2(unsigned char byte1, unsigned char byte2);
#ifdef CPLUSPLUS
}
#endif
