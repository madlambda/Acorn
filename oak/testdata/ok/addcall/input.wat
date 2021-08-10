(module
  (func $i (import "imports" "printnum") (param i32))
  (func $add (param $lhs i32) (param $rhs i32)
    get_local $lhs
    get_local $rhs
    i32.add
    call $i)
  (export "add" (func $add))
)
