#include <iostream>
#include "renderer.hpp"

using namespace Crails;
using namespace std;

list<Renderer*> Crails::renderers;

void Renderer::finalize()
{
  for (auto it = renderers.begin() ; it != renderers.end() ; ++it)
    delete *it;
  renderers.clear();
}

static string get_format(const string& format, const string& default_format)
{
  return format.length() == 0 ? default_format : format;
}

void Renderer::render(const std::string& view, const string& accept, RenderTarget& target, SharedVars& vars)
{
  const Renderer* renderer = pick_renderer(view, accept);

  if (renderer == nullptr)
    throw MissingTemplate(view, get_format(accept, default_format));
  renderer->render_template(view, target, vars);
}

Renderer* Renderer::pick_renderer(const string& view, const string& accept)
{
  string format = get_format(accept, default_format);

  for (auto it = renderers.begin() ; it != renderers.end() ; ++it)
  {
    if ((*it)->can_render(format, view))
      return *it;
  }
  return nullptr;
}

bool Renderer::has_renderer(const std::string& view, const string& accept)
{
  return pick_renderer(view, accept) != nullptr;
}
