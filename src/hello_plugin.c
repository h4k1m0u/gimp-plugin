#include <libgimp/gimp.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <gio/gio.h>

static void query(void);
static void run(const gchar* name, gint nparams, const GimpParam* param, gint* nreturn_vals, GimpParam** return_vals);
static void blur(GimpDrawable* drawable);

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
  gimp_install_procedure(
    "plugin-hello",
    "Plugin hello description",
    "Plugin hello help",
    "Hakim Benoudjit",
    "Hakim Benoudjit ©",
    "2021",
    "Plugin hello menu entry",
    "RGB*",
    GIMP_PLUGIN,
    G_N_ELEMENTS(args), 0,
    args, NULL
  );

  // register menu entry for plug-in procedure
  gimp_plugin_menu_register("plugin-hello", "<Image>/Filters/Misc");
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
    g_message("Hello world from Gimp plugin");
  }

  // get opened drawable (layer?)
  GimpDrawable* drawable = gimp_drawable_get(param[2].data.d_drawable);

  // apply blurring to drawable
  blur(drawable);

  // flush image manipulation changes to UI & free drawable
  gimp_displays_flush();
  gimp_drawable_detach(drawable);
}

static void blur(GimpDrawable* drawable) {
  // get bounds of selection (or else of entire image)
  gint x1, y1, x2, y2;
  gimp_drawable_mask_bounds(drawable->drawable_id, &x1, &y1, &x2, &y2);
  g_message("width = %d, height = %d", x2 - x1, y2 - y1);

  // init regions for reading/writing from drawable
  GimpPixelRgn region_read, region_write;
  gimp_pixel_rgn_init(&region_read, drawable, x1, y1, x2, y2, FALSE, FALSE);
  gimp_pixel_rgn_init(&region_write, drawable, x1, y1, x2, y2, TRUE, TRUE);

  // calcualte average in neighborhood along each channel
  gint n_channels = gimp_drawable_bpp(drawable->drawable_id);
  g_message("n_channels = %d", n_channels);

  // loop through all pixels in region
  for (size_t i_col = x1; i_col < x2; ++i_col) {
    for (size_t i_row = y1; i_row < y2; ++i_row) {
      // read nine pixels in neighborhood in rgba format (four bytes)
      gint n_neighbors = 9;
      guchar pixels[n_neighbors][n_channels];
      gimp_pixel_rgn_get_pixel(&region_read, pixels[0], i_col - 1, i_row - 1);
      gimp_pixel_rgn_get_pixel(&region_read, pixels[1], i_col,     i_row - 1);
      gimp_pixel_rgn_get_pixel(&region_read, pixels[2], i_col + 1, i_row - 1);
      gimp_pixel_rgn_get_pixel(&region_read, pixels[3], i_col - 1, i_row);
      gimp_pixel_rgn_get_pixel(&region_read, pixels[4], i_col,     i_row);
      gimp_pixel_rgn_get_pixel(&region_read, pixels[5], i_col + 1, i_row);
      gimp_pixel_rgn_get_pixel(&region_read, pixels[6], i_col - 1, i_row + 1);
      gimp_pixel_rgn_get_pixel(&region_read, pixels[7], i_col,     i_row + 1);
      gimp_pixel_rgn_get_pixel(&region_read, pixels[8], i_col + 1, i_row + 1);

      guchar output[n_channels];
      for (size_t i_channel = 0; i_channel < n_channels; ++i_channel) {
        gint sum = 0;
        for (size_t i_neighbor = 0; i_neighbor < n_neighbors; ++i_neighbor) {
          sum += pixels[i_neighbor][i_channel];
        }

        output[i_channel] = sum / n_neighbors;
      }

      // set pixel in center as average of neighboring pixels
      gimp_pixel_rgn_set_pixel(&region_write, output, i_col, i_row);
    }
  }

  // merge shadow buffer with drawable & update drawable region & transfer to core (ie gimp)
  gimp_drawable_merge_shadow(drawable->drawable_id, TRUE);
  gimp_drawable_update(drawable->drawable_id, x1, y1, x2, y2);
  gimp_drawable_flush(drawable);
}
