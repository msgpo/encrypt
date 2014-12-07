/*
 * Copyright © 2005-2014, albinoloverats ~ Software Development
 * email: webmaster@albinoloverats.net
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

#ifndef _COMMON_H_
#define _COMMON_H_

/*!
 * \file    common.h
 * \author  albinoloverats ~ Software Development
 * \date    2009-2014
 * \brief   Mostly common macros, useful when dealing with different OS's
 *
 * Various macros which help with the transition from one OS to another.
 * There were originally part of the main common library until it was
 * torn apart to reduce complexity.
 */

#define NOTSET 0 /*!< Value to use when nothing else is available */

#if !defined _WIN32 && !defined O_BINARY
    #define O_BINARY NOTSET /*!< Value is only relevant on MS systems (and is required), pretend it exists elsewhere */
#endif

#if defined __APPLE__ || defined __FreeBSD__
    #define program_invocation_short_name getprogname() /*!< This is the best/closest we have */
#endif

#if defined __APPLE__ || defined __FreeBSD__ || defined __CYGWIN__
    #undef F_RDLCK         /*!< Undefined value as it causes runtime issues */
    #define F_RDLCK NOTSET /*!< Set value to NOTSET */
    #undef F_WRLCK         /*!< Undefined value as it causes runtime issues */
    #define F_WRLCK NOTSET /*!< Set value to NOTSET */
#endif

#ifndef __bswap_64
    #define __bswap_64(x) /*!< Define ourselves a 8-byte swap macro */ \
        ((((x) & 0xff00000000000000ull) >> 56) \
       | (((x) & 0x00ff000000000000ull) >> 40) \
       | (((x) & 0x0000ff0000000000ull) >> 24) \
       | (((x) & 0x000000ff00000000ull) >> 8)  \
       | (((x) & 0x00000000ff000000ull) << 8)  \
       | (((x) & 0x0000000000ff0000ull) << 24) \
       | (((x) & 0x000000000000ff00ull) << 40) \
       | (((x) & 0x00000000000000ffull) << 56))
#endif

#if __BYTE_ORDER == __LITTLE_ENDIAN || BYTE_ORDER == LITTLE_ENDIAN || _WIN32
    #ifndef __APPLE__
        #define ntohll(x) __bswap_64(x) /*!< Do need to swap bytes from network byte order */
        #define htonll(x) __bswap_64(x) /*!< Do need to swap bytes to network byte order */
    #endif
#elif __BYTE_ORDER == __BIG_ENDIAN || BYTE_ORDER == BIG_ENDIAN
    #define ntohll(x) (x) /*!< No need to swap bytes from network byte order */
    #define htonll(x) (x) /*!< No need to swap bytes to network byte order */
#else
    #error "Unknown endianness!"
#endif

//#if !defined _WIN32 && !defined __CYGWIN__ && !defined __APPLE__
//    #define _(s) gettext(s) /*!< Allow use of _() to refer to gettext() */
//#else
    #define _(s) s /*!< Don't yet support translations on MS Windows or Apple OS X */
//#endif

#define CONCAT(A, B) CONCAT2(A, B) /*!< Function overloading argument concatenation (part 1) */
#define CONCAT2(A, B) A ## B       /*!< Function overloading argument concatenation (part 2) */

/*! Brief overview of the GNU General Public License (version 3) */
#define TEXT_LICENCE \
    "This program is free software: you can redistribute it and/or modify\n"  \
    "it under the terms of the GNU General Public License as published by\n"  \
    "the Free Software Foundation, either version 3 of the License, or\n"     \
    "(at your option) any later version.\n\n"                                 \
    "This program is distributed in the hope that it will be useful,\n"       \
    "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"        \
    "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"         \
    "GNU General Public License for more details.\n\n"                        \
    "You should have received a copy of the GNU General Public License\n"     \
    "along with this program.  If not, see <http://www.gnu.org/licenses/>.\n"

#define PERCENT 100.0f /*!< Floating point 100% */

#define THOUSAND                   1000 /*!< Integer value for 1 thousand */
#define MILLION                 1000000 /*!< Integer value for 1 million (ideal for timing related things) */
#define THOUSAND_MILLION     1000000000 /*!< Integer value for 1000 million */
#define BILLION           1000000000000 /*!< Integer value for 1 billion (ideal for timing in nanoseconds) */

#define KILOBYTE                   1024 /*!< Bytes in a kilobyte */
#define MEGABYTE                1048576 /*!< Bytes in a megabyte */
#define GIGABYTE             1073741824 /*!< Bytes in a gigabyte */
#define TERABYTE          1099511627776 /*!< Bytes in a terabyte */

typedef unsigned char byte_t; /*!< A byte is just 8 bits, no signedness or an other fancy stuff ;-) */

#endif /* _COMMON_H_ */
