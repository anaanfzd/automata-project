#ifndef AUTOMATAVIZ_UITHEME_H
#define AUTOMATAVIZ_UITHEME_H

#include <SFML/Graphics.hpp>

namespace AutomataViz {

struct UITheme {
    // Window & Panels
    static inline const sf::Color BgDark          = sf::Color(15, 23, 42);      // #0f172a
    static inline const sf::Color BgPanel         = sf::Color(30, 41, 59);      // #1e293b
    static inline const sf::Color BgPanelSecondary= sf::Color(24, 33, 47);      // #18212f
    static inline const sf::Color BgInput         = sf::Color(15, 23, 42);      // #0f172a
    static inline const sf::Color BorderDark      = sf::Color(51, 65, 85);      // #334155
    static inline const sf::Color BorderLight     = sf::Color(71, 85, 105);     // #475569

    // Primary Accents
    static inline const sf::Color AccentBlue      = sf::Color(37, 99, 235);     // #2563eb
    static inline const sf::Color AccentCyan      = sf::Color(14, 165, 233);    // #0ea5e9
    static inline const sf::Color AccentGreen     = sf::Color(16, 185, 129);    // #10b981
    static inline const sf::Color AccentAmber     = sf::Color(245, 158, 11);    // #f59e0b
    static inline const sf::Color AccentRed       = sf::Color(239, 68, 68);     // #ef4444
    static inline const sf::Color AccentPurple    = sf::Color(139, 92, 246);    // #8b5cf6

    // Text
    static inline const sf::Color TextWhite       = sf::Color(248, 250, 252);   // #f8fafc
    static inline const sf::Color TextMuted       = sf::Color(148, 163, 184);   // #94a3b8
    static inline const sf::Color TextDim         = sf::Color(100, 116, 139);   // #64748b

    // Automata State Visuals
    static inline const sf::Color NodeDefaultFill = sf::Color(30, 41, 59);      // #1e293b
    static inline const sf::Color NodeDefaultBorder= sf::Color(100, 116, 139);  // #64748b
    static inline const sf::Color NodeStartBorder = sf::Color(56, 189, 248);    // #38bdf8
    static inline const sf::Color NodeFinalBorder = sf::Color(52, 211, 153);    // #34d399
    static inline const sf::Color NodeActiveFill  = sf::Color(30, 58, 138);     // #1e3a8a
    static inline const sf::Color NodeActiveBorder= sf::Color(96, 165, 250);    // #60a5fa
    static inline const sf::Color NodeNewFill     = sf::Color(20, 83, 45);      // #14532d
    static inline const sf::Color NodeNewBorder   = sf::Color(74, 222, 128);    // #4ade80

    // Transitions & Edges
    static inline const sf::Color EdgeNormal      = sf::Color(148, 163, 184);   // #94a3b8
    static inline const sf::Color EdgeHighlight   = sf::Color(56, 189, 248);    // #38bdf8
    static inline const sf::Color EdgeLabelBg     = sf::Color(15, 23, 42, 230); // dark pill
    static inline const sf::Color EdgeLabelBorder = sf::Color(71, 85, 105);

    // Canvas Grid
    static inline const sf::Color GridLine        = sf::Color(30, 41, 59, 120);
    static inline const sf::Color GridDot         = sf::Color(51, 65, 85, 150);
};

} // namespace AutomataViz

#endif // AUTOMATAVIZ_UITHEME_H
