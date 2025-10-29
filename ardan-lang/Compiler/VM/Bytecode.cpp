//
//  Bytecode.cpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 19/09/2025.
//

#include "Bytecode.hpp"

const char* opcodeToString(OpCode op) {
    switch (op) {
        // === Constants & Stack ===
        case OpCode::Nop:               return "Nop";
        case OpCode::LoadConstant:      return "LoadConstant";
        case OpCode::Pop:               return "Pop";
        case OpCode::Dup:               return "Dup";

        // === Locals / Globals ===
        case OpCode::LoadLocal:         return "LoadLocal";
        case OpCode::StoreLocal:        return "StoreLocal";
        case OpCode::LoadGlobal:        return "LoadGlobal";
        case OpCode::StoreGlobal:       return "StoreGlobal";
        case OpCode::CreateGlobal:      return "CreateGlobal";

        // === Objects / Arrays ===
        case OpCode::NewObject:         return "NewObject";
        case OpCode::SetProperty:       return "SetProperty";
        case OpCode::GetProperty:       return "GetProperty";
        case OpCode::NewArray:          return "NewArray";
        case OpCode::ArrayPush:         return "ArrayPush";

        // === Arithmetic / Unary ===
        case OpCode::Add:               return "Add";
        case OpCode::Subtract:          return "Subtract";
        case OpCode::Multiply:          return "Multiply";
        case OpCode::Divide:            return "Divide";
        case OpCode::Modulo:            return "Modulo";
        case OpCode::Power:             return "Power";
        case OpCode::Negate:            return "Negate";
        case OpCode::LogicalNot:        return "LogicalNot";
        case OpCode::Positive:          return "Positive";

        // === Comparisons ===
        case OpCode::Equal:             return "Equal";
        case OpCode::NotEqual:          return "NotEqual";
        case OpCode::LessThan:          return "LessThan";
        case OpCode::LessThanOrEqual:   return "LessThanOrEqual";
        case OpCode::GreaterThan:       return "GreaterThan";
        case OpCode::GreaterThanOrEqual:return "GreaterThanOrEqual";
        case OpCode::StrictEqual:       return "StrictEqual";
        case OpCode::StrictNotEqual:    return "StrictNotEqual";
        case OpCode::LogicalAnd:        return "LogicalAnd";
        case OpCode::LogicalOr:         return "LogicalOr";
        case OpCode::NullishCoalescing: return "NullishCoalescing";
        case OpCode::Increment:         return "Increment";
        case OpCode::Decrement:         return "Decrement";

        // === Bitwise ===
        case OpCode::BitAnd:            return "BitAnd";
        case OpCode::BitOr:             return "BitOr";
        case OpCode::BitXor:            return "BitXor";
        case OpCode::ShiftLeft:         return "ShiftLeft";
        case OpCode::ShiftRight:        return "ShiftRight";
        case OpCode::UnsignedShiftRight:return "UnsignedShiftRight";

        // === Jumps & Control Flow ===
        case OpCode::Jump:              return "Jump";
        case OpCode::JumpIfFalse:       return "JumpIfFalse";
        case OpCode::Loop:              return "Loop";

        // === Calls & Returns ===
        case OpCode::Call:              return "Call";
        case OpCode::Return:            return "Return";

        // === Dynamic Properties ===
        case OpCode::GetPropertyDynamic:return "GetPropertyDynamic";
        case OpCode::SetPropertyDynamic:return "SetPropertyDynamic";
        case OpCode::Dup2:              return "Dup2";

        // === Classes ===
        case OpCode::NewClass:          return "NewClass";

        // === Try / Catch / Finally ===
        case OpCode::Try:               return "Try";
        case OpCode::EndTry:            return "EndTry";
        case OpCode::EndFinally:        return "EndFinally";
        case OpCode::Throw:             return "Throw";

        // === Object Utilities ===
        case OpCode::EnumKeys:          return "EnumKeys";
        case OpCode::GetObjectLength:   return "GetObjectLength";
        case OpCode::GetIndexPropertyDynamic: return "GetIndexPropertyDynamic";

        // === Debug ===
        case OpCode::Debug:             return "Debug";

        // === Chunk & Arguments ===
        case OpCode::LoadChunkIndex:    return "LoadChunkIndex";
        case OpCode::LoadArgument:      return "LoadArgument";
        case OpCode::LoadArguments:     return "LoadArguments";
        case OpCode::Slice:             return "Slice";
        case OpCode::LoadArgumentsLength:return "LoadArgumentsLength";

        // === Closures ===
        case OpCode::CreateClosure:     return "CreateClosure";
        case OpCode::GetUpvalue:        return "GetUpvalue";
        case OpCode::SetUpvalue:        return "SetUpvalue";
        case OpCode::CloseUpvalue:      return "CloseUpvalue";

        // === Stack / Locals ===
        case OpCode::ClearStack:        return "ClearStack";
        case OpCode::ClearLocals:       return "ClearLocals";

        // === Class / Instance ===
        case OpCode::SetStaticProperty: return "SetStaticProperty";
        case OpCode::CreateInstance:    return "CreateInstance";
        case OpCode::InvokeConstructor: return "InvokeConstructor";
        case OpCode::GetThisProperty:   return "GetThisProperty";
        case OpCode::SetThisProperty:   return "SetThisProperty";
        case OpCode::GetThis:           return "GetThis";
        case OpCode::GetParentObject:   return "GetParentObject";
        case OpCode::SuperCall:         return "SuperCall";
        case OpCode::CreateObjectLiteral:return "CreateObjectLiteral";

        // === Halt / Sentinel ===
        case OpCode::Halt:              return "Halt";

        case OpCode::LoadLocalVar:                  return "LoadLocalVar";
        case OpCode::LoadGlobalVar:                 return "LoadGlobalVar";
        case OpCode::StoreLocalVar:                 return "StoreLocalVar";
        case OpCode::StoreGlobalVar:                return "StoreGlobalVar";
        case OpCode::StoreLocalLet:                 return "StoreLocalLet";
        case OpCode::StoreGlobalLet:                return "StoreGlobalLet";
        case OpCode::CreateLocalVar:                return "CreateLocalVar";
        case OpCode::CreateLocalLet:                return "CreateLocalLet";
        case OpCode::CreateLocalConst:              return "CreateLocalConst";
        case OpCode::CreateGlobalVar:               return "CreateGlobalVar";
        case OpCode::CreateGlobalLet:               return "CreateGlobalLet";
        case OpCode::CreateGlobalConst:             return "CreateGlobalConst";
        case OpCode::StoreThisProperty:             return "StoreThisProperty";
        case OpCode::LoadUpvalue:                   return "LoadUpvalue";
        case OpCode::StoreUpvalueVar:               return "StoreUpvalueVar";
        case OpCode::StoreUpvalueLet:               return "StoreUpvalueLet";
        case OpCode::StoreUpvalueConst:             return "StoreUpvalueConst";
        case OpCode::LoadThisProperty:              return "LoadThisProperty";
        case OpCode::TypeOf:                        return "TypeOf";
        case OpCode::Delete:                        return "Delete";
        case OpCode::InstanceOf:                    return "InstanceOf";
        case OpCode::CreateObjectLiteralProperty:   return "CreateObjectLiteralProperty";
        case OpCode::CreateClassPrivatePropertyVar: return "CreateClassPrivatePropertyVar";
        case OpCode::CreateClassPublicPropertyVar:  return "CreateClassPublicPropertyVar";
        case OpCode::CreateClassProtectedPropertyVar:return "CreateClassProtectedPropertyVar";
        case OpCode::CreateClassPrivatePropertyConst:return "CreateClassPrivatePropertyConst";
        case OpCode::CreateClassPublicPropertyConst: return "CreateClassPublicPropertyConst";
        case OpCode::CreateClassProtectedPropertyConst:return "CreateClassProtectedPropertyConst";
        case OpCode::CreateClassPrivateStaticPropertyVar:  return "CreateClassPrivateStaticPropertyVar";
        case OpCode::CreateClassPublicStaticPropertyVar:   return "CreateClassPublicStaticPropertyVar";
        case OpCode::CreateClassProtectedStaticPropertyVar:return "CreateClassProtectedStaticPropertyVar";
        case OpCode::CreateClassPrivateStaticPropertyConst: return "CreateClassPrivateStaticPropertyConst";
        case OpCode::CreateClassPublicStaticPropertyConst:  return "CreateClassPublicStaticPropertyConst";
        case OpCode::CreateClassProtectedStaticPropertyConst:return "CreateClassProtectedStaticPropertyConst";
        case OpCode::CreateClassProtectedStaticMethod: return "CreateClassProtectedStaticMethod";
        case OpCode::CreateClassPrivateStaticMethod:   return "CreateClassPrivateStaticMethod";
        case OpCode::CreateClassPublicStaticMethod:    return "CreateClassPublicStaticMethod";
        case OpCode::CreateClassProtectedMethod:       return "CreateClassProtectedMethod";
        case OpCode::CreateClassPrivateMethod:         return "CreateClassPrivateMethod";
        case OpCode::CreateClassPublicMethod:          return "CreateClassPublicMethod";

    }
    return "Unknown";
}
