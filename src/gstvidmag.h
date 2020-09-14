/* GStreamer
 * Copyright (C) 2020 Chris Hiszpanski <chris@hiszpanski.name>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef __GST_VIDMAG_H__
#define __GST_VIDMAG_H__

#include <gst/gst.h>
#include <gst/video/video.h>
#include <gst/video/gstvideofilter.h>

G_BEGIN_DECLS

#define GST_TYPE_VIDMAG \
  (gst_vidmag_get_type())
#define GST_VIDMAG(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_VIDMAG,GstVidMag))
#define GST_VIDMAG_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_VIDMAG,GstVidMagClass))
#define GST_IS_VIDMAG(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_VIDMAG))
#define GST_IS_VIDMAG_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_VIDMAG))

typedef struct _GstVidMag GstVidMag;
typedef struct _GstVidMagClass GstVidMagClass;

struct _GstVidMag {
	GstVideoFilter parent;

	// spatial cutoff wavelength
	gfloat cutoff;

	// band-pass filter lower and upper bounds
	gfloat wl, wh;

	// amplification factor
	gfloat gain;
};

struct _GstVidMagClass {
	GstVideoFilterClass parent_class;
};

GType gst_vidmag_get_type (void);

G_END_DECLS

#endif /* __GST_VIDMAG_H__ */
