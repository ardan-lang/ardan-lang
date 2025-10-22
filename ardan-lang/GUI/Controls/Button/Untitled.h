//
//  Untitled.h
//  ardan-lang
//
//  Created by Chidume Nnamdi on 22/10/2025.
//

// File: Button.h
//#pragma once
//
//#include <string>
//#include <functional>
//
//#ifdef __OBJC__
//@class NSButton;
//#else
//class NSButton;
//#endif
//
//namespace UI {
//
//class View; // forward
//
//class Button {
//public:
//    using OnClickHandler = std::function<void()>;
//
//    Button(const std::string& title = "");
//    ~Button();
//
//    Button(const Button&) = delete;
//    Button& operator=(const Button&) = delete;
//
//    Button(Button&&) noexcept;
//    Button& operator=(Button&&) noexcept;
//
//    void setTitle(const std::string& title);
//    std::string title() const;
//
//    void setEnabled(bool enabled);
//    bool enabled() const;
//
//    void setOnClick(OnClickHandler handler);
//
//    // show/hide
//    void show();
//    void hide();
//
//    // attach to a View
//    void attachTo(View* view);
//
//    // native handle for advanced use
//    void* nativeHandle() const;
//
//private:
//    NSButton* _button;
//    OnClickHandler _onClick;
//
//    void setup();
//    void attachAction();
//
//    friend class View; // so View can manage adding subviews
//};
//
//} // namespace UI
//
//
//// File: View.h
//#pragma once
//
//#include <string>
//#include <vector>
//
//#ifdef __OBJC__
//@class NSView;
//#else
//class NSView;
//#endif
//
//namespace UI {
//
//class Button;
//
//class View {
//public:
//    View();
//    ~View();
//
//    View(const View&) = delete;
//    View& operator=(const View&) = delete;
//
//    View(View&&) noexcept;
//    View& operator=(View&&) noexcept;
//
//    // add subviews (Button or View)
//    void addSubview(View* child);
//    void addSubview(Button* button);
//
//    // frame helpers (x,y,w,h)
//    void setFrame(int x, int y, int w, int h);
//    void getFrame(int &x, int &y, int &w, int &h) const;
//
//    void* nativeHandle() const;
//
//private:
//    NSView* _view;
//    std::vector<View*> _children;
//
//    friend class Window;
//};
//
//} // namespace UI
//
//
//// File: Window.h
//#pragma once
//
//#include <string>
//
//#ifdef __OBJC__
//@class NSWindow;
//#else
//class NSWindow;
//#endif
//
//namespace UI {
//
//class View;
//
//class Window {
//public:
//    Window(int width = 400, int height = 300, const std::string& title = "C++ Cocoa Window");
//    ~Window();
//
//    Window(const Window&) = delete;
//    Window& operator=(const Window&) = delete;
//
//    void setContentView(View* view);
//    void show();
//    void close();
//
//    void* nativeHandle() const;
//
//private:
//    NSWindow* _window;
//};
//
//} // namespace UI
//
//
//// File: Button.mm
//#import <Cocoa/Cocoa.h>
//#import <objc/runtime.h>
//#include "Button.h"
//#include "View.h"
//
//using namespace UI;
//
//@interface ButtonTarget : NSObject
//@property (nonatomic, assign) void* cpp_ptr; // raw pointer to Button
//@end
//
//@implementation ButtonTarget
//- (void)onClick:(id)sender {
//    Button* btn = reinterpret_cast<Button*>(_cpp_ptr);
//    if (btn && btn->_onClick) {
//        btn->_onClick();
//    }
//}
//@end
//
//namespace UI {
//
//Button::Button(const std::string& title)
//: _button([[NSButton alloc] initWithFrame:NSMakeRect(0,0,80,30)]) {
//    setup();
//    setTitle(title);
//}
//
//Button::~Button() {
//    if (_button) {
//        // remove associated target to break retain cycles
//        objc_setAssociatedObject(_button, "cpp_target", nil, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
//        [_button removeFromSuperview];
//        [_button release];
//        _button = nil;
//    }
//}
//
//Button::Button(Button&& other) noexcept
//: _button(other._button), _onClick(std::move(other._onClick)) {
//    other._button = nullptr;
//}
//
//Button& Button::operator=(Button&& other) noexcept {
//    if (this != &other) {
//        if (_button) {
//            objc_setAssociatedObject(_button, "cpp_target", nil, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
//            [_button removeFromSuperview];
//            [_button release];
//        }
//        _button = other._button;
//        _onClick = std::move(other._onClick);
//        other._button = nullptr;
//    }
//    return *this;
//}
//
//void Button::setup() {
//    // default appearance
//    [_button setBezelStyle:NSBezelStyleRounded];
//    [_button setButtonType:NSButtonTypeMomentaryPushIn];
//    attachAction();
//}
//
//void Button::attachAction() {
//    // create a small ObjC target object and associate it with the NSButton so it lives as long as the button
//    ButtonTarget* target = [[ButtonTarget alloc] init];
//    target.cpp_ptr = reinterpret_cast<void*>(this);
//    _button.target = target;
//    _button.action = @selector(onClick:);
//    objc_setAssociatedObject(_button, "cpp_target", target, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
//    [target release];
//}
//
//void Button::setTitle(const std::string& title) {
//    NSString* s = title.empty() ? @"" : [NSString stringWithUTF8String:title.c_str()];
//    [_button setTitle:s];
//}
//
//std::string Button::title() const {
//    NSString* s = [_button title];
//    return s ? std::string([s UTF8String]) : std::string();
//}
//
//void Button::setEnabled(bool enabled) {
//    [_button setEnabled:enabled];
//}
//
//bool Button::enabled() const {
//    return [_button isEnabled];
//}
//
//void Button::setOnClick(OnClickHandler handler) {
//    _onClick = std::move(handler);
//}
//
//void Button::show() {
//    [_button setHidden:NO];
//}
//
//void Button::hide() {
//    [_button setHidden:YES];
//}
//
//void Button::attachTo(View* view) {
//    if (!view || !_button) return;
//    NSView* parent = static_cast<NSView*>(view->_view);
//    if (parent) {
//        [parent addSubview:_button];
//    }
//}
//
//void* Button::nativeHandle() const {
//    return _button;
//}
//
//} // namespace UI
//
//
//// File: View.mm
//#import <Cocoa/Cocoa.h>
//#include "View.h"
//#include "Button.h"
//
//using namespace UI;
//
//namespace UI {
//
//View::View()
//: _view([[NSView alloc] initWithFrame:NSMakeRect(0,0,400,300)]) {
//}
//
//View::~View() {
//    if (_view) {
//        // remove all subviews
//        for (NSView* sv in [_view subviews]) {
//            [sv removeFromSuperview];
//        }
//        [_view release];
//        _view = nil;
//    }
//}
//
//View::View(View&& other) noexcept
//: _view(other._view), _children(std::move(other._children)) {
//    other._view = nullptr;
//}
//
//View& View::operator=(View&& other) noexcept {
//    if (this != &other) {
//        if (_view) {
//            for (NSView* sv in [_view subviews]) {
//                [sv removeFromSuperview];
//            }
//            [_view release];
//        }
//        _view = other._view;
//        _children = std::move(other._children);
//        other._view = nullptr;
//    }
//    return *this;
//}
//
//void View::addSubview(View* child) {
//    if (!child || !_view) return;
//    [_view addSubview:child->_view];
//    _children.push_back(child);
//}
//
//void View::addSubview(Button* button) {
//    if (!button || !_view) return;
//    [_view addSubview:button->_button];
//}
//
//void View::setFrame(int x, int y, int w, int h) {
//    if (!_view) return;
//    NSRect f = NSMakeRect(x, y, w, h);
//    [_view setFrame:f];
//}
//
//void View::getFrame(int &x, int &y, int &w, int &h) const {
//    if (!_view) return;
//    NSRect f = [_view frame];
//    x = (int)f.origin.x; y = (int)f.origin.y; w = (int)f.size.width; h = (int)f.size.height;
//}
//
//void* View::nativeHandle() const {
//    return _view;
//}
//
//} // namespace UI
//
//
//// File: Window.mm
//#import <Cocoa/Cocoa.h>
//#include "Window.h"
//#include "View.h"
//
//using namespace UI;
//
//namespace UI {
//
//Window::Window(int width, int height, const std::string& title)
//: _window(nil) {
//    NSUInteger style = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable | NSWindowStyleMaskMiniaturizable;
//    NSRect rect = NSMakeRect(100, 100, width, height);
//    _window = [[NSWindow alloc] initWithContentRect:rect styleMask:style backing:NSBackingStoreBuffered defer:NO];
//    NSString* s = [NSString stringWithUTF8String:title.c_str()];
//    [_window setTitle:s ? s : @"Window"];
//    [_window center];
//}
//
//Window::~Window() {
//    if (_window) {
//        [_window close];
//        [_window release];
//        _window = nil;
//    }
//}
//
//void Window::setContentView(View* view) {
//    if (!_window) return;
//    if (view && view->_view) {
//        [_window setContentView:view->_view];
//    }
//}
//
//void Window::show() {
//    if (!_window) return;
//    [_window makeKeyAndOrderFront:nil];
//    [NSApp activateIgnoringOtherApps:YES];
//}
//
//void Window::close() {
//    if (!_window) return;
//    [_window close];
//}
//
//void* Window::nativeHandle() const {
//    return _window;
//}
//
//} // namespace UI
//
//
//// File: main.mm (example usage)
//#import <Cocoa/Cocoa.h>
//#include "Window.h"
//#include "View.h"
//#include "Button.h"
//#include <iostream>
//
//int main(int argc, const char * argv[]) {
//    @autoreleasepool {
//        [NSApplication sharedApplication];
//
//        // Create window
//        UI::Window win(480, 320, "C++ -> Cocoa Window");
//
//        // Create root view
//        UI::View* root = new UI::View();
//        root->setFrame(0, 0, 480, 320);
//        win.setContentView(root);
//
//        // Create a button
//        UI::Button* btn = new UI::Button("Click Me");
//        btn->setOnClick([](){
//            std::cout << "Button clicked (C++)!\n";
//            // You can also do Cocoa UI updates here if you need to by dispatching to main thread
//        });
//        // position the button (0,0 at bottom-left in Cocoa default coords for views inside window content)
//        btn->nativeHandle();
//        // set frame via ObjC: cast and set frame
//        NSButton* nb = static_cast<NSButton*>(btn->nativeHandle());
//        [nb setFrame:NSMakeRect(20, 20, 120, 40)];
//
//        root->addSubview(btn);
//
//        // show window
//        win.show();
//
//        // Run the app
//        return NSApplicationMain(argc, argv);
//    }
//}

/*
  Build notes:
  - Compile the .mm files as Objective-C++ and link with Cocoa framework.
  - Example (clang++/g++ driver on macOS):
    clang++ -std=c++17 -ObjC++ Button.mm View.mm Window.mm main.mm -framework Cocoa -o cpp_cocoa_ui

  - If you separate headers/implementations into folders, adjust includes accordingly.
  - This code is a starting point — add memory ownership helpers or smart-pointer wrappers as you prefer.
*/
