/*
 * Copyright (C) 2020 Chris Hiszpanski <chris@hiszpanski.name>
 */

/**
 * SECTION:element-evmfilter
 *
 * FIXME:Describe evmfilter here.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v -m fakesrc ! evmfilter ! fakesink silent=TRUE
 * ]|
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gst/gst.h>

#include "gstevmfilter.h"

GST_DEBUG_CATEGORY_STATIC(gst_evmfilter_debug);
#define GST_CAT_DEFAULT gst_evmfilter_debug

// filter signals
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

/* the capabilities of the inputs and outputs.
 *
 * describe the real formats here.
 */

static GstStaticPadTemplate sink_factory = GST_STATIC_PAD_TEMPLATE(
	"sink",
	GST_PAD_SINK,
	GST_PAD_ALWAYS,
	GST_STATIC_CAPS("ANY")
);

static GstStaticPadTemplate src_factory = GST_STATIC_PAD_TEMPLATE(
	"src",
	GST_PAD_SRC,
	GST_PAD_ALWAYS,
	GST_STATIC_CAPS("ANY")
);

#define gst_evmfilter_parent_class parent_class
G_DEFINE_TYPE(GstEVMFilter, gst_evmfilter, GST_TYPE_ELEMENT);


///////////////////////////  FORWARD DECLARATIONS  ///////////////////////////

static void gst_evmfilter_set_property(
	GObject *object,
	guint prop_id,
    const GValue *value,
    GParamSpec *pspec
);

static void gst_evmfilter_get_property(
	GObject *object,
	guint prop_id,
    GValue *value,
    GParamSpec *pspec
);

static gboolean gst_evmfilter_sink_event(
	GstPad *pad,
	GstObject *parent,
	GstEvent *event
);

static GstFlowReturn gst_evmfilter_chain(
	GstPad *pad,
	GstObject *parent,
	GstBuffer *buf
);


/* initialize the evmfilter's class */
static void gst_evmfilter_class_init(GstEVMFilterClass *klass) {
	GObjectClass *gobject_class;
	GstElementClass *gstelement_class;

	gobject_class = (GObjectClass *)klass;
	gstelement_class = (GstElementClass *)klass;

	gobject_class->set_property = gst_evmfilter_set_property;
	gobject_class->get_property = gst_evmfilter_get_property;

	g_object_class_install_property(
		gobject_class,
		PROP_BPF_LOWER,
		g_param_spec_float(
			"wl",
			"Lower bound",
			"Band-pass filter lower bound (Hz)",
			0.0,
			100.0,
			0.4,
			G_PARAM_READWRITE
		)
	);

	g_object_class_install_property(
		gobject_class,
		PROP_BPF_UPPER,
		g_param_spec_float(
			"wh",
			"Upper bound",
			"Band-pass filter upper bound (Hz)",
			1.0,
			120.0,
			3.0,
			G_PARAM_READWRITE
		)
	);

	g_object_class_install_property(
		gobject_class,
		PROP_CUTOFF,
		g_param_spec_float(
			"cutoff",
			"Cutoff",
			"Spatial cutoff wavelength",
			0.0,
			1000.0,
			16.0,
			G_PARAM_READWRITE
		)
	);

	g_object_class_install_property(
		gobject_class,
		PROP_GAIN,
		g_param_spec_float(
			"gain",
			"Gain",
			"Amplification factor",
			0.0,
			150.0,
			10.0,
			G_PARAM_READWRITE
		)
	);

	gst_element_class_set_details_simple(gstelement_class,
		"Eulerian video magnification filter",
		"Filter/Video",
		"Amplifies small color or motion changes",
		"Chris Hiszpanski <chris@hiszpanski.name>"
	);

	gst_element_class_add_pad_template(
		gstelement_class,
		gst_static_pad_template_get(&src_factory)
	);
	gst_element_class_add_pad_template(
		gstelement_class,
		gst_static_pad_template_get(&sink_factory)
	);
}

/* initialize the new element
 * instantiate pads and add them to element
 * set pad calback functions
 * initialize instance structure
 */
static void gst_evmfilter_init(GstEVMFilter *filter) {
	filter->sinkpad = gst_pad_new_from_static_template(
		&sink_factory,
		"sink"
	);
	gst_pad_set_event_function(
		filter->sinkpad,
		GST_DEBUG_FUNCPTR(gst_evmfilter_sink_event)
	);
	gst_pad_set_chain_function(
		filter->sinkpad,
		GST_DEBUG_FUNCPTR(gst_evmfilter_chain)
	);
	GST_PAD_SET_PROXY_CAPS(filter->sinkpad);
	gst_element_add_pad(GST_ELEMENT(filter), filter->sinkpad);

	filter->srcpad = gst_pad_new_from_static_template(&src_factory, "src");
	GST_PAD_SET_PROXY_CAPS(filter->srcpad);
	gst_element_add_pad(GST_ELEMENT(filter), filter->srcpad);

	filter->cutoff = 16.0;
	filter->gain = 10.0;
	filter->wl = 0.4;
	filter->wh = 3.0;
}

static void gst_evmfilter_set_property(
	GObject *object,
	guint prop_id,
    const GValue *value,
    GParamSpec *pspec
) {
	GstEVMFilter *filter = GST_EVMFILTER(object);

	switch (prop_id) {
		case PROP_CUTOFF:
			filter->cutoff = g_value_get_float(value);
			break;
		case PROP_GAIN:
			filter->gain = g_value_get_float(value);
			break;
		case PROP_BPF_LOWER:
			filter->wl = g_value_get_float(value);
			break;
		case PROP_BPF_UPPER:
			filter->wh = g_value_get_float(value);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
			break;
	}
}

static void gst_evmfilter_get_property(
	GObject *object,
	guint prop_id,
    GValue *value,
    GParamSpec *pspec
) {
	GstEVMFilter *filter = GST_EVMFILTER(object);

	switch (prop_id) {
		case PROP_BPF_LOWER:
			g_value_set_float(value, filter->wl);
			break;
		case PROP_BPF_UPPER:
			g_value_set_float(value, filter->wh);
			break;
		case PROP_CUTOFF:
			g_value_set_float(value, filter->cutoff);
			break;
		case PROP_GAIN:
			g_value_set_float(value, filter->gain);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
			break;
	}
}

static gboolean gst_evmfilter_sink_event(
	GstPad *pad,
	GstObject *parent,
	GstEvent *event
) {
	GstEVMFilter *filter;
	gboolean ret;

	filter = GST_EVMFILTER(parent);

	GST_LOG_OBJECT(
		filter,
		"Received %s event: %" GST_PTR_FORMAT,
		GST_EVENT_TYPE_NAME(event),
		event
	);

	switch (GST_EVENT_TYPE(event)) {
		case GST_EVENT_CAPS:
		{
			GstCaps * caps;

			gst_event_parse_caps(event, &caps);
			/* do something with the caps */

			/* and forward */
			ret = gst_pad_event_default(pad, parent, event);
			break;
		}
		default:
			ret = gst_pad_event_default(pad, parent, event);
			break;
	}

	return ret;
}

// chain function: this function does the actual processing

static GstFlowReturn gst_evmfilter_chain(
	GstPad *pad,
	GstObject *parent,
	GstBuffer *buf
) {
	GstEVMFilter *filter;

	filter = GST_EVMFILTER(parent);

	g_print("%" G_GSIZE_FORMAT "\n", gst_buffer_get_size(buf));

	/* just push out the incoming buffer without touching it */
	return gst_pad_push(filter->srcpad, buf);
}

// entry point to initialize the plug-in
static gboolean evmfilter_init(GstPlugin *evmfilter) {
	GST_DEBUG_CATEGORY_INIT(
		gst_evmfilter_debug,
		"evmfilter",
		0,
		"Amplifies small color or motion changes"
	);

	return gst_element_register(
		evmfilter,
		"evmfilter",
		GST_RANK_NONE,
		GST_TYPE_EVMFILTER
	);
}

#ifndef PACKAGE
#define PACKAGE "vidmag"
#endif

GST_PLUGIN_DEFINE(
    GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    evmfilter,
    "Video magnification filter",
    evmfilter_init,
    PACKAGE_VERSION,
    GST_LICENSE,
    GST_PACKAGE_NAME,
    GST_PACKAGE_ORIGIN
)
