#include <libgimp/gimp.h>

#include "box_blur.h"

/**
 * Box blur with a 9 x 9 kernel. Blurs particularly edges well.
 *
 * To see how GEGL buffer and shadow buffer are used:
 * https://gitlab.gnome.org/GNOME/gimp/-/blob/gimp-2-10/plug-ins/common/despeckle.c#L353
 *
 * @param drawable
 */
void box_blur(gint32 drawable_id) {
  // gegl buffer & shadow buffer for reading/writing resp.
  GeglBuffer* buffer = gimp_drawable_get_buffer(drawable_id);
  GeglBuffer* shadow_buffer = gimp_drawable_get_shadow_buffer(drawable_id);

  // read all image at once from input buffer
  const GeglRectangle* extent = gegl_buffer_get_extent(buffer);
  const Babl* format = gimp_drawable_get_format(drawable_id);
  gint n_channels = gimp_drawable_bpp(drawable_id);
  guchar* image = g_new(guchar, extent->width * extent->height * n_channels);
  gegl_buffer_get(buffer, GEGL_RECTANGLE(extent->x, extent->y, extent->width, extent->height),
                  1.0, format, image, GEGL_AUTO_ROWSTRIDE, GEGL_ABYSS_NONE);
  g_message("Image(width, height, n_channels) = (%d, %d, %d)", extent->width, extent->height, n_channels);

  // get bounds of selection (or else of entire image)
  gint x1, y1, width, height;
  gimp_drawable_mask_intersect(drawable_id, &x1, &y1, &width, &height);
  gint x2 = x1 + width;
  gint y2 = y1 + height;
  g_message("Selection(x1, y1) = (%d, %d)", x1, y1);
  g_message("Selection(width, height, n_channels) = (%d, %d, %d)", width, height, n_channels);

  // buffer for row of image pixels (each pixel has n channels)
  gint n_bytes_row = width * n_channels;
  guchar* row_minus_1 = g_new(guchar, n_bytes_row);
  guchar* row = g_new(guchar, n_bytes_row);
  guchar* row_plus_1 = g_new(guchar, n_bytes_row);
  guchar* row_out = g_new(guchar, n_bytes_row);

  for (gint i_row = y1; i_row < y2; ++i_row) {
    // copy from image to rows pointers (row before, row, and row after)
    gint offset_minus_1 = (extent->width * MAX(i_row - 1, y1) + x1) * n_channels;
    gint offset = (extent->width * i_row + x1) * n_channels;
    gint offset_plus_1 = (extent->width * MIN(i_row + 1, y2 - 1) + x1) * n_channels;
    memcpy(row_minus_1, &image[offset_minus_1], n_bytes_row);
    memcpy(row, &image[offset], n_bytes_row);
    memcpy(row_plus_1, &image[offset_plus_1], n_bytes_row);

    for (gint i_col = 0; i_col < width; ++i_col) {
      // update all channels for each row pixel
      gint n_neighbors = 9;
      for (gint i_channel = 0; i_channel < n_channels; ++i_channel) {
        gint i_cell = i_col*n_channels + i_channel;
        gint i_cell_minus_1 = MAX(i_cell - n_channels, i_channel);
        gint i_cell_plus_1 = MIN(i_cell + n_channels, n_bytes_row - n_channels + i_channel);
        gint sum = row_minus_1[i_cell_minus_1] +
                   row_minus_1[i_cell] +
                   row_minus_1[i_cell_plus_1] +
                   row[i_cell_minus_1] +
                   row[i_cell] +
                   row[i_cell_plus_1] +
                   row_plus_1[i_cell_minus_1] +
                   row_plus_1[i_cell] +
                   row_plus_1[i_cell_plus_1];
        row_out[i_cell] = sum / n_neighbors;
      }
    }

    // copy from resulting row pointer to image
    memcpy(&image[offset], row_out, n_bytes_row);

    // update progress bar using equation of line passing through (x, y) = (y1, 0%) and (y2, 100%)
    if (i_row % 100 == 0) {
      gdouble percentage = (gdouble) (i_row - y1) / (y2 - y1);
      gimp_progress_update(percentage);
      // g_usleep(0.1 * G_USEC_PER_SEC);
    }
  }

  // write resulting image at once to shadow buffer
  gegl_buffer_set(shadow_buffer, GEGL_RECTANGLE(extent->x, extent->y, extent->width, extent->height),
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
