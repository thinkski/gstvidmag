project('gstvidmag', 'c', version : '1.0.0', license : 'LGPL')

plugins_install_dir = join_paths(get_option('libdir'), 'gstreamer-1.0')

cc = meson.get_compiler('c')

gstvidmag_version = meson.project_version()

api_version = '1.0'

gstbase_dep = dependency('gstreamer-base-1.0',
  fallback : ['gstreamer', 'gstbase_dep'])
gstvideo_dep = dependency('gstreamer-video-1.0',
  fallback : ['gstreamer', 'gstvideo_dep'])

plugin_c_args = ['-DHAVE_CONFIG_H']

cdata = configuration_data()
cdata.set_quoted('VERSION', gstvidmag_version)
cdata.set_quoted('PACKAGE', 'gstvidmag')
cdata.set_quoted('GST_LICENSE', 'LGPL')
cdata.set_quoted('GST_API_VERSION', api_version)
cdata.set_quoted('GST_PACKAGE_NAME', 'Video Magnification')
cdata.set_quoted('GST_PACKAGE_ORIGIN', 'https://chris.hiszpanski.name')
configure_file(output : 'config.h', configuration : cdata)

plugin_sources = [ 'src/gstvidmag.c' ]

gstvidmag = library('gstvidmag',
  plugin_sources,
  c_args: plugin_c_args,
  dependencies : [gstbase_dep, gstvideo_dep],
  install : true,
  install_dir : plugins_install_dir,
)
