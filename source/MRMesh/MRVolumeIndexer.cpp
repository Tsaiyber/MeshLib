#include "MRVolumeIndexer.h"
#include "MRBitSet.h"
#include "MRBitSetParallelFor.h"
#include "MRGTest.h"

namespace MR
{

bool VolumeIndexer::hasNeighbour( const Vector3i & pos, OutEdge toNei ) const
{
    switch ( toNei )
    {
    default:
        assert( false );
        [[fallthrough]];
    case OutEdge::Invalid:
        return {};
    case OutEdge::PlusZ:
        return pos.z + 1 < dims_.z;
    case OutEdge::MinusZ:
        return pos.z > 0;
    case OutEdge::PlusY:
        return pos.y + 1 < dims_.y;
    case OutEdge::MinusY:
        return pos.y > 0;
    case OutEdge::PlusX:
        return pos.x + 1 < dims_.x;
    case OutEdge::MinusX:
        return pos.x > 0;
    }
}

void expandVoxelsMask( VoxelBitSet& mask, const VolumeIndexer& indexer, int expansion )
{
    if ( expansion <= 0 )
    {
        assert( false );
        return;
    }
    VoxelBitSet newBitSet( indexer.size() );
    for ( int iter = 0; iter < expansion; ++iter )
    {
        newBitSet.reset();
        BitSetParallelForAll( mask, [&] ( VoxelId vId )
        {
            if ( mask.test( vId ) )
                return;
            VoxelId edgeVid{};
            bool found = false;
            for ( int step = 0; step < int( OutEdge::Count ); ++step )
            {
                edgeVid = indexer.getNeighbor( vId, OutEdge( step ) );
                if ( edgeVid && mask.test( edgeVid ) )
                {
                    found = true;
                    break;
                }
            }
            if ( found )
                newBitSet.set( vId );
        } );
        mask |= newBitSet;
    }
}

void shrinkVoxelsMask( VoxelBitSet& mask, const VolumeIndexer& indexer, int shrinkage /*= 1 */ )
{
    if ( shrinkage <= 0 )
    {
        assert( false );
        return;
    }
    VoxelBitSet newBitSet( indexer.size() );
    for ( int iter = 0; iter < shrinkage; ++iter )
    {
        newBitSet.reset();
        BitSetParallelFor( mask, [&] ( VoxelId vId )
        {
            VoxelId edgeVid{};
            bool found = false;
            for ( int step = 0; step < int( OutEdge::Count ); ++step )
            {
                edgeVid = indexer.getNeighbor( vId, OutEdge( step ) );
                if ( !edgeVid || !mask.test( edgeVid ) )
                {
                    found = true;
                    break;
                }
            }
            if ( found )
                newBitSet.set( vId );
        } );
        mask -= newBitSet;
    }
}

TEST( MRMesh, ExpandShrinkVoxels )
{
    VolumeIndexer indexer( Vector3i::diagonal( 8 ) );
    VoxelBitSet mask( indexer.size() );
    mask.set( indexer.toVoxelId( { 4, 4, 4 } ) );
    mask.set( indexer.toVoxelId( { 4, 4, 5 } ) );

    VoxelBitSet refMask = mask;
    refMask.set( indexer.toVoxelId( { 4, 4, 3 } ) );
    refMask.set( indexer.toVoxelId( { 4, 5, 4 } ) );
    refMask.set( indexer.toVoxelId( { 5, 4, 4 } ) );
    refMask.set( indexer.toVoxelId( { 4, 3, 4 } ) );
    refMask.set( indexer.toVoxelId( { 3, 4, 4 } ) );

    refMask.set( indexer.toVoxelId( { 4, 4, 6 } ) );
    refMask.set( indexer.toVoxelId( { 4, 5, 5 } ) );
    refMask.set( indexer.toVoxelId( { 5, 4, 5 } ) );
    refMask.set( indexer.toVoxelId( { 4, 3, 5 } ) );
    refMask.set( indexer.toVoxelId( { 3, 4, 5 } ) );

    auto storeMask = mask;
    expandVoxelsMask( mask, indexer );
    EXPECT_TRUE( mask.is_subset_of( refMask ) );
    shrinkVoxelsMask( mask, indexer );
    EXPECT_TRUE( mask.is_subset_of( storeMask ) );
}

} //namespace MR
