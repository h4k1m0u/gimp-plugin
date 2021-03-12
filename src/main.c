// #include <libgimp/gimp.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <gio/gio.h>

int main(int argc, char** argv) {
  // double-linked list with glib
  g_autoptr(GList) list = NULL;
  list = g_list_append(list, "item");
  g_printf("List value: %s\n\n", (char *) list->data);

  // open file with gio (g_autoptr automatically clears pointers)
  g_autoptr(GFile) file = g_file_new_for_path("./assets/file.txt");
  g_autoptr(GFileInputStream) stream = g_file_read(file, NULL, NULL);
  gchar buffer[100];

  // glib strings
  g_autoptr(GString) s = g_string_new("glib string");
  g_printf("Strings\n");
  g_printf("\"%s\"\n\n", s->str);

  // read file content
  while (TRUE) {
    gssize n_bytes_read = g_input_stream_read(G_INPUT_STREAM(stream), buffer, G_N_ELEMENTS(buffer) - 1, NULL, NULL);

    if (n_bytes_read > 0) {
      // strings must null-terminated in C (buffer isn't)
      buffer[n_bytes_read] = '\0';
      g_printf("Content:\n%s", buffer);
    } else {
      break;
    }
  }

  return 0;
}
