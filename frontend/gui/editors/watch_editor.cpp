#include "watch_editor.h"
#include "session.h"
#include <fmt/core.h>
#include "bootstrap-icons.h"

WatchEditor::WatchEditor()
{
}

WatchEditor::~WatchEditor()
{
}

const char *WatchEditor::dataType_get_desc(ImGuiDataType data_type) const
{
    const char *descs[] = {"Int8", "Uint8", "Int16", "Uint16", "Int32", "Uint32", "Int64", "Uint64", "Float", "Double"};
    IM_ASSERT(data_type >= 0 && data_type < ImGuiDataType_COUNT);
    return descs[data_type];
}

ImGuiDataType WatchEditor::dataType_get_type(const char *data_type)
{
    for (int i = 0; i < ImGuiDataType_COUNT; ++i)
    {
        auto t = dataType_get_desc(i);

        while (*data_type or *t)
        {
            char c = std::toupper(*data_type++);
            char d = std::toupper(*t++);
            if (c != d)
                continue;
        }
        return i;
    }
    return -1;
}

size_t WatchEditor::dataType_get_size(ImGuiDataType data_type) const
{
    const size_t sizes[] = {1, 1, 2, 2, 4, 4, 8, 8, sizeof(float), sizeof(double)};
    IM_ASSERT(data_type >= 0 && data_type < ImGuiDataType_COUNT);
    return sizes[data_type];
}

const char *WatchEditor::format_binary(const uint8_t *buf, int width) const
{
    IM_ASSERT(width <= 64);
    size_t out_n = 0;
    static char out_buf[64 + 8 + 1];
    int n = width / 8;
    for (int j = n - 1; j >= 0; --j)
    {
        for (int i = 0; i < 8; ++i)
            out_buf[out_n++] = (buf[j] & (1 << (7 - i))) ? '1' : '0';
        out_buf[out_n++] = ' ';
    }
    IM_ASSERT(out_n < IM_ARRAYSIZE(out_buf));
    out_buf[out_n] = 0;
    return out_buf;
}

void WatchEditor::draw_preview_data(const ImU8 *mem_data, size_t mem_size, ImGuiDataType data_type, DataFormat data_format, char *out_buf, size_t out_buf_size) const
{
    uint8_t buf[8];
    size_t size = dataType_get_size(data_type);

    memcpy(buf, mem_data, size);

    if (data_format == Watch_DataFormat_Bin)
    {
        uint8_t binbuf[8];
        memcpy(binbuf, buf, size);
        snprintf(out_buf, out_buf_size, "%s", format_binary(binbuf, (int)size * 8));
        return;
    }

    out_buf[0] = 0;
    switch (data_type)
    {
    case ImGuiDataType_S8: {
        int8_t int8 = 0;
        memcpy(&int8, buf, size);
        if (data_format == Watch_DataFormat_Dec)
        {
            snprintf(out_buf, out_buf_size, "%hhd", int8);
            return;
        }
        if (data_format == Watch_DataFormat_Hex)
        {
            snprintf(out_buf, out_buf_size, "0x%02x", int8 & 0xFF);
            return;
        }
        break;
    }
    case ImGuiDataType_U8: {
        uint8_t uint8 = 0;
        memcpy(&uint8, buf, size);
        if (data_format == Watch_DataFormat_Dec)
        {
            snprintf(out_buf, out_buf_size, "%hhu", uint8);
            return;
        }
        if (data_format == Watch_DataFormat_Hex)
        {
            snprintf(out_buf, out_buf_size, "0x%02x", uint8 & 0XFF);
            return;
        }
        break;
    }
    case ImGuiDataType_S16: {
        int16_t int16 = 0;
        memcpy(&int16, buf, size);
        if (data_format == Watch_DataFormat_Dec)
        {
            snprintf(out_buf, out_buf_size, "%hd", int16);
            return;
        }
        if (data_format == Watch_DataFormat_Hex)
        {
            snprintf(out_buf, out_buf_size, "0x%04x", int16 & 0xFFFF);
            return;
        }
        break;
    }
    case ImGuiDataType_U16: {
        uint16_t uint16 = 0;
        memcpy(&uint16, buf, size);
        if (data_format == Watch_DataFormat_Dec)
        {
            snprintf(out_buf, out_buf_size, "%hu", uint16);
            return;
        }
        if (data_format == Watch_DataFormat_Hex)
        {
            snprintf(out_buf, out_buf_size, "0x%04x", uint16 & 0xFFFF);
            return;
        }
        break;
    }
    case ImGuiDataType_S32: {
        int32_t int32 = 0;
        memcpy(&int32, buf, size);
        if (data_format == Watch_DataFormat_Dec)
        {
            snprintf(out_buf, out_buf_size, "%d", int32);
            return;
        }
        if (data_format == Watch_DataFormat_Hex)
        {
            snprintf(out_buf, out_buf_size, "0x%08x", int32);
            return;
        }
        break;
    }
    case ImGuiDataType_U32: {
        uint32_t uint32 = 0;
        memcpy(&uint32, buf, size);
        if (data_format == Watch_DataFormat_Dec)
        {
            snprintf(out_buf, out_buf_size, "%u", uint32);
            return;
        }
        if (data_format == Watch_DataFormat_Hex)
        {
            snprintf(out_buf, out_buf_size, "0x%08x", uint32);
            return;
        }
        break;
    }
    case ImGuiDataType_S64: {
        int64_t int64 = 0;
        memcpy(&int64, buf, size);
        if (data_format == Watch_DataFormat_Dec)
        {
            snprintf(out_buf, out_buf_size, "%lld", (long long)int64);
            return;
        }
        if (data_format == Watch_DataFormat_Hex)
        {
            snprintf(out_buf, out_buf_size, "0x%016llx", (long long)int64);
            return;
        }
        break;
    }
    case ImGuiDataType_U64: {
        uint64_t uint64 = 0;
        memcpy(&uint64, buf, size);
        if (data_format == Watch_DataFormat_Dec)
        {
            snprintf(out_buf, out_buf_size, "%llu", (long long)uint64);
            return;
        }
        if (data_format == Watch_DataFormat_Hex)
        {
            snprintf(out_buf, out_buf_size, "0x%016llx", (long long)uint64);
            return;
        }
        break;
    }
    case ImGuiDataType_Float: {
        float float32 = 0.0f;
        memcpy(&float32, buf, size);
        if (data_format == Watch_DataFormat_Dec)
        {
            snprintf(out_buf, out_buf_size, "%f", float32);
            return;
        }
        if (data_format == Watch_DataFormat_Hex)
        {
            snprintf(out_buf, out_buf_size, "%a", float32);
            return;
        }
        break;
    }
    case ImGuiDataType_Double: {
        double float64 = 0.0;
        memcpy(&float64, buf, size);
        if (data_format == Watch_DataFormat_Dec)
        {
            snprintf(out_buf, out_buf_size, "%f", float64);
            return;
        }
        if (data_format == Watch_DataFormat_Hex)
        {
            snprintf(out_buf, out_buf_size, "%a", float64);
            return;
        }
        break;
    }
    case ImGuiDataType_COUNT:
        break;
    }             // Switch
    IM_ASSERT(0); // Shouldn't reach
}

void WatchEditor::delete_watch(const char *label)
{
    std::string ls = label;
    _items.erase(std::remove_if(_items.begin(), _items.end(), [ls](WatchItem x) { return x.label == ls; }), _items.end());
}

void WatchEditor::delete_watch(uint16_t id)
{
    _items.erase(std::remove_if(_items.begin(), _items.end(), [id](WatchItem x) { return x.id == id; }), _items.end());
}

void WatchEditor::delete_watch(const WatchItem *item)
{
    _items.erase(std::remove(_items.begin(), _items.end(), *item), _items.end());
}

void WatchEditor::add_watch(const char *label, ImGuiDataType type, const char *addr)
{
    int a = _session->symbols().get_addr(std::string(label));

    if (a < 0 && strlen(addr) <= 4)
    {
        sscanf(addr, "%04X", &a);
    }

    if (a > 0)
    {
        add_watch(label, type, a);
    }
}

void WatchEditor::add_watch(const char *label, ImGuiDataType type, uint16_t addr)
{
    if (strlen(label) <= 0 || addr > 0xffff)
    {
        return;
    }

    if (std::any_of(_items.begin(), _items.end(), [type, addr](const WatchItem i) { return i.address == addr && i.type == type; }))
    {
        return;
    }

    WatchItem item;

    if (!_items.empty())
    {
        item.id = _items.back().id + 1;
    }
    item.type = type;
    item.label = label;
    item.address = addr;

    memset(_newItemAddrBuf, 0, sizeof(_newItemAddrBuf));
    memset(_newItemLabelBuf, 0, sizeof(_newItemLabelBuf));

    _items.push_back(item);
}

void WatchEditor::add_watch(const char *label, const char *type, uint16_t addr)
{
    ImGuiDataType t = dataType_get_type(type);

    if (t < 0)
    {
        return;
    }

    add_watch(label, t, addr);
}

std::vector<WatchItem> &WatchEditor::watches()
{
    return _items;
}

void WatchEditor::render()
{
    ImU8 dataBuf[8];
    char dataOutputBuf[8 * 8];
    char labelBuf[10];
    WatchItem item{};

    ImGui::AlignTextToFramePadding();

    ImGui::Text("Label");

    ImGui::SameLine();
    ImGui::SetNextItemWidth(120);
    ImGui::InputText("##watchlabel", _newItemLabelBuf, 49);

    ImGui::SameLine();
    ImGui::Text("Address");

    ImGui::SameLine();
    ImGui::SetNextItemWidth(120);
    if (imgui_autocomplete_input("##watchaddr", _newItemAddrBuf, sizeof(_newItemAddrBuf), _session->symbols().overrides(), ImGuiInputTextFlags_None) && !_newItemLabelBuf[0])
    {
        strncpy(_newItemLabelBuf, _newItemAddrBuf, sizeof(_newItemAddrBuf));
    }

    ImGui::SameLine();
    ImGui::Text("Type");

    ImGui::SameLine();
    ImGui::SetNextItemWidth(70);
    if (ImGui::BeginCombo("##watchtype", dataType_get_desc(_newItemDataType), ImGuiComboFlags_HeightLargest))
    {
        for (int n = 0; n < ImGuiDataType_COUNT; n++)
            if (ImGui::Selectable(dataType_get_desc((ImGuiDataType)n), _newItemDataType == n))
                _newItemDataType = (ImGuiDataType)n;
        ImGui::EndCombo();
    }

    ImGui::SameLine();
    ImGui::SetNextItemWidth(50);
    if (!is_read_only() && ImGui::Button("Add"))
    {
        if (strlen(_newItemLabelBuf) <= 0 || is_read_only())
        {
            return;
        }

        add_watch(_newItemLabelBuf, _newItemDataType, _newItemAddrBuf);
    }

    ImGui::Separator();

    if (ImGui::BeginTable("##watchitems", 6, ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingFixedFit))
    {
        ImGui::TableSetupColumn("Del");
        ImGui::TableSetupColumn("Label");
        ImGui::TableSetupColumn("Address");
        ImGui::TableSetupColumn("Hex");
        ImGui::TableSetupColumn("Dec");
        ImGui::TableSetupColumn("Bin");
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableHeadersRow();

        for (const auto &item : _items)
        {
            auto size = dataType_get_size(item.type);

            do
            {
                --size;
                dataBuf[size] = _session->system()->Peek_RAM(item.address + (uint16_t)size);
            } while (size > 0);

            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(15);
            std::string l1 = fmt::format("{}##wd{}", BootstrapIcons_trash, item.id);
            if (ImGui::Button(l1.c_str()))
            {
                delete_watch(&item);
            }

            ImGui::TableNextColumn();
            ImGui::Text("%s", item.label.c_str());

            ImGui::TableNextColumn();
            ImGui::Text("%s", fmt::format("${:04X}", item.address).c_str());

            ImGui::TableNextColumn();
            draw_preview_data(dataBuf, sizeof(dataBuf), item.type, Watch_DataFormat_Hex, dataOutputBuf, sizeof(dataOutputBuf));
            ImGui::Text("%s", dataOutputBuf);

            ImGui::TableNextColumn();
            draw_preview_data(dataBuf, sizeof(dataBuf), item.type, Watch_DataFormat_Dec, dataOutputBuf, sizeof(dataOutputBuf));
            ImGui::Text("%s", dataOutputBuf);

            ImGui::TableNextColumn();
            draw_preview_data(dataBuf, sizeof(dataBuf), item.type, Watch_DataFormat_Bin, dataOutputBuf, sizeof(dataOutputBuf));
            ImGui::Text("%s", dataOutputBuf);
        }
        ImGui::EndTable();
    }
}