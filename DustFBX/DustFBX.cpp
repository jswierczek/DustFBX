// DustFBX.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <fbxsdk.h>
#include <fbxsdk/fileio/fbxiosettings.h>


// Create texture for cube.
void CreateTexture(FbxScene* pScene, FbxMesh* pMesh)
{
    // A texture need to be connected to a property on the material,
    // so let's use the material (if it exists) or create a new one
    FbxSurfaceLambert* lMaterial = NULL;

    //get the node of mesh, add material for it.
    FbxNode* lNode = pMesh->GetNode();
  //  if(lNode)
    {
        lMaterial = lNode->GetSrcObject<FbxSurfaceLambert>(0);
        if (lMaterial == NULL)
        {
            FbxString lMaterialName = "myCube";
            FbxString lShadingName  = "myCube";
            FbxDouble3 lBlack(0.0, 0.0, 0.0);
            FbxDouble3 lRed(1.0, 0.0, 0.0);
            FbxDouble3 lDiffuseColor(0.75, 0.75, 0.0);
            lMaterial = FbxSurfaceLambert::Create(pScene, lMaterialName.Buffer());

            // Generate primary and secondary colors.
            //lMaterial->Emissive           .Set(lBlack);
           // lMaterial->Ambient            .Set(lRed);
            //lMaterial->AmbientFactor      .Set(1.);
            // Add texture for diffuse channel
            lMaterial->Diffuse           .Set(lDiffuseColor);
            //lMaterial->DiffuseFactor     .Set(1.);
            //lMaterial->TransparencyFactor.Set(0.4);
            lMaterial->ShadingModel      .Set(lShadingName);
            //lMaterial->Shininess         .Set(0.5);
            //lMaterial->Specular          .Set(lBlack);
            //lMaterial->SpecularFactor    .Set(0.3);

            lNode->AddMaterial(lMaterial);
        }
    }

    FbxFileTexture* lTexture = FbxFileTexture::Create(pScene,"myCube");

    // Set texture properties.
    lTexture->SetFileName("myCube.JPG"); // Resource file is in current directory.
	lTexture->SetTextureUse(FbxTexture::eStandard);
    lTexture->SetMappingType(FbxTexture::eUV);
	lTexture->SetMaterialUse(FbxFileTexture::eModelMaterial);
    //lTexture->SetSwapUV(false);
    //lTexture->SetTranslation(0.0, 0.0);
    //lTexture->SetScale(1.0, 1.0);
    //lTexture->SetRotation(0.0, 0.0);

    // don't forget to connect the texture to the corresponding property of the material
    if (lMaterial)
        lMaterial->Diffuse.ConnectSrcObject(lTexture);
	/*
    lTexture = FbxFileTexture::Create(pScene,"Ambient Texture");

    // Set texture properties.
    lTexture->SetFileName("myCube.jpg"); // Resource file is in current directory.
	lTexture->SetTextureUse(FbxTexture::eStandard);
    lTexture->SetMappingType(FbxTexture::eUV);
	lTexture->SetMaterialUse(FbxFileTexture::eModelMaterial);
    lTexture->SetSwapUV(false);
    lTexture->SetTranslation(0.0, 0.0);
    lTexture->SetScale(1.0, 1.0);
    lTexture->SetRotation(0.0, 0.0);

    // don't forget to connect the texture to the corresponding property of the material
    if (lMaterial)
        lMaterial->Ambient.ConnectSrcObject(lTexture);

    lTexture = FbxFileTexture::Create(pScene,"Emissive Texture");

    // Set texture properties.
    lTexture->SetFileName("myCube.jpg"); // Resource file is in current directory.
	lTexture->SetTextureUse(FbxTexture::eStandard);
    lTexture->SetMappingType(FbxTexture::eUV);
	lTexture->SetMaterialUse(FbxFileTexture::eModelMaterial);
    lTexture->SetSwapUV(false);
    lTexture->SetTranslation(0.0, 0.0);
    lTexture->SetScale(1.0, 1.0);
    lTexture->SetRotation(0.0, 0.0);

    // don't forget to connect the texture to the corresponding property of the material
    if (lMaterial)
        lMaterial->Emissive.ConnectSrcObject(lTexture);*/
}

FbxNode* CreateCubeWithTexture(FbxScene* pScene, char* pName)
{
    int i, j;
    FbxMesh* lMesh = FbxMesh::Create(pScene,pName);

    FbxVector4 vertex0(-50, 0, 50);
    FbxVector4 vertex1(50, 0, 50);
    FbxVector4 vertex2(50, 100, 50);
    FbxVector4 vertex3(-50, 100, 50);
    FbxVector4 vertex4(-50, 0, -50);
    FbxVector4 vertex5(50, 0, -50);
    FbxVector4 vertex6(50, 100, -50);
    FbxVector4 vertex7(-50, 100, -50);

    FbxVector4 lNormalXPos(1, 0, 0);
    FbxVector4 lNormalXNeg(-1, 0, 0);
    FbxVector4 lNormalYPos(0, 1, 0);
    FbxVector4 lNormalYNeg(0, -1, 0);
    FbxVector4 lNormalZPos(0, 0, 1);
    FbxVector4 lNormalZNeg(0, 0, -1);

    // Create control points.
    lMesh->InitControlPoints(24);
    FbxVector4* lControlPoints = lMesh->GetControlPoints();

    lControlPoints[0] = vertex0;
    lControlPoints[1] = vertex1;
    lControlPoints[2] = vertex2;
    lControlPoints[3] = vertex3;
    lControlPoints[4] = vertex1;
    lControlPoints[5] = vertex5;
    lControlPoints[6] = vertex6;
    lControlPoints[7] = vertex2;
    lControlPoints[8] = vertex5;
    lControlPoints[9] = vertex4;
    lControlPoints[10] = vertex7;
    lControlPoints[11] = vertex6;
    lControlPoints[12] = vertex4;
    lControlPoints[13] = vertex0;
    lControlPoints[14] = vertex3;
    lControlPoints[15] = vertex7;
    lControlPoints[16] = vertex3;
    lControlPoints[17] = vertex2;
    lControlPoints[18] = vertex6;
    lControlPoints[19] = vertex7;
    lControlPoints[20] = vertex1;
    lControlPoints[21] = vertex0;
    lControlPoints[22] = vertex4;
    lControlPoints[23] = vertex5;

    // We want to have one normal for each vertex (or control point),
    // so we set the mapping mode to eBY_CONTROL_POINT.
    FbxGeometryElementNormal* lGeometryElementNormal= lMesh->CreateElementNormal();

	lGeometryElementNormal->SetMappingMode(FbxGeometryElement::eByControlPoint);

    // Here are two different ways to set the normal values.
    bool firstWayNormalCalculations=true;
    if (firstWayNormalCalculations)
    {    
        // The first method is to set the actual normal value
        // for every control point.
		lGeometryElementNormal->SetReferenceMode(FbxGeometryElement::eDirect);

        lGeometryElementNormal->GetDirectArray().Add(lNormalZPos);
        lGeometryElementNormal->GetDirectArray().Add(lNormalZPos);
        lGeometryElementNormal->GetDirectArray().Add(lNormalZPos);
        lGeometryElementNormal->GetDirectArray().Add(lNormalZPos);
        lGeometryElementNormal->GetDirectArray().Add(lNormalXPos);
        lGeometryElementNormal->GetDirectArray().Add(lNormalXPos);
        lGeometryElementNormal->GetDirectArray().Add(lNormalXPos);
        lGeometryElementNormal->GetDirectArray().Add(lNormalXPos);
        lGeometryElementNormal->GetDirectArray().Add(lNormalZNeg);
        lGeometryElementNormal->GetDirectArray().Add(lNormalZNeg);
        lGeometryElementNormal->GetDirectArray().Add(lNormalZNeg);
        lGeometryElementNormal->GetDirectArray().Add(lNormalZNeg);
        lGeometryElementNormal->GetDirectArray().Add(lNormalXNeg);
        lGeometryElementNormal->GetDirectArray().Add(lNormalXNeg);
        lGeometryElementNormal->GetDirectArray().Add(lNormalXNeg);
        lGeometryElementNormal->GetDirectArray().Add(lNormalXNeg);
        lGeometryElementNormal->GetDirectArray().Add(lNormalYPos);
        lGeometryElementNormal->GetDirectArray().Add(lNormalYPos);
        lGeometryElementNormal->GetDirectArray().Add(lNormalYPos);
        lGeometryElementNormal->GetDirectArray().Add(lNormalYPos);
        lGeometryElementNormal->GetDirectArray().Add(lNormalYNeg);
        lGeometryElementNormal->GetDirectArray().Add(lNormalYNeg);
        lGeometryElementNormal->GetDirectArray().Add(lNormalYNeg);
        lGeometryElementNormal->GetDirectArray().Add(lNormalYNeg);
    }
    else
    {
        // The second method is to the possible values of the normals
        // in the direct array, and set the index of that value
        // in the index array for every control point.
		lGeometryElementNormal->SetReferenceMode(FbxGeometryElement::eIndexToDirect);

        // Add the 6 different normals to the direct array
        lGeometryElementNormal->GetDirectArray().Add(lNormalZPos);
        lGeometryElementNormal->GetDirectArray().Add(lNormalXPos);
        lGeometryElementNormal->GetDirectArray().Add(lNormalZNeg);
        lGeometryElementNormal->GetDirectArray().Add(lNormalXNeg);
        lGeometryElementNormal->GetDirectArray().Add(lNormalYPos);
        lGeometryElementNormal->GetDirectArray().Add(lNormalYNeg);

        // Now for each control point, we need to specify which normal to use
        lGeometryElementNormal->GetIndexArray().Add(0); // index of lNormalZPos in the direct array.
        lGeometryElementNormal->GetIndexArray().Add(0); // index of lNormalZPos in the direct array.
        lGeometryElementNormal->GetIndexArray().Add(0); // index of lNormalZPos in the direct array.
        lGeometryElementNormal->GetIndexArray().Add(0); // index of lNormalZPos in the direct array.
        lGeometryElementNormal->GetIndexArray().Add(1); // index of lNormalXPos in the direct array.
        lGeometryElementNormal->GetIndexArray().Add(1); // index of lNormalXPos in the direct array.
        lGeometryElementNormal->GetIndexArray().Add(1); // index of lNormalXPos in the direct array.
        lGeometryElementNormal->GetIndexArray().Add(1); // index of lNormalXPos in the direct array.
        lGeometryElementNormal->GetIndexArray().Add(2); // index of lNormalZNeg in the direct array.
        lGeometryElementNormal->GetIndexArray().Add(2); // index of lNormalZNeg in the direct array.
        lGeometryElementNormal->GetIndexArray().Add(2); // index of lNormalZNeg in the direct array.
        lGeometryElementNormal->GetIndexArray().Add(2); // index of lNormalZNeg in the direct array.
        lGeometryElementNormal->GetIndexArray().Add(3); // index of lNormalXNeg in the direct array.
        lGeometryElementNormal->GetIndexArray().Add(3); // index of lNormalXNeg in the direct array.
        lGeometryElementNormal->GetIndexArray().Add(3); // index of lNormalXNeg in the direct array.
        lGeometryElementNormal->GetIndexArray().Add(3); // index of lNormalXNeg in the direct array.
        lGeometryElementNormal->GetIndexArray().Add(4); // index of lNormalYPos in the direct array.
        lGeometryElementNormal->GetIndexArray().Add(4); // index of lNormalYPos in the direct array.
        lGeometryElementNormal->GetIndexArray().Add(4); // index of lNormalYPos in the direct array.
        lGeometryElementNormal->GetIndexArray().Add(4); // index of lNormalYPos in the direct array.
        lGeometryElementNormal->GetIndexArray().Add(5); // index of lNormalYNeg in the direct array.
        lGeometryElementNormal->GetIndexArray().Add(5); // index of lNormalYNeg in the direct array.
        lGeometryElementNormal->GetIndexArray().Add(5); // index of lNormalYNeg in the direct array.
        lGeometryElementNormal->GetIndexArray().Add(5); // index of lNormalYNeg in the direct array.
    }
    // Array of polygon vertices.
    int lPolygonVertices[] = { 0, 1, 2, 3,
        4, 5, 6, 7,
        8, 9, 10, 11,
        12, 13, 14, 15,
        16, 17, 18, 19,
        20, 21, 22, 23 };

	FbxGeometryElementMaterial *lm = lMesh->CreateElementMaterial();
	lm->SetName("myCube");
	lm->SetMappingMode( FbxLayerElement::eAllSame );

    // Create UV for Diffuse channel
    FbxGeometryElementUV* lUVDiffuseElement = lMesh->CreateElementUV( "DiffuseUV");
	lUVDiffuseElement->SetMappingMode(FbxGeometryElement::eByPolygonVertex);
	lUVDiffuseElement->SetReferenceMode(FbxGeometryElement::eIndexToDirect);

    FbxVector2 lVectors0(0, 0);
    FbxVector2 lVectors1(1, 0);
    FbxVector2 lVectors2(1, 1);
    FbxVector2 lVectors3(0, 1);

    lUVDiffuseElement->GetDirectArray().Add(lVectors0);
    lUVDiffuseElement->GetDirectArray().Add(lVectors1);
    lUVDiffuseElement->GetDirectArray().Add(lVectors2);
    lUVDiffuseElement->GetDirectArray().Add(lVectors3);

    // Create UV for Ambient channel
    //FbxGeometryElementUV* lUVAmbientElement = lMesh->CreateElementUV("AmbientUV");

	//lUVAmbientElement->SetMappingMode(FbxGeometryElement::eByPolygonVertex);
	//lUVAmbientElement->SetReferenceMode(FbxGeometryElement::eIndexToDirect);

    lVectors0.Set(0, 0);
    lVectors1.Set(1, 0);
    lVectors2.Set(0, 0.418586879968643);
    lVectors3.Set(1, 0.418586879968643);

    //lUVAmbientElement->GetDirectArray().Add(lVectors0);
    //lUVAmbientElement->GetDirectArray().Add(lVectors1);
    //lUVAmbientElement->GetDirectArray().Add(lVectors2);
    //lUVAmbientElement->GetDirectArray().Add(lVectors3);

    // Create UV for Emissive channel
    //FbxGeometryElementUV* lUVEmissiveElement = lMesh->CreateElementUV("EmissiveUV");

	//lUVEmissiveElement->SetMappingMode(FbxGeometryElement::eByPolygonVertex);
	//lUVEmissiveElement->SetReferenceMode(FbxGeometryElement::eIndexToDirect);

    lVectors0.Set(0.2343, 0);
    lVectors1.Set(1, 0.555);
    lVectors2.Set(0.333, 0.999);
    lVectors3.Set(0.555, 0.666);

    //lUVEmissiveElement->GetDirectArray().Add(lVectors0);
    //lUVEmissiveElement->GetDirectArray().Add(lVectors1);
    //lUVEmissiveElement->GetDirectArray().Add(lVectors2);
    //lUVEmissiveElement->GetDirectArray().Add(lVectors3);

    //Now we have set the UVs as eINDEX_TO_DIRECT reference and in eBY_POLYGON_VERTEX  mapping mode
    //we must update the size of the index array.
    lUVDiffuseElement->GetIndexArray().SetCount(24);
    //lUVAmbientElement->GetIndexArray().SetCount(24);
    //lUVEmissiveElement->GetIndexArray().SetCount(24);

    // Create polygons. Assign texture and texture UV indices.
    for(i = 0; i < 6; i++)
    {
        //we won't use the default way of assigning textures, as we have
        //textures on more than just the default (diffuse) channel.
        lMesh->BeginPolygon();//-1, -1, false);

		for(j = 0; j < 4; j++)
        {
            //this function points 
            lMesh->AddPolygon(lPolygonVertices[i*4 + j] // Control point index. 
            );
            //Now we have to update the index array of the UVs for diffuse, ambient and emissive
            lUVDiffuseElement->GetIndexArray().SetAt(i*4+j, j);
            //lUVAmbientElement->GetIndexArray().SetAt(i*4+j, j);
            //lUVEmissiveElement->GetIndexArray().SetAt(i*4+j, j);

        }

        lMesh->EndPolygon ();
    }

    FbxNode* lNode = FbxNode::Create(pScene,pName);

    lNode->SetNodeAttribute(lMesh);
	lNode->SetShadingMode(FbxNode::eTextureShading);

    CreateTexture(pScene, lMesh);

    return lNode;
}

void createScene( FbxScene* apScene )
{
	apScene->GetRootNode()->AddChild( CreateCubeWithTexture( apScene, "myCube" ) );
}

int readInt( unsigned char* &apBuffer )
{
	int lValue = *(int*)(apBuffer);
	apBuffer += sizeof( int );
	return lValue;
}

char* readString( unsigned char* &apBuffer )
{
	unsigned char lLength;
	lLength = *(unsigned char*)(apBuffer++);
	lLength &= 0x7F;
	char* lChars = new char[ lLength + 1 ];
	for (int i = 0; i < lLength; i++ )
		lChars[ i ] = *(char*)(apBuffer++);
    lChars[lLength] = 0;
	return lChars;
}

float readFloat( unsigned char* &apBuffer )
{
	float lValue = *(float*)apBuffer;
	apBuffer += sizeof( float );
	return lValue;
}

bool readBool( unsigned char* &apBuffer )
{
	bool lValue = *(bool*)apBuffer;
	apBuffer += sizeof( bool );
	return lValue;
}

struct Vertex
{
	float x,y,z;
	float nx,ny,nz;
	float u,v;
};

struct Mesh
{
	int id;
	char* textureName;
	int verticesCount;
	Vertex* vertices;
	int indicesCount;
	int* indices;
};

struct Sector
{
	int id;
	float x, y, z;
	int meshesCount;
	Mesh* meshes;
};

void setupMesh( FbxMesh* apDst, Mesh* apSrc )
{
	apDst->InitControlPoints( apSrc->verticesCount );
    
	//FbxGeometryElementMaterial* lMaterialElement = apDst->CreateElementMaterial();
	//lMaterialElement->SetMappingMode(FbxGeometryElement::eAllSame);
	//lMaterialElement->SetReferenceMode(FbxGeometryElement::eDirect);

    FbxGeometryElementNormal* lpNormals= apDst->CreateElementNormal();
	lpNormals->SetMappingMode(FbxGeometryElement::eByControlPoint);
	lpNormals->SetReferenceMode(FbxGeometryElement::eDirect);
    
	FbxGeometryElementUV* lpUVs = apDst->CreateElementUV( "DiffuseUV");
	lpUVs->SetMappingMode(FbxGeometryElement::eByControlPoint);
	lpUVs->SetReferenceMode(FbxGeometryElement::eDirect);
	
	FbxVector4* lpCP = apDst->GetControlPoints();
	for (int v = 0; v < apSrc->verticesCount; v++)
	{
		lpCP[ v ].Set( apSrc->vertices[ v ].x, apSrc->vertices[ v ].y, apSrc->vertices[ v ].z ); 
		FbxVector4 lNormal;
		lNormal.Set(  apSrc->vertices[ v ].nx, apSrc->vertices[ v ].ny, apSrc->vertices[ v ].nz );
		lpNormals->GetDirectArray().Add( lNormal );
		FbxVector2 lUV;
		lUV.Set( apSrc->vertices[ v ].u, apSrc->vertices[ v ].v );
		lpUVs->GetDirectArray().Add( lUV );
	}
	for (int i = 0; i < apSrc->indicesCount; )
	{
		apDst->BeginPolygon();
		apDst->AddPolygon( apSrc->indices[ i++ ] );
		apDst->AddPolygon( apSrc->indices[ i++ ] );		
		apDst->AddPolygon( apSrc->indices[ i++ ] );
		apDst->EndPolygon();
	}
}

void createSectorNode( FbxScene* apScene, Sector* apSector )
{
	char lName[256];
	sprintf( lName, "sec:%d", apSector->id );
	FbxNode* lpSectorNode = FbxNode::Create( apScene, lName );
	apScene->GetRootNode()->AddChild( lpSectorNode );
	for (int m = 0; m < apSector->meshesCount; m++ )
	{
		sprintf( lName, "msh:%d", apSector->meshes[ m ].id );
		FbxNode* lpMeshNode = FbxNode::Create( apScene, lName );
		FbxMesh* lpMesh = FbxMesh::Create( apScene, lName );
		lpMeshNode->SetNodeAttribute( lpMesh );
		lpMeshNode->SetShadingMode( FbxNode::eTextureShading );
		lpSectorNode->AddChild( lpMeshNode );

		setupMesh( lpMesh, apSector->meshes + m );
		
		FbxString lMaterialName = apSector->meshes[ m ].textureName;
        FbxString lShadingName  = apSector->meshes[ m ].textureName;
		FbxGeometryElementMaterial* lMaterialElement = lpMesh->CreateElementMaterial();
		lMaterialElement->SetName(lMaterialName.Buffer() );
		lMaterialElement->SetMappingMode(FbxGeometryElement::eAllSame);
	
	/*
		FbxLayer* lpLayer = lpMesh->GetLayer( 0 );
        FbxLayerElementMaterial* lLayerElementMaterial = FbxLayerElementMaterial::Create(lpMesh, lMaterialName.Buffer());
        lLayerElementMaterial->SetMappingMode(FbxLayerElement::eAllSame);
        lLayerElementMaterial->SetReferenceMode(FbxLayerElement::eIndexToDirect);
        lpLayer->SetMaterials(lLayerElementMaterial);
        lLayerElementMaterial->GetIndexArray().Add(0);
		*/
		FbxSurfaceLambert* lpMaterial = lpMeshNode->GetSrcObject<FbxSurfaceLambert>(0);

		FbxDouble3 lDC(1,1,1);
        lpMaterial = FbxSurfaceLambert::Create(apScene, lMaterialName.Buffer());
		lpMaterial->Diffuse.Set( lDC );
		lpMaterial->ShadingModel.Set(lShadingName);
        lpMeshNode->AddMaterial(lpMaterial);
		FbxFileTexture* lpTexture = FbxFileTexture::Create(apScene,lMaterialName.Buffer());
		lpTexture->SetFileName(lMaterialName.Buffer()); 
		lpTexture->SetTextureUse(FbxTexture::eStandard);
		lpTexture->SetMappingType(FbxTexture::eUV);
		lpTexture->SetMaterialUse(FbxFileTexture::eModelMaterial);
		//lpTexture->UVSet.Set(FbxString("DiffuseUV"));
        lpMaterial->Diffuse.ConnectSrcObject(lpTexture);
    }
}

void dust2FBX( FbxScene* apScene, char* apFileName )
{
	char lSource[256];

	sprintf( lSource, "%s.gfbx", apFileName);
	FILE *lpFile = fopen( lSource, "rb" );
	fseek( lpFile, 0, SEEK_END );
	int lFileSize = ftell( lpFile );
	fseek( lpFile, 0, SEEK_SET );
	unsigned char* lpFileBuffer = new unsigned char[ lFileSize ];
	fread( lpFileBuffer, 1, lFileSize, lpFile );
	fclose( lpFile );
	unsigned char* lpBufferPosition = lpFileBuffer;

	int lSectorsCount = readInt( lpBufferPosition );
	Sector* lpSectors = new Sector[ lSectorsCount ];
	for (int s = 0; s < lSectorsCount; s++)
	{
		lpSectors[ s ].id = readInt( lpBufferPosition );
		lpSectors[ s ].x = readFloat( lpBufferPosition );
		lpSectors[ s ].y = readFloat( lpBufferPosition );
		lpSectors[ s ].z = readFloat( lpBufferPosition );
		lpSectors[ s ].meshesCount = readInt( lpBufferPosition );
		lpSectors[ s ].meshes = new Mesh[ lpSectors[ s ].meshesCount ];
		Mesh* lpMeshes = lpSectors[ s ].meshes;
		for (int m = 0; m < lpSectors[ s ].meshesCount; m++)
		{
			lpMeshes[ m ].id = readInt( lpBufferPosition );
			lpMeshes[ m ].textureName = readString( lpBufferPosition );
			lpMeshes[ m ].verticesCount = readInt( lpBufferPosition );
			lpMeshes[ m ].indicesCount = readInt( lpBufferPosition );
			lpMeshes[ m ].vertices = new Vertex[ lpMeshes[ m ].verticesCount ];
			lpMeshes[ m ].indices = new int [ lpMeshes[ m ].indicesCount ];
			Vertex *lpVertices = lpMeshes[ m ].vertices;
			for (int v = 0; v < lpMeshes[ m ].verticesCount; v++ )
			{
				lpVertices[ v ].x = readFloat( lpBufferPosition );
				lpVertices[ v ].y = readFloat( lpBufferPosition );
				lpVertices[ v ].z = readFloat( lpBufferPosition );
				lpVertices[ v ].u = readFloat( lpBufferPosition );
				lpVertices[ v ].v = readFloat( lpBufferPosition );
				lpVertices[ v ].nx = readFloat( lpBufferPosition );
				lpVertices[ v ].ny = readFloat( lpBufferPosition );
				lpVertices[ v ].nz = readFloat( lpBufferPosition );
			}
			for (int i = 0; i < lpMeshes[ m ].indicesCount; i++ )
				lpMeshes[ m ].indices[ i ] = readInt( lpBufferPosition );
		}
	}

	int done = 0;
	for(int s = 0; s < lSectorsCount; s++ )
		if ( lpSectors[ s ].meshesCount > 0 )
		{
			//if ( s > 8000 )
			{
			createSectorNode( apScene, lpSectors + s );
			done++;
			}
			//if ( done == 1 )
			//	break;
		}

	delete lpFileBuffer;
}

int _tmain(int argc, _TCHAR* argv[])
{
	// Create the FBX SDK manager
	FbxManager* lSdkManager = FbxManager::Create();

	// Create an IOSettings object.
	FbxIOSettings * ios = FbxIOSettings::Create(lSdkManager, IOSROOT );
	lSdkManager->SetIOSettings(ios);

	// Set the FbxIOSettings EXP_FBX_EMBEDDED property to true.
	(*(lSdkManager->GetIOSettings())).SetBoolProp(EXP_FBX_EMBEDDED, true);

	// Get the appropriate file format.
	int lFileFormat = lSdkManager->GetIOPluginRegistry()->GetNativeWriterFormat();

	// Create an exporter.
	FbxExporter* lExporter = FbxExporter::Create(lSdkManager, "");

	// Declare the path and filename of the file to which the scene will be exported.
	// In this case, the file will be in the same directory as the executable.
	const char* lFilename = "Anasta1.fbx";

	// Initialize the exporter.
	bool lExportStatus = lExporter->Initialize(lFilename, lFileFormat, lSdkManager->GetIOSettings());

	if( !lExportStatus )
	{
		printf("Call to FbxExporter::Initialize() failed.\n");
		return false;
	}

	// Create a new scene so it can be populated by the imported file.
	FbxScene* lpScene = FbxScene::Create(lSdkManager, "Anasta1" );

	//createScene( lpScene );
	dust2FBX( lpScene, "Anasta1" );

	// Export the scene to the file.
	lExporter->Export(lpScene);

	// Destroy the exporter.
	lExporter->Destroy();

	return 0;
}

