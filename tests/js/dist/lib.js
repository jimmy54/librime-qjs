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
export { MyClass, greet }
