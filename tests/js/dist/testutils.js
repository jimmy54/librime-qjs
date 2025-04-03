var __defProp = Object.defineProperty
var __name = (target, value) => __defProp(target, 'name', { value, configurable: true })
var totalTests = 0
var passedTests = 0
function assert(condition, message = '') {
  totalTests++
  if (condition) {
    passedTests++
    console.log('\u2713 ' + message)
  } else {
    console.log('\u2717 ' + message)
    console.log('  Expected true, but got false')
    throw new Error('Assertion failed' + (message ? ': ' + message : ''))
  }
}
__name(assert, 'assert')
function assertEquals(actual, expected, message = '') {
  totalTests++
  const actualStr = JSON.stringify(actual)
  const expectedStr = JSON.stringify(expected)
  if (actualStr === expectedStr) {
    passedTests++
    console.log('\u2713 ' + message)
  } else {
    console.log('\u2717 ' + message)
    console.log('  Expected: ' + expectedStr)
    console.log('  Actual:   ' + actualStr)
    throw new Error('Assertion failed' + (message ? ': ' + message : ''))
  }
}
__name(assertEquals, 'assertEquals')
export { assert, assertEquals, passedTests, totalTests }
