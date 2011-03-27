/*
    Copyright  2011, The AROS Development Team.
    $Id$
*/

#include <clib/alib_protos.h>

#include <graphics/rpattr.h>
#include <proto/utility.h>
#include <proto/intuition.h>
#include <proto/dos.h>
#include <proto/graphics.h>
#include <string.h>

#include "screendecorclass.h"
#include "drawfuncs.h"
#include "config.h"

#define SETIMAGE_SCR(id) SetImage(data->di.img_##id, &sd->img_##id, truecolor, screen)
#define DELIMAGE_SCR(id) RemoveLUTImage(&sd->img_##id)

struct scrdecor_data
{
    /* This are original images loaded from disk */
    struct DecorImages di;

    UWORD            sbarheight;
    UWORD            slogo_off;
    UWORD            stitle_off;
    UWORD            winbarheight;

    BOOL             outline;
    BOOL             shadow;

    LONG             leftborder, bottomborder, rightborder;
    LONG             lut_col_a, lut_col_d;
    LONG             text_col, shadow_col;
};

static void DisposeScreenSkinning(struct scrdecor_data *data)
{
    DisposeImageContainer(data->di.img_sdepth);
    DisposeImageContainer(data->di.img_sbarlogo);
    DisposeImageContainer(data->di.img_stitlebar);

    DisposeImageContainer(data->di.img_size);
    DisposeImageContainer(data->di.img_close);
    DisposeImageContainer(data->di.img_depth);
    DisposeImageContainer(data->di.img_zoom);
    DisposeImageContainer(data->di.img_mui);
    DisposeImageContainer(data->di.img_popup);
    DisposeImageContainer(data->di.img_snapshot);
    DisposeImageContainer(data->di.img_iconify);
    DisposeImageContainer(data->di.img_lock);
    DisposeImageContainer(data->di.img_up);
    DisposeImageContainer(data->di.img_down);
    DisposeImageContainer(data->di.img_left);
    DisposeImageContainer(data->di.img_right);
    DisposeImageContainer(data->di.img_winbar_normal);
    DisposeImageContainer(data->di.img_border_normal);
    DisposeImageContainer(data->di.img_border_deactivated);
    DisposeImageContainer(data->di.img_verticalcontainer);
    DisposeImageContainer(data->di.img_verticalknob);
    DisposeImageContainer(data->di.img_horizontalcontainer);
    DisposeImageContainer(data->di.img_horizontalknob);

    DisposeImageContainer(data->di.img_menu);
    DisposeImageContainer(data->di.img_menucheck);
    DisposeImageContainer(data->di.img_amigakey);
    DisposeImageContainer(data->di.img_submenu);

    data->di.img_size = NULL;
    data->di.img_close = NULL;
    data->di.img_depth = NULL;
    data->di.img_zoom = NULL;
    data->di.img_mui = NULL;
    data->di.img_popup = NULL;
    data->di.img_snapshot = NULL;
    data->di.img_iconify = NULL;
    data->di.img_lock = NULL;
    data->di.img_up = NULL;
    data->di.img_down = NULL;
    data->di.img_left = NULL;
    data->di.img_right = NULL;
    data->di.img_winbar_normal = NULL;
    data->di.img_border_normal = NULL;
    data->di.img_border_deactivated = NULL;
    data->di.img_verticalcontainer = NULL;
    data->di.img_verticalknob = NULL;
    data->di.img_horizontalcontainer = NULL;
    data->di.img_horizontalknob = NULL;

    data->di.img_sdepth = NULL;
    data->di.img_sbarlogo = NULL;
    data->di.img_stitlebar = NULL;
}

static BOOL InitScreenSkinning(STRPTR path, struct scrdecor_data *data) {
    
    char    buffer[256];
    char    *line, *v;
    BPTR    file;
    BPTR    lock;
    BPTR    olddir = 0;
    lock = Lock(path, ACCESS_READ);
    if (lock)
    {
        olddir = CurrentDir(lock);
    }
    else return FALSE;

    data->leftborder = 4;
    data->rightborder = 4;
    data->bottomborder = 4;

    data->lut_col_a = 0x00cccccc;
    data->lut_col_d = 0x00888888;

    data->outline = FALSE;
    data->shadow = FALSE;

    data->text_col = 0x00cccccc;
    data->shadow_col = 0x00444444;

    file = Open("System/Config", MODE_OLDFILE);
    if (file)
    {
        do
        {
            line = FGets(file, buffer, 256);
            if (line)
            {
                if ((v = strstr(line, "LeftBorder ")) == line) {
                    data->leftborder = GetInt(v);
                } else  if ((v = strstr(line, "RightBorder ")) == line) {
                    data->rightborder = GetInt(v);
                } else  if ((v = strstr(line, "BottomBorder ")) == line) {
                    data->bottomborder = GetInt(v);
                } else  if ((v = strstr(line, "LogoOffset ")) == line) {
                    data->slogo_off = GetInt(v);
                } else  if ((v = strstr(line, "TitleOffset ")) == line) {
                    data->stitle_off = GetInt(v);
                } else  if ((v = strstr(line, "SBarHeight ")) == line) {
                    data->sbarheight = GetInt(v);
                } else  if ((v = strstr(line, "BarHeight ")) == line) {
                    data->winbarheight = GetInt(v); //screen, window
                } else  if ((v = strstr(line, "LUTBaseColors ")) == line) {
                    GetColors(v, &data->lut_col_a, &data->lut_col_d);
                } else  if ((v = strstr(line, "ScreenTitleColors ")) == line) {
                    GetColors(v, &data->text_col, &data->shadow_col);
                } else if ((v = strstr(line, "ScreenTitleMode ")) == line) {
                    data->outline = GetBool(v, "Outline");
                    data->shadow = GetBool(v, "Shadow");
                }
            }
        }
        while(line);
        Close(file);
    }

    data->di.img_sdepth = GetImageFromFile(path, "System/SDepth/", TRUE);
    data->di.img_stitlebar = GetImageFromFile(path, "System/STitlebar/", TRUE);
    data->di.img_sbarlogo = GetImageFromFile(path, "System/SBarLogo/Default", FALSE);

    data->di.img_size = GetImageFromFile(path, "System/Size/", TRUE);
    data->di.img_close = GetImageFromFile(path, "System/Close/", TRUE);
    data->di.img_depth = GetImageFromFile(path, "System/Depth/", TRUE);
    data->di.img_zoom = GetImageFromFile(path, "System/Zoom/", TRUE);
    data->di.img_mui = GetImageFromFile(path, "System/MUI/", TRUE);
    data->di.img_popup = GetImageFromFile(path, "System/PopUp/", TRUE);
    data->di.img_snapshot = GetImageFromFile(path, "System/Snapshot/", TRUE);
    data->di.img_iconify = GetImageFromFile(path, "System/Iconify/", TRUE);
    data->di.img_lock = GetImageFromFile(path, "System/Lock/", TRUE);
    data->di.img_up = GetImageFromFile(path, "System/ArrowUp/", TRUE);
    data->di.img_down = GetImageFromFile(path, "System/ArrowDown/", TRUE);
    data->di.img_left = GetImageFromFile(path, "System/ArrowLeft/", TRUE);
    data->di.img_right = GetImageFromFile(path, "System/ArrowRight/", TRUE);
    data->di.img_winbar_normal = GetImageFromFile(path, "System/Titlebar/", TRUE);
    data->di.img_border_normal = GetImageFromFile(path, "System/Borders/Default", FALSE);
    data->di.img_border_deactivated = GetImageFromFile(path, "System/Borders/Default_Deactivated", FALSE);
    data->di.img_verticalcontainer = GetImageFromFile(path, "System/Container/Vertical", FALSE);
    data->di.img_verticalknob = GetImageFromFile(path, "System/Knob/Vertical", FALSE);
    data->di.img_horizontalcontainer = GetImageFromFile(path, "System/Container/Horizontal", FALSE);
    data->di.img_horizontalknob = GetImageFromFile(path, "System/Knob/Horizontal", FALSE);

    data->di.img_menu = GetImageFromFile(path, "Menu/Background/Default", FALSE);
    data->di.img_amigakey = GetImageFromFile(path, "Menu/AmigaKey/", TRUE);
    data->di.img_menucheck = GetImageFromFile(path, "Menu/Checkmark/", TRUE);
    data->di.img_submenu = GetImageFromFile(path, "Menu/SubMenu/", TRUE);

    if (data->di.img_stitlebar)
    {
        data->di.img_stitlebar->tile_left = 8;
        data->di.img_stitlebar->tile_right = 8;
        data->di.img_stitlebar->tile_top = 9;
        data->di.img_stitlebar->tile_bottom = 8;
    }

    if (olddir) CurrentDir(olddir);
    UnLock(lock);

    if (data->di.img_sdepth) return TRUE;
    DisposeScreenSkinning(data);
    return FALSE;
}





static IPTR scrdecor_new(Class *cl, Object *obj, struct opSet *msg)
{
    struct scrdecor_data   *data;
    UWORD                   barh;

    obj = (Object *)DoSuperMethodA(cl, obj, (Msg)msg);

    if (obj)
    {
        data = INST_DATA(cl, obj);
        STRPTR path = (STRPTR) GetTagData(SDA_Configuration, (IPTR) "Theme:", msg->ops_AttrList);

        if (!InitScreenSkinning(path, data))
        {
            CoerceMethod(cl,obj,OM_DISPOSE);
            obj = NULL;
        }
        else
        {
            barh = data->sbarheight;

            if (data->di.img_sbarlogo) if (data->di.img_sbarlogo->h > barh) barh = data->di.img_sbarlogo->h;
            if (data->di.img_stitlebar) if (data->di.img_stitlebar->h > barh) barh = data->di.img_stitlebar->h;
        }
    }
    return (IPTR)obj;
}

static IPTR scrdecor_dispose(Class *cl, Object *obj, struct opSet *msg)
{
    struct scrdecor_data *data = INST_DATA(cl, obj);

    DisposeScreenSkinning(data);

    return 1;
}

/**************************************************************************************************/

static IPTR scrdecor_get(Class *cl, Object *obj, struct opGet *msg)
{
    struct scrdecor_data *data = INST_DATA(cl, obj);

    switch(msg->opg_AttrID)
    {
        case SDA_TrueColorOnly:
            *msg->opg_Storage = TRUE;
            break;

        case SDA_DecorImages:
            *msg->opg_Storage = (IPTR)&data->di;
            break;

        default:
            return DoSuperMethodA(cl, obj, (Msg)msg);
    }
    return 1;
}

/**************************************************************************************************/

static void scr_findtitlearea(struct Screen *scr, LONG *left, LONG *right)
{
    struct Gadget *g;

    *left = 0;
    *right = scr->Width - 1;

    for (g = scr->FirstGadget; g; g = g->NextGadget)
    {
        if (!(g->Flags & GFLG_RELRIGHT))
        {
            if (g->LeftEdge + g->Width > *left) *left = g->LeftEdge + g->Width;
        }
        else
        {
            if (g->LeftEdge + scr->Width - 1 - 1 < *right) *right = g->LeftEdge + scr->Width - 1 - 1;
        }
    }
}

static IPTR scrdecor_draw_screenbar(Class *cl, Object *obj, struct sdpDrawScreenBar *msg)
{
    struct scrdecor_data   *data = INST_DATA(cl, obj);
    struct ScreenData      *sd = (struct ScreenData *) msg->sdp_UserBuffer;
    struct TextExtent       te;
    struct RastPort        *rp = msg->sdp_RPort;
    struct Screen          *scr = msg->sdp_Screen;
    UWORD                  *pens = msg->sdp_Dri->dri_Pens;
    LONG                    left, right, titlelen = 0;
    BOOL                    hastitle = TRUE;
    BOOL		    beeping = scr->Flags & BEEPING;

    if (beeping) {
        SetAPen(rp, pens[BARDETAILPEN]);
        RectFill(rp, 0, 0, scr->Width, sd->img_stitlebar.h);
    } else {
        if (sd->img_stitlebar.ok)
	    WriteTiledImage(NULL, rp, &sd->img_stitlebar, 0, 0, sd->img_stitlebar.w, sd->img_stitlebar.h, 0, 0, scr->Width, sd->img_stitlebar.h);
    }
    if (sd->img_sbarlogo.ok) WriteTiledImage(NULL, rp, &sd->img_sbarlogo, 0, 0, sd->img_sbarlogo.w, sd->img_sbarlogo.h, data->slogo_off, (scr->BarHeight + 1 - sd->img_sbarlogo.h) / 2, sd->img_sbarlogo.w, sd->img_sbarlogo.h);
    if (scr->Title == NULL) hastitle = FALSE;

    if (hastitle)
    {
        scr_findtitlearea(scr, &left, &right);
        titlelen = strlen(scr->Title);
        titlelen = TextFit(rp, scr->Title, titlelen, &te, NULL, 1, right - data->stitle_off, data->sbarheight);
        if (titlelen == 0) hastitle = 0;
    }

    if (hastitle)
    {
        UWORD tx = data->stitle_off;
        UWORD ty = (scr->BarHeight + 1 - msg->sdp_Dri->dri_Font->tf_YSize) / 2 + rp->TxBaseline;

        SetFont(rp, msg->sdp_Dri->dri_Font);
        SetDrMd(rp, JAM1);
//         Move(rp, data->stitle_off, (scr->BarHeight + 1 - msg->sdp_Dri->dri_Font->tf_YSize) / 2 + rp->TxBaseline);
//         Text(rp, scr->Title, titlelen);

        if (!sd->truecolor || ((data->outline == FALSE) && (data->shadow == FALSE)))
        {
	    SetAPen(rp, pens[beeping ? BARBLOCKPEN : BARDETAILPEN]);
            Move(rp, tx, ty);
            Text(rp, scr->Title, titlelen);
        }
        else if (data->outline)
        {

                SetSoftStyle(rp, FSF_BOLD, AskSoftStyle(rp));
                SetRPAttrs(rp, RPTAG_PenMode, FALSE, RPTAG_FgColor, data->shadow_col, TAG_DONE);

                Move(rp, tx + 1, ty ); Text(rp, scr->Title, titlelen);
                Move(rp, tx + 2, ty ); Text(rp, scr->Title, titlelen);
                Move(rp, tx , ty ); Text(rp, scr->Title, titlelen);
                Move(rp, tx, ty + 1);  Text(rp, scr->Title, titlelen);
                Move(rp, tx, ty + 2);  Text(rp, scr->Title, titlelen);
                Move(rp, tx + 1, ty + 2);  Text(rp, scr->Title, titlelen);
                Move(rp, tx + 2, ty + 1);  Text(rp, scr->Title, titlelen);
                Move(rp, tx + 2, ty + 2);  Text(rp, scr->Title, titlelen);

                SetRPAttrs(rp, RPTAG_PenMode, FALSE, RPTAG_FgColor, data->text_col, TAG_DONE);
                Move(rp, tx + 1, ty + 1);
                Text(rp, scr->Title, titlelen);
                SetSoftStyle(rp, FS_NORMAL, AskSoftStyle(rp));
        }
        else
        {
                SetRPAttrs(rp, RPTAG_PenMode, FALSE, RPTAG_FgColor, data->shadow_col, TAG_DONE);
                Move(rp, tx + 1, ty + 1 );
                Text(rp, scr->Title, titlelen);

                SetRPAttrs(rp, RPTAG_PenMode, FALSE, RPTAG_FgColor, data->text_col, TAG_DONE);
                Move(rp, tx, ty);
                Text(rp, scr->Title, titlelen);

        }
    }
    return TRUE;
}

static IPTR scrdecor_getdefsize_sysimage(Class *cl, Object *obj, struct sdpGetDefSizeSysImage *msg)
{
    struct scrdecor_data *data = INST_DATA(cl, obj);

    if (msg->sdp_Which == SDEPTHIMAGE)
    {
        if (data->di.img_sdepth)
        {
            *msg->sdp_Height = data->di.img_sdepth->h;
            *msg->sdp_Width = data->di.img_sdepth->w >> 1;
        }
        else return DoSuperMethodA(cl, obj, (Msg) msg);
    }
    else return DoSuperMethodA(cl, obj, (Msg) msg);

    return TRUE;
}

static IPTR scrdecor_draw_sysimage(Class *cl, Object *obj, struct sdpDrawSysImage *msg)
{
    struct ScreenData      *sd = (struct ScreenData *) msg->sdp_UserBuffer;

    struct RastPort        *rp = msg->sdp_RPort;
    LONG                    left = msg->sdp_X;
    LONG                    top = msg->sdp_Y;
    LONG                    state = msg->sdp_State;

    if (msg->sdp_Which == SDEPTHIMAGE)
    {
        if (&sd->img_sdepth)
        {
            DrawAlphaStateImageToRP(2 /*NULL*/, rp, &sd->img_sdepth, state, left, top, TRUE);
        }
        else return DoSuperMethodA(cl, obj, (Msg) msg);
    }
    else return DoSuperMethodA(cl, obj, (Msg) msg);

    return TRUE;
}

static IPTR scrdecor_layoutscrgadgets(Class *cl, Object *obj, struct sdpLayoutScreenGadgets *msg)
{
    struct Gadget          *gadget = msg->sdp_Gadgets;
    struct ScreenData      *sd = (struct ScreenData *) msg->sdp_UserBuffer;

    struct scrdecor_data   *data = INST_DATA(cl, obj);

    while(gadget)
    {
        switch(gadget->GadgetType & GTYP_SYSTYPEMASK)
        {
            case GTYP_SDEPTH:
                gadget->LeftEdge = -gadget->Width;
                gadget->TopEdge = (data->sbarheight - sd->img_sdepth.h) >> 1;
                gadget->Flags &= ~GFLG_RELWIDTH;
                gadget->Flags |= GFLG_RELRIGHT;
                break;
        }

        if (msg->sdp_Flags & SDF_LSG_MULTIPLE)
        {
            gadget = gadget->NextGadget;
        }
        else
        {
            gadget = NULL;
        }
    }

    return TRUE;
}


static IPTR scrdecor_initscreen(Class *cl, Object *obj, struct sdpInitScreen *msg)
{
    struct scrdecor_data *data = INST_DATA(cl, obj);
    struct ScreenData *sd = (struct ScreenData *)msg->sdp_UserBuffer;
    struct Screen *screen = msg->sdp_Screen;

    sd->truecolor = msg->sdp_TrueColor;

    BOOL    truecolor = sd->truecolor;

    msg->sdp_WBorTop = data->winbarheight - 1 - msg->sdp_FontHeight;
    msg->sdp_BarHBorder = 1;
    msg->sdp_BarHeight = data->sbarheight - 1; //compatiblity issue
    msg->sdp_WBorLeft = data->leftborder;
    msg->sdp_WBorRight = data->rightborder;
    msg->sdp_WBorBottom = data->bottomborder;

    sd->ActivePen = -1;
    sd->DeactivePen = -1;
    if (!truecolor) {
        sd->ActivePen = ObtainPen(screen->ViewPort.ColorMap, -1, (data->lut_col_a << 8) & 0xff000000, (data->lut_col_a << 16) & 0xff000000, (data->lut_col_a << 24) & 0xff000000, PEN_EXCLUSIVE);
        sd->DeactivePen = ObtainPen(screen->ViewPort.ColorMap, -1, (data->lut_col_d << 8) & 0xff000000, (data->lut_col_d << 16) & 0xff000000, (data->lut_col_d << 24) & 0xff000000, PEN_EXCLUSIVE);
    }

    SETIMAGE_SCR(sdepth);
    SETIMAGE_SCR(sbarlogo);
    SETIMAGE_SCR(stitlebar);

    SETIMAGE_SCR(size);
    SETIMAGE_SCR(close);
    SETIMAGE_SCR(depth);
    SETIMAGE_SCR(zoom);
    SETIMAGE_SCR(up);
    SETIMAGE_SCR(down);
    SETIMAGE_SCR(left);
    SETIMAGE_SCR(right);
    SETIMAGE_SCR(mui);
    SETIMAGE_SCR(popup);
    SETIMAGE_SCR(snapshot);
    SETIMAGE_SCR(iconify);
    SETIMAGE_SCR(lock);
    SETIMAGE_SCR(winbar_normal);
    SETIMAGE_SCR(border_normal);
    SETIMAGE_SCR(border_deactivated);
    SETIMAGE_SCR(verticalcontainer);
    SETIMAGE_SCR(verticalknob);
    SETIMAGE_SCR(horizontalcontainer);
    SETIMAGE_SCR(horizontalknob);

    SETIMAGE_SCR(menu);
    SETIMAGE_SCR(amigakey);
    SETIMAGE_SCR(menucheck);
    SETIMAGE_SCR(submenu);

    return TRUE;
}

static IPTR scrdecor_exitscreen(Class *cl, Object *obj, struct sdpExitScreen *msg)
{
    struct ScreenData *sd = (struct ScreenData *)msg->sdp_UserBuffer;

    DELIMAGE_SCR(sdepth);
    DELIMAGE_SCR(sbarlogo);
    DELIMAGE_SCR(stitlebar);

    DELIMAGE_SCR(size);
    DELIMAGE_SCR(close);
    DELIMAGE_SCR(depth);
    DELIMAGE_SCR(zoom);
    DELIMAGE_SCR(up);
    DELIMAGE_SCR(down);
    DELIMAGE_SCR(left);
    DELIMAGE_SCR(right);
    DELIMAGE_SCR(mui);
    DELIMAGE_SCR(popup);
    DELIMAGE_SCR(snapshot);
    DELIMAGE_SCR(iconify);
    DELIMAGE_SCR(lock);
    DELIMAGE_SCR(winbar_normal);
    DELIMAGE_SCR(border_normal);
    DELIMAGE_SCR(border_deactivated);
    DELIMAGE_SCR(verticalcontainer);
    DELIMAGE_SCR(verticalknob);
    DELIMAGE_SCR(horizontalcontainer);
    DELIMAGE_SCR(horizontalknob);

    DELIMAGE_SCR(menu);
    DELIMAGE_SCR(amigakey);
    DELIMAGE_SCR(menucheck);
    DELIMAGE_SCR(submenu);

    return TRUE;
}

/**************************************************************************************************/

static IPTR scrdecor_dispatcher(struct IClass *cl, Object *obj, Msg msg)
{
    IPTR retval;
  
    switch(msg->MethodID)
    {
        case OM_NEW:
            retval = scrdecor_new(cl, obj, (struct opSet *)msg);
            break;

        case OM_DISPOSE:
            retval = scrdecor_dispose(cl, obj, (struct opSet *)msg);
            break;

        case OM_GET:
            retval = scrdecor_get(cl, obj, (struct opGet *)msg);
            break;

        case SDM_GETDEFSIZE_SYSIMAGE:
            retval = scrdecor_getdefsize_sysimage(cl, obj, (struct sdpGetDefSizeSysImage *)msg);
            break;

        case SDM_DRAW_SCREENBAR:
            retval = scrdecor_draw_screenbar(cl, obj, (struct sdpDrawScreenBar *)msg);
            break;

        case SDM_DRAW_SYSIMAGE:
            retval = scrdecor_draw_sysimage(cl, obj, (struct sdpDrawSysImage *)msg);
            break;

        case SDM_LAYOUT_SCREENGADGETS:
            retval = scrdecor_layoutscrgadgets(cl, obj, (struct sdpLayoutScreenGadgets *)msg);
            break;

        case SDM_INITSCREEN:
            retval = scrdecor_initscreen(cl, obj, (struct sdpInitScreen *)msg);
            break;

       case SDM_EXITSCREEN:
            retval = scrdecor_exitscreen(cl, obj, (struct sdpExitScreen *)msg);
            break;

        default:
            retval = DoSuperMethodA(cl, obj, msg);
            break;
    }

    return retval;
}

struct IClass * MakeScreenDecorClass()
{
    struct IClass * cl = MakeClass(NULL, SCRDECORCLASS, NULL, sizeof(struct scrdecor_data), 0);
    if (cl)
    {
        cl->cl_Dispatcher.h_Entry    = HookEntry;
        cl->cl_Dispatcher.h_SubEntry = (HOOKFUNC)scrdecor_dispatcher;
    }
    
    return cl;
}
