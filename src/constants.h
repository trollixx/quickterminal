#ifndef CONSTANTS_H
#define CONSTANTS_H

namespace ActionId {
// Application
const char About[] = "QTerminal.Application.About";
const char AboutQt[] = "QTerminal.Application.AboutQt";
const char Preferences[] = "QTerminal.Application.Preferences";
const char Exit[] = "QTerminal.Application.Exit";

// Window
const char NewWindow[] = "QTerminal.Window.New";
const char CloseWindow[] = "QTerminal.Window.Close";
const char ShowMenu[] = "QTerminal.Window.ShowMenu";
const char ShowTabs[] = "QTerminal.Window.ShowTabs";

// Tab
const char NewTab[] = "QTerminal.Tab.New";
const char CloseTab[] = "QTerminal.Tab.Close";
const char NextTab[] = "QTerminal.Tab.Next";
const char PreviousTab[] = "QTerminal.Tab.Previous";

// Terminal
const char SplitHorizontally[] = "QTerminal.Terminal.SplitHorizontally";
const char SplitVertically[] = "QTerminal.Terminal.SplitVertically";
const char CloseTerminal[] = "QTerminal.Terminal.Close";
const char Copy[] = "QTerminal.Terminal.Copy";
const char Paste[] = "QTerminal.Terminal.Paste";
const char PasteSelection[] = "QTerminal.Terminal.PasteSelection";
const char Clear[] = "QTerminal.Terminal.Clear";
const char SelectAll[] = "QTerminal.Terminal.SelectAll"; /// TODO: Select All action
const char Find[] = "QTerminal.Terminal.Find";
const char ZoomIn[] = "QTerminal.Terminal.ZoomIn";
const char ZoomOut[] = "QTerminal.Terminal.ZoomOut";
const char ZoomReset[] = "QTerminal.Terminal.ZoomReset";
}

// Fallback icons
namespace ActionIcon {
const char ApplicationExit[] = ":/icons/application-exit.png";
}

#endif // CONSTANTS_H
