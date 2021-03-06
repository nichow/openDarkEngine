/******************************************************************************
 *
 *    This file is part of openDarkEngine project
 *    Copyright (C) 2009 openDarkEngine team
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *	  $Id$
 *
 *****************************************************************************/

/** @file FileCompat.h
 * @brief A various utility methods for File class usage
 */

#ifndef __FILECOMPAT_H
#define __FILECOMPAT_H

#include "File.h"
#include "Plane.h"
#include "Quaternion.h"
#include "Vector2.h"
#include "Vector3.h"
#include "config.h"

namespace Opde {
// Vector2
File &operator<<(File &st, const Vector2 &val);
File &operator>>(File &st, Vector2 &val);

// Vector3
File &operator<<(File &st, const Vector3 &val);
File &operator>>(File &st, Vector3 &val);

// Plane
File &operator<<(File &st, const Plane &val);
File &operator>>(File &st, Plane &val);

// Quaternion
File &operator<<(File &st, const Quaternion &val);
File &operator>>(File &st, Quaternion &val);
} // namespace Opde

#endif
