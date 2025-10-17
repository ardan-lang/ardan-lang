//
//  gui.h
//  ardan-lang
//
//  Created by Chidume Nnamdi on 15/09/2025.
//

//#ifndef gui_h
//#define gui_h
//
//#include <stdio.h>
//#import <Cocoa/Cocoa.h>
//
//void showWindow();
//
//#endif /* gui_h */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

//Window, Panel, Button, Label, TextBox, CheckBox, ListView,
//Image, ProgressBar, Grid, StackPanel, ScrollView, Dialog,
//Menu, MenuItem, Slider, ComboBox, and Canvas

void gui_init();
void gui_create_window(const char* title, int x, int y, int w, int h);
void gui_add_button(const char* label, int x, int y, int w, int h, void (*callback)());
void gui_run();

#ifdef __cplusplus
}
#endif
