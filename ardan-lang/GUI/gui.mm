//
//  gui.cpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 15/09/2025.
//

#include "gui.h"
#import <Cocoa/Cocoa.h>

void showWindow() {
    @autoreleasepool {
        NSApplication *app = [NSApplication sharedApplication];

        NSRect frame = NSMakeRect(200, 200, 400, 300);
        NSWindow *window = [[NSWindow alloc] initWithContentRect:frame
                                                       styleMask:(NSWindowStyleMaskTitled |
                                                                  NSWindowStyleMaskClosable |
                                                                  NSWindowStyleMaskResizable)
                                                         backing:NSBackingStoreBuffered
                                                           defer:NO];
        [window setTitle:@"Hello from GUI/gui.mm"];
        [window makeKeyAndOrderFront:nil];

        [app run];
    }
}
