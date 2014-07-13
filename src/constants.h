/****************************************************************************
**
** Copyright (C) 2014 Oleg Shparber <trollixx+quickterminal@gmail.com>
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License as
** published by the Free Software Foundation; either version 2 of
** the License, or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <http://www.gnu.org/licenses/>.
**
****************************************************************************/

#ifndef CONSTANTS_H
#define CONSTANTS_H

namespace ActionId {
// Application
const char About[] = "QuickTerminal.Application.About";
const char AboutQt[] = "QuickTerminal.Application.AboutQt";
const char Preferences[] = "QuickTerminal.Application.Preferences";
const char Exit[] = "QuickTerminal.Application.Exit";

// Window
const char NewWindow[] = "QuickTerminal.Window.New";
const char CloseWindow[] = "QuickTerminal.Window.Close";
const char ShowMenu[] = "QuickTerminal.Window.ShowMenu";
const char ShowTabs[] = "QuickTerminal.Window.ShowTabs";
const char ToggleVisibility[] = "QuickTerminal.Window.ToggleVisibility"; // DropDown Mode

// Tab
const char NewTab[] = "QuickTerminal.Tab.New";
const char CloseTab[] = "QuickTerminal.Tab.Close";
const char NextTab[] = "QuickTerminal.Tab.Next";
const char PreviousTab[] = "QuickTerminal.Tab.Previous";

// Terminal
const char SplitHorizontally[] = "QuickTerminal.Terminal.SplitHorizontally";
const char SplitVertically[] = "QuickTerminal.Terminal.SplitVertically";
const char CloseTerminal[] = "QuickTerminal.Terminal.Close";
const char Copy[] = "QuickTerminal.Terminal.Copy";
const char Paste[] = "QuickTerminal.Terminal.Paste";
const char PasteSelection[] = "QuickTerminal.Terminal.PasteSelection";
const char Clear[] = "QuickTerminal.Terminal.Clear";
const char SelectAll[] = "QuickTerminal.Terminal.SelectAll"; /// TODO: Select All action
const char Find[] = "QuickTerminal.Terminal.Find";
const char ZoomIn[] = "QuickTerminal.Terminal.ZoomIn";
const char ZoomOut[] = "QuickTerminal.Terminal.ZoomOut";
const char ZoomReset[] = "QuickTerminal.Terminal.ZoomReset";
}

// Fallback icons
namespace Icon {
const char Application[] = ":/icons/application-icon.png";
const char ApplicationExit[] = ":/icons/application-exit.png";
}

#endif // CONSTANTS_H
