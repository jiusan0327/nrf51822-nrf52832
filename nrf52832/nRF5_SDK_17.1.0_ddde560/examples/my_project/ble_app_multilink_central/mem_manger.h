#ifndef _MEM_MANAGER_H_
#define _MEM_MANAGER_H_

#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */

#include <stdint.h>
#include <stddef.h>
#define memcpy memCpyUser
#define memset memSetUser

#define atom_inc(p)     while(__strex((__ldrex(p)+1U),p))
#define atom_dec(p)     while(__strex((__ldrex(p)-1U),p))


extern void* memCpyUser(void *destaddr, void const *srcaddr, uint32_t len);
extern void* memSetUser(void *destaddr, char ch, uint32_t len);
void *my_memmove(void *dest, const void *src, size_t n) ;
#ifdef __cplusplus
}
#endif /* __cplusplus */












#endif






