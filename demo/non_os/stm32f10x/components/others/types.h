
#ifndef TYPES_H_
#define TYPES_H_

#include <stdint.h>
#include <stddef.h>
typedef int                             bool_t;      /**< boolean type */

#ifndef TRUE
	#define TRUE            1
#endif

#ifndef FALSE
	#define FALSE           0
#endif

#ifndef NULL
	#define NULL 0
#endif

#define success                  0
#define fail                     1


#ifndef disable
    #define disable 0
#endif

#ifndef enable
    #define enable 1
#endif

#endif /* TYPES_H_ */
