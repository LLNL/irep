#ifndef wkt_mesh_h
#define wkt_mesh_h
#include "ir_start.h"

Beg_struct(irt_box)
  ir_int(nx, 1)        Doc(( Number of grid cells in 1st dimension ))
  ir_int(ny, 1)        Doc(( Number of grid cells in 2nd dimension ))
  ir_int(nz, 1)        Doc(( Number of grid cells in 3rd dimension ))
  ir_dbl(xmin, 0.0)    Doc(( Location of left-most cell face ))
  ir_dbl(xmax, 1.0)    Doc(( Location of right-most cell face ))
  ir_dbl(ymin, 0.0)    Doc(( Location of nearest cell face ))
  ir_dbl(ymax, 1.0)    Doc(( Location of farthest cell face ))
  ir_dbl(zmin, 0.0)    Doc(( Location of lowest cell face ))
  ir_dbl(zmax, 1.0)    Doc(( Location of highest cell face ))
End_struct(irt_box)

Beg_struct(irt_mesh)
  ir_str(file, FILENAMESIZE, 'none')  Doc(( Name of mesh ))
  ir_int(refinement_level, 0)
  ir_log(amr, false)                  Doc(( Use amr? ))
  Structure(irt_box,box)
End_struct(irt_mesh)

// Declare the structure
ir_wkt(irt_mesh, mesh)

#include "ir_end.h"
#endif  // wkt_mesh_h
