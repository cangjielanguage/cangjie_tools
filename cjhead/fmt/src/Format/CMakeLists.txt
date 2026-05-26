file(GLOB FORMAT_SRC "*.cpp" "NodeFomatter/Decl/*.cpp" "NodeFomatter/Node/*.cpp" "NodeFomatter/Pattern/*.cpp" "NodeFomatter/Expr/*.cpp" "NodeFomatter/Type/*.cpp"  "DocProcessor/*.cpp")
if (CANGJIE_AD_FLAG)
set(AUTODIFF_SRC
        NodeFomatter/Expr/AutoDiffExpr/AdjointExprFormatter.cpp
        NodeFomatter/Expr/AutoDiffExpr/GradExprFormatter.cpp
        NodeFomatter/Expr/AutoDiffExpr/VJPExprFormatter.cpp
        NodeFomatter/Expr/AutoDiffExpr/ValWithGradExprFormatter.cpp)
endif ()
add_library(CangjieFormat OBJECT ${FORMAT_SRC} ${AUTODIFF_SRC})
