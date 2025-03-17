export let totalTests = 0
export let passedTests = 0

export function assert(condition, message = '') {
  totalTests++
  if (condition) {
    passedTests++
    console.log('✓ ' + message)
  } else {
    console.log('✗ ' + message)
    console.log('  Expected true, but got false')
    throw new Error('Assertion failed' + (message ? ': ' + message : ''))
  }
}

export function assertEquals(actual, expected, message = '') {
  totalTests++
  const actualStr = JSON.stringify(actual)
  const expectedStr = JSON.stringify(expected)
  if (actualStr === expectedStr) {
    passedTests++
    console.log('✓ ' + message)
  } else {
    console.log('✗ ' + message)
    console.log('  Expected: ' + expectedStr)
    console.log('  Actual:   ' + actualStr)
    throw new Error('Assertion failed' + (message ? ': ' + message : ''))
  }
}
