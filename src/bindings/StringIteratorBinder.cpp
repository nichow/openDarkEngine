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

#include "StringIteratorBinder.h"
#include "bindings.h"

namespace Opde {

namespace Python {

// -------------------- String iterator --------------------
const char *StringIteratorBinder::msName = "StringIterator";

// ------------------------------------------
PyTypeObject StringIteratorBinder::msType = {
    PyVarObject_HEAD_INIT(&PyType_Type,
                          0) "opde.StringIterator", /* char *tp_name; */
    sizeof(StringIteratorBinder::Base),           /* int tp_basicsize; */
    0,                             // int tp_itemsize;       /* not used much */
    StringIteratorBinder::dealloc, /* destructor tp_dealloc; */
    0,                             /* printfunc  tp_print;   */
    0,                             // getattrfunc  tp_getattr; /* __getattr__ */
    0,                       // setattrfunc  tp_setattr;  /* __setattr__ */
    0,                       // cmpfunc  tp_compare;  /* __cmp__ */
    repr,                    // reprfunc  tp_repr;    /* __repr__ */
    0,                       /* PyNumberMethods *tp_as_number; */
    0,                       /* PySequenceMethods *tp_as_sequence; */
    0,                       /* PyMappingMethods *tp_as_mapping; */
    0,                       // hashfunc tp_hash;     /* __hash__ */
    0,                       // ternaryfunc tp_call;  /* __call__ */
    0,                       // reprfunc tp_str;      /* __str__ */
    PyObject_GenericGetAttr, // getattrofunc tp_getattro; */
    0,                       /* setattrofunc tp_setattro; */
    0,                       /* PyBufferProcs *tp_as_buffer; */
    Py_TPFLAGS_DEFAULT,      /* long tp_flags; */
    0,                       /* char *tp_doc;  */
    0,                       /* traverseproc tp_traverse; */
    0,                       /* inquiry tp_clear; */
    0,                       /* richcmpfunc tp_richcompare; */
    0,                       /* long tp_weaklistoffset; */
    getIterObject,           /* getiterfunc tp_iter; */
    getNext,                 /* iternextfunc tp_iternext; */
    msMethods,               /* struct PyMethodDef *tp_methods; */
    0,                       /* struct memberlist *tp_members; */
    0,                       /* struct getsetlist *tp_getset; */
};

// ------------------------------------------
PyMethodDef StringIteratorBinder::msMethods[] = {
    {NULL, NULL},
};

// ------------------------------------------
PyObject *StringIteratorBinder::getIterObject(PyObject *self) {
    Py_INCREF(self);
    return self;
}

// ------------------------------------------
PyObject *StringIteratorBinder::getNext(PyObject *self) {
    StringIteratorPtr o;

    if (!python_cast<StringIteratorPtr>(self, &msType, &o))
        __PY_CONVERR_RET;

    // Get returnable object, advance to next.
    PyObject *next = NULL;

    if (o && !o->end()) {
        const std::string &s = o->next();

#ifdef IS_PY3K
        next = PyBytes_FromString(s.c_str());
#else
        next = PyString_FromString(s.c_str());
#endif
    }

    return next;
}

// ------------------------------------------
PyObject *StringIteratorBinder::repr(PyObject *self) {
#ifdef IS_PY3K
    return PyBytes_FromFormat("<StringIterator at %p>", self);
#else
    return PyString_FromFormat("<StringIterator at %p>", self);
#endif
}

// ------------------------------------------
PyObject *StringIteratorBinder::create(StringIteratorPtr &strit) {
    Base *object = construct(&msType);

    if (object != NULL) {
        object->mInstance = strit;
    }

    return (PyObject *)object;
}

// ------------------------------------------
void StringIteratorBinder::init(PyObject *module) {
    publishType(module, &msType, msName);
}

} // namespace Python

} // namespace Opde
