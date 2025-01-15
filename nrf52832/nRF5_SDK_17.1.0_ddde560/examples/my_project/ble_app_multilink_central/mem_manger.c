#include <stdint.h>
#include "mem_manger.h"


//跟c库中memcpy的区分
void* memCpyUser(void *destaddr, void const *srcaddr, uint32_t len)
{
    uint32_t i;

    uint8_t *dest = (uint8_t *) destaddr;
    uint8_t const *src = (uint8_t const *) srcaddr;

	int8_t remainder_dest = (uint32_t)dest % 4;
    int8_t remainder_src = (uint32_t)src % 4;

	uint8_t isWord = 0;

	if ((destaddr == 0) || (srcaddr == 0))
	{
		return 0;
	}

	if( (remainder_dest == remainder_src) && (len >= 4) )
	{
        if( remainder_dest == 0 )
        {
            isWord = 1;
        }
        else if(remainder_dest == 1)
        {
        	uint8_t length = 3;
        	for(i = 0; i < length; i++)
        	{
        		*(dest+i) = *(src+i);
        	}

            isWord = 1;
            dest += length;
		    src += length;
		    len = len-length;
        }
        else if(remainder_dest == 2)
        {
        	uint8_t length = 2;
        	for(i = 0; i < length; i++)
        	{
        	    *(dest+i) = *(src+i);
        	}

            isWord = 1;
            dest += length;
		    src += length;
		    len = len-length;
        }
        else if(remainder_dest == 3)
        {
        	uint8_t length = 1;
        	for(i = 0; i < length; i++)
        	{
        	    *(dest+i) = *(src+i);
        	}

            isWord = 1;
            dest += length;
		    src += length;
		    len = len-length;
        }
    }
    else if(len >= 4)
    {
    	int8_t delta = remainder_dest - remainder_src;
    	if(delta == 2 || delta == -2)
    	{
    	    if(remainder_dest == 0 || remainder_dest == 2)
    	    {
    	        isWord = 2;
    	    }
    	    else if(remainder_dest == 1 || remainder_dest == 3)
    	    {
    	    	uint8_t length = 1;
	        	*(dest) = *(src);

	        	isWord = 2;
	            dest += length;
			    src += length;
			    len = len-length;
    	    }
    	}
    }

	if(isWord == 1)
	{
	    uint32_t *dest1 = (uint32_t *)dest;
	    uint32_t const *src1 = (uint32_t const *) src;

		while (len >= 4)
		{
			*(dest1++) = *(src1++);
			len -= 4;
		}

		uint8_t *dest2 = (uint8_t *)dest1;
		uint8_t const *src2 = (uint8_t const *) src1;

		while (len-- > 0)
		{
			*(dest2++) = *(src2++);
		}
	}
	else if(isWord == 2)
	{
	    uint16_t *dest1 = (uint16_t *)dest;
	    uint16_t const *src1 = (uint16_t const *) src;

		while (len >= 2)
		{
			*(dest1++) = *(src1++);
			len -= 2;
		}

		uint8_t *dest2 = (uint8_t *)dest1;
		uint8_t const *src2 = (uint8_t const *) src1;

		while (len-- > 0)
		{
			*(dest2++) = *(src2++);
		}
	}
	else
	{
		while (len-- > 0)
		{
			*(dest++) = *(src++);
		}
	}

	return destaddr;
}

void* memSetUser(void *destaddr, char ch, uint32_t len)
{
	uint32_t i;
	uint8_t *dest = (uint8_t *) destaddr;

	for (i = 0; i < len; i++)
	{
		*dest = ch;
		dest++;
	}

	return destaddr;
}



void *my_memmove(void *dest, const void *src, size_t n) {
    unsigned char *d = dest;
    const unsigned char *s = src;

    if (d < s) {
        // Non-overlapping case: Copy forward
        while (n--) {
            *d++ = *s++;
        }
    } else {
        // Overlapping case: Copy backward
        d += n;
        s += n;
        while (n--) {
            *(--d) = *(--s);
        }
    }

    return dest;
}
