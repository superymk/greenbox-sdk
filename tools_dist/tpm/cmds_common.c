// SPDX-License-Identifier: GPL-2.0-only
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <wolftpm/tpm2.h>
#include <wolftpm/tpm2_wrap.h>

#include <math.h>
#include <log.h>

#include "cmds.h"
#include "app_os_if.h"
#include "utils_string.h"
#include "utils.h"
#include "hypercall.h"
#include "./hash/hash.h"

extern int tpm2_aik_convert_format(void);

static WOLFTPM2_DEV tpm2_dev;
static bool tpm2_initialized = false;

static bool tpm2_is_initialized(void)
{
    return tpm2_initialized;
}

static bool _tpm_check_pcr(uint32_t pcr)
{
    // Check: params must be valid
    if ((pcr >= 24))
        return false;

    return true;
}

static int _tpm20_srk_create_new(WOLFTPM2_DEV *dev, TPM_HANDLE *out_srk_handle, uint32_t *out_srk_unique_first_uint32)
{
    int rc = TPM_RC_FAILURE;
    TPMI_ALG_PUBLIC alg = TPM_ALG_RSA; /* TPM_ALG_ECC */
    WOLFTPM2_KEY storage;              /* SRK */

    XMEMSET(&storage, 0, sizeof(storage));

    // Create primary storage key
    rc = wolfTPM2_CreateSRK(dev, &storage, alg, NULL, 0);
    if (rc != TPM_RC_SUCCESS)
    {
        TOOL_ERR("Create a new SRK failed 0x%x: %s\n", rc, TPM2_GetRCString(rc));
        goto exit;
    }

    *out_srk_handle = storage.handle.hndl;
    *out_srk_unique_first_uint32 = *(uint32_t *)storage.pub.publicArea.unique.rsa.buffer;

    // [NOTE] Do not shutdown TPM on SRK creation success. Or the subsequent code cannot use the SRK.
    printf("Create SRK succeed\n");

    // On success
    return TPM_RC_SUCCESS;

exit:
    return rc;
}

static int wolfTPM2_CreateAndLoadKey2(WOLFTPM2_DEV *dev, WOLFTPM2_KEY *key, WOLFTPM2_KEYBLOB *key_blob,
                                      WOLFTPM2_HANDLE *parent, TPMT_PUBLIC *publicTemplate,
                                      const byte *auth, int authSz)
{
    int rc;
    WOLFTPM2_KEYBLOB keyBlob;

    rc = wolfTPM2_CreateKey(dev, &keyBlob, parent, publicTemplate, auth, authSz);
    if (rc == TPM_RC_SUCCESS)
    {
        rc = wolfTPM2_LoadKey(dev, &keyBlob, parent);
    }

    /* return loaded key */
    XMEMCPY(key, &keyBlob, sizeof(WOLFTPM2_KEY));

    XMEMCPY(key_blob, &keyBlob, sizeof(WOLFTPM2_KEYBLOB));

    return rc;
}

__attribute__((unused)) static int wolfTPM2_CreateAndLoadAIK2(WOLFTPM2_DEV *dev, WOLFTPM2_KEY *aikKey, WOLFTPM2_KEYBLOB *key_blob,
                                                              TPM_ALG_ID alg, WOLFTPM2_KEY *srkKey, const byte *auth, int authSz)
{
    int rc;
    TPMT_PUBLIC publicTemplate;

    if (alg == TPM_ALG_RSA)
    {
        rc = wolfTPM2_GetKeyTemplate_RSA_AIK(&publicTemplate);
    }
    else if (alg == TPM_ALG_ECC)
    {
        rc = wolfTPM2_GetKeyTemplate_ECC_AIK(&publicTemplate);
    }
    else
    {
        return BAD_FUNC_ARG;
    }
    /* GetKeyTemplate check */
    if (rc != 0)
        return rc;

    rc = wolfTPM2_CreateAndLoadKey2(dev, aikKey, key_blob, &srkKey->handle,
                                    &publicTemplate, auth, authSz);

    return rc;
}

static int _tpm20_srk_get(WOLFTPM2_DEV *dev, TPM_HANDLE handle, WOLFTPM2_KEY *pStorageKey)
{
    int rc = TPM_RC_FAILURE;

    /* See if SRK already exists */
    rc = wolfTPM2_ReadPublicKey(dev, pStorageKey, handle);
    if (rc != 0)
    {
        printf("Loading SRK: Failed to get SRK at handle 0x%08X, status:0x%x: %s\n", handle, rc, TPM2_GetRCString(rc));
        return rc;
    }

    printf("Loading SRK: handle 0x%x (%d bytes)\n",
           (word32)pStorageKey->handle.hndl, pStorageKey->pub.size);
    return TPM_RC_SUCCESS;
}

static int _tpm20_aik_get(WOLFTPM2_DEV *dev, TPM_HANDLE handle, WOLFTPM2_KEY *pAISKey)
{
    int rc = TPM_RC_FAILURE;

    /* See if AIK already exists */
    rc = wolfTPM2_ReadPublicKey(dev, pAISKey, handle);
    if (rc != 0)
    {
        printf("Loading AIK: Failed to get AIK at handle 0x%08X, status:0x%x: %s\n", handle, rc, TPM2_GetRCString(rc));
        return rc;
    }

    printf("Loading AIK: handle 0x%x (%d bytes)\n",
           (word32)pAISKey->handle.hndl, pAISKey->pub.size);
    return TPM_RC_SUCCESS;
}

__attribute__((unused)) static int writeBin(const char *filename, const byte *buf, word32 bufSz)
{
    int rc = TPM_RC_FAILURE;
    XFILE fp = NULL;
    size_t fileSz = 0;

    fp = XFOPEN(filename, "wb");
    if (fp != XBADFILE)
    {
        fileSz = XFWRITE(buf, 1, bufSz, fp);
        /* sanity check */
        if (fileSz == (word32)bufSz)
        {
            rc = TPM_RC_SUCCESS;
        }
#ifdef DEBUG_WOLFTPM
        printf("Wrote %d bytes to %s\n", (int)fileSz, filename);
#endif
        XFCLOSE(fp);
    }

    return rc;
}

__attribute__((unused)) static int readBin(const char *filename, byte *buf, word32 *bufSz)
{
    int rc = TPM_RC_FAILURE;
    XFILE fp = NULL;
    size_t fileSz = 0;
    size_t bytes_read = 0;

    fp = XFOPEN(filename, "rb");
    if (fp != XBADFILE)
    {
        XFSEEK(fp, 0, XSEEK_END);
        fileSz = XFTELL(fp);
        XREWIND(fp);
        if (fileSz > (size_t)*bufSz)
        {
            printf("File size check failed\n");
            rc = BUFFER_E;
        }
        else
        {
            *bufSz = (int)fileSz;
#ifdef DEBUG_WOLFTPM
            printf("Reading %d bytes from %s\n", (int)fileSz, filename);
#endif
            bytes_read = XFREAD(buf, 1, fileSz, fp);
            if (bytes_read == fileSz)
            {
                rc = TPM_RC_SUCCESS;
            }
        }
        XFCLOSE(fp);
    }
    else
    {
        rc = BUFFER_E;
        printf("File %s not found!\n", filename);
    }

    return rc;
}

__attribute__((unused)) static int writeKeyBlob(const char *filename,
                                                WOLFTPM2_KEYBLOB *key)
{
    int rc = TPM_RC_FAILURE;
    XFILE fp = NULL;
    size_t fileSz = 0;
    byte pubAreaBuffer[sizeof(TPM2B_PUBLIC)];
    int pubAreaSize;

    fp = XFOPEN(filename, "wb");
    if (fp != XBADFILE)
    {
        /* Make publicArea in encoded format to eliminate empty fields,
         * save space */
        rc = TPM2_AppendPublic(pubAreaBuffer, (word32)sizeof(pubAreaBuffer),
                               &pubAreaSize, &key->pub);
        if (rc != TPM_RC_SUCCESS)
            return rc;
        if (pubAreaSize != (key->pub.size + (int)sizeof(key->pub.size)))
        {
            printf("writeKeyBlob: Sanity check for publicArea size failed\n");
            return BUFFER_E;
        }
        // #ifdef WOLFTPM_DEBUG_VERBOSE
        //     TPM2_PrintBin(pubAreaBuffer, pubAreaSize);
        // #endif
        /* Write size marker for the public part */
        fileSz += XFWRITE(&key->pub.size, 1, sizeof(key->pub.size), fp);
        /* Write the public part with bytes aligned */
        fileSz += XFWRITE(pubAreaBuffer, 1, sizeof(UINT16) + key->pub.size, fp);
        /* Write the private part, size marker is included */
        fileSz += XFWRITE(&key->priv, 1, sizeof(UINT16) + key->priv.size, fp);
        XFCLOSE(fp);
    }
    printf("Wrote %d bytes to %s\n", (int)fileSz, filename);

    return rc;
}

__attribute__((unused)) static int readKeyBlob(const char *filename, WOLFTPM2_KEYBLOB *key)
{
    int rc = TPM_RC_FAILURE;
    XFILE fp = NULL;
    size_t fileSz = 0;
    size_t bytes_read = 0;
    byte pubAreaBuffer[sizeof(TPM2B_PUBLIC)];
    int pubAreaSize;

    XMEMSET(key, 0, sizeof(WOLFTPM2_KEYBLOB));

    fp = XFOPEN(filename, "rb");
    if (fp != XBADFILE)
    {
        XFSEEK(fp, 0, XSEEK_END);
        fileSz = XFTELL(fp);
        XREWIND(fp);
        if (fileSz > sizeof(key->priv) + sizeof(key->pub))
        {
            printf("File size check failed\n");
            rc = BUFFER_E;
            goto exit;
        }
        printf("Reading %d bytes from %s\n", (int)fileSz, filename);

        bytes_read = XFREAD(&key->pub.size, 1, sizeof(key->pub.size), fp);
        if (bytes_read != sizeof(key->pub.size))
        {
            printf("Read %zu, expected size marker of %zu bytes\n",
                   bytes_read, sizeof(key->pub.size));
            goto exit;
        }
        fileSz -= bytes_read;

        bytes_read = XFREAD(pubAreaBuffer, 1,
                            sizeof(UINT16) + key->pub.size, fp);
        if (bytes_read != sizeof(UINT16) + key->pub.size)
        {
            printf("Read %zu, expected public blob %zu bytes\n",
                   bytes_read, sizeof(UINT16) + key->pub.size);
            goto exit;
        }
        fileSz -= bytes_read; /* Reminder bytes for private key part */

        /* Decode the byte stream into a publicArea structure ready for use */
        rc = TPM2_ParsePublic(&key->pub, pubAreaBuffer,
                              (word32)sizeof(pubAreaBuffer), &pubAreaSize);
        if (rc != TPM_RC_SUCCESS)
            return rc;
        // #ifdef DEBUG_WOLFTPM
        //     TPM2_PrintPublicArea(&key->pub);
        // #endif

        if (fileSz > 0)
        {
            printf("Reading the private part of the key\n");
            bytes_read = XFREAD(&key->priv, 1, fileSz, fp);
            if (bytes_read != fileSz)
            {
                printf("Read %zu, expected private blob %zu bytes\n",
                       bytes_read, fileSz);
                goto exit;
            }
            rc = 0; /* success */
        }
        if (key->priv.size == 0)
        {
            printf("No private key loaded\n");
        }

        /* sanity check the sizes */
        if (pubAreaSize != (key->pub.size + (int)sizeof(key->pub.size)) ||
            key->priv.size > sizeof(key->priv.buffer))
        {
            printf("Struct size check failed (pub %d, priv %d)\n",
                   key->pub.size, key->priv.size);
            rc = BUFFER_E;
        }
    }
    else
    {
        rc = BUFFER_E;
        printf("File %s not found!\n", filename);
        printf("Keys can be generated by running:\n"
               "  ./examples/keygen/keygen rsa_test_blob.raw -rsa -t\n"
               "  ./examples/keygen/keygen ecc_test_blob.raw -ecc -t\n");
    }

exit:
    if (fp)
        XFCLOSE(fp);

    return rc;
}

// [NOTE] Linux and Windows have to use different UXMHF64_SRK_PERSISTED_HANDLE. Otherwise, we encountered auth error on a dual-OS
// machine: Win10 cannot create the AIK because Linux has already taken the handle
static int _tpm2_aik_create_new(WOLFTPM2_DEV *dev, uint32_t srk_handle, const char *aik_pem_filepath, const char *aik_keyblob_filepath)
{
    int rc = TPM_RC_FAILURE;
    MHV_STATUS sb_status = MHV_SUCCESS;
    TPMI_ALG_PUBLIC alg = TPM_ALG_RSA; /* TPM_ALG_ECC */
    WOLFTPM2_KEY storage;              /* SRK */
    WOLFTPM2_KEY aik;                  /* AIK */
    WOLFTPM2_KEYBLOB aik_keyblob;
    uint32_t aik_unique_first_uint32 = 0;

    XMEMSET(&storage, 0, sizeof(storage));
    XMEMSET(&aik, 0, sizeof(aik));
    XMEMSET(&aik_keyblob, 0, sizeof(aik_keyblob));

    printf("\tOutput AIK PEM file: %s\n", aik_pem_filepath);
    printf("\tOutput AIK keyblob file: %s\n", aik_keyblob_filepath);
    printf("\tUse %s SRK/AIK\n", TPM2_GetAlgName(alg));

    // get SRK
    rc = _tpm20_srk_get(dev, srk_handle, &storage);
    if (rc != TPM_RC_SUCCESS)
        goto exit;

    // Create key for Attestation purposes
    rc = wolfTPM2_CreateAndLoadAIK2(dev, &aik, &aik_keyblob, alg, &storage, NULL, 0);
    if (rc != TPM_RC_SUCCESS)
    {
        printf("wolfTPM2_CreateAndLoadAIK2 failed 0x%x: %s\n", rc,
               TPM2_GetRCString(rc));
        goto exit;
    }
    printf("wolfTPM2_CreateAndLoadAIK2: AIK 0x%x (%d bytes)\n",
           (word32)aik.handle.hndl, aik.pub.size);

    aik_unique_first_uint32 = *(uint32_t *)aik.pub.publicArea.unique.rsa.buffer;

    // Replace the persisted AIK if any. This is unlike the procedure for SRK keys: persistent SRK keys are not replaced
    // because same templates and physical TPM HW yield the same SRK keys.
    TOOL_INFO("Persisting AIK...\n");
    sb_status = (MHV_STATUS)hypercall_2args(NS_HC_TPM2_REPLACE_AIK, aik.handle.hndl, aik_unique_first_uint32);
    if (MHV_FAIL(sb_status))
    {
        // Warn about any persist AIK in TPM errors but not return errors. This is because we will them in files anyway.
        TOOL_WARN("<tpm2_aik_create> Hypercall (%u) error: %u!\n", NS_HC_TPM2_REPLACE_AIK, (uint32_t)sb_status);
        TOOL_WARN("AIK will be saved on disk only\n");
    }

    // Store the AIK keyblob to file
    {
        WOLFTPM2_KEYBLOB *key = &aik_keyblob;
        WOLFTPM2_BUFFER blob;
        word32 privBufferSz, pubBufferSz;

        // Test getting size only
        rc = wolfTPM2_GetKeyBlobAsSeparateBuffers(NULL, &pubBufferSz,
                                                  NULL, &privBufferSz, key);
        if (rc != LENGTH_ONLY_E)
        {
            printf("Store AIK keyblob error: Failed to get size! rc:0x%08X\n", rc);
            goto exit;
        }

        // Test exporting private and public parts separately
        rc = wolfTPM2_GetKeyBlobAsSeparateBuffers(blob.buffer, &pubBufferSz,
                                                  &blob.buffer[pubBufferSz], &privBufferSz, key);
        if (rc != TPM_RC_SUCCESS)
        {
            printf("Store AIK keyblob error: Failed to export priv key and pub key separately! rc:0x%08X\n", rc);
            goto exit;
        }

        // Test getting size only
        rc = wolfTPM2_GetKeyBlobAsBuffer(NULL, sizeof(blob.buffer), key);
        if (rc <= 0)
        {
            printf("Store AIK keyblob error: Failed to get key size! rc:0x%08X\n", rc);
            goto exit;
        }

        // Export private and public key
        rc = wolfTPM2_GetKeyBlobAsBuffer(blob.buffer, sizeof(blob.buffer), key);
        if (rc <= 0)
        {
            printf("Store AIK keyblob error: Failed to export private and public key! rc:0x%08X\n", rc);
            goto exit;
        }
        blob.size = rc;

        rc = writeBin(aik_keyblob_filepath, (byte *)&blob, (word32)(sizeof(blob)));
        if (rc != TPM_RC_SUCCESS)
        {
            printf("Store AIK keyblob error: Write AIK blob fail! rc:0x%08X\n", rc);
            goto exit;
        }
    }

    // Store the AIK PEM to file
    {
        byte pem[MAX_RSA_KEY_BYTES];
        word32 pemSz;

        pemSz = (word32)sizeof(pem);
        rc = wolfTPM2_RsaKey_TpmToPemPub(dev, &aik, pem, &pemSz);
        if (rc == 0)
        {
            rc = writeBin(aik_pem_filepath, pem, pemSz);
        }

        if (rc != TPM_RC_SUCCESS)
        {
            printf("Write AIK pem failed 0x%x\n", rc);
            goto exit;
        }
    }

    // On success
    printf("Create AIK succeed\n");
    rc = TPM_RC_SUCCESS;

exit:
    // Close key handles
    wolfTPM2_UnloadHandle(dev, &aik.handle);
    wolfTPM2_UnloadHandle(dev, &storage.handle);

    return rc;
}

static int _tpm2_quote_pcr(
    WOLFTPM2_DEV *dev,
    uint32_t srk_handle,
    const char *aik_keyblob_filepath,
    uint32_t pcr1,
    uint32_t pcr2,
    struct tpm_quote_nonce* nonce,
    const char *quote_blob_filepath,
    const char *quote_pcr_digest_filepath,
    const char *quote_signature_filepath)
{
    int rc = TPM_RC_FAILURE;
    BYTE *data = NULL;
    int dataSz;
    TPMS_ATTEST attestedData;
    TPMI_ALG_PUBLIC alg = TPM_ALG_RSA; /* TPM_ALG_ECC */
    WOLFTPM2_KEY storage;              /* SRK */
    WOLFTPM2_KEY aik;                  /* AIK */
    WOLFTPM2_KEYBLOB aik_keyblob;
    union
    {
        Quote_In quoteAsk;
        byte maxInput[MAX_COMMAND_SIZE];
    } cmdIn;
    union
    {
        Quote_Out quoteResult;
        byte maxOutput[MAX_RESPONSE_SIZE];
    } cmdOut;
    TPM_ALG_ID paramEncAlg = TPM_ALG_CFB;
    WOLFTPM2_SESSION tpmSession;
    XFILE f;

    // Check: Parameters must be valid
    if (!_tpm_check_pcr(pcr1) || !_tpm_check_pcr(pcr2))
        return BAD_FUNC_ARG;

    XMEMSET(&storage, 0, sizeof(storage));
    XMEMSET(&aik, 0, sizeof(aik));
    XMEMSET(&aik_keyblob, 0, sizeof(aik_keyblob));

    printf("\tInput AIK keyblob file: %s\n", aik_keyblob_filepath);
    printf("\tUse %s SRK/AIK\n", TPM2_GetAlgName(alg));
    printf("\tUse Parameter Encryption: %s\n", TPM2_GetAlgName(paramEncAlg));

    // Load AIK from disk
    // get SRK
    rc = _tpm20_srk_get(dev, srk_handle, &storage);
    if (rc != TPM_RC_SUCCESS)
        goto exit;

    // Try to get the persistent AIK from disk. If failed, get it from the AIK files.
    rc = _tpm20_aik_get(dev, UXMHF64_AIK_PERSISTED_HANDLE, &aik);
    if (rc != TPM_RC_SUCCESS)
    {
        // Read the AIK keyblob to file
        // rc = readKeyBlob(aik_keyblob_filepath, &aik_keyblob);
        // if(rc != TPM_RC_SUCCESS)
        // {
        //     printf("Read AIK blob failed 0x%x\n", rc);
        //     goto exit;
        // }
        {
            WOLFTPM2_KEYBLOB *key = &aik_keyblob;
            WOLFTPM2_BUFFER blob;
            word32 blob_size = (word32)sizeof(WOLFTPM2_BUFFER);

            rc = readBin(aik_keyblob_filepath, (byte *)&blob, &blob_size);
            if (rc != TPM_RC_SUCCESS)
            {
                printf("Load AIK: Read AIK blob failed! rc:0x%08X\n", rc);
                goto exit;
            }

            // Load key blob (private/public) from buffer
            rc = wolfTPM2_SetKeyBlobFromBuffer(key, blob.buffer, blob.size);
            if (rc != 0)
            {
                printf("Load AIK: Failed to set key blob from buffer! rc:0x%08X\n", rc);
                goto exit;
            }

            key->handle.auth.size = 0;
        }

        // Load key to TPM and get temp handle
        rc = wolfTPM2_LoadKey(dev, &aik_keyblob, &storage.handle);
        if (rc != 0)
        {
            printf("Load AIK: Failed to load key! rc:0x%08X\n", rc);
            goto exit;
        }

        // Construct <aik>
        XMEMCPY(&aik, &aik_keyblob, sizeof(WOLFTPM2_KEY));
    }

    // Generate TPM Quote
    if (paramEncAlg != TPM_ALG_NULL)
    {
        // Start an authenticated session (salted / unbound) with parameter encryption
        rc = wolfTPM2_StartSession(dev, &tpmSession, &storage, NULL,
                                   TPM_SE_HMAC, paramEncAlg);
        if (rc != 0)
        {
            goto exit;
        }
        printf("TPM2_StartAuthSession: sessionHandle 0x%x\n",
               (word32)tpmSession.handle.hndl);

        // set session for authorization of the storage key
        rc = wolfTPM2_SetAuthSession(dev, 1, &tpmSession,
                                     (TPMA_SESSION_decrypt | TPMA_SESSION_encrypt | TPMA_SESSION_continueSession));
        if (rc != 0)
        {
            goto exit;
        }
    }

    //// set auth for using the AIK
    wolfTPM2_SetAuthHandle(dev, 0, &aik.handle);

    //// Prepare Quote request
    XMEMSET(&cmdIn.quoteAsk, 0, sizeof(cmdIn.quoteAsk));
    XMEMSET(&cmdOut.quoteResult, 0, sizeof(cmdOut.quoteResult));
    cmdIn.quoteAsk.signHandle = aik.handle.hndl;
    cmdIn.quoteAsk.inScheme.scheme = alg == TPM_ALG_RSA ? TPM_ALG_RSASSA : TPM_ALG_ECDSA;
    cmdIn.quoteAsk.inScheme.details.any.hashAlg = TPM_ALG_SHA256;
    cmdIn.quoteAsk.qualifyingData.size = sizeof(struct tpm_quote_nonce);
    memcpy(cmdIn.quoteAsk.qualifyingData.buffer, nonce, cmdIn.quoteAsk.qualifyingData.size);

    //// Choose PCR for signing
    {
        byte pcrArray[PCR_SELECT_MAX];
        word32 pcrArraySz = 0;

        XMEMSET(pcrArray, 0, sizeof(pcrArray));

        pcrArray[pcrArraySz++] = pcr1;
        pcrArray[pcrArraySz++] = pcr2;

        TPM2_SetupPCRSelArray(&cmdIn.quoteAsk.PCRselect, TPM_ALG_SHA256, pcrArray, pcrArraySz);
    }

    //// Get the PCR measurement signed by the TPM using the AIK key
    rc = TPM2_Quote(&cmdIn.quoteAsk, &cmdOut.quoteResult);
    if (rc != TPM_RC_SUCCESS)
    {
        printf("TPM2_Quote failed 0x%x: %s\n", rc, TPM2_GetRCString(rc));
        goto exit;
    }
    printf("TPM2_Quote: Quote PCR %u and %u success\n", pcr1, pcr2);

    rc = TPM2_ParseAttest(&cmdOut.quoteResult.quoted, &attestedData);
    if (rc != TPM_RC_SUCCESS)
    {
        printf("TPM2_Packet_ParseAttest failed 0x%x: %s\n", rc,
               TPM2_GetRCString(rc));
        goto exit;
    }
    if (attestedData.magic != TPM_GENERATED_VALUE)
    {
        printf("\tError, attested data not generated by the TPM = 0x%X\n",
               attestedData.magic);
    }

    // Save quote to the disk
    //// Save quote blob to the disk
    data = (UINT8 *)&cmdOut.quoteResult.quoted;
    dataSz = *(UINT16 *)data;
    data += sizeof(UINT16); /* skip the size field of TPMS_ATTEST */
    // dataSz = (int)sizeof(TPMS_ATTEST) - sizeof(UINT16);
    f = XFOPEN(quote_blob_filepath, "wb");
    if (f != XBADFILE)
    {
        dataSz = (int)XFWRITE(data, 1, dataSz, f);
        XFCLOSE(f);
    }
    printf("Wrote %d bytes to %s\n", dataSz, quote_blob_filepath);

    rc = fs_chmod_file_0664(quote_blob_filepath);
    if(rc)
    {
        printf("Chmod 0664 on file %s error\n", quote_blob_filepath);
    }
    
    //// Save PCR digest of the quote
    data = (BYTE *)attestedData.attested.quote.pcrDigest.buffer;
    dataSz = attestedData.attested.quote.pcrDigest.size;
    f = XFOPEN(quote_pcr_digest_filepath, "wb");
    if (f != XBADFILE)
    {
        dataSz = (int)XFWRITE(data, 1, dataSz, f);
        XFCLOSE(f);
    }
    printf("Wrote %d bytes to %s\n", dataSz, quote_pcr_digest_filepath);

    rc = fs_chmod_file_0664(quote_pcr_digest_filepath);
    if(rc)
    {
        printf("Chmod 0664 on file %s error\n", quote_pcr_digest_filepath);
    }

    //// Save PCR digest of the quote
    data = (BYTE *)cmdOut.quoteResult.signature.signature.rsassa.sig.buffer;
    dataSz = cmdOut.quoteResult.signature.signature.rsassa.sig.size;
    f = XFOPEN(quote_signature_filepath, "wb");
    if (f != XBADFILE)
    {
        dataSz = (int)XFWRITE(data, 1, dataSz, f);
        XFCLOSE(f);
    }
    printf("Wrote %d bytes to %s\n", dataSz, quote_signature_filepath);

    rc = fs_chmod_file_0664(quote_signature_filepath);
    if(rc)
    {
        printf("Chmod 0664 on file %s error\n", quote_signature_filepath);
    }

    // On success
    printf("TPM with signature attests (type 0x%x):\n", attestedData.type);
    printf("\tTPM signed %lu count of PCRs\n",
           (unsigned long)attestedData.attested.quote.pcrSelect.count);
    rc = TPM_RC_SUCCESS;

exit:
    // Close key handles
    wolfTPM2_UnloadHandle(dev, &aik.handle);
    wolfTPM2_UnloadHandle(dev, &storage.handle);
    wolfTPM2_UnloadHandle(dev, &tpmSession.handle);

    return rc;
}

/************ Public Functions **********/
int tpm2_dev_init(void)
{
    int rc = TPM_RC_FAILURE;

    rc = wolfTPM2_Init(&tpm2_dev, TPM2_IoCb, NULL);
    if (rc != TPM_RC_SUCCESS)
    {
        tpm2_initialized = false;

        printf("wolfTPM2_Init failed 0x%x: %s\n", rc, TPM2_GetRCString(rc));
        return TPM_RC_FAILURE;
    }

    // On success
    tpm2_initialized = true;
    return TPM_RC_SUCCESS;
}

void tpm2_dev_fini(void)
{
    if (tpm2_initialized)
        wolfTPM2_Cleanup(&tpm2_dev);

    tpm2_initialized = false;
}

int tpm2_srk_create(void)
{
    int rc = TPM_RC_FAILURE;
    MHV_STATUS sb_status = MHV_SUCCESS;
    ulong_t srk_exist = false;
    TPM_HANDLE new_srk_handle = 0;
    WOLFTPM2_HANDLE new_srk_wolftpm_handle = {0};
    uint32_t srk_unique_first_uint32 = 0;

    // Check: TPM library must be initialized.
    if (!tpm2_is_initialized())
        return BAD_FUNC_ARG;

    // Check: SecAgent must be present, so SecBase is present.
    if (!secagent_test_presence())
        return TPM_RC_FAILURE;

    // Check: Does a good SRK exist at the handle <UXMHF64_SRK_PERSISTED_HANDLE>?
    sb_status = (MHV_STATUS)hypercall_ret1(NS_HC_TPM2_PROBE_SRK, &srk_exist);
    if (MHV_FAIL(sb_status))
    {
        TOOL_ERR("<tpm2_srk_create> Hypercall (%u) error: %u!\n", NS_HC_TPM2_PROBE_SRK, (uint32_t)sb_status);
        return TPM_RC_FAILURE;
    }

    if (srk_exist)
    {
        // GreenBox has created an accessible good SRK before and hence can reuse it.
        TOOL_INFO("SRK exists. No need to re-create it.\n");
        return TPM_RC_SUCCESS;
    }

    // Create a new SRK
    TOOL_INFO("Creating SRK...\n");
    rc = _tpm20_srk_create_new(&tpm2_dev, &new_srk_handle, &srk_unique_first_uint32);
    if (rc != TPM_RC_SUCCESS)
    {
        TOOL_ERR("<tpm2_srk_create> create SRK error! 0x%x: %s\n", rc, TPM2_GetRCString(rc));
        rc = TPM_RC_FAILURE;
        goto exit;
    }

    // Replace the persisted SRK if any.
    TOOL_INFO("Persisting SRK...\n");
    sb_status = (MHV_STATUS)hypercall_2args(NS_HC_TPM2_REPLACE_SRK, new_srk_handle, srk_unique_first_uint32);
    if (MHV_FAIL(sb_status))
    {
        TOOL_ERR("<tpm2_srk_create> Hypercall (%u) error: %u!\n", NS_HC_TPM2_REPLACE_SRK, (uint32_t)sb_status);
        rc = TPM_RC_FAILURE;
        goto exit;
    }

    // On success
    TOOL_INFO("Created a new persistent SRK\n");
    rc = TPM_RC_SUCCESS;

exit:
    new_srk_wolftpm_handle.hndl = new_srk_handle;
    wolfTPM2_UnloadHandle(&tpm2_dev, &new_srk_wolftpm_handle);
    return rc;
}

int tpm2_aik_create(void)
{
    int ret = 0;
    char aik_keyblob_filepath[FS_MAX_PATH_BUFFER_LEN] = {0};
    char aik_pem_filepath[FS_MAX_PATH_BUFFER_LEN] = {0};

    // Check: TPM library must be initialized.
    if (!tpm2_is_initialized())
        return BAD_FUNC_ARG;

    // Check: SecAgent must be present, so SecBase is present.
    if (!secagent_test_presence())
        return TPM_RC_FAILURE;

    gb_get_tpmtools_userdata_absolute_path(aik_keyblob_filepath);
    strcat(aik_keyblob_filepath, "aik.keyblob");
    ret = fs_create_directories(aik_keyblob_filepath);
    if (ret)
    {
        printf("Create directory for aik.keyblob error! ret: 0x%x\n", ret);
        return ret;
    }

    gb_get_tpmtools_userdata_absolute_path(aik_pem_filepath);
    strcat(aik_pem_filepath, "aik.pem");
    ret = fs_create_directories(aik_pem_filepath);
    if (ret)
    {
        printf("Create directory for aik.pem error! ret: 0x%x\n", ret);
        return ret;
    }

    ret = _tpm2_aik_create_new(&tpm2_dev, UXMHF64_SRK_PERSISTED_HANDLE, aik_pem_filepath, aik_keyblob_filepath);
    if (ret != TPM_RC_SUCCESS)
    {
        printf("Create AIK failed! ret: 0x%x\n", ret);
        return ret;
    }

    ret = tpm2_aik_convert_format();
    if (ret)
    {
        printf("Create AIK format failed! ret: %d\n", ret);
        return ret;
    }

    // On success
    return 0;
}

int tpm2_quote_pcr(uint32_t pcr1, uint32_t pcr2, struct tpm_quote_nonce* nonce)
{
    int ret = 0;
    char aik_keyblob_filepath[FS_MAX_PATH_BUFFER_LEN] = {0};
    char quote_blob_filepath[FS_MAX_PATH_BUFFER_LEN] = {0};
    char quote_pcr_digest_filepath[FS_MAX_PATH_BUFFER_LEN] = {0};
    char quote_signature_filepath[FS_MAX_PATH_BUFFER_LEN] = {0};

    // Check: params must be valid
    if (!_tpm_check_pcr(pcr1) || !_tpm_check_pcr(pcr2))
        return BAD_FUNC_ARG;
    if (!nonce)
        return BAD_FUNC_ARG;

    // Check: TPM library must be initialized.
    if (!tpm2_is_initialized())
        return BAD_FUNC_ARG;

    gb_get_tpmtools_userdata_absolute_path(aik_keyblob_filepath);
    strcat(aik_keyblob_filepath, "aik.keyblob");
    ret = fs_create_directories(aik_keyblob_filepath);
    if (ret)
    {
        printf("Create directory for aik.keyblob error! ret: 0x%x\n", ret);
        return ret;
    }

    gb_get_tpmtools_userdata_absolute_path(quote_blob_filepath);
    strcat(quote_blob_filepath, "quote.blob");
    ret = fs_create_directories(quote_blob_filepath);
    if (ret)
    {
        printf("Create directory for quote.blob error! ret: 0x%x\n", ret);
        return ret;
    }

    gb_get_tpmtools_userdata_absolute_path(quote_pcr_digest_filepath);
    strcat(quote_pcr_digest_filepath, "quote.pcrs");
    ret = fs_create_directories(quote_pcr_digest_filepath);
    if (ret)
    {
        printf("Create directory for quote.pcrs error! ret: 0x%x\n", ret);
        return ret;
    }

    gb_get_tpmtools_userdata_absolute_path(quote_signature_filepath);
    strcat(quote_signature_filepath, "quote.sig");
    ret = fs_create_directories(quote_signature_filepath);
    if (ret)
    {
        printf("Create directory for quote.sig error! ret: 0x%x\n", ret);
        return ret;
    }

    return _tpm2_quote_pcr(&tpm2_dev, UXMHF64_SRK_PERSISTED_HANDLE, aik_keyblob_filepath, pcr1, pcr2, nonce, quote_blob_filepath, quote_pcr_digest_filepath, quote_signature_filepath);
}

int tpm2_pcr_read(uint32_t pcr, uint8_t *value)
{
    int rc = TPM_RC_FAILURE;
    int pcrIndex = pcr;
    union
    {
        PCR_Read_In pcrRead;
        byte maxInput[MAX_COMMAND_SIZE];
    } cmdIn;
    union
    {
        PCR_Read_Out pcrRead;
        byte maxOutput[MAX_RESPONSE_SIZE];
    } cmdOut;

    // Check: params must be valid
    if (!_tpm_check_pcr(pcr))
        return BAD_FUNC_ARG;
    if (!value)
        return BAD_FUNC_ARG;

    // Check: TPM library must be initialized.
    if (!tpm2_is_initialized())
        return BAD_FUNC_ARG;

    XMEMSET(&cmdIn.pcrRead, 0, sizeof(cmdIn.pcrRead));
    // Note: TPM2_SetupPCRSel can be called with multiple PCR indexes to
    // read more than one at a time
    TPM2_SetupPCRSel(&cmdIn.pcrRead.pcrSelectionIn, TPM_ALG_SHA256, pcrIndex);
    rc = TPM2_PCR_Read(&cmdIn.pcrRead, &cmdOut.pcrRead);
    if (rc != TPM_RC_SUCCESS)
    {
        printf("TPM2_PCR_Read failed 0x%x: %s\n", rc, TPM2_GetRCString(rc));
        goto exit;
    }

    // Return result
    for (int i = 0; i < (int)cmdOut.pcrRead.pcrValues.count; i++)
    {
        // printf("PCR%d (idx %d) digest:\n", pcrIndex, i);
        // for (j=0; j < cmdOut.pcrRead.pcrValues.digests[i].size; j++)
        //     printf("%02X", cmdOut.pcrRead.pcrValues.digests[i].buffer[j]);
        // printf("\n");
        memcpy(value, cmdOut.pcrRead.pcrValues.digests[i].buffer, cmdOut.pcrRead.pcrValues.digests[i].size);
    }

    // On success
    rc = TPM_RC_SUCCESS;

exit:
    return rc;
}

int tpm2_save_mhv_pcrs_to_file(uint32_t pcr1, uint32_t pcr2)
{
    int rc = TPM_RC_FAILURE;
    char pcr1_filepath[FS_MAX_PATH_BUFFER_LEN] = {0};
    char pcr2_filepath[FS_MAX_PATH_BUFFER_LEN] = {0};
    uint8_t value[SHA256_DIGEST_LENGTH] = {0};

    // Check: params must be valid
    if (!_tpm_check_pcr(pcr1) || !_tpm_check_pcr(pcr2))
        return BAD_FUNC_ARG;

    // Check: TPM library must be initialized.
    if (!tpm2_is_initialized())
        return BAD_FUNC_ARG;

    gb_get_tpmtools_userdata_absolute_path(pcr1_filepath);
    strcat(pcr1_filepath, "pcr1.blob");
    gb_get_tpmtools_userdata_absolute_path(pcr2_filepath);
    strcat(pcr2_filepath, "pcr2.blob");

    // Read PCR <pcr1> to file
    rc = tpm2_pcr_read(pcr1, value);
    if (rc != TPM_RC_SUCCESS)
    {
        printf("TPM Tool: Read PCR 4 error!\n");
        return TPM_RC_FAILURE;
    }

    rc = writeBin(pcr1_filepath, value, SHA256_DIGEST_LENGTH);
    if (rc != TPM_RC_SUCCESS)
    {
        printf("TPM Tool: Save PCR 3 to file error! rc:0x%08X\n", rc);
        return TPM_RC_FAILURE;
    }

    // Read PCR <pcr2> to file
    rc = tpm2_pcr_read(pcr2, value);
    if (rc != TPM_RC_SUCCESS)
    {
        printf("TPM Tool: Read PCR 15 error!\n");
        return TPM_RC_FAILURE;
    }

    rc = writeBin(pcr2_filepath, value, SHA256_DIGEST_LENGTH);
    if (rc != TPM_RC_SUCCESS)
    {
        printf("TPM Tool: Save PCR 15 to file error! rc:0x%08X\n", rc);
        return TPM_RC_FAILURE;
    }

    // On success
    return TPM_RC_SUCCESS;
}

int secbase_get_tpm_pcrs(NS_HC_PARAMS_TPM_GET_SECBASE_MEASUREMENT *out_result)
{
    MHV_STATUS sb_status = MHV_SUCCESS;

    // Check: params must be valid
    if (!out_result)
        return BAD_FUNC_ARG;

    // Check: TPM library must be initialized.
    if (!tpm2_is_initialized())
        return BAD_FUNC_ARG;

    sb_status = (MHV_STATUS)hypercall_1arg(NS_HC_TPM_GET_SECBASE_MEASUREMENT, (ulong_t)out_result);
    if (MHV_FAIL(sb_status))
    {
        printf("<secbase_get_tpm_pcrs> Hypercall error: %u\n", (uint32_t)sb_status);
        return TPM_RC_FAILURE;
    }

    // On success
    return TPM_RC_SUCCESS;
}