/* Includes */
#include <time.h>

/* Returns the current time_t
 * as a string formatted by asctime */

extern char *asctime(const struct tm *tim_p);
extern struct tm *localtime(const time_t * tim_p);

char *ctime(const time_t *timer) 
{

  return (char *)asctime(localtime(timer));
}
