#pragma once

namespace Linalg
{
	class Vector4
	{
		public:
		float x{ 0.0f };
		float y{ 0.0f };
		float z{ 0.0f };
		float w{ 0.0f };

		constexpr Vector4() noexcept = default;

		constexpr Vector4(float a_x, float a_y, float a_z, float a_w) noexcept : 
			x(a_x),
			y(a_y),
			z(a_z),
			w(a_w)
		{};

		constexpr Vector4(RE::NiPoint3 a_point) noexcept :
			x(a_point.x),
			y(a_point.y),
			z(a_point.z),
			w(1)
		{};

		float&			operator[](std::size_t a_idx);
		const float&	operator[](std::size_t a_idx) const;
		Vector4			operator+(const Vector4& a_rhs) const;
		Vector4			operator-(const Vector4& a_rhs) const;
		Vector4&		operator-=(const Vector4& a_rhs);
		float			operator*(const Vector4& a_rhs) const;
		Vector4			operator*(const float a_scalar) const;
		Vector4&		operator*=(float a_scalar);
		Vector4&		operator/=(float a_scalar);
		Vector4			operator/(float a_scalar) const;
		Vector4			operator-() const;

		float Norm();
		RE::NiPoint3 ToNDC();
	};

	class Matrix4
	{
		public:
			float entry[4][4];

			constexpr Matrix4() noexcept
			{
				// Row 1
				entry[0][0] = 0.0f;
				entry[0][1] = 0.0f;
				entry[0][2] = 0.0f;
				entry[0][3] = 0.0f;

				// Row 2
				entry[1][0] = 0.0f;
				entry[1][1] = 0.0f;
				entry[1][2] = 0.0f;
				entry[1][3] = 0.0f;

				entry[2][0] = 0.0f;
				entry[2][1] = 0.0f;
				entry[2][2] = 0.0f;
				entry[2][3] = 0.0f;

				entry[3][0] = 0.0f;
				entry[3][1] = 0.0f;
				entry[3][2] = 0.0f;
				entry[3][3] = 0.0f;
			}

			constexpr Matrix4(float a_array[4][4]) noexcept
			{
				entry[0][0] = a_array[0][0];
				entry[0][1] = a_array[0][1];
				entry[0][2] = a_array[0][2];
				entry[0][3] = a_array[0][3];

				entry[1][0] = a_array[1][0];
				entry[1][1] = a_array[1][1];
				entry[1][2] = a_array[1][2];
				entry[1][3] = a_array[1][3];

				entry[2][0] = a_array[2][0];
				entry[2][1] = a_array[2][1];
				entry[2][2] = a_array[2][2];
				entry[2][3] = a_array[2][3];

				entry[3][0] = a_array[3][0];
				entry[3][1] = a_array[3][1];
				entry[3][2] = a_array[3][2];
				entry[3][3] = a_array[3][3];
			}

			float&			operator()(size_t a_row, size_t a_col);
			const float&	operator()(size_t a_row, size_t a_col) const;
			Vector4			operator()(size_t a_col);
			Vector4			operator*(const Vector4& a_vector) const;
			Matrix4			operator*(const Matrix4& a_matrix) const;

			Matrix4			T() const;

			void Print(const char* a_msg);

	};

	void PrintMatrix(const char* a_title, RE::NiMatrix3 a_matrix, int a_indent = 0);
	void PrintMatrix(const char* a_title, float a_Matrix4[4][4], int a_indent = 0);
	void PrintMatrix(const char* a_title, glm::mat4 a_Matrix4, int a_indent = 0);
	void PrintMatrix(const char* a_title, Matrix4 a_Matrix4, int a_indent = 0);
}