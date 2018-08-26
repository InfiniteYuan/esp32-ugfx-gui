/*
 * Copyright (c) 2012, 2013, Joel Bodenmann aka Tectu <joel@unormal.org>
 * Copyright (c) 2012, 2013, Andrew Hannam aka inmarket
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *    * Neither the name of the <organization> nor the
 *      names of its contributors may be used to endorse or promote products
 *      derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _GFXCONF_H
#define _GFXCONF_H

/* GFX sub-systems to turn on */
#define GFX_USE_GDISP			                TRUE
#define GFX_USE_GEVENT			                TRUE
#define GFX_USE_GTIMER			                TRUE
#define GFX_USE_GINPUT			                TRUE

#define GFX_USE_GFILE                           TRUE
#define GFILE_NEED_ROMFS                        TRUE
#define GFILE_MAX_GFILES                         24

/* Features for the GDISP sub-system. */
#define GDISP_NEED_CONTROL		                TRUE
#define GDISP_NEED_VALIDATION		            TRUE
#define GDISP_NEED_CLIP			                TRUE
#define GDISP_NEED_TEXT			                TRUE
#define GDISP_NEED_MULTITHREAD		            TRUE
#define GDISP_NEED_IMAGE		                TRUE
#define GDISP_NEED_IMAGE_BMP		            TRUE
#define GDISP_NEED_CIRCLE		                TRUE
#define GDISP_NEED_ELLIPSE		                TRUE
#define GDISP_NEED_ARC			                TRUE
#define GDISP_NEED_STARTUP_LOGO                 FALSE


//////////////////
#define GFX_USE_GWIN                            TRUE

#define GWIN_NEED_WINDOWMANAGER                 TRUE
#define GQUEUE_NEED_ASYNC                       TRUE
#define GFX_USE_GQUEUE                          TRUE
#define GWIN_REDRAW_IMMEDIATE                   TRUE
#define GWIN_REDRAW_SINGLEOP                    TRUE

#define GWIN_NEED_WIDGET                        TRUE
#define GWIN_NEED_BUTTON                        TRUE
#define GWIN_BUTTON_LAZY_RELEASE                FALSE
#define GWIN_NEED_SLIDER                        TRUE
#define GWIN_FLAT_STYLING                       FALSE

#define GWIN_NEED_CONTAINERS                    TRUE
#define GWIN_NEED_CONTAINER                     TRUE
#define GWIN_NEED_FRAME                         FALSE
#define GWIN_NEED_TABSET                        FALSE


#define GWIN_NEED_CONSOLE                            TRUE
    #define GWIN_CONSOLE_USE_HISTORY                 TRUE
        #define GWIN_CONSOLE_HISTORY_AVERAGING       TRUE
        #define GWIN_CONSOLE_HISTORY_ATCREATE        TRUE
    #define GWIN_CONSOLE_ESCSEQ                      TRUE
    #define GWIN_CONSOLE_USE_BASESTREAM              TRUE
#define GWIN_NEED_GRAPH                              TRUE



/* Builtin Fonts */
#define GDISP_INCLUDE_FONT_DEJAVUSANS16      TRUE


/* Features for the GINPUT sub-system. */
#define GINPUT_NEED_MOUSE                       TRUE

#endif /* _GFXCONF_H */
