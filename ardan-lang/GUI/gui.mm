//
//  gui.cpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 15/09/2025.
//

#include "gui.h"
#import <Cocoa/Cocoa.h>

@interface MyAppDelegate : NSObject <NSApplicationDelegate>
@property (strong) NSWindow *window;
@property (strong) NSMutableArray *callbacks; // store C callbacks
@end

@implementation MyAppDelegate
- (void)applicationDidFinishLaunching:(NSNotification *)notification {}
@end

static MyAppDelegate *globalDelegate = nil;

extern "C" void gui_init() {
    @autoreleasepool {
        NSApplication *app = [NSApplication sharedApplication];
        globalDelegate = [[MyAppDelegate alloc] init];
        globalDelegate.callbacks = [NSMutableArray array];
        [app setDelegate:globalDelegate];
    }
}

extern "C" void gui_create_window(const char* title, int x, int y, int w, int h) {
    @autoreleasepool {
        NSRect frame = NSMakeRect(x, y, w, h);
        globalDelegate.window = [[NSWindow alloc] initWithContentRect:frame
                                                            styleMask:(NSWindowStyleMaskTitled |
                                                                       NSWindowStyleMaskClosable |
                                                                       NSWindowStyleMaskResizable)
                                                              backing:NSBackingStoreBuffered
                                                                defer:NO];
        [globalDelegate.window setTitle:[NSString stringWithUTF8String:title]];
        [globalDelegate.window makeKeyAndOrderFront:nil];
    }
}

extern "C" void gui_add_button(const char* label, int x, int y, int w, int h, void (*callback)()) {
    @autoreleasepool {
        NSButton *button = [[NSButton alloc] initWithFrame:NSMakeRect(x, y, w, h)];
        [button setTitle:[NSString stringWithUTF8String:label]];
        [button setBezelStyle:NSBezelStyleRounded];

        // store callback
//        [globalDelegate.callbacks addObject:[NSValue valueWithPointer:callback]];

        // dynamic action
        [button setTarget:globalDelegate];
        [button setAction:@selector(onButtonClick:)];

        // dynamically add selector if not already
//        if (![globalDelegate respondsToSelector:@selector(onButtonClick:)]) {
//            class_addMethod([globalDelegate class],
//                            @selector(onButtonClick:),
//                            (IMP)+[](id self, SEL _cmd, id sender) {
//                                MyAppDelegate *d = (MyAppDelegate*)self;
//                                for (NSValue *val in d.callbacks) {
//                                    void (*cb)() = (void(*)())[val pointerValue];
//                                    cb();
//                                }
//                            },
//                            "v@:@");
//        }

        [[globalDelegate.window contentView] addSubview:button];
    }
}

extern "C" void gui_run() {
    NSApplication *app = [NSApplication sharedApplication];
    [app run];
}
