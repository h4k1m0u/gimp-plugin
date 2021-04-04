#include <libgimp/gimp.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <gio/gio.h>
#include <string.h>

static void query(void);
static void run(const gchar* name, gint nparams, const GimpParam* param, gint* nreturn_vals, GimpParam** return_vals);
static void box_blur(gint32 drawable_id);

// used to copy rgba pixels
typedef struct RGBA {
  guchar r;
  guchar g;
  guchar b;
  guchar a;
} RGBA;

// plugin entry points
GimpPlugInInfo PLUG_IN_INFO = {
  NULL,
  NULL,
  query, // called on each modification of the plugin
  run    // plugin's centerpiece
};

MAIN()

static void query(void) {
  // declare plugin input parameters
  static GimpParamDef args[] = {
    {
      GIMP_PDB_INT32,
      "run-mode",
      "Run mode"
    },
    {
      GIMP_PDB_IMAGE,
      "image",
      "Input image"
    },
    {
      GIMP_PDB_DRAWABLE,
      "drawable",
      "Input drawable"
    },
  };

  // install new procedure with PDB
  const gchar* name_procedure = "box_blur";
  gimp_install_procedure(
    name_procedure,             // name
    "Box blur description",     // blurp
    "Box blur help",            // help
    "Hakim Benoudjit",          // author
    "Hakim Benoudjit ©",        // copyright
    "2021",                     // date
    "Box blur",  // menu entry label
    "RGB*",                     // accepted image types
    GIMP_PLUGIN,                // procedure type
    G_N_ELEMENTS(args), 0,      // # of params taken & returned
    args, NULL                  // params & returned values
  );

  // register menu entry for plug-in procedure
  gimp_plugin_menu_register(name_procedure, "<Image>/Filters/Misc");
}

static void run(const gchar* name, gint nparams, const GimpParam* param, gint* nreturn_vals, GimpParam** return_vals) {
  static GimpParam values[1];
  values[0].type = GIMP_PDB_STATUS;
  values[0].data.d_status = GIMP_PDB_SUCCESS;

  // declare mendatory output parameter (procedure status)
  *nreturn_vals = 1;
  *return_vals = values;

  // get run_mode input parameter (interactive, from script, repeast last)
  GimpRunMode run_mode = param[0].data.d_int32;
  if (run_mode == GIMP_RUN_INTERACTIVE) {
    // message shown on Gimp's error console window
    g_message("Running interactively...");
  }

  // get opened drawable (layer?)
  // GimpDrawable* drawable = gimp_drawable_get(param[2].data.d_drawable);

  // init gegl before using its functions
  gegl_init(NULL, NULL);

  // apply blurring to drawable
  gint32 drawable_id = param[2].data.d_drawable;
  box_blur(drawable_id);

  // paint gegl buffer in blue
  /*
  GeglColor* blue = gegl_color_new("blue");
  GeglBuffer* buffer = gimp_drawable_get_buffer(drawable_id);
  gegl_buffer_set_color(buffer, NULL, blue);
  const GeglRectangle* extent = gegl_buffer_get_extent(buffer);
  gimp_drawable_update(drawable_id, extent->x, extent->y, extent->width, extent->height);
  g_object_unref(blue);
  g_object_unref(buffer);
  */

  // flush image manipulation changes to UI
  gimp_displays_flush();

  // free resources used by gegl
  gegl_exit();
}

/**
 * Box blur with a 9 x 9 kernel. Blurs particularly edges well.
 *
 * To see how GEGL buffer and shadow buffer are used:
 * https://gitlab.gnome.org/GNOME/gimp/-/blob/gimp-2-10/plug-ins/common/despeckle.c#L353
 *
 * @param drawable
 */
static void box_blur(gint32 drawable_id) {
  // gegl buffer & shadow buffer for reading/writing resp.
  GeglBuffer* buffer = gimp_drawable_get_buffer(drawable_id);
  GeglBuffer* shadow_buffer = gimp_drawable_get_shadow_buffer(drawable_id);

  // get bounds of selection (or else of entire image)
  gint x1, y1, width, height;
  gimp_drawable_mask_intersect(drawable_id, &x1, &y1, &width, &height);
  // const GeglRectangle* extent = gegl_buffer_get_extent(buffer);
  gint x2 = x1 + width;
  gint y2 = y1 + height;
  gint n_channels = gimp_drawable_bpp(drawable_id);
  gint n_bytes_row = width * n_channels;
  g_message("(x1, y1) = (%d, %d)", x1, y1);
  g_message("(width, height, n_channels) = (%d, %d, %d)", width, height, n_channels);

  // buffer for row of image pixels (each pixel has n channels)
  guchar* row_minus_1 = g_new(guchar, n_bytes_row);
  guchar* row = g_new(guchar, n_bytes_row);
  guchar* row_plus_1 = g_new(guchar, n_bytes_row);
  guchar* row_out = g_new(guchar, n_bytes_row);

  // read all image at once from input buffer
  g_message("size image: %d", n_bytes_row * height);
  guchar* image = g_new(guchar, n_bytes_row * height);
  const Babl* format = gimp_drawable_get_format(drawable_id);
  gegl_buffer_get(buffer, GEGL_RECTANGLE(x1, y1, width, height),
                  1.0, format, image, GEGL_AUTO_ROWSTRIDE, GEGL_ABYSS_NONE);

  for (gint i_row = y1; i_row < y2; ++i_row) {
    // copy from image to rows pointers (row before, row, and row after)
    memcpy(row_minus_1, &image[MAX(i_row - 1, y1) * n_bytes_row], n_bytes_row);
    memcpy(row, &image[i_row * n_bytes_row], n_bytes_row);
    memcpy(row_plus_1, &image[MIN(i_row + 1, y2 - 1) * n_bytes_row], n_bytes_row);

    for (gint i_col = x1; i_col < x2; ++i_col) {
      // update all channels for each row pixel
      gint n_neighbors = 9;
      for (gint i_channel = 0; i_channel < n_channels; ++i_channel) {
        gint i_cell = i_col*n_channels + i_channel;
        gint sum = row_minus_1[i_cell - n_channels] +
                          row_minus_1[i_cell] +
                          row_minus_1[i_cell + n_channels] +
                          row[i_cell - n_channels] +
                          row[i_cell] +
                          row[i_cell + n_channels] +
                          row_plus_1[i_cell - n_channels] +
                          row_plus_1[i_cell] +
                          row_plus_1[i_cell + n_channels];
        row_out[i_cell] = sum / n_neighbors;
      }
    }

    // copy from resulting row pointer to image
    memcpy(image + (i_row * n_bytes_row), row_out, n_bytes_row);
  }

  // write resulting image at once to shadow buffer
  gegl_buffer_set(shadow_buffer, GEGL_RECTANGLE(x1, y1, width, height),
                  0, format, image, GEGL_AUTO_ROWSTRIDE);

  // flush required by shadow buffer & merge shadow buffer with drawable & update drawable
  gegl_buffer_flush(shadow_buffer);
  gimp_drawable_merge_shadow(drawable_id, TRUE);
  gimp_drawable_update(drawable_id, x1, y1, width, height);

  // free allocated pointers & buffers
  g_free(row_minus_1);
  g_free(row);
  g_free(row_plus_1);
  g_free(row_out);
  g_free(image);
  g_object_unref(buffer);
  g_object_unref(shadow_buffer);

  g_message("Done");
}
