#include <gtk/gtk.h>

static void on_click(GtkButton* button, gpointer window) {
  // destroy window & quit application
  g_message("Button was clicked");
  gtk_widget_destroy(window);
  gtk_main_quit();
}

// official gtk tutorial: https://developer.gnome.org/gtk-tutorial/stable/book1.html
int main(int argc, char** argv) {
  // initialize gtk & looks for passed arguments
  gtk_init(&argc, &argv);

  // show main window
  GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

  // button
  GtkWidget* button = gtk_button_new_with_label("Click me");
  gtk_container_add(GTK_CONTAINER(window), button);

  // button click event handlers
  g_signal_connect(button, "clicked", G_CALLBACK(on_click), window);

  // display widgets & window
  gtk_widget_show(button);
  gtk_widget_show(window);

  // main loop
  gtk_main();

  return 0;
}
