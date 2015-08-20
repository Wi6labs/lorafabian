#ifndef CONTIKI_CONF_H
#define CONTIKI_CONF_H

#include <stdint.h>

#define CCIF
#define CLIF

#define WITH_UIP 1
#define WITH_ASCII 1

#define CLOCK_CONF_SECOND 100

#define COFFEE_IO_SEMANTICS 1

#define PACKETBUF_CONF_SIZE			512
#if 0
#define DEBUG 1
/* #define DEBUG     */
#ifdef DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif
#endif

/* #define PROCESS_CONF_STATS 1 */

/* These names are deprecated, use C99 names. */
typedef uint8_t u8_t;
typedef uint16_t u16_t;
typedef uint32_t u32_t;
typedef int8_t s8_t;
typedef int16_t s16_t;
typedef int32_t s32_t;

typedef unsigned int clock_time_t;
typedef unsigned int uip_stats_t;

#ifndef BV
#define BV(x) (1<<(x))
#endif


#ifdef USE_NETSTACK

/* Network setup for IPv6 */
#define NETSTACK_CONF_NETWORK sicslowpan_driver
#define NETSTACK_CONF_MAC     xbee_pro_s1_mac_driver //nullmac_driver
#ifndef NETSTACK_CONF_RDC
#define NETSTACK_CONF_RDC     nullrdc_driver //contikimac_driver
#endif /*NETSTACK_CONF_RDC*/
#define NETSTACK_CONF_RADIO   nullradio_driver
#define NETSTACK_CONF_FRAMER  framer_nullmac //framer_802154
#define RIMEADDR_CONF_SIZE              8

#define UIP_CONF_LL_802154              1
#define UIP_CONF_LLH_LEN                0
#define UIP_CONF_UDP_CONNS              2

//#define UIP_CONF_ROUTER                 1
#undef UIP_CONF_IPV6_RPL
#define UIP_CONF_IPV6_RPL               0
#define UIP_CONF_TCP                    0

/* Handle 10 neighbors */
#define NBR_TABLE_CONF_MAX_NEIGHBORS     5
/* Handle 10 routes    */
#define UIP_CONF_MAX_ROUTES   10

#define UIP_CONF_ND6_SEND_RA		0
//#define UIP_CONF_ND6_SEND_NA    0
#define UIP_CONF_ND6_REACHABLE_TIME     600000
#define UIP_CONF_ND6_RETRANS_TIMER      10000

#define UIP_CONF_IPV6                   1
#define UIP_CONF_IPV6_QUEUE_PKT         0
#define UIP_CONF_IPV6_CHECKS            1
#define UIP_CONF_IPV6_REASSEMBLY        0
#define UIP_CONF_NETIF_MAX_ADDRESSES    3
#define UIP_CONF_ND6_MAX_PREFIXES       3
#define UIP_CONF_ND6_MAX_DEFROUTERS     2
#define UIP_CONF_IP_FORWARD             0

//REST_MAX_CHUNK_SIZE
#define UIP_CONF_BUFFER_SIZE		256

#define SICSLOWPAN_CONF_COMPRESSION_IPV6        0
#define SICSLOWPAN_CONF_COMPRESSION_HC1         1
#define SICSLOWPAN_CONF_COMPRESSION_HC01        2
#define SICSLOWPAN_CONF_COMPRESSION             SICSLOWPAN_COMPRESSION_HC06
#ifndef SICSLOWPAN_CONF_FRAG
#define SICSLOWPAN_CONF_FRAG                    1
#define SICSLOWPAN_CONF_MAXAGE                  8
#endif /* SICSLOWPAN_CONF_FRAG */
#define SICSLOWPAN_CONF_CONVENTIONAL_MAC	1
#define SICSLOWPAN_CONF_MAX_ADDR_CONTEXTS       2

#endif /*USE_NETSTACK*/

/* Prefix for relocation sections in ELF files */
#define REL_SECT_PREFIX ".rel"

#define CC_BYTE_ALIGNED __attribute__ ((packed, aligned(1)))

#define USB_EP1_SIZE 64
#define USB_EP2_SIZE 64

#ifndef RAND_MAX
#define RAND_MAX 0x7fff
#endif /*RAND_MAX*/


#ifdef PROJECT_CONF_H
#include PROJECT_CONF_H
#endif /* PROJECT_CONF_H */
#endif /* __CONTIKI_CONF_H__ */
