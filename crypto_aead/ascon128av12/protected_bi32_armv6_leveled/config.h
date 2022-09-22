#ifndef CONFIG_H_
#define CONFIG_H_

#include "api.h"

/* extern bit interleaving */
#ifndef ASCON_EXTERN_BI
#define ASCON_EXTERN_BI 0
#endif

/* number of PA rounds */
#ifndef ASCON_PA_ROUNDS
#define ASCON_PA_ROUNDS 12
#endif

/* number of PB rounds */
#ifndef ASCON_PB_ROUNDS
#define ASCON_PB_ROUNDS 8
#endif

#if NUM_SHARES_M > NUM_SHARES_KEY
#error "NUM_SHARES_M > NUM_SHARES_KEY currently not supported"
#endif

#if NUM_SHARES_C > NUM_SHARES_M
#error "NUM_SHARES_C > NUM_SHARES_M currently not supported"
#endif

#if NUM_SHARES_AD > NUM_SHARES_M
#error "NUM_SHARES_AD > NUM_SHARES_M currently not supported"
#endif

#if NUM_SHARES_NPUB > NUM_SHARES_KEY
#error "NUM_SHARES_NPUB > NUM_SHARES_KEY currently not supported"
#endif

#endif /* CONFIG_H_ */
