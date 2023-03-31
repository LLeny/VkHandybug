#pragma once

#include "global.h"
#include "imgui.h"
#include <cctype>
#include <ciso646>
#include <cstring>
#include "editor.h"

typedef struct WatchItem
{
    uint32_t id = 0;
    std::string label;
    ImGuiDataType type = ImGuiDataType_U8;
    uint16_t address = 0;

    bool operator==(const WatchItem &b)
    {
        return id == b.id;
    }

} WatchItem;

enum DataFormat
{
    Watch_DataFormat_Bin = 0,
    Watch_DataFormat_Dec = 1,
    Watch_DataFormat_Hex = 2,
    Watch_DataFormat_COUNT
};

class WatchEditor : public IEditor
{
  public:
    WatchEditor();
    ~WatchEditor() override;

    void render() override;

    void delete_watch(const char *label);
    void add_watch(const char *label, const char *type, uint16_t addr);
    std::vector<WatchItem> &watches();

  private:
    std::vector<WatchItem> _items;

    char _newItemLabelBuf[50]{};
    char _newItemAddrBuf[50]{};
    ImGuiDataType _newItemDataType{ImGuiDataType_U8};

    void delete_watch(const WatchItem *item);
    void delete_watch(uint16_t id);
    void add_watch(const char *label, ImGuiDataType type, const char *addr);
    void add_watch(const char *label, ImGuiDataType type, uint16_t addr);
    ImGuiDataType dataType_get_type(const char *data_type);
    const char *dataType_get_desc(ImGuiDataType data_type) const;
    size_t dataType_get_size(ImGuiDataType data_type) const;
    void draw_preview_data(const ImU8 *mem_data, size_t mem_size, ImGuiDataType data_type, DataFormat data_format, char *out_buf, size_t out_buf_size) const;
    const char *format_binary(const uint8_t *buf, int width) const;
};