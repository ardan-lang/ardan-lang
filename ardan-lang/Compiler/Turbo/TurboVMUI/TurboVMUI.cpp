//
//  TurboVMUI.cpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 28/10/2025.
//

#include "TurboVMUI.hpp"
#include "../TurboVM.hpp"

void TurboVM::runCreateUIView(Instruction i) {
    
    // TurboOpCode::CreateUIView, uiViewReg, emitConstant(Value::str(expr->name))
    
    Value uiName = frame->chunk->constants[i.b];
    
    auto ui = toValue(env->get(uiName.toString()));
    
    if (ui.type != ValueType::CLASS) {
        throw runtime_error("View must be a class.");
    }
    
    shared_ptr<JSClass> klass = ui.classValue;
    Value klass_value = Value::klass(klass);
    Value instance_obj = CreateInstance(klass_value);
    
    shared_ptr<JSObject> uiObject = instance_obj.objectValue;
    
    vector<Value> arg = { argStack.begin(), argStack.end() };
    argStack.clear();
    
    invokeMethod(instance_obj, "constructor", arg);
    
    frame->registers[i.a] = instance_obj;
    
}

void TurboVM::runAddChildSubView(Instruction i) {
    // TurboOpCode::AddChildSubView, childUIViewReg, uiViewReg
    
    auto childUIView = frame->registers[i.a];
    
    auto uiView = frame->registers[i.b];
        
    invokeMethod(uiView, "addComponent", { childUIView });
    
}

void TurboVM::runSetUIViewArgument(Instruction i) {
    
}

void TurboVM::runCallUIViewModifier(Instruction i) {
    // TurboOpCode::CallUIViewModifier, uiViewReg, i
    
    vector<Value> args = { argStack.begin(), argStack.end() };
    argStack.clear();
    
    string modifier_name = frame->chunk->constants[i.b].toString();
    Value uiView = frame->registers[i.a];
    
    auto result = uiView.objectValue->get(modifier_name);
    
    frame->registers[i.a] = callMethod(result, args, uiView);

}
