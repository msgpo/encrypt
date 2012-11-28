/*
 * encrypt ~ a simple, modular, (multi-OS,) encryption utility
 * Copyright © 2005-2012, albinoloverats ~ Software Development
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

#include <unistd.h>
#include <stdbool.h>
#include <gcrypt.h>
#include <lzma.h>

#include "common/common.h"
#include "common/error.h"
#include "common/logging.h"

#ifdef _WIN32
    #include "common/win32_ext.h"
#endif

#include "io.h"

typedef enum eof_e
{
    EOF_NO,
    EOF_MAYBE,
    EOF_YES
}
eof_e;

extern ssize_t lzma_write(int64_t f, const void * const restrict d, size_t l, io_params_t *c)
{
    lzma_action x = LZMA_RUN;
    if (!d && !l)
        x = LZMA_FINISH;
    c->lzma->next_in = d;
    c->lzma->avail_in = l;

    uint8_t stream = 0x00;
    c->lzma->next_out = &stream;
    c->lzma->avail_out = sizeof stream;
    do
    {
        bool lzf = false;
        switch (lzma_code(c->lzma, x))
        {
            case LZMA_STREAM_END:
                lzf = true;
            case LZMA_OK:
                break;
            default:
                die(_("Unexpected error during compression"));
        }
        if (c->lzma->avail_out == 0)
        {
            enc_write(f, &stream, sizeof stream, c);
            c->lzma->next_out = &stream;
            c->lzma->avail_out = sizeof stream;
        }
        if (lzf && c->lzma->avail_in == 0 && c->lzma->avail_out == sizeof stream)
            return l;
    }
    while (x == LZMA_FINISH || c->lzma->avail_in > 0);

    return l;
}

extern ssize_t lzma_read(int64_t f, void * const d, size_t l, io_params_t *c)
{
    lzma_action a = LZMA_RUN;
    static eof_e eof = EOF_NO;

    c->lzma->next_out = d;
    c->lzma->avail_out = l;

    if (eof == EOF_YES)
        return 0;
    else if (eof == EOF_MAYBE)
    {
        a = LZMA_FINISH;
        goto proc_remain;
    }

    while (true)
    {
        uint8_t buf = 0x00;
        if (c->lzma->avail_in == 0)
        {
            c->lzma->next_in = &buf;
            if ((c->lzma->avail_in = enc_read(f, &buf, 1, c)) < 1)
                a = LZMA_FINISH;
        }
proc_remain:
        switch (lzma_code(c->lzma, a))
        {
            case LZMA_STREAM_END:
                eof = EOF_MAYBE;
            case LZMA_OK:
                break;
            default:
                die(_("Unexpected error during decompression"));
        }

        if (c->lzma->avail_out == 0 || eof != EOF_NO)
            return l - c->lzma->avail_out;
    }
}

extern int lzma_sync(int64_t f, io_params_t *c)
{
    lzma_write(f, NULL, 0, c);
    enc_sync(f, c);
    return 0;
}

extern ssize_t enc_write(int64_t f, const void * const restrict d, size_t l, io_params_t *c)
{
    static uint8_t *stream = NULL;
    static size_t block = 0;
    static off_t offset[2] = { 0, 0 }; /* 0: length of data buffered so far (in stream); 1: length of data processed (from d) */
    if (!block)
        gcry_cipher_algo_info(c->algorithm, GCRYCTL_GET_BLKLEN, NULL, &block);
    if (!stream)
        if (!(stream = calloc(block, sizeof( uint8_t ))))
            die(_("Out of memory @ %s:%d:%s [%zu]"), __FILE__, __LINE__, __func__, block * sizeof( uint8_t ));

    size_t remainder[2] = { l, block - offset[0] }; /* 0: length of data yet to buffer (from d); 1: available space in output buffer (stream) */
    if (!d && !l)
    {
#ifdef __DEBUG__
        memset(stream + offset[0], 0x00, remainder[1]);
#else
        gcry_create_nonce(stream + offset[0], remainder[1]);
        gcry_cipher_encrypt(c->cipher, stream, block, NULL, 0);
#endif
        ssize_t e = write(f, stream, block);
        fsync(f);
        block = 0;
        free(stream);
        stream = NULL;
        memset(offset, 0x00, sizeof offset);
        return e;
    }

    offset[1] = 0;
    while (remainder[0])
    {
        if (remainder[0] < remainder[1])
        {
            memcpy(stream + offset[0], d + offset[1], remainder[0]);
            offset[0] += remainder[0];
            return l;
        }
        memcpy(stream + offset[0], d + offset[1], remainder[1]);
#ifndef __DEBUG__
        gcry_cipher_encrypt(c->cipher, stream, block, NULL, 0);
#endif
        ssize_t e = EXIT_SUCCESS;
        if ((e = write(f, stream, block)) < 0)
            return e;
        offset[0] = 0;
        memset(stream, 0x00, block);
        offset[1] += remainder[1];
        remainder[0] -= remainder[1];
        remainder[1] = block - offset[0];
    }
    return l;
}

extern ssize_t enc_read(int64_t f, void * const d, size_t l, io_params_t *c)
{
    static uint8_t *stream = NULL;
    static size_t block = 0;
    static size_t offset[3] = { 0, 0, 0 }; /* 0: length of available data in input buffer (stream); 1: available space in read buffer (d); 2: next available memory location for data (from d) */
    if (!block)
        gcry_cipher_algo_info(c->algorithm, GCRYCTL_GET_BLKLEN, NULL, &block);
    if (!stream)
        if (!(stream = calloc(block, sizeof( uint8_t ))))
            die(_("Out of memory @ %s:%d:%s [%zu]"), __FILE__, __LINE__, __func__, block * sizeof( uint8_t ));

    offset[1] = l;
    offset[2] = 0;
    while (true)
    {
        if (offset[0] >= offset[1])
        {
            memcpy(d + offset[2], stream, offset[1]);
            offset[0] -= offset[1];
            uint8_t *x = calloc(block, sizeof( uint8_t ));
            if (!x)
                die(_("Out of memory @ %s:%d:%s [%zu]"), __FILE__, __LINE__, __func__, block * sizeof( uint8_t ));
            memcpy(x, stream + offset[1], offset[0]);
            memset(stream, 0x00, block);
            memcpy(stream, x, offset[0]);
            free(x);
            x = NULL;
            return l;
        }

        memcpy(d + offset[2], stream, offset[0]);
        offset[2] += offset[0];
        offset[1] -= offset[0];
        offset[0] = 0;

        ssize_t e = EXIT_SUCCESS;
        if ((e = read(f, stream, block)) < 0)
            return e;
#ifndef __DEBUG__
        gcry_cipher_decrypt(c->cipher, stream, block, NULL, 0);
#endif
        offset[0] = block;
    }
}

extern int enc_sync(int64_t f, io_params_t *c)
{
    enc_write(f, NULL, 0, c);
    return 0;
}
