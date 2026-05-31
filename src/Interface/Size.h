#pragma once

namespace ScaleformUI
{
	class Size
	{
		public:
			enum class SizeType : uint16_t
			{
				kLocal = 0,
				kGlobal = 1,
				kScreenWidthRatio = 2,
				kScreenHeightRatio = 3,
				kParentWidthRatio = 4,
				kParentHeightRatio = 5,
			};
			float size = 0.0f;
			std::unique_ptr<Size> next = nullptr;

			Size() {}
			Size(const Size& a_other) 
			{
				size = a_other.size;
				type = a_other.type;
				if (a_other.next) next = std::make_unique<Size>(*a_other.next.get());
			}
			Size(float a_size) : size(a_size) {}

			Size& local() { SetType(SizeType::kLocal); return *this; }
			Size& global() { SetType(SizeType::kGlobal); return *this; }
			Size& vw() { SetType(SizeType::kScreenWidthRatio); return *this; }
			Size& vh() { SetType(SizeType::kScreenHeightRatio); return *this; }
			Size& pw() { SetType(SizeType::kParentWidthRatio); return *this; }
			Size& ph() { SetType(SizeType::kParentHeightRatio); return *this; }

			bool IsLocal() const { return type == SizeType::kLocal; }
			bool IsGlobal() const { return type == SizeType::kGlobal; }
			bool IsVW() const { return type == SizeType::kScreenWidthRatio; }
			bool IsVH() const { return type == SizeType::kScreenHeightRatio; }
			bool IsPW() const { return type == SizeType::kParentWidthRatio; }
			bool IsPH() const { return type == SizeType::kParentHeightRatio; }

			Size& operator+(Size a_other) 
			{
				Size* _this = this;
				while (_this->next) 
				{
					_this = _this->next.get();
				}
				_this->next = std::make_unique<Size>(a_other);

				return *this; 
			}
			Size& operator-(Size a_other)
			{
				Size negative = a_other;
				negative.size *= -1;
				return *this + negative;
			}

		private:
			SizeType type = SizeType::kLocal;
			void SetType(SizeType a_type) { type = a_type; }

	};
}
