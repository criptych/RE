#pragma once

#include "MathUtil.h"

#include "Vector4.h"

#define MATRIX_COLUMN_MAJOR	1

struct Matrix4;

// matrix 4x4
// column major by default, can toggle MATRIX_COLUMN_MAJOR 
// comments are mostly based on column major version
__declspec(align(16)) struct Matrix4
{
public:
	union
	{
		float m[4][4];
		Vec128 mVec[4];
		// rows or columns based on which one we choose
		Vector4 mLine[4];
		struct
		{
			Vector4 mScaledAxisX;
			Vector4 mScaledAxisY;
			Vector4 mScaledAxisZ;
			Vector4 mTranslation;
		};
	};

	Matrix4() {};

	Matrix4(const Vector4& line0, const Vector4& line1, const Vector4& line2, const Vector4& line3)
	{
		mLine[0] = line0;
		mLine[1] = line1;
		mLine[2] = line2;
		mLine[3] = line3;
	}

	//Matrix4(Vec128 vec0, Vec128 vec1, Vec128 vec2, Vec128 vec3)
	//{
	//	mVec[0] = vec0;
	//	mVec[1] = vec1;
	//	mVec[2] = vec2;
	//	mVec[3] = vec3;
	//}
	
	// const getter
	static __forceinline Matrix4 Identity()
	{ 
		static Matrix4 identity(
			VecSet(1.f, 0.f, 0.f, 0.f),
			VecSet(0.f, 1.f, 0.f, 0.f),
			VecSet(0.f, 0.f, 1.f, 0.f),
			VecSet(0.f, 0.f, 0.f, 1.f)
			);
		return identity;
	}

	// add
	inline Matrix4 operator+(const Matrix4& m) const
	{
		Matrix4 r;
		r.mVec[0] = VecAdd(mVec[0], m.mVec[0]);
		r.mVec[1] = VecAdd(mVec[1], m.mVec[1]);
		r.mVec[2] = VecAdd(mVec[2], m.mVec[2]);
		r.mVec[3] = VecAdd(mVec[3], m.mVec[3]);
		return r;
	}
	inline Matrix4& operator+=(const Matrix4& m)
	{
		mVec[0] = VecAdd(mVec[0], m.mVec[0]);
		mVec[1] = VecAdd(mVec[1], m.mVec[1]);
		mVec[2] = VecAdd(mVec[2], m.mVec[2]);
		mVec[3] = VecAdd(mVec[3], m.mVec[3]);
		return *this;
	}

	inline Matrix4& operator+=(float f)
	{
		Vec128 t = VecSet1(f);
		mVec[0] = VecAdd(mVec[0], t);
		mVec[1] = VecAdd(mVec[1], t);
		mVec[2] = VecAdd(mVec[2], t);
		mVec[3] = VecAdd(mVec[3], t);
		return *this;
	}

	// sub
	inline Matrix4 operator-(const Matrix4& m) const
	{
		Matrix4 r;
		r.mVec[0] = VecSub(mVec[0], m.mVec[0]);
		r.mVec[1] = VecSub(mVec[1], m.mVec[1]);
		r.mVec[2] = VecSub(mVec[2], m.mVec[2]);
		r.mVec[3] = VecSub(mVec[3], m.mVec[3]);
		return r;
	}
	inline Matrix4& operator-=(const Matrix4& m)
	{
		mVec[0] = VecSub(mVec[0], m.mVec[0]);
		mVec[1] = VecSub(mVec[1], m.mVec[1]);
		mVec[2] = VecSub(mVec[2], m.mVec[2]);
		mVec[3] = VecSub(mVec[3], m.mVec[3]);
		return *this;
	}

	// mul scalar
	inline Matrix4 operator*(float f) const
	{
		Matrix4 r;
		Vec128 t = VecSet1(f);
		r.mVec[0] = VecMul(mVec[0], t);
		r.mVec[1] = VecMul(mVec[1], t);
		r.mVec[2] = VecMul(mVec[2], t);
		r.mVec[3] = VecMul(mVec[3], t);
		return r;
	}
	inline Matrix4& operator*=(float f)
	{
		Vec128 t = VecSet1(f);
		mVec[0] = VecMul(mVec[0], t);
		mVec[1] = VecMul(mVec[1], t);
		mVec[2] = VecMul(mVec[2], t);
		mVec[3] = VecMul(mVec[3], t);
		return *this;
	}

	// div scalar
	inline Matrix4 operator/(float f) const
	{
		Matrix4 r;
		Vec128 t = VecSet1(f);
		r.mVec[0] = VecDiv(mVec[0], t);
		r.mVec[1] = VecDiv(mVec[1], t);
		r.mVec[2] = VecDiv(mVec[2], t);
		r.mVec[3] = VecDiv(mVec[3], t);
		return r;
	}
	inline Matrix4& operator/=(float f)
	{
		Vec128 t = VecSet1(f);
		mVec[0] = VecDiv(mVec[0], t);
		mVec[1] = VecDiv(mVec[1], t);
		mVec[2] = VecDiv(mVec[2], t);
		mVec[3] = VecDiv(mVec[3], t);
		return *this;
	}

	// mul vector
	inline Vector4 operator*(const Vector4& v) const
	{
		Vec128 r;
		r =				VecMul(mVec[0], VecSwizzle1(v.mVec,0));
		r = VecAdd(r,	VecMul(mVec[1], VecSwizzle1(v.mVec,1)));
		r = VecAdd(r,	VecMul(mVec[2], VecSwizzle1(v.mVec,2)));
		r = VecAdd(r,	VecMul(mVec[3], VecSwizzle1(v.mVec,3)));
		return r;
	}

	// mul matrix
	inline Matrix4 operator*(const Matrix4& m) const
	{
#if MATRIX_COLUMN_MAJOR
		const Vec128* vecA = mVec;
		const Vec128* vecB = m.mVec;
#else
		const Vec128* vecA = m.mVec;
		const Vec128* vecB = mVec;
#endif

		Matrix4 r;
		// line 0
		r.mVec[0] =						VecMul(vecA[0], VecSwizzle1(vecB[0], 0));
		r.mVec[0] = VecAdd(r.mVec[0],	VecMul(vecA[1], VecSwizzle1(vecB[0], 1)));
		r.mVec[0] = VecAdd(r.mVec[0],	VecMul(vecA[2], VecSwizzle1(vecB[0], 2)));
		r.mVec[0] = VecAdd(r.mVec[0],	VecMul(vecA[3], VecSwizzle1(vecB[0], 3)));
		// line 1
		r.mVec[1] =						VecMul(vecA[0], VecSwizzle1(vecB[1], 0));
		r.mVec[1] = VecAdd(r.mVec[1],	VecMul(vecA[1], VecSwizzle1(vecB[1], 1)));
		r.mVec[1] = VecAdd(r.mVec[1],	VecMul(vecA[2], VecSwizzle1(vecB[1], 2)));
		r.mVec[1] = VecAdd(r.mVec[1],	VecMul(vecA[3], VecSwizzle1(vecB[1], 3)));
		// line 2
		r.mVec[2] =						VecMul(vecA[0], VecSwizzle1(vecB[2], 0));
		r.mVec[2] = VecAdd(r.mVec[2],	VecMul(vecA[1], VecSwizzle1(vecB[2], 1)));
		r.mVec[2] = VecAdd(r.mVec[2],	VecMul(vecA[2], VecSwizzle1(vecB[2], 2)));
		r.mVec[2] = VecAdd(r.mVec[2],	VecMul(vecA[3], VecSwizzle1(vecB[2], 3)));
		// line 3
		r.mVec[3] =						VecMul(vecA[0], VecSwizzle1(vecB[3], 0));
		r.mVec[3] = VecAdd(r.mVec[3],	VecMul(vecA[1], VecSwizzle1(vecB[3], 1)));
		r.mVec[3] = VecAdd(r.mVec[3],	VecMul(vecA[2], VecSwizzle1(vecB[3], 2)));
		r.mVec[3] = VecAdd(r.mVec[3],	VecMul(vecA[3], VecSwizzle1(vecB[3], 3)));
		return r;
	}

	// compose two matrix A,B as transform by A then B.
	// for column major returns B*A
	// for row major return A*B
	inline Matrix4 Compose(const Matrix4& m) const
	{
#if MATRIX_COLUMN_MAJOR
		return m * (*this);
#else
		return (*this) * m;
#endif
	}

	// compose two matrix A,B as transform by A then B.
	// for column major returns B*A
	// for row major return A*B
	static inline Matrix4 Compose(const Matrix4& m1, const Matrix4& m2)	{ return m1.Compose(m2); }

	inline Matrix4 GetTransposed() const
	{
		// from _MM_TRANSPOSE4_PS()
		Matrix4 r;

		Vec128 t0 = VecShuffle_0101(mVec[0], mVec[1]); // 00, 01, 10, 11
		Vec128 t1 = VecShuffle_0101(mVec[2], mVec[3]); // 20, 21, 30, 31
		Vec128 t2 = VecShuffle_2323(mVec[0], mVec[1]); // 02, 03, 12, 13
		Vec128 t3 = VecShuffle_2323(mVec[2], mVec[3]); // 22, 23, 32, 33

		r.mVec[0] = VecShuffle(t0, t1, 0, 2, 0, 2); // 00, 10, 20, 30
		r.mVec[1] = VecShuffle(t0, t1, 1, 3, 1, 3); // 01, 11, 21, 31
		r.mVec[2] = VecShuffle(t2, t3, 0, 2, 0, 2); // 02, 12, 22, 32
		r.mVec[3] = VecShuffle(t2, t3, 1, 3, 1, 3); // 03, 13, 23, 33

		return r;		
	}

	// inv(A) = Transpose(inv(Transpose(A)))
	// Inverse function is the same no matter column major or row major
	// this version treats it as column major
	__forceinline Matrix4 GetInverse() const
	{
		// use block matrix method
		// A is a matrix, then i(A) or iA means inverse of A, A# (or A_ in code) means adjugate of A, |A| is determinant, tr(A) is trace
		// some properties: iA = A# / |A|, (AB)# = B#A#, (cA)# = c^(n-1)A#, |AB| = |A||B|, |cA| = c^n|A|, tr(AB) = tr(BA)
		// for 2x2 matrix: (A#)# = A, (cA)# = cA#, |-A| = |A|, |A+B| = |A| + |B| + tr(A#B), tr(A) = A0*A3 - A1*A2
		// M is 4x4 matrix, we divide into 4 2x2 matrices
		// let M = | A  B |, then iM = | i(A-BiDC)        -iABi(D-CiAB) | = 1/|M| * | (|D|A - B(D#C))#    (|B|C - D(A#B)#)# |
		//         | C  D |            | -iDCi(A-BiDC)    i(D-CiAB)     |           | (|C|B - A(D#C)#)#   (|A|D - C(A#B))#  |
		// we have property: |M| = |A|*|D-CiAB| = |D|*|A-BiDC| = |AD-BC| = |A||D| + |B||C| - tr((A#B)(D#C))
		// we use __m128 to represent sub 2x2 column major matrix as A = | A0  A2 |
		//                                                               | A1  A3 |
				
		// sub matrices
		Vec128 A = VecShuffle_0101(mVec[0], mVec[1]);
		Vec128 C = VecShuffle_2323(mVec[0], mVec[1]);
		Vec128 B = VecShuffle_0101(mVec[2], mVec[3]);
		Vec128 D = VecShuffle_2323(mVec[2], mVec[3]);

		Vec128 detA = VecSet1(m[0][0] * m[1][1] - m[0][1] * m[1][0]);
		Vec128 detC = VecSet1(m[0][2] * m[1][3] - m[0][3] * m[1][2]);
		Vec128 detB = VecSet1(m[2][0] * m[3][1] - m[2][1] * m[3][0]);
		Vec128 detD = VecSet1(m[2][2] * m[3][3] - m[2][3] * m[3][2]);

#if 0 // for determinant, float version is faster
		// determinant as (|A| |C| |B| |D|)
		Vec128 detSub = VecSub(
			VecMul(VecShuffle(mVec[0], mVec[2], 0,2,0,2), VecShuffle(mVec[1], mVec[3], 1,3,1,3)),
			VecMul(VecShuffle(mVec[0], mVec[2], 1,3,1,3), VecShuffle(mVec[1], mVec[3], 0,2,0,2))
			);
		Vec128 detA = VecSwizzle1(detSub, 0);
		Vec128 detC = VecSwizzle1(detSub, 1);
		Vec128 detB = VecSwizzle1(detSub, 2);
		Vec128 detD = VecSwizzle1(detSub, 3);
#endif

		// let iM = 1/|M| * | X  Y |
		//                  | Z  W |

		// D#C
		Vec128 D_C = Mat2AdjMul_CM(D, C);
		// A#B
		Vec128 A_B = Mat2AdjMul_CM(A, B);
		// X# = |D|A - B(D#C)
		Vec128 X_ = VecSub(VecMul(detD, A), Mat2Mul_CM(B, D_C));
		// W# = |A|D - C(A#B)
		Vec128 W_ = VecSub(VecMul(detA, D), Mat2Mul_CM(C, A_B));

		// |M| = |A|*|D| + ... (continue later)
		Vec128 detM = VecMul(detA, detD);

		// Y# = |B|C - D(A#B)#
		Vec128 Y_ = VecSub(VecMul(detB, C), Mat2MulAdj_CM(D, A_B));
		// Z# = |C|B - A(D#C)#
		Vec128 Z_ = VecSub(VecMul(detC, B), Mat2MulAdj_CM(A, D_C));

		// |M| = |A|*|D| + |B|*|C| ... (continue later)
		detM = VecAdd(detM, VecMul(detB, detC));

		// tr((A#B)(D#C))
		Vec128 tr = VecDotV(A_B, VecSwizzle(D_C, 0,2,1,3));
		// |M| = |A|*|D| + |B|*|C| - tr((A#B)(D#C))
		detM = VecSub(detM, tr);

		const Vec128 adjSignMask = VecSet(1.f, -1.f, -1.f, 1.f);
		// (1/|M|, -1/|M|, -1/|M|, 1/|M|)
		Vec128 rDetM = VecDiv(adjSignMask, detM);

		X_ = VecMul(X_, rDetM);
		Y_ = VecMul(Y_, rDetM);
		Z_ = VecMul(Z_, rDetM);
		W_ = VecMul(W_, rDetM);

		Matrix4 r;

		// apply adjugate and store, here we combine adjugate shuffle and store shuffle
		// btw adjuagate fuction: Adj(Vec) = VecMul(VecSwizzle(Vec, 3,1,2,0), adjSignMask)
		r.mVec[0] = VecShuffle(X_, Z_, 3,1,3,1);
		r.mVec[1] = VecShuffle(X_, Z_, 2,0,2,0);
		r.mVec[2] = VecShuffle(Y_, W_, 3,1,3,1);
		r.mVec[3] = VecShuffle(Y_, W_, 2,0,2,0);

		return r;
	}
	
	// this row major version GetInverse is just for comparison
#if 0
	// inv(A) = Transpose(inv(Transpose(A)))
	// Inverse function is the same no matter column major or row major
	// this version treats it as row major
	__forceinline Matrix4 GetInverse() const
	{
		// use block matrix method
		// A is a matrix, then i(A) or iA means inverse of A, A# (or A_ in code) means adjugate of A, |A| is determinant, tr(A) is trace
		// some properties: iA = A# / |A|, (AB)# = B#A#, (cA)# = c^(n-1)A#, |AB| = |A||B|, |cA| = c^n|A|, tr(AB) = tr(BA)
		// for 2x2 matrix: (A#)# = A, (cA)# = cA#, |-A| = |A|, |A+B| = |A| + |B| + tr(A#B), tr(A) = A0*A3 - A1*A2
		// M is 4x4 matrix, we divide into 4 2x2 matrices
		// let M = | A  B |, then iM = | i(A-BiDC)        -iABi(D-CiAB) | = 1/|M| * | (|D|A - B(D#C))#    (|B|C - D(A#B)#)# |
		//         | C  D |            | -iDCi(A-BiDC)    i(D-CiAB)     |           | (|C|B - A(D#C)#)#   (|A|D - C(A#B))#  |
		// we have property: |M| = |A|*|D-CiAB| = |D|*|A-BiDC| = |AD-BC| = |A||D| + |B||C| - tr((A#B)(D#C))
		// we use __m128 to represent sub 2x2 column major matrix as A = | A0  A1 |
		//                                                               | A2  A3 |

		// sub matrices
		Vec128 A = VecShuffle_0101(mVec[0], mVec[1]);
		Vec128 B = VecShuffle_2323(mVec[0], mVec[1]);
		Vec128 C = VecShuffle_0101(mVec[2], mVec[3]);
		Vec128 D = VecShuffle_2323(mVec[2], mVec[3]);

		Vec128 detA = VecSet1(m[0][0] * m[1][1] - m[0][1] * m[1][0]);
		Vec128 detB = VecSet1(m[0][2] * m[1][3] - m[0][3] * m[1][2]);
		Vec128 detC = VecSet1(m[2][0] * m[3][1] - m[2][1] * m[3][0]);
		Vec128 detD = VecSet1(m[2][2] * m[3][3] - m[2][3] * m[3][2]);

#if 0 // for determinant, float version is faster
		// determinant as (|A| |B| |C| |D|)
		Vec128 detSub = VecSub(
			VecMul(VecShuffle(mVec[0], mVec[2], 0, 2, 0, 2), VecShuffle(mVec[1], mVec[3], 1, 3, 1, 3)),
			VecMul(VecShuffle(mVec[0], mVec[2], 1, 3, 1, 3), VecShuffle(mVec[1], mVec[3], 0, 2, 0, 2))
		);
		Vec128 detA = VecSwizzle1(detSub, 0);
		Vec128 detB = VecSwizzle1(detSub, 1);
		Vec128 detC = VecSwizzle1(detSub, 2);
		Vec128 detD = VecSwizzle1(detSub, 3);
#endif

		// let iM = 1/|M| * | X  Y |
		//                  | Z  W |

		// D#C
		Vec128 D_C = Mat2AdjMul_RM(D, C);
		// A#B
		Vec128 A_B = Mat2AdjMul_RM(A, B);
		// X# = |D|A - B(D#C)
		Vec128 X_ = VecSub(VecMul(detD, A), Mat2Mul_RM(B, D_C));
		// W# = |A|D - C(A#B)
		Vec128 W_ = VecSub(VecMul(detA, D), Mat2Mul_RM(C, A_B));

		// |M| = |A|*|D| + ... (continue later)
		Vec128 detM = VecMul(detA, detD);

		// Y# = |B|C - D(A#B)#
		Vec128 Y_ = VecSub(VecMul(detB, C), Mat2MulAdj_RM(D, A_B));
		// Z# = |C|B - A(D#C)#
		Vec128 Z_ = VecSub(VecMul(detC, B), Mat2MulAdj_RM(A, D_C));

		// |M| = |A|*|D| + |B|*|C| ... (continue later)
		detM = VecAdd(detM, VecMul(detB, detC));

		// tr((A#B)(D#C))
		Vec128 tr = VecDotV(A_B, VecSwizzle(D_C, 0, 2, 1, 3));
		// |M| = |A|*|D| + |B|*|C| - tr((A#B)(D#C))
		detM = VecSub(detM, tr);

		const Vec128 adjSignMask = VecSet(1.f, -1.f, -1.f, 1.f);
		// (1/|M|, -1/|M|, -1/|M|, 1/|M|)
		Vec128 rDetM = VecDiv(adjSignMask, detM);

		X_ = VecMul(X_, rDetM);
		Y_ = VecMul(Y_, rDetM);
		Z_ = VecMul(Z_, rDetM);
		W_ = VecMul(W_, rDetM);

		Matrix4 r;

		// apply adjugate and store, here we combine adjugate shuffle and store shuffle
		// btw adjuagate fuction: Adj(Vec) = VecMul(VecSwizzle(Vec, 3,1,2,0), adjSignMask)
		r.mVec[0] = VecShuffle(X_, Y_, 3, 1, 3, 1);
		r.mVec[1] = VecShuffle(X_, Y_, 2, 0, 2, 0);
		r.mVec[2] = VecShuffle(Z_, W_, 3, 1, 3, 1);
		r.mVec[3] = VecShuffle(Z_, W_, 2, 0, 2, 0);

		return r;
	}
#endif

	// returns (00, 10, 20, 23), (01, 11, 21, 23), (02, 12, 22, 23)
	__forceinline void Internal_GetTransposed3(Vec128& outTLine0, Vec128& outTLine1, Vec128& outTLine2) const
	{
		Vec128 t0 = VecShuffle_0101(mVec[0], mVec[1]); // 00, 01, 10, 11
		Vec128 t1 = VecShuffle_2323(mVec[0], mVec[1]); // 02, 03, 12, 13
		outTLine0 = VecShuffle(t0, mVec[2], 0,2,0,3); // 00, 10, 20, 23
		outTLine1 = VecShuffle(t0, mVec[2], 1,3,1,3); // 01, 11, 21, 23
		outTLine2 = VecShuffle(t1, mVec[2], 0,2,2,3); // 02, 12, 22, 23
	}

	// if this matrix is a tranform matrix as scaled axes and translation
	// | aX bY cZ T |
	// | 0  0  0  1 |
	// returns its inverse
	// | X/a  -dot(T,X)/a |
	// | Y/b  -dot(T,Y)/b |
	// | Z/c  -dot(T,Z)/c |
	// |  0      1        |
	// Requires this matrix to be transform matrix, NoScale version requires this matrix be of scale 1
	__forceinline Matrix4 GetTransformInverseNoScale() const
	{
		Matrix4 r;

		// transpose 3x3, we know m03 = m13 = m23 = 0
		Internal_GetTransposed3(r.mVec[0], r.mVec[1], r.mVec[2]);

		// last line
		r.mVec[3] =						VecMul(r.mVec[0], VecSwizzle1(mVec[3], 0));
		r.mVec[3] = VecAdd(r.mVec[3],	VecMul(r.mVec[1], VecSwizzle1(mVec[3], 1)));
		r.mVec[3] = VecAdd(r.mVec[3],	VecMul(r.mVec[2], VecSwizzle1(mVec[3], 2)));
		r.mVec[3] = VecSub(VecSet(0.f, 0.f, 0.f, 1.f), r.mVec[3]);

		return r;
	}
	// Requires this matrix to be transform matrix
	__forceinline Matrix4 GetTransformInverse() const
	{
		Matrix4 r;

		// transpose 3x3, we know m03 = m13 = m23 = 0
		Internal_GetTransposed3(r.mVec[0], r.mVec[1], r.mVec[2]);

		 // (mLine[0].SizeSqr3(), mLine[1].SizeSqr3(), mLine[2].SizeSqr3(), 0)
		Vec128 sizeSqr; 
		sizeSqr =					VecMul(r.mVec[0], r.mVec[0]);
		sizeSqr = VecAdd(sizeSqr,	VecMul(r.mVec[1], r.mVec[1]));
		sizeSqr = VecAdd(sizeSqr,	VecMul(r.mVec[2], r.mVec[2]));

		// optional test to avoid divide by 0
		// if(sizeSqr < SMALL_NUMBER) sizeSqr = 1;
		Vec128 rSizeSqr = VecBlendVar(
			VecDiv(VecConst::Vec_One, sizeSqr),
			VecConst::Vec_One,
			VecCmpLT(sizeSqr, VecConst::Vec_Small_Num)
			);

		r.mVec[0] = VecMul(r.mVec[0], rSizeSqr);
		r.mVec[1] = VecMul(r.mVec[1], rSizeSqr);
		r.mVec[2] = VecMul(r.mVec[2], rSizeSqr);

		// last line
		r.mVec[3] =						VecMul(r.mVec[0], VecSwizzle1(mVec[3], 0));
		r.mVec[3] = VecAdd(r.mVec[3],	VecMul(r.mVec[1], VecSwizzle1(mVec[3], 1)));
		r.mVec[3] = VecAdd(r.mVec[3],	VecMul(r.mVec[2], VecSwizzle1(mVec[3], 2)));
		r.mVec[3] = VecSub(VecSet(0.f, 0.f, 0.f, 1.f), r.mVec[3]);

		return r;
	}

#if 0 // no need for calculating normal matrix	
	__forceinline Matrix4 GetInverseTransposed3() const
	{
		Matrix4 r;

		r.mVec[0] = VecCross(mVec[1], mVec[2]);
		Vec128 det = VecDot3V(mVec[0], r.mVec[0]);

		r.mVec[1] = VecCross(mVec[2], mVec[0]);
		r.mVec[2] = VecCross(mVec[0], mVec[1]);

		Vec128 rDet = VecDiv(VecConst::Vec_One, det);

		r.mVec[0] = VecMul(r.mVec[0], rDet);
		r.mVec[1] = VecMul(r.mVec[1], rDet);
		r.mVec[2] = VecMul(r.mVec[2], rDet);
			
		r.mVec[3] = VecSet(0.f, 0.f, 0.f, 1.f);
		return r;
	}
	// Requires this matrix to be transform matrix
	__forceinline Matrix4 GetTransformInverseTransposed3() const
	{
		Matrix4 r;

		// transpose 3x3, we know m03 = m13 = m23 = 0
		Vec128 tLine0, tLine1, tLine2;
		Internal_GetTransposed3(tLine0, tLine1, tLine2);

		// (mLine[0].SizeSqr3(), mLine[1].SizeSqr3(), mLine[2].SizeSqr3(), 0)
		Vec128 sizeSqr;
		sizeSqr =					VecMul(tLine0, tLine0);
		sizeSqr = VecAdd(sizeSqr,	VecMul(tLine1, tLine1));
		sizeSqr = VecAdd(sizeSqr,	VecMul(tLine2, tLine2));
		
		// optional test to avoid divide by 0
		// if(sizeSqr < SMALL_NUMBER) sizeSqr = 1;
		Vec128 rSizeSqr = VecBlendVar(
			VecDiv(VecConst::Vec_One, sizeSqr),
			VecConst::Vec_One,
			VecCmpLT(sizeSqr, VecConst::Vec_Small_Num)
			);

		r.mVec[0] = VecMul(mVec[0], rSizeSqr);
		r.mVec[1] = VecMul(mVec[1], rSizeSqr);
		r.mVec[2] = VecMul(mVec[2], rSizeSqr);

		r.mVec[3] = VecSet(0.f, 0.f, 0.f, 1.f);
		return r;
	}
#endif

	// return value W = 0 (if the matrix is well-defined transform matrix)
	inline Vector4_3 TransformVector(const Vector4_3& v) const
	{
		Vec128 r;
		r =				VecMul(mVec[0], VecSwizzle1(v.mVec, 0));
		r = VecAdd(r,	VecMul(mVec[1], VecSwizzle1(v.mVec, 1)));
		r = VecAdd(r,	VecMul(mVec[2], VecSwizzle1(v.mVec, 2)));
		return r;
	}
	// return value W = 1 (if the matrix is well-defined transform matrix)
	inline Vector4_3 TransformPoint(const Vector4_3& v) const
	{
		Vec128 r;
		r = VecAdd(mVec[3], VecMul(mVec[0], VecSwizzle1(v.mVec, 0)));
		r = VecAdd(r,		VecMul(mVec[1], VecSwizzle1(v.mVec, 1)));
		r = VecAdd(r,		VecMul(mVec[2], VecSwizzle1(v.mVec, 2)));
		return r;
	}

	// inverse transform
	// if this matrix is a tranform matrix as scaled axes and translation
	// | aX bY cZ T |
	// | 0  0  0  1 |
	// inverse tranform of vector vector4_3 v would be
	// (dot3(v, X)/a, dot3(v, Y)/b, dot3(v, Z)/c, 0)
	// inverse tranform of point vector4_3 v would be
	// (dot3(v-T, X)/a, dot3(v-T, Y)/b, dot3(v-T, Z)/c, 0)

	// return value W = 0
	// Requires this matrix to be transform matrix, NoScale version requires this matrix be of scale 1
	__forceinline Vector4_3 InverseTransformVectorNoScale(const Vector4_3& v) const
	{
		// transpose 3x3, we know m03 = m13 = m23 = 0
		Vec128 tLine0, tLine1, tLine2;
		Internal_GetTransposed3(tLine0, tLine1, tLine2);

		// (dot3(v, mLine[0]), dot3(v, mLine[1]), dot3(v, mLine[2]), 0)
		Vec128 r;
		r =				VecMul(tLine0, VecSwizzle1(v.mVec, 0));
		r = VecAdd(r,	VecMul(tLine1, VecSwizzle1(v.mVec, 1)));
		r = VecAdd(r,	VecMul(tLine2, VecSwizzle1(v.mVec, 2)));

		return r;
	}
	// return value W = 0
	// Requires this matrix to be transform matrix
	__forceinline Vector4_3 InverseTransformVector(const Vector4_3& v) const
	{
		// transpose 3x3, we know m03 = m13 = m23 = 0
		Vec128 tLine0, tLine1, tLine2;
		Internal_GetTransposed3(tLine0, tLine1, tLine2);

		// (mLine[0].SizeSqr3(), mLine[1].SizeSqr3(), mLine[2].SizeSqr3(), 0)
		Vec128 sizeSqr;
		sizeSqr =					VecMul(tLine0, tLine0);
		sizeSqr = VecAdd(sizeSqr,	VecMul(tLine1, tLine1));
		sizeSqr = VecAdd(sizeSqr,	VecMul(tLine2, tLine2));

		// optional test to avoid divide by 0
		// if(sizeSqr < SMALL_NUMBER) sizeSqr = 1;
		sizeSqr = VecBlendVar(
			sizeSqr,
			VecConst::Vec_One,
			VecCmpLT(sizeSqr, VecConst::Vec_Small_Num)
			);

		// (dot3(v, mLine[0]), dot3(v, mLine[1]), dot3(v, mLine[2]), 0)
		Vec128 r;
		r =				VecMul(tLine0, VecSwizzle1(v.mVec, 0));
		r = VecAdd(r,	VecMul(tLine1, VecSwizzle1(v.mVec, 1)));
		r = VecAdd(r,	VecMul(tLine2, VecSwizzle1(v.mVec, 2)));
		return VecDiv(r, sizeSqr);
	}
	// return value W = 0
	// NoScale version requires this matrix be of scale 1
	inline Vector4_3 InverseTransformPointNoScale(const Vector4_3& v) const
	{
		return InverseTransformVectorNoScale(v - mLine[3]);
	}
	// return value W = 0
	inline Vector4_3 InverseTransformPoint(const Vector4_3& v) const
	{
		return InverseTransformVector(v - mLine[3]);
	}

	inline float GetDeterminant3() const
	{
		return VecDot3(mVec[0], VecCross(mVec[1], mVec[2]));

#if 0
		return
			+ m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1])
			- m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0])
			+ m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);
#endif
	}

	// det(A) = det(Transpose(A))
	// Determinant is the same no matter column major or row major
	// this version treats it as column major
	inline float GetDeterminant() const
	{
		// sub matrices
		Vec128 A = VecShuffle_0101(mVec[0], mVec[1]);
		Vec128 C = VecShuffle_2323(mVec[0], mVec[1]);
		Vec128 B = VecShuffle_0101(mVec[2], mVec[3]);
		Vec128 D = VecShuffle_2323(mVec[2], mVec[3]);

		float detA = (m[0][0] * m[1][1] - m[0][1] * m[1][0]);
		float detC = (m[0][2] * m[1][3] - m[0][3] * m[1][2]);
		float detB = (m[2][0] * m[3][1] - m[2][1] * m[3][0]);
		float detD = (m[2][2] * m[3][3] - m[2][3] * m[3][2]);

		// D#C
		Vec128 D_C = Mat2AdjMul_CM(D, C);
		// A#B
		Vec128 A_B = Mat2AdjMul_CM(A, B);

		// |M| = |A|*|D| + |B|*|C| - tr((A#B)(D#C)
		return detA * detD + detB * detC - VecDot(A_B, VecSwizzle(D_C, 0, 2, 1, 3));
	}

#if 0 // direct SIMD version, slower than float version
	inline float GetDeterminant() const
	{
		Vec128 a = VecMul(	VecSwizzle(mVec[1], 1,2,3,3),
			VecCross(		VecSwizzle(mVec[2], 1,2,3,3), VecSwizzle(mVec[3], 1,2,3,3)));

		Vec128 b = VecMul(	VecSwizzle(mVec[1], 0,2,3,3),
			VecCross(		VecSwizzle(mVec[3], 0,2,3,3), VecSwizzle(mVec[2], 0,2,3,3)));

		Vec128 c = VecMul(	VecSwizzle(mVec[1], 0,1,3,3),
			VecCross(		VecSwizzle(mVec[2], 0,1,3,3), VecSwizzle(mVec[3], 0,1,3,3)));
		
		Vec128 d = VecMul( mVec[1],
			VecCross(		mVec[3], mVec[2]));

		Vec128 ab_0101 = VecShuffle_0101(a, b);
		Vec128 ab_2323 = VecShuffle_2323(a, b);
		Vec128 cd_0101 = VecShuffle_0101(c, d);
		Vec128 cd_2323 = VecShuffle_2323(c, d);

		Vec128 v = VecShuffle(ab_0101, cd_0101, 0,2,0,2);
		v = VecAdd(VecShuffle(ab_0101, cd_0101, 1,3,1,3), v);
		v = VecAdd(VecShuffle(ab_2323, cd_2323, 0,2,0,2), v);

		v = VecMul(v, mVec[0]);
		Vec128 shuf = VecSwizzle_1133(v); // 1, 1, 3, 3
		Vec128 sums = VecAdd(v, shuf); // 0+1, 1+1, 2+3, 3+3
		shuf = VecShuffle_2323(sums, shuf); // 2+3, 3+3, 3, 3
		sums = VecAdd(sums, shuf); //0+1+2+3, x, x, x, 

		return VecToFloat(sums);
	}
#endif


};
