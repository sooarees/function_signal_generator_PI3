#include <stddef.h>
#include <stdint.h>

// funcoes minimas usadas sem a biblioteca padrao

void *memcpy(void *destination, const void *source, size_t count)
{
	uint8_t *dst = destination;
	const uint8_t *src = source;

	while (count-- > 0U)
	{
		*dst++ = *src++;
	}

	return destination;
}

void *memset(void *destination, int value, size_t count)
{
	uint8_t *dst = destination;

	while (count-- > 0U)
	{
		*dst++ = (uint8_t)value;
	}

	return destination;
}
