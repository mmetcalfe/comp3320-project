#pragma once
namespace utility {

    inline const std::string getAiPropertyTypeInfoName(aiPropertyTypeInfo type) {
        switch (type) {
            case aiPTI_Float:
                return "aiPTI_Float";
            case aiPTI_String:
                return "aiPTI_String";
            case aiPTI_Integer:
                return "aiPTI_Integer";
            case aiPTI_Buffer:
                return "aiPTI_Buffer";
            default:
                return "Invalid";
        }
    }

    inline const std::string getAiTextureTypeName(aiTextureType type) {
        switch (type) {
            case aiTextureType_NONE:
                return "aiTextureType_NONE";
            case aiTextureType_DIFFUSE:
                return "aiTextureType_DIFFUSE";
            case aiTextureType_SPECULAR:
                return "aiTextureType_SPECULAR";
            case aiTextureType_AMBIENT:
                return "aiTextureType_AMBIENT";
            case aiTextureType_EMISSIVE:
                return "aiTextureType_EMISSIVE";
            case aiTextureType_HEIGHT:
                return "aiTextureType_HEIGHT";
            case aiTextureType_NORMALS:
                return "aiTextureType_NORMALS";
            case aiTextureType_SHININESS:
                return "aiTextureType_SHININESS";
            case aiTextureType_OPACITY:
                return "aiTextureType_OPACITY";
            case aiTextureType_DISPLACEMENT:
                return "aiTextureType_DISPLACEMENT";
            case aiTextureType_LIGHTMAP:
                return "aiTextureType_LIGHTMAP";
            case aiTextureType_REFLECTION:
                return "aiTextureType_REFLECTION";
            case aiTextureType_UNKNOWN:
                return "aiTextureType_UNKNOWN";
            default:
                return "Invalid";
        }
    }

    inline void printAiSceneMeshInfo(const aiScene *scene) {
        std::cout << "  HasMeshes: " << std::boolalpha << scene->HasMeshes() << "," << std::endl;
        std::cout << "  NumMeshes: " << scene->mNumMeshes << "," << std::endl;
        for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
            auto *mesh = scene->mMeshes[i];
            std::cout << "  Mesh " << i << ": {" << std::endl;
            std::cout << "    GetNumColorChannels: " << mesh->GetNumColorChannels() << "," << std::endl;
            std::cout << "    GetNumUVChannels: " << mesh->GetNumUVChannels() << "," << std::endl;
            std::cout << "    HasBones: " << std::boolalpha << mesh->HasBones() << "," << std::endl;
            std::cout << "    HasFaces: " << std::boolalpha << mesh->HasFaces() << "," << std::endl;
            std::cout << "    HasNormals: " << std::boolalpha << mesh->HasNormals() << "," << std::endl;
            std::cout << "    HasPositions: " << std::boolalpha << mesh->HasPositions() << "," << std::endl;
            std::cout << "    HasTangentsAndBitangents: " << std::boolalpha << mesh->HasTangentsAndBitangents() << "," << std::endl;
//        std::cout << "    HasTextureCoords: " << mesh->HasTextureCoords() << "," << std::endl;
//        std::cout << "    HasVertexColors: " << mesh->HasVertexColors() << "," << std::endl;

            std::cout << "    mMaterialIndex: " << mesh->mMaterialIndex << "," << std::endl;
            std::cout << "    mName: " << mesh->mName.C_Str() << "," << std::endl;
            std::cout << "    mNumAnimMeshes: " << mesh->mNumAnimMeshes << "," << std::endl;
            std::cout << "    mNumBones: " << mesh->mNumBones << "," << std::endl;
            std::cout << "    mNumFaces: " << mesh->mNumFaces << "," << std::endl;
            std::cout << "    mNumUVComponents[0]: " << mesh->mNumUVComponents[0] << "," << std::endl;
            std::cout << "    mNumVertices: " << mesh->mNumVertices << "," << std::endl;
            std::cout << "    mPrimitiveTypes: " << mesh->mPrimitiveTypes << "," << std::endl;

            // faces -> element buffer
            // vectices -> vertex buffer

            std::cout << "  }," << std::endl;
        }
    }

    inline void printAiSceneMaterialInfo(const aiScene *scene) {
        for (unsigned int i = 0; i < scene->mNumMaterials; i++) {
            auto *material = scene->mMaterials[i];
            std::cout << "  Material " << i << ": {" << std::endl;
            std::cout << "    NumProperties: " << material->mNumProperties << "," << std::endl;
            std::cout << "    Properties " << " {" << std::endl;
            for (unsigned int p = 0; p < material->mNumProperties; p++) {
                auto* property = material->mProperties[p];
                std::cout << "      " << property->mKey.C_Str() << ": {" << std::endl // API_MATKEY_XXX defines
                          << "        mType: " << getAiPropertyTypeInfoName(property->mType) << "," << std::endl
                          << "        mSemantic: " << getAiTextureTypeName(aiTextureType(property->mSemantic)) << "," << std::endl
                          << "        mIndex: " << property->mIndex << "," << std::endl
                          << "        mDataLength: " << property->mDataLength << "," << std::endl
                          << "      }," << std::endl;
            }
            std::cout << "    }" << std::endl;

            auto diffTexCount = material->GetTextureCount(aiTextureType_DIFFUSE);
            if (diffTexCount > 0) {
//            std::cout << "  Material " << i << ": {" << std::endl;

                std::cout << "    Num Diffuse Textures: " << diffTexCount << "," << std::endl;
                std::cout << "    Diffuse Textures: {" << std::endl;
                for (unsigned int t = 0; t < diffTexCount; t++) {
                    aiString path;
                    material->GetTexture(aiTextureType_DIFFUSE, t, &path);
                    std::cout << "    Texture " << t << ": " << path.C_Str() << std::endl;
                }
                std::cout << "    }" << std::endl;
//            std::cout << "  }," << std::endl;
            }
            std::cout << "  }," << std::endl;
        }
    }

    inline void printAiNodeHeirarchy(const aiNode *node, std::string indent = "  ") {
        auto newIndent = indent + "  ";
        std::cout << indent << "Node '" << node->mName.C_Str() << "': {" << std::endl;
        std::cout << newIndent << "mNumMeshes: " << node->mNumMeshes << "," << std::endl;
        std::cout << newIndent << "mMeshes: [";
        for (unsigned int i = 0; i < node->mNumMeshes; i++) {
            std::cout << node->mMeshes[i] << (i == node->mNumMeshes - 1 ? "" : ", ");
        }
        std::cout << "]," << std::endl;
        std::cout << newIndent << "mTransformation: [" << std::endl;
//    std::cout << newIndent << indent << "[" << std::endl;
        for (int row = 0; row < 4; row++) {
            std::cout << newIndent << newIndent << "[";
            for (int col = 0; col < 4; col++) {
                std::cout << std::setw(6) << std::setprecision(3) << node->mTransformation[row][col];
                if (col < 3)
                    std::cout << ", ";
            }
            std::cout << "]";
            if (row < 3)
                std::cout << ",";
            std::cout << std::endl;
        }
        std::cout << newIndent << indent << "]";
        std::cout << "," << std::endl;
        std::cout << newIndent << "mNumChildren: " << node->mNumChildren << "," << std::endl;
        std::cout << newIndent << "mChildren: [" << std::endl;
        for (unsigned int i = 0; i < node->mNumChildren; i++) {
            printAiNodeHeirarchy(node->mChildren[i], newIndent + "  ");
        }
        std::cout << newIndent << "]," << std::endl;
        std::cout << indent << "}," << std::endl;
    }

    inline void printAiSceneInfo(const aiScene *scene, const std::string &modelFileName = "printAiSceneInfo") {
        std::cout << modelFileName << " {" << std::endl;
        std::cout << "  HasAnimations: " << std::boolalpha << scene->HasAnimations() << "," << std::endl;
        std::cout << "  NumAnimations: " << scene->mNumAnimations << "," << std::endl;
        std::cout << "  HasCameras: " << std::boolalpha << scene->HasCameras() << "," << std::endl;
        std::cout << "  NumCameras: " << scene->mNumCameras << "," << std::endl;
        std::cout << "  HasLights: " << std::boolalpha << scene->HasLights() << "," << std::endl;
        std::cout << "  NumLights: " << scene->mNumLights << "," << std::endl;
        std::cout << "  HasMaterials: " << std::boolalpha << scene->HasMaterials() << "," << std::endl;
        std::cout << "  NumMaterials: " << scene->mNumMaterials << "," << std::endl;

        printAiSceneMaterialInfo(scene);

        printAiSceneMeshInfo(scene);

        std::cout << "  HasTextures: " << std::boolalpha << scene->HasTextures() << "," << std::endl;
        std::cout << "  NumTextures: " << scene->mNumTextures << "," << std::endl;

//    printAiNodeHeirarchy(scene->mRootNode, "  ");

        std::cout << "}" << std::endl;
    }

}