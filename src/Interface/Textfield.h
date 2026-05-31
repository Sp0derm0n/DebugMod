#pragma once

#include "Element.h"

namespace ScaleformUI
{
	class Textfield : public Element
	{
		public:
			Textfield(std::string& a_instanceName);
			
			Textfield*	AsTextfield() override { return static_cast<Textfield*>(this); }
			bool		IsTextfield() override { return true; }
			void		SetGFx(RE::GFxValue& a_GFxvalue) override;

			void		SetTextfieldWidth(Size a_width);
			void		SetTextfieldHeight(Size a_height);
			void		SetTextfieldWidth(float a_width);
			void		SetTextfieldHeight(float a_height);

			void		SetText(const char* a_text) override;
			void		SetText(std::string a_text) override;
			void		SetFont(const char* a_font) override;
			void		SetFontSize(uint32_t a_size) override;
			void		SetFontColor(uint32_t a_defaultColor) override;
			void		SetBold(bool a_enabled) override;
			void		SetItalic(bool a_enabled) override;
			void		SetUnderline(bool a_enabled) override;
			void		SetTextAlign(const char* a_align) override; 
			void		SetIndent(float a_indent) override; 
			void		SetBlockIndent(float a_indent) override;
			void		SetBullet(bool a_enabled) override;
			void		SetKerning(bool a_enabled) override;
			void		SetLeftMargin(float a_margin) override;
			void		SetRightMargin(float a_margin) override;
			void		SetLetterSpacing(float a_spacing) override;
			void		SetLineSpacing(float a_spacing) override;	
			void		SetAutoSize(bool a_enabled) override;
			void		SetWordWrap(bool a_enabled) override;

		private:
			friend class Menu;

			RE::GFxValue textFormatGFx;

			void SetTextFormatGFx(RE::GFxValue a_textFormat) { textFormatGFx = a_textFormat; }
			void OnTextFieldChange();
	};
}