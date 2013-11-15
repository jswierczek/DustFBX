// DustFBX.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "fbx.h"

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

//////////////////////

void setupFbxMeshMaterial( FbxSurfaceLambert* apMaterial, FbxMesh* apMesh, FbxString aMaterialName )
{
	FbxGeometryElementMaterial* lMaterialElement = apMesh->CreateElementMaterial();
	lMaterialElement->SetName(aMaterialName.Buffer());
	lMaterialElement->SetMappingMode(FbxGeometryElement::eAllSame);
	lMaterialElement->SetReferenceMode(FbxLayerElement::eDirect);
		
	FbxLayer* lpLayer = apMesh->GetLayer( 0 );
    FbxLayerElementMaterial* lLayerElementMaterial = FbxLayerElementMaterial::Create(apMesh, aMaterialName.Buffer());
    lLayerElementMaterial->SetMappingMode(FbxLayerElement::eAllSame);
	lLayerElementMaterial->SetReferenceMode(FbxLayerElement::eDirect);
    lpLayer->SetMaterials(lLayerElementMaterial);
    lLayerElementMaterial->GetIndexArray().Add(0);
		
	FbxDouble3 lDC(1,1,1);
	apMaterial->Diffuse.Set( lDC );
	apMaterial->ShadingModel.Set(aMaterialName);
    apMaterial->Emissive.Set( FbxDouble3(0,0,0) );
    apMaterial->Ambient.Set(FbxDouble3(0,0,0));
    apMaterial->AmbientFactor.Set(1.);
    apMaterial->Diffuse.Set(FbxDouble3(1,1,1));
    apMaterial->DiffuseFactor.Set(1.);
}

void setupFbxMeshTexture( FbxFileTexture* apTexture, FbxString aTextureName )
{
	apTexture->SetFileName(aTextureName.Buffer()); 
	apTexture->SetTextureUse(FbxTexture::eStandard);
	apTexture->SetMappingType(FbxTexture::eUV);
	apTexture->SetMaterialUse(FbxFileTexture::eModelMaterial);
	apTexture->UVSet.Set(FbxString("DiffuseUV"));
	apTexture->SetTranslation(0.0, 0.0);
	apTexture->SetScale(1.0, 1.0);
	apTexture->SetRotation(0.0, 0.0);
}

//////////////////////

struct Vertex
{
	float x,y,z;
	float nx,ny,nz;
	float u,v;
};

struct SectorMesh
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
	SectorMesh* meshes;
};

void setupSectorMeshGeometry( FbxMesh* apDst, SectorMesh* apSrc )
{
	apDst->InitControlPoints( apSrc->verticesCount );

    FbxGeometryElementNormal* lpNormals= apDst->CreateElementNormal();
	lpNormals->SetMappingMode(FbxGeometryElement::eByControlPoint);
	lpNormals->SetReferenceMode(FbxGeometryElement::eDirect);
    
	FbxGeometryElementUV* lpUVs = apDst->CreateElementUV( "DiffuseUV" );
	lpUVs->SetMappingMode(FbxGeometryElement::eByControlPoint);
	lpUVs->SetReferenceMode(FbxGeometryElement::eDirect);
	
	FbxVector4* lpCP = apDst->GetControlPoints();
	FbxVector4 lNormal;
	FbxVector2 lUV;
	for (int v = 0; v < apSrc->verticesCount; v++)
	{
		lpCP[ v ].Set( apSrc->vertices[ v ].x, apSrc->vertices[ v ].y, apSrc->vertices[ v ].z ); 

		lNormal.Set(  apSrc->vertices[ v ].nx, apSrc->vertices[ v ].ny, apSrc->vertices[ v ].nz );
		lpNormals->GetDirectArray().Add( lNormal );

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
		FbxString lMaterialName = apSector->meshes[ m ].textureName;
		FbxString lTextureName = apSector->meshes[ m ].textureName;
		sprintf( lName, "msh:%d", apSector->meshes[ m ].id );
		
		FbxNode* lpMeshNode = FbxNode::Create( apScene, lName );
		FbxMesh* lpMesh = FbxMesh::Create( apScene, lName );
		FbxSurfaceLambert* lpMaterial = FbxSurfaceLambert::Create( apScene, lMaterialName.Buffer());
		FbxFileTexture* lpTexture = FbxFileTexture::Create(apScene,lTextureName.Buffer());
		
		lpMeshNode->AddMaterial(lpMaterial);
		lpMeshNode->SetNodeAttribute( lpMesh );
		lpMeshNode->SetShadingMode( FbxNode::eTextureShading );
		lpMaterial->Diffuse.ConnectSrcObject(lpTexture);

		setupSectorMeshGeometry( lpMesh, apSector->meshes + m );
		setupFbxMeshMaterial( lpMaterial, lpMesh, lMaterialName );
		setupFbxMeshTexture( lpTexture, lTextureName );

		lpSectorNode->AddChild( lpMeshNode );
	}
}

void dustLevel2FBX( FbxScene* apScene, char* apFileName )
{
	char lSource[256];

	sprintf( lSource, "..//..//DustResources//levels//%s.gfbx", apFileName);
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
		lpSectors[ s ].meshes = new SectorMesh[ lpSectors[ s ].meshesCount ];
		SectorMesh* lpMeshes = lpSectors[ s ].meshes;
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

	//////
	// fbx creation
	for(int s = 0; s < lSectorsCount; s++ )
		if ( lpSectors[ s ].meshesCount > 0 )
			createSectorNode( apScene, lpSectors + s );
	//////

	for (int s = 0; s < lSectorsCount; s++)
	{
		for (int m = 0; m < lpSectors[ s ].meshesCount; m++)
		{
			delete lpSectors[ s ].meshes[ m ].textureName;
			delete lpSectors[ s ].meshes[ m ].vertices;
			delete lpSectors[ s ].meshes[ m ].indices;
		}
		delete lpSectors[ s ].meshes;
	}
	delete lpSectors;
	delete lpFileBuffer;
}

////////////////////////

struct MotionSequence
{
	int firstKeyframeNumber;
	int keyframesCount;
};

struct MotionKeyframe
{
	int number;
	float x,y,z;
	float qx,qy,qz,qw;
};

struct MotionVertex
{
	float x,y,z;
	float nx,ny,nz;
	float u,v;
};

struct MotionFace
{
	int v1, v2, v3;
};

struct MotionSubmesh
{
	char* materialName;
	int facesCount;
	int verticesCount;
	MotionFace* faces;
	MotionVertex* vertices;
};

struct MotionMesh
{
	int visible;
	int type;
	char* name;
	int submeshesCount;
	MotionSubmesh* submeshes;
	MotionKeyframe *keyframes;
};

struct Motion
{
	int meshesCount;
	int lightframesCount;
	int keyframesCount;
	int sequencesCount;
	char* name;
	MotionSequence* sequences;
	MotionMesh* meshes;
};

void loadMotionMesh( char* apMotionName, MotionMesh* apMesh )
{
	char lSource[256];

	sprintf( lSource, "..//..//DustResources//objects//%s_%s.mesh", apMotionName, apMesh->name);
	FILE *lpFile = fopen( lSource, "rb" );
	fseek( lpFile, 0, SEEK_END );
	int lFileSize = ftell( lpFile );
	fseek( lpFile, 0, SEEK_SET );
	unsigned char* lpFileBuffer = new unsigned char[ lFileSize ];
	fread( lpFileBuffer, 1, lFileSize, lpFile );
	fclose( lpFile );
	unsigned char* lpBufferPosition = lpFileBuffer;

	apMesh->submeshesCount = readInt( lpBufferPosition );
	apMesh->submeshes = new MotionSubmesh[ apMesh->submeshesCount ];
	for( int i = 0; i < apMesh->submeshesCount; i++ )
	{
		apMesh->submeshes[ i ].materialName = readString( lpBufferPosition );
		apMesh->submeshes[ i ].facesCount = readInt( lpBufferPosition );
		apMesh->submeshes[ i ].verticesCount = readInt( lpBufferPosition );
		apMesh->submeshes[ i ].faces = new MotionFace[ apMesh->submeshes[ i ].facesCount ];
		apMesh->submeshes[ i ].vertices = new MotionVertex[ apMesh->submeshes[ i ].verticesCount ];
		for ( int j = 0 ; j < apMesh->submeshes[ i ].verticesCount; j++ )
		{
			apMesh->submeshes[ i ].vertices[ j ].x = readFloat( lpBufferPosition );
			apMesh->submeshes[ i ].vertices[ j ].y = readFloat( lpBufferPosition );
			apMesh->submeshes[ i ].vertices[ j ].z = readFloat( lpBufferPosition );
			apMesh->submeshes[ i ].vertices[ j ].nx = readFloat( lpBufferPosition );
			apMesh->submeshes[ i ].vertices[ j ].ny = readFloat( lpBufferPosition );
			apMesh->submeshes[ i ].vertices[ j ].nz = readFloat( lpBufferPosition );
			apMesh->submeshes[ i ].vertices[ j ].u = readFloat( lpBufferPosition );
			apMesh->submeshes[ i ].vertices[ j ].v = readFloat( lpBufferPosition );
		}
		for ( int j = 0; j < apMesh->submeshes[ i ].facesCount; j++ )
		{
			apMesh->submeshes[ i ].faces[ j ].v1 = readInt ( lpBufferPosition );
			apMesh->submeshes[ i ].faces[ j ].v2 = readInt ( lpBufferPosition );
			apMesh->submeshes[ i ].faces[ j ].v3 = readInt ( lpBufferPosition );
		}
	}

	delete lpFileBuffer;
}

void setupMotionMeshGeometry( FbxMesh* apDst, MotionSubmesh* apSrc )
{
	apDst->InitControlPoints( apSrc->verticesCount );

    FbxGeometryElementNormal* lpNormals= apDst->CreateElementNormal();
	lpNormals->SetMappingMode(FbxGeometryElement::eByControlPoint);
	lpNormals->SetReferenceMode(FbxGeometryElement::eDirect);
    
	FbxGeometryElementUV* lpUVs = apDst->CreateElementUV( "DiffuseUV" );
	lpUVs->SetMappingMode(FbxGeometryElement::eByControlPoint);
	lpUVs->SetReferenceMode(FbxGeometryElement::eDirect);
	
	FbxVector4* lpCP = apDst->GetControlPoints();
	FbxVector4 lNormal;
	FbxVector2 lUV;
	for (int v = 0; v < apSrc->verticesCount; v++)
	{
		lpCP[ v ].Set( apSrc->vertices[ v ].x, apSrc->vertices[ v ].y, apSrc->vertices[ v ].z ); 

		lNormal.Set(  apSrc->vertices[ v ].nx, apSrc->vertices[ v ].ny, apSrc->vertices[ v ].nz );
		lpNormals->GetDirectArray().Add( lNormal );

		lUV.Set( apSrc->vertices[ v ].u, apSrc->vertices[ v ].v );
		lpUVs->GetDirectArray().Add( lUV );
	}
	
	for (int i = 0; i < apSrc->facesCount; i++ )
	{
		apDst->BeginPolygon();
		apDst->AddPolygon( apSrc->faces[ i ].v1 );
		apDst->AddPolygon( apSrc->faces[ i ].v2 );		
		apDst->AddPolygon( apSrc->faces[ i ].v3 );
		apDst->EndPolygon();
	}
}

void createMotionMeshNode( FbxScene* apScene, Motion* apMotion )
{
	char lName[256];
	sprintf( lName, "motion:%s", apMotion->name );
	FbxNode* lpSectorNode = FbxNode::Create( apScene, lName );
	apScene->GetRootNode()->AddChild( lpSectorNode );
	
	FbxAnimStack* lpAnimations = FbxAnimStack::Create( apScene, "Sequences" );

	FbxAnimLayer** lpSequences = new FbxAnimLayer*[ apMotion->sequencesCount ];
	for (int a = 0; a < apMotion->sequencesCount; a++ )
	{
		char lSequenceName[256]; sprintf( lSequenceName, "seq:%d", a );
		FbxAnimLayer* lpSequence = FbxAnimLayer::Create( apScene, lSequenceName );
		lpAnimations->AddMember( lpSequence );
		lpSequences[ a ] = lpSequence;
	}

	for (int m = 0; m < apMotion->meshesCount; m++ )
	{
		if ( apMotion->meshes[ m ].visible )
			for ( int sm = 0; sm < apMotion->meshes[ m ].submeshesCount; sm++ )
			{
				char lMaterial[ 256 ];
				if ( strncmp( apMotion->meshes[ m ].submeshes[ sm ].materialName, "texture_", 8 ) == 0 )
				{
					strcpy( lMaterial, apMotion->meshes[ m ].submeshes[ sm ].materialName + 8 );
					strcat( lMaterial, ".jpg" );
				}
				else
					strcpy( lMaterial, apMotion->meshes[ m ].submeshes[ sm ].materialName );
				FbxString lMaterialName = lMaterial;
				FbxString lTextureName = lMaterial;
				sprintf( lName, "msh:%s.%d", apMotion->meshes[ m ].name, sm );
		
				FbxNode* lpMeshNode = FbxNode::Create( apScene, lName );
				FbxMesh* lpMesh = FbxMesh::Create( apScene, lName );
				FbxSurfaceLambert* lpMaterial = FbxSurfaceLambert::Create( apScene, lMaterialName.Buffer());
				FbxFileTexture* lpTexture = FbxFileTexture::Create(apScene,lTextureName.Buffer());
		
				lpMeshNode->AddMaterial(lpMaterial);
				lpMeshNode->SetNodeAttribute( lpMesh );
				lpMeshNode->SetShadingMode( FbxNode::eTextureShading );
				lpMaterial->Diffuse.ConnectSrcObject(lpTexture);

				setupMotionMeshGeometry( lpMesh, &apMotion->meshes[ m ].submeshes[ sm ] );
				setupFbxMeshMaterial( lpMaterial, lpMesh, lMaterialName );
				setupFbxMeshTexture( lpTexture, lTextureName );

				lpSectorNode->AddChild( lpMeshNode );
				
				for (int a = 0; a < apMotion->sequencesCount; a++ )
				{
					FbxAnimCurveNode* lpCurveNode = FbxAnimCurveNode::CreateTypedCurveNode( lpMeshNode->LclTranslation, apScene );
					lpSequences[ a ]->AddMember( lpCurveNode );	
					FbxAnimCurve* lpCurveT = lpMeshNode->LclTranslation.GetCurve( lpSequences[ a ], true );
					FbxAnimCurve* lpCurveR = lpMeshNode->LclRotation.GetCurve( lpSequences[ a], true );
					
					lpCurveT->KeyModifyBegin();
					FbxTime lTime;
					FbxAnimCurveKey lCurveKey;
					for (int k = 0; k < apMotion->sequences[ a ].keyframesCount; k++)
					{
						MotionKeyframe *key = &apMotion->meshes[ m ].keyframes[ apMotion->sequences[ a ].firstKeyframeNumber + k ];
						lTime.SetSecondDouble( (double)k );
						lCurveKey.Set( lTime, key->x );
						lCurveKey.Set( lTime, key->y );
						lCurveKey.Set( lTime, key->z );
						lpCurveT->KeyAdd( lTime, lCurveKey );
					}
					lpCurveT->KeyModifyEnd();

					lpCurveR->KeyModifyBegin();
					for (int k = 0; k < apMotion->sequences[ a ].keyframesCount; k++)
					{
						MotionKeyframe *key = &apMotion->meshes[ m ].keyframes[ apMotion->sequences[ a ].firstKeyframeNumber + k ];
						lTime.SetSecondDouble( (double)k );

						float roll  = atan2(2*key->qy*key->qw - 2*key->qx*key->qz, 1 - 2*key->qy*key->qy - 2*key->qz*key->qz);
						float pitch = atan2(2*key->qx*key->qw - 2*key->qy*key->qz, 1 - 2*key->qx*key->qx - 2*key->qz*key->qz);
						float yaw   = asin(2*key->qx*key->qy + 2*key->qz*key->qw);

						lCurveKey.Set( lTime, roll );
						lCurveKey.Set( lTime, pitch );
						lCurveKey.Set( lTime, yaw );
						lpCurveR->KeyAdd( lTime, lCurveKey );
					}
					lpCurveR->KeyModifyEnd();
				}
			}
	}

	delete lpSequences;
}

void dustMotion2FBX( FbxScene* apScene, char* apFileName )
{
	char lSource[256];

	sprintf( lSource, "..//..//DustResources//objects//%s.motion", apFileName);
	FILE *lpFile = fopen( lSource, "rb" );
	fseek( lpFile, 0, SEEK_END );
	int lFileSize = ftell( lpFile );
	fseek( lpFile, 0, SEEK_SET );
	unsigned char* lpFileBuffer = new unsigned char[ lFileSize ];
	fread( lpFileBuffer, 1, lFileSize, lpFile );
	fclose( lpFile );
	unsigned char* lpBufferPosition = lpFileBuffer;

	Motion lMotion;
	lMotion.name = new char[ strlen( apFileName ) + 1 ]; strcpy( lMotion.name, apFileName );
	lMotion.meshesCount = readInt( lpBufferPosition );
	lMotion.keyframesCount = readInt( lpBufferPosition );
	lMotion.lightframesCount = readInt( lpBufferPosition );
	lMotion.sequencesCount = readInt( lpBufferPosition );

	lMotion.sequences = new MotionSequence[ lMotion.sequencesCount ];
	for ( int i = 0; i < lMotion.sequencesCount; i++ )
	{
		lMotion.sequences[ i ].firstKeyframeNumber = readInt( lpBufferPosition );
		lMotion.sequences[ i ].keyframesCount = readInt( lpBufferPosition );
	}

	lMotion.meshes = new MotionMesh[ lMotion.meshesCount ];
	for ( int i = 0; i < lMotion.meshesCount; i++ )
	{
		lMotion.meshes[ i ].name = readString( lpBufferPosition );
		lMotion.meshes[ i ].visible = readBool( lpBufferPosition );
		lMotion.meshes[ i ].type = readInt( lpBufferPosition );
		lMotion.meshes[ i ].keyframes = new MotionKeyframe[ lMotion.keyframesCount ];
		for (int j = 0; j < lMotion.keyframesCount; j++ )
		{
			lMotion.meshes[i].keyframes[j].number = readInt( lpBufferPosition );
			lMotion.meshes[i].keyframes[j].x = readFloat( lpBufferPosition );
			lMotion.meshes[i].keyframes[j].y = readFloat( lpBufferPosition );
			lMotion.meshes[i].keyframes[j].z = readFloat( lpBufferPosition );
			lMotion.meshes[i].keyframes[j].qx = readFloat( lpBufferPosition );
			lMotion.meshes[i].keyframes[j].qy = readFloat( lpBufferPosition );
			lMotion.meshes[i].keyframes[j].qz = readFloat( lpBufferPosition );
			lMotion.meshes[i].keyframes[j].qw = readFloat( lpBufferPosition );
		}
		loadMotionMesh( apFileName, lMotion.meshes + i );
	}

	////
	// fbx creation
	createMotionMeshNode( apScene, &lMotion );
	////

	for ( int i = 0; i < lMotion.meshesCount; i++ )
	{
		for ( int j = 0; j < lMotion.meshes[ i ].submeshesCount; j++ )
		{
			delete lMotion.meshes[ i ].submeshes[ j ].faces;
			delete lMotion.meshes[ i ].submeshes[ j ].vertices;
			delete lMotion.meshes[ i ].submeshes[ j ].materialName;
		}
		delete lMotion.meshes[ i ].name;
		delete lMotion.meshes[ i ].keyframes;
		delete lMotion.meshes[ i ].submeshes;
	}
	delete lMotion.sequences;
	delete lMotion.meshes;
	delete lMotion.name;
	delete lpFileBuffer;
}

int _tmain(int argc, _TCHAR* argv[])
{
    FbxManager* lSdkManager = NULL;
    FbxScene* lScene = NULL;
    bool lResult = false;

    InitializeSdkObjects(lSdkManager, lScene);

	//////////////
	// Konwersja jednego pliku *.gfbx tworzonego przez DustConverter
	// Pliki Ÿród³owe s¹ szukane w DustResources a efekty dzia³ania s¹ tworzone w DustAssets. 
	// Tekstury nie s¹ w³¹czane do pliku FBX
	
	//dustLevel2FBX( lScene, "Anasta1" );
	//SaveScene( lSdkManager, lScene, "..//..//DustAssets//levels//Anasta1.fbx",-1,false);

	///////////////
	// Konwersja jednego pliku *.motion wraz zale¿nymi plikami *.mesh tworzonego przez DustConverter
	// Pliki Ÿród³owe s¹ szukane w DustResources a efekty dzia³ania s¹ tworzone w DustAssets. 
	// Tekstury nie s¹ w³¹czane do pliku FBX.
	
	dustMotion2FBX( lScene, "autogun" );
	SaveScene( lSdkManager, lScene, "autogun.fbx", -1, false );

    DestroySdkObjects(lSdkManager, lResult);

	return 0;
}

