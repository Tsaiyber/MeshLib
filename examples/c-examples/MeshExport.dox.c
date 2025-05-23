#include <MRMeshC/MRMesh.h>
#include <MRMeshC/MRVector3.h>
#include <MRMeshC/MRCube.h>
#include <MRMeshC/MRMeshTopology.h>
#include <MRMeshC/MRString.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main( int argc, char* argv[] )
{
    // error messages will be stored here
    MRString* errorString = NULL;

    // Create mesh
    MRVector3f size = mrVector3fDiagonal( 1.f );
    MRVector3f base = mrVector3fDiagonal( -0.5f );
    MRMesh* mesh = mrMakeCube( &size, &base );

    // extract vertices
    const MRVector3f* vertices = mrMeshPoints( mesh );
    size_t verticesNum = mrMeshPointsNum( mesh );
    // you can access coordinates via struct fields...
    printf( "Vertices coordinates:\n" );
    for ( size_t i = 0; i < verticesNum; ++i )
        printf( "Vertex %d coordinates: %f; %f; %f\n", (int)i, vertices[i].x, vertices[i].y, vertices[i].z );
    // ...or cast them to a row-major float array
    float* vertexData = malloc( sizeof( float ) * 3 * verticesNum );
    memcpy( vertexData, vertices, sizeof( MRVector3f ) * verticesNum );

    // extract faces
    MRTriangulation* t = mrMeshGetTriangulation( mesh );
    const MRThreeVertIds* faces = t->data;
    size_t facesNum = t->size;
    // faces are stored as vertex id triples...
    printf( "Face's vertex ids:\n" );
    for ( size_t i = 0; i < verticesNum; ++i )
        printf( "Face %d vertex ids: %d, %d, %d\n", (int)i, faces[i][0].id, faces[i][1].id, faces[i][2].id );
    // ...and can also be cast to an integer array
    int* faceData = malloc( sizeof( int ) * 3 * facesNum );
    memcpy( faceData, faces, sizeof( MRThreeVertIds ) * facesNum );

    mrMeshFree( mesh );
    free( vertexData );
    mrTriangulationFree( t );
    free( faceData );
    return EXIT_SUCCESS;
}
