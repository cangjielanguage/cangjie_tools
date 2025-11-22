
### Testing Commmand

#### ArkTS to Cangjie binding

```bash
./target/bin/main --lib --module-name="my_module" -d ./tests/cases -o ./tests/expected/my_module/
```

#### C to Cangjie binding

./target/bin/main -c --module-name="my_module" -d ./tests/c_cases -o ./tests/expected/c_module/ --clang-args="-I/usr/include"
```
