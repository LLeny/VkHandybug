#include "memory_editor.h"
#include "session.h"
#include "config.h"
#include "log.h"
#include <fmt/core.h>

MemEditor::MemEditor(int id)
{
    _id = id;
    _identifier = fmt::format("Memory {}", id);

    _memoryEditor.HandlersContext = this;

    _memoryEditor.WriteFn = [](ImU8 *data, size_t off, ImU8 d, void *context) {
        ((MemEditor *)context)->write_changes((uint16_t)off, d);
    };

    _memoryEditor.ReadFn = [](const ImU8 *data, size_t off, void *context) {
        return ((MemEditor *)context)->read_mem((uint16_t)off);
    };
}

MemEditor::~MemEditor()
{
    Config::get_instance().delete_memory_editor(_session->identifier(), this);
}

void MemEditor::set_session(std::shared_ptr<Session> session)
{
    _session = session;
    Config::get_instance().load_memory_editor(_session->identifier(), this);
}

bool MemEditor::enabled()
{
    return _session != nullptr;
}

int MemEditor::id()
{
    return _id;
}

std::string MemEditor::identifer()
{
    return _identifier;
}

bool MemEditor::is_read_only()
{
    if (!enabled())
    {
        return true;
    }

    return _session->status() != SessionStatus_Break;
}

uint16_t MemEditor::bank_upper_bound(LynxMemBank access)
{
    uint16_t bounds[] = {0xFFFF, 0x1FF, 0xFF, 0xFF, 0xFFFF};
    IM_ASSERT(access >= LynxMemBank_MIN && access < LynxMemBank_MAX);
    return bounds[access];
}

const char *MemEditor::bank_label(LynxMemBank access)
{
    const char *descs[] = {"RAM", "ROM", "Suzy", "Mikey", "CPU", "CART", "EEPROM"};
    IM_ASSERT(access >= LynxMemBank_MIN && access < LynxMemBank_MAX);
    return descs[access];
}

void MemEditor::draw_bank_button(LynxMemBank bank)
{
    bool pushed = false;

    if (_memBank == bank)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetColorU32(ImGuiCol_ButtonHovered));
        pushed = true;
    }

    if (ImGui::SmallButton(bank_label(bank)))
    {
        _memBank = bank;
        _bankUppberBound = bank_upper_bound(bank) + 1;
    }

    if (pushed)
    {
        ImGui::PopStyleColor();
    }
}

void MemEditor::draw_contents()
{
    if (!enabled())
    {
        return;
    }

    for (int a = LynxMemBank_MIN; a < LynxMemBank_MAX; ++a)
    {
        draw_bank_button((LynxMemBank)a);
        if (a < LynxMemBank_MAX - 1)
        {
            ImGui::SameLine();
        }
    }

    _memoryEditor.ReadOnly = is_read_only();
    _memoryEditor.DrawContents(nullptr, _bankUppberBound);
}

void MemEditor::write_changes(uint16_t offset, ImU8 data)
{
    switch (_memBank)
    {
    case LynxMemBank_RAM:
        _session->system()->Poke_RAM(offset, data);
        break;
    case LynxMemBank_ROM:
        _session->system()->mRom->Poke(offset, data);
        break;
    case LynxMemBank_Mikey:
        _session->system()->mMikie->Poke(offset, data);
        break;
    case LynxMemBank_Suzy:
        _session->system()->mSusie->Poke(offset, data);
        break;
    case LynxMemBank_CPU:
        _session->system()->Poke_CPU(offset, data);
        break;
    case LynxMemBank_CART:
        _session->system()->Poke_CART(offset, data);
        break;
    case LynxMemBank_EEPROM:
        _session->system()->mEEPROM->Poke(offset, data);
        break;
    default:
        LOG(LOGLEVEL_ERROR) << "MemEditor: Can't write to bank: " << (int)_memBank;
        break;
    }
}

ImU8 MemEditor::read_mem(uint16_t offset)
{
    switch (_memBank)
    {
    case LynxMemBank_RAM:
        return _session->system()->Peek_RAM(offset);
    case LynxMemBank_ROM:
        return _session->system()->mRom->Peek(offset);
    case LynxMemBank_Mikey:
        return _session->system()->mMikie->Peek(offset);
    case LynxMemBank_Suzy:
        return _session->system()->mSusie->Peek(offset);
    case LynxMemBank_CPU:
        return _session->system()->Peek_CPU(offset);
    case LynxMemBank_CART:
        return _session->system()->Peek_CART(offset);
    case LynxMemBank_EEPROM:
        return _session->system()->mEEPROM->Peek(offset);
    default:
        return 0;
    }
}
