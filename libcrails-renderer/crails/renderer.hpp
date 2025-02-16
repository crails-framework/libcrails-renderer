#ifndef  RENDERER_HPP
# define RENDERER_HPP

# include <list>
# include <map>
# include <memory>
# include <string_view>
# include <crails/utils/singleton.hpp>
# include <crails/utils/backtrace.hpp>
# include <crails/datatree.hpp>
# include <crails/shared_vars.hpp>
# include "render_target.hpp"

namespace Crails
{
  class Renderer;

  struct MissingTemplate : public boost_ext::exception
  {
    MissingTemplate(const std::string& name, const std::string& accept, const Renderer* = nullptr);
    MissingTemplate(const std::string& name, const Renderer* = nullptr);

    const char* what() const throw() { return message.c_str(); }
    void debug() const;
    std::string name, message;
    const Renderer* renderer;
  };

  class Renderer;

  class Renderers
  {
    SINGLETON(Renderers)
    typedef std::list<std::unique_ptr<const Renderer>> RendererList;
  protected:
    Renderers() {}
    virtual ~Renderers() {}
  public:
    RendererList::const_iterator begin() const { return renderers.begin(); }
    RendererList::const_iterator end() const { return renderers.end(); }
    const std::string& get_default_format() const { return default_format; }

  protected:
    std::string  default_format = "*/*";
    RendererList renderers;
  };

  class Renderer
  {
    friend class MissingTemplate;
  protected:
    typedef void (*Generator)(const Renderer&, RenderTarget&, SharedVars&);
    typedef std::map<std::string, Generator> Templates;
  public:
    virtual ~Renderer() {}
    virtual std::string_view get_name() const = 0;
    virtual const std::vector<std::string>& get_mimetypes() const = 0;
    virtual bool can_render(const std::string& view) const;
    virtual void render_template(const std::string& view, RenderTarget&, SharedVars& vars) const = 0;
    const Templates& get_templates() const { return templates; }
    void merge(const Renderer&);

    static const Renderer* pick_renderer(const std::string& view, const std::string& accept);
    static bool            has_renderer(const std::string& view, const std::string& accept);
    static void            render(const std::string& view, const std::string& accept, RenderTarget&, SharedVars& vars);

  protected:
    Templates templates;
  };
}

#endif
