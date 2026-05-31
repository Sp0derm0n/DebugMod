#pragma once

namespace ScaleformUI
{
	template<class E>
	class UIFlag
	{
		using enum_type = E;
		public:
			constexpr UIFlag() noexcept = default;
			constexpr UIFlag(enum_type a_enumValue) noexcept : _impl(static_cast<uint32_t>(a_enumValue)) {}

			constexpr enum_type get() const noexcept { return static_cast<enum_type>(_impl); }
			constexpr uint32_t underlying() const noexcept { return _impl; }

			constexpr void set(enum_type a_enumValue) noexcept
			{
				_impl |= static_cast<uint32_t>(a_enumValue);
			}

			constexpr void reset(enum_type a_enumValue) noexcept
			{
				_impl &= ~static_cast<uint32_t>(a_enumValue);
			}

			constexpr bool hasFlag(enum_type a_enumValue) const noexcept
			{
				return _impl & static_cast<uint32_t>(a_enumValue);
			}

			constexpr bool isNone() const noexcept
			{
				return _impl == 0;
			}

		private:
			uint32_t _impl{ 0 };
	};

	enum class Alignment : uint16_t
	{
		kNone = 0,
		kLeft = 1 << 0,
		kCenterH = 1 << 1,
		kRight = 1 << 2,
		kTop = 1 << 3,
		kCenterV = 1 << 4,
		kBottom = 1 << 5,
		kInside = 1 << 6,
		kOutside = 1 << 7
	};

	static inline bool CompareAlignments(Alignment a_align1, Alignment a_align2)
	{
		return static_cast<uint16_t>(a_align1) & static_cast<uint16_t>(a_align2);
	}

	static inline Alignment CombineAlignments(Alignment a_align1, Alignment a_align2)
	{
		return static_cast<Alignment>(static_cast<uint16_t>(a_align1) + static_cast<uint16_t>(a_align2));
	}

	static inline void PrintGFxChildren(RE::GFxValue& a_value)
	{
		a_value.VisitMembers([&](const char* a_memberName, RE::GFxValue a_memberGFx)
		{
			std::string value = ""s;

			if (a_memberGFx.IsBool()) value = a_memberGFx.GetBool() ? "true"s : "false"s;
			else if (a_memberGFx.IsArray()) value = "Array"s;
			else if (a_memberGFx.IsDisplayObject()) value = "Display Object";
			else if (a_memberGFx.IsNull()) value = "Null";
			else if (a_memberGFx.IsNumber()) value = std::to_string(a_memberGFx.GetNumber());
			else if (a_memberGFx.IsString()) value = a_memberGFx.GetString();
			else if (a_memberGFx.IsStringW()) value = "wString";
			else if (a_memberGFx.IsUndefined()) value = "Undefined";

			logger::debug("  {}: {}", a_memberName, value);
		});
	}

#ifdef LOG_UI
	class UILogger
	{
		public:
			void Print(std::string a_msg);
			void PrintWarning(std::string a_msg);
			void PrintError(std::string a_msg);

		private:
			enum class LogLevel
			{
				kAll = 0,
				kWarning = 1,
				kError = 2
			};

			uint32_t UIIndent;

	};
		
		std::string& GetUIIndent();
#endif
	
}