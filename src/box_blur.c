#include <libgimp/gimp.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <gio/gio.h>

static void query(void);
static void run(const gchar* name, gint nparams, const GimpParam* param, gint* nreturn_vals, GimpParam** return_vals);
static void box_blur(GimpDrawable* drawable);

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
  GimpDrawable* drawable = gimp_drawable_get(param[2].data.d_drawable);

  // apply blurring to drawable
  box_blur(drawable);

  // flush image manipulation changes to UI & free drawable
  gimp_displays_flush();
  gimp_drawable_detach(drawable);
}

static void box_blur(GimpDrawable* drawable) {
  // https://en.wikipedia.org/wiki/Box_blur
  // get bounds of selection (or else of entire image)
  gint x1, y1, x2, y2;
  gimp_drawable_mask_bounds(drawable->drawable_id, &x1, &y1, &x2, &y2);
  gint width = x2 - x1;
  gint height = y2 - y1;
  g_message("width = %d, height = %d", width, height);

  // init regions for reading/writing from drawable
  GimpPixelRgn region_read, region_write;
  gimp_pixel_rgn_init(&region_read, drawable, x1, y1, width, height, FALSE, FALSE);
  gimp_pixel_rgn_init(&region_write, drawable, x1, y1, width, height, TRUE, TRUE);

  // calcualte average in neighborhood along each channel
  gint n_channels = gimp_drawable_bpp(drawable->drawable_id);
  g_message("n_channels = %d", n_channels);

  // buffer for row of image pixels (each pixel has n channels)
  guchar* row_minus_1 = g_new(guchar, n_channels * width);
  guchar* row = g_new(guchar, n_channels * width);
  guchar* row_plus_1 = g_new(guchar, n_channels * width);
  guchar* row_out = g_new(guchar, n_channels * width);

  for (gint i_row = y1; i_row < y2; ++i_row) {
    // read three rows of pixels covering neighborhood (row before, row, row after)
    gimp_pixel_rgn_get_row(&region_read, row_minus_1, x1, MAX(i_row - 1, y1), width);
    gimp_pixel_rgn_get_row(&region_read, row, x1, i_row, width);
    gimp_pixel_rgn_get_row(&region_read, row_plus_1, x1, MIN(i_row + 1, y2 - 1), width);

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

    // write entire row all at once
    gimp_pixel_rgn_set_row(&region_write, row_out, 0, i_row, width);
  }

  // free allocated pointers
  g_free(row_minus_1);
  g_free(row);
  g_free(row_plus_1);
  g_free(row_out);

  // merge shadow buffer with drawable & update drawable region & transfer to core (ie gimp)
  gimp_drawable_merge_shadow(drawable->drawable_id, TRUE);
  gimp_drawable_update(drawable->drawable_id, x1, y1, width, height);
  gimp_drawable_flush(drawable);
  g_message("Done");
}
