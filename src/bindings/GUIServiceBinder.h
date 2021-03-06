/******************************************************************************
 *
 *    This file is part of openDarkEngine project
 *    Copyright (C) 2005-2009 openDarkEngine team
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
 *
 *	   $Id$
 *
 *****************************************************************************/

#ifndef __GUISERVICEBINDER_H
#define __GUISERVICEBINDER_H

#include "bindings.h"

#include "gui/GUIService.h"

namespace Opde {

namespace Python {

/// GUI service python binder
class GUIServiceBinder : public shared_ptr_binder<GUIServicePtr> {
public:
    static void init(PyObject *module);

    // --- Python type related methods ---
    static PyObject *create();

    // --- Methods ---
    static PyObject *setActive(PyObject *self, PyObject *args);
    static PyObject *setVisible(PyObject *self, PyObject *args);
    static PyObject *getActiveSheet(PyObject *self, PyObject *args);
    static PyObject *setActiveSheet(PyObject *self, PyObject *args);
    static PyObject *createSheet(PyObject *self, PyObject *args);
    static PyObject *destroySheet(PyObject *self, PyObject *args);

protected:
    /// Static type definition for GUIService
    static PyTypeObject msType;

    /// Name of the python type
    static const char *msName;

    /// Method list
    static PyMethodDef msMethods[];
};

} // namespace Python
} // namespace Opde

#endif
