#pragma once
#include "global.h"
#include "editor.h"

class App;

class SessionControlEditor : public IEditor
{
  public:
    SessionControlEditor();
    ~SessionControlEditor();

    void set_app(std::shared_ptr<App> app);

    void render() override;

  private:
    std::shared_ptr<App> _app;

    bool create_button(std::string text, ImVec2 size, std::string tooltip);
};