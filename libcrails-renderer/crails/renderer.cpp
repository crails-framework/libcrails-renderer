#include <iostream>
#include <crails/logger.hpp>
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

void Renderer::merge(const Renderer& other)
{
  for (const auto& entry : other.templates)
  {
    auto item = templates.find(entry.first);

    if (item == templates.end())
      templates.emplace(entry.first, entry.second);
    else
      item->second = entry.second;
  }
}

MissingTemplate::MissingTemplate(const string& name, const string& accept, const Renderer* renderer)
  : name(name), message("Template not found '" + name + "' with format '" + accept + '\''), renderer(renderer)
{
  if (renderer) debug();
}

MissingTemplate::MissingTemplate(const string& name, const Renderer* renderer)
  : name(name), message("Template not found '" + name + '\''), renderer(renderer)
{
  if (renderer) debug();
}

void MissingTemplate::debug() const
{
  logger << Logger::Error << message << ": Available templates were:\n";
  for (const auto& entry : renderer->templates)
    logger << " - " << entry.first << '\n';
  logger << Logger::endl;
}
