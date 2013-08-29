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

void setupSectorMeshGeometry( FbxMesh* apDst, Mesh* apSrc )
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

void setupSectorMeshMaterial( FbxSurfaceLambert* apMaterial, FbxMesh* apMesh, FbxString aMaterialName )
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

void setupSectorMaterialTexture( FbxFileTexture* apTexture, FbxString aTextureName )
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
		setupSectorMeshMaterial( lpMaterial, lpMesh, lMaterialName );
		setupSectorMaterialTexture( lpTexture, lTextureName );

		lpSectorNode->AddChild( lpMeshNode );
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


	for(int s = 0; s < lSectorsCount; s++ )
		if ( lpSectors[ s ].meshesCount > 0 )
			createSectorNode( apScene, lpSectors + s );

	delete lpFileBuffer;
}

int _tmain(int argc, _TCHAR* argv[])
{
    FbxManager* lSdkManager = NULL;
    FbxScene* lScene = NULL;
    bool lResult = false;

    InitializeSdkObjects(lSdkManager, lScene);

	dust2FBX( lScene, "Anasta1" );

	SaveScene( lSdkManager, lScene, "Anasta1.fbx",-1,false);

    DestroySdkObjects(lSdkManager, lResult);

	return 0;
}

