#include "Textfield.h"
#include "Menu.h"
#include "UIUtils.h"

ScaleformUI::Textfield::Textfield(std::string& a_instanceName) : Element(a_instanceName)
{
	elementType = ELEMENT_TYPE::kTEXTFIELD;
}

void ScaleformUI::Textfield::SetGFx(RE::GFxValue& a_GFxValue)
{
	Element::SetGFx(a_GFxValue);
	
	bool success = a_GFxValue.Invoke("getTextFormat", &textFormatGFx);
}


void ScaleformUI::Textfield::OnTextFieldChange()
{
	// Trouble: copying gfx value only copies the pointer to the objectinterface, not the object itself
	RE::GFxValue size;
	GetGFx().Invoke("setTextFormat", nullptr, &textFormatGFx, 1);
	RetrievePositionalInfo();
}

void ScaleformUI::Textfield::SetText(const char* a_text)
{
	#ifdef LOG_UI
		UIIndent++;
	#endif

	auto thisGFx = GetGFx();
	bool success = thisGFx.SetText(a_text);

	OnTextFieldChange();


	#ifdef LOG_UI
		if (!success) logger::debug("{}UI ERROR: Failed to set text '{}' in textfield '{}'", GetUIIndent(), a_text, GetInstanceName());
		UIIndent--;
	#endif
}

void ScaleformUI::Textfield::SetText(std::string a_text)
{
	SetText(a_text.c_str());
}

void ScaleformUI::Textfield::SetFont(const char* a_font)
{
	#ifdef LOG_UI
		UIIndent++;
	#endif

	auto thisGFx = GetGFx();
	auto success = textFormatGFx.SetMember("font", { a_font });

	OnTextFieldChange();


	#ifdef LOG_UI
		if (!success) logger::debug("{}UI ERROR: Failed to set font '{}' of Text field '{}'", GetUIIndent(), a_font, GetInstanceName());
		UIIndent--;
	#endif
}
void ScaleformUI::Textfield::SetFontSize(uint32_t a_size)
{
	#ifdef LOG_UI
		UIIndent++;
	#endif

	auto thisGFx = GetGFx();
	auto success = textFormatGFx.SetMember("size", { a_size });

	OnTextFieldChange();

	#ifdef LOG_UI
		if (!success) logger::debug("{}UI ERROR: Failed to set font size '{}' of Text field '{}'", GetUIIndent(), a_size, GetInstanceName());
		UIIndent--;
	#endif
}
void ScaleformUI::Textfield::SetFontColor(uint32_t a_color)
{
	#ifdef LOG_UI
		UIIndent++;
	#endif

	auto thisGFx = GetGFx();
	auto success = textFormatGFx.SetMember("color", { a_color });

	OnTextFieldChange();

	#ifdef LOG_UI
		if (!success) logger::debug("{}UI ERROR: Failed to set text color '{}' of Text field '{}'", GetUIIndent(), a_color, GetInstanceName());
		UIIndent--;
	#endif
}
void ScaleformUI::Textfield::SetBold(bool a_enabled)
{
	#ifdef LOG_UI
		UIIndent++;
	#endif

	auto thisGFx = GetGFx();
	auto success = textFormatGFx.SetMember("bold", { a_enabled });

	OnTextFieldChange();

	#ifdef LOG_UI
		if (!success) logger::debug("{}UI ERROR: Failed to set bold to '{}' of Text field '{}'", GetUIIndent(), a_enabled, GetInstanceName());
		UIIndent--;
	#endif
}
void ScaleformUI::Textfield::SetItalic(bool a_enabled)
{
	#ifdef LOG_UI
		UIIndent++;
	#endif

	auto thisGFx = GetGFx();
	auto success = textFormatGFx.SetMember("italic", { a_enabled });

	OnTextFieldChange();

	#ifdef LOG_UI
		if (!success) logger::debug("{}UI ERROR: Failed to set italic to '{}' of Text field '{}'", GetUIIndent(), a_enabled, GetInstanceName());
		UIIndent--;
	#endif
}
void ScaleformUI::Textfield::SetUnderline(bool a_enabled)
{
	#ifdef LOG_UI
		UIIndent++;
	#endif

	auto thisGFx = GetGFx();
	auto success = textFormatGFx.SetMember("underline", { a_enabled });

	OnTextFieldChange();

	#ifdef LOG_UI
		if (!success) logger::debug("{}UI ERROR: Failed to set a_enabled '{}' of Text field '{}'", GetUIIndent(), a_enabled, GetInstanceName());
		UIIndent--;
	#endif
}
void ScaleformUI::Textfield::SetTextAlign(const char* a_align)
{
	#ifdef LOG_UI
		UIIndent++;
	#endif

	auto thisGFx = GetGFx();
	auto success = textFormatGFx.SetMember("align", { a_align });

	OnTextFieldChange();

	#ifdef LOG_UI
		if (!success) logger::debug("{}UI ERROR: Failed to set text alignment '{}' of Text field '{}'", GetUIIndent(), a_align, GetInstanceName());
		UIIndent--;
	#endif
}
void ScaleformUI::Textfield::SetIndent(float a_indent)
{
	#ifdef LOG_UI
		UIIndent++;
	#endif

	auto thisGFx = GetGFx();
	auto success = textFormatGFx.SetMember("indent", { a_indent });

	OnTextFieldChange();

	#ifdef LOG_UI
		if (!success) logger::debug("{}UI ERROR: Failed to set text indent to '{}' of Text field '{}'", GetUIIndent(), a_indent, GetInstanceName());
		UIIndent--;
	#endif
}
void ScaleformUI::Textfield::SetBlockIndent(float a_indent)
{
	#ifdef LOG_UI
		UIIndent++;
	#endif

	auto thisGFx = GetGFx();
	auto success = textFormatGFx.SetMember("blockIndent", { a_indent });

	OnTextFieldChange();

	#ifdef LOG_UI
		if (!success) logger::debug("{}UI ERROR: Failed to set text block indent to '{}' of Text field '{}'", GetUIIndent(), a_indent, GetInstanceName());
		UIIndent--;
	#endif
}
void ScaleformUI::Textfield::SetBullet(bool a_enabled)
{
	#ifdef LOG_UI
		UIIndent++;
	#endif

	auto thisGFx = GetGFx();
	auto success = textFormatGFx.SetMember("bullet", { a_enabled });

	OnTextFieldChange();

	#ifdef LOG_UI
		if (!success) logger::debug("{}UI ERROR: Failed to set text bullets to '{}' of Text field '{}'", GetUIIndent(), a_enabled, GetInstanceName());
		UIIndent--;
	#endif
}
void ScaleformUI::Textfield::SetKerning(bool a_enabled)
{
	#ifdef LOG_UI
		UIIndent++;
	#endif

	auto thisGFx = GetGFx();
	auto success = textFormatGFx.SetMember("kerning", { a_enabled });

	OnTextFieldChange();

	#ifdef LOG_UI
		if (!success) logger::debug("{}UI ERROR: Failed to set kerning to '{}' of Text field '{}'", GetUIIndent(), a_enabled, GetInstanceName());
		UIIndent--;
	#endif
}
void ScaleformUI::Textfield::SetLeftMargin(float a_margin)
{
	#ifdef LOG_UI
		UIIndent++;
	#endif

	auto thisGFx = GetGFx();
	auto success = textFormatGFx.SetMember("leftMargin", { a_margin });

	OnTextFieldChange();

	#ifdef LOG_UI
		if (!success) logger::debug("{}UI ERROR: Failed to set left margin to '{}' of Text field '{}'", GetUIIndent(), a_margin, GetInstanceName());
		UIIndent--;
	#endif
}
void ScaleformUI::Textfield::SetRightMargin(float a_margin)
{
	#ifdef LOG_UI
		UIIndent++;
	#endif

	auto thisGFx = GetGFx();
	auto success = textFormatGFx.SetMember("rightMargin", { a_margin });

	OnTextFieldChange();

	#ifdef LOG_UI
		if (!success) logger::debug("{}UI ERROR: Failed to set right margin to '{}' of Text field '{}'", GetUIIndent(), a_margin, GetInstanceName());
		UIIndent--;
	#endif
}
void ScaleformUI::Textfield::SetLetterSpacing(float a_spacing)
{
	#ifdef LOG_UI
		UIIndent++;
	#endif

	auto thisGFx = GetGFx();
	auto success = textFormatGFx.SetMember("letterSpacing", { a_spacing });
	
	OnTextFieldChange();

	#ifdef LOG_UI
		if (!success) logger::debug("{}UI ERROR: Failed to set letter spacing to '{}' of Text field '{}'", GetUIIndent(), a_spacing, GetInstanceName());
		UIIndent--;
	#endif
}
void ScaleformUI::Textfield::SetLineSpacing(float a_spacing)
{
	#ifdef LOG_UI
		UIIndent++;
	#endif

	auto thisGFx = GetGFx();
	auto success = textFormatGFx.SetMember("leading", { a_spacing });

	OnTextFieldChange();

	#ifdef LOG_UI
		if (!success) logger::debug("{}UI ERROR: Failed to set line spacing to '{}' of Text field '{}'", GetUIIndent(), a_spacing, GetInstanceName());
		UIIndent--;
	#endif
}
void ScaleformUI::Textfield::SetAutoSize(bool a_enabled)
{
	#ifdef LOG_UI
		UIIndent++;
	#endif

	auto thisGFx = GetGFx();
	auto success = thisGFx.SetMember("autoSize", { a_enabled });

	OnTextFieldChange();

	#ifdef LOG_UI
		if (!success) logger::debug("{}UI ERROR: Failed to set autosize to '{}' of Text field '{}'", GetUIIndent(), a_enabled, GetInstanceName());
		UIIndent--;
	#endif
}
void ScaleformUI::Textfield::SetWordWrap(bool a_enabled)
{
	#ifdef LOG_UI
		UIIndent++;
	#endif

	auto thisGFx = GetGFx();
	auto success = thisGFx.SetMember("wordWrap", { a_enabled });

	OnTextFieldChange();

	#ifdef LOG_UI
		if (!success) logger::debug("{}UI ERROR: Failed to set wordwrap to '{}' of Text field '{}'", GetUIIndent(), a_enabled, GetInstanceName());
		UIIndent--;
	#endif
}

void ScaleformUI::Textfield::SetTextfieldWidth(Size a_width)
{
	#ifdef LOG_UI
		UIIndent++;
	#endif

	float width = SizeToDistanceY(a_width);
	SetAutoSize(false); // Must be false when setting width/height

	auto thisGFx = GetGFx();
	auto success = thisGFx.SetMember("_width", { width });

	OnTextFieldChange();

	#ifdef LOG_UI
		if (!success) logger::debug("{}UI ERROR: Failed to set width to '{}' of Text field '{}'", GetUIIndent(), a_width, GetInstanceName());
		UIIndent--;
	#endif
}

void ScaleformUI::Textfield::SetTextfieldHeight(Size a_height)
{
	#ifdef LOG_UI
		UIIndent++;
	#endif

	float height = SizeToDistanceY(a_height);
	SetAutoSize(false); // Must be false when setting width/height

	auto thisGFx = GetGFx();
	auto success = thisGFx.SetMember("_height", { height });

	OnTextFieldChange();

	#ifdef LOG_UI
		if (!success) logger::debug("{}UI ERROR: Failed to set height to '{}' of Text field '{}'", GetUIIndent(), a_height, GetInstanceName());
		UIIndent--;
	#endif
}


void ScaleformUI::Textfield::SetTextfieldWidth(float a_width)
{
	SetTextfieldWidth(Size(a_width));
}

void ScaleformUI::Textfield::SetTextfieldHeight(float a_height)
{
	SetTextfieldHeight(Size(a_height));
}
