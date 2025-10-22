//
//  OCButton.mm
//  ardan-lang
//
//  Created by Chidume Nnamdi on 22/10/2025.
//

//#import <Cocoa/Cocoa.h>
//#include "Button.h"
//
//using namespace UI;
//
//// Objective-C helper to forward click events to C++
//@interface ButtonTarget : NSObject
//@property (nonatomic, assign) Button* cppButton;
//@end
//
//@implementation ButtonTarget
//- (void)onClick:(id)sender {
//    if (cppButton && cppButton->_onClick) {
//        cppButton->_onClick();
//    }
//}
//@end
//
//namespace UI {
//
//Button::Button(const std::string& title)
//: _button([[NSButton alloc] init]) {
//    setup();
//    setTitle(title);
//}
//
//Button::~Button() {
//    if (_button) {
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
//        if (_button) [_button release];
//        _button = other._button;
//        _onClick = std::move(other._onClick);
//        other._button = nullptr;
//    }
//    return *this;
//}
//
//void Button::setup() {
//    [_button setBezelStyle:NSBezelStyleRounded];
//    [_button setButtonType:NSButtonTypeMomentaryPushIn];
//    [_button setTarget:nil];
//    [_button setAction:nil];
//    attachAction();
//}
//
//void Button::attachAction() {
//    ButtonTarget* target = [[ButtonTarget alloc] init];
//    target.cppButton = this;
//    [_button setTarget:target];
//    [_button setAction:@selector(onClick:)];
//    // Retain target strongly via associated object
//    objc_setAssociatedObject(_button, "cpp_target", target, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
//    [target release];
//}
//
//void Button::setTitle(const std::string& title) {
//    [_button setTitle:[NSString stringWithUTF8String:title.c_str()]];
//}
//
//void Button::setEnabled(bool enabled) {
//    [_button setEnabled:enabled];
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
//void* Button::nativeHandle() const {
//    return _button;
//}
//
//} // namespace UI
