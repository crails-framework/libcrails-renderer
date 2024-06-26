#include <iostream>
#include <crails/logger.hpp>
#include <crails/utils/split.hpp>
#include <crails/utils/join.hpp>
#include "renderer.hpp"

using namespace Crails;
using namespace std;

static bool find_in_string(const string& a, const string& b)
{
  int index = a.find(b);

  return index >= 0 && index < a.length();
}

static string get_format(const string& format, const string& default_format)
{
  if (format.length() == 0)
    logger << Logger::Debug << "Renderer: get_format: query did not include a format, using default: " << default_format << Logger::endl;
  return format.length() == 0 ? default_format : format;
}

static bool match_mimetype_strict(const string& mimetype, const string& format)
{
  return find_in_string(format, mimetype);
}

static bool match_mimetype_loose(const string& mimetype, const string& format)
{
  string broad_mimetype = mimetype.substr(0, mimetype.find('/') + 1) + '*';

  return find_in_string(format, broad_mimetype);
}

static bool match_mimetype_any(const string&, const string& format)
{
  return find_in_string(format, "*/*");
}

typedef bool (*MimetypeMatcher)(const string&, const string&);

static const std::vector<MimetypeMatcher> mimetype_matchers = {
  &match_mimetype_strict, &match_mimetype_loose, &match_mimetype_any
};

static const Renderer* pick_renderer_with_filter(const Renderers& renderers, const string& view, const string& accept, MimetypeMatcher matcher)
{
  for (auto it = renderers.begin() ; it != renderers.end() ; ++it)
  {
    for (const string& mimetype : (*it)->get_mimetypes())
    {
      if ((*matcher)(mimetype, accept) && (*it)->can_render(view))
        return (*it).get();
    }
  }
  return nullptr;
}

static std::string debug_renderer_identifier(const Renderer* renderer)
{
  if (renderer)
  {
    return '<' + std::string(renderer->get_name()) + '(' + Crails::join(renderer->get_mimetypes(), ';') + ")>";
  }
  return "<no renderer>";
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
  list<string> accepted_formats;
  string format;

  if (!renderers)
    throw boost_ext::runtime_error("Crails::Renderers singleton hasn't been initialized.");
  format = get_format(accept, renderers->get_default_format());
  for (const string& accepted_format : split(format, ','))
  {
    for (MimetypeMatcher matcher : mimetype_matchers)
    {
      const Renderer* result = pick_renderer_with_filter(*renderers, view, accepted_format, matcher);

      if (result)
      {
        logger << Logger::Debug << "Renderer::pick_renderer: picked "
               << std::bind(&debug_renderer_identifier, result)
               << " for " << view << " with formats " << accepted_format
               << " (out of accepted formats: `" << accept << "`)" << Logger::endl;
        return result;
      }
    }
  }
  return nullptr;
}

bool Renderer::has_renderer(const std::string& view, const string& accept)
{
  return pick_renderer(view, accept) != nullptr;
}

bool Renderer::can_render(const std::string& view) const
{
  return templates.find(view) != templates.end();
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
  if (renderer)
  {
    logger << Logger::Error << message << ": Available templates for renderer `" << std::bind(&debug_renderer_identifier, renderer) << "` were:\n";
    for (const auto& entry : renderer->templates)
      logger << " - " << entry.first << '\n';
    logger << Logger::endl;
  }
  else
    logger << Logger::Error << "No renderer was picked." << Logger::endl;
}
