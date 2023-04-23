#include "types.h"
#include "x86.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wvoid-pointer-to-int-cast"
#pragma clang diagnostic ignored "-Wincompatible-library-redeclaration"


///这是一个C语言中的内存清零函数，用于将一段内存区域中的所有字节都设置为指定的值。
// 函数名为memset，参数包括要清零的内存区域的起始地址dst、要设置的值c以及要清零的字节数n。
//函数首先进行了一些优化判断：如果dst的地址是4的倍数，并且要清零的字节数n也是4的倍数，则使用stosl指令一次性设置4个字节的值，提高了效率。
//否则，使用stosb指令逐个字节地设置值。
//在实现过程中，首先对要设置的值c进行了位运算，保留其低8位。然后用这个8位的值构造32位整数，每个字节都设置为c的低8位值。最后，通过调用stosl或stosb指令，将这个32位的值或8位的值设置到内存区域中。
//该函数返回清零后的内存区域起始地址dst。
void *
memset(void *dst, int c, uint n) {
    if ((int) dst % 4 == 0 && n % 4 == 0) {
        c &= 0xFF;
        stosl(dst, (c << 24) | (c << 16) | (c << 8) | c, n / 4);
    } else
        stosb(dst, c, n);
    return dst;
}

int
memcmp(const void *v1, const void *v2, uint n) {
    const uchar *s1, *s2;

    s1 = v1;
    s2 = v2;
    while (n-- > 0) {
        if (*s1 != *s2)
            return *s1 - *s2;
        s1++, s2++;
    }

    return 0;
}

void *
memmove(void *dst, const void *src, uint n) {
    const char *s;
    char *d;

    s = src;
    d = dst;
    if (s < d && s + n > d) {
        s += n;
        d += n;
        while (n-- > 0)
            *--d = *--s;
    } else
        while (n-- > 0)
            *d++ = *s++;

    return dst;
}

// memcpy exists to placate GCC.  Use memmove.
void *
memcpy(void *dst, const void *src, uint n) {
    return memmove(dst, src, n);
}

int
strncmp(const char *p, const char *q, uint n) {
    while (n > 0 && *p && *p == *q)
        n--, p++, q++;
    if (n == 0)
        return 0;
    return (uchar) *p - (uchar) *q;
}

char *
strncpy(char *s, const char *t, int n) {
    char *os;

    os = s;
    while (n-- > 0 && (*s++ = *t++) != 0);
    while (n-- > 0)
        *s++ = 0;
    return os;
}

// Like strncpy but guaranteed to NUL-terminate.
char *
safestrcpy(char *s, const char *t, int n) {
    char *os;

    os = s;
    if (n <= 0)
        return os;
    while (--n > 0 && (*s++ = *t++) != 0);
    *s = 0;
    return os;
}

int
strlen(const char *s) {
    int n;

    for (n = 0; s[n]; n++);
    return n;
}


#pragma clang diagnostic pop