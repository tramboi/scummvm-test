/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

// Resource manager core part

// FIXME/TODO: The name "(Graphics) resource manager", and the associated
// filenames, are misleading. This should be renamed to "Graphics manager"
// or something like that.

#include "sci/gfx/gfx_resource.h"
#include "sci/gfx/gfx_tools.h"
#include "sci/gfx/gfx_driver.h"
#include "sci/gfx/gfx_resmgr.h"
#include "sci/gfx/gfx_state_internal.h"
#include "sci/gfx/font.h"

#include "common/system.h"

namespace Sci {

// Invalid hash mode: Used to invalidate modified pics
#define MODE_INVALID -1

struct param_struct {
	int args[4];
	gfx_driver_t *driver;
};

GfxResManager::GfxResManager(int version, gfx_options_t *options, gfx_driver_t *driver, ResourceManager *resManager) : 
				_version(version), _options(options), _driver(driver), _resManager(resManager), 
				_lockCounter(0), _tagLockCounter(0), _staticPalette(0) {
	gfxr_init_static_palette();

	if (_version < SCI_VERSION_01_VGA) {
		_staticPalette = gfx_sci0_pic_colors->getref();
	} else if (_version == SCI_VERSION_1_1) {
		GFXDEBUG("Palettes are not yet supported in this SCI version\n");
#ifdef ENABLE_SCI32
	} else if (_version == SCI_VERSION_32) {
		GFXDEBUG("Palettes are not yet supported in this SCI version\n");
#endif
	} else {
		Resource *res = resManager->findResource(kResourceTypePalette, 999, 0);
		if (res && res->data)
			_staticPalette = gfxr_read_pal1(res->id, res->data, res->size);
	}
}

GfxResManager::~GfxResManager() {
	_staticPalette->free();
	_staticPalette = 0;
}

#define DRAW_PIC01(pic, picStyle, isSci1) \
	gfxr_draw_pic01((pic), flags, default_palette, res->size, res->data, (picStyle), res->id, (isSci1), _staticPalette);

#define DRAW_PIC11(pic, picStyle) \
	gfxr_draw_pic11((pic), flags, default_palette, res->size, res->data, (picStyle), res->id, _staticPalette);

int GfxResManager::calculatePic(gfxr_pic_t *scaled_pic, gfxr_pic_t *unscaled_pic, int flags, int default_palette, int nr) {
	Resource *res = _resManager->findResource(kResourceTypePic, nr, 0);
	int need_unscaled = unscaled_pic != NULL;
	gfxr_pic0_params_t style, basic_style;

	basic_style.line_mode = GFX_LINE_MODE_CORRECT;
	basic_style.brush_mode = GFX_BRUSH_MODE_SCALED;

#ifdef CUSTOM_GRAPHICS_OPTIONS
	style.line_mode = _options->pic0_line_mode;
	style.brush_mode = _options->pic0_brush_mode;
#else
	style.line_mode = GFX_LINE_MODE_CORRECT;
	style.brush_mode = GFX_BRUSH_MODE_RANDOM_ELLIPSES;
#endif

	if (!res || !res->data)
		return GFX_ERROR;

	if (need_unscaled) {
		if (_version == SCI_VERSION_1_1)
			DRAW_PIC11(unscaled_pic, &basic_style)
		else
			DRAW_PIC01(unscaled_pic, &basic_style, _version >= SCI_VERSION_01_VGA)
	}

	if (scaled_pic && scaled_pic->undithered_buffer)
		memcpy(scaled_pic->visual_map->index_data, scaled_pic->undithered_buffer, scaled_pic->undithered_buffer_size);

	if (_version == SCI_VERSION_1_1)
		DRAW_PIC11(scaled_pic, &style)
	else
		DRAW_PIC01(scaled_pic, &style, _version >= SCI_VERSION_01_VGA)

	if (_version < SCI_VERSION_01_VGA) {
		if (need_unscaled)
			gfxr_remove_artifacts_pic0(scaled_pic, unscaled_pic);

		if (!scaled_pic->undithered_buffer)
			scaled_pic->undithered_buffer = sci_malloc(scaled_pic->undithered_buffer_size);

		memcpy(scaled_pic->undithered_buffer, scaled_pic->visual_map->index_data, scaled_pic->undithered_buffer_size);

#ifdef CUSTOM_GRAPHICS_OPTIONS
		gfxr_dither_pic0(scaled_pic, _options->pic0_dither_mode, _options->pic0_dither_pattern);
#else
		gfxr_dither_pic0(scaled_pic, GFXR_DITHER_MODE_D256, GFXR_DITHER_PATTERN_SCALED);
#endif
	}

	// Mark default palettes
	if (scaled_pic)
		scaled_pic->visual_map->loop = default_palette;

	if (unscaled_pic)
		unscaled_pic->visual_map->loop = default_palette;

	return GFX_OK;
}

int GfxResManager::getOptionsHash(gfx_resource_type_t type) {
	switch (type) {
	case GFX_RESOURCE_TYPE_VIEW:
		// This should never happen
		error("getOptionsHash called on a VIEW resource");

	case GFX_RESOURCE_TYPE_PIC:
#ifdef CUSTOM_GRAPHICS_OPTIONS
		if (_version >= SCI_VERSION_01_VGA)
			// NOTE: here, it is assumed that the upper port bound is always 10, but this doesn't seem to matter for the
			// generated options hash anyway
			return 10;
		else
			return (_options->pic0_unscaled) ? 0x10000 : (_options->pic0_dither_mode << 12)
			       | (_options->pic0_dither_pattern << 8) | (_options->pic0_brush_mode << 4) 
				   | (_options->pic0_line_mode);
#else
		if (_version >= SCI_VERSION_01_VGA)
			return 10;
		else
			return 0x10000 | (GFXR_DITHER_PATTERN_SCALED << 8) | (GFX_BRUSH_MODE_RANDOM_ELLIPSES << 4) | GFX_LINE_MODE_CORRECT;
#endif

	case GFX_RESOURCE_TYPE_FONT:
	case GFX_RESOURCE_TYPE_CURSOR:
		return 0;

	case GFX_RESOURCE_TYPES_NR:
	default:
		GFXERROR("Invalid resource type: %d\n", type);
		return -1;
	}
}

#define FREEALL(freecmd, type) \
	if (resource->scaled_data.type) \
		freecmd(resource->scaled_data.type); \
	resource->scaled_data.type = NULL; \
	if (resource->unscaled_data.type) \
		freecmd(resource->unscaled_data.type); \
	resource->unscaled_data.type = NULL;

void gfxr_free_resource(gfx_resource_t *resource, int type) {
	if (!resource)
		return;

	switch (type) {

	case GFX_RESOURCE_TYPE_VIEW:
		FREEALL(gfxr_free_view, view);
		break;

	case GFX_RESOURCE_TYPE_PIC:
		FREEALL(gfxr_free_pic, pic);
		break;

	case GFX_RESOURCE_TYPE_FONT:
		FREEALL(gfxr_free_font, font);
		break;

	case GFX_RESOURCE_TYPE_CURSOR:
		FREEALL(gfx_free_pixmap, pointer);
		break;

	default:
		GFXWARN("Attempt to free invalid resource type %d\n", type);
	}

	free(resource);
}

void GfxResManager::freeAllResources() {
	for (int type = 0; type < GFX_RESOURCE_TYPES_NR; ++type) {
		for (IntResMap::iterator iter = _resourceMaps[type].begin(); iter != _resourceMaps[type].end(); ++iter) {
			gfxr_free_resource(iter->_value, type);
			iter->_value = 0;
		}
	}
}

void GfxResManager::freeResManager() {
	freeAllResources();
	_lockCounter = _tagLockCounter = 0;
}

void GfxResManager::freeTaggedResources() {
	// Current heuristics: free tagged views and old pics

	IntResMap::iterator iter;
	int type;
	const int tmp = _tagLockCounter;

	type = GFX_RESOURCE_TYPE_VIEW;
	for (iter = _resourceMaps[type].begin(); iter != _resourceMaps[type].end(); ++iter) {
		gfx_resource_t *resource = iter->_value;

		if (resource) {
			if (resource->lock_sequence_nr < tmp) {
				gfxr_free_resource(resource, type);
				iter->_value = 0;
			} else {
				resource->lock_sequence_nr = 0;
			}
		}
	}

	type = GFX_RESOURCE_TYPE_PIC;
	for (iter = _resourceMaps[type].begin(); iter != _resourceMaps[type].end(); ++iter) {
		gfx_resource_t *resource = iter->_value;

		if (resource) {
			if (resource->lock_sequence_nr < 0) {
				gfxr_free_resource(resource, type);
				iter->_value = 0;
			} else {
				resource->lock_sequence_nr--;
			}
		}
	}

	_tagLockCounter = 0;
}


void GfxResManager::setStaticPalette(Palette *newPalette)
{
	if (_staticPalette)
		_staticPalette->free();

	_staticPalette = newPalette;
	_staticPalette->name = "static palette";

	_staticPalette->mergeInto(_driver->mode->palette);
}

#if 0
// FIXME: the options for GFX_MASK_VISUAL are actually unused
#define XLATE_AS_APPROPRIATE(key, entry) \
	if (maps & key) { \
		if (res->unscaled_data.pic&& (force || !res->unscaled_data.pic->entry->data)) { \
				if (key == GFX_MASK_VISUAL) \
					gfx_get_res_config(options, res->unscaled_data.pic->entry); \
			        gfx_xlate_pixmap(res->unscaled_data.pic->entry, mode, filter); \
		} if (scaled && res->scaled_data.pic && (force || !res->scaled_data.pic->entry->data)) { \
				if (key == GFX_MASK_VISUAL) \
					gfx_get_res_config(options, res->scaled_data.pic->entry); \
				gfx_xlate_pixmap(res->scaled_data.pic->entry, mode, filter); \
		} \
	}
#endif

#define XLATE_AS_APPROPRIATE(key, entry) \
	if (maps & key) { \
		if (res->unscaled_data.pic&& (force || !res->unscaled_data.pic->entry->data)) { \
			        gfx_xlate_pixmap(res->unscaled_data.pic->entry, mode, filter); \
		} if (scaled && res->scaled_data.pic && (force || !res->scaled_data.pic->entry->data)) { \
				gfx_xlate_pixmap(res->scaled_data.pic->entry, mode, filter); \
		} \
	}

static gfxr_pic_t *gfxr_pic_xlate_common(gfx_resource_t *res, int maps, int scaled, int force, gfx_mode_t *mode,
										 gfx_xlate_filter_t filter, gfx_options_t *options) {

	XLATE_AS_APPROPRIATE(GFX_MASK_VISUAL, visual_map);
	XLATE_AS_APPROPRIATE(GFX_MASK_PRIORITY, priority_map);
	XLATE_AS_APPROPRIATE(GFX_MASK_CONTROL, control_map);

	return scaled ? res->scaled_data.pic : res->unscaled_data.pic;
}
#undef XLATE_AS_APPROPRIATE


gfxr_pic_t *GfxResManager::getPic(int num, int maps, int flags, int default_palette, bool scaled) {
	gfxr_pic_t *npic = NULL;
	IntResMap &resMap = _resourceMaps[GFX_RESOURCE_TYPE_PIC];
	gfx_resource_t *res = NULL;
	int hash = getOptionsHash(GFX_RESOURCE_TYPE_PIC);
	int need_unscaled = (_driver->mode->xfact != 1 || _driver->mode->yfact != 1);

	hash |= (flags << 20) | ((default_palette & 0x7) << 28);

	res = resMap.contains(num) ? resMap[num] : NULL;

	if (!res || res->mode != hash) {
		gfxr_pic_t *pic;
		gfxr_pic_t *unscaled_pic = NULL;

#ifdef CUSTOM_GRAPHICS_OPTIONS
		if (_options->pic0_unscaled) {
			need_unscaled = 0;
			pic = gfxr_init_pic(&mode_1x1_color_index, GFXR_RES_ID(GFX_RESOURCE_TYPE_PIC, num), _version >= SCI_VERSION_01_VGA);
		} else
			pic = gfxr_init_pic(_driver->mode, GFXR_RES_ID(GFX_RESOURCE_TYPE_PIC, num), _version >= SCI_VERSION_01_VGA);
#else
		need_unscaled = 0;
		pic = gfxr_init_pic(&mode_1x1_color_index, GFXR_RES_ID(GFX_RESOURCE_TYPE_PIC, num), _version >= SCI_VERSION_01_VGA);
#endif

		if (!pic) {
			GFXERROR("Failed to allocate scaled pic!\n");
			return NULL;
		}

		gfxr_clear_pic0(pic, SCI_TITLEBAR_SIZE);

		if (need_unscaled) {
			unscaled_pic = gfxr_init_pic(&mode_1x1_color_index, GFXR_RES_ID(GFX_RESOURCE_TYPE_PIC, num), _version >= SCI_VERSION_01_VGA);
			if (!unscaled_pic) {
				GFXERROR("Failed to allocate unscaled pic!\n");
				return NULL;
			}
			gfxr_clear_pic0(pic, SCI_TITLEBAR_SIZE);
		}
		if (calculatePic(pic, unscaled_pic, flags, default_palette, num)) {
			gfxr_free_pic(pic);
			if (unscaled_pic)
				gfxr_free_pic(unscaled_pic);
			return NULL;
		}
		if (!res) {
			res = (gfx_resource_t *)sci_malloc(sizeof(gfx_resource_t));
			res->ID = GFXR_RES_ID(GFX_RESOURCE_TYPE_PIC, num);
#ifdef CUSTOM_GRAPHICS_OPTIONS
			res->lock_sequence_nr = _options->buffer_pics_nr;
#else
			res->lock_sequence_nr = 0;
#endif
			resMap[num] = res;
		} else {
			gfxr_free_pic(res->scaled_data.pic);
			if (res->unscaled_data.pic)
				gfxr_free_pic(res->unscaled_data.pic);
		}

		res->mode = hash;
		res->scaled_data.pic = pic;
		res->unscaled_data.pic = unscaled_pic;
	} else {
#ifdef CUSTOM_GRAPHICS_OPTIONS
		res->lock_sequence_nr = _options->buffer_pics_nr; // Update lock counter
#else
		res->lock_sequence_nr = 0;
#endif
	}

#ifdef CUSTOM_GRAPHICS_OPTIONS
	npic = gfxr_pic_xlate_common(res, maps, scaled || _options->pic0_unscaled, 0, _driver->mode,
	                             _options->pic_xlate_filter, _options);
#else
	npic = gfxr_pic_xlate_common(res, maps, 1, 0, _driver->mode,
	                             GFX_XLATE_FILTER_NONE, _options);
#endif


	return npic;
}

static void set_pic_id(gfx_resource_t *res, int id) {
	if (res->scaled_data.pic) {
		gfxr_pic_t *pic = res->scaled_data.pic;
		pic->control_map->ID = id;
		pic->priority_map->ID = id;
		pic->visual_map->ID = id;
	}

	if (res->unscaled_data.pic) {
		gfxr_pic_t *pic = res->unscaled_data.pic;
		pic->control_map->ID = id;
		pic->priority_map->ID = id;
		pic->visual_map->ID = id;
	}
}

static int get_pic_id(gfx_resource_t *res) {
	if (res->scaled_data.pic)
		return res->scaled_data.pic->visual_map->ID;
	else
		return res->unscaled_data.pic->visual_map->ID;
}

static void _gfxr_unscale_pixmap_index_data(gfx_pixmap_t *pxm, gfx_mode_t *mode) {
	int xmod = mode->xfact; // Step size horizontally
	int ymod = pxm->index_width * mode->yfact; // Vertical step size
	int maxpos = pxm->index_width * pxm->index_height;
	int pos;
	byte *dest = pxm->index_data;

	if (!(pxm->flags & GFX_PIXMAP_FLAG_SCALED_INDEX))
		return; // It's not scaled!

	for (pos = 0; pos < maxpos; pos += ymod) {
		int c;

		for (c = 0; c < pxm->index_width; c += xmod)
			*dest++ = pxm->index_data[pos + c];
			// No overwrite since line and offset readers move much faster (proof by in-duction, trivial
			// and left to the reader)
	}

	pxm->index_width /= mode->xfact;
	pxm->index_height /= mode->yfact;
	pxm->flags &= ~GFX_PIXMAP_FLAG_SCALED_INDEX;
}

gfxr_pic_t *GfxResManager::addToPic(int old_nr, int new_nr, int flags, int old_default_palette, int default_palette) {
	IntResMap &resMap = _resourceMaps[GFX_RESOURCE_TYPE_PIC];
	gfxr_pic_t *pic = NULL;
	gfx_resource_t *res = NULL;
	int hash = getOptionsHash(GFX_RESOURCE_TYPE_PIC);
#ifdef CUSTOM_GRAPHICS_OPTIONS
	int need_unscaled = !(_options->pic0_unscaled) && (_driver->mode->xfact != 1 || _driver->mode->yfact != 1);
#else
	int need_unscaled = 1;
#endif

	res = resMap.contains(old_nr) ? resMap[old_nr] : NULL;

	if (!res || (res->mode != MODE_INVALID && res->mode != hash)) {
		getPic(old_nr, 0, flags, old_default_palette, 1);

		res = resMap.contains(old_nr) ? resMap[old_nr] : NULL;

		if (!res) {
			GFXWARN("Attempt to add pic %d to non-existing pic %d\n", new_nr, old_nr);
			return NULL;
		}
	}

#ifdef CUSTOM_GRAPHICS_OPTIONS
	if (_options->pic0_unscaled) // Unscale priority map, if we scaled it earlier
#endif
		_gfxr_unscale_pixmap_index_data(res->scaled_data.pic->priority_map, _driver->mode);

	// The following two operations are needed when returning scaled maps (which is always the case here)
#ifdef CUSTOM_GRAPHICS_OPTIONS
	res->lock_sequence_nr = _options->buffer_pics_nr;
#else
	res->lock_sequence_nr = 0;
#endif
	calculatePic(res->scaled_data.pic, need_unscaled ? res->unscaled_data.pic : NULL,
		                               flags | DRAWPIC01_FLAG_OVERLAID_PIC, default_palette, new_nr);

	res->mode = MODE_INVALID; // Invalidate

#ifdef CUSTOM_GRAPHICS_OPTIONS
	if (_options->pic0_unscaled) // Scale priority map again, if needed
#endif
		res->scaled_data.pic->priority_map = gfx_pixmap_scale_index_data(res->scaled_data.pic->priority_map, _driver->mode);

	{
		int old_ID = get_pic_id(res);
		set_pic_id(res, GFXR_RES_ID(GFX_RESOURCE_TYPE_PIC, new_nr)); // To ensure that our graphical translation options work properly
#ifdef CUSTOM_GRAPHICS_OPTIONS
		pic = gfxr_pic_xlate_common(res, GFX_MASK_VISUAL, 1, 1, _driver->mode, _options->pic_xlate_filter, _options);
#else
		pic = gfxr_pic_xlate_common(res, GFX_MASK_VISUAL, 1, 1, _driver->mode, GFX_XLATE_FILTER_NONE, _options);
#endif
		set_pic_id(res, old_ID);
	}

	return pic;
}

gfxr_view_t *gfxr_draw_view11(int id, byte *resource, int size);

gfxr_view_t *GfxResManager::getView(int nr, int *loop, int *cel, int palette) {
	IntResMap &resMap = _resourceMaps[GFX_RESOURCE_TYPE_VIEW];
	gfx_resource_t *res = NULL;
	int hash = palette;
	gfxr_view_t *view = NULL;
	gfxr_loop_t *loop_data = NULL;
	gfx_pixmap_t *cel_data = NULL;

	res = resMap.contains(nr) ? resMap[nr] : NULL;

	if (!res || res->mode != hash) {
		Resource *viewRes = _resManager->findResource(kResourceTypeView, nr, 0);
		if (!viewRes || !viewRes->data)
			return NULL;

		int resid = GFXR_RES_ID(GFX_RESOURCE_TYPE_VIEW, nr);

		if (_version < SCI_VERSION_01)
			view = gfxr_draw_view0(resid, viewRes->data, viewRes->size, -1);
		else if (_version == SCI_VERSION_01)
			view = gfxr_draw_view0(resid, viewRes->data, viewRes->size, palette);
		else if (_version >= SCI_VERSION_01_VGA && _version <= SCI_VERSION_1_LATE)
			view = gfxr_draw_view1(resid, viewRes->data, viewRes->size, _staticPalette);
		else if (_version >= SCI_VERSION_1_1)
			view = gfxr_draw_view11(resid, viewRes->data, viewRes->size);

		if (_version >= SCI_VERSION_01_VGA) {
			if (!view->palette) {
				view->palette = new Palette(_staticPalette->size());
				view->palette->name = "interpreter_get_view";
			}
			
			// Palettize view
			for (unsigned i = 0; i < MIN(view->palette->size(), _staticPalette->size()); i++) {
				const PaletteEntry& vc = view->palette->getColor(i);
				if (vc.r == 0 && vc.g == 0 && vc.b == 0) {
					const PaletteEntry& sc = _staticPalette->getColor(i);
					view->palette->setColor(i, sc.r, sc.g, sc.b);
				}
			}

		}

		if (!res) {
			res = (gfx_resource_t *)sci_malloc(sizeof(gfx_resource_t));
			res->scaled_data.view = NULL;
			res->ID = GFXR_RES_ID(GFX_RESOURCE_TYPE_VIEW, nr);
			res->lock_sequence_nr = _tagLockCounter;
			res->mode = hash;
			resMap[nr] = res;
		} else {
			gfxr_free_view(res->unscaled_data.view);
		}

		res->mode = hash;
		res->unscaled_data.view = view;

	} else {
		res->lock_sequence_nr = _tagLockCounter; // Update lock counter
		view = res->unscaled_data.view;
	}

	*loop = CLIP<int>(*loop, 0, view->loops_nr - 1);

	if (*loop < 0) {
		GFXWARN("View %d has no loops\n", nr);
		return NULL;
	}

	loop_data = view->loops + (*loop);
	if (loop_data == NULL) {
		GFXWARN("Trying to load invalid loop %d of view %d\n", *loop, nr);
		return NULL;
	}

	*cel = CLIP<int>(*cel, 0, loop_data->cels_nr - 1);

	if (*cel < 0) {
		GFXWARN("View %d loop %d has no cels\n", nr, *loop);
		return NULL;
	}

	cel_data = loop_data->cels[*cel];
	if (loop_data == NULL) {
		GFXWARN("Trying to load invalid view/loop/cel %d/%d/%d\n", nr, *loop, *cel);
		return NULL;
	}

	if (!cel_data->data) {
#ifdef CUSTOM_GRAPHICS_OPTIONS
		gfx_get_res_config(_options, cel_data);
		gfx_xlate_pixmap(cel_data, _driver->mode, _options->view_xlate_filter);
#else
		gfx_xlate_pixmap(cel_data, _driver->mode, GFX_XLATE_FILTER_NONE);
#endif
	}

	return view;
}

gfx_bitmap_font_t *GfxResManager::getFont(int num, bool scaled) {
	IntResMap &resMap = _resourceMaps[GFX_RESOURCE_TYPE_FONT];
	gfx_resource_t *res = NULL;
	int hash = getOptionsHash(GFX_RESOURCE_TYPE_FONT);

	res = resMap.contains(num) ? resMap[num] : NULL;

	if (!res || res->mode != hash) {
		Resource *fontRes = _resManager->findResource(kResourceTypeFont, num, 0);
		if (!fontRes || !fontRes->data)
			return NULL;

		gfx_bitmap_font_t *font = gfxr_read_font(fontRes->id, fontRes->data, fontRes->size);

		if (!res) {
			res = (gfx_resource_t *)sci_malloc(sizeof(gfx_resource_t));
			res->scaled_data.font = NULL;
			res->ID = GFXR_RES_ID(GFX_RESOURCE_TYPE_FONT, num);
			res->lock_sequence_nr = _tagLockCounter;
			res->mode = hash;
			resMap[num] = res;
		} else {
			gfxr_free_font(res->unscaled_data.font);
		}

		res->unscaled_data.font = font;

		return font;
	} else {
		res->lock_sequence_nr = _tagLockCounter; // Update lock counter
		if (res->unscaled_data.pointer)
			return res->unscaled_data.font;
		else
			return res->scaled_data.font;
	}
}

gfx_pixmap_t *GfxResManager::getCursor(int num) {
	IntResMap &resMap = _resourceMaps[GFX_RESOURCE_TYPE_CURSOR];
	gfx_resource_t *res = NULL;
	int hash = getOptionsHash(GFX_RESOURCE_TYPE_CURSOR);

	res = resMap.contains(num) ? resMap[num] : NULL;

	if (!res || res->mode != hash) {
		Resource *cursorRes = _resManager->findResource(kResourceTypeCursor, num, 0);
		if (!cursorRes || !cursorRes->data)
			return NULL;

		if (_version >= SCI_VERSION_1_1) {
			GFXWARN("Attempt to retrieve cursor in SCI1.1 or later\n");
			return NULL;
		}

		gfx_pixmap_t *cursor = gfxr_draw_cursor(GFXR_RES_ID(GFX_RESOURCE_TYPE_CURSOR, num), 
										cursorRes->data, cursorRes->size, _version != SCI_VERSION_0);

		if (!cursor)
			return NULL;

		if (!res) {
			res = (gfx_resource_t *)sci_malloc(sizeof(gfx_resource_t));
			res->scaled_data.pointer = NULL;
			res->ID = GFXR_RES_ID(GFX_RESOURCE_TYPE_CURSOR, num);
			res->lock_sequence_nr = _tagLockCounter;
			res->mode = hash;
			resMap[num] = res;
		} else {
			gfx_free_pixmap(res->unscaled_data.pointer);
		}
#ifdef CUSTOM_GRAPHICS_OPTIONS
		gfx_get_res_config(_options, cursor);
		gfx_xlate_pixmap(cursor, _driver->mode, _options->cursor_xlate_filter);
#else
		gfx_xlate_pixmap(cursor, _driver->mode, GFX_XLATE_FILTER_NONE);
#endif

		res->unscaled_data.pointer = cursor;

		return cursor;
	} else {
		res->lock_sequence_nr = _tagLockCounter; // Update lock counter
		return res->unscaled_data.pointer;
	}
}

} // End of namespace Sci
