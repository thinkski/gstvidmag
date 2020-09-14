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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gstvidmag.h"

static GstStaticPadTemplate vidmag_src_factory =
GST_STATIC_PAD_TEMPLATE ("src",
	GST_PAD_SRC,
	GST_PAD_ALWAYS,
	GST_STATIC_CAPS (GST_VIDEO_CAPS_MAKE ("{ I420, YV12 }"))
	);

static GstStaticPadTemplate vidmag_sink_factory =
GST_STATIC_PAD_TEMPLATE ("sink",
	GST_PAD_SINK,
	GST_PAD_ALWAYS,
	GST_STATIC_CAPS (GST_VIDEO_CAPS_MAKE ("{ I420, YV12 }"))
	);

enum {
	/* FILL ME */
	LAST_SIGNAL
};

// filter properties
enum {
	PROP_0,
	PROP_BPF_LOWER,
	PROP_BPF_UPPER,
	PROP_CUTOFF,
	PROP_GAIN
};

#define gst_vidmag_parent_class parent_class
G_DEFINE_TYPE (GstVidMag, gst_vidmag, GST_TYPE_VIDEO_FILTER);

static GstFlowReturn gst_vidmag_transform_frame (GstVideoFilter * filter,
	GstVideoFrame * in_frame, GstVideoFrame * out_frame);

static void gst_vidmag_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);
static void gst_vidmag_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);

static void
gst_vidmag_class_init (GstVidMagClass * klass)
{
	GObjectClass *gobject_class;
	GstElementClass *gstelement_class;
	GstVideoFilterClass *vfilter_class;

	gobject_class = (GObjectClass *) klass;
	gstelement_class = (GstElementClass *) klass;
	vfilter_class = (GstVideoFilterClass *) klass;

	gobject_class->set_property = gst_vidmag_set_property;
	gobject_class->get_property = gst_vidmag_get_property;

	g_object_class_install_property (G_OBJECT_CLASS (klass), PROP_BPF_LOWER,
		g_param_spec_float ("wl", "Lower bound",
			"Band-pass filter lower bound (Hz)", 0.0, 100.0, 0.4,
			G_PARAM_READWRITE));

	g_object_class_install_property (G_OBJECT_CLASS (klass), PROP_BPF_UPPER,
		g_param_spec_float ("wh", "Upper bound",
			"Band-pass filter upper bound (Hz)", 1.0, 120.0, 3.0,
			G_PARAM_READWRITE));

	g_object_class_install_property (G_OBJECT_CLASS (klass), PROP_CUTOFF,
		g_param_spec_float ("cutoff", "Cutoff", "Spatial cutoff wavelength",
			0.0, 1000.0, 16.0, G_PARAM_READWRITE));

	g_object_class_install_property (G_OBJECT_CLASS (klass), PROP_GAIN,
		g_param_spec_float ("gain", "Gain", "Amplification factor",
			0.0, 150.0, 10.0, G_PARAM_READWRITE));

	gst_element_class_set_static_metadata (gstelement_class,
		"Video magnification", "Filter/Effect/Video",
		"Magnifies small color or motion temporal variations",
		"Chris Hiszpanski <chris@hiszpanski.name>");

	gst_element_class_add_static_pad_template (gstelement_class,
		&vidmag_src_factory);
	gst_element_class_add_static_pad_template (gstelement_class,
		&vidmag_sink_factory);

	vfilter_class->transform_frame =
		GST_DEBUG_FUNCPTR (gst_vidmag_transform_frame);
}

static void gst_vidmag_init(GstVidMag * filter) {
	filter->cutoff = 16.0;
	filter->gain = 10.0;
	filter->wl = 0.4;
	filter->wh = 3.0;
}

static void
gst_vidmag_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
	GstVidMag *filter;

	filter = GST_VIDMAG (object);

	switch (prop_id) {
		case PROP_CUTOFF:
			filter->cutoff = g_value_get_float (value);
			break;
		case PROP_GAIN:
			filter->gain = g_value_get_float (value);
			break;
		case PROP_BPF_LOWER:
			filter->wl = g_value_get_float (value);
			break;
		case PROP_BPF_UPPER:
			filter->wh = g_value_get_float (value);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
gst_vidmag_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
	GstVidMag *filter;

	filter = GST_VIDMAG (object);

	switch (prop_id) {
		case PROP_BPF_LOWER:
			g_value_set_float (value, filter->wl);
			break;
		case PROP_BPF_UPPER:
			g_value_set_float (value, filter->wh);
			break;
		case PROP_CUTOFF:
			g_value_set_float (value, filter->cutoff);
			break;
		case PROP_GAIN:
			g_value_set_float (value, filter->gain);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static GstFlowReturn gst_vidmag_transform_frame(
	GstVideoFilter *filter,
	GstVideoFrame *in_frame,
	GstVideoFrame *out_frame
) {
	GstVidMag *vidmag;

	vidmag = GST_VIDMAG (filter);

	g_print("%" G_GINT32_FORMAT " x %" G_GINT32_FORMAT "\n",
		GST_VIDEO_FRAME_WIDTH(in_frame),
		GST_VIDEO_FRAME_HEIGHT(in_frame));

	gst_video_frame_copy_plane (out_frame, in_frame, 0);
	gst_video_frame_copy_plane (out_frame, in_frame, 1);
	gst_video_frame_copy_plane (out_frame, in_frame, 2);

	return GST_FLOW_OK;
}

static gboolean
plugin_init (GstPlugin * plugin)
{
	return gst_element_register (plugin, "vidmag", GST_RANK_NONE,
		GST_TYPE_VIDMAG);
}

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    vidmag,
    "Video magnification plugin",
    plugin_init, VERSION, GST_LICENSE, GST_PACKAGE_NAME, GST_PACKAGE_ORIGIN);
