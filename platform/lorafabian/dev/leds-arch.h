#ifndef LEDS_H
#define LEDS_H

/*#define LEDS_ALL 3
#define LEDS_YELLOW   1
#define LEDS_GREEN    2
*/
void leds_arch_init(void);
unsigned char leds_arch_get(void);
void leds_arch_set(unsigned char leds);

#endif
