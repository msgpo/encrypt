/*
 * encrypt ~ a simple, modular, (multi-OS,) encryption utility
 * Copyright (c) 2005-2008, albinoloverats ~ Software Development
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

#ifndef _CALLBACKS_H_
  #define _CALLBACKS_H_

  #include <gtk/gtk.h>

void on_button_about_clicked(GtkWidget *);
void on_button_do_clicked(GtkWidget *);
void on_button_about_close_clicked(GtkWidget *);
void on_button_wait_close_clicked(GtkWidget *);
void on_button_generate_clicked(void);
void on_button_gen_go_clicked(GtkWidget *);
void on_button_gen_close_clicked(GtkWidget *);
void on_entry_gen_save_name_changed(GtkWidget *);

#endif /* _CALLBACKS_H_ */
