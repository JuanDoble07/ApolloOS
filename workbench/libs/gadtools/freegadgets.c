/*
    (C) 1997 AROS - The Amiga Replacement OS
    $Id$

    Desc:
    Lang: english
*/
#include "gadtools_intern.h"

/*********************************************************************

    NAME */
#include <proto/gadtools.h>
#include <intuition/intuition.h>

	AROS_LH1(VOID, FreeGadgets,

/*  SYNOPSIS */
	AROS_LHA(struct Gadget *, glist, A0),

/*  LOCATION */
	struct Library *, GadtoolsBase, 6, Gadtools)

/*  FUNCTION
	Frees all gadtools gadgets from a linked list of gadgets.

    INPUTS
	glist - pointer to the first gadget to be freed

    RESULT

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO
	CreateGadgetA()

    INTERNALS

    HISTORY

***************************************************************************/
{
    AROS_LIBFUNC_INIT
    AROS_LIBBASE_EXT_DECL(struct GadtoolsBase *,GadtoolsBase)

    struct Gadget *lastgad = NULL, *nextgad;

    if (glist == NULL)
	return;

    for (;nextgad;glist=nextgad)
    {
	nextgad = glist->NextGadget;
	if ((glist->Flags & GTYP_GADTOOLS) == GTYP_GADTOOLS)
	    DisposeObject(glist);
	else
	{
	    if (lastgad != NULL)
		lastgad->NextGadget = glist;
	    lastgad = glist;
	}
    }

    if (lastgad != NULL)
	lastgad->NextGadget = NULL;

    AROS_LIBFUNC_EXIT
} /* FreeGadgets */
