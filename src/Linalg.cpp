#include "Linalg.h"

namespace Linalg
{
	float& Vector4::operator[](std::size_t a_idx)
	{
		assert(a_idx < 4);
		return std::addressof(x)[a_idx];
	}

	const float& Vector4::operator[](std::size_t a_idx) const
	{
		assert(a_idx < 4);
		return std::addressof(x)[a_idx];
	}

	Vector4 Vector4::operator+(const Vector4& a_rhs) const
	{
		return Vector4( x+a_rhs.x, y+a_rhs.y, z+a_rhs.z, w+a_rhs.w);
	}

	Vector4 Vector4::operator-(const Vector4& a_rhs) const
	{
		return Vector4( x-a_rhs.x, y-a_rhs.y, z-a_rhs.z, w-a_rhs.w);
	}

	Vector4& Vector4::operator-=(const Vector4& a_rhs)
	{
		x -= a_rhs.x;
		y -= a_rhs.y;
		z -= a_rhs.z;
		w -= a_rhs.w;
		return *this;
	}

	float Vector4::operator*(const Vector4& a_rhs) const
	{
		return x*a_rhs.x + y*a_rhs.y + z*a_rhs.z + w*a_rhs.w;
	}

	Vector4 Vector4::operator*(const float a_scalar) const
	{
		return Vector4(x*a_scalar, y*a_scalar, z*a_scalar, w*a_scalar);
	}

	Vector4& Vector4::operator*=(float a_scalar)
	{
		x *= a_scalar;
		y *= a_scalar;
		z *= a_scalar;
		w *= a_scalar;
		return *this;
	}

	Vector4& Vector4::operator/=(float a_scalar)
	{
		x /= a_scalar;
		y /= a_scalar;
		z /= a_scalar;
		w /= a_scalar;
		return *this;
	}
	Vector4 Vector4::operator/(float a_scalar) const
	{
		return Vector4(x/a_scalar, y/a_scalar, z/a_scalar, w/a_scalar);
	}
	Vector4 Vector4::operator-() const
	{
		return Vector4(-x, -y, -z, -w);
	}

	float Vector4::Norm()
	{
		float x = this->x;
		float y = this->y;
		float z = this->z;
		float d = this->w;
		return sqrt(x*x + y*y + z*z + w*w);
	}

	RE::NiPoint3 Vector4::ToNDC()
	{
		return RE::NiPoint3(x/w, y/w, z/w);
	}

	// ------- MATRIX 4 ----------------------------------------------------------------------------------------

	float& Matrix4::operator()(size_t a_row, size_t a_col)
	{
		return entry[a_row][a_col];
	}

	const float& Matrix4::operator()(size_t a_row, size_t a_col) const
	{
		return entry[a_row][a_col];
	}


	Vector4  Matrix4::operator()(size_t a_col)
	{
		Vector4 vec;
		vec.x = this->entry[0][a_col];
		vec.y = this->entry[1][a_col];
		vec.z = this->entry[2][a_col];
		vec.w = this->entry[3][a_col];
		return vec;
	}

	Vector4 Matrix4::operator*(const Vector4& a_vector) const
	{
		return Vector4
		(
			a_vector.x*entry[0][0] + a_vector.y*entry[0][1] + a_vector.z*entry[0][2] + a_vector.w*entry[0][3],
			a_vector.x*entry[1][0] + a_vector.y*entry[1][1] + a_vector.z*entry[1][2] + a_vector.w*entry[1][3],
			a_vector.x*entry[2][0] + a_vector.y*entry[2][1] + a_vector.z*entry[2][2] + a_vector.w*entry[2][3],
			a_vector.x*entry[3][0] + a_vector.y*entry[3][1] + a_vector.z*entry[3][2] + a_vector.w*entry[3][3]
		);
	}

	Matrix4 Matrix4::operator*(const Matrix4& a_matrix) const
	{
		Matrix4 mat;
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				for (int k = 0; k < 4; k++)
				{
					mat(i, j) += (*this)(i, k) * a_matrix(k, j);
				}
			}
		}
		return mat;
	}

	Matrix4 Matrix4::T() const
	{
		Matrix4 transpose;
		transpose.entry[0][0] = this->entry[0][0];
		transpose.entry[0][1] = this->entry[1][0];
		transpose.entry[0][2] = this->entry[2][0];
		transpose.entry[0][3] = this->entry[3][0];
		transpose.entry[1][0] = this->entry[0][1];
		transpose.entry[1][1] = this->entry[1][1];
		transpose.entry[1][2] = this->entry[2][1];
		transpose.entry[1][3] = this->entry[3][1];
		transpose.entry[2][0] = this->entry[0][2];
		transpose.entry[2][1] = this->entry[1][2];
		transpose.entry[2][2] = this->entry[2][2];
		transpose.entry[2][3] = this->entry[3][2];
		transpose.entry[3][0] = this->entry[0][3];
		transpose.entry[3][1] = this->entry[1][3];
		transpose.entry[3][2] = this->entry[2][3];
		transpose.entry[3][3] = this->entry[3][3];

		return transpose;
	}
	Matrix4 Matrix4::Inverse(const Matrix4& Q, const Matrix4& R) const
	{
		int size = 4;
		Matrix4 inverse;
		for (int i = 0; i < size; i++)
		{
			Vector4 unitVector;
			for (int j = 0; j < size; j++) if(j == i) unitVector[j] = 1;

			Vector4 vec = Matrix4::solve(unitVector, Q, R);
			for (int k = 0; k < size; k++) inverse(k, i) = vec[k];
		}
		return inverse;
	}
	
	void Matrix4::QRDecomposition(Matrix4& Q, Matrix4& R) const
	{
		int size = 4;
		Q = *this;
		for (int i = 0; i < size; i++)
		{
			R(i,i) = Q(i).Norm();
			for (int j = 0; j < size; j++) Q(j, i) /= R(i,i);

			for (int j = i + 1; j < size; j++)
			{
				R(i,j) = Q(i) * Q(j);
				Vector4 subt = Q(i) * R(i,j);
				for(int k = 0; k < size; k++) Q(k, j) -= subt[k];
			}
		}
	}

	Vector4 Matrix4::solve(const Vector4& a_vector, const Matrix4& Q, const Matrix4& R)
	{
		int size = 4;

		Vector4 c = Q.T() * a_vector;
		Vector4 x;

		for (int i = size - 1; i >= 0; i--)
		{
			double sum = c[i];
			for (int j = 1; j < size; j++) sum -= R(i,j)*x[j];
			x[i] = sum/R(i,i);
		}
		return x;
	}

	void Matrix4::Print(const char* a_msg)
	{
		PrintMatrix(a_msg, *this);
	}


	void PrintMatrix(const char* a_title, RE::NiMatrix3 a_matrix)
	{
		logger::info("{}", a_title);
		logger::info("{:>6.2f} {:>6.2f} {:>6.2f}", a_matrix.entry[0][0], a_matrix.entry[0][1],a_matrix.entry[0][2]);
		logger::info("{:>6.2f} {:>6.2f} {:>6.2f}", a_matrix.entry[1][0], a_matrix.entry[1][1],a_matrix.entry[1][2]);
		logger::info("{:>6.2f} {:>6.2f} {:>6.2f}", a_matrix.entry[2][0], a_matrix.entry[2][1],a_matrix.entry[2][2]);

	}

	void  PrintMatrix(const char* a_title, float a_Matrix4[4][4])
	{
		logger::info("{}", a_title);
		logger::info("{:>19.9f} {:>19.9f} {:>19.9f} {:>19.9f}", a_Matrix4[0][0], a_Matrix4[0][1] ,a_Matrix4[0][2] ,a_Matrix4[0][3]);
		logger::info("{:>19.9f} {:>19.9f} {:>19.9f} {:>19.9f}", a_Matrix4[1][0], a_Matrix4[1][1] ,a_Matrix4[1][2] ,a_Matrix4[1][3]);
		logger::info("{:>19.9f} {:>19.9f} {:>19.9f} {:>19.9f}", a_Matrix4[2][0], a_Matrix4[2][1] ,a_Matrix4[2][2] ,a_Matrix4[2][3]);
		logger::info("{:>19.9f} {:>19.9f} {:>19.9f} {:>19.9f}", a_Matrix4[3][0], a_Matrix4[3][1] ,a_Matrix4[3][2] ,a_Matrix4[3][3]);
	}

	void  PrintMatrix(const char* a_title, Matrix4 a_Matrix4)
	{
		PrintMatrix(a_title, a_Matrix4.entry);
	}
}
