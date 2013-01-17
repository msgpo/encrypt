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

#ifndef _ENCRYPT_INIT_H_
#define _ENCRYPT_INIT_H_

#define APP_NAME "encrypt"
#define ALT_NAME "decrypt"

#define APP_USAGE "[-c algorithm] [-s algorithm] [-k key/-p password] [-x] [source] [destination]"
#define ALT_USAGE "[-k key/-p password] [input] [output]"

#ifndef _WIN32
    #define ENCRYPTRC ".encryptrc"
#else
    #define ENCRYPTRC "_encryptrc"
#endif
#define CONF_COMPRESS "compress"
#define CONF_CIPHER "cipher"
#define CONF_HASH "hash"

#define CONF_TRUE     "true"
#define CONF_ON       "on"
#define CONF_ENABLED  "enabled"
#define CONF_FALSE    "false"
#define CONF_OFF      "off"
#define CONF_DISABLED "disabled"

/*!
 * \brief  Structure of expected options
 *
 * Structure returned from init() with values for any expected
 * options.
 */
typedef struct args_t
{
    char *cipher;    /*!< The cryptoraphic cipher selected by the user */
    char *hash;      /*!< The hash function selected by the user */
    char *key;       /*!< The key file for key generation */
    char *password;  /*!< The password for key generation */
    char *source;    /*!< The input file/stream */
    char *output;    /*!< The output file/stream */
    bool compress:1; /*!< Compress the file (with xz) before encrypting */
}
args_t;

/*!
 * \brief           Application init function
 * \param[in]  argc Number of command line arguments
 * \param[out] argv Command line arguments
 * \return          Any command line options that were set
 *
 * Provide simple command line argument parsing, and pass back whatever
 * options where set. Removes a lot of the cruft from the legacy common
 * code that used to exist here.
 */
extern args_t init(int argc, char **argv);

extern void init_deinit(args_t args);

/*!
 * \brief         Update configuration file
 * \param[in]  o  Option to update
 * \param[out] v  New value
 *
 * Set or update the given configuration option with the given value.
 */
extern void update_config(char *o, char *v);

/*!
 * \brief         Show list of command line options
 *
 * Show list of command line options, and ways to invoke the application.
 * Usually when --help is given as a command line argument.
 */
extern void show_help(void) __attribute__((noreturn));

/*!
  * \brief        Show brief GPL licence text
  *
  * Display a brief overview of the GNU GPL v3 licence, such as when the
  * command line argument is --licence.
  */
extern void show_licence(void) __attribute__((noreturn));

/*!
 * \brief         Show simple usage instructions
 *
 * Display simple application usage instruction.
 */
extern void show_usage(void) __attribute__((noreturn));

/*!
 * \brief         Show application version
 *
 * Display the version of the application.
 */
extern void show_version(void) __attribute__((noreturn));

#endif /* ! _INIT_H_ */
