extern "C" {

void* 	pmLock0();
void 		setupTossRet();

void		asm_fatal(int which, int parm1, int parm2, int parm3);
#pragma aux asm_fatal parm [EDX] [EAX] [EBX] [ECX];

}

