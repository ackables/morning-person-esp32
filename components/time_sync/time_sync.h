#ifndef TIME_SYNC_H
#define TIME_SYNC_H

#include "esp_sntp.h"  // for SNTP APIs

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize and start the SNTP service.
 *        Configures SNTP operating mode, server, and notification callback.
 */
void initialize_sntp(void);

/**
 * @brief Obtain and synchronize the system time via SNTP.
 *        Waits up to ~10 seconds for the first successful time sync.
 */
void obtain_time(void);

#ifdef __cplusplus
}
#endif

#endif // TIME_SYNC_H
