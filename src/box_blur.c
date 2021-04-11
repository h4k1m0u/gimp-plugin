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

  // 2D array for rows between [-radius, radius] from current row
  gint n_bytes_row = width * n_channels;
  guchar* row_out = g_new(guchar, n_bytes_row);
  gint radius = 3; // cannot use guint as unary minus has precedance over assignment (and guint cannot be negative below, so overflows)
  gint n_neighbors = (2*radius + 1) * (2*radius + 1);
  guchar** rows = g_new(guchar*, 2*radius + 1);

  for (gint i_row_offset = -radius; i_row_offset <= radius; ++i_row_offset) {
    rows[i_row_offset + radius] = g_new(guchar, n_bytes_row);
  }

  // blurring calculated on row-basis
  for (gint i_row = y1; i_row < y2; ++i_row) {
    // copy from image to 2D rows pointers (rows before, row, and rows after)
    for (gint i_row_offset = -radius; i_row_offset <= radius; ++i_row_offset) {
      gint i_row_neighbor = CLAMP(i_row + i_row_offset, y1, y2 - 1);
      gint offset = (extent->width * i_row_neighbor + x1) * n_channels;
      memcpy(rows[i_row_offset + radius], &image[offset], n_bytes_row);
    }

    // process each row pixel
    for (gint i_col = 0; i_col < width; ++i_col) {
      // neighborhood around row pixel for each channel
      for (gint i_channel = 0; i_channel < n_channels; ++i_channel) {
        gint sum = 0;

        for (gint i_row_offset = -radius; i_row_offset <= radius; ++i_row_offset) {
          for (gint i_col_offset = -radius; i_col_offset <= radius; ++i_col_offset) {
            gint i_row_neighbor = i_row_offset + radius;
            gint i_col_neighbor = CLAMP(i_col + i_col_offset, 0, width - 1);
            sum += rows[i_row_neighbor][i_col_neighbor*n_channels + i_channel];
          }
        }

        row_out[i_col*n_channels + i_channel] = sum / n_neighbors;
      }
    }

    // copy from resulting row pointer to image
    gint offset = (extent->width * i_row + x1) * n_channels;
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
  g_free(rows);
  g_free(row_out);
  g_free(image);
  g_object_unref(buffer);
  g_object_unref(shadow_buffer);

  g_message("Done");
}
