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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.	
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "common/system.h"
#include "gui/ThemeRenderer.h"

namespace GUI {

bool ThemeRenderer::loadDefaultXML() {
	const char *defaultXML =
/**
 * Default theme description file. Work in progress.
 * Newlines are not necessary, parser ignores them.
 * You may use single quotes (') instead of scaped double quotes.
 * Remember to indent properly the XML so it's easier to read and
 * to maintain!
 * Also remember to scape the end of each line. :p
 */
"<render_info>																 \
	<palette>																 \
		<color name = 'darkred'												 \
				rgb = '168, 42, 12'											 \
		/>																	 \
		<color name = 'brightred'											 \
				rgb = '200, 124, 104'										 \
		/>																	 \
		<color name = 'xtrabrightred'										 \
				rgb = '251, 241, 206'										 \
		/>																	 \
		<color name = 'blandyellow'											 \
				rgb = '247, 228, 166'										 \
		/>																	 \
		<color name = 'bgreen'												 \
				rgb = '96, 160, 8'											 \
		/>																	 \
		<color name = 'blue'												 \
				rgb = '0, 255, 255'											 \
		/>																	 \
		<color name = 'black'												 \
				rgb = '0, 0, 0'												 \
		/>																	 \
		<color name = 'white'												 \
				rgb = '255, 255, 255'										 \
		/>																	 \
	</palette>																 \
																			 \
	<fonts>																	 \
		<font	id = 'text_default'											 \
				type = 'default'											 \
				color = 'black'												 \
		/>																	 \
		<font	id = 'text_hover'											 \
				type = 'default'											 \
				color = 'bgreen'											 \
		/>																	 \
		<font	id = 'text_disabled'										 \
				type = 'default'											 \
				color = '128, 128, 128'										 \
		/>																	 \
		<font	id = 'text_inverted'										 \
				type = 'default'											 \
				color = '0, 0, 0'											 \
		/>																	 \
		<font	id = 'text_button'											 \
				type = 'default'											 \
				color = 'white'												 \
		/>																	 \
		<font	id = 'text_button_hover'									 \
				type = 'default'											 \
				color = 'blandyellow'										 \
		/>																	 \
	</fonts>																 \
																			 \
	<defaults fill = 'gradient' fg_color = 'white' />						 \
																			 \
	<drawdata id = 'text_selection' cache = false>							 \
		<drawstep	func = 'square'											 \
					fill = 'foreground'										 \
					fg_color = 'bgreen'										 \
		/>																	 \
	</drawdata>																 \
																			 \
	<drawdata id = 'mainmenu_bg' cache = false>								 \
		<drawstep	func = 'fill'											 \
					fill = 'gradient'										 \
					gradient_start = '208, 112, 8'							 \
					gradient_end = '232, 192, 16'							 \
		/>																	 \
	</drawdata>																 \
																			 \
	<drawdata id = 'separator' cache = false>								 \
		<drawstep	func = 'square'											 \
					fill = 'foreground'										 \
					height = '1'											 \
					ypos = 'center'											 \
					fg_color = 'black'										 \
		/>																	 \
	</drawdata>																 \
																			 \
	<drawdata id = 'scrollbar_base' cache = false>							 \
		<drawstep	func = 'roundedsq'										 \
					stroke = 1												 \
					radius = 6												 \
					fill = 'background'										 \
					fg_color = '176, 164, 160'								 \
					bg_color = '240, 228, 160'								 \
		/>																	 \
	</drawdata>																 \
																			 \
	<drawdata id = 'scrollbar_handle_hover' cache = false>					 \
		<drawstep	func = 'roundedsq'										 \
					stroke = 1												 \
					radius = 6												 \
					fill = 'gradient'										 \
					fg_color = 'blandyellow'								 \
					gradient_start = 'xtrabrightred'						 \
					gradient_end = 'darkred'								 \
		/>																	 \
	</drawdata>																 \
																			 \
	<drawdata id = 'scrollbar_handle_idle' cache = false>					 \
		<drawstep	func = 'roundedsq'										 \
					stroke = 1												 \
					radius = 6												 \
					fill = 'gradient'										 \
					fg_color = 'blandyellow'								 \
					gradient_start = 'brightred'							 \
					gradient_end = 'darkred'								 \
		/>																	 \
	</drawdata>																 \
																			 \
	<drawdata id = 'scrollbar_button_idle' cache = false>					 \
		<drawstep	func = 'roundedsq'										 \
					radius = '4'											 \
					fill = 'none'											 \
					fg_color = '176, 164, 160'								 \
					stroke = 1												 \
		/>																	 \
		<drawstep	func = 'triangle'										 \
					fg_color = '0, 0, 0'									 \
					fill = 'foreground'										 \
					width = 'auto'											 \
					height = 'auto'											 \
					xpos = 'center'											 \
					ypos = 'center'											 \
					orientation = 'top'										 \
		/>																	 \
	</drawdata>																 \
																			 \
	<drawdata id = 'scrollbar_button_hover' cache = false>					 \
		<drawstep	func = 'roundedsq'										 \
					radius = '4'											 \
					fill = 'background'										 \
					fg_color = '120, 120, 120'								 \
					bg_color = '206, 121, 99'								 \
					stroke = 1												 \
		/>																	 \
		<drawstep	func = 'triangle'										 \
					fg_color = '0, 0, 0'									 \
					fill = 'foreground'										 \
					width = 'auto'											 \
					height = 'auto'											 \
					xpos = 'center'											 \
					ypos = 'center'											 \
					orientation = 'top'										 \
		/>																	 \
	</drawdata>																 \
																			 \
	<drawdata id = 'tab_active' cache = false>								 \
		<text	font = 'text_default'										 \
				vertical_align = 'center'									 \
				horizontal_align = 'center'									 \
		/>																	 \
		<drawstep	func = 'tab'											 \
					radius = '4'											 \
					stroke = '0'											 \
					fill = 'gradient'										 \
					gradient_end = 'xtrabrightred'							 \
					gradient_start = 'blandyellow'						 	 \
					shadow = 3												 \
		/>																	 \
	</drawdata>																 \
																			 \
	<drawdata id = 'tab_inactive' cache = false>							 \
		<text	font = 'text_default'										 \
				vertical_align = 'center'									 \
				horizontal_align = 'center'									 \
		/>																	 \
		<drawstep	func = 'tab'											 \
					radius = '4'											 \
					stroke = '0'											 \
					fill = 'foreground'										 \
					fg_color = '240, 205, 118'								 \
					shadow = 3												 \
		/>																	 \
	</drawdata>																 \
																			 \
	<drawdata id = 'tab_background' cache = false>							 \
		<drawstep	func = 'tab'											 \
					radius = '12'											 \
					stroke = '0'											 \
					fill = 'foreground'										 \
					fg_color = '232, 180, 81'								 \
					shadow = 3												 \
		/>																	 \
	</drawdata>																 \
																			 \
	<drawdata id = 'widget_slider' cache = false>							 \
		<drawstep	func = 'roundedsq'										 \
					stroke = 1												 \
					radius = 8												 \
					fill = 'none'											 \
					fg_color = '0, 0, 0'									 \
		/>																	 \
	</drawdata>																 \
																			 \
	<drawdata id = 'slider_full' cache = false>								 \
		<drawstep	func = 'roundedsq'										 \
					stroke = 1												 \
					radius = 8												 \
					fill = 'gradient'										 \
					fg_color = '0, 0, 0'									 \
					gradient_start = '214, 113, 8'							 \
					gradient_end = '240, 200, 25'							 \
		/>																	 \
	</drawdata>																 \
																			 \
	<drawdata id = 'popup_idle' cache = false>								 \
		<drawstep	func = 'square'											 \
					stroke = 0												 \
					fg_color = '0, 0, 0'									 \
					fill = 'gradient'										 \
					gradient_start = '214, 113, 8'							 \
					gradient_end = '240, 200, 25'							 \
					shadow = 3												 \
		/>																	 \
		<drawstep	func = 'triangle'										 \
					fg_color = '0, 0, 0'									 \
					fill = 'foreground'										 \
					width = 'height'										 \
					height = 'auto'											 \
					xpos = 'right'											 \
					ypos = 'center'											 \
					orientation = 'bottom'									 \
		/>																	 \
		<text	font = 'text_default'										 \
				vertical_align = 'center'									 \
				horizontal_align = 'right'									 \
		/>																	 \
	</drawdata>																 \
																			 \
																			 \
	<drawdata id = 'popup_hover' cache = false>								 \
		<drawstep	func = 'square'											 \
					stroke = 0												 \
					fg_color = 'black'										 \
					fill = 'gradient'										 \
					gradient_start = '214, 113, 8'							 \
					gradient_end = '240, 200, 25'							 \
					shadow = 0												 \
		/>																	 \
		<drawstep	func = 'triangle'										 \
					fg_color = '0, 0, 0'									 \
					fill = 'foreground'										 \
					width = 'height'										 \
					height = 'auto'											 \
					xpos = 'right'											 \
					ypos = 'center'											 \
					orientation = 'bottom'									 \
		/>																	 \
		<text	font = 'text_hover'											 \
				vertical_align = 'center'									 \
				horizontal_align = 'right'									 \
		/>																	 \
	</drawdata>																 \
																			 \
	<drawdata id = 'default_bg' cache = false>								 \
		<drawstep	func = 'roundedsq'										 \
					radius = 12												 \
					stroke = 0												 \
					fg_color = 'xtrabrightred'								 \
					fill = 'foreground'										 \
					shadow = 3												 \
		/>																	 \
	</drawdata>																 \
																			 \
	<drawdata id = 'button_idle' cache = false>								 \
		<text	font = 'text_button'										 \
				vertical_align = 'center'									 \
				horizontal_align = 'center'									 \
		/>																	 \
		<drawstep	func = 'roundedsq'										 \
					radius = '6'											 \
					stroke = 1												 \
					fill = 'gradient'										 \
					shadow = 2												 \
					fg_color = 'blandyellow'								 \
					gradient_start = 'brightred'							 \
					gradient_end = 'darkred'								 \
		/>																	 \
	</drawdata>																 \
																			 \
	<drawdata id = 'button_hover' cache = false>							 \
		<text	font = 'text_button_hover'									 \
				vertical_align = 'center'									 \
				horizontal_align = 'center'									 \
		/>																	 \
		<drawstep	func = 'roundedsq'										 \
					radius = '6'											 \
					gradient_factor = 1										 \
					stroke = 1 fill = 'gradient'							 \
					shadow = 0												 \
					fg_color = 'blandyellow'								 \
					gradient_start = 'xtrabrightred'						 \
					gradient_end = 'darkred'								 \
		/>																	 \
	</drawdata>																 \
																			 \
	<drawdata id = 'button_disabled' cache = false>							 \
		<text	font = 'text_disabled'										 \
				vertical_align = 'center'									 \
				horizontal_align = 'center'									 \
		/>																	 \
		<drawstep	func = 'roundedsq'										 \
					radius = '8'											 \
					stroke = 0												 \
					fill = 'foreground'										 \
					fg_color = '200, 200, 200'								 \
					shadow = 3												 \
		/>																	 \
	</drawdata>																 \
																			 \
	<drawdata id = 'checkbox_disabled' cache = false>						 \
		<text	font = 'text_disabled'										 \
				vertical_align = 'top'										 \
				horizontal_align = 'left'									 \
		/>																	 \
		<drawstep	func = 'roundedsq'										 \
					fill = 'none'											 \
					radius = 8										 	 	 \
					fg_color = 'black'						 				 \
					shadow = 0												 \
		/>																	 \
	</drawdata>																 \
																			 \
	<drawdata id = 'checkbox_selected' cache = false>						 \
		<text	font = 'text_default'										 \
				vertical_align = 'top'										 \
				horizontal_align = 'left'									 \
		/>																	 \
		<drawstep	func = 'square'											 \
					fill = 'gradient'										 \
					gradient_start = '206, 121, 99'							 \
					gradient_end = '173, 40, 8'								 \
					shadow = 0												 \
		/>																	 \
		<drawstep	func = 'circle'											 \
					radius = '4'											 \
					fill = 'foreground'										 \
		/>																	 \
	</drawdata>																 \
																			 \
	<drawdata id = 'checkbox_default' cache = false>						 \
		<text	font = 'text_default'										 \
				vertical_align = 'top'										 \
				horizontal_align = 'left'									 \
		/>																	 \
		<drawstep	func = 'square'											 \
					fill = 'gradient'										 \
					gradient_start = '206, 121, 99'							 \
					gradient_end = '173, 40, 8'								 \
					shadow = 0												 \
		/>																	 \
	</drawdata>																 \
																			 \
	<drawdata id = 'widget_default' cache = false>							 \
		<drawstep	func = 'roundedsq'										 \
					gradient_factor = 6										 \
					radius = '8'											 \
					fill = 'gradient'										 \
					gradient_start = '240, 224, 136'						 \
					gradient_end = 'xtrabrightred'							 \
					shadow = 3												 \
		/>																	 \
	</drawdata>																 \
</render_info>																 \
																			 \
<layout_info>																 \
	<globals>																 \
		<def var = 'Widget.Size' value = '30' />							 \
		<def var = 'Line.Height' value = '16' />							 \
		<def var = 'Font.Height' value = '16' />							 \
																			 \
		<widget name = 'Inset'												 \
				pos = '23, 94'												 \
				size = '666, 666'											 \
		/>																	 \
		<widget name = 'Button'												 \
				size = '120, 25'											 \
		/>																	 \
		<widget name = 'Slider'												 \
				size = '666, 666'											 \
		/>																	 \
		<widget name = 'ListWidget'											 \
				padding = '7, 5, 5, 5'										 \
		/>																	 \
		<widget name = 'PopUpWidget'										 \
				padding = '7, 5, 0, 0'										 \
		/>																	 \
		<widget name = 'EditTextWidget'										 \
				padding = '7, 5, 0, 0'										 \
		/>																	 \
		<widget name = 'Console'											 \
				padding = '7, 5, 5, 5'										 \
		/>																	 \
																			 \
		<widget name = 'TabWidget'>											 \
			<child	name = 'Tab'											 \
					size = '75, 27'											 \
					padding = '0, 0, 8, 0'									 \
			/>																 \
			<child name = 'NavButton'										 \
					size = '15, 18'											 \
					padding = '0, 3, 4, 0'									 \
			/>																 \
		</widget>															 \
	</globals>																 \
																			 \
	<dialog name = 'Launcher'>												 \
		<widget name = 'Version'											 \
				pos = 'center, 21'											 \
				size = '247, Globals.Line.Height'							 \
		/>																	 \
		<widget name = 'Logo'												 \
				pos = 'center, 5'											 \
				size = '283, 80'											 \
		/>																	 \
		<widget name = 'GameList'											 \
				pos = 'Globals.Inset.X, Globals.Inset.Y'					 \
				size = 'Globals.Inset.Width, Globals.Inset.Height'			 \
		/>																	 \
																			 \
		<widget name = 'StartButton'										 \
				size = 'Globals.Button.Width, Globals.Button.Height'		 \
		/>																	 \
		<widget name = 'AddGameButton'										 \
				size = 'Globals.Button.Width, Globals.Button.Height'		 \
		/>																	 \
		<widget name = 'EditGameButton'										 \
				size = 'Globals.Button.Width, Globals.Button.Height'		 \
		/>																	 \
		<widget name = 'RemoveGameButton'									 \
				size = 'Globals.Button.Width, Globals.Button.Height'		 \
		/>																	 \
		<widget name = 'OptionsButton'										 \
				size = 'Globals.Button.Width, Globals.Button.Height'		 \
		/>																	 \
		<widget name = 'AboutButton'										 \
				size = 'Globals.Button.Width, Globals.Button.Height'		 \
		/>																	 \
		<widget name = 'QuittButton'										 \
				size = 'Globals.Button.Width, Globals.Button.Height'		 \
		/>																	 \
	</dialog>																 \
</layout_info>";

	if (!parser()->loadBuffer((const byte*)defaultXML, strlen(defaultXML), false))
		return false;

	return parser()->parse();
}

}
