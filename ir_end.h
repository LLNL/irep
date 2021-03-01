// Copyright 2016-2021 Lawrence Livermore National Security, LLC and other
// IREP Project Developers. See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: MIT

// Include this file at the end of each wkt_*.h file.

#if defined(IREP_LANG_FORTRAN)
end module

#elif defined(__cplusplus)
} /* end extern "C" */
} /* end namespace irep */
#endif
