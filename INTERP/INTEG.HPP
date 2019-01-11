//	integ.hpp
#ifdef DEBUG
#ifndef INTEG_HPP
#define INTEG_HPP

#define CHECK_OFF				0x00000000
#define CHECK_SIGNATURES	0x00000001
#define CHECK_CHECKSUMS		0x00000002
#define CHECK_FREE			0x00000004
#define CHECK_LISTS			0x00000008
#define CHECK_HEAP			0x00000010
#define CHECK_GLOBALS		0x00000020
#define CHECK_ALL				0x0000003F
#define CHECK_MONO_OFF		0x80000000

void	IntegrityChecking(int mask = CHECK_ALL, char* fmt = 0, ...);
Bool	CheckIntegrity(char* fmt = 0, ...);
void	CheckIntegrityRequest();

extern int	checkIntegrity;
extern Bool	checkIntegrityRequest;

#endif
#endif


