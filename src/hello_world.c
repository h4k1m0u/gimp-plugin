#include <libgimp/gimp.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <gio/gio.h>

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
}
