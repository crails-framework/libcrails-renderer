#include <crails/renderer.hpp>
#include <crails/logger.hpp>
#include <iostream>

struct StubRenderer : public Crails::Renderer
{
  StubRenderer(unsigned int id, const std::vector<std::string>& mimetypes) : id(id), mimetypes(mimetypes) {}

  unsigned int id;
  std::vector<std::string> mimetypes;

  std::string_view get_name() const override { return "StubRenderer"; }
  const std::vector<std::string>& get_mimetypes() const override { return mimetypes; }
  bool can_render(const std::string&) const override { return true; }
  void render_template(const std::string& view, Crails::RenderTarget&, Crails::SharedVars& vars) const override {}
};

class TestRenderers : public Crails::Renderers
{
  SINGLETON_IMPLEMENTATION(TestRenderers, Crails::Renderers)
public:
  TestRenderers()
  {
  }

  void append(const StubRenderer* renderer)
  {
    renderers.push_back(std::unique_ptr<const Crails::Renderer>(renderer));
  }
};

int main()
{
  SingletonInstantiator<TestRenderers> renderers;
  unsigned int error_count = 0;
  Crails::logger.set_log_level(Crails::Logger::endl);

  StubRenderer* decoy = new StubRenderer(0, {"nimporte/nawak"});
  StubRenderer* html = new StubRenderer(1, {"text/html"});
  StubRenderer* atom = new StubRenderer(2, {"application/atom+xml"});
  StubRenderer* json = new StubRenderer(3, {"application/feed+json", "application/json"});

  renderers->append(html);
  renderers->append(atom);
  renderers->append(json);
  renderers->append(decoy);

  if (Crails::Renderer::pick_renderer("", "text/*") != html)
  {
    std::cout << ":: Can pick renderer from a partial Accept mimetype ?" << std::endl;
    std::cerr << "!! Cannot pick renderer from a partial Accept mimetype" << std::endl;
    error_count++;
  }

  if (Crails::Renderer::pick_renderer("", "") == nullptr)
  {
    std::cout << ":: Can pick renderer with no Accept header ?" << std::endl;
    std::cerr << "!! Cannot pick renderer with no Accept header" << std::endl;
    error_count++;
  }

  if (Crails::Renderer::pick_renderer("", "application/feed+json, text/html") != json)
  {
    std::cout << ":: Can pick renderer with multiple Accept mimetypes ?" << std::endl;
    std::cerr << "!! Cannot pick renderer from multiple Accept mimetypes" << std::endl;
    error_count++;
  }

  if (Crails::Renderer::pick_renderer("", "text/plain, text/*") != html)
  {
    std::cout << ":: Can pick renderer with lower priority in Accept header ?" << std::endl;
    std::cerr << "!! Cannot pick renderer with lower priority in Accept header" << std::endl;
    error_count++;
  }

  if (Crails::Renderer::pick_renderer("", "application/atom+xml, text/html") != atom)
  {
    std::cout << ":: Can pick renderer while respecting priority settings ?" << std::endl;
    std::cerr << "!! Does not respect priority settings when picking renderer" << std::endl;
    error_count++;
  }

  if (Crails::Renderer::pick_renderer("", "text/html;q=0.5, application/atom+xml;q=0.9") != atom)
  {
    std::cout << ":: Can pick renderer while respecting explicit q values ?" << std::endl;
    std::cerr << "!! Does not respect explicit q values over header order" << std::endl;
    error_count++;
  }

  if (Crails::Renderer::pick_renderer("", "application/atom+xml;q=0, text/html") != html)
  {
    std::cout << ":: Does q=0 correctly exclude an otherwise-matching format ?" << std::endl;
    std::cerr << "!! Does not exclude a format explicitly marked as not acceptable (q=0)" << std::endl;
    error_count++;
  }

  if (Crails::Renderer::pick_renderer("", "video/mp4") != nullptr)
  {
    std::cout << ":: Does pick_renderer return no renderer when nothing matches ?" << std::endl;
    std::cerr << "!! Returned a renderer despite no compatible mimetype being registered" << std::endl;
    error_count++;
  }
  if (Crails::Renderer::has_renderer("", "video/mp4"))
  {
    std::cerr << "!! has_renderer disagrees with pick_renderer on an unmatched format" << std::endl;
    error_count++;
  }

  return error_count;
}
