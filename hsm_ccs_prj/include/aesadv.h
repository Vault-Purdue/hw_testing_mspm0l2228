/**
 * @file aesadv.h
 * @author Vault Team - Purdue
 * @brief AESADV encryption functions
 * @date 2026
 *
 * Wrapper functions for using the AESADV encryption hardware
 */

#ifndef _AESADV_H_
#define _AESADV_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <ti/driverlib/driverlib.h>
#include <ti/driverlib/dl_aesadv.h>

/*********************** CONSTANTS ************************/
#define AES_TRANSACTION_LENGTH (4)

void HSM_AESADV_encrypt(uint32_t *key, uint32_t *input, uint32_t *output);
void HSM_AESADV_decrypt(uint32_t *key, uint32_t *input, uint32_t *output);

#endif