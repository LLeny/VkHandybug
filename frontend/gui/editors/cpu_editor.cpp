
#include "cpu_editor.h"

#include <fmt/core.h>

CPUEditor::CPUEditor()
{
}

CPUEditor::~CPUEditor()
{
}

void CPUEditor::draw_register(const char *label, uint8_t reg)
{
    std::string regBuf = fmt::format("{:02X}", reg);
    std::string labelbuf = fmt::format("##{}", label);

    ImGui::AlignTextToFramePadding();

    ImGui::Text("%s", label);

    ImGui::SameLine(LABEL_WIDTH);
    ImGui::SetNextItemWidth(ITEM_WIDTH);

    ImGui::BeginDisabled(is_read_only());
    if (ImGui::InputText(labelbuf.data(), regBuf.data(), regBuf.size() + 1, ImGuiInputTextFlags_CharsHexadecimal, 0, (void *)label))
    {
        int r;
        std::from_chars(regBuf.data(), regBuf.data() + regBuf.size(), r, 16);

        C6502_REGS regs;
        _session->system()->mCpu->GetRegs(regs);

        if (0 == strcmp(label, "A"))
        {
            regs.A = r;
        }
        else if (0 == strcmp(label, "X"))
        {
            regs.X = r;
        }
        else if (0 == strcmp(label, "Y"))
        {
            regs.Y = r;
        }
        _session->system()->mCpu->SetRegs(regs);
    }
    ImGui::EndDisabled();

    ImGui::SameLine(2 * ITEM_WIDTH);

    ImGui::Text("%s", fmt::format("{:08b}", reg).c_str());

    ImGui::SameLine(4 * ITEM_WIDTH);
    ImGui::Text("%d", reg);

    ImGui::SameLine(5 * ITEM_WIDTH);
    ImGui::Text("%c", reg);
}

void CPUEditor::draw_PS(const char *label, uint16_t ps)
{
    std::string regBuf = fmt::format("{:04X}", ps);
    std::string labelbuf = fmt::format("##{}", label);

    ImGui::AlignTextToFramePadding();

    ImGui::Text("%s", label);

    ImGui::SameLine(LABEL_WIDTH);
    ImGui::SetNextItemWidth(ITEM_WIDTH);

    ImGui::BeginDisabled(is_read_only());
    if (ImGui::InputText(labelbuf.data(), regBuf.data(), regBuf.size() + 1, ImGuiInputTextFlags_CharsHexadecimal))
    {
        int v;
        std::from_chars(regBuf.data(), regBuf.data() + regBuf.size(), v, 16);

        C6502_REGS regs;
        _session->system()->mCpu->GetRegs(regs);

        if (0 == strcmp(label, "S"))
        {
            regs.SP = (v & 0xff) | 0x0100;
        }
        else if (0 == strcmp(label, "PC"))
        {
            regs.PC = v;
        }

        _session->system()->mCpu->SetRegs(regs);
    }
    ImGui::EndDisabled();
}

void CPUEditor::draw_flag(const char *label, bool enabled, bool *b)
{
    std::string labelbuf = fmt::format("##{}", label);
    *b = enabled;
    ImGui::AlignTextToFramePadding();

    ImGui::Text("%s", label);
    ImGui::SameLine();
    ImGui::Checkbox(labelbuf.data(), b);
    if (ImGui::IsItemClicked())
    {
        C6502_REGS regs;
        _session->system()->mCpu->GetRegs(regs);
        int mask;

        if (0 == strcmp(label, "N"))
        {
            mask = 0x80;
        }
        else if (0 == strcmp(label, "V"))
        {
            mask = 0x40;
        }
        else if (0 == strcmp(label, "D"))
        {
            mask = 0x08;
        }
        else if (0 == strcmp(label, "I"))
        {
            mask = 0x04;
        }
        else if (0 == strcmp(label, "Z"))
        {
            mask = 0x02;
        }
        else if (0 == strcmp(label, "C"))
        {
            mask = 0x01;
        }
        else
        {
            return;
        }

        auto p = regs.PS;
        regs.PS = p & mask ? p & ~mask : p | mask;

        _session->system()->mCpu->SetRegs(regs);
    }
}

void CPUEditor::render()
{
    if (!enabled())
    {
        return;
    }

    C6502_REGS regs;
    _session->system()->mCpu->GetRegs(regs);

    draw_register("A", (uint8_t)regs.A);
    draw_register("X", (uint8_t)regs.X);
    draw_register("Y", (uint8_t)regs.Y);

    draw_PS("PC", (uint16_t)regs.PC);
    draw_PS("S", (uint16_t)regs.SP);

    // NVss DIZC
    auto p = regs.PS;
    ImGui::BeginDisabled(is_read_only());
    draw_flag("N", p & 0x80, &_N);
    ImGui::SameLine();
    draw_flag("V", p & 0x40, &_V);
    ImGui::SameLine();
    draw_flag("D", p & 0x08, &_D);
    ImGui::SameLine();
    draw_flag("I", p & 0x04, &_I);
    ImGui::SameLine();
    draw_flag("Z", p & 0x02, &_Z);
    ImGui::SameLine();
    draw_flag("C", p & 0x01, &_C);
    ImGui::EndDisabled();
}