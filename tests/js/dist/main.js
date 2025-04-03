var __defProp = Object.defineProperty
var __name = (target, value) => __defProp(target, 'name', { value, configurable: true })
function greet(name) {
  return `Hello ${name}!`
}
__name(greet, 'greet')
var _MyClass = class _MyClass {
  constructor(value) {
    this.value = value
  }
  myMethod() {
    return this.value + 1
  }
  greet(name) {
    return greet(name)
  }
}
__name(_MyClass, 'MyClass')
var MyClass = _MyClass
globalThis.MyClass = MyClass
var obj = new MyClass(10)
console.log(obj.greet('QuickJS'))
console.log(obj.myMethod())
var _a
console.log((_a = obj.greet) == null ? void 0 : _a.name.includes('greet'))
var _a2
console.log((_a2 = obj.hello) == null ? void 0 : _a2.name.includes('greet'))
