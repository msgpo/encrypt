/*
 * encrypt ~ a simple, modular, (multi-OS) encryption utility
 * Copyright © 2005-2013, albinoloverats ~ Software Development
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

#ifndef _ENCRYPT_IO_H_
#define _ENCRYPT_IO_H_

/*!
 * \file    io.h
 * \author  Ashley M Anderson
 * \date    2009-2013
 * \brief   IO functions for encrypt
 *
 * Advanced IO functions for encryption/compression. Wraps read/write
 * with crypto functions and LZMA compression for encrypt.
 */

#include <stdint.h> /*!< Necessary include as c99 standard integer types are referenced in this header */

#define IO_STDIN_FILENO io_use_stdin() /*!< Macro wrapper for io_use_stdin() */
#define IO_STDOUT_FILENO io_use_stdout() /*!< Macro wrapper for io_use_stdout() */

typedef void * IO_HANDLE; /*<! Handle type for IO functions */

/*!
 * \brief  Extra options passed to IO crypto init
 *
 * A structure for any additional options necessary to revert to an
 * older setup for encryption/decryption. As things are imporved going
 * forward, it is still necessary to keep backwards compatibility with
 * previous versions.
 */
typedef struct
{
    bool x_iv:1; /*!< Whether to use the older (less correct) IV generation */
    unsigned int x_hz:15; /*!< The number of iterations for key/IV generation */
}
io_extra_t;

/*!
 * \brief         Open a file
 * \param[in]  n  The file name
 * \param[in]  f  File open flags
 * \param[in]  m  File open mode
 *
 * Open a file. Using these IO functions wraps the typical IO functions
 * with encryption and compression support.
 */
extern IO_HANDLE io_open(const char *n, int f, mode_t m);

/*!
 * \brief         Destroy an IO instance
 * \param[in]  h  An IO instance to destroy
 *
 * Close's the file and free resources when no longer needed.
 */
extern int io_close(IO_HANDLE h);

/*!
 * \brief         Get IO instance for STDIN
 * \return        An IO instance for STDIN
 *
 * Get an IO_HANDLE instance for STDIN stream.
 * instead.
 */
extern IO_HANDLE io_use_stdin(void);

/*!
 * \brief         Get IO instance for STDOUT
 * \return        An IO instance for STDOUT
 *
 * Get an IO_HANDLE instance for STDOUT stream.
 */
extern IO_HANDLE io_use_stdout(void);

/*!
 * \brief         Check if IO instance is STDIN
 * \param[in]  h  An IO instance
 * \return        Whether IO instance is STDIN
 *
 * Returns true if IO_HANDLE instance is STDIN stream.
 */
extern bool io_is_stdin(IO_HANDLE h);

/*!
 * \brief         Check if IO instance is STDOUT
 * \param[in]  h  An IO instance
 * \return        Whether IO instance is STDOUT
 *
 * Returns true if IO_HANDLE instance is STDOUT stream.
 */
extern bool io_is_stdout(IO_HANDLE h);

/*!
 * \brief         Write data
 * \param[in]  f  An IO instance
 * \param[in]  d  The data to write
 * \param[in]  l  The length of data to write
 * \return        The number of bytes written
 *
 * Write the given data to the given file descriptor, performing any
 * necessary operations before it is actually written. Returns the
 * number of bytes actually written.
 */
extern ssize_t io_write(IO_HANDLE f, const void *d, size_t l);

/*!
 * \brief         Read data
 * \param[in]  f  An IO instance
 * \param[out] d  The data read
 * \param[in]  l  The length of data to read (size of d)
 * \return        The number of bytes read
 *
 * Read the specified number of bytes from the given file descriptor,
 * performing any necessary operations before it's returned.
 */
extern ssize_t io_read(IO_HANDLE f, void *d, size_t l);

/*!
 * \brief         Sync data waiting to be written
 * \param[in]  f  An IO instance
 *
 * Performs a sync of all outstanding data to be written.
 */
extern int io_sync(IO_HANDLE f);

/*!
 * \brief         Seek to position in file
 * \param[in]  f  An IO instance
 * \param[in]  o  The offset set position
 * \param[in]  w  From whence the fofset should be measured
 * \return        The new offset within the file
 *
 * This function is the same as lseek(), in both the accepted values
 * for the offset and whence, as well as the return value.
 */
extern off_t io_seek(IO_HANDLE f, off_t o, int w);

/*
 * \brief         Encryption/Decryption initialisation
 * \param[in]  f  An IO instance
 * \param[in]  c  The name of the cipher to use
 * \param[in]  h  The name of the hash to use for key generation
 * \param[in]  k  Raw key data
 * \param[in]  l  The length of the key data
 * \param[in]  x  Any extra modifing options
 *
 * Initialise encryption/decryption of data read/written. This is then
 * active for the rest of the life of the IO_HANDLE.
 */
extern void io_encryption_init(IO_HANDLE f, const char *c, const char *h, const uint8_t *k, size_t l, io_extra_t x);

/*
 * \brief         Compression initialisation
 * \param[in]  f  An IO instance
 *
 * Turn on compression/decompression for the rest of the life of this
 * handle.
 */
extern void io_compression_init(IO_HANDLE f);

/*
 * \brief         Read/Write data checksum initialisation
 * \param[in]  f  An IO instance
 *
 * For all subsequent data that is read/written feed it through a hash
 * function as a way of generating a checksum to detect errors.
 */
extern void io_encryption_checksum_init(IO_HANDLE f, char *h);

/*
 * \brief         Read/Write data checksum generation
 * \param[in]  f  An IO instance
 *
 * Retrieve the hash checksum of all data read/written so far.
 */
extern void io_encryption_checksum(IO_HANDLE ptr, uint8_t **b, size_t *l);

#endif /* ! _ENCRYPT_IO_H_ */
