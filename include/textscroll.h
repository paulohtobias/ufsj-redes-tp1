/* Text Widget/Automatic scrolling
 *
 * This example demonstrates how to use the gravity of 
 * GtkTextMarks to keep a text view scrolled to the bottom
 * while appending text.
 */

#ifndef TEXTSCROLL_H
#define TEXTSCROLL_H

#include <gtk/gtk.h>

/* Scroll to the end of the buffer.
 */
gboolean scroll_to_end (GtkTextView *textview);

/* Scroll to the bottom of the buffer.
 */
gboolean scroll_to_bottom(GtkTextView *textview);

guint setup_scroll(GtkTextView *textview, gboolean to_end);

void remove_timeout(GtkWidget *window, gpointer timeout);

void create_text_view(GtkWidget *hbox, gboolean to_end);

GtkWidget *do_textscroll(GtkWidget *do_widget);

#endif //TEXTSCROLL_H
