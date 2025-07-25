/**
*  Copyright (C) 2016 3D Repo Ltd
*
*  This program is free software: you can redistribute it and/or modify
*  it under the terms of the GNU Affero General Public License as
*  published by the Free Software Foundation, either version 3 of the
*  License, or (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU Affero General Public License for more details.
*
*  You should have received a copy of the GNU Affero General Public License
*  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <cstdlib>
#include <repo/lib/datastructure/repo_matrix.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <gtest/gtest-matchers.h>

#include "../../repo_test_utils.h"
#include "../../repo_test_matchers.h"

using namespace repo::lib;
using namespace testing;

#define RAD(x) ((float)x*(3.14159274101257324219 / 180))

bool checkIsIdentity(const RepoMatrix &mat)
{
	std::vector <float> identity = {1,0,0,0,
									0,1,0,0,
									0,0,1,0,
									0,0,0,1};
	return compareStdVectors(identity, mat.getData());
}

TEST(RepoMatrixTest, constructorTest)
{
	std::vector<float> sourceMat1;
	std::vector<std::vector<float>> sourceMat2;
	std::vector<double> sourceMat3;
	RepoMatrix matrix;
	RepoMatrix matrix2(sourceMat1);
	RepoMatrix matrix3(sourceMat2);
	RepoMatrix matrix4(sourceMat3);

	for (int i = 0; i < 16; ++i)
	{
		sourceMat1.push_back((rand()%1000) / 1000.f);
		if (i % 4 == 0) sourceMat2.push_back(std::vector<float>());
		sourceMat2.back().push_back((rand() % 1000) / 1000.f);
		sourceMat3.push_back((rand() % 1000) / 1000.f);
	}

	RepoMatrix matrix7(sourceMat1), matrix5(sourceMat2), matrix6(sourceMat3);

	std::vector<double> sourceMat64 = {
	0.41611923158633757, 0.41192361684200907, 0.3802399115383849, 1383.5544861408555,
	0.3102479090644362, 0.8211657559760365, 0.42445244930658144, 6159.647077873367,
	0.4608093818203498, 0.18089090705175348, 0.9258387270989096, 7714.581019037681,
	0, 0, 0, 1
	};

	std::vector<float> sourceMat32;
	sourceMat32.resize(16);
	for (auto i = 0; i < 16; i++)
	{
		sourceMat32[i] = (double)sourceMat64[i];
	}

	RepoMatrix singleFromSingle(sourceMat32.data());
	RepoMatrix singleFromDouble(sourceMat64.data());
	RepoMatrix64 doubleFromSingle(sourceMat32.data());
	RepoMatrix64 doubleFromDouble(sourceMat64.data());

	EXPECT_THAT(singleFromSingle.isIdentity(), IsFalse());
	EXPECT_THAT(singleFromDouble.isIdentity(), IsFalse());
	EXPECT_THAT(doubleFromSingle.isIdentity(), IsFalse());
	EXPECT_THAT(doubleFromDouble.isIdentity(), IsFalse());

	EXPECT_THAT(singleFromSingle.getData(), ElementsAreArray(sourceMat32));
	EXPECT_THAT(singleFromDouble.getData(), ElementsAreArray(sourceMat32));

	EXPECT_THAT(doubleFromSingle.getData(), ElementsAreArray(sourceMat32));
	EXPECT_THAT(doubleFromDouble.getData(), ElementsAreArray(sourceMat64));

	RepoMatrix64 colMajor(sourceMat64.data(), false);
	EXPECT_THAT(colMajor, Eq(doubleFromDouble.transpose()));
}

TEST(RepoMatrixTest, determinantTest)
{
	RepoMatrix id;
	EXPECT_EQ(1, id.determinant());
	std::vector<float> matValues = { 2, 0.3f,   0.4f, 1.23f,
		0.45f,    1, 0.488f, 12345,
		0,   0,     3.5f,     0,
		0,   0,       0,     1
	};
	RepoMatrix rand(matValues);

	EXPECT_EQ(6.5275f, rand.determinant());
}


TEST(RepoMatrixTest, equalsTest)
{
	RepoMatrix id, id2;

	EXPECT_TRUE(id.equals(id2));
	EXPECT_TRUE(id2.equals(id));
	EXPECT_TRUE(id.equals(id));

	std::vector<float> ranV, ranV2;
	for (int i = 0; i < 16; ++i)
	{
		ranV.push_back((rand() % 1000) / 1000.f);
		ranV2.push_back((rand() % 1000) / 1000.f);
	}

	RepoMatrix rand1(ranV), rand2(ranV2), rand3(ranV);
	EXPECT_TRUE(rand1.equals(rand1));
	EXPECT_TRUE(rand1.equals(rand3));
	EXPECT_TRUE(rand3.equals(rand1));
	EXPECT_FALSE(id.equals(rand1));
	EXPECT_FALSE(rand2.equals(rand1));
	EXPECT_FALSE(rand1.equals(rand2));

}

TEST(RepoMatrixTest, getDataTest)
{
	std::vector<float> sourceMat1, sourceMat2_;
	std::vector<std::vector<float>> sourceMat2;
	RepoMatrix matrix;
	RepoMatrix matrix2(sourceMat1);
	RepoMatrix matrix3(sourceMat2);
	EXPECT_TRUE(checkIsIdentity(matrix));
	EXPECT_TRUE(checkIsIdentity(matrix2));
	EXPECT_TRUE(checkIsIdentity(matrix3));


	for (int i = 0; i < 16; ++i)
	{
		sourceMat1.push_back((rand() % 1000) / 1000.f);
		if (i % 4 == 0) sourceMat2.push_back(std::vector<float>());
		sourceMat2.back().push_back((rand() % 1000) / 1000.f);
		sourceMat2_.push_back(sourceMat2.back().back());
	}

	RepoMatrix matrix4(sourceMat1), matrix5(sourceMat2);

	EXPECT_TRUE(compareStdVectors(sourceMat1, matrix4.getData()));
	EXPECT_TRUE(compareStdVectors(sourceMat2_, matrix5.getData()));
}

TEST(RepoMatrixTest, invertTest)
{
	RepoMatrix id;
	EXPECT_TRUE(checkIsIdentity(id.invert()));
	std::vector<float> matValues = { 2, 0.3f, 0.4f, 1.23f,
		0.45f, 1, 0.488f, 12345,
		0, 0, 3.5f, 0,
		0, 0, 0, 1
	};
	RepoMatrix rand(matValues);

	std::vector<float> expectedRes = {
		0.5361930294906166f, -0.16085790884718498f, -0.038851014936805824f, 1985.13146972656250000f,
		-0.24128684401512146f, 1.0723860589812333f, -0.12194561470700879f, -13238.30859375000000000f,
		0, 0, 0.28571426868438721f, 0,
		0,	0,	0,	1 };

	EXPECT_TRUE(compareStdVectors(expectedRes, rand.invert().getData()));
}

TEST(RepoMatrixTest, isIdentityTest)
{
	RepoMatrix id;
	EXPECT_TRUE(id.isIdentity());
	std::vector<float> idv = {1,0,0,0,
							 0,1,0,0,
							 0,0,1,0,
							 0,0,0,1};
	std::vector<float> higherb, lowerb, higherOver, lowerOver, higherUnder, lowerUnder;

	EXPECT_TRUE(RepoMatrix(idv).isIdentity());

	float eps = 1e-5;
	for (int i = 0; i < idv.size(); ++i)
	{
		higherb.push_back(idv[i] + eps);
		lowerb.push_back(idv[i] - eps);

		higherUnder.push_back(higherb[i] - 1e-8);
		lowerUnder.push_back(lowerb[i] + 1e-8);

		higherOver.push_back(higherb[i] + 1e-8);
		lowerOver.push_back(lowerb[i] - 1e-8);
	}

	EXPECT_TRUE(RepoMatrix(higherb).isIdentity(eps));
	EXPECT_TRUE(RepoMatrix(lowerb).isIdentity(eps));
	EXPECT_FALSE(RepoMatrix(higherOver).isIdentity(eps));
	EXPECT_FALSE(RepoMatrix(lowerOver).isIdentity(eps));
	EXPECT_TRUE(RepoMatrix(higherUnder).isIdentity(eps));
	EXPECT_TRUE(RepoMatrix(lowerUnder).isIdentity(eps));
}

TEST(RepoMatrixTest, toStringTest)
{

	std::vector<float> data;
	std::stringstream ss, ssID;

	for (int i = 0; i < 16; ++i)
	{
		data.push_back((rand() % 1000) / 1000.f);
		ss << " " << data[i];

		ssID << " " << (i % 5 == 0) ? 1.0f : 0.0f;
		if (i % 4 == 3)
		{
			ss << "\n";
			ssID << "\n";
		}
	}

	EXPECT_EQ(ss.str(), RepoMatrix(data).toString());
	EXPECT_EQ(ssID.str(), RepoMatrix().toString());

}

TEST(RepoMatrixTest, transposeTest)
{
	EXPECT_TRUE(checkIsIdentity(RepoMatrix().transpose()));
	std::vector<float> data(16), transposedData(16);

	for (int i = 0; i < 16; ++i)
	{
		data[i] = (rand() % 1000) / 1000.f;
		int row = i % 4;
		transposedData[(row * 4) + (i / 4)] = data[i];
	}
	EXPECT_TRUE(compareStdVectors(RepoMatrix(data).transpose().getData(), transposedData));

}

TEST(RepoMatrixTest, matVecTest)
{
	RepoVector3D sampleVec(3.4653f, 2.543637f, 0.3253252f);
	RepoMatrix id;
	auto newVec = id * sampleVec;

	EXPECT_EQ(sampleVec.x, newVec.x);
	EXPECT_EQ(sampleVec.y, newVec.y);
	EXPECT_EQ(sampleVec.z, newVec.z);
	std::vector<float> matValues = { 2, 0.3f, 0.4f, 1.23f,
		0.45f, 1, 0.488f, 12345,
		0.5f, 0, 3.5f, 0,
		0, 0, 0, 1
	};
	RepoMatrix rand(matValues);

	auto newVec2 = rand * sampleVec;

	EXPECT_EQ(9.05382156372070310f, newVec2.x);
	EXPECT_EQ(12349.26171875000000000f, newVec2.y);
	EXPECT_EQ(2.87128829956054690f, newVec2.z);
}

TEST(RepoMatrixTest, matMatTest)
{
	EXPECT_TRUE(checkIsIdentity(RepoMatrix()*RepoMatrix()));

	std::vector <float> matValues = { 2, 0.3f, 0.4f, 1.23f,
		0.45f, 1, 0.488f, 12345,
		0.5f, 0, 3.5f, 0,
		0, 4.56f, 0.0001f, 1
	};
	RepoMatrix rand(matValues);

	std::vector<float> matValues2 = { 3.254f, 13.12456f, 0.0001f, 1.264f,
		0.5f,   0.645f,   10, 321.02f,
		0.7892f, 10.3256f, 1, 0.5f,
		0.5f, 0.6f, 0.7f, 1
	};

	RepoMatrix rand2(matValues2);

	EXPECT_EQ(rand, RepoMatrix()*rand);
	EXPECT_EQ(rand, rand*RepoMatrix());
	auto resultRand = rand * rand2;

	std::vector<float> expectedRes =
	{ 7.58868026733398440f, 31.31086158752441400f, 4.26119995117187500f, 100.26399993896484000f,
	6174.84960937500000000f, 7418.59033203125000000f, 8651.98828125000000000f, 12666.83300781250000000f,
	4.38920021057128910f, 42.70187759399414100f, 3.50005006790161130f, 2.38199996948242190f,
	2.78007888793945310f, 3.54223251342773440f, 46.30009841918945300f, 1464.85107421875000000f };

	EXPECT_TRUE(compareStdVectors(expectedRes, resultRand.getData()));
}

TEST(RepoMatrixTest, eqOpTest)
{
	RepoMatrix id, id2;

	EXPECT_TRUE(id==id2);
	EXPECT_TRUE(id2==id);
	EXPECT_TRUE(id==id);

	std::vector<float> ranV, ranV2;
	for (int i = 0; i < 16; ++i)
	{
		ranV.push_back((rand() % 1000) / 1000.f);
		ranV2.push_back((rand() % 1000) / 1000.f);
	}

	RepoMatrix rand1(ranV), rand2(ranV2), rand3(ranV);
	EXPECT_TRUE(rand1==rand1);
	EXPECT_TRUE(rand1==rand3);
	EXPECT_TRUE(rand3==rand1);
	EXPECT_FALSE(id==rand1);
	EXPECT_FALSE(rand2==rand1);
	EXPECT_FALSE(rand1==rand2);

}

TEST(RepoMatrixTest, neqOpTest)
{
	RepoMatrix id, id2;

	EXPECT_FALSE(id != id2);
	EXPECT_FALSE(id2 != id);
	EXPECT_FALSE(id != id);

	std::vector<float> ranV, ranV2;
	for (int i = 0; i < 16; ++i)
	{
		ranV.push_back((rand() % 1000) / 1000.f);
		ranV2.push_back((rand() % 1000) / 1000.f);
	}

	RepoMatrix rand1(ranV), rand2(ranV2), rand3(ranV);
	EXPECT_FALSE(rand1 != rand1);
	EXPECT_FALSE(rand1 != rand3);
	EXPECT_FALSE(rand3 != rand1);
	EXPECT_TRUE(id != rand1);
	EXPECT_TRUE(rand2 != rand1);
	EXPECT_TRUE(rand1 != rand2);

}

TEST(RepoMatrixTest, RotateX)
{
	repo::lib::RepoVector3D x(1, 0, 0);
	repo::lib::RepoVector3D y(0, 1, 0);
	repo::lib::RepoVector3D z(0, 0, 1);

	EXPECT_THAT(RepoMatrix::rotationX(RAD(45)) * x, VectorNear(x));
	EXPECT_THAT(RepoMatrix::rotationX(RAD(90)) * y, VectorNear(z));
	EXPECT_THAT(RepoMatrix::rotationX(RAD(45)) * repo::lib::RepoVector3D(1, 1, 1), VectorNear(repo::lib::RepoVector3D(1, 0, 1.41421353816986084)));
}

TEST(RepoMatrixTest, RotateY)
{
	repo::lib::RepoVector3D x(1, 0, 0);
	repo::lib::RepoVector3D y(0, 1, 0);
	repo::lib::RepoVector3D z(0, 0, 1);

	EXPECT_THAT(RepoMatrix::rotationY(RAD(45)) * y, VectorNear(y));
	EXPECT_THAT(RepoMatrix::rotationY(RAD(90)) * z, VectorNear(x));
	EXPECT_THAT(RepoMatrix::rotationY(RAD(45)) * repo::lib::RepoVector3D(1, 1, 1), VectorNear(repo::lib::RepoVector3D(1.41421353816986084, 1, 0)));
}

TEST(RepoMatrixTest, RotateZ)
{
	repo::lib::RepoVector3D x(1, 0, 0);
	repo::lib::RepoVector3D y(0, 1, 0);
	repo::lib::RepoVector3D z(0, 0, 1);

	EXPECT_THAT(RepoMatrix::rotationZ(RAD(45)) * z, VectorNear(z));
	EXPECT_THAT(RepoMatrix::rotationZ(RAD(90)) * x, VectorNear(y));
	EXPECT_THAT(RepoMatrix::rotationZ(RAD(45)) * repo::lib::RepoVector3D(1, 1, 1), VectorNear(repo::lib::RepoVector3D(0, 1.41421353816986084, 1)));
}

TEST(RepoMatrixTest, Translate)
{
	repo::lib::RepoVector3D a(rand(), rand(), rand());
	repo::lib::RepoVector3D b(rand(), rand(), rand());

	repo::lib::RepoVector3D c(a.x + b.x, a.y + b.y, a.z + b.z);

	EXPECT_THAT(RepoMatrix::translate(a) * b, VectorNear(c));
}