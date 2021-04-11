#include <libgimp/gimp.h>

#include "box_blur.h"
#include "fill.h"

static void query(void);
static void run(const gchar* name, gint nparams, const GimpParam* param, gint* nreturn_vals, GimpParam** return_vals);

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
  //
  guint radius = 9;
  gint x;
  for (x = -radius; x < radius; x++);
  g_message("-radius = %d", x);
  //

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

  // init gegl before using its functions
  gegl_init(NULL, NULL);

  // apply blurring to drawable
  gimp_progress_init("Box blur...");
  gint32 drawable_id = param[2].data.d_drawable;
  box_blur(drawable_id);
  // fill(drawable_id, "red");

  // flush image manipulation changes to UI
  gimp_displays_flush();

  // free resources used by gegl
  gegl_exit();
}
