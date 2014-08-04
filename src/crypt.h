/*
 * encrypt ~ a simple, modular, (multi-OS) encryption utility
 * Copyright © 2005-2014, albinoloverats ~ Software Development
 * email: encrypt@albinoloverats.net
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef _ENCRYPT_CRYPTO_H_
#define _ENCRYPT_CRYPTO_H_

/*!
 * \file    crypt.h
 * \author  Ashley M Anderson
 * \date    2009-2014
 * \brief   Main crypt header file
 *
 * What is essentially the parent class for enc/decryption routines.
 * Many of the constants, wrapper functions exist here.
 */

#include <stdint.h>     /*!< Necessary include as c99 standard integer types are referenced in this header */
#include <stdbool.h>    /*!< Necessary include as c99 standard boolean type is referenced in this header */
#include <time.h>       /*!< Necessary include as time_t type is referenced in this header */
#include <pthread.h>    /*!< Necessary include as pthread handle is referenced in this header */
#include <gcrypt.h>     /*!< Necessary include as encryption modes are referenced in this header */

#include "crypt_io.h"   /*!< Necessary as IO_HANDLE type is referenced in this header */

#define ENCRYPT_VERSION "2014.XX" /*!< Current (display) version of encrypt application */
#define UPDATE_URL "https://albinoloverats.net/encrypt.release" /*!< URI to check for updates */

#define HEADER_0 0x3697de5d96fca0fallu              /*!< The first 8 bytes of an encrypted file */
#define HEADER_1 0xc845c2fa95e2f52dllu              /*!< The second 8 bytes of an encrypted file */

#define BLOCK_SIZE 1024 /*!< Default IO block size */

#ifndef GIT_COMMIT
    #define GIT_COMMIT "unknown"
#endif
#define GIT_COMMIT_LENGTH 7

#define DEFAULT_CIPHER "AES"
#define DEFAULT_HASH "SHA256"
#define DEFAULT_MODE "OFB"

/*!
 * \brief  Encryption status
 *
 * The value used by the encryption/decryption routines to indicate
 * their status.
 */
typedef enum
{
    /* success and running states */
    STATUS_SUCCESS,                         /*!< Success */
    STATUS_INIT,                            /*!< Initialisation in progress or complete */
    STATUS_RUNNING,                         /*!< Execution is in progress */
    STATUS_CANCELLED,                       /*!< User cancelled the operation */
    /* failures - decryption did not complete */
    STATUS_FAILED_INIT,                     /*!< Error during initialisation */
    STATUS_FAILED_UNKNOWN_VERSION,          /*!< Failed due to unknown/unsupported encrypt data stream version */
    STATUS_FAILED_UNKNOWN_CIPHER_ALGORITHM, /*!< Failed due to unknown/unsupported algorithm (cipher or hash) */
    STATUS_FAILED_UNKNOWN_HASH_ALGORITHM,   /*!< Failed due to unknown/unsupported algorithm (cipher or hash) */
    STATUS_FAILED_UNKNOWN_CIPHER_MODE,      /*!< Failed due to unknown/unsupported algorithm (cipher or hash) */
    STATUS_FAILED_DECRYPTION,               /*!< Failed decryption verification (likely wrong password) */
    STATUS_FAILED_UNKNOWN_TAG,              /*!< Failed due to unknown tag */
    STATUS_FAILED_IO,                       /*!< Read/write error */
    STATUS_FAILED_KEY,                      /*!< Key generation/read error */
    STATUS_FAILED_OUTPUT_MISMATCH,          /*!< Tried to write directory into a file or vice-versa */
    STATUS_FAILED_OTHER,                    /*!< Unknown error */
    /* warnings - decryption finished but with possible errors */
    STATUS_WARNING_CHECKSUM,                /*!< Data checksum was invalid, possible data corruption */
    STATUS_WARNING_LINK                     /*!< Warning where links are unsupported by the system */
}
crypto_status_e;

/*!
 * \brief  File type tags
 *
 * File type when encryption entire directory hierarchies. Packed as it
 * will only be stored as a single byte in the encrypted data. (Whether
 * it's packed in memory is not actually up to us, but hey, we tried.)
 */
typedef enum
{
    FILE_DIRECTORY, /*!< File is a directory */
    FILE_REGULAR,   /*!< File is a file */
    FILE_SYMLINK,   /*!, File is a soft link */
    FILE_LINK       /*!, File is a hard link */
} __attribute__((packed))
file_type_e;

/*!
 * \brief  File container version
 *
 * An enum of the encrypted file container version. The values
 * correspond to the #defined values.
 */
typedef enum
{
    VERSION_UNKNOWN = 0,              /*!< Unknown version, or not encrypted  */
    VERSION_2011_08,                  /*!< Version 2011.08 */
    VERSION_2011_10,                  /*!< Version 2011.10 */
    VERSION_2012_11,                  /*!< Version 2012.11 */
    VERSION_2013_02,                  /*!< Version 2013.02 */
    VERSION_2013_11,                  /*!< Version 2013.11 */
    VERSION_2014_06,                  /*!< Version 2014.06 */

    VERSION_CURRENT//= VERSION_DEVELOPMENT /*!< Next release / current development version */
}
version_e;

/*!
 * \brief  Stream metadata tags
 *
 * Tag values used to provide metadata capabilities to the encrypted
 * data stream. NB new tags obviously break backward compatibility.
 */
typedef enum
{
    TAG_SIZE,       /*!< Encrypted data size */
    TAG_BLOCKED,    /*!< Data is split into blocks (of given size) */
    TAG_COMPRESSED, /*!< Data is compressed */
    TAG_DIRECTORY   /*!< Data is a directory hierarchy */
        /*
         * TODO add tags for stat data (mode, atime, ctime, mtime)
         */
} __attribute__((packed))
stream_tags_e;

/*!
 * \brief  Current progress
 *
 * Provide the foreground thread a way to check on the progress. Thus a
 * percentage can be calculated using 100 * offset / size. Either the
 * number of bytes, or directory entries depending on what you're taking
 * the progress of.
 */
typedef struct
{
    uint64_t offset; /*!< Progress */
    uint64_t size;   /*!< Maximum */
}
progress_t;

#if 0
/*!
 * \brief  Key data structure
 *
 * Key the user key data/length together
 */
typedef struct
{
    uint8_t *data;  /*!< Key data */
    size_t length; /*!< Key data length */
}
raw_key_t;
#endif

/*!
 * \brief  Main cryptographic structure
 *
 * This is essentially the cryptographic handle, produced by the encrypt
 * or decrypt initialisation functions and given to execute. Some
 * information can be extracted, such as progress and status.
 */
typedef struct
{
    IO_HANDLE source;              /*!< Where to get data from */
    IO_HANDLE output;              /*!< Where to put data to */

    char *path;                    /*!< Path to en/decrypted directory */
    enum gcry_cipher_algos cipher; /*!< The chosen cipher algorithm */
    enum gcry_md_algos hash;       /*!< The chosen key hash algorithm */
    enum gcry_cipher_modes mode;   /*!< The chosen encryption mode */

#if 0
    raw_key_t *raw_key;            /*!< Encryption key (NB Not yet used) */
#endif
    uint8_t *key;                  /*!< Key data */
    size_t length;                 /*!< Key data length */

    pthread_t *thread;             /*!< Execution thread */
    void *(*process)(void *);      /*!< Main processing function; used by execute() */
    crypto_status_e status;        /*!< Current status */
    progress_t current;            /*!< Progress of current file */
    progress_t total;              /*!< Overall progress (all files) */

    void *misc;                    /*!< Miscellaneous data, specific to either encryption or decryption only */

    version_e version;             /*!< Version of the encrypted file container */
    uint64_t blocksize;            /*!< Whether data is split into blocks, and thus their size */
    bool compressed:1;             /*!< Whether data stream is compress */
    bool directory:1;              /*!< Whether data stream is a directory hierarchy */
    bool follow_links:1;           /*!< Whether encrypt should follow symlinks (true: store the file it points to; false: store the link itself */
    bool raw:1;                    /*!< Whether the header should be skipped (not recommended but ideal in some situations) */
}
crypto_t;

/*!
 * \brief          Initialise libgcrypt library
 *
 * Initialise the libgcrypt library. Subsequent calls to the function
 * are ignored.
 */
extern void init_crypto(void);

/*!
 * \brief          Execute crypto routine
 * \params[in]  c  Cryptographic instance
 *
 * Launches a background thread that performs the desired cryptographic
 * action setup by either encrypt_init() or decrypt_init(). The task is
 * backgrounded to allow the foreground to keep the UI updated (if
 * necessary).
 */
extern void execute(crypto_t *c) __attribute__((nonnull(1)));

/*!
 * \brief         Get a meaningful status message
 * \param[in]  c  Cryptographic instance
 * \return        Status message
 *
 * Get a meaningful status message which corresponds to the current
 * status of the crypto instance.
 */
extern const char *status(const crypto_t * const restrict c) __attribute__((nonnull(1)));

/*!
 * \brief         Deinitialise a cryptographic instance
 * \param[in]  c  A pointer to the instance to release
 *
 * Free's the resources used by the crypto instance after it is no
 * longer needed.
 */
extern void deinit(crypto_t **c) __attribute__((nonnull(1)));

#if 0
/*!
 * \brief         Destroy key structure
 * \param[in]  k  A pointer to a key structure
 *
 * Free the given key data structure; the key data is cleared (memset to
 * zero and then freed).
 */
extern void key_free(raw_key_t **k);
#endif

/*!
 * \brief         Get list of usable ciphers
 * \return        An array of char* of cipher names
 *
 * Get an array of strings which lists the names of usable cipher
 * algorithms. NB: The array is allocated statically and SHOULD NOT be
 * free'd (or otherwise altered).
 */
extern const char **list_of_ciphers(void) __attribute__((pure));

/*!
 * \brief         Get list of usable hashes
 * \return        An array of char* of hash names
 *
 * Get an array of strings which lists the names of usable hash
 * algorithms. NB: The array is allocated statically and SHOULD NOT be
 * free'd (or otherwise altered).
 */
extern const char **list_of_hashes(void) __attribute__((pure));

/*!
 * \brief         Get list of available cipher modes
 * \return        An array of char* of mode names
 *
 * Get an array of strings which lists the names of available cipher
 * modes. NB: The array is allocated statically and SHOULD NOT be
 * free'd (or otherwise altered).
 */
extern const char **list_of_modes(void) __attribute__((pure));

/*!
 * \brief         Get cipher ID, given its name
 * \param[in]  n  Cipher name
 * \return        The ID used by libgcrypt
 *
 * Get the ID used internally by libgcrypt for the given cipher name.
 */
extern enum gcry_cipher_algos cipher_id_from_name(const char * const restrict n) __attribute__((pure, nonnull(1)));

/*!
 * \brief         Get hash ID, given its name
 * \param[in]  n  Hash name
 * \return        The ID used by libgcrypt
 *
 * Get the ID used internally by libgcrypt for the given hash name.
 */
extern enum gcry_md_algos hash_id_from_name(const char * const restrict n) __attribute__((pure, nonnull(1)));

/*!
 * \brief         Get cipher mode ID, given its name
 * \param[in]  n  Mode name
 * \return        The ID used by libgcrypt
 *
 * Get the ID used internally by libgcrypt for the given mode name.
 */
extern enum gcry_cipher_modes mode_id_from_name(const char * const restrict n) __attribute__((pure, nonnull(1)));

/*!
 * \brief         Get cipher name, given its ID
 * \param[in]  c  The libgcrypt cipher enum
 * \return        A string representation of the cipher
 *
 * "Correct" the name that libgcrypt gives us; this ensures
 * compatibility with the Android version.
 */
extern const char *cipher_name_from_id(enum gcry_cipher_algos c) __attribute__((pure));

/*!
 * \brief         Get hash name, given its ID
 * \param[in]  h  The libgcrypt hash enum
 * \return        A string representation of the hash
 *
 * "Correct" the name that libgcrypt gives us; this ensures
 * compatibility with the Android version.
 */
extern const char *hash_name_from_id(enum gcry_md_algos h) __attribute__((pure));

/*!
 * \brief         Get cipher mode name, given its ID
 * \param[in]  m  The libgcrypt mode enum
 * \return        A string representation of the mode
 *
 * As this isn't provided by libgcrypt, in the same way that it is for
 * algorithms, and the fact we're artificially limiting the choices,
 * here's a function to get the name from the enum.
 */
extern const char *mode_name_from_id(enum gcry_cipher_modes m) __attribute__((pure));

/*!
 * \brief         Get cipher mode name, given its ID
 * \param[in]  m  The libgcrypt mode enum
 * \return        A string representation of the mode
 *
 * As this isn't provided by libgcrypt, in the same way that it is for
 * algorithms, and the fact we're artificially limiting the choices,
 * here's a function to get the name from the enum.
 */
extern const char *mode_name_from_id(enum gcry_cipher_modes m) __attribute__((pure));


#define IS_ENCRYPTED_ARGS_COUNT(...) IS_ENCRYPTED_ARGS_COUNT2(__VA_ARGS__, 4, 3, 2, 1)
#define IS_ENCRYPTED_ARGS_COUNT2(_1, _2, _3, _4, _, ...) _

#define is_encrypted_1(A)           is_encrypted_aux(false, A, NULL, NULL, NULL)
#define is_encrypted_4(A, B, C, D)  is_encrypted_aux(true, A, B, C, D)
#define is_encrypted(...) CONCAT(is_encrypted_, IS_ENCRYPTED_ARGS_COUNT(__VA_ARGS__))(__VA_ARGS__)

/*!
 * \brief         Determine if a file is encrypted
 * \param[in]  b  Whether passing in 3 arguments or not
 * \param[in]  n  The file path/name
 * \param[out] c  Pointer to cipher (free when no longer needed)
 * \param[out] h  Pointer to hash (free when no longer needed)
 * \param[out] m  Pointer to mode (if available) (free when no longer needed)
 * \return        The version of encrypted used
 *
 * Returns the version of encrypt used to encrypt the file, or 0 if it's
 * not encrypted.
 */
extern version_e is_encrypted_aux(bool b, const char *n, char **c, char **h, char **m) __attribute__((nonnull(2)));

/*!
 * \brief         Log which version the file is encrypted with
 * \param[in]  m  The bytes read from the file
 * \return        The version; 0 if unknown
 *
 * Check which version of encrypt a file was encrypted with.
 */
extern version_e check_version(uint64_t m);

/*!
 * \brief         Get the version ID
 * \param[in]  v  The version
 * \return        The version ID
 *
 * Get the version magic number which corresponds to the version enum.
 */
extern uint64_t get_version(version_e v);

/*!
 * \brief         Get the version as a string
 * \param[in]  v  The version
 * \return        The version as a string
 *
 * Get the version string which corresponds to the version enum.
 */
extern const char *get_version_string(version_e v);

/*!
 * \brief         Parse the version from a string
 * \param[in]  v  The version as a string
 * \return        The version
 *
 * Parse the version string and return the corresponds version enum.
 */
extern version_e parse_version(const char *v);

#endif /* ! _ENCRYPT_CRYPTO_H */
