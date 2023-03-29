#pragma once
#include "global.h"
#include "editor.h"

class SessionControlEditor : public IEditor
{
  public:
    SessionControlEditor();
    ~SessionControlEditor();

    void render() override;

  private:
    bool create_button(std::string text, ImVec2 size, std::string tooltip);
};