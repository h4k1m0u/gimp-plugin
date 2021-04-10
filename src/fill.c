#include <libgimp/gimp.h>

#include "fill.h"

/**
 * Fill drawable with given color
 *
 * @param drawable
 * @param color_name  Color name or hex
 */
void fill(gint32 drawable_id, const gchar* color_name) {
  // gegl shadow buffer for writing on image
  GeglBuffer* shadow_buffer = gimp_drawable_get_shadow_buffer(drawable_id);

  // paint gegl shadow buffer in given color
  GeglColor* color = gegl_color_new(color_name);
  gegl_buffer_set_color(shadow_buffer, NULL, color);
  g_object_unref(color);

  // flush required by shadow buffer & merge shadow buffer with drawable & update drawable
  gegl_buffer_flush(shadow_buffer);
  gimp_drawable_merge_shadow(drawable_id, TRUE);
  const GeglRectangle* extent = gegl_buffer_get_extent(shadow_buffer);
  gimp_drawable_update(drawable_id, extent->x, extent->y, extent->width, extent->height);

  // free buffers
  g_object_unref(shadow_buffer);
}
