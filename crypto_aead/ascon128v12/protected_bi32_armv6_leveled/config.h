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
#define ASCON_PB_ROUNDS 6
#endif

#endif /* CONFIG_H_ */
