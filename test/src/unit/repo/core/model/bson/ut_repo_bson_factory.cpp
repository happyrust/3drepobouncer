/**
*  Copyright (C) 2015 3D Repo Ltd
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

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <gtest/gtest-matchers.h>
#include "../../../../repo_test_utils.h"
#include "../../../../repo_test_matchers.h"
#include "../../../../repo_test_mesh_utils.h"

#include <repo/core/model/bson/repo_bson_factory.h>
#include <repo/core/model/bson/repo_bson_builder.h>
#include <repo/lib/datastructure/repo_variant_utils.h>
#include "repo/lib/datastructure/repo_structs.h"

using namespace repo::lib;
using namespace repo::core::model;
using namespace testing;
using namespace repo;

TEST(RepoBSONFactoryTest, MakeMaterialNodeTest)
{
	repo_material_t mat_struct;
	mat_struct.opacity = 0.9;
	mat_struct.shininess = 1.0;
	mat_struct.shininessStrength = 0.5;
	mat_struct.lineWeight = 3;
	mat_struct.isWireframe = true;
	mat_struct.isTwoSided = false;

	std::string name = "MatTest";

	MaterialNode material = RepoBSONFactory::makeMaterialNode(mat_struct, name);

	EXPECT_EQ(name, material.getName());
	EXPECT_EQ(material.getTypeAsEnum(), NodeType::MATERIAL);

	// MaterialNodes should have their unique and shared Id's initialised, as they
	// will sometimes parent texture nodes.

	EXPECT_THAT(material.getUniqueID(), Not(Eq(repo::lib::RepoUUID::defaultValue)));
	EXPECT_THAT(material.getSharedID(), Not(Eq(repo::lib::RepoUUID::defaultValue)));

	auto matOut = material.getMaterialStruct();

	EXPECT_TRUE(compareMaterialStructs(mat_struct, matOut));

	repo_material_t emptyStruct;

	//See if it breaks if the vectors in the struct is never filled
	MaterialNode material2 = RepoBSONFactory::makeMaterialNode(emptyStruct, name);
	EXPECT_EQ(name, material2.getName());
	EXPECT_EQ(material2.getTypeAsEnum(), NodeType::MATERIAL);
}

TEST(RepoBSONFactoryTest, MakeMetaDataNodeTest)
{
	std::vector<std::string> keys({ "one", "two", "three", "four", "five" });
	std::vector<std::string> values({ "!", "!!", "!!!", "!!!!", "!!!!!" });

	std::string name = "MetaTest";

	std::vector<repo::lib::RepoVariant> variants;
	for (int i = 0; i < keys.size(); i++) {
		variants.push_back(repo::lib::RepoVariant(values[i]));
	}

	MetadataNode metaNode = RepoBSONFactory::makeMetaDataNode(keys, variants, name);

	EXPECT_EQ(name, metaNode.getName());
	EXPECT_EQ(metaNode.getTypeAsEnum(), NodeType::METADATA);

	auto metadata = metaNode.getAllMetadata();

	ASSERT_TRUE(metadata.size());

	repo::lib::StringConversionVisitor stringify;
	for(auto m : metadata)
	{
		auto key = m.first;
		auto value = m.second.apply_visitor(stringify);

		auto keyIt = std::find(keys.begin(), keys.end(), key);
		ASSERT_NE(keyIt, keys.end());
		auto vectorIdx = keyIt - keys.begin();

		EXPECT_EQ(key, keys[vectorIdx]);
		EXPECT_EQ(value, values[vectorIdx]);
	}
}

TEST(RepoBSONFactoryTest, MakeMeshNodeTest)
{
	uint32_t nCount = 10;

	//Set up faces
	std::vector<repo_face_t> faces;
	std::vector<repo::lib::RepoVector3D> vectors;
	std::vector<repo::lib::RepoVector3D> normals;
	std::vector<std::vector<repo::lib::RepoVector2D>> uvChannels;
	uvChannels.resize(1);
	faces.reserve(nCount);
	vectors.reserve(nCount);
	normals.reserve(nCount);
	uvChannels[0].reserve(nCount);
	for (uint32_t i = 0; i < nCount; ++i)
	{
		repo_face_t face = { (uint32_t)std::rand(), (uint32_t)std::rand(), (uint32_t)std::rand() };
		faces.push_back(face);

		vectors.push_back({ (float)std::rand() / 100.0f, (float)std::rand() / 100.0f, (float)std::rand() / 100.0f });
		normals.push_back({ (float)std::rand() / 100.0f, (float)std::rand() / 100.0f, (float)std::rand() / 100.0f });
		uvChannels[0].push_back({ (float)std::rand() / 100.0f, (float)std::rand() / 100.0f });
	}
	repo::lib::RepoBounds boundingBox(
		repo::lib::RepoVector3D64(std::rand() / 100.f, std::rand() / 100.f, std::rand() / 100.f),
		repo::lib::RepoVector3D64(std::rand() / 100.f, std::rand() / 100.f, std::rand() / 100.f)
	);

	std::string name = "meshTest";

	//End of setting up data... the actual testing happens here.

	auto mesh = RepoBSONFactory::makeMeshNode(vectors, faces, normals, boundingBox, uvChannels);

	auto vOut = mesh.getVertices();
	auto nOut = mesh.getNormals();
	auto fOut = mesh.getFaces();
	auto uvOut = mesh.getUVChannelsSeparated();
	EXPECT_FALSE(mesh.getUniqueID().isDefaultValue());
	EXPECT_FALSE(mesh.getSharedID().isDefaultValue());
	EXPECT_TRUE(compareStdVectors(vectors, vOut));
	EXPECT_TRUE(compareStdVectors(normals, nOut));
	EXPECT_TRUE(compareStdVectors(faces, fOut));
	EXPECT_TRUE(compareStdVectors(uvChannels, uvOut));

	ASSERT_EQ(MeshNode::Primitive::TRIANGLES, mesh.getPrimitive());

	auto bbox = mesh.getBoundingBox();
	EXPECT_THAT(bbox, Eq(boundingBox));

	faces.clear();
	for (uint32_t i = 0; i < nCount; ++i)
	{
		repo_face_t face = { (uint32_t)std::rand(), (uint32_t)std::rand() };
		faces.push_back(face);
	}

	// Re-create the mesh but using lines instead of triangles. This should change the primitive type, but otherwise all properties should be handled identically.

	mesh = RepoBSONFactory::makeMeshNode(vectors, faces, normals, boundingBox, uvChannels);

	ASSERT_EQ(MeshNode::Primitive::LINES, mesh.getPrimitive());

	vOut = mesh.getVertices();
	nOut = mesh.getNormals();
	fOut = mesh.getFaces();
	uvOut = mesh.getUVChannelsSeparated();
	EXPECT_FALSE(mesh.getUniqueID().isDefaultValue());
	EXPECT_FALSE(mesh.getSharedID().isDefaultValue());
	EXPECT_TRUE(compareStdVectors(vectors, vOut));
	EXPECT_TRUE(compareStdVectors(normals, nOut));
	EXPECT_TRUE(compareStdVectors(faces, fOut));
	EXPECT_TRUE(compareStdVectors(uvChannels, uvOut));

	bbox = mesh.getBoundingBox();
	EXPECT_THAT(bbox, Eq(boundingBox));

	// Re-create the mesh but with an unsupported primitive type. If the mesh does not have a type set, the API should return triangles,
	// but if the primitive has *attempted* to be inferred and failed, the type should report as unknown.

	faces.clear();
	for (uint32_t i = 0; i < nCount; ++i)
	{
		repo_face_t face; // empty faces should result in an unknown primitive type
		faces.push_back(face);
	}

	mesh = RepoBSONFactory::makeMeshNode(vectors, faces, normals, boundingBox);

	EXPECT_FALSE(mesh.getUniqueID().isDefaultValue());
	EXPECT_FALSE(mesh.getSharedID().isDefaultValue());

	ASSERT_EQ(MeshNode::Primitive::UNKNOWN, mesh.getPrimitive());

	// Create a mesh with an empty set of UV channels - empty channels should be
	// ignored by makeMeshNode

	mesh = RepoBSONFactory::makeMeshNode(vectors, faces, normals, boundingBox, { });
	EXPECT_THAT(mesh.getNumUVChannels(), Eq(0));
	EXPECT_THAT(mesh.getUVChannelsSeparated(), IsEmpty());

	mesh = RepoBSONFactory::makeMeshNode(vectors, faces, normals, boundingBox, { {} });
	EXPECT_THAT(mesh.getNumUVChannels(), Eq(0));
	EXPECT_THAT(mesh.getUVChannelsSeparated(), IsEmpty());

	mesh = RepoBSONFactory::makeMeshNode(vectors, faces, normals, boundingBox, { {}, {} });
	EXPECT_THAT(mesh.getNumUVChannels(), Eq(0));
	EXPECT_THAT(mesh.getUVChannelsSeparated(), IsEmpty());

}

TEST(RepoBSONFactoryTest, MakeReferenceNodeTest)
{
	std::string dbName = "testDB";
	std::string proName = "testProj";
	repo::lib::RepoUUID revId = repo::lib::RepoUUID::createUUID();
	bool isUnique = true;
	std::string name = "refNodeName";

	ReferenceNode ref = RepoBSONFactory::makeReferenceNode(dbName, proName, revId, isUnique, name);

	EXPECT_EQ(dbName, ref.getDatabaseName());
	EXPECT_EQ(proName, ref.getProjectId());
	EXPECT_EQ(revId, ref.getProjectRevision());
	EXPECT_EQ(isUnique, ref.useSpecificRevision());
	EXPECT_EQ(name, ref.getName());

	ReferenceNode ref2 = RepoBSONFactory::makeReferenceNode(dbName, proName, revId, !isUnique, name);
	EXPECT_EQ(!isUnique, ref2.useSpecificRevision());
}

TEST(RepoBSONFactoryTest, MakeRevisionNodeTest)
{
	std::string owner = "revOwner";
	repo::lib::RepoUUID branchID = repo::lib::RepoUUID::createUUID();
	std::vector<std::string> files = { "test1", "test5" };
	std::vector<repo::lib::RepoUUID> parents;
	size_t parentCount = 5;
	parents.reserve(parentCount);
	for (size_t i = 0; i < parentCount; ++i)
		parents.push_back(repo::lib::RepoUUID::createUUID());
	std::string message = "this is some random message to test message";
	std::string tag = "this is a random tag to test tags";
	std::vector<double> offset = { std::rand() / 100., std::rand() / 100., std::rand() / 100. };
	repo::lib::RepoUUID revId = repo::lib::RepoUUID::createUUID();

	auto rev = RepoBSONFactory::makeRevisionNode(owner, branchID, revId, files, parents, offset, message, tag);
	EXPECT_EQ(owner, rev.getAuthor());
	EXPECT_EQ(branchID, rev.getSharedID());
	EXPECT_EQ(revId, rev.getUniqueID());
	EXPECT_EQ(message, rev.getMessage());
	EXPECT_EQ(tag, rev.getTag());
	//fileNames changes after it gets into the bson, just check the size
	EXPECT_EQ(files.size(), rev.getOrgFiles().size());

	EXPECT_THAT(rev.getParentIDs(), UnorderedElementsAreArray(parents));
	EXPECT_THAT(rev.getCoordOffset(), Eq(offset));

	//ensure no random parent being generated
	std::vector<repo::lib::RepoUUID> emptyParents;
	auto rev2 = RepoBSONFactory::makeRevisionNode(owner, branchID, revId, files, emptyParents, offset, message, tag);
	EXPECT_EQ(0, rev2.getParentIDs().size());
}

TEST(RepoBSONFactoryTest, MakeTextureNodeTest)
{
	std::string ext = "jpg";
	std::string name = "textureNode." + ext;
	std::string data = "The value of this texture is represented by this string as all it takes is a char*";
	int width = 100, height = 110;

	TextureNode tex = RepoBSONFactory::makeTextureNode(name, data.c_str(), data.size(), width, height);

	ASSERT_FALSE(tex.isEmpty());

	EXPECT_EQ(name, tex.getName());
	EXPECT_EQ(width, tex.getWidth());
	EXPECT_EQ(height, tex.getHeight());
	EXPECT_EQ(ext, tex.getFileExtension());
	auto rawOut = tex.getRawData();
	ASSERT_EQ(data.size(), rawOut.size());
	EXPECT_EQ(0, memcmp(data.c_str(), rawOut.data(), data.size()));

	//make sure the code doesn't fail over if for some reason the name does not contain the extension
	TextureNode tex2 = RepoBSONFactory::makeTextureNode("noExtensionName", data.c_str(), data.size(), width, height);
}

TEST(RepoBSONFactoryTest, MakeTransformationNodeTest)
{
	//If I make a transformation with no parameters, it should be identity matrix
	std::vector<float> identity =
	{ 1, 0, 0, 0,
	0, 1, 0, 0,
	0, 0, 1, 0,
	0, 0, 0, 1 };

	TransformationNode trans = RepoBSONFactory::makeTransformationNode();

	EXPECT_TRUE(compareStdVectors(identity, trans.getTransMatrix().getData()));

	std::vector<std::vector<float>> transMat;
	std::vector<float> transMatFlat;
	transMat.resize(4);
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j)
		{
			transMat[i].push_back(std::rand() / 100.);
			transMatFlat.push_back(transMat[i][j]);
		}
	std::string name = "myTransTest";

	std::vector<repo::lib::RepoUUID> parents;
	for (int i = 0; i < 10; ++i)
		parents.push_back(repo::lib::RepoUUID::createUUID());

	TransformationNode trans2 = RepoBSONFactory::makeTransformationNode(transMat, name, parents);

	EXPECT_EQ(name, trans2.getName());
	auto matrix = trans2.getTransMatrix();

	EXPECT_TRUE(compareStdVectors(transMatFlat, matrix.getData()));
	EXPECT_THAT(trans2.getParentIDs(), UnorderedElementsAreArray(parents));

	//ensure random parents aren't thrown in
	parents.clear();
	TransformationNode trans3 = RepoBSONFactory::makeTransformationNode(transMat, name, parents);
	EXPECT_EQ(parents.size(), trans3.getParentIDs().size());
}

TEST(RepoBSONFactoryTest, MakeRepoBundleAssets)
{
	// Generate an assets list document with 64500 supermeshes. This is an
	// arbitrary number, greater than the amount we would expect to handle for
	// the near future. The document size with this number should be less than
	// the 16 Mb maximum document size of mongo.

	const int NUM_ASSETS = 64500;

	std::vector<std::string> jsonFiles;
	std::vector<std::string> bundleFiles;
	std::vector<RepoSupermeshMetadata> metadata;

	for (size_t i = 0; i < NUM_ASSETS; i++)
	{
		jsonFiles.push_back(repo::lib::RepoUUID::createUUID().toString());
		bundleFiles.push_back(repo::lib::RepoUUID::createUUID().toString());
		RepoSupermeshMetadata m;
		m.max = { 1, 1, 1 };
		m.min = { -1,-1,-1 };
		m.numFaces = INT_MAX;
		m.numVertices = USHRT_MAX;
		m.numUVChannels = 8;
		m.primitive = 3;
		metadata.push_back(m);
	}

	auto assets = RepoBSONFactory::makeRepoBundleAssets(
		repo::lib::RepoUUID::createUUID(),
		bundleFiles,
		"teamspace",
		"model",
		{ 0,0,0 },
		jsonFiles,
		metadata);

	auto bsonsize = ((RepoBSON)assets).objsize();

	EXPECT_LT(bsonsize, 16777216);
}