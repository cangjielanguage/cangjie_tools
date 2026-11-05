package my_module

// 声明不透明类型（只给出类型名，不定义）
@C
public struct OpaqueType {
    init() {
        throw Exception("type should not be inited")
    }
}

// 提供操作接口
foreign func create_opaque(initial_value: Int32): CPointer<OpaqueType>

foreign func set_value(obj: CPointer<OpaqueType>, value: Int32): Unit

foreign func get_value(obj: CPointer<OpaqueType>): Int32

foreign func destroy_opaque(obj: CPointer<OpaqueType>): Unit
