// DustFBX.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "fbx.h"
#include <math.h>
#include <stdlib.h>
#include <windows.h>
#include <tchar.h> 
#include <stdio.h>
#include <strsafe.h>
#pragma comment(lib, "User32.lib")

/////////
char** findMotions(int &aFilesCount)
{
   WIN32_FIND_DATA ffd;
   LARGE_INTEGER filesize;
   TCHAR szDir[MAX_PATH];
   size_t length_of_arg;
   HANDLE hFind = INVALID_HANDLE_VALUE;
   DWORD dwError=0;

   StringCchCopy(szDir, MAX_PATH, TEXT("..\\..\\DustResources\\fbx\\*.motion"));

   hFind = FindFirstFile(szDir, &ffd);
   
   aFilesCount = 0;
   char** lFiles = new char*[1024];

   if ( hFind != INVALID_HANDLE_VALUE )
   {
	   do
	   {
		   lFiles[aFilesCount] = new char[256];
		   wcstombs(lFiles[aFilesCount],ffd.cFileName,255);
		   aFilesCount++;
	   }
	   while (FindNextFile(hFind, &ffd) != 0);

	   FindClose(hFind);
   }
   return lFiles;
}

/////////

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
	int texture1FramesCount;	
	int texture2FramesCount;
	int transparent;
	int type;
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

void createSectorNode( FbxScene* apScene, Sector* apSector, FILE* apTagsFile )
{
	char lName[256];
	sprintf( lName, "sec:%d", apSector->id );
	FbxNode* lpSectorNode = FbxNode::Create( apScene, lName );
	apScene->GetRootNode()->AddChild( lpSectorNode );
	static int meshid = 0;
	for (int m = 0; m < apSector->meshesCount; m++ )
	{
		sprintf( lName, "msh:%d", meshid++ );
		
		FbxString lMaterialName = apSector->meshes[ m ].textureName;
		FbxString lTextureName = apSector->meshes[ m ].textureName;
		lTextureName.Append(".png",4);
			
		char tag[256];
		sprintf( tag, "%s-I%05d,A%02d",lName,apSector->meshes[ m ].id,apSector->meshes[ m ].texture1FramesCount );
		if ( apSector->meshes[m].transparent)
			strcat( tag, ",T");
		if ( apSector->meshes[m].texture2FramesCount > 0 )
		{
			char tag2[256];
			sprintf( tag2, ",S%02d", apSector->meshes[m].texture2FramesCount );
			strcat( tag, tag2 );
		}
		strcat( tag, "\n");
		fputs( tag, apTagsFile );
		
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

//////////////
// Konwersja jednego pliku *.gfbx tworzonego przez DustConverter
// Pliki Ÿród³owe s¹ szukane podkatalogu fbx katalogu DustResources, podkatalog ten musi byæ jako working directory
// Pliki docelowe tworzone s¹ w katalogu DustUnity//Assets
// Plik FBX zawiera tylko geometriê - bez tekstur
	
void dustLevel2FBX( FbxScene* apScene, char* apFileName )
{
	char lSource[256];

	sprintf( lSource, "%s.geometry", apFileName);
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
			lpMeshes[ m ].texture1FramesCount = readInt( lpBufferPosition );
			lpMeshes[ m ].texture2FramesCount = readInt( lpBufferPosition );
			lpMeshes[ m ].transparent = readInt( lpBufferPosition );
			lpMeshes[ m ].type = readInt( lpBufferPosition );
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
	char lTagsFileName[256];
	sprintf( lTagsFileName, "..//..//DustUnity//Assets//tmp//%s.tag", apFileName );
	FILE* lpTagsFile = fopen( lTagsFileName, "w" );
	for(int s = 0; s < lSectorsCount; s++ )
		if ( lpSectors[ s ].meshesCount > 0 )
			createSectorNode( apScene, lpSectors + s, lpTagsFile );
	fclose( lpTagsFile);
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
	float yaw, pitch, roll;
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
	char lTmp[256];
	strcpy( lTmp, apMotionName );
	strrchr( lTmp, '.')[0] = 0;

	sprintf( lSource, "%s_%s.mesh", lTmp, apMesh->name);
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
	
	FbxAnimStack** lpAnimations = new FbxAnimStack*[ apMotion->sequencesCount ];
	
	for (int a = 0; a < apMotion->sequencesCount; a++ )
	{
		char lSequenceName[256]; sprintf( lSequenceName, "seq:%d", a );
		lpAnimations[ a ] = FbxAnimStack::Create( apScene, lSequenceName );
		FbxAnimLayer* lpLayer = FbxAnimLayer::Create( apScene, "base" );
		lpAnimations[ a ]->AddMember( lpLayer );

		FbxAnimCurveNode* lpCurveNode = FbxAnimCurveNode::CreateTypedCurveNode( lpSectorNode->LclTranslation, apScene );
		lpLayer->AddMember( lpCurveNode );	
		lpCurveNode = FbxAnimCurveNode::CreateTypedCurveNode( lpSectorNode->LclRotation, apScene );
		lpLayer->AddMember( lpCurveNode );
	}

	for (int m = 0; m < apMotion->meshesCount; m++ )
	{
		FbxNode* lpMeshNode = FbxNode::Create( apScene, apMotion->meshes[ m ].name );
		lpSectorNode->AddChild( lpMeshNode );

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
		
				FbxNode* lpSubmeshNode = FbxNode::Create( apScene, lName );
				FbxMesh* lpMesh = FbxMesh::Create( apScene, lName );
				FbxSurfaceLambert* lpMaterial = FbxSurfaceLambert::Create( apScene, lMaterialName.Buffer());
				FbxFileTexture* lpTexture = FbxFileTexture::Create(apScene,lTextureName.Buffer());
		
				lpSubmeshNode->AddMaterial(lpMaterial);
				lpSubmeshNode->SetNodeAttribute( lpMesh );
				lpSubmeshNode->SetShadingMode( FbxNode::eTextureShading );
				lpMaterial->Diffuse.ConnectSrcObject(lpTexture);

				setupMotionMeshGeometry( lpMesh, &apMotion->meshes[ m ].submeshes[ sm ] );
				setupFbxMeshMaterial( lpMaterial, lpMesh, lMaterialName );
				setupFbxMeshTexture( lpTexture, lTextureName );

				lpMeshNode->AddChild( lpSubmeshNode );
			}
				
		if ( apMotion->sequences[ 0 ].keyframesCount > 0 )
		{
			MotionKeyframe *key = &apMotion->meshes[ m ].keyframes[ apMotion->sequences[ 0 ].firstKeyframeNumber + 0 ];
			FbxDouble3 lT, lR;
			lT[0] = key->x;
			lT[1] = key->y;
			lT[2] = key->z;
			lR[0] = key->pitch * 180.0 / 3.1415927f;
			lR[1] = key->yaw * 180.0 / 3.1415927f;
			lR[2] = key->roll * 180.0 / 3.1415927f;
			lpMeshNode->LclTranslation.Set( lT );
			lpMeshNode->LclRotation.Set( lR );
		}
			
		for (int a = 0; a < apMotion->sequencesCount; a++ )
		{
			FbxAnimLayer* lpLayer = (FbxAnimLayer*)lpAnimations[ a ]->GetMember( 0 );

			FbxAnimCurve* lpCurveTX = lpMeshNode->LclTranslation.GetCurve( lpLayer, "X", true );
			FbxAnimCurve* lpCurveTY = lpMeshNode->LclTranslation.GetCurve( lpLayer, "Y", true );
			FbxAnimCurve* lpCurveTZ = lpMeshNode->LclTranslation.GetCurve( lpLayer, "Z", true );
			FbxAnimCurve* lpCurveRP = lpMeshNode->LclRotation.GetCurve( lpLayer, "X", true );
			FbxAnimCurve* lpCurveRY = lpMeshNode->LclRotation.GetCurve( lpLayer, "Y", true );
			FbxAnimCurve* lpCurveRR = lpMeshNode->LclRotation.GetCurve( lpLayer, "Z", true );
					
			FbxTime lTime;
			FbxAnimCurveKey lCurveKey;

			lpCurveTX->KeyModifyBegin();
			lpCurveTY->KeyModifyBegin();
			lpCurveTZ->KeyModifyBegin();
			lpCurveRY->KeyModifyBegin();
			lpCurveRP->KeyModifyBegin();
			lpCurveRR->KeyModifyBegin();
			for (int k = 0; k < apMotion->sequences[ a ].keyframesCount; k++)
			{
				MotionKeyframe *key = &apMotion->meshes[ m ].keyframes[ apMotion->sequences[ a ].firstKeyframeNumber + k ];
				lTime.SetSecondDouble( (double)k/10 );

				lCurveKey.Set( lTime, key->x );
				int idx = lpCurveTX->KeyAdd( lTime, lCurveKey );
				lpCurveTX->KeySetInterpolation( idx, FbxAnimCurveDef::eInterpolationConstant );	

				lCurveKey.Set( lTime, key->y );
				idx = lpCurveTY->KeyAdd( lTime, lCurveKey );
				lpCurveTY->KeySetInterpolation( idx, FbxAnimCurveDef::eInterpolationConstant );	
						
				lCurveKey.Set( lTime, key->z );
				idx = lpCurveTZ->KeyAdd( lTime, lCurveKey );
				lpCurveTZ->KeySetInterpolation( idx, FbxAnimCurveDef::eInterpolationConstant );	

				lCurveKey.Set( lTime, key->roll * 180.0f / 3.1415927f );
				idx = lpCurveRR->KeyAdd( lTime, lCurveKey );
				lpCurveRR->KeySetInterpolation( idx, FbxAnimCurveDef::eInterpolationConstant );	
						
				lCurveKey.Set( lTime, key->pitch * 180.0f / 3.1415927f );
				idx = lpCurveRP->KeyAdd( lTime, lCurveKey );
				lpCurveRP->KeySetInterpolation( idx, FbxAnimCurveDef::eInterpolationConstant );	

				lCurveKey.Set( lTime, key->yaw * 180.0f / 3.1415927f );
				idx = lpCurveRY->KeyAdd( lTime, lCurveKey );
				lpCurveRY->KeySetInterpolation( idx, FbxAnimCurveDef::eInterpolationConstant );	
			}
			lpCurveTX->KeyModifyEnd();
			lpCurveTY->KeyModifyEnd();
			lpCurveTZ->KeyModifyEnd();
			lpCurveRY->KeyModifyEnd();
			lpCurveRP->KeyModifyEnd();
			lpCurveRR->KeyModifyEnd();				
		}
	}

	delete lpAnimations;
}

///////////////
// Konwersja jednego pliku *.motion wraz zale¿nymi plikami *.mesh tworzonych przez DustConverter
// Pliki Ÿród³owe s¹ szukane podkatalogu fbx katalogu DustResources, podkatalog ten musi byæ jako working directory
// Pliki docelowe tworzone s¹ w katalogu DustUnity//Assets
// Plik FBX zawiera tekstury, geometriê i animacje
	
void dustMotion2FBX( FbxScene* apScene, char* apFileName )
{
	char lSource[256];

	sprintf( lSource, "%s", apFileName);
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

	if ( lMotion.sequencesCount == 1 )
		if ( lMotion.sequences[0].keyframesCount == 0)
			lMotion.sequences[0].keyframesCount = lMotion.keyframesCount;

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
			lMotion.meshes[i].keyframes[j].pitch = readFloat( lpBufferPosition );
			lMotion.meshes[i].keyframes[j].yaw = readFloat( lpBufferPosition );
			lMotion.meshes[i].keyframes[j].roll = readFloat( lpBufferPosition );
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
	int lLevelsCount = 0 ;//18;	bool lLevel = true;

    FbxManager* lSdkManager = NULL;
    FbxScene* lScene = NULL;
    bool lResult = false;

	char* lLevels[] = { "ANASTA1" };
                        //"ANASTA1", "ANASTA2", "GOLEB1", "GOLEB2", "GOLEB3", "GOLEB4", "GOLEB5", 
                        //"PLAT1", "PLAT2", "PLAT3", "PLAT4", "PLAT5", "PLAT6","PLAT7","PLAT8",
                        //"KANYON", "WALKIRIE", "OUTRO" };
	for (int i = 0; i < lLevelsCount; i++ )
	{
		InitializeSdkObjects(lSdkManager, lScene);
		dustLevel2FBX( lScene, lLevels[i] );
		char out[256];
		sprintf( out, "..//..//DustUnity//Assets//Levels//%s.fbx", lLevels[i]);
		SaveScene( lSdkManager, lScene, out,-1, false);
		DestroySdkObjects(lSdkManager, lResult);
	}

	int lMotionsCount;
	char** lMotions = findMotions(lMotionsCount);
		
	for (int i = 0; i < lMotionsCount; i ++ )
	{
		InitializeSdkObjects(lSdkManager, lScene);
		dustMotion2FBX( lScene, lMotions[ i ] );
		char out[256];
		char tmp[256];
		strcpy( tmp, lMotions[i] );
		strrchr( tmp, '.')[0] = 0;
		sprintf( out, "..//..//DustUnity//Assets//Objects//%s.fbx", tmp);
		SaveScene( lSdkManager, lScene,out, -1, true );
		DestroySdkObjects(lSdkManager, lResult);
	}
	return 0;
}

