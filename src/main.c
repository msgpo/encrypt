/*
 * encrypt ~ a simple, multi-OS encryption utility
 * Copyright © 2005-2017, albinoloverats ~ Software Development
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

#include <errno.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>

#include <string.h>
#include <stdbool.h>

#include <pthread.h>
#include <sys/stat.h>
#include <libgen.h>

#include "common/common.h"
#include "common/non-gnu.h"
#include "common/error.h"
#include "common/ccrypt.h"
#include "common/version.h"
#include "common/cli.h"

#ifdef _WIN32
	#include <Shlobj.h>
	extern char *program_invocation_short_name;
#endif

#include "init.h"
#include "crypt.h"
#include "encrypt.h"
#include "decrypt.h"

#ifdef BUILD_GUI
	#include "gui.h"
	#include "gui-gtk.h"
#endif


extern char *gui_file_hack_source;
extern char *gui_file_hack_output;

static bool list_ciphers(void);
static bool list_hashes(void);
static bool list_modes(void);

int main(int argc, char **argv)
{
#ifdef __DEBUG__
	fprintf(stderr, _("\n**** DEBUG BUILD ****\n\n"));
#endif

#ifdef _WIN32
	setbuf(stdout, NULL);
	setbuf(stderr, NULL);
	program_invocation_short_name = strdup(argv[0]);
#endif
	/*
	 * start background thread to check for newer version of encrypt
	 *
	 * NB If (When) encrypt makes it into a package manager for some
	 * distros this can/should be removed as it will be unnecessary
	 */
	version_check_for_update(ENCRYPT_VERSION, UPDATE_URL, DOWNLOAD_URL_TEMPLATE);

	args_t args = init(argc, argv);

	/*
	 * list available algorithms if asked to (possibly both hash and
	 * crypto)
	 */
	bool la = false;
	if (args.cipher && !strcasecmp(args.cipher, "list"))
		la = list_ciphers();
	if (args.hash && !strcasecmp(args.hash, "list"))
		la = list_hashes();
	if (args.mode && !strcasecmp(args.mode, "list"))
		la = list_modes();
	if (la)
		return EXIT_SUCCESS;

#ifndef _WIN32
	bool dude = false;
	if (!strcmp(basename(argv[0]), ALT_NAME))
		dude = true;
#endif

#ifdef BUILD_GUI
	gtk_widgets_t *widgets;
	GtkBuilder *builder;
	GError *error = NULL;

	if (args.source)
	{
		char *ptr = malloc(0);
		char *c = ptr;
		char *h = ptr;
		char *m = ptr;
		if (is_encrypted(args.source, &c, &h, &m))
		{
			free(args.cipher);
			free(args.hash);
			free(args.mode);
			args.cipher = c;
			args.hash = h;
			args.mode = m;
		}
		free(ptr);
	}
	#ifndef _WIN32
	struct stat n;
	fstat(STDIN_FILENO, &n);
	struct stat t;
	fstat(STDOUT_FILENO, &t);

	if (args.nogui)
	  ;
	else
	{
	#endif /* ! _WIN32 */

		if (gtk_init_check(&argc, &argv))
		{
			builder = gtk_builder_new();
	#ifndef _WIN32
		#if !defined __DEBUG__ && !defined __DEBUG_GUI__
			const char *glade_ui_file = GLADE_UI_FILE_DEFAULT;
		#else
			const char *glade_ui_file = GLADE_UI_FILE_BACKUP;
		#endif
	#else
			char *glade_ui_file = calloc(MAX_PATH, sizeof( char ));
			if (!glade_ui_file)
				die(_("Out of memory @ %s:%d:%s [%zu]"), __FILE__, __LINE__, __func__, MAX_PATH);
		#ifndef __DEBUG__
			SHGetFolderPath(NULL, CSIDL_PROGRAM_FILES, NULL, 0, glade_ui_file);
			strcat(glade_ui_file, "\\");
		#endif /* __DEBUG__ */
			strcat(glade_ui_file, GLADE_UI_FILE_DEFAULT);
	#endif /* ! _WIN32 */
			if (!gtk_builder_add_from_file(builder, glade_ui_file, &error))
			{
				fprintf(stderr, "%s", error->message);
				g_error_free(error);
				error = NULL;
				if (!gtk_builder_add_from_file(builder, GLADE_UI_FILE_BACKUP, &error))
					die(_("%s"), error->message);
			}
	#ifdef _WIN32
			free(glade_ui_file);
	#endif
			/*
			 * allocate widgets structure
			 */
			widgets = g_slice_new(gtk_widgets_t);
			/*
			 * get widgets from UI
			 */
			CH_GET_WIDGET(builder, main_window, widgets);
			CH_GET_WIDGET(builder, open_button, widgets);
			CH_GET_WIDGET(builder, open_dialog, widgets);
			CH_GET_WIDGET(builder, open_file_label, widgets);
			CH_GET_WIDGET(builder, open_file_image, widgets);
			CH_GET_WIDGET(builder, save_button, widgets);
			CH_GET_WIDGET(builder, save_dialog, widgets);
			CH_GET_WIDGET(builder, save_file_label, widgets);
			CH_GET_WIDGET(builder, save_file_image, widgets);
			CH_GET_WIDGET(builder, crypto_combo, widgets);
			CH_GET_WIDGET(builder, hash_combo, widgets);
			CH_GET_WIDGET(builder, mode_combo, widgets);
			CH_GET_WIDGET(builder, password_entry, widgets);
			CH_GET_WIDGET(builder, key_button, widgets);
			CH_GET_WIDGET(builder, key_dialog, widgets);
			CH_GET_WIDGET(builder, key_file_label, widgets);
			CH_GET_WIDGET(builder, key_file_image, widgets);
			CH_GET_WIDGET(builder, encrypt_button, widgets);
			CH_GET_WIDGET(builder, status_bar, widgets);
			CH_GET_WIDGET(builder, progress_dialog, widgets);
			CH_GET_WIDGET(builder, progress_bar_total, widgets);
			CH_GET_WIDGET(builder, progress_bar_current, widgets);
			CH_GET_WIDGET(builder, progress_label, widgets);
			CH_GET_WIDGET(builder, progress_cancel_button, widgets);
			CH_GET_WIDGET(builder, progress_close_button, widgets);
			CH_GET_WIDGET(builder, about_dialog, widgets);
			CH_GET_WIDGET(builder, about_new_version_label, widgets);
			CH_GET_WIDGET(builder, compress_menu_item, widgets);
			CH_GET_WIDGET(builder, follow_menu_item, widgets);
			CH_GET_WIDGET(builder, raw_menu_item, widgets);
			CH_GET_WIDGET(builder, compat_menu, widgets);
			CH_GET_WIDGET(builder, key_file_menu_item, widgets);
			CH_GET_WIDGET(builder, key_password_menu_item, widgets);
			CH_GET_WIDGET(builder, raw_encrypt_button, widgets);
			CH_GET_WIDGET(builder, raw_decrypt_button, widgets);
			CH_GET_WIDGET(builder, abort_dialog, widgets);
			CH_GET_WIDGET(builder, abort_message, widgets);

			gtk_builder_connect_signals(builder, widgets);
			g_object_unref(G_OBJECT(builder));
			gtk_widget_show(widgets->main_window);

			error_gui_init(widgets->abort_dialog, widgets->abort_message);

			if (args.source)
			{
	#ifndef _WIN32
				if (args.source[0] != '/')
				{
					char *cwd = getcwd(NULL, 0);
					asprintf(&gui_file_hack_source, "%s/%s", cwd, args.source);
					free(cwd);
				}
				else
	#endif
					gui_file_hack_source = strdup(args.source);
				gtk_file_chooser_set_filename((GtkFileChooser *)widgets->open_dialog, gui_file_hack_source);
			}
			if (args.output)
			{
	#ifndef _WIN32
				if (args.output[0] != '/')
				{
					char *cwd = getcwd(NULL, 0);
					asprintf(&gui_file_hack_output, "%s/%s", cwd, args.output);
					free(cwd);
				}
				else
	#endif
					gui_file_hack_output = strdup(args.output);
				gtk_file_chooser_set_filename((GtkFileChooser *)widgets->save_dialog, gui_file_hack_output);
			}
			file_dialog_okay(NULL, widgets);

			auto_select_algorithms(widgets, args.cipher, args.hash, args.mode);
			set_compatibility_menu(widgets, args.version);
			set_key_source_menu(widgets, args.key_source);

			gtk_check_menu_item_set_active((GtkCheckMenuItem *)widgets->compress_menu_item, args.compress);
			gtk_check_menu_item_set_active((GtkCheckMenuItem *)widgets->follow_menu_item, args.follow);
			gtk_check_menu_item_set_active((GtkCheckMenuItem *)widgets->raw_menu_item, args.raw);

			set_raw_buttons(widgets, args.raw);
			set_status_bar((GtkStatusbar *)widgets->status_bar, STATUS_BAR_READY);

			gtk_main();

			g_slice_free(gtk_widgets_t, widgets);

			goto clean_up;

		}
		else
			fprintf(stderr, _("Could not create GUI - falling back to command line\n"));
	#ifndef _WIN32
	}
	#endif
#endif /* we couldn’t create the gui, so revert back to command line */

#ifndef _WIN32 /* it’s GUI or nothing for Windows */
	/*
	 * get raw key data in form of password/phrase, key file
	 */
	uint8_t *key = NULL;
	size_t length = 0;
	if (args.key)
	{
		key = (uint8_t *)args.key;
		length = 0;
	}
	else if (args.password)
	{
		key = (uint8_t *)args.password;
		length = strlen(args.password);
	}
	else if (isatty(STDIN_FILENO))
	{
		key = (uint8_t *)getpass(_("Please enter a password: "));
		length = strlen((char *)key);
		printf("\n");
	}
	else
		show_usage();
	/*
	 * here we go ...
	 */
	crypto_t *c;

	if (dude || (args.source && is_encrypted(args.source)))
		c = decrypt_init(args.source, args.output, args.cipher, args.hash, args.mode, key, length, args.raw);
	else
		c = encrypt_init(args.source, args.output, args.cipher, args.hash, args.mode, key, length, args.raw, args.compress, args.follow, parse_version(args.version));

	init_deinit(args);

	if (c->status == STATUS_INIT)
	{
		execute(c);
		/*
		 * only display the UI if not outputting to stdout (and if stderr
		 * is a terminal)
		 */
		struct stat t;
		fstat(STDOUT_FILENO, &t);

		bool ui = isatty(STDERR_FILENO) && (!io_is_stdout(c->output) || c->path || S_ISREG(t.st_mode));
		if (ui)
		{
			cli_t p = { (cli_status_e *)&c->status, &c->current, &c->total };
			cli_display(&p);
		}
		else
			while (c->status == STATUS_INIT || c->status == STATUS_RUNNING)
				sleep(1);
	}

	if (c->status != STATUS_SUCCESS)
		fprintf(stderr, _("%s\n"), status(c));

	deinit(&c);

#endif /* ! _WIN32 */

#ifdef BUILD_GUI
clean_up:
#endif

	if (new_version_available)
		fprintf(stderr, _(NEW_VERSION_URL), version_available, program_invocation_short_name, strlen(new_version_url) ? new_version_url : PROJECT_URL);

#ifdef __DEBUG__
	fprintf(stderr, _("\n**** DEBUG BUILD ****\n\n"));
#endif

	return EXIT_SUCCESS;
}

static bool list_ciphers(void)
{
	const char **l = list_of_ciphers();
	for (int i = 0; l[i] ; i++)
		fprintf(stderr, "%s\n", l[i]);
	return true;
}

static bool list_hashes(void)
{
	const char **l = list_of_hashes();
	for (int i = 0; l[i]; i++)
		fprintf(stderr, "%s\n", l[i]);
	return true;
}

static bool list_modes(void)
{
	const char **l = list_of_modes();
	for (int i = 0; l[i]; i++)
		fprintf(stderr, "%s\n", l[i]);
	return true;
}
