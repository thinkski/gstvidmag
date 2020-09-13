/*
 * Copyright (C) 2020 Chris Hiszpanski <chris@hiszpanski.name>
 */

#ifndef __GST_EVMFILTER_H__
#define __GST_EVMFILTER_H__

#include <gst/gst.h>

G_BEGIN_DECLS

#define GST_TYPE_EVMFILTER (gst_evmfilter_get_type())
G_DECLARE_FINAL_TYPE (GstEVMFilter, gst_evmfilter,
    GST, PLUGIN_TEMPLATE, GstElement)

typedef struct _GstEVMFilter
{
  GstElement element;

  GstPad *sinkpad, *srcpad;

  // spatial cutoff wavelength
  gfloat cutoff;

  // band-pass filter lower and upper bounds
  gfloat wl, wh;

  // amplification factor
  gfloat gain;
} GstEVMFilter;

#define GST_EVMFILTER(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_EVMFILTER,GstEVMFilter))

G_END_DECLS

#endif /* __GST_EVMFILTER_H__ */
