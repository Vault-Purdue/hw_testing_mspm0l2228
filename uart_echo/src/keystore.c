/**
 * @file keystore.c
 * @author Vault Team - Purdue
 * @brief KEYSTORE Driver
 * @date 2026
 *
 * Functions for interacting with the KEYSTORE module to store the root key for AES encryption.
 */

#include "keystore.h"

static bool initialized = false;
static const DL_KEYSTORECTL_Config rootKeyTransferConfig = {
    .keySlot = KEYSTORE_ROOT_KEY_SLOT,
    .keySize = KEYSTORE_KEY_SIZE,
    .cryptoSel = DL_KEYSTORECTL_CRYPTO_SEL_AES,
};

int HSM_KEYSTORE_initRootKeyStorage(uint32_t *buf) {
    
    // If for some reason we attempt to call this function again
    if (initialized) return 0;

    // "This ... can only be written in a secure operating state before the INITDONE signal is asserted."
    if (DL_SYSCTL_isINITDONEIssued()) return -1;

    // Set the number of 256-bit keys
    DL_KEYSTORECTL_setNumberOf256Keys(KEYSTORECTL, KEYSTORE_NUM_256_KEYS);

    // Validate status
    while (DL_KEYSTORECTL_getStatus(KEYSTORECTL) == DL_KEYSTORECTL_STATUS_NO_CONFIG);
    if (DL_KEYSTORECTL_getStatus(KEYSTORECTL) != DL_KEYSTORECTL_STATUS_VALID) return -2;

    // Write the key to KEYSTORE
    DL_KEYSTORECTL_KeyWrConfig rootKeyWriteConfig = {
        .keySlot = KEYSTORE_ROOT_KEY_SLOT,
        .keySize = KEYSTORE_KEY_SIZE,
        .key = buf
    };
    DL_KEYSTORECTL_writeKey(KEYSTORECTL, (DL_KEYSTORECTL_KeyWrConfig*) &rootKeyWriteConfig);

    // Validate status
    while (DL_KEYSTORECTL_getStatus(KEYSTORECTL) == DL_KEYSTORECTL_STATUS_BUSY_RX);
    if (DL_KEYSTORECTL_getStatus(KEYSTORECTL) != DL_KEYSTORECTL_STATUS_VALID) return -3;
    
    initialized = true;
    return 0;
}

int HSM_KEYSTORE_transferRootKeyToAES(void) {
    
    // Validate status
    if (DL_KEYSTORECTL_getStatus(KEYSTORECTL) != DL_KEYSTORECTL_STATUS_VALID) return -1;

    // We need to verify that our root key slot is occupied
    if (DL_KEYSTORECTL_getValidKeySlots(KEYSTORECTL) == 0) return -2;

    // Transfer key
    DL_KEYSTORECTL_transferKey(KEYSTORECTL, (DL_KEYSTORECTL_Config*) &rootKeyTransferConfig);

    // Validate status
    while (DL_KEYSTORECTL_getStatus(KEYSTORECTL) == DL_KEYSTORECTL_STATUS_BUSY_TX);
    if (DL_KEYSTORECTL_getStatus(KEYSTORECTL) != DL_KEYSTORECTL_STATUS_VALID) return -3;

    return 0;
}