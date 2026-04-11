/**
 * @file aesadv.c
 * @author Vault Team - Purdue
 * @brief AES Driver
 * @date 2026
 *
 * Functions for interacting with the AES encryption module
 */

#include "aesadv.h"

#define AES_TRANSACTION_LENGTH (4)



/* AES_ADV configuration for ECB mode in encrypt direction */
static DL_AESADV_Config gAESADV_config = {
      .mode              = DL_AESADV_MODE_ECB,
      .direction         = DL_AESADV_DIR_ENCRYPT,
      .iv                = NULL,
      .nonce             = NULL,
      .lowerCryptoLength = 16,
      .upperCryptoLength = 0,
      .aadLength         = 0,
};

void HSM_AESADV_encrypt(uint32_t *key, uint32_t *input, uint32_t *output){
    gAESADV_config.direction = DL_AESADV_DIR_ENCRYPT;
    DL_AESADV_setKeySize(AESADV, DL_AESADV_KEY_SIZE_128_BIT);
    DL_AESADV_setKeyAligned(AESADV, &key[0], DL_AESADV_KEY_SIZE_128_BIT);

    DL_AESADV_initECB(AESADV, &gAESADV_config);
    while (!DL_AESADV_isInputReady(AESADV)){
        ;
    }
    DL_AESADV_loadInputDataAligned(AESADV, input);
    while (!DL_AESADV_isOutputReady(AESADV)){
        ;
    }
    DL_AESADV_readOutputDataAligned(AESADV, output);
    return;
}

void HSM_AESADV_decrypt(uint32_t *key, uint32_t *input, uint32_t *output){
    gAESADV_config.direction = DL_AESADV_DIR_DECRYPT;
    DL_AESADV_setKeySize(AESADV, DL_AESADV_KEY_SIZE_128_BIT);
    DL_AESADV_setKeyAligned(AESADV, &key[0], DL_AESADV_KEY_SIZE_128_BIT);
    DL_AESADV_initECB(AESADV, &gAESADV_config);
    while (!DL_AESADV_isInputReady(AESADV)){
        ;
    }
    DL_AESADV_loadInputDataAligned(AESADV, input);
    while (!DL_AESADV_isOutputReady(AESADV)){
        ;
    }
    DL_AESADV_readOutputDataAligned(AESADV, output);
    return;
}   