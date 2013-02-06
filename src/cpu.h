#ifndef _CPU_H_
#define _CPU_H_

void cpu_68k_enable();
void cpu_68k_disable();
void cpu_68kfast_enable(); 
void cpu_68kfast_disable();
void pcmciamode_enable();
void pcmciamode_disable();
void instcache_disable();
void instcache_enable();
void instcache_reset_enable();
void instcache_reset_disable();

#endif /* _CPU_H_ */
