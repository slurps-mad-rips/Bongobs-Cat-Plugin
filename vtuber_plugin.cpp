#include <obs-module.h>
#include "VtuberPlugin.hpp"

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("bongobs-cat", "en-US")
MODULE_EXPORT const char *obs_module_description(void)
{
  return "Bongo Cat";
}

bool obs_module_load(void)
{
  obs_source_info Vtuber_video {
    .id = "bongobs-cat",
    .type = OBS_SOURCE_TYPE_INPUT,
    .output_flags = OBS_SOURCE_VIDEO,
    .get_name = VtuberPlugin::VtuberPlugin::VtuberGetName,
    .create = VtuberPlugin::VtuberPlugin::VtuberCreate,
    .destroy = VtuberPlugin::VtuberPlugin::VtuberDestroy,
    .get_width = VtuberPlugin::VtuberPlugin::VtuberWidth,
    .get_height = VtuberPlugin::VtuberPlugin::VtuberHeight,
    .get_defaults = VtuberPlugin::VtuberPlugin::Vtuber_defaults,
    .get_properties = VtuberPlugin::VtuberPlugin::VtuberGetProperties,
    .update = VtuberPlugin::VtuberPlugin::Vtuber_update,
    .activate = nullptr,
    .deactivate = nullptr,
    .show = nullptr,
    .hide = nullptr,
    .video_tick = nullptr,
    .video_render = VtuberPlugin::VtuberPlugin::VtuberRender
  };

  obs_register_source(&Vtuber_video);

  return true;
}

#ifdef _WIN32
void obs_module_unload(void)
{

}
#endif
