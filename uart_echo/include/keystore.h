/**
 * @file keystore.h
 * @author Vault Team - Purdue
 * @brief KEYSTORE Driver Header File
 * @date 2026
 *
 * Constants/stubs for the KEYSTORE driver.
 */

#ifndef _KEYSTORE_H_
#define _KEYSTORE_H_

#include <stdbool.h>
#include <stdint.h>
#include <ti/driverlib/driverlib.h>
#include <ti/driverlib/dl_keystorectl.h>

/*********************** CONSTANTS ************************/

#define KEYSTORE_KEY_SIZE DL_KEYSTORECTL_KEY_SIZE_256_BITS
#define KEYSTORE_NUM_256_KEYS DL_KEYSTORECTL_NUM_256_KEYS_TWO
#define KEYSTORE_ROOT_KEY_SLOT DL_KEYSTORECTL_KEY_SLOT_0

/********************* FUNCTION STUBS *********************/

/**
 * @brief Initializes the KEYSTORE configurations and writes the root key to storage.
 * 
 * @param buf Root key buffer.
 *
 * @retval 0: root key was successfully written to KEYSTORE
 * @retval -1: INITDONE was issued
 * @retval -2: non-valid status after setting config
 * @retval -3: non-valid status after writing to KEYSTORE
 */
int KEYSTORE_initRootKeyStorage(uint32_t *buf);

/**
 * @brief Transfers the root key in storage to the AESADV.
 *
 * @retval 0: root key was successfully transferred to AESADV
 * @retval -1: non-valid status before transfer
 * @retval -2: root key not in KEYSTORE
 * @retval -3: non-valid status after transferring to AESADV
 */
int KEYSTORE_transferRootKeyToAES(void);

#endif