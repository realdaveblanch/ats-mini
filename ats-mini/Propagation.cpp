#include "Common.h"
#include "Utils.h"
#include "Draw.h"
#include "Themes.h"
#include "Menu.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

// Data Cache
struct PropData {
    bool valid;
    float sfi;
    float kp;
    char rating10m[16];
    char rating20m[16];
    char rating40m[16];
    char rating80m[16];
};

static PropData propData = { false, 0, 0, "--", "--", "--", "--" };

// Helper to parse JSON value manually (lightweight)
// Returns true if found
bool extractJsonString(String& json, const char* key, char* buffer, int bufLen) {
    int keyPos = json.indexOf(key);
    if (keyPos == -1) return false;
    
    // Find colon
    int colonPos = json.indexOf(':', keyPos);
    if (colonPos == -1) return false;
    
    // Find start quote
    int quoteStart = json.indexOf('"', colonPos);
    if (quoteStart == -1) return false;
    
    // Find end quote
    int quoteEnd = json.indexOf('"', quoteStart + 1);
    if (quoteEnd == -1) return false;
    
    String val = json.substring(quoteStart + 1, quoteEnd);
    strncpy(buffer, val.c_str(), bufLen);
    buffer[bufLen-1] = 0; // Ensure null term
    return true;
}

// Helper for float values (e.g. "sfi": 155.0)
float extractJsonFloat(String& json, const char* key) {
    int keyPos = json.indexOf(key);
    if (keyPos == -1) return 0.0;
    
    int colonPos = json.indexOf(':', keyPos);
    if (colonPos == -1) return 0.0;
    
    // Find start of number (skip spaces)
    int valStart = colonPos + 1;
    while (valStart < json.length() && (json[valStart] == ' ' || json[valStart] == '\t')) valStart++;
    
    // Find end of number (comma, brace, or space)
    int valEnd = valStart;
    while (valEnd < json.length() && (isdigit(json[valEnd]) || json[valEnd] == '.' || json[valEnd] == '-')) valEnd++;
    
    String val = json.substring(valStart, valEnd);
    return val.toFloat();
}

// Called explicitly from Network.cpp after WiFi connect
void updatePropagationData() {
    if (getWiFiStatus() != 2) return; // Need Internet
    if (propData.valid) return; // Already have data, one-shot
    
    WiFiClientSecure *client = new WiFiClientSecure;
    if(client) {
        client->setInsecure(); // Skip certificate check
        HTTPClient https;
        
        // Use wspr.hb9vqq.ch API with shorter timeout
        https.setTimeout(3000); 
        
        if (https.begin(*client, "https://wspr.hb9vqq.ch/api/dx.json")) {
            int httpCode = https.GET();
            if (httpCode == HTTP_CODE_OK) {
                String payload = https.getString();
                
                propData.sfi = extractJsonFloat(payload, "\"sfi\"");
                propData.kp  = extractJsonFloat(payload, "\"kp\"");
                
                auto getRating = [&](const char* band) {
                    int bandPos = payload.indexOf(band);
                    if (bandPos == -1) return String("--");
                    int ratePos = payload.indexOf("\"rating\"", bandPos);
                    if (ratePos == -1) return String("--");
                    int q1 = payload.indexOf('"', ratePos + 8);
                    int q2 = payload.indexOf('"', q1 + 1);
                    if (q1 != -1 && q2 != -1) return payload.substring(q1+1, q2);
                    return String("--");
                };
                
                strncpy(propData.rating10m, getRating("\"10m\"").c_str(), 15);
                strncpy(propData.rating20m, getRating("\"20m\"").c_str(), 15);
                strncpy(propData.rating40m, getRating("\"40m\"").c_str(), 15);
                strncpy(propData.rating80m, getRating("\"80m\"").c_str(), 15);
                
                propData.valid = true;
            }
            https.end();
        }
        delete client;
    }
}

// Removed periodic tick, now called manually

void drawPropagation() {
    // Background
    spr.fillSprite(TH.bg);
    
    // Draw Header
    spr.setTextDatum(TC_DATUM);
    spr.setTextColor(TH.menu_hdr);
    spr.fillSmoothRoundRect(10, 10, 300, 150, 4, TH.menu_border);
    spr.fillSmoothRoundRect(12, 12, 296, 146, 4, TH.menu_bg);
    
    spr.drawString(propData.valid ? "Solar & DX (Live)" : "Propagation Forecast", 160, 20, 4);
    spr.drawLine(10, 45, 310, 45, TH.menu_border);
    
    // Get Time for Local display
    uint8_t h, m;
    int t = 0;
    int localHour = 12; // Default noon
    
    if (clockGetHM(&h, &m)) {
        t = (int)h * 60 + m + getCurrentUTCOffset() * 15;
        t = t < 0 ? t + 24*60 : t;
        t = t % (24*60);
        localHour = t / 60;
    }

    if (!propData.valid) {
        // Fallback: Show Estimated Data based on Time
        const char* phase = "Unknown";
        const char* bestBands = "";
        uint16_t iconColor = TH.text;
        
        if (localHour >= 6 && localHour < 9) {
            phase = "Morning / Greyline";
            bestBands = "20m, 30m, 40m";
            iconColor = 0xFFE0; // Yellow
        } else if (localHour >= 9 && localHour < 16) {
            phase = "Daytime";
            bestBands = "10m - 20m";
            iconColor = 0xF800; // Red
        } else if (localHour >= 16 && localHour < 19) {
            phase = "Evening / Greyline";
            bestBands = "20m - 60m";
            iconColor = 0xFDA0; // Orange
        } else {
            phase = "Night Time";
            bestBands = "40m - 160m";
            iconColor = 0x001F; // Blue
        }
        
        spr.setTextColor(iconColor);
        spr.drawString(phase, 160, 60, 4);
        
        char timeStr[32];
        sprintf(timeStr, "Local: %02d:%02d (No WiFi)", localHour, t%60);
        spr.setTextColor(TH.menu_item);
        spr.drawString(timeStr, 160, 90, 2);
        
        spr.setTextColor(TH.text);
        spr.drawString("Best Bands:", 160, 115, 2);
        spr.setTextColor(TH.band_text);
        spr.drawString(bestBands, 160, 135, 4);
        
        spr.pushSprite(0, 0);
        return;
    }
    
    // Draw LIVE Data
    char buf[32];
    spr.setTextDatum(TL_DATUM);
    
    // SFI
    spr.setTextColor(TH.menu_param);
    sprintf(buf, "SFI: %.0f", propData.sfi);
    spr.drawString(buf, 30, 60, 4);
    
    // Kp
    sprintf(buf, "Kp: %.1f", propData.kp);
    spr.drawString(buf, 180, 60, 4);
    
    // Draw Band Ratings
    spr.setTextDatum(TC_DATUM);
    spr.setTextColor(TH.menu_item);
    spr.drawString("Band Conditions:", 160, 95, 2);
    
    int y = 115;
    spr.setTextDatum(TL_DATUM);
    spr.setTextColor(TH.band_text);
    spr.drawString("10m:", 30, y, 2);
    spr.drawString("20m:", 100, y, 2);
    spr.drawString("40m:", 170, y, 2);
    spr.drawString("80m:", 240, y, 2);
    
    auto getColor = [](const char* r) {
        if (strcmp(r, "Good") == 0) return (uint16_t)0x07E0;
        if (strcmp(r, "Fair") == 0) return (uint16_t)0xFFE0;
        return (uint16_t)0xF800;
    };
    
    spr.setTextColor(getColor(propData.rating10m));
    spr.drawString(propData.rating10m, 30, y+15, 2);
    spr.setTextColor(getColor(propData.rating20m));
    spr.drawString(propData.rating20m, 100, y+15, 2);
    spr.setTextColor(getColor(propData.rating40m));
    spr.drawString(propData.rating40m, 170, y+15, 2);
    spr.setTextColor(getColor(propData.rating80m));
    spr.drawString(propData.rating80m, 240, y+15, 2);

    spr.pushSprite(0, 0);
}
