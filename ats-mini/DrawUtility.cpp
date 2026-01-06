#include "Common.h"
#include "Utils.h"
#include "Draw.h"
#include "Menu.h"
#include "Themes.h"

// Variable to track current utility index (shared with Menu logic usually, but Draw needs it)
// We will rely on getUtilData(utilIdx) where utilIdx is managed in Menu.cpp but we need access.
// Let's add an extern or getter/setter in Menu.h for it? Or simpler: make it global in Utils.h?
// No, Menu.cpp handles the state.
// We can use a getter from Menu.h if we implement one, OR just put the drawing logic here using an extern.

extern int utilIdx; // We will define this in Menu.cpp

void drawUtility() {
    // Clear screen
    spr.fillSprite(TH.bg);
    
    const UtilFreq* u = getUtilData(utilIdx);
    
    // Draw Category Header
    spr.setTextDatum(TC_DATUM);
    spr.setTextColor(TH.menu_hdr);
    spr.fillSmoothRoundRect(10, 10, 300, 150, 4, TH.menu_border);
    spr.fillSmoothRoundRect(12, 12, 296, 146, 4, TH.menu_bg);
    
    // Category Name
    spr.drawString(u->cat, 160, 20, 4);
    spr.drawLine(10, 45, 310, 45, TH.menu_border);
    
    // Station Name
    spr.setTextColor(TH.menu_item);
    spr.setFreeFont(&Orbitron_Light_24); // Big font
    spr.drawString(u->name, 160, 65);
    spr.setTextFont(0); // Reset font
    
    // Frequency and Mode
    char freqStr[32];
    const char* modeStr = (u->mode == AM) ? "AM" : (u->mode == USB ? "USB" : "LSB");
    
    if (u->freq < 30000000) {
        sprintf(freqStr, "%lu kHz  %s", u->freq / 1000, modeStr);
    } else {
        sprintf(freqStr, "%.2f MHz  %s", u->freq / 1000000.0, modeStr);
    }
    
    spr.setTextColor(TH.band_text);
    spr.drawString(freqStr, 160, 105, 4);
    
    // Navigation Hint
    spr.setTextColor(TH.text_muted);
    spr.drawString("<  Turn to Browse  >", 160, 135, 2);
    
    spr.pushSprite(0, 0);
}
