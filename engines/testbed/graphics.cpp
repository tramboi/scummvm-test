#include "common/events.h"
#include "common/random.h"

#include "testbed/graphics.h"
#include "testbed/testsuite.h"

#include "graphics/cursorman.h"
#include "graphics/fontman.h"
#include "graphics/surface.h"
#include "graphics/VectorRendererSpec.h"

namespace Testbed {

byte GFXTestSuite::_palette[3 * 4] = {0, 0, 0, 0, 255, 255, 255, 0, 255, 255, 255, 0};

GFXTestSuite::GFXTestSuite() {
	// Initialize color palettes
	// The fourth field is for alpha channel which is unused
	// Assuming 8bpp as of now
	g_system->setPalette(_palette, 0, 3);
	
	// Init Mouse Palette (White-black-yellow)
	GFXtests::initMousePalette();
	
	// Add tests here
	
	// Blitting buffer on screen
	// addTest("BlitBitmaps", &GFXtests::copyRectToScreen);
	
	// GFX Transcations
	// addTest("FullScreenMode", &GFXtests::fullScreenMode);
	// addTest("AspectRatio", &GFXtests::aspectRatio);
	// addTest("IconifyingWindow", &GFXtests::iconifyWindow);
	
	// Mouse Layer tests (Palettes and movements)
	// addTest("PalettizedCursors", &GFXtests::palettizedCursors);
	// FIXME: need to fix it
	// addTest("ScaledCursors", &GFXtests::scaledCursors);
	
	// Effects
	// addTest("shakingEffect", &GFXtests::shakingEffect);
	// addTest("focusRectangle", &GFXtests::focusRectangle);

	// Overlay
	// addTest("Overlays", &GFXtests::overlayGraphics);

	// Specific Tests:
	addTest("Palette Rotation", &GFXtests::paletteRotation);
	addTest("Pixel Formats", &GFXtests::pixelFormats);
		
}

const char *GFXTestSuite::getName() const {
	return "GFX";
}

void GFXTestSuite::setCustomColor(uint r, uint g, uint b) {
	_palette[8] = r; 
	_palette[9] = g;
	_palette[10] = b;
	g_system->setPalette(_palette, 0, 3);
}

// Helper functions used by GFX tests

void GFXtests::initMousePalette() {
	byte palette[3 * 4]; // Black, white and yellow
	
	palette[0] = palette[1] = palette[2] = 0;
	palette[4] = palette[5] = palette[6] = 255;
	palette[8] = palette[9] = 255;
	palette[10] = 0;
		
	CursorMan.replaceCursorPalette(palette, 0, 3);
	
}

void GFXtests::drawCursor(bool cursorPaletteDisabled, const char *gfxModeName, int cursorTargetScale) {	

	// Buffer initialized with yellow color
	byte buffer[500];
	memset(buffer, 2, sizeof(buffer));
	
	/* Disable HotSpot highlighting as of now
	
	// Paint the cursor with yellow, except the hotspot
	for (int i = 0; i < 16; i++) {
		for (int j = 0; j < 16; j++) {
			if (i != j && i != 15 - j) {
				buffer[i * 16 + j] = 2;
			}
		}
	}

	*/

	// Uncommenting the next line and commenting the line after that would reproduce the crash
	// CursorMan.replaceCursor(buffer, 11, 11, 0, 0, 255, cursorTargetScale);
	CursorMan.replaceCursor(buffer, 12, 13, 0, 0, 255, cursorTargetScale);
	CursorMan.showMouse(true);
	
	if (cursorPaletteDisabled) {
		CursorMan.disableCursorPalette(true);
	} else {
		initMousePalette();
		CursorMan.disableCursorPalette(false);
	}

	g_system->updateScreen();
}

void rotatePalette(byte palette[], int size) {
	// Ignore rotating black color, as that is background
	// take a temporary palette color
	byte tColor[4] = {0};
	// save the first color in tColor
	memcpy(tColor, &palette[4], 4 * sizeof(byte));

	// Move each color upward by 1
	for (int i = 1; i < size - 1; i++) {
		memcpy(&palette[i * 4], &palette[(i + 1) * 4], 4 * sizeof(byte));
	}
	// Assign last color to tcolor
	memcpy(&palette[(size -1) * 4], tColor, 4 * sizeof(byte));
	g_system->setPalette(palette, 0, 10);
}

/**
 * Sets up mouse loop, exits when user clicks any of the mouse button
 */
void GFXtests::setupMouseLoop(bool disableCursorPalette, const char *gfxModeName, int cursorTargetScale) {

	bool isFeaturePresent;
	isFeaturePresent = g_system->hasFeature(OSystem::kFeatureCursorHasPalette);

	if (isFeaturePresent) {
		
		GFXtests::drawCursor(disableCursorPalette, gfxModeName, cursorTargetScale);

		Common::EventManager *eventMan = g_system->getEventManager();
		Common::Event event;
		Common::Point pt(0, 100);

		bool quitLoop = false;
		uint32 lastRedraw = 0;
		const uint32 waitTime = 1000 / 45;	
		
		Testsuite::clearScreen();
		Common::String info = disableCursorPalette ? "Using Game Palette" : "Using cursor palette";
		info += " to render the cursor, Click to finish";
		
		Testsuite::writeOnScreen(info, pt);
		
		info = "GFX Mode";
		info += gfxModeName;
		info += " ";

		char cScale = cursorTargetScale + '0';
		info += "Cursor scale: ";
		info += cScale;
		
		if (!Common::String(gfxModeName).equals("")) {
			Testsuite::writeOnScreen(info, Common::Point(0, 120));
		}

		while (!quitLoop) {
			while (eventMan->pollEvent(event)) {
	
				if (lastRedraw + waitTime < g_system->getMillis()) {
					g_system->updateScreen();
					lastRedraw = g_system->getMillis();
				}

				switch (event.type) {
				case Common::EVENT_MOUSEMOVE:
					break;
				case Common::EVENT_LBUTTONDOWN:
				case Common::EVENT_RBUTTONDOWN:
					quitLoop = true;
					Testsuite::clearScreen();
					Testsuite::writeOnScreen("Mouse clicked", pt);
					g_system->delayMillis(1000);
					break;
				default:	
					;// Ignore any other event

				}
			}
		}
	} else {
		Testsuite::displayMessage("feature not supported");
	}
}

void GFXtests::mouseMovements() {
	// Testing Mouse Movements now!
	Common::Point pt(0, 100);
	Testsuite::writeOnScreen("Moving mouse automatically from (0, 0) to (100, 100)", pt);
	g_system->warpMouse(0, 0);
	g_system->updateScreen();
	g_system->delayMillis(1000);

	for (int i = 0; i <= 100; i++) {
		g_system->delayMillis(20);
		g_system->warpMouse(i, i);
		g_system->updateScreen();
	}
	
	Testsuite::clearScreen();
	Testsuite::writeOnScreen("Mouse Moved to (100, 100)", pt);
	g_system->delayMillis(1000);	
}

/**
 * Used by aspectRatio()
 */

void GFXtests::drawEllipse(int cx, int cy, int a, int b) {	
	
	// Take a buffer of screen size

	byte buffer[200][320] = {{0}};
	float theta;
	int x, y, x1, y1;

	// Illuminate the center
	buffer[cx][cy] = 1;
	
	// Illuminate the points lying on ellipse

	for (theta = 0; theta <= PI / 2; theta += PI / 360  ) {
		x = (int)(b * sin(theta) + 0.5);
		y = (int)(a * cos(theta) + 0.5);
		
		// This gives us four points
		
		x1 = x + cx;
		y1 = y + cy;
		
		buffer[x1][y1] = 1;

		x1 = (-1) * x + cx;
		y1 = y + cy;
		
		buffer[x1][y1] = 1;
		
		x1 = x + cx;
		y1 = (-1) * y + cy;
		
		buffer[x1][y1] = 1;

		x1 = (-1) * x + cx;
		y1 = (-1) * y + cy;
		
		buffer[x1][y1] = 1;
	}

	g_system->copyRectToScreen(&buffer[0][0], 320, 0, 0, 320, 200);
	g_system->updateScreen();
}

// GFXtests go here

/**
 * Tests the fullscreen mode by: toggling between fullscreen and windowed mode
 */

bool GFXtests::fullScreenMode() {
	
	Common::Point pt(0, 100);
	Common::Rect rect = Testsuite::writeOnScreen("Testing fullscreen mode", pt);
	
	bool isFeaturePresent;
	bool isFeatureEnabled;
	bool passed = true;
	Common::String prompt;
	OptionSelected shouldSelect;

	isFeaturePresent = g_system->hasFeature(OSystem::kFeatureFullscreenMode);

	if (isFeaturePresent) {
		// Toggle
		isFeatureEnabled = g_system->getFeatureState(OSystem::kFeatureFullscreenMode);
		shouldSelect = isFeatureEnabled ? kOptionLeft : kOptionRight;
		
		g_system->delayMillis(1000);
		
		if (isFeatureEnabled) {
			printf("LOG: Current Mode is Fullsecreen\n");
		} else {
			printf("LOG: Current Mode is Windowed\n");
		}

		prompt = " Which mode do you see currently ?  ";
		
		if (!Testsuite::handleInteractiveInput(prompt, "Fullscreen", "Windowed", shouldSelect)) {
			// User selected incorrect current state
			passed = false;
			printf("LOG: g_system->getFeatureState() failed\n");
		}

		g_system->beginGFXTransaction();
		g_system->setFeatureState(OSystem::kFeatureFullscreenMode, !isFeatureEnabled);
		g_system->endGFXTransaction();

		// Current state should be now !isFeatureEnabled
		isFeatureEnabled = g_system->getFeatureState(OSystem::kFeatureFullscreenMode);
		shouldSelect = isFeatureEnabled ? kOptionLeft : kOptionRight;
			
		g_system->delayMillis(1000);
		
		prompt = "  Which screen mode do you see now ?   ";
		
		if (!Testsuite::handleInteractiveInput(prompt, "Fullscreen", "Windowed", shouldSelect)) {
			// User selected incorrect mode
			passed = false;
			printf("LOG: g_system->setFeatureState() failed\n");
		}
		
		g_system->beginGFXTransaction();
		g_system->setFeatureState(OSystem::kFeatureFullscreenMode, !isFeatureEnabled);
		g_system->endGFXTransaction();
		
		g_system->delayMillis(1000);
		
		prompt = "This should be your initial state. Is it?";
		
		if (!Testsuite::handleInteractiveInput(prompt, "Yes, it is", "Nopes", shouldSelect)) {
			// User selected incorrect mode
			printf("LOG: switching back to initial state failed\n");
			passed = false;
		}

	} else {
		Testsuite::displayMessage("feature not supported");
	}

	Testsuite::clearScreen();
	return passed;
}

/**
 * Tests the aspect ratio correction by: drawing an ellipse, when corrected the ellipse should render to a circle
 */

bool GFXtests::aspectRatio() {
	// Draw an ellipse on the screen
	
	drawEllipse(100, 160, 72, 60);
	
	Common::Point pt(0, 180);
	Testsuite::writeOnScreen("Testing Aspect Ratio Correction!", pt);
	
	bool isFeaturePresent;
	bool isFeatureEnabled;
	bool passed;
	Common::String prompt;
	OptionSelected shouldSelect;

	isFeaturePresent = g_system->hasFeature(OSystem::kFeatureAspectRatioCorrection);
	isFeatureEnabled = g_system->getFeatureState(OSystem::kFeatureAspectRatioCorrection);
	g_system->delayMillis(1000);

	if (isFeaturePresent) {
		// Toggle
		shouldSelect = isFeatureEnabled ? kOptionLeft : kOptionRight;
		prompt = " What does the curve on screen appears to you ?";
		if (!Testsuite::handleInteractiveInput(prompt, "Circle", "Ellipse", shouldSelect)) {
			// User selected incorrect option
			passed = false;
			printf("LOG: Aspect Ratio Correction failed\n");
		}
		
		g_system->beginGFXTransaction();
		g_system->setFeatureState(OSystem::kFeatureAspectRatioCorrection, !isFeatureEnabled);
		g_system->endGFXTransaction();
		
		g_system->delayMillis(1000);
		
		shouldSelect = !isFeatureEnabled ? kOptionLeft : kOptionRight;
		prompt = " What does the curve on screen appears to you ?";
		if (!Testsuite::handleInteractiveInput(prompt, "Circle", "Ellipse", shouldSelect)) {
			// User selected incorrect option
			passed = false;
			printf("LOG: Aspect Ratio Correction failed\n");
		}
		
		g_system->beginGFXTransaction();
		g_system->setFeatureState(OSystem::kFeatureAspectRatioCorrection, isFeatureEnabled);
		g_system->endGFXTransaction();
	} else {
		Testsuite::displayMessage("feature not supported");
	}

	g_system->delayMillis(500);
	Testsuite::clearScreen();
	return passed;
}

/**
 * Tests Palettized cursors.
 * Method: Create a yellow colored cursor, should be able to move it. Once you click test terminates
 */

bool GFXtests::palettizedCursors() {

	
	bool passed = true;
	
	Testsuite::displayMessage("Testing Cursors. You should expect to see a yellow colored square cursor.\n"
	"You should be able to move it. The test finishes when the mouse(L/R) is clicked");
	
	// Testing with cursor Palette
	setupMouseLoop();
	// Test Automated Mouse movements (warp)
	mouseMovements();
	
	if (Testsuite::handleInteractiveInput("Which color did the cursor appeared to you?", "Yellow", "Any other", kOptionRight)) {
		printf("LOG: Couldn't use cursor palette for rendering cursor\n");
		passed = false;
	}	

	// Testing with game Palette
	GFXTestSuite::setCustomColor(255, 0, 0);
	setupMouseLoop(true);
	
	if (Testsuite::handleInteractiveInput("Which color did the cursor appeared to you?", "Red", "Any other", kOptionRight)) {
		printf("LOG: Couldn't use Game palette for rendering cursor\n");
		passed = false;
	}	
	g_system->delayMillis(1000);

	if (!Testsuite::handleInteractiveInput("Did Cursor tests went as you were expecting?")) {
		passed = false;
	}
	
	return passed;
}


/**
 * This basically blits the screen by the contents of its buffer.
 *
 */
bool GFXtests::copyRectToScreen() {
	Testsuite::displayMessage("Testing Blitting a Bitmap to screen.\n"
	"You should expect to see a 20x40 yellow horizontal rectangle centred at the screen.");

	GFXTestSuite::setCustomColor(255, 255, 0);
	byte buffer[20 * 40];
	memset(buffer, 2, 20 * 40);

	uint x = g_system->getWidth() / 2 - 20;
	uint y = g_system->getHeight() / 2 - 10;

	g_system->copyRectToScreen(buffer, 40, x, y, 40, 20);
	g_system->updateScreen();
	g_system->delayMillis(1000);

	Common::Rect rect(x, y, x+40, y+20);
	Testsuite::clearScreen();
	
	if (Testsuite::handleInteractiveInput("Did the test worked as you were expecting?", "Yes", "No", kOptionRight)) {
		return false;
	}

	return true;

}

/**
 * Testing feature : Iconifying window
 * It is expected the screen minimizes when this feature is enabled 
 */
bool GFXtests::iconifyWindow() {
	
	Testsuite::displayMessage("Testing Iconify Window mode.\n If the feature is supported by the backend,"
	"you should expect the window to be minimized. However you would manually need to de-iconify.");

	Common::Point pt(0, 100);
	Common::Rect rect = Testsuite::writeOnScreen("Testing Iconifying window", pt);
	
	bool isFeaturePresent;
	bool isFeatureEnabled;

	isFeaturePresent = g_system->hasFeature(OSystem::kFeatureIconifyWindow);
	isFeatureEnabled = g_system->getFeatureState(OSystem::kFeatureIconifyWindow);
	g_system->delayMillis(1000);

	if (isFeaturePresent) {
		// Toggle

		g_system->beginGFXTransaction();
		g_system->setFeatureState(OSystem::kFeatureIconifyWindow, !isFeatureEnabled);
		g_system->endGFXTransaction();

		g_system->delayMillis(1000);
		
		g_system->beginGFXTransaction();
		g_system->setFeatureState(OSystem::kFeatureIconifyWindow, isFeatureEnabled);
		g_system->endGFXTransaction();
	} else {
		Testsuite::displayMessage("feature not supported");
	}
	
	if (Testsuite::handleInteractiveInput("Did the test worked as you were expecting?", "Yes", "No", kOptionRight)) {
		return false;
	}

	Testsuite::clearScreen();
	return true;
}

/**
 * Testing feature: Scaled cursors
 */

bool GFXtests::scaledCursors() {

	Testsuite::displayMessage("Testing : Scaled cursors\n"
	"Here every graphics mode is tried with a cursorTargetScale of 1,2 and 3"
	"This may take time, You may skip the later scalers and just examine the first three i.e 1x,2x and 3x");

	int maxLimit = 1000;
	if (!Testsuite::handleInteractiveInput("Do you want to restrict scalers to 1x, 2x and 3x only?", "Yes", "No", kOptionRight)) {
		maxLimit = 3;
	}
	
	const int currGFXMode = g_system->getGraphicsMode();
	const OSystem::GraphicsMode *gfxMode = g_system->getSupportedGraphicsModes();
	
	while (gfxMode->name && maxLimit > 0) {
		// for every graphics mode display cursors for cursorTargetScale 1, 2 and 3
		// Switch Graphics mode
		// FIXME: Crashes with "3x" mode now.:
		g_system->beginGFXTransaction();

		bool isGFXModeSet = g_system->setGraphicsMode(gfxMode->id);
		g_system->initSize(320, 200);
		
		OSystem::TransactionError gfxError = g_system->endGFXTransaction();

		if (gfxError == OSystem::kTransactionSuccess && isGFXModeSet) {
			
			setupMouseLoop(false, gfxMode->name, 1);
			Testsuite::clearScreen();
			
			setupMouseLoop(false, gfxMode->name, 2);
			Testsuite::clearScreen();
			
			setupMouseLoop(false, gfxMode->name, 3);
			Testsuite::clearScreen();

		} else {
			printf("LOG: Switching to graphics mode %s failed\n", gfxMode->name);
			return false;
		}
		gfxMode++;
		maxLimit--;
	}

	// Restore Original State 
	g_system->beginGFXTransaction();
	bool isGFXModeSet = g_system->setGraphicsMode(currGFXMode);
	g_system->initSize(320, 200);
	OSystem::TransactionError gfxError = g_system->endGFXTransaction();

	if (gfxError != OSystem::kTransactionSuccess || !isGFXModeSet) {
		printf("LOG: Switcing to initial state failed\n");
		return false;
	}

	return true;
}

bool GFXtests::shakingEffect() {
	Common::Point pt(0, 100);
	Testsuite::writeOnScreen("If Shaking effect works,this should shake!", pt);
	int times = 35;
	while (times--) {
		g_system->setShakePos(10);
		g_system->updateScreen();
		g_system->setShakePos(0);
		g_system->updateScreen();
	}
	g_system->delayMillis(1500);

	if (Testsuite::handleInteractiveInput("Did the test worked as you were expecting?", "Yes", "No", kOptionRight)) {
		printf("LOG: Shaking Effect didn't worked");
		return false;
	}
	Testsuite::clearScreen();
	return true;
}

bool GFXtests::focusRectangle() {

	Testsuite::displayMessage("Testing : Setting and hiding Focus \n"
	"If this feature is implemented, the focus should be toggled between the two rectangles on the corners");
	Testsuite::clearScreen();

	const Graphics::Font &font(*FontMan.getFontByUsage(Graphics::FontManager::kConsoleFont));

	Graphics::Surface *screen = g_system->lockScreen();
	int screenHeight = g_system->getHeight();
	int screenWidth = g_system->getWidth();

	int height = font.getFontHeight();
	int width = screenWidth / 2;

	Common::Rect rectLeft(0, 0, width, height * 2);
	screen->fillRect(rectLeft, kColorWhite);
	font.drawString(screen, "Focus 1", rectLeft.left, rectLeft.top, width, kColorBlack, Graphics::kTextAlignLeft);

	Common::Rect rectRight(screenWidth - width, screenHeight - height * 2 , screenWidth, screenHeight);
	screen->fillRect(rectRight, kColorWhite);
	font.drawString(screen, "Focus 2", rectRight.left, rectRight.top, width, kColorBlack, Graphics::kTextAlignRight);
	g_system->unlockScreen();
	g_system->updateScreen();

	g_system->clearFocusRectangle();

	g_system->setFocusRectangle(rectLeft);
	g_system->updateScreen();
	
	g_system->delayMillis(1000);

	g_system->setFocusRectangle(rectRight);
	g_system->updateScreen();
	
	g_system->clearFocusRectangle();

	if (Testsuite::handleInteractiveInput("Did you noticed a variation in focus?", "Yes", "No", kOptionRight)) {
		printf("LOG: Focus Rectangle feature doesn't works. Check platform.\n");
	}

	return true;
}

bool GFXtests::overlayGraphics() {
	
	Graphics::PixelFormat pf = g_system->getOverlayFormat();
	
	OverlayColor buffer[20 * 40];
	OverlayColor value = pf.RGBToColor(0, 255, 0);

	for (int i = 0; i < 20 * 40; i++) {
		buffer[i] = value;
	}
	
	// FIXME: Not Working.
	g_system->showOverlay();
	g_system->copyRectToOverlay(buffer, 40, 100, 100, 40, 20);
	g_system->updateScreen();
	
	g_system->delayMillis(1000);
	
	g_system->hideOverlay();
	g_system->updateScreen();

	if (Testsuite::handleInteractiveInput("Did you see a green overlayed rectangle?", "Yes", "No", kOptionRight)) {
		printf("LOG: Overlay Rectangle feature doesn't works\n");
		return false;
	}
	return true;
}

bool GFXtests::paletteRotation() {
	byte palette[10 * 4] = {0, 0, 0, 0,
							255, 255, 255, 0,
							135, 48, 21, 0,
							205, 190, 87, 0,
							0, 32, 64, 0,
							181, 126, 145, 0,
							47, 78, 36, 0,
							185, 115, 20, 0,
							160, 164, 137, 0,
							43, 52, 0, 0}; // 10 colors : B, W and 8 random 

	Common::RandomSource rs;
	
	// Initialize this palette randomly
	g_system->setPalette(palette, 0, 10);

	// Draw 10 Rectangles, each of width 100 pixels and height 10 pixels
	byte buffer[10 * 100 * 10] = {0};
	
	for (int i = 0; i < 10; i++) {
		memset(buffer + i * 1000, i, 1000 * sizeof(byte));
	}
	
	g_system->copyRectToScreen(buffer, 100, 110, 50, 100, 100);

	int toRotate = 10;

	while (toRotate--) {
		g_system->updateScreen();
		g_system->delayMillis(1000);
		rotatePalette(palette, 10);
	}

	if(Testsuite::handleInteractiveInput("Did you want to rotate the palettes?", "Yes", "No", kOptionRight)) {
		return false;
	}

	return true;
}

bool GFXtests::pixelFormats() {

}

}
