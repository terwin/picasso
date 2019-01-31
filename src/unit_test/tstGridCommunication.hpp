#include <Harlow_GridCommunication.hpp>
#include <Harlow_Types.hpp>
#include <Harlow_GlobalGrid.hpp>
#include <Harlow_GridField.hpp>
#include <Harlow_GridExecPolicy.hpp>

#include <Kokkos_Core.hpp>

#include <gtest/gtest.h>

#include <mpi.h>

using namespace Harlow;

namespace Test
{
//---------------------------------------------------------------------------//
// Check the gather.
template<class ViewType>
void checkGather( const GridBlock& block,
                  typename ViewType::value_type data_val,
                  ViewType field )
{
    // -I face.
    if ( !block.onBoundary(DomainBoundary::LowX) || block.isPeriodic(Dim::I) )
        for ( int j = block.localCellBegin(Dim::J);
              j < block.localCellEnd(Dim::J); ++j )
            for ( int k = block.localCellBegin(Dim::K);
                  k < block.localCellEnd(Dim::K); ++k )
                EXPECT_EQ( field(0,j,k), data_val );

    // +I face.
    if ( !block.onBoundary(DomainBoundary::HighX) || block.isPeriodic(Dim::I) )
        for ( int j = block.localCellBegin(Dim::J);
              j < block.localCellEnd(Dim::J); ++j )
            for ( int k = block.localCellBegin(Dim::K);
                  k < block.localCellEnd(Dim::K); ++k )
                EXPECT_EQ( field(block.numCell(Dim::I)-1,j,k), data_val );

    // -J face.
    if ( !block.onBoundary(DomainBoundary::LowY) || block.isPeriodic(Dim::J) )
        for ( int i = block.localCellBegin(Dim::I);
              i < block.localCellEnd(Dim::I); ++i )
            for ( int k = block.localCellBegin(Dim::K);
                  k < block.localCellEnd(Dim::K); ++k )
                EXPECT_EQ( field(i,0,k), data_val );

    // +J face.
    if ( !block.onBoundary(DomainBoundary::HighY) || block.isPeriodic(Dim::J) )
        for ( int i = block.localCellBegin(Dim::I);
              i < block.localCellEnd(Dim::I); ++i )
            for ( int k = block.localCellBegin(Dim::K);
                  k < block.localCellEnd(Dim::K); ++k )
                EXPECT_EQ( field(i,block.numCell(Dim::J)-1,k), data_val );

    // -K face.
    if ( !block.onBoundary(DomainBoundary::LowZ) || block.isPeriodic(Dim::K) )
        for ( int i = block.localCellBegin(Dim::I);
              i < block.localCellEnd(Dim::I); ++i )
        for ( int j = block.localCellBegin(Dim::J);
              j < block.localCellEnd(Dim::J); ++j )
                EXPECT_EQ( field(i,j,0), data_val );

    // +K face.
    if ( !block.onBoundary(DomainBoundary::HighZ) || block.isPeriodic(Dim::K) )
        for ( int i = block.localCellBegin(Dim::I);
              i < block.localCellEnd(Dim::I); ++i )
            for ( int j = block.localCellBegin(Dim::J);
                  j < block.localCellEnd(Dim::J); ++j )
                EXPECT_EQ( field(i,j,block.numCell(Dim::K)-1), data_val );
}

//---------------------------------------------------------------------------//
// Check the scatter. Note that some halo cells go to multiple neighbors so
// these will get multiple contributions.
template<class ViewType>
void checkScatter( const GridBlock& block,
                   typename ViewType::value_type data_val,
                   ViewType field )
{
    // Start with the interior halo values. Interior values should have
    // received a contribution from only one neighbor.

    // -I face.
    if ( !block.onBoundary(DomainBoundary::LowX) || block.isPeriodic(Dim::I) )
        for ( int j = block.localCellBegin(Dim::J) + 1;
              j < block.localCellEnd(Dim::J) - 1; ++j )
            for ( int k = block.localCellBegin(Dim::K) + 1;
                  k < block.localCellEnd(Dim::K) - 1; ++k )
            {
                int i = 1;
                EXPECT_EQ( field(i,j,k), 2*data_val );
            }

    // +I face.
    if ( !block.onBoundary(DomainBoundary::HighX) || block.isPeriodic(Dim::I) )
        for ( int j = block.localCellBegin(Dim::J) + 1;
              j < block.localCellEnd(Dim::J) - 1; ++j)
            for ( int k = block.localCellBegin(Dim::K) + 1;
                  k < block.localCellEnd(Dim::K) - 1; ++k )
            {
                int i = block.numCell(Dim::I)-2;
                EXPECT_EQ( field(i,j,k), 2*data_val );
            }

    // -J face.
    if ( !block.onBoundary(DomainBoundary::LowY) || block.isPeriodic(Dim::J) )
        for ( int i = block.localCellBegin(Dim::I) + 1;
              i < block.localCellEnd(Dim::I) - 1; ++i )
            for ( int k = block.localCellBegin(Dim::K) + 1;
                  k < block.localCellEnd(Dim::K) - 1; ++k )
            {
                int j = 1;
                EXPECT_EQ( field(i,j,k), 2*data_val );
            }

    // +J face.
    if ( !block.onBoundary(DomainBoundary::HighY) || block.isPeriodic(Dim::J) )
        for ( int i = block.localCellBegin(Dim::I) + 1;
              i < block.localCellEnd(Dim::I) - 1; ++i )
            for ( int k = block.localCellBegin(Dim::K) + 1;
                  k < block.localCellEnd(Dim::K) - 1; ++k )
            {
                int j = block.numCell(Dim::J)-2;
                EXPECT_EQ( field(i,j,k), 2*data_val );
            }

    // -K face.
    if ( !block.onBoundary(DomainBoundary::LowZ) || block.isPeriodic(Dim::K) )
        for ( int i = block.localCellBegin(Dim::I) + 1;
              i < block.localCellEnd(Dim::I) - 1; ++i )
        for ( int j = block.localCellBegin(Dim::J) + 1;
              j < block.localCellEnd(Dim::J) - 1; ++j )
            {
                int k = 1;
                EXPECT_EQ( field(i,j,k), 2*data_val );
            }

    // +K face.
    if ( !block.onBoundary(DomainBoundary::HighZ) || block.isPeriodic(Dim::K) )
        for ( int i = block.localCellBegin(Dim::I) + 1;
              i < block.localCellEnd(Dim::I) - 1; ++i )
            for ( int j = block.localCellBegin(Dim::J) + 1;
                  j < block.localCellEnd(Dim::J) - 1; ++j )
            {
                int k = block.numCell(Dim::K)-2;
                EXPECT_EQ( field(i,j,k), 2*data_val );
            }
}

//---------------------------------------------------------------------------//
void gatherScatterTest( const std::vector<int>& ranks_per_dim,
                        const std::vector<bool>& is_dim_periodic )
{
    // Create the global grid.
    double cell_size = 0.23;
    std::vector<int> global_num_cell = { 101, 85, 99 };
    std::vector<double> global_low_corner = { 1.2, 3.3, -2.8 };
    std::vector<double> global_high_corner =
        { global_low_corner[0] + cell_size * global_num_cell[0],
          global_low_corner[1] + cell_size * global_num_cell[1],
          global_low_corner[2] + cell_size * global_num_cell[2] };
    auto global_grid = std::make_shared<GlobalGrid>(
        MPI_COMM_WORLD,
        ranks_per_dim,
        is_dim_periodic,
        global_low_corner,
        global_high_corner,
        cell_size );

    // Create a scalar cell field on the grid.
    int halo_width = 1;
    GridField<double,TEST_MEMSPACE> grid_field(
        global_grid,
        FieldLocation::Cell,
        halo_width,
        "TestField" );

    // Fill the locally owned field with data.
    double data_val = 2.3;
    auto field = grid_field.data();
    Kokkos::parallel_for(
        "test field fill",
        createLocalCellExecPolicy<TEST_EXECSPACE>(grid_field.block()),
        KOKKOS_LAMBDA(const int i, const int j, const int k){
            field(i,j,k) = data_val; }
        );

    // Gather into the halo.
    GridCommunication::gather( grid_field );

    // Check the gather. The halo should have received the data value if its
    // not on a physical boundary or if it is on a periodic boundary.
    auto field_mirror =
        Kokkos::create_mirror_view_and_copy( Kokkos::HostSpace(), field );
    checkGather( grid_field.block(), data_val, field_mirror );

    // Now scatter back.
    GridCommunication::scatter( grid_field );

    // Check the scatter. The interior nodes on the boundary should now have
    // 2x the data value if not on a physical boundary or if the boundary is
    // periodic.
    Kokkos::deep_copy( field_mirror, field );
    checkScatter( grid_field.block(), data_val, field_mirror );
}

//---------------------------------------------------------------------------//
// RUN TESTS
//---------------------------------------------------------------------------//
TEST_F( TEST_CATEGORY, not_periodic_test )
{
    // Let MPI compute the partitioning for this test.
    int comm_size;
    MPI_Comm_size( MPI_COMM_WORLD, &comm_size );
    std::vector<int> ranks_per_dim( 3 );
    MPI_Dims_create( comm_size, 3, ranks_per_dim.data() );

    // Boundaries are not periodic.
    std::vector<bool> is_dim_periodic = {false,false,false};

    // Test with different block configurations to make sure all the
    // dimensions get partitioned even at small numbers of ranks.
    gatherScatterTest( ranks_per_dim, is_dim_periodic );
    std::swap( ranks_per_dim[0], ranks_per_dim[1] );
    gatherScatterTest( ranks_per_dim, is_dim_periodic );
    std::swap( ranks_per_dim[0], ranks_per_dim[2] );
    gatherScatterTest( ranks_per_dim, is_dim_periodic );
    std::swap( ranks_per_dim[1], ranks_per_dim[2] );
    gatherScatterTest( ranks_per_dim, is_dim_periodic );
    std::swap( ranks_per_dim[0], ranks_per_dim[1] );
    gatherScatterTest( ranks_per_dim, is_dim_periodic );
}

//---------------------------------------------------------------------------//
TEST_F( TEST_CATEGORY, periodic_test )
{
    // Let MPI compute the partitioning for this test.
    int comm_size;
    MPI_Comm_size( MPI_COMM_WORLD, &comm_size );
    std::vector<int> ranks_per_dim( 3 );
    MPI_Dims_create( comm_size, 3, ranks_per_dim.data() );

    // Every boundary is periodic
    std::vector<bool> is_dim_periodic = {true,true,true};

    // Test with different block configurations to make sure all the
    // dimensions get partitioned even at small numbers of ranks.
    gatherScatterTest( ranks_per_dim, is_dim_periodic );
    std::swap( ranks_per_dim[0], ranks_per_dim[1] );
    gatherScatterTest( ranks_per_dim, is_dim_periodic );
    std::swap( ranks_per_dim[0], ranks_per_dim[2] );
    gatherScatterTest( ranks_per_dim, is_dim_periodic );
    std::swap( ranks_per_dim[1], ranks_per_dim[2] );
    gatherScatterTest( ranks_per_dim, is_dim_periodic );
    std::swap( ranks_per_dim[0], ranks_per_dim[1] );
    gatherScatterTest( ranks_per_dim, is_dim_periodic );
}

//---------------------------------------------------------------------------//

} // end namespace Test