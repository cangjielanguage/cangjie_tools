// cjbind 0.2.7 生成，请勿编辑, DO NOT EDIT

package my_module

@C
public struct OpaqueType {
    init() {
        throw Exception("type should not be inited")
    }
}

foreign func create_opaque(initial_value: Int32): CPointer<OpaqueType>

foreign func set_value(obj: CPointer<OpaqueType>, value: Int32): Unit

foreign func get_value(obj: CPointer<OpaqueType>): Int32

foreign func destroy_opaque(obj: CPointer<OpaqueType>): Unit
