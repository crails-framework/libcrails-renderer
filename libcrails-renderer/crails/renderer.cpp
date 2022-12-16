#include <iostream>
#include "renderer.hpp"

using namespace Crails;
using namespace std;

static string get_format(const string& format, const string& default_format)
{
  return format.length() == 0 ? default_format : format;
}

void Renderer::render(const std::string& view, const string& accept, RenderTarget& target, SharedVars& vars)
{
  const Renderer* renderer = pick_renderer(view, accept);

  if (renderer == nullptr)
  {
    const Renderers* renderers = Renderers::singleton::get();
    throw MissingTemplate(view, get_format(accept, renderers->get_default_format()));
  }
  renderer->render_template(view, target, vars);
}

const Renderer* Renderer::pick_renderer(const string& view, const string& accept)
{
  const Renderers* renderers = Renderers::singleton::get();
  string format;

  if (!renderers)
    throw boost_ext::runtime_error("Crails::Renderers singleton hasn't been initialized.");
  format = get_format(accept, renderers->get_default_format());
  for (auto it = renderers->begin() ; it != renderers->end() ; ++it)
  {
    if ((*it)->can_render(format, view))
      return (*it).get();
  }
  return nullptr;
}

bool Renderer::has_renderer(const std::string& view, const string& accept)
{
  return pick_renderer(view, accept) != nullptr;
}
