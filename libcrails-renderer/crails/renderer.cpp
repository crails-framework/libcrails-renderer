#include <crails/logger.hpp>
#include <crails/utils/split.hpp>
#include <crails/utils/join.hpp>
#include <algorithm>
#include <vector>
#include "renderer.hpp"

using namespace Crails;
using namespace std;

struct AcceptedFormat
{
  string mimetype;
  double q;
  bool operator>(const AcceptedFormat& b) const { return q > b.q; }
  bool operator<(const AcceptedFormat& b) const { return q < b.q; }
};

static bool find_in_string(const string& a, const string& b)
{
  return a.find(b) != string::npos;
}

static string get_format(const string& format, const string& default_format)
{
  if (format.length() == 0)
    logger << Logger::Debug << "Renderer: get_format: query did not include a format, using default: " << default_format << Logger::endl;
  return format.length() == 0 ? default_format : format;
}

static string trim(const string& value)
{
  size_t begin = value.find_first_not_of(" \t");
  size_t end = value.find_last_not_of(" \t");

  if (begin == string::npos)
    return "";
  return value.substr(begin, end - begin + 1);
}

static AcceptedFormat parse_accepted_format(const string& token)
{
  AcceptedFormat result{"", 1.0};
  size_t semicolon = token.find(';');

  result.mimetype = trim(semicolon == string::npos ? token : token.substr(0, semicolon));
  if (semicolon != string::npos)
  {
    for (const string& parameter : split(token.substr(semicolon + 1), ';'))
    {
      string trimmed = trim(parameter);

      if (trimmed.size() > 2 && trimmed[0] == 'q' && trimmed[1] == '=')
      {
        try { result.q = std::stod(trimmed.substr(2)); }
        catch (const std::exception&) { result.q = 1.0; }
      }
    }
  }
  return result;
}

static list<string> parse_accept_header(const string& accept)
{
  vector<AcceptedFormat> parsed;
  list<string> result;

  for (const string& raw_format : split(accept, ','))
  {
    AcceptedFormat format = parse_accepted_format(raw_format);

    if (format.mimetype.length() > 0 && format.q > 0.0)
      parsed.push_back(format);
  }
  std::stable_sort(parsed.begin(), parsed.end(), std::greater<AcceptedFormat>{});
  for (const AcceptedFormat& format : parsed)
    result.push_back(format.mimetype);
  return result;
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
  string format;
  list<string> accepted_formats;

  if (!renderers)
    throw boost_ext::runtime_error("Crails::Renderers singleton hasn't been initialized.");
  format = get_format(accept, renderers->get_default_format());
  accepted_formats = parse_accept_header(format);
  for (const string& accepted_format : accepted_formats)
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
