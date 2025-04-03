var __defProp = Object.defineProperty
var __name = (target, value) => __defProp(target, 'name', { value, configurable: true })
var _BadTranslator = class _BadTranslator {
  translate() {
    console.log('no return')
  }
}
__name(_BadTranslator, 'BadTranslator')
var BadTranslator = _BadTranslator
export { BadTranslator }
