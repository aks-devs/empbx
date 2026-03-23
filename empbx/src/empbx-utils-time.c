/**
 **
 ** (C)2025 akstel.org
 **/
#include <empbx.h>

/**
 * unix timestamp in seconds
 */
uint32_t empbx_time_epoch_now() {
    return time(NULL);
}

/**
 *
 * unix time in micro seconds
 **/
uint64_t empbx_time_micro_now() {
    struct timeval tv = { 0 };

    gettimeofday(&tv,NULL);
    return (1000000 * tv.tv_sec + tv.tv_usec);
}
