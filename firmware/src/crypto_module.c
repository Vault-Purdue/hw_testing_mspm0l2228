/**
 * @file crypto_module.c
 * @author Vault Team - Purdue
 * @brief Crypto module - AES-256-GCM encrypt/decrypt
 * @date 2026
 *
 * Calls into the AESADV and TRNG drivers.
 */

#include "crypto_module.h"
#include "driver/aes_adv_gcm.h"
#include "driver/trng.h"

/* ========================================================================= */

// Wipe the plaintext buffer if decrypt fails (aka if someone was tampering)
// so the file manager doesn't end up with some garbage bits 
static void wipe(void *p, uint32_t n)
{
    volatile uint8_t *v = (volatile uint8_t *)p;
    while (n--) {
        *v++ = 0;
    }
}

/* ========================================================================= */
/* This wrapper lets the crypto module request an arbitrary number of bytes
 * (e.g. 12 bytes for a GCM IV).                                            */

static int trng_get_bytes(uint8_t *out, uint32_t len)
{
    uint32_t tmp[TRNG_128_BIT_BUF_SIZE]; // 16-byte temp buffer

    while (len > 0) {
        // Generate 128 bits (16 bytes) of random data per iteration
        trngGenerate128BitNumber(tmp);

        uint32_t chunk = (len < 16) ? len : 16;
        memcpy(out, tmp, chunk);
        out += chunk;
        len -= chunk;
    }

    // Scrub the temp buffer so key material doesn't linger on the stack
    wipe(tmp, sizeof(tmp));
    return 0;
}

/* ========================================================================= */

// Powers up the AES hardware. Call once at startup.
crypto_status crypto_init(void)
{
    AESADV_init();
    return CRYPTO_OK;
}

// Fills a buffer with the random bytes from the TRNG 
crypto_status crypto_random(uint8_t *out, uint32_t len)
{
    if (out == NULL) {
        return CRYPTO_ERR_NULL_PARAM;
    }
    if (len == 0) {
        return CRYPTO_OK;
    }

    if (trng_get_bytes(out, len) != 0) {
        return CRYPTO_ERR_TRNG_FAIL;
    }
    return CRYPTO_OK;
}

// This generates a new 12 byte Initialization Vector for the AES-GCM
crypto_status crypto_generate_iv(uint8_t iv[CRYPTO_GCM_IV_SIZE])
{
    return crypto_random(iv, CRYPTO_GCM_IV_SIZE);
}

crypto_status crypto_gcm_encrypt(
    const uint8_t key[CRYPTO_AES_KEY_SIZE],
    const uint8_t iv[CRYPTO_GCM_IV_SIZE],
    const uint8_t *aad, uint32_t aad_len,
    const uint8_t *pt, uint32_t pt_len,
    uint8_t *ct,
    uint8_t tag[CRYPTO_GCM_TAG_SIZE])
{
    if (key == NULL || iv == NULL || tag == NULL) {
        return CRYPTO_ERR_NULL_PARAM;
    }
    if (pt_len > 0 && (pt == NULL || ct == NULL)) {
        return CRYPTO_ERR_NULL_PARAM;
    }
    if (aad_len > 0 && aad == NULL) {
        return CRYPTO_ERR_NULL_PARAM;
    }

    int rc = AESADV_AESGCM256_encrypt(
        key, iv,
        aad, (size_t)aad_len,
        pt, (size_t)pt_len,
        ct, tag);

    if (rc != 0) {
        return CRYPTO_ERR_HW_FAIL;
    }
    return CRYPTO_OK;
}

crypto_status crypto_gcm_decrypt(
    const uint8_t key[CRYPTO_AES_KEY_SIZE],
    const uint8_t iv[CRYPTO_GCM_IV_SIZE],
    const uint8_t *aad, uint32_t aad_len,
    const uint8_t *ct, uint32_t ct_len,
    const uint8_t tag[CRYPTO_GCM_TAG_SIZE],
    uint8_t *pt)
{
    if (key == NULL || iv == NULL || tag == NULL) {
        return CRYPTO_ERR_NULL_PARAM;
    }
    if (ct_len > 0 && (ct == NULL || pt == NULL)) {
        return CRYPTO_ERR_NULL_PARAM;
    }
    if (aad_len > 0 && aad == NULL) {
        return CRYPTO_ERR_NULL_PARAM;
    }
    
    int rc = AESADV_AESGCM256_decrypt(
        key, iv,
        aad, (size_t)aad_len,
        ct, (size_t)ct_len,
        tag, pt);

    if (rc != 0) {
        // Wipe the plaintext buffer so the caller can't accidentally
        // trust unauthenticated data.
        if (pt != NULL && ct_len > 0) {
            wipe(pt, ct_len);
        }
        return CRYPTO_ERR_AUTH_FAIL;
    }
    return CRYPTO_OK;
}
/* ========================================================================= */
