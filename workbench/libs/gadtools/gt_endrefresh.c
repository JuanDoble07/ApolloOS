/*
    (C) 1997 AROS - The Amiga Replacement OS
    $Id$

    Desc:
    Lang: english
*/
#include <proto/intuition.h>
#include "gadtools_intern.h"

/*********************************************************************

    NAME */
#include <proto/gadtools.h>

        AROS_LH2(void, GT_EndRefresh,

/*  SYNOPSIS */
	AROS_LHA(struct Window *, win, A0),
        AROS_LHA(BOOL, complete, D0),

/*  LOCATION */
	struct Library *, GadtoolsBase, 16, Gadtools)

/*  FUNCTION
        GT_EndRefresh() must be called, when a refresh initiated with
        GT_BeginRefresh() is done.

    INPUTS
        win      - the refreshed window
        complete - TRUE, if refreshing is finished

    RESULT

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO
        GT_BeginRefresh(), intuition.library/EndRefresh()

    INTERNALS

    HISTORY

***************************************************************************/
{
    AROS_LIBFUNC_INIT
    AROS_LIBBASE_EXT_DECL(struct GadtoolsBase *,GadtoolsBase)

    EndRefresh(win, complete);

    AROS_LIBFUNC_EXIT
} /* GT_EndRefresh */
