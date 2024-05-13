#include <crails/renderer.hpp>
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

  StubRenderer* null = new StubRenderer(0, {"nimporte/nawak"});
  StubRenderer* html = new StubRenderer(1, {"text/html"});
  StubRenderer* atom = new StubRenderer(2, {"application/atom+xml"});
  StubRenderer* json = new StubRenderer(3, {"application/feed+json", "application/json"});

  renderers->append(html);
  renderers->append(atom);
  renderers->append(json);

  std::cout << ":: Can pick renderer from a partial Accept mimetype ?" << std::endl;
  if (Crails::Renderer::pick_renderer("", "text/*") != html)
  {
    std::cerr << "!! Cannot pick renderer from a partial Accept mimetype" << std::endl;
    error_count++;
  }

  std::cout << ":: Can pick renderer with no Accept header ?" << std::endl;
  if (Crails::Renderer::pick_renderer("", "") == null)
  {
    std::cerr << "!! Cannot pick renderer with no Accept header" << std::endl;
    error_count++;
  }

  std::cout << ":: Can pick renderer with multiple Accept mimetypes ?" << std::endl;
  if (Crails::Renderer::pick_renderer("", "application/feed+json, text/html") == json)
  {
    std::cerr << "!! Cannot pick renderer from multiple Accept mimetypes" << std::endl;
    error_count++;
  }

  std::cout << ":: Can pick renderer with lower priority in Accept header ?" << std::endl;
  if (Crails::Renderer::pick_renderer("", "text/plain, text/*") != html)
  {
    std::cerr << "!! Cannot pick renderer with lower priority in Accept header" << std::endl;
    error_count++;
  }

  std::cout << ":: Can pick renderer while respecting priority settings ?" << std::endl;
  if (Crails::Renderer::pick_renderer("", "application/atom+xml, text/html") == atom)
  {
    std::cerr << "!! Does not respect priority settings when picking renderer" << std::endl;
    error_count++;
  }

  return error_count;
}
