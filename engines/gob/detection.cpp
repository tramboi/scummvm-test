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

#include "base/plugins.h"
#include "engines/advancedDetector.h"

#include "gob/gob.h"

namespace Gob {

struct GOBGameDescription {
	ADGameDescription desc;

	GameType gameType;
	int32 features;
	const char *startStkBase;
	const char *startTotBase;
};

}

using namespace Common;

static const PlainGameDescriptor gobGames[] = {
	{"gob", "Gob engine game"},
	{"gob1", "Gobliiins"},
	{"gob1cd", "Gobliiins CD"},
	{"gob2", "Gobliins 2"},
	{"gob2cd", "Gobliins 2 CD"},
	{"ween", "Ween: The Prophecy"},
	{"bargon", "Bargon Attack"},
	{"littlered", "Little Red Riding Hood"},
	{"ajworld", "A.J's World of Discovery"},
	{"gob3", "Goblins Quest 3"},
	{"gob3cd", "Goblins Quest 3 CD"},
	{"lostintime", "Lost in Time"},
	{"inca2", "Inca II: Wiracocha"},
	{"woodruff", "The Bizarre Adventures of Woodruff and the Schnibble"},
	{"dynasty", "The Last Dynasty"},
	{"urban", "Urban Runner"},
	{"archibald", "Playtoon 1 - Uncle Archibald"},
	{"spirou", "Playtoon 2 - Spirou"},
	{"fascination", "Fascination"},
	{"geisha", "Geisha"},
	{"adibou4", "Adibou v4"},
	{0, 0}
};

static const ADObsoleteGameID obsoleteGameIDsTable[] = {
	{"gob1", "gob", kPlatformUnknown},
	{"gob2", "gob", kPlatformUnknown},
	{0, 0, kPlatformUnknown}
};

namespace Gob {

static const GOBGameDescription gameDescriptions[] = {
	{ // Supplied by Florian Zeitz on scummvm-devel
		{
			"gob1",
			"EGA",
			AD_ENTRY1("intro.stk", "c65e9cc8ba23a38456242e1f2b1caad4"),
			UNK_LANG,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeGob1,
		kFeaturesEGA,
		0,
		0
	},
	{
		{
			"gob1",
			"EGA",
			AD_ENTRY1("intro.stk", "f9233283a0be2464248d83e14b95f09c"),
			RU_RUS,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeGob1,
		kFeaturesEGA,
		0,
		0
	},
	{ // Supplied by Theruler76 in bug report #1201233
		{
			"gob1",
			"VGA",
			AD_ENTRY1("intro.stk", "26a9118c0770fa5ac93a9626761600b2"),
			UNK_LANG,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeGob1,
		kFeaturesNone,
		0,
		0
	},
	{ // Supplied by raziel_ in bug report #1891864
		{
			"gob1",
			"VGA",
			AD_ENTRY1s("intro.stk", "e157cb59c6d330ca70d12ab0ef1dd12b", 288972),
			EN_GRB,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeGob1,
		kFeaturesAdlib,
		0,
		0
	},
	{ // CD 1.000 version.
		{
			"gob1cd",
			"v1.000",
			AD_ENTRY1("intro.stk", "2fbf4b5b82bbaee87eb45d4404c28998"),
			EN_USA,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeGob1,
		kFeaturesCD,
		0,
		0
	},
	{ // CD 1.000 version.
		{
			"gob1cd",
			"v1.000",
			AD_ENTRY1("intro.stk", "2fbf4b5b82bbaee87eb45d4404c28998"),
			DE_DEU,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeGob1,
		kFeaturesCD,
		0,
		0
	},
	{ // CD 1.000 version.
		{
			"gob1cd",
			"v1.000",
			AD_ENTRY1("intro.stk", "2fbf4b5b82bbaee87eb45d4404c28998"),
			FR_FRA,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeGob1,
		kFeaturesCD,
		0,
		0
	},
	{ // CD 1.000 version.
		{
			"gob1cd",
			"v1.000",
			AD_ENTRY1("intro.stk", "2fbf4b5b82bbaee87eb45d4404c28998"),
			IT_ITA,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeGob1,
		kFeaturesCD,
		0,
		0
	},
	{ // CD 1.000 version.
		{
			"gob1cd",
			"v1.000",
			AD_ENTRY1("intro.stk", "2fbf4b5b82bbaee87eb45d4404c28998"),
			ES_ESP,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeGob1,
		kFeaturesCD,
		0,
		0
	},
	{ // CD 1.02 version. Multilingual
		{
			"gob1cd",
			"v1.02",
			AD_ENTRY1("intro.stk", "8bd873137b6831c896ee8ad217a6a398"),
			EN_USA,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeGob1,
		kFeaturesCD,
		0,
		0
	},
	{ // CD 1.02 version. Multilingual
		{
			"gob1cd",
			"v1.02",
			AD_ENTRY1("intro.stk", "8bd873137b6831c896ee8ad217a6a398"),
			DE_DEU,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeGob1,
		kFeaturesCD,
		0,
		0
	},
	{ // CD 1.02 version. Multilingual
		{
			"gob1cd",
			"v1.02",
			AD_ENTRY1("intro.stk", "8bd873137b6831c896ee8ad217a6a398"),
			FR_FRA,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeGob1,
		kFeaturesCD,
		0,
		0
	},
	{ // CD 1.02 version. Multilingual
		{
			"gob1cd",
			"v1.02",
			AD_ENTRY1("intro.stk", "8bd873137b6831c896ee8ad217a6a398"),
			IT_ITA,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeGob1,
		kFeaturesCD,
		0,
		0
	},
	{ // CD 1.02 version. Multilingual
		{
			"gob1cd",
			"v1.02",
			AD_ENTRY1("intro.stk", "8bd873137b6831c896ee8ad217a6a398"),
			ES_ESP,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeGob1,
		kFeaturesCD,
		0,
		0
	},
	{
		{
			"gob1",
			"Demo",
			AD_ENTRY1("intro.stk", "972f22c6ff8144a6636423f0354ca549"),
			UNK_LANG,
			kPlatformAmiga,
			ADGF_DEMO
		},
		kGameTypeGob1,
		kFeaturesNone,
		0,
		0
	},
	{
		{
			"gob1",
			"Interactive Demo",
			AD_ENTRY1("intro.stk", "e72bd1e3828c7dec4c8a3e58c48bdfdb"),
			UNK_LANG,
			kPlatformPC,
			ADGF_DEMO
		},
		kGameTypeGob1,
		kFeaturesNone,
		0,
		0
	},
	{
		{
			"gob1",
			"Interactive Demo",
			AD_ENTRY1s("intro.stk", "a796096280d5efd48cf8e7dfbe426eb5", 193595),
			UNK_LANG,
			kPlatformPC,
			ADGF_DEMO
		},
		kGameTypeGob1,
		kFeaturesNone,
		0,
		0
	},
	{ // Supplied by raina in the forums
		{
			"gob1",
			"",
			AD_ENTRY1s("intro.stk", "6d837c6380d8f4d984c9f6cc0026df4f", 192712),
			EN_ANY,
			kPlatformMacintosh,
			ADGF_NO_FLAGS
		},
		kGameTypeGob1,
		kFeaturesNone,
		0,
		0
	},
	{ // Supplied by paul66 in bug report #1652352
		{
			"gob1",
			"",
			AD_ENTRY1("intro.stk", "00a42a7d2d22e6b6ab1b8c673c4ed267"),
			EN_ANY,
			kPlatformMacintosh,
			ADGF_NO_FLAGS
		},
		kGameTypeGob1,
		kFeaturesAdlib,
		0,
		0
	},
	{ // Supplied by paul66 in bug report #1652352
		{
			"gob1",
			"",
			AD_ENTRY1("intro.stk", "00a42a7d2d22e6b6ab1b8c673c4ed267"),
			DE_DEU,
			kPlatformMacintosh,
			ADGF_NO_FLAGS
		},
		kGameTypeGob1,
		kFeaturesAdlib,
		0,
		0
	},
	{ // Supplied by paul66 in bug report #1652352
		{
			"gob1",
			"",
			AD_ENTRY1("intro.stk", "00a42a7d2d22e6b6ab1b8c673c4ed267"),
			FR_FRA,
			kPlatformMacintosh,
			ADGF_NO_FLAGS
		},
		kGameTypeGob1,
		kFeaturesAdlib,
		0,
		0
	},
	{ // Supplied by paul66 in bug report #1652352
		{
			"gob1",
			"",
			AD_ENTRY1("intro.stk", "00a42a7d2d22e6b6ab1b8c673c4ed267"),
			IT_ITA,
			kPlatformMacintosh,
			ADGF_NO_FLAGS
		},
		kGameTypeGob1,
		kFeaturesAdlib,
		0,
		0
	},
	{ // Supplied by paul66 in bug report #1652352
		{
			"gob1",
			"",
			AD_ENTRY1("intro.stk", "00a42a7d2d22e6b6ab1b8c673c4ed267"),
			ES_ESP,
			kPlatformMacintosh,
			ADGF_NO_FLAGS
		},
		kGameTypeGob1,
		kFeaturesAdlib,
		0,
		0
	},
	{
		{
			"gob1",
			"",
			{
				{"intro.stk", 0, "f5f028ee39c456fa51fa63b606583918", 313472},
				{"musmac1.mid", 0, "4f66903b33df8a20edd4c748809c0b56", 8161},
				{NULL, 0, NULL, 0}
			},
			FR_FRA,
			kPlatformWindows,
			ADGF_NO_FLAGS
		},
		kGameTypeGob1,
		kFeaturesAdlib,
		0,
		0
	},
	{ // Supplied by fac76 in bug report #1883808
		{
			"gob2",
			"",
			AD_ENTRY1s("intro.stk", "eebf2810122cfd17399260cd1468e994", 554014),
			EN_ANY,
			kPlatformAmiga,
			ADGF_NO_FLAGS
		},
		kGameTypeGob2,
		kFeaturesNone,
		0,
		0
	},
	{
		{
			"gob2",
			"",
			AD_ENTRY1("intro.stk", "d28b9e9b41f31acfa58dcd12406c7b2c"),
			DE_DEU,
			kPlatformAmiga,
			ADGF_NO_FLAGS
		},
		kGameTypeGob2,
		kFeaturesNone,
		0,
		0
	},
	{ // Supplied by goodoldgeorg in bug report #2602057
		{
			"gob2",
			"",
			AD_ENTRY1("intro.stk", "686c88f7302a80b744aae9f8413e853d"),
			IT_ITA,
			kPlatformAmiga,
			ADGF_NO_FLAGS
		},
		kGameTypeGob2,
		kFeaturesNone,
		0,
		0
	},
	{ // Supplied by bgk in bug report #1706861
		{
			"gob2",
			"",
			AD_ENTRY1s("intro.stk", "4b13c02d1069b86bcfec80f4e474b98b", 554680),
			FR_FRA,
			kPlatformAtariST,
			ADGF_NO_FLAGS
		},
		kGameTypeGob2,
		kFeaturesNone,
		0,
		0
	},
	{ // Supplied by fac76 in bug report #1673397
		{
			"gob2",
			"",
			{
				{"intro.stk", 0, "b45b984ee8017efd6ea965b9becd4d66", 828443},
				{"musmac1.mid", 0, "7f96f491448c7a001b32df89cf8d2af2", 1658},
				{NULL, 0, NULL, 0}
			},
			UNK_LANG,
			kPlatformMacintosh,
			ADGF_NO_FLAGS
		},
		kGameTypeGob2,
		kFeaturesAdlib,
		0,
		0
	},
	{ // Supplied by koalet in bug report #2478585
		{
			"gob2",
			"",
			{
				{"intro.stk", 0, "a13ecb4f6d8fd881ebbcc02e45cb5475", 837275},
				{"musmac1.mid", 0, "7f96f491448c7a001b32df89cf8d2af2", 1658},
				{NULL, 0, NULL, 0}
			},
			FR_FRA,
			kPlatformMacintosh,
			ADGF_NO_FLAGS
		},
		kGameTypeGob2,
		kFeaturesAdlib,
		0,
		0
	},
	{
		{
			"gob2",
			"",
			AD_ENTRY1("intro.stk", "b45b984ee8017efd6ea965b9becd4d66"),
			EN_GRB,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeGob2,
		kFeaturesAdlib,
		0,
		0
	},
	{
		{
			"gob2",
			"",
			AD_ENTRY1("intro.stk", "dedb5d31d8c8050a8cf77abedcc53dae"),
			EN_USA,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeGob2,
		kFeaturesAdlib,
		0,
		0
	},
	{ // Supplied by raziel_ in bug report #1891867
		{
			"gob2",
			"",
			AD_ENTRY1s("intro.stk", "25a99827cd59751a80bed9620fb677a0", 893302),
			EN_USA,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeGob2,
		kFeaturesAdlib,
		0,
		0
	},
	{
		{
			"gob2",
			"",
			AD_ENTRY1s("intro.stk", "a13ecb4f6d8fd881ebbcc02e45cb5475", 837275),
			FR_FRA,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeGob2,
		kFeaturesAdlib,
		0,
		0
	},
	{ // Supplied by blackwhiteeagle in bug report #1605235
		{
			"gob2",
			"",
			AD_ENTRY1("intro.stk", "3e4e7db0d201587dd2df4003b2993ef6"),
			DE_DEU,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeGob2,
		kFeaturesAdlib,
		0,
		0
	},
	{
		{
			"gob2",
			"",
			AD_ENTRY1("intro.stk", "a13892cdf4badda85a6f6fb47603a128"),
			DE_DEU,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeGob2,
		kFeaturesAdlib,
		0,
		0
	},
	{ // Supplied by goodoldgeorg in bug report #2602017
		{
			"gob2",
			"",
			AD_ENTRY1("intro.stk", "c47faf1d406504e6ffe63243610bb1f4"),
			IT_ITA,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeGob2,
		kFeaturesAdlib,
		0,
		0
	},
	{
		{
			"gob2",
			"",
			AD_ENTRY1("intro.stk", "cd3e1df8b273636ee32e34b7064f50e8"),
			RU_RUS,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeGob2,
		kFeaturesAdlib,
		0,
		0
	},
	{ // Supplied by arcepi in bug report #1659884
		{
			"gob2",
			"",
			AD_ENTRY1s("intro.stk", "5f53c56e3aa2f1e76c2e4f0caa15887f", 829232),
			ES_ESP,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeGob2,
		kFeaturesAdlib,
		0,
		0
	},
	{
		{
			"gob2cd",
			"v1.000",
			AD_ENTRY1("intro.stk", "9de5fbb41cf97182109e5fecc9d90347"),
			EN_USA,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeGob2,
		kFeaturesCD,
		0,
		0
	},
	{
		{
			"gob2cd",
			"v1.02",
			AD_ENTRY1("intro.stk", "24a6b32757752ccb1917ce92fd7c2a04"),
			EN_ANY,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeGob2,
		kFeaturesCD,
		0,
		0
	},
	{
		{
			"gob2cd",
			"v1.02",
			AD_ENTRY1("intro.stk", "24a6b32757752ccb1917ce92fd7c2a04"),
			DE_DEU,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeGob2,
		kFeaturesCD,
		0,
		0
	},
	{
		{
			"gob2cd",
			"v1.02",
			AD_ENTRY1("intro.stk", "24a6b32757752ccb1917ce92fd7c2a04"),
			FR_FRA,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeGob2,
		kFeaturesCD,
		0,
		0
	},
	{
		{
			"gob2cd",
			"v1.02",
			AD_ENTRY1("intro.stk", "24a6b32757752ccb1917ce92fd7c2a04"),
			IT_ITA,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeGob2,
		kFeaturesCD,
		0,
		0
	},
	{
		{
			"gob2cd",
			"v1.02",
			AD_ENTRY1("intro.stk", "24a6b32757752ccb1917ce92fd7c2a04"),
			ES_ESP,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeGob2,
		kFeaturesCD,
		0,
		0
	},
	{
		{
			"gob2",
			"Non-Interactive Demo",
			AD_ENTRY1("intro.stk", "8b1c98ff2ab2e14f47a1b891e9b92217"),
			UNK_LANG,
			kPlatformPC,
			ADGF_DEMO
		},
		kGameTypeGob2,
		kFeaturesAdlib,
		0,
		"usa.tot"
	},
	{
		{
			"gob2",
			"Interactive Demo",
			AD_ENTRY1("intro.stk", "cf1c95b2939bd8ff58a25c756cb6125e"),
			UNK_LANG,
			kPlatformPC,
			ADGF_DEMO
		},
		kGameTypeGob2,
		kFeaturesAdlib,
		0,
		0
	},
	{
		{
			"gob2",
			"Interactive Demo",
			AD_ENTRY1("intro.stk", "4b278c2678ea01383fd5ca114d947eea"),
			UNK_LANG,
			kPlatformAmiga,
			ADGF_DEMO
		},
		kGameTypeGob2,
		kFeaturesNone,
		0,
		0
	},
	{ // Supplied by polluks in bug report #1895126
		{
			"gob2",
			"Interactive Demo",
			AD_ENTRY1s("intro.stk", "9fa85aea959fa8c582085855fbd99346", 553063),
			UNK_LANG,
			kPlatformAmiga,
			ADGF_DEMO
		},
		kGameTypeGob2,
		kFeaturesNone,
		0,
		0
	},
	{
		{
			"gob2",
			"",
			{
				{"intro.stk", 0, "285d7340f98ebad65d465585da12910b", 837286},
				{"musmac1.mid", 0, "834e55205b710d0af5f14a6f2320dd8e", 8661},
				{NULL, 0, NULL, 0}
			},
			FR_FRA,
			kPlatformWindows,
			ADGF_NO_FLAGS
		},
		kGameTypeGob2,
		kFeaturesAdlib,
		0,
		0
	},
	{ // Supplied by vampir_raziel in bug report #1658373
		{
			"ween",
			"",
			{
				{"intro.stk", 0, "bfd9d02faf3d8d60a2cf744f95eb48dd", 456570},
				{"ween.ins", 0, "d2cb24292c9ddafcad07e23382027218", 87800},
				{NULL, 0, NULL, 0}
			},
			EN_GRB,
			kPlatformAmiga,
			ADGF_NO_FLAGS
		},
		kGameTypeWeen,
		kFeaturesNone,
		0,
		0
	},
	{ // Supplied by vampir_raziel in bug report #1658373
		{
			"ween",
			"",
			AD_ENTRY1s("intro.stk", "257fe669705ac4971efdfd5656eef16a", 457719),
			FR_FRA,
			kPlatformAmiga,
			ADGF_NO_FLAGS
		},
		kGameTypeWeen,
		kFeaturesNone,
		0,
		0
	},
	{ // Supplied by vampir_raziel in bug report #1658373
		{
			"ween",
			"",
			AD_ENTRY1s("intro.stk", "dffd1ab98fe76150d6933329ca6f4cc4", 459458),
			FR_FRA,
			kPlatformAmiga,
			ADGF_NO_FLAGS
		},
		kGameTypeWeen,
		kFeaturesNone,
		0,
		0
	},
	{ // Supplied by vampir_raziel in bug report #1658373
		{
			"ween",
			"",
			AD_ENTRY1s("intro.stk", "af83debf2cbea21faa591c7b4608fe92", 458192),
			DE_DEU,
			kPlatformAmiga,
			ADGF_NO_FLAGS
		},
		kGameTypeWeen,
		kFeaturesNone,
		0,
		0
	},
	{ // Supplied by goodoldgeorg in bug report #2563539 
		{
			"ween",
			"",
			{
				{"intro.stk", 0, "dffd1ab98fe76150d6933329ca6f4cc4", 459458},
				{"ween.ins", 0, "d2cb24292c9ddafcad07e23382027218", 87800},
				{NULL, 0, NULL, 0}
			},
			IT_ITA,
			kPlatformAmiga,
			ADGF_NO_FLAGS
		},
		kGameTypeWeen,
		kFeaturesNone,
		0,
		0
	},
	{ // Supplied by pwigren in bug report #1764174
		{
			"ween",
			"",
			{
				{"intro.stk", 0, "bfd9d02faf3d8d60a2cf744f95eb48dd", 456570},
				{"music__5.snd", 0, "7d1819b9981ecddd53d3aacbc75f1cc8", 13446},
				{NULL, 0, NULL, 0}
			},
			EN_GRB,
			kPlatformAtariST,
			ADGF_NO_FLAGS
		},
		kGameTypeWeen,
		kFeaturesNone,
		0,
		0
	},
	{
		{
			"ween",
			"",
			AD_ENTRY1("intro.stk", "e6d13fb3b858cb4f78a8780d184d5b2c"),
			FR_FRA,
			kPlatformAtariST,
			ADGF_NO_FLAGS
		},
		kGameTypeWeen,
		kFeaturesNone,
		0,
		0
	},
	{
		{
			"ween",
			"",
			AD_ENTRY1("intro.stk", "2bb8878a8042244dd2b96ff682381baa"),
			EN_GRB,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeWeen,
		kFeaturesAdlib,
		0,
		0
	},
	{
		{
			"ween",
			"",
			AD_ENTRY1s("intro.stk", "de92e5c6a8c163007ffceebef6e67f7d", 7117568),
			EN_USA,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeWeen,
		kFeaturesAdlib,
		0,
		0
	},
	{ // Supplied by cybot_tmin in bug report #1667743
		{
			"ween",
			"",
			AD_ENTRY1s("intro.stk", "6d60f9205ecfbd8735da2ee7823a70dc", 7014426),
			ES_ESP,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeWeen,
		kFeaturesAdlib,
		0,
		0
	},
	{
		{
			"ween",
			"",
			AD_ENTRY1("intro.stk", "4b10525a3782aa7ecd9d833b5c1d308b"),
			FR_FRA,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeWeen,
		kFeaturesAdlib,
		0,
		0
	},
	{ // Supplied by cartman_ on #scummvm
		{
			"ween",
			"",
			AD_ENTRY1("intro.stk", "63170e71f04faba88673b3f510f9c4c8"),
			DE_DEU,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeWeen,
		kFeaturesAdlib,
		0,
		0
	},
	{ // Supplied by glorfindel in bugreport #1722142
		{
			"ween",
			"",
			AD_ENTRY1s("intro.stk", "8b57cd510da8a3bbd99e3a0297a8ebd1", 7018771),
			IT_ITA,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeWeen,
		kFeaturesAdlib,
		0,
		0
	},
	{
		{
			"ween",
			"Demo",
			AD_ENTRY1("intro.stk", "2e9c2898f6bf206ede801e3b2e7ee428"),
			UNK_LANG,
			kPlatformPC,
			ADGF_DEMO
		},
		kGameTypeWeen,
		kFeaturesAdlib,
		0,
		"show.tot"
	},
	{
		{
			"ween",
			"Demo",
			AD_ENTRY1("intro.stk", "15fb91a1b9b09684b28ac75edf66e504"),
			EN_USA,
			kPlatformPC,
			ADGF_DEMO
		},
		kGameTypeWeen,
		kFeaturesAdlib,
		0,
		"show.tot"
	},
	{
		{
			"bargon",
			"",
			AD_ENTRY1("intro.stk", "da3c54be18ab73fbdb32db24624a9c23"),
			UNK_LANG,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeBargon,
		kFeaturesNone,
		0,
		0
	},
	{ // Supplied by Trekky in the forums
		{
			"bargon",
			"",
			AD_ENTRY1s("intro.stk", "2f54b330d21f65b04b7c1f8cca76426c", 262109),
			FR_FRA,
			kPlatformAtariST,
			ADGF_NO_FLAGS
		},
		kGameTypeBargon,
		kFeaturesNone,
		0,
		0
	},
	{ // Supplied by cesardark in bug #1681649
		{
			"bargon",
			"",
			AD_ENTRY1s("intro.stk", "11103b304286c23945560b391fd37e7d", 3181890),
			ES_ESP,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeBargon,
		kFeaturesNone,
		0,
		0
	},
	{ // Supplied by paul66 in bug #1692667
		{
			"bargon",
			"",
			AD_ENTRY1s("intro.stk", "da3c54be18ab73fbdb32db24624a9c23", 3181825),
			DE_DEU,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeBargon,
		kFeaturesNone,
		0,
		0
	},
	{ // Supplied by pwigren in bugreport #1764174
		{
			"bargon",
			"",
			AD_ENTRY1s("intro.stk", "569d679fe41d49972d34c9fce5930dda", 269825),
			EN_ANY,
			kPlatformAmiga,
			ADGF_NO_FLAGS
		},
		kGameTypeBargon,
		kFeaturesNone,
		0,
		0
	},
	{ // Supplied by kizkoool in bugreport #2089734
		{
			"bargon",
			"",
			AD_ENTRY1s("intro.stk", "00f6b4e2ee26e5c40b488e2df5adcf03", 3975580),
			FR_FRA,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeBargon,
		kFeaturesNone,
		0,
		0
	},
	{ // Supplied by glorfindel in bugreport #1722142
		{
			"bargon",
			"Fanmade",
			AD_ENTRY1s("intro.stk", "da3c54be18ab73fbdb32db24624a9c23", 3181825),
			IT_ITA,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeBargon,
		kFeaturesNone,
		0,
		0
	},
	{
		{
			"littlered",
			"",
			AD_ENTRY1s("intro.stk", "0b72992f5d8b5e6e0330572a5753ea25", 256490),
			UNK_LANG,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeGob2,
		kFeaturesAdlib | kFeaturesEGA,
		0,
		0
	},
	{
		{
			"littlered",
			"",
			{
				{"intro.stk", 0, "0b72992f5d8b5e6e0330572a5753ea25", 256490},
				{"mod.babayaga", 0, "43484cde74e0860785f8e19f0bc776d1", 60248},
				{NULL, 0, NULL, 0}
			},
			UNK_LANG,
			kPlatformAmiga,
			ADGF_NO_FLAGS
		},
		kGameTypeGob2,
		kFeaturesNone,
		0,
		0
	},
	{
		{
			"ajworld",
			"",
			AD_ENTRY1s("intro.stk", "e453bea7b28a67c930764d945f64d898", 3913628),
			EN_ANY,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeGob2,
		kFeaturesAdlib,
		0,
		0
	},
	{
		{
			"lostintime",
			"",
			AD_ENTRY1s("intro.stk", "7b7f48490dedc8a7cb999388e2fadbe3", 3930674),
			EN_USA,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeLostInTime,
		kFeaturesAdlib,
		0,
		0
	},
	{ // Supplied by Arshlan in the forums
		{
			"lostintime",
			"",
			AD_ENTRY1s("intro.stk", "3712e7527ba8ce5637d2aadf62783005", 72318),
			FR_FRA,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeLostInTime,
		kFeaturesAdlib,
		0,
		0
	},
	{ // Supplied by cartman_ on #scummvm
		{
			"lostintime",
			"",
			AD_ENTRY1s("intro.stk", "f1f78b663893b58887add182a77df151", 3944090),
			DE_DEU,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeLostInTime,
		kFeaturesAdlib,
		0,
		0
	},
	{ // Supplied by goodoldgeorg in bug report #2105220
		{
			"lostintime",
			"",
			AD_ENTRY1s("intro.stk", "cd322cb3c64ef2ba2f2134aa2122cfe9", 3936700),
			ES_ESP,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeLostInTime,
		kFeaturesAdlib,
		0,
		0
	},
	{ // Supplied by koalet in bug report #2479034
		{
			"lostintime",
			"",
			{
				{"intro.stk", 0, "af98bcdc70e1f1c1635577fd726fe7f1", 3937310},
				{"musmac1.mid", 0, "ae7229bb09c6abe4e60a2768b24bc890", 9398},
				{NULL, 0, NULL, 0}
			},
			FR_FRA,
			kPlatformMacintosh,
			ADGF_NO_FLAGS
		},
		kGameTypeLostInTime,
		kFeaturesAdlib,
		0,
		0
	},
	{
		{
			"lostintime",
			"",
			AD_ENTRY1s("intro.stk", "6263d09e996c1b4e84ef2d650b820e57", 4831170),
			EN_USA,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeLostInTime,
		kFeaturesCD,
		0,
		0
	},
	{
		{
			"lostintime",
			"",
			AD_ENTRY1s("intro.stk", "6263d09e996c1b4e84ef2d650b820e57", 4831170),
			FR_FRA,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeLostInTime,
		kFeaturesCD,
		0,
		0
	},
	{
		{
			"lostintime",
			"",
			AD_ENTRY1s("intro.stk", "6263d09e996c1b4e84ef2d650b820e57", 4831170),
			IT_ITA,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeLostInTime,
		kFeaturesCD,
		0,
		0
	},
	{
		{
			"lostintime",
			"",
			AD_ENTRY1s("intro.stk", "6263d09e996c1b4e84ef2d650b820e57", 4831170),
			DE_DEU,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeLostInTime,
		kFeaturesCD,
		0,
		0
	},
	{
		{
			"lostintime",
			"",
			AD_ENTRY1s("intro.stk", "6263d09e996c1b4e84ef2d650b820e57", 4831170),
			ES_ESP,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeLostInTime,
		kFeaturesCD,
		0,
		0
	},
	{
		{
			"lostintime",
			"",
			AD_ENTRY1s("intro.stk", "6263d09e996c1b4e84ef2d650b820e57", 4831170),
			EN_GRB,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeLostInTime,
		kFeaturesCD,
		0,
		0
	},
	{ // Supplied by SiRoCs in bug report #2093672
		{
			"lostintime",
			"",
			AD_ENTRY1s("intro.stk", "795be7011ec31bf5bb8ce4efdb9ee5d3", 4838904),
			EN_USA,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeLostInTime,
		kFeaturesCD,
		0,
		0
	},
	{ // Supplied by SiRoCs in bug report #2093672
		{
			"lostintime",
			"",
			AD_ENTRY1s("intro.stk", "795be7011ec31bf5bb8ce4efdb9ee5d3", 4838904),
			FR_FRA,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeLostInTime,
		kFeaturesCD,
		0,
		0
	},
	{ // Supplied by SiRoCs in bug report #2093672
		{
			"lostintime",
			"",
			AD_ENTRY1s("intro.stk", "795be7011ec31bf5bb8ce4efdb9ee5d3", 4838904),
			IT_ITA,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeLostInTime,
		kFeaturesCD,
		0,
		0
	},
	{ // Supplied by SiRoCs in bug report #2093672
		{
			"lostintime",
			"",
			AD_ENTRY1s("intro.stk", "795be7011ec31bf5bb8ce4efdb9ee5d3", 4838904),
			DE_DEU,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeLostInTime,
		kFeaturesCD,
		0,
		0
	},
	{ // Supplied by SiRoCs in bug report #2093672
		{
			"lostintime",
			"",
			AD_ENTRY1s("intro.stk", "795be7011ec31bf5bb8ce4efdb9ee5d3", 4838904),
			ES_ESP,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeLostInTime,
		kFeaturesCD,
		0,
		0
	},
	{ // Supplied by SiRoCs in bug report #2093672
		{
			"lostintime",
			"",
			AD_ENTRY1s("intro.stk", "795be7011ec31bf5bb8ce4efdb9ee5d3", 4838904),
			EN_GRB,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeLostInTime,
		kFeaturesCD,
		0,
		0
	},
	{
		{
			"fascination",
			"CD Version (Censored)",
			AD_ENTRY1s("disk0.stk", "9c61e9c22077f72921f07153e37ccf01", 545952),
			UNK_LANG,
			kPlatformPC,
			ADGF_NO_FLAGS,
		},
		kGameTypeFascination,
		kFeaturesCD,
		"disk0.stk",
		0
	},
	{
		{
			"fascination",
			"VGA 3 disks edition",
			AD_ENTRY1s("disk0.stk", "a50a8495e1b2d67699fb562cb98fc3e2", 1064387),
			UNK_LANG,
			kPlatformPC,
			ADGF_NO_FLAGS,
		},
		kGameTypeFascination,
		kFeaturesNone,
		"disk0.stk",
		0
	},
	//Provided by Sanguine
	{
		{
			"fascination",
			"VGA 3 disks edition",
			AD_ENTRY1s("disk0.stk", "c14330d052fe4da5a441ac9d81bc5891", 1061955),
			UNK_LANG,
			kPlatformPC,
			ADGF_NO_FLAGS,
		},
		kGameTypeFascination,
		kFeaturesNone,
		"disk0.stk",
		0
	},
	{
		{
			"fascination",
			"VGA",
			AD_ENTRY1s("disk0.stk", "e8ab4f200a2304849f462dc901705599", 183337),
			UNK_LANG,
			kPlatformPC,
			ADGF_NO_FLAGS,
		},
		kGameTypeFascination,
		kFeaturesNone,
		"disk0.stk",
		0
	},
	{
		{
			"fascination",
			"",
			AD_ENTRY1s("disk0.stk", "68b1c01564f774c0b640075fbad1b695", 189968),
			DE_DEU,
			kPlatformAmiga,
			ADGF_NO_FLAGS,
		},
		kGameTypeFascination,
		kFeaturesNone,
		"disk0.stk",
		0
	},	
	{
		{
			"fascination",
			"",
			AD_ENTRY1s("disk0.stk", "7062117e9c5adfb6bfb2dac3ff74df9e", 189951),
			EN_ANY,
			kPlatformAmiga,
			ADGF_NO_FLAGS,
		},
		kGameTypeFascination,
		kFeaturesNone,
		"disk0.stk",
		0
	},	
	{
		{
			"fascination",
			"",
			AD_ENTRY1s("disk0.stk", "55c154e5a3e8e98afebdcff4b522e1eb", 190005),
			FR_FRA,
			kPlatformAmiga,
			ADGF_NO_FLAGS,
		},
		kGameTypeFascination,
		kFeaturesNone,
		"disk0.stk",
		0
	},	
	{
		{
			"fascination",
			"",
			AD_ENTRY1s("disk0.stk", "7691827fff35df7799f14cfd6be178ad", 189931),
			IT_ITA,
			kPlatformAmiga,
			ADGF_NO_FLAGS,
		},
		kGameTypeFascination,
		kFeaturesNone,
		"disk0.stk",
		0
	},	
	{
		{
			"geisha",
			"",
			AD_ENTRY1s("disk1.stk", "6eebbb98ad90cd3c44549fc2ab30f632", 212153),
			UNK_LANG,
			kPlatformPC,
			ADGF_NO_FLAGS,
		},
		kGameTypeGeisha,
		kFeaturesNone,
		"disk1.stk",
		"intro.tot"
	},
	{
		{
			"lostintime",
			"Demo",
			AD_ENTRY1("demo.stk", "c06f8cc20eb239d4c71f225ce3093edf"),
			UNK_LANG,
			kPlatformPC,
			ADGF_DEMO
		},
		kGameTypeLostInTime,
		kFeaturesAdlib,
		"demo.stk",
		"demo.tot"
	},
	{
		{
			"lostintime",
			"Non-interactive Demo",
			AD_ENTRY1("demo.stk", "2eba8abd9e3878c57307576012dd2fec"),
			UNK_LANG,
			kPlatformPC,
			ADGF_DEMO
		},
		kGameTypeLostInTime,
		kFeaturesAdlib,
		"demo.stk",
		"demo.tot"
	},
	{
		{
			"gob3",
			"",
			AD_ENTRY1s("intro.stk", "32b0f57f5ae79a9ae97e8011df38af42", 157084),
			EN_GRB,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeGob3,
		kFeaturesAdlib,
		0,
		0
	},
	{ // Supplied by raziel_ in bug report #1891869
		{
			"gob3",
			"",
			AD_ENTRY1s("intro.stk", "16b014bf32dbd6ab4c5163c44f56fed1", 445104),
			EN_GRB,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeGob3,
		kFeaturesAdlib,
		0,
		0
	},
	{ // Supplied by fac76 in bug report #1742716
		{
			"gob3",
			"",
			{
				{"intro.stk", 0, "32b0f57f5ae79a9ae97e8011df38af42", 157084},
				{"musmac1.mid", 0, "834e55205b710d0af5f14a6f2320dd8e", 8661},
				{NULL, 0, NULL, 0}
			},
			EN_GRB,
			kPlatformMacintosh,
			ADGF_NO_FLAGS
		},
		kGameTypeGob3,
		kFeaturesAdlib,
		0,
		0
	},
	{
		{
			"gob3",
			"",
			AD_ENTRY1("intro.stk", "1e2f64ec8dfa89f42ee49936a27e66e7"),
			EN_USA,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeGob3,
		kFeaturesAdlib,
		0,
		0
	},
	{ // Supplied by paul66 in bug report #1652352
		{
			"gob3",
			"",
			AD_ENTRY1("intro.stk", "f6d225b25a180606fa5dbe6405c97380"),
			DE_DEU,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeGob3,
		kFeaturesAdlib,
		0,
		0
	},
	{
		{
			"gob3",
			"",
			AD_ENTRY1("intro.stk", "e42a4f2337d6549487a80864d7826972"),
			FR_FRA,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeGob3,
		kFeaturesAdlib,
		0,
		0
	},
	{ // Supplied by Paranoimia on #scummvm
		{
			"gob3",
			"",
			AD_ENTRY1s("intro.stk", "fe8144daece35538085adb59c2d29613", 159402),
			IT_ITA,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeGob3,
		kFeaturesAdlib,
		0,
		0
	},
	{
		{
			"gob3",
			"",
			AD_ENTRY1("intro.stk", "4e3af248a48a2321364736afab868527"),
			RU_RUS,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeGob3,
		kFeaturesAdlib,
		0,
		0
	},
	{
		{
			"gob3",
			"",
			AD_ENTRY1("intro.stk", "8d28ce1591b0e9cc79bf41cad0fc4c9c"),
			UNK_LANG,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeGob3,
		kFeaturesAdlib,
		0,
		0
	},
	{ // Supplied by SiRoCs in bug report #2098621
		{
			"gob3",
			"",
			AD_ENTRY1s("intro.stk", "d3b72938fbbc8159198088811f9e6d19", 160382),
			ES_ESP,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeGob3,
		kFeaturesAdlib,
		0,
		0
	},
	{
		{
			"gob3",
			"",
			AD_ENTRY1("intro.stk", "bd679eafde2084d8011f247e51b5a805"),
			EN_GRB,
			kPlatformAmiga,
			ADGF_NO_FLAGS
		},
		kGameTypeGob3,
		kFeaturesNone,
		0,
		"menu.tot"
	},
	{
		{
			"gob3",
			"",
			AD_ENTRY1("intro.stk", "bd679eafde2084d8011f247e51b5a805"),
			DE_DEU,
			kPlatformAmiga,
			ADGF_NO_FLAGS
		},
		kGameTypeGob3,
		kFeaturesNone,
		0,
		"menu.tot"
	},
	{
		{
			"gob3cd",
			"v1.000",
			AD_ENTRY1("intro.stk", "6f2c226c62dd7ab0ab6f850e89d3fc47"),
			EN_USA,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeGob3,
		kFeaturesCD,
		0,
		0
	},
	{ // Supplied by paul66 and noizert in bug reports #1652352 and #1691230
		{
			"gob3cd",
			"v1.02",
			AD_ENTRY1("intro.stk", "c3e9132ea9dc0fb866b6d60dcda10261"),
			EN_ANY,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeGob3,
		kFeaturesCD,
		0,
		0
	},
	{ // Supplied by paul66 and noizert in bug reports #1652352 and #1691230
		{
			"gob3cd",
			"v1.02",
			AD_ENTRY1("intro.stk", "c3e9132ea9dc0fb866b6d60dcda10261"),
			DE_DEU,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeGob3,
		kFeaturesCD,
		0,
		0
	},
	{ // Supplied by paul66 and noizert in bug reports #1652352 and #1691230
		{
			"gob3cd",
			"v1.02",
			AD_ENTRY1("intro.stk", "c3e9132ea9dc0fb866b6d60dcda10261"),
			FR_FRA,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeGob3,
		kFeaturesCD,
		0,
		0
	},
	{ // Supplied by paul66 and noizert in bug reports #1652352 and #1691230
		{
			"gob3cd",
			"v1.02",
			AD_ENTRY1("intro.stk", "c3e9132ea9dc0fb866b6d60dcda10261"),
			IT_ITA,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeGob3,
		kFeaturesCD,
		0,
		0
	},
	{ // Supplied by paul66 and noizert in bug reports #1652352 and #1691230
		{
			"gob3cd",
			"v1.02",
			AD_ENTRY1("intro.stk", "c3e9132ea9dc0fb866b6d60dcda10261"),
			ES_ESP,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeGob3,
		kFeaturesCD,
		0,
		0
	},
	{
		{
			"gob3",
			"Interactive Demo",
			AD_ENTRY1("intro.stk", "7aebd94e49c2c5c518c9e7b74f25de9d"),
			FR_FRA,
			kPlatformPC,
			ADGF_DEMO
		},
		kGameTypeGob3,
		kFeaturesAdlib,
		0,
		0
	},
	{
		{
			"gob3",
			"Interactive Demo 2",
			AD_ENTRY1("intro.stk", "e5dcbc9f6658ebb1e8fe26bc4da0806d"),
			FR_FRA,
			kPlatformPC,
			ADGF_DEMO
		},
		kGameTypeGob3,
		kFeaturesAdlib,
		0,
		0
	},
	{
		{
			"gob3",
			"Non-interactive Demo",
			AD_ENTRY1("intro.stk", "b9b898fccebe02b69c086052d5024a55"),
			UNK_LANG,
			kPlatformPC,
			ADGF_DEMO
		},
		kGameTypeGob3,
		kFeaturesAdlib,
		0,
		0
	},
	{
		{
			"gob3",
			"Interactive Demo 3",
			AD_ENTRY1s("intro.stk", "9e20ad7b471b01f84db526da34eaf0a2", 395561),
			EN_ANY,
			kPlatformPC,
			ADGF_DEMO
		},
		kGameTypeGob3,
		kFeaturesAdlib,
		0,
		0
	},
	{
		{
			"gob3",
			"",
			{
				{"intro.stk", 0, "edd7403e5dc2a14459d2665a4c17714d", 209534},
				{"musmac1.mid", 0, "948c546cad3a9de5bff3fe4107c82bf1", 6404},
				{NULL, 0, NULL, 0}
			},
			FR_FRA,
			kPlatformWindows,
			ADGF_NO_FLAGS
		},
		kGameTypeGob3,
		kFeaturesAdlib,
		0,
		0
	},
	{
		{
			"inca2",
			"",
			AD_ENTRY1s("intro.stk", "47c3b452767c4f49ea7b109143e77c30", 916828),
			EN_USA,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeInca2,
		kFeaturesCD,
		0,
		0
	},
	{
		{
			"inca2",
			"",
			AD_ENTRY1s("intro.stk", "47c3b452767c4f49ea7b109143e77c30", 916828),
			DE_DEU,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeInca2,
		kFeaturesCD,
		0,
		0
	},
	{
		{
			"inca2",
			"",
			AD_ENTRY1s("intro.stk", "47c3b452767c4f49ea7b109143e77c30", 916828),
			FR_FRA,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeInca2,
		kFeaturesCD,
		0,
		0
	},
	{
		{
			"inca2",
			"",
			AD_ENTRY1s("intro.stk", "47c3b452767c4f49ea7b109143e77c30", 916828),
			IT_ITA,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeInca2,
		kFeaturesCD,
		0,
		0
	},
	{
		{
			"inca2",
			"",
			AD_ENTRY1s("intro.stk", "47c3b452767c4f49ea7b109143e77c30", 916828),
			ES_ESP,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeInca2,
		kFeaturesCD,
		0,
		0
	},
	{
		{
			"inca2",
			"",
			AD_ENTRY1s("intro.stk", "1fa92b00fe80a20f34ec34a8e2fa869e", 923072),
			EN_USA,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeInca2,
		kFeaturesAdlib,
		0,
		0
	},
	{
		{
			"inca2",
			"",
			AD_ENTRY1s("intro.stk", "1fa92b00fe80a20f34ec34a8e2fa869e", 923072),
			FR_FRA,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeInca2,
		kFeaturesAdlib,
		0,
		0
	},
	{
		{
			"inca2",
			"",
			AD_ENTRY1s("intro.stk", "1fa92b00fe80a20f34ec34a8e2fa869e", 923072),
			DE_DEU,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeInca2,
		kFeaturesAdlib,
		0,
		0
	},
	{
		{
			"inca2",
			"Non-Interactive Demo",
			{
				{"demo.bat", 0, "01a1c983c3d360cd4a96f93961a805de", 483},
				{"cons.imd", 0, "f896ba0c4a1ac7f7260d342655980b49", 17804},
				{"conseil.imd", 0, "aaedd5482d5b271e233e86c5a03cf62e", 33999},
				{"int.imd", 0, "6308222fcefbcb20925f01c1aff70dee", 30871},
				{"inter.imd", 0, "39bd6d3540f3bedcc97293f352c7f3fc", 191719},
				{"machu.imd", 0, "c0bc8211d93b467bfd063b63fe61b85c", 34609},
				{"post.imd", 0, "d75cad0e3fc22cb0c8b6faf597f509b2", 1047709},
				{"posta.imd", 0, "2a5b3fe75681ddf4d21ac724db8111b4", 547250},
				{"postb.imd", 0, "24260ce4e80a4c472352b76637265d09", 868312},
				{"postc.imd", 0, "24accbcc8b83a9c2be4bd82849a2bd29", 415637},
				{"tum.imd", 0, "0993d4810ec9deb3f77c5e92095320fd", 20330},
				{"tumi.imd", 0, "bf53f229480d694de0947fe3366fbec6", 248952},
				{NULL, 0, NULL, 0}
			},
			EN_ANY,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeInca2,
		kFeaturesAdlib | kFeaturesBATDemo,
		0,
		"demo.bat"
	},
	{
		{
			"woodruff",
			"",
			AD_ENTRY1s("intro.stk", "dccf9d31cb720b34d75487408821b77e", 20296390),
			EN_GRB,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeWoodruff,
		kFeatures640,
		0,
		0
	},
	{
		{
			"woodruff",
			"",
			AD_ENTRY1s("intro.stk", "dccf9d31cb720b34d75487408821b77e", 20296390),
			DE_DEU,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeWoodruff,
		kFeatures640,
		0,
		0
	},
	{
		{
			"woodruff",
			"",
			AD_ENTRY1s("intro.stk", "dccf9d31cb720b34d75487408821b77e", 20296390),
			FR_FRA,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeWoodruff,
		kFeatures640,
		0,
		0
	},
	{
		{
			"woodruff",
			"",
			AD_ENTRY1s("intro.stk", "dccf9d31cb720b34d75487408821b77e", 20296390),
			IT_ITA,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeWoodruff,
		kFeatures640,
		0,
		0
	},
	{
		{
			"woodruff",
			"",
			AD_ENTRY1s("intro.stk", "dccf9d31cb720b34d75487408821b77e", 20296390),
			ES_ESP,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeWoodruff,
		kFeatures640,
		0,
		0
	},
	{
		{
			"woodruff",
			"",
			AD_ENTRY1s("intro.stk", "b50fee012a5abcd0ac2963e1b4b56bec", 20298108),
			EN_GRB,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeWoodruff,
		kFeatures640,
		0,
		0
	},
	{
		{
			"woodruff",
			"",
			AD_ENTRY1s("intro.stk", "b50fee012a5abcd0ac2963e1b4b56bec", 20298108),
			DE_DEU,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeWoodruff,
		kFeatures640,
		0,
		0
	},
	{
		{
			"woodruff",
			"",
			AD_ENTRY1s("intro.stk", "b50fee012a5abcd0ac2963e1b4b56bec", 20298108),
			FR_FRA,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeWoodruff,
		kFeatures640,
		0,
		0
	},
	{
		{
			"woodruff",
			"",
			AD_ENTRY1s("intro.stk", "b50fee012a5abcd0ac2963e1b4b56bec", 20298108),
			IT_ITA,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeWoodruff,
		kFeatures640,
		0,
		0
	},
	{
		{
			"woodruff",
			"",
			AD_ENTRY1s("intro.stk", "b50fee012a5abcd0ac2963e1b4b56bec", 20298108),
			ES_ESP,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeWoodruff,
		kFeatures640,
		0,
		0
	},
	{
		{
			"woodruff",
			"",
			AD_ENTRY1s("intro.stk", "5f5f4e0a72c33391e67a47674b120cc6", 20296422),
			DE_DEU,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeWoodruff,
		kFeatures640,
		0,
		0
	},
	{ // Supplied by jvprat on #scummvm
		{
			"woodruff",
			"",
			AD_ENTRY1s("intro.stk", "270529d9b8cce770b1575908a3800b52", 20296452),
			ES_ESP,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeWoodruff,
		kFeatures640,
		0,
		0
	},
	{ // Supplied by jvprat on #scummvm
		{
			"woodruff",
			"",
			AD_ENTRY1s("intro.stk", "270529d9b8cce770b1575908a3800b52", 20296452),
			EN_GRB,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeWoodruff,
		kFeatures640,
		0,
		0
	},
	{ // Supplied by jvprat on #scummvm
		{
			"woodruff",
			"",
			AD_ENTRY1s("intro.stk", "270529d9b8cce770b1575908a3800b52", 20296452),
			DE_DEU,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeWoodruff,
		kFeatures640,
		0,
		0
	},
	{ // Supplied by jvprat on #scummvm
		{
			"woodruff",
			"",
			AD_ENTRY1s("intro.stk", "270529d9b8cce770b1575908a3800b52", 20296452),
			FR_FRA,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeWoodruff,
		kFeatures640,
		0,
		0
	},
	{ // Supplied by jvprat on #scummvm
		{
			"woodruff",
			"",
			AD_ENTRY1s("intro.stk", "270529d9b8cce770b1575908a3800b52", 20296452),
			IT_ITA,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeWoodruff,
		kFeatures640,
		0,
		0
	},
	{ // Supplied by Hkz on #scummvm
		{
			"woodruff",
			"",
			AD_ENTRY1s("intro.stk", "f4c344023b073782d2fddd9d8b515318", 7069736),
			IT_ITA,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeWoodruff,
		kFeatures640,
		0,
		0
	},
	{ // Supplied by Hkz on #scummvm
		{
			"woodruff",
			"",
			AD_ENTRY1s("intro.stk", "f4c344023b073782d2fddd9d8b515318", 7069736),
			DE_DEU,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeWoodruff,
		kFeatures640,
		0,
		0
	},
	{ // Supplied by Hkz on #scummvm
		{
			"woodruff",
			"",
			AD_ENTRY1s("intro.stk", "f4c344023b073782d2fddd9d8b515318", 7069736),
			FR_FRA,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeWoodruff,
		kFeatures640,
		0,
		0
	},
	{ // Supplied by DjDiabolik in bug report #1971294
		{
			"woodruff",
			"",
			AD_ENTRY1s("intro.stk", "60348a87651f92e8492ee070556a96d8", 7069736),
			EN_GRB,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeWoodruff,
		kFeatures640,
		0,
		0
	},
	{ // Supplied by DjDiabolik in bug report #1971294
		{
			"woodruff",
			"",
			AD_ENTRY1s("intro.stk", "60348a87651f92e8492ee070556a96d8", 7069736),
			DE_DEU,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeWoodruff,
		kFeatures640,
		0,
		0
	},
	{ // Supplied by DjDiabolik in bug report #1971294
		{
			"woodruff",
			"",
			AD_ENTRY1s("intro.stk", "60348a87651f92e8492ee070556a96d8", 7069736),
			FR_FRA,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeWoodruff,
		kFeatures640,
		0,
		0
	},
	{ // Supplied by DjDiabolik in bug report #1971294
		{
			"woodruff",
			"",
			AD_ENTRY1s("intro.stk", "60348a87651f92e8492ee070556a96d8", 7069736),
			IT_ITA,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeWoodruff,
		kFeatures640,
		0,
		0
	},
	{ // Supplied by DjDiabolik in bug report #1971294
		{
			"woodruff",
			"",
			AD_ENTRY1s("intro.stk", "60348a87651f92e8492ee070556a96d8", 7069736),
			ES_ESP,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeWoodruff,
		kFeatures640,
		0,
		0
	},
	{ // Supplied by goodoldgeorg in bug report #2098838
		{
			"woodruff",
			"",
			AD_ENTRY1s("intro.stk", "08a96bf061af1fa4f75c6a7cc56b60a4", 20734979),
			PL_POL,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeWoodruff,
		kFeatures640,
		0,
		0
	},
	{
		{
			"woodruff",
			"Non-Interactive Demo",
			{
				{"demo.scn", 0, "16bb85fc5f8e519147b60475dbf33962", 89},
				{"wooddem3.vmd", 0, "a1700596172c2d4e264760030c3a3d47", 8994250},
				{NULL, 0, NULL, 0}
			},
			EN_ANY,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeWoodruff,
		kFeatures640 | kFeaturesSCNDemo,
		0,
		"demo.scn"
	},
	{
		{
			"dynasty",
			"",
			AD_ENTRY1s("intro.stk", "6190e32404b672f4bbbc39cf76f41fda", 2511470),
			EN_USA,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeDynasty,
		kFeatures640,
		0,
		0
	},
	{
		{
			"dynasty",
			"",
			AD_ENTRY1s("intro.stk", "61e4069c16e27775a6cc6d20f529fb36", 2511300),
			EN_USA,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeDynasty,
		kFeatures640,
		0,
		0
	},
	{
		{
			"dynasty",
			"",
			AD_ENTRY1s("intro.stk", "61e4069c16e27775a6cc6d20f529fb36", 2511300),
			FR_FRA,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeDynasty,
		kFeatures640,
		0,
		0
	},
	{
		{
			"dynasty",
			"Demo",
			AD_ENTRY1s("intro.stk", "464538a17ed39755d7f1ba9c751af1bd", 1847864),
			EN_USA,
			kPlatformPC,
			ADGF_DEMO
		},
		kGameTypeDynasty,
		kFeatures640,
		0,
		0
	},
	{
		{
			"dynasty",
			"Demo",
			AD_ENTRY1s("lda1.stk", "0e56a899357cbc0bf503260fd2dd634e", 15032774),
			UNK_LANG,
			kPlatformWindows,
			ADGF_DEMO
		},
		kGameTypeDynasty,
		kFeatures640,
		"lda1.stk",
		0
	},
	{
		{
			"urban",
			"",
			AD_ENTRY1s("intro.stk", "3ab2c542bd9216ae5d02cc6f45701ae1", 1252436),
			EN_USA,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeUrban,
		kFeatures640,
		0,
		0
	},
	{
		{
			"urban",
			"Non-Interactive Demo",
			{
				{"wdemo.s24", 0, "14ac9bd51db7a075d69ddb144904b271", 87},
				{"demo.vmd", 0, "65d04715d871c292518b56dd160b0161", 9091237},
				{"urband.vmd", 0, "60343891868c91854dd5c82766c70ecc", 922461},
				{NULL, 0, NULL, 0}
			},
			EN_ANY,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeUrban,
		kFeatures640 | kFeaturesSCNDemo,
		0,
		"wdemo.s24"
	},
	{
		{
			"spirou",
			"",
			AD_ENTRY1s("intro2.stk", "5e214cec5041d6a4a810feba8ddaaa92",247576),
			UNK_LANG,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeSpirou,
		kFeatures640,
		"intro2.stk",
		0
	},
	{
		{
			"archibald",
			"",
			AD_ENTRY1s("intro2.stk", "9aa412f5b8a1ee1761cb7b26e97fbd56",247094),
			UNK_LANG,
			kPlatformWindows,
			ADGF_NO_FLAGS
		},
		kGameTypeArchibald,
		kFeatures640,
		"intro2.stk",
		0
	},
	{
		{
			"archibald",
			"Non-Interactive Demo",
			{
				{"play123.scn", 0, "4689a31f543915e488c3bc46ea358add", 258},
				{"archi.vmd", 0, "a410fcc8116bc173f038100f5857191c", 5617210},
				{"chato.vmd", 0, "5a10e39cb66c396f2f9d8fb35e9ac016", 5445937},
				{"genedeb.vmd", 0, "3bb4a45585f88f4d839efdda6a1b582b", 1244228},
				{"generik.vmd", 0, "b46bdd64b063e86927fb2826500ad512", 603242},
				{"genespi.vmd", 0, "b7611916f32a370ae9832962fc17ef72", 758719},
				{"spirou.vmd", 0, "8513dbf7ac51c057b21d371d6b217b47", 2550788},
				{NULL, 0, NULL, 0}
			},
			EN_ANY,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeArchibald,
		kFeatures640 | kFeaturesSCNDemo,
		0,
		"play123.scn"
	},
	{ // Supplied by gamin in the forums
		{
			"urban",
			"",
			AD_ENTRY1s("intro.stk", "b991ed1d31c793e560edefdb349882ef", 1276408),
			FR_FRA,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeUrban,
		kFeatures640,
		0,
		0
	},
	{
		{
			"adibou4",
			"",
			AD_ENTRY1s("intro.stk", "a3c35d19b2d28ea261d96321d208cb5a", 6021466),
			FR_FRA,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeAdibou4,
		kFeatures640,
		0,
		0
	},
	{ AD_TABLE_END_MARKER, kGameTypeNone, kFeaturesNone, 0, 0 }
};

static const GOBGameDescription fallbackDescs[] = {
	{
		{
			"gob1",
			"unknown",
			AD_ENTRY1(0, 0),
			UNK_LANG,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeGob1,
		kFeaturesNone,
		0,
		0
	},
	{
		{
			"gob1cd",
			"unknown",
			AD_ENTRY1(0, 0),
			UNK_LANG,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeGob1,
		kFeaturesCD,
		0,
		0
	},
	{
		{
			"gob2",
			"unknown",
			AD_ENTRY1(0, 0),
			UNK_LANG,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeGob2,
		kFeaturesAdlib,
		0,
		0
	},
	{
		{
			"gob2mac",
			"unknown",
			AD_ENTRY1(0, 0),
			UNK_LANG,
			kPlatformMacintosh,
			ADGF_NO_FLAGS
		},
		kGameTypeGob2,
		kFeaturesAdlib,
		0,
		0
	},
	{
		{
			"gob2cd",
			"unknown",
			AD_ENTRY1(0, 0),
			UNK_LANG,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeGob2,
		kFeaturesCD,
		0,
		0
	},
	{
		{
			"bargon",
			"",
			AD_ENTRY1(0, 0),
			UNK_LANG,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeBargon,
		kFeaturesNone,
		0,
		0
	},
	{
		{
			"gob3",
			"unknown",
			AD_ENTRY1(0, 0),
			UNK_LANG,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeGob3,
		kFeaturesAdlib,
		0,
		0
	},
	{
		{
			"gob3cd",
			"unknown",
			AD_ENTRY1(0, 0),
			UNK_LANG,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeGob3,
		kFeaturesCD,
		0,
		0
	},
	{
		{
			"woodruff",
			"unknown",
			AD_ENTRY1(0, 0),
			UNK_LANG,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeWoodruff,
		kFeatures640,
		0,
		0
	},
	{
		{
			"lostintime",
			"unknown",
			AD_ENTRY1(0, 0),
			UNK_LANG,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeLostInTime,
		kFeaturesAdlib,
		0,
		0
	},
	{
		{
			"lostintime",
			"unknown",
			AD_ENTRY1(0, 0),
			UNK_LANG,
			kPlatformMacintosh,
			ADGF_NO_FLAGS
		},
		kGameTypeLostInTime,
		kFeaturesAdlib,
		0,
		0
	},
	{
		{
			"lostintime",
			"unknown",
			AD_ENTRY1(0, 0),
			UNK_LANG,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeLostInTime,
		kFeaturesCD,
		0,
		0
	},
	{
		{
			"urban",
			"unknown",
			AD_ENTRY1(0, 0),
			UNK_LANG,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeUrban,
		kFeaturesCD,
		0,
		0
	},
	{
		{
			"fascination",
			"unknown",
			AD_ENTRY1(0, 0),
			UNK_LANG,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeFascination,
		kFeaturesNone,
		"disk0.stk",
		0
	},
	{
		{
			"geisha",
			"unknown",
			AD_ENTRY1(0, 0),
			UNK_LANG,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeGeisha,
		kFeaturesNone,
		"disk1.stk",
		"intro.tot"
	},
	{
		{
			"adibou4",
			"",
			AD_ENTRY1(0, 0),
			UNK_LANG,
			kPlatformPC,
			ADGF_NO_FLAGS
		},
		kGameTypeAdibou4,
		kFeatures640,
		"adif41.stk",
		0
	},
};

static const ADFileBasedFallback fileBased[] = {
	{ &fallbackDescs[ 0], { "intro.stk", "disk1.stk", "disk2.stk", "disk3.stk", "disk4.stk", 0 } },
	{ &fallbackDescs[ 1], { "intro.stk", "gob.lic", 0 } },
	{ &fallbackDescs[ 2], { "intro.stk", 0 } },
	{ &fallbackDescs[ 2], { "intro.stk", "disk2.stk", "disk3.stk", 0 } },
	{ &fallbackDescs[ 3], { "intro.stk", "disk2.stk", "disk3.stk", "musmac1.mid", 0 } },
	{ &fallbackDescs[ 4], { "intro.stk", "gobnew.lic", 0 } },
	{ &fallbackDescs[ 5], { "intro.stk", "scaa.imd", "scba.imd", "scbf.imd", 0 } },
	{ &fallbackDescs[ 6], { "intro.stk", "imd.itk", 0 } },
	{ &fallbackDescs[ 7], { "intro.stk", "mus_gob3.lic", 0 } },
	{ &fallbackDescs[ 8], { "intro.stk", "woodruff.itk", 0 } },
	{ &fallbackDescs[ 9], { "intro.stk", "commun1.itk", 0 } },
	{ &fallbackDescs[10], { "intro.stk", "commun1.itk", "musmac1.mid", 0 } },
	{ &fallbackDescs[11], { "intro.stk", "commun1.itk", "lost.lic", 0 } },
	{ &fallbackDescs[12], { "intro.stk", "cd1.itk", "objet1.itk", 0 } },
	{ &fallbackDescs[13], { "disk0.stk", "disk1.stk", "disk2.stk", "disk3.stk", 0 } },
	{ &fallbackDescs[14], { "disk1.stk", "disk2.stk", "disk3.stk", 0 } },
	{ &fallbackDescs[15], { "adif41.stk", "adim41.stk", 0 } },
	{ 0, { 0 } }
};

}

static const ADParams detectionParams = {
	// Pointer to ADGameDescription or its superset structure
	(const byte *)Gob::gameDescriptions,
	// Size of that superset structure
	sizeof(Gob::GOBGameDescription),
	// Number of bytes to compute MD5 sum for
	5000,
	// List of all engine targets
	gobGames,
	// Structure for autoupgrading obsolete targets
	obsoleteGameIDsTable,
	// Name of single gameid (optional)
	"gob",
	// List of files for file-based fallback detection (optional)
	Gob::fileBased,
	// Flags
	0
};

class GobMetaEngine : public AdvancedMetaEngine {
public:
	GobMetaEngine() : AdvancedMetaEngine(detectionParams) {}

	virtual const char *getName() const {
		return "Gob Engine";
	}

	virtual const char *getOriginalCopyright() const {
		return "Goblins Games (C) Coktel Vision";
	}

	virtual bool hasFeature(MetaEngineFeature f) const;
	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;
};

bool GobMetaEngine::hasFeature(MetaEngineFeature f) const {
	return false;
}

bool Gob::GobEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsRTL);
}
bool GobMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	const Gob::GOBGameDescription *gd = (const Gob::GOBGameDescription *)desc;
	if (gd) {
		*engine = new Gob::GobEngine(syst);
		((Gob::GobEngine *)*engine)->initGame(gd);
	}
	return gd != 0;
}

#if PLUGIN_ENABLED_DYNAMIC(GOB)
	REGISTER_PLUGIN_DYNAMIC(GOB, PLUGIN_TYPE_ENGINE, GobMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(GOB, PLUGIN_TYPE_ENGINE, GobMetaEngine);
#endif

namespace Gob {

void GobEngine::initGame(const GOBGameDescription *gd) {
	if (gd->startTotBase == 0) {
		_startTot = new char[10];
		strcpy(_startTot, "intro.tot");
	} else {
		_startTot = new char[strlen(gd->startTotBase) + 1];
		strcpy(_startTot, gd->startTotBase);
	}
	if (gd->startStkBase == 0) {
		_startStk = new char[10];
		strcpy(_startStk, "intro.stk");
	} else {
		_startStk = new char[strlen(gd->startStkBase) + 1];
		strcpy(_startStk, gd->startStkBase);
	}
	_gameType = gd->gameType;
	_features = gd->features;
	_language = gd->desc.language;
	_platform = gd->desc.platform;
}
} // End of namespace Gob
