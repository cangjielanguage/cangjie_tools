// cjbind 0.2.7 生成，请勿编辑, DO NOT EDIT

package my_module

@C
public struct FlexibleString {
    public let length: Int32
    public let data: VArray<UInt8, $0>

    public init(length: Int32, data: VArray<UInt8, $0>) {
        this.length = length
        this.data = data
    }
}
